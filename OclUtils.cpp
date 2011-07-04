
#include <stdio.h>
#include <stdlib.h> // abort()
#include <string>   // std::string
#include <cstdarg>  // va_arg, va_list, etc.
#include <cmath>

#include <Memory.hpp> // Print_N_Times()

#include "OclUtils.hpp"

const char TMP_FILE[] = "/tmp/gpu_usage.txt";
const char string_base[] = "Device = ";


// *****************************************************************************
char *read_opencl_kernel(const std::string filename, int *length)
{
    FILE *f = fopen(filename.c_str(), "r");
    void *buffer;

    if (!f)
    {
        std_cout << "Unable to open " << filename << " for reading\n";
        abort();
    }

    fseek(f, 0, SEEK_END);
    *length = int(ftell(f));
    fseek(f, 0, SEEK_SET);

    buffer = malloc(*length+1);
    *length = int(fread(buffer, 1, *length, f));
    fclose(f);
    ((char*)buffer)[*length] = '\0';

    return (char*)buffer;
}

// *****************************************************************************
OpenCL_platform::OpenCL_platform()
{
    id = NULL;
}

// *****************************************************************************
void OpenCL_platform::Print()
{
    std_cout
        << "    Platform information:\n"
        << "        id:         " << id << "\n"
        << "        profile:    " << profile << "\n"
        << "        version:    " << version << "\n"
        << "        name:       " << name << "\n"
        << "        vendor:     " << vendor << "\n"
        << "        extensions: " << extensions << "\n"
    ;

    std_cout
        << "    Available OpenCL devices on platform:\n";
    devices_list.Print();
}

// *****************************************************************************
OpenCL_platforms_list::OpenCL_platforms_list()
{
}

// *****************************************************************************
void OpenCL_platforms_list::Initialize()
{
    int err;
    cl_uint nb_platforms;
    char tmp_string[1024];

    // Get number of platforms available
    err = clGetPlatformIDs(0, NULL, &nb_platforms);
    OpenCL_Test_Success(err, "clGetPlatformIDs");

    if (nb_platforms == 0)
    {
        std_cout << "ERROR: No OpenCL platform found! Exiting.\n";
        abort();
    }

    platforms.resize(nb_platforms);

    // Get a list of the OpenCL platforms available.
    cl_platform_id *tmp_platforms;
    tmp_platforms = (cl_platform_id*) calloc_and_check(nb_platforms, sizeof(cl_platform_id), "cl_platform_id*");
    err = clGetPlatformIDs(nb_platforms, tmp_platforms, NULL);
    OpenCL_Test_Success(err, "clGetPlatformIDs");

    std::list<OpenCL_platform>::iterator it = platforms.begin();
    for (unsigned int i = 0 ; i < nb_platforms ; i++, it++)
    {
        OpenCL_platform &platform = *it;
        platform.id = tmp_platforms[i];

        // Query platform information
        err = clGetPlatformInfo(it->id, CL_PLATFORM_PROFILE, sizeof(tmp_string), &tmp_string, NULL);
        OpenCL_Test_Success(err, "clGetPlatformInfo (CL_PLATFORM_PROFILE)");
        platform.profile = std::string(tmp_string);

        err = clGetPlatformInfo(it->id, CL_PLATFORM_VERSION, sizeof(tmp_string), &tmp_string, NULL);
        OpenCL_Test_Success(err, "clGetPlatformInfo (CL_PLATFORM_VERSION)");
        platform.version = std::string(tmp_string);

        err = clGetPlatformInfo(it->id, CL_PLATFORM_NAME, sizeof(tmp_string), &tmp_string, NULL);
        OpenCL_Test_Success(err, "clGetPlatformInfo (CL_PLATFORM_NAME)");
        platform.name = std::string(tmp_string);

        err = clGetPlatformInfo(it->id, CL_PLATFORM_VENDOR, sizeof(tmp_string), &tmp_string, NULL);
        OpenCL_Test_Success(err, "clGetPlatformInfo (CL_PLATFORM_VENDOR)");
        platform.vendor = std::string(tmp_string);

        err = clGetPlatformInfo(it->id, CL_PLATFORM_EXTENSIONS, sizeof(tmp_string), &tmp_string, NULL);
        OpenCL_Test_Success(err, "clGetPlatformInfo (CL_PLATFORM_EXTENSIONS)");
        platform.extensions = std::string(tmp_string);

        platform.devices_list.Initialize(platform);

    }

    free_me(tmp_platforms, nb_platforms);
}

// *****************************************************************************
void OpenCL_platforms_list::Print()
{
    std_cout << "Available platforms:\n";
    std::list<OpenCL_platform>::iterator it = platforms.begin();
    for (unsigned int i = 0 ; i < platforms.size() ; i++, it++)
    {
        it->Print();
    }
}

// *****************************************************************************
OpenCL_device::OpenCL_device()
{
    parent_platform             = NULL;
    name                        = "";
    id                          = -1;
    device_is_gpu               = false;
    max_compute_units           = 0;

    device  = NULL;
    context = NULL;

    device_is_used = false;
}

// *****************************************************************************
OpenCL_device::~OpenCL_device()
{
    if (context)
        clReleaseContext(context);
}

// *****************************************************************************
void OpenCL_device::Set_Information(const int _id, cl_device_id _device, const bool _device_is_gpu)
{
    id              = _id;
    device          = _device;
    device_is_gpu   = _device_is_gpu;

    std::ifstream file(TMP_FILE, std::ios::in);

    if (file)
    {
        std::string line;

        while (std::getline(file, line))
        {
            char string_to_find[512];
            sprintf(string_to_find, "%s%d", string_base, id);

            if (line.find(string_to_find) != std::string::npos)
            {
                device_is_used = true;
            }
        }

        file.close();
    }
    else
    {
        device_is_used = false;
    }

    char tmp_string[4096];

    cl_int err;

    // http://www.khronos.org/registry/cl/sdk/1.0/docs/man/xhtml/clGetDeviceInfo.html
    err  = clGetDeviceInfo(device, CL_DEVICE_ADDRESS_BITS,                  sizeof(cl_uint),                        &address_bits,                  NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_AVAILABLE,                     sizeof(cl_bool),                        &available,                     NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_COMPILER_AVAILABLE,            sizeof(cl_bool),                        &compiler_available,            NULL);
    //err |= clGetDeviceInfo(device, CL_DEVICE_DOUBLE_FP_CONFIG,              sizeof(cl_device_fp_config),            &double_fp_config,              NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_ENDIAN_LITTLE,                 sizeof(cl_bool),                        &endian_little,                 NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_ERROR_CORRECTION_SUPPORT,      sizeof(cl_bool),                        &error_correction_support,      NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_EXECUTION_CAPABILITIES,        sizeof(cl_device_exec_capabilities),    &execution_capabilities,        NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE,         sizeof(cl_ulong),                       &global_mem_cache_size,         NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_CACHE_TYPE,         sizeof(cl_device_mem_cache_type),       &global_mem_cache_type,         NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE,     sizeof(cl_uint),                        &global_mem_cacheline_size,     NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE,               sizeof(cl_ulong),                       &global_mem_size,               NULL);
    //err |= clGetDeviceInfo(device, CL_DEVICE_HALF_FP_CONFIG,                sizeof(cl_device_fp_config),            &half_fp_config,                NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_IMAGE_SUPPORT,                 sizeof(cl_bool),                        &image_support,                 NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_IMAGE2D_MAX_HEIGHT,            sizeof(size_t),                         &image2d_max_height,            NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_IMAGE2D_MAX_WIDTH,             sizeof(size_t),                         &image2d_max_width,             NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_IMAGE3D_MAX_DEPTH,             sizeof(size_t),                         &image3d_max_depth,             NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_IMAGE3D_MAX_HEIGHT,            sizeof(size_t),                         &image3d_max_height,            NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_IMAGE3D_MAX_WIDTH,             sizeof(size_t),                         &image3d_max_width,             NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE,                sizeof(cl_ulong),                       &local_mem_size,                NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_TYPE,                sizeof(cl_device_local_mem_type),       &local_mem_type,                NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_MAX_CLOCK_FREQUENCY,           sizeof(cl_uint),                        &max_clock_frequency,           NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS,             sizeof(cl_uint),                        &max_compute_units,             NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_MAX_CONSTANT_ARGS,             sizeof(cl_uint),                        &max_constant_args,             NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE,      sizeof(cl_ulong),                       &max_constant_buffer_size,      NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_MAX_MEM_ALLOC_SIZE,            sizeof(cl_ulong),                       &max_mem_alloc_size,            NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_MAX_PARAMETER_SIZE,            sizeof(size_t),                         &max_parameter_size,            NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_MAX_READ_IMAGE_ARGS,           sizeof(cl_uint),                        &max_read_image_args,           NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_MAX_SAMPLERS,                  sizeof(cl_uint),                        &max_samplers,                  NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE,           sizeof(size_t),                         &max_work_group_size,           NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,      sizeof(cl_uint),                        &max_work_item_dimensions,      NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_SIZES,           sizeof(max_work_item_sizes),            &max_work_item_sizes,           NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_MAX_WRITE_IMAGE_ARGS,          sizeof(cl_uint),                        &max_write_image_args,          NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_MEM_BASE_ADDR_ALIGN,           sizeof(cl_uint),                        &mem_base_addr_align,           NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE,      sizeof(cl_uint),                        &min_data_type_align_size,      NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_PLATFORM,                      sizeof(cl_platform_id),                 &platform,                      NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR,   sizeof(cl_uint),                        &preferred_vector_width_char,   NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT,  sizeof(cl_uint),                        &preferred_vector_width_short,  NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT,    sizeof(cl_uint),                        &preferred_vector_width_int,    NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG,   sizeof(cl_uint),                        &preferred_vector_width_long,   NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT,  sizeof(cl_uint),                        &preferred_vector_width_float,  NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, sizeof(cl_uint),                        &preferred_vector_width_double, NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_PROFILING_TIMER_RESOLUTION,    sizeof(size_t),                         &profiling_timer_resolution,    NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_QUEUE_PROPERTIES,              sizeof(cl_command_queue_properties),    &queue_properties,              NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_SINGLE_FP_CONFIG,              sizeof(cl_device_fp_config),            &single_fp_config,              NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_TYPE,                          sizeof(cl_device_type),                 &type,                          NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_VENDOR_ID,                     sizeof(cl_uint),                        &vendor_id,                     NULL);

    err |= clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS,                    sizeof(tmp_string),                     &tmp_string,                    NULL);
    extensions = std::string(tmp_string);
    err |= clGetDeviceInfo(device, CL_DEVICE_NAME,                          sizeof(tmp_string),                     &tmp_string,                    NULL);
    name = std::string(tmp_string);
    err |= clGetDeviceInfo(device, CL_DEVICE_PROFILE,                       sizeof(tmp_string),                     &tmp_string,                    NULL);
    profile = std::string(tmp_string);
    err |= clGetDeviceInfo(device, CL_DEVICE_VENDOR,                        sizeof(tmp_string),                     &tmp_string,                    NULL);
    vendor = std::string(tmp_string);
    err |= clGetDeviceInfo(device, CL_DEVICE_VERSION,                       sizeof(tmp_string),                     &tmp_string,                    NULL);
    version = std::string(tmp_string);
    err |= clGetDeviceInfo(device, CL_DRIVER_VERSION,                       sizeof(tmp_string),                     &tmp_string,                    NULL);
    driver_version = std::string(tmp_string);

    OpenCL_Test_Success(err, "OpenCL_device::Set_Information()");

    // Nvidia specific extensions
    // http://developer.download.nvidia.com/compute/cuda/3_2_prod/toolkit/docs/OpenCL_Extensions/cl_nv_device_attribute_query.txt
    if (extensions.find("cl_nv_device_attribute_query") != std::string::npos)
    {
        err  = clGetDeviceInfo(device, CL_DEVICE_COMPUTE_CAPABILITY_MAJOR_NV,   sizeof(cl_uint),                    &nvidia_device_compute_capability_major,    NULL);
        err |= clGetDeviceInfo(device, CL_DEVICE_COMPUTE_CAPABILITY_MINOR_NV,   sizeof(cl_uint),                    &nvidia_device_compute_capability_minor,    NULL);
        err |= clGetDeviceInfo(device, CL_DEVICE_REGISTERS_PER_BLOCK_NV,        sizeof(cl_uint),                    &nvidia_device_registers_per_block,         NULL);
        err |= clGetDeviceInfo(device, CL_DEVICE_WARP_SIZE_NV,                  sizeof(cl_uint),                    &nvidia_device_warp_size,                   NULL);
        err |= clGetDeviceInfo(device, CL_DEVICE_GPU_OVERLAP_NV,                sizeof(cl_bool),                    &nvidia_device_gpu_overlap,                 NULL);
        err |= clGetDeviceInfo(device, CL_DEVICE_KERNEL_EXEC_TIMEOUT_NV,        sizeof(cl_bool),                    &nvidia_device_kernel_exec_timeout,         NULL);
        err |= clGetDeviceInfo(device, CL_DEVICE_INTEGRATED_MEMORY_NV,          sizeof(cl_bool),                    &nvidia_device_integrated_memory,           NULL);

        OpenCL_Test_Success(err, "OpenCL_device::Set_Information() (Nvida specific extensions)");
    }
    else
    {
        is_nvidia                               = false;
        nvidia_device_compute_capability_major  = 0;
        nvidia_device_compute_capability_minor  = 0;
        nvidia_device_registers_per_block       = 0;
        nvidia_device_warp_size                 = 0;
        nvidia_device_gpu_overlap               = false;
        nvidia_device_kernel_exec_timeout       = false;
        nvidia_device_integrated_memory         = false;
    }

    if      (type == CL_DEVICE_TYPE_CPU)
        type_string = "CL_DEVICE_TYPE_CPU";
    else if (type == CL_DEVICE_TYPE_GPU)
        type_string = "CL_DEVICE_TYPE_GPU";
    else if (type == CL_DEVICE_TYPE_ACCELERATOR)
        type_string = "CL_DEVICE_TYPE_ACCELERATOR";
    else if (type == CL_DEVICE_TYPE_DEFAULT)
        type_string = "CL_DEVICE_TYPE_DEFAULT";
    else
    {
        std_cout << "ERROR: Unknown OpenCL type \"" << type << "\". Exiting.\n";
        abort();
    }

    if      (queue_properties == CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE)
        queue_properties_string = "CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE";
    else if (queue_properties == CL_QUEUE_PROFILING_ENABLE)
        queue_properties_string = "CL_QUEUE_PROFILING_ENABLE";
    else
    {
        std_cout << "ERROR: Unknown OpenCL queue properties \"" << queue_properties << "\". Exiting.\n";
        abort();
    }

    single_fp_config_string = "";
    if      (single_fp_config & CL_FP_DENORM)
        single_fp_config_string += "CL_FP_DENORM, ";
    if (single_fp_config & CL_FP_INF_NAN)
        single_fp_config_string += "CL_FP_INF_NAN, ";
    if (single_fp_config & CL_FP_ROUND_TO_NEAREST)
        single_fp_config_string += "CL_FP_ROUND_TO_NEAREST, ";
    if (single_fp_config & CL_FP_ROUND_TO_ZERO)
        single_fp_config_string += "CL_FP_ROUND_TO_ZERO, ";
    if (single_fp_config & CL_FP_ROUND_TO_INF)
        single_fp_config_string += "CL_FP_ROUND_TO_INF, ";
    if (single_fp_config & CL_FP_FMA)
        single_fp_config_string += "CL_FP_FMA, ";
}

// *****************************************************************************
cl_int OpenCL_device::Set_Context()
{
    cl_int err;
    context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    return err;
}

// *****************************************************************************
void OpenCL_device::Print()
{
    std_cout << "    "; Print_N_Times("-", 105);

    std_cout
        << "    name: " << name << "\n"
        << "        id:                             " << id << "\n"
        << "        parent platform:                " << (parent_platform != NULL ? parent_platform->name : "") << "\n"
        << "        device_is_used:                 " << (device_is_used ? "yes" : "no ") << "\n"
        << "        max_compute_unit:               " << max_compute_units << "\n"
        << "        device is GPU?                  " << (device_is_gpu ? "yes" : "no ") << "\n"

        << "        address_bits:                   " << address_bits << "\n"
        << "        available:                      " << (available ? "yes" : "no") << "\n"
        << "        compiler_available:             " << (compiler_available ? "yes" : "no") << "\n"
        << "        double_fp_config:               " << double_fp_config << "\n"
        << "        endian_little:                  " << (endian_little ? "yes" : "no") << "\n"
        << "        error_correction_support:       " << (error_correction_support ? "yes" : "no") << "\n"
        << "        execution_capabilities:         " << execution_capabilities << "\n"
        << "        global_mem_cache_size:          " << Bytes_in_String(global_mem_cache_size) << "\n"
        << "        global_mem_cache_type:          " << global_mem_cache_type << "\n"
        << "        global_mem_cacheline_size:      " << Bytes_in_String(global_mem_cacheline_size) << "\n"
        << "        global_mem_size:                " << Bytes_in_String(global_mem_size) << "\n"
        << "        half_fp_config:                 " << half_fp_config << "\n"
        << "        image_support:                  " << (image_support ? "yes" : "no") << "\n"
        << "        image2d_max_height:             " << image2d_max_height << "\n"
        << "        image2d_max_width:              " << image2d_max_width << "\n"
        << "        image3d_max_depth:              " << image3d_max_depth << "\n"
        << "        image3d_max_height:             " << image3d_max_height << "\n"
        << "        image3d_max_width:              " << image3d_max_width << "\n"
        << "        local_mem_size:                 " << Bytes_in_String(local_mem_size) << "\n"
        << "        local_mem_type:                 " << local_mem_type << "\n"
        << "        max_clock_frequency:            " << max_clock_frequency << " MHz\n"
        << "        max_compute_units:              " << max_compute_units << "\n"
        << "        max_constant_args:              " << max_constant_args << "\n"
        << "        max_constant_buffer_size:       " << Bytes_in_String(max_constant_buffer_size) << "\n"
        << "        max_mem_alloc_size:             " << Bytes_in_String(max_mem_alloc_size) << "\n"
        << "        max_parameter_size:             " << Bytes_in_String(max_parameter_size) << "\n"
        << "        max_read_image_args:            " << max_read_image_args << "\n"
        << "        max_samplers:                   " << max_samplers << "\n"
        << "        max_work_group_size:            " << Bytes_in_String(max_work_group_size) << "\n"
        << "        max_work_item_dimensions:       " << max_work_item_dimensions << "\n"
        << "        max_work_item_sizes:            " << "(" << max_work_item_sizes[0] << ", " << max_work_item_sizes[1] << ", " << max_work_item_sizes[2] << ")\n"
        << "        max_write_image_args:           " << max_write_image_args << "\n"
        << "        mem_base_addr_align:            " << mem_base_addr_align << "\n"
        << "        min_data_type_align_size:       " << Bytes_in_String(min_data_type_align_size) << "\n"
        << "        platform:                       " << platform << "\n"
        << "        preferred_vector_width_char:    " << preferred_vector_width_char << "\n"
        << "        preferred_vector_width_short:   " << preferred_vector_width_short << "\n"
        << "        preferred_vector_width_int:     " << preferred_vector_width_int << "\n"
        << "        preferred_vector_width_long:    " << preferred_vector_width_long << "\n"
        << "        preferred_vector_width_float:   " << preferred_vector_width_float << "\n"
        << "        preferred_vector_width_double:  " << preferred_vector_width_double << "\n"
        << "        profiling_timer_resolution:     " << profiling_timer_resolution << " ns\n"
        << "        queue_properties:               " << queue_properties_string << " (" << queue_properties << ")" << "\n"
        << "        single_fp_config:               " << single_fp_config_string << " (" << single_fp_config << ")\n"
        << "        type:                           " << type_string << " (" << type << ")" << "\n"
        << "        vendor_id:                      " << vendor_id << "\n"
        << "        extensions:                     " << extensions << "\n"
        //<< "        name:                           " << name << "\n"
        << "        profile:                        " << profile << "\n"
        << "        vendor:                         " << vendor << "\n"
        << "        version:                        " << version << "\n"
        << "        driver_version:                 " << driver_version << "\n";

    if (is_nvidia)
    {
        std_cout
            << "        GPU is from NVidia\n"
            << "            nvidia_device_compute_capability_major: " << nvidia_device_compute_capability_major << "\n"
            << "            nvidia_device_compute_capability_minor: " << nvidia_device_compute_capability_minor << "\n"
            << "            nvidia_device_registers_per_block:      " << nvidia_device_registers_per_block      << "\n"
            << "            nvidia_device_warp_size:                " << nvidia_device_warp_size                << "\n"
            << "            nvidia_device_gpu_overlap:              " << (nvidia_device_gpu_overlap         ? "yes" : "no") << "\n"
            << "            nvidia_device_kernel_exec_timeout:      " << (nvidia_device_kernel_exec_timeout ? "yes" : "no") << "\n"
            << "            nvidia_device_integrated_memory:        " << (nvidia_device_integrated_memory   ? "yes" : "no") << "\n";
    }
    else
    {
        std_cout
            << "        GPU is NOT from NVidia\n";
    }

    // Avialable global memory on device
    std_cout.Format(0, 3, 'g');
    std_cout << "Available memory (global):   " << Bytes_in_String(global_mem_size) << "\n";

    // Avialable local memory on device
    std_cout.Format(0, 3, 'g');
    std_cout << "Available memory (local):    " << Bytes_in_String(local_mem_size) << "\n";

    // Avialable constant memory on device
    std_cout.Format(0, 3, 'g');
    std_cout << "Available memory (constant): " << Bytes_in_String(max_constant_buffer_size) << "\n";
}

// *****************************************************************************
bool OpenCL_device::operator<(const OpenCL_device &other)
{
    // Start by checking if ones not in use. When this is the case give it priority.
    // Then compare the maximum number of compute unit
    // NOTE: We want a sorted list where device with higher compute units
    //       are located at the top (front). We thus invert the test here.
    bool result = false;

    if      (this->device_is_used == false && other.device_is_used == true)  // "this" wins (it is not in use).
        result = true;
    else if (this->device_is_used == true  && other.device_is_used == false) // "other" wins (it is not in use).
        result = false;
    else // both are used or not used. Thus, we must compare the ammount of compute units.
    {
        if (this->max_compute_units > other.max_compute_units) // "this" wins (having more compute units).
            result = true;
        else                                                 // "other" wins (having more or equal compute units).
            result = false;
    }

    return result;
}

// *****************************************************************************
OpenCL_devices_list::OpenCL_devices_list()
{
    nb_cpu  = 0;
    nb_gpu  = 0;
    err     = 0;
    preferred_device = NULL;
    write_to_tmp = true;
}

// *****************************************************************************
OpenCL_devices_list::~OpenCL_devices_list()
{
    if (write_to_tmp)
    {
        std::string file_content; // Write the data from the file.

        std::ifstream file_read(TMP_FILE, std::ios::in);

        if (file_read)
        {
            std::string line;

            while (std::getline(file_read, line))
            {
                char string_to_find[512];
                sprintf(string_to_find, "%s%d", string_base, preferred_device->Get_Id());

                // Read every line except the one corresponding to the current device.
                if (line.find(string_to_find) == std::string::npos)
                {
                    file_content += line + "\n"; // Add the lines to the string.
                }
            }

            file_read.close();
        }

        // Write back the string to file (the current device being deleted).
        std::ofstream file_write(TMP_FILE, std::ios::out | std::ios::trunc);

        if (file_write)
        {
            file_write << file_content;

            file_write.close();
        }
    }
}

// *****************************************************************************
void OpenCL_devices_list::Print()
{
    for (it = device_list.begin() ; it != device_list.end() ; ++it)
        it->Print();

    Print_N_Times("*", 109);
    std_cout << "Order of preference for OpenCL devices:\n";
    int i = 0;
    for (it = device_list.begin() ; it != device_list.end() ; ++it)
    {
        std_cout << i++ << ".   " << it->Get_Name() << " (id = " << it->Get_ID() << ")\n";
    }
    Print_N_Times("*", 109);
}

// *****************************************************************************
void OpenCL_devices_list::Initialize(const OpenCL_platform &_platform)
{
    Print_N_Times("-", 109);
    std_cout << "OpenCL: Initialize OpenCL object and context\n" << std::flush;

    platform_id = _platform.id;

    // Get the number of GPU devices available to the platform
    // Number of GPU
    err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 0, NULL, &nb_gpu);
    if (err == CL_DEVICE_NOT_FOUND)
    {
        std_cout << "OpenCL: WARNING: Can't find a usable GPU!\n" << std::flush;
        err = CL_SUCCESS;
    }
    OpenCL_Test_Success(err, "clGetDeviceIDs()");

    // Number of CPU
    err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_CPU, 0, NULL, &nb_cpu);
    if (err == CL_DEVICE_NOT_FOUND)
    {
        std_cout << "OpenCL: WARNING: Can't find a usable CPU!\n" << std::flush;
        err = CL_SUCCESS;
    }
    OpenCL_Test_Success(err, "clGetDeviceIDs()");
    assert(nb_devices() >= 1);

    // Create the device list
    device_list.resize(nb_devices());
    // Temporary list
    cl_device_id *tmp_devices;

    it = device_list.begin();

    bool is_all_devices_in_use = true; // We want to know if all devices are in use.

    // Add CPUs to list
    if (nb_cpu >= 1)
    {
        tmp_devices = new cl_device_id[nb_cpu];
        err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_CPU, nb_cpu, tmp_devices, NULL);
        OpenCL_Test_Success(err, "clGetDeviceIDs()");

        for (unsigned int i = 0 ; i < nb_cpu ; ++i, ++it)
        {
            it->Set_Information(i, tmp_devices[i], false); // device_is_gpu == false

            // When one device is not in use... One device is not in use!
            if (!it->Is_In_Use())
                is_all_devices_in_use = false;

        }
        delete[] tmp_devices;
    }

    // Add GPUs to list
    if (nb_gpu >= 1)
    {
        tmp_devices = new cl_device_id[nb_gpu];
        err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, nb_gpu, tmp_devices, NULL);
        OpenCL_Test_Success(err, "clGetDeviceIDs()");
        for (unsigned int i = 0 ; i < nb_gpu ; ++i, ++it)
        {
            it->Set_Information(nb_cpu+i, tmp_devices[i], true); // device_is_gpu == true

            // When one device is not in use... One device is not in use!
            if (!it->Is_In_Use())
                is_all_devices_in_use = false;
        }
        delete[] tmp_devices;
    }

    assert(it == device_list.end());

    // When all devices are in use we ask the user if he would like
    // his simulation to be forced.
    if (is_all_devices_in_use == true)
    {
        bool correct_answer = false;

        // Don't write to tmp. This would suppress lines created by other program running.
        write_to_tmp = false;

        while (!correct_answer)
        {
            // Ask the user if he still wants to execute the program.
            std_cout << "OpenCL: WARNING: It seem's that all OpenCL devices are in use!\n"
                     << "                 If you are certain no other program is using the device(s), you can delete the file '" << TMP_FILE << "'\n"
                     << "                 Do you want to force the execution and continue? [y/n]\n";
            std::string answer;
            std::cin >> answer;

            if (answer == "yes" || answer == "Y" || answer == "y" || answer == "oui" || answer == "O" || answer == "o")
            {
                correct_answer = true;
                std_cout << "Proceeding... \n";
            }
            else if (answer == "no" || answer == "No" || answer == "N" || answer == "n" || answer == "non" || answer == "Non")
            {
                correct_answer = true;
                std_cout << "Exiting... \n";
                abort();
            }
            else
            {
                correct_answer = false;
                std_cout << "You entered an invalid answer!\n";
            }
        }
    }

    for (it = device_list.begin() ; it != device_list.end() ; ++it)
        it->parent_platform = &_platform;

    // Sort the list. The order is defined by "OpenCL_device::operator<" (line 112)
    device_list.sort();

    Print();

    // Initialize context on a device
    preferred_device = NULL;    // The preferred device is unknown for now.
    for (it = device_list.begin() ; it != device_list.end() ; ++it)
    {
        std_cout << "Trying to set an OpenCL context on " << it->Get_Name() << " (id = " << it->Get_ID() << ")...";
        if (it->Set_Context() == CL_SUCCESS)
        {
            std_cout << " Success!\n";
            preferred_device = &(*it);

            if (write_to_tmp)
            {
                std::ofstream file(TMP_FILE, std::ios::out | std::ios::app);

                if (file)
                {
                    file << string_base << preferred_device->Get_Id() << std::endl;

                    file.close();
                }
            }

            break;
        }
        else
        {
            std_cout << " Failed. Maybe next one will work?\n";
        }
    }
    if (preferred_device == NULL)
    {
        std_cout << "ERROR: Cannot set an OpenCL context on any of the available devices!\nExiting" << std::flush;
        abort();
    }
}

// *****************************************************************************

OpenCL_Kernel::OpenCL_Kernel(std::string _filename, bool _use_mt, cl_context _context, cl_device_id _device_id):
                           filename(_filename), use_mt(_use_mt), context(_context), device_id(_device_id)
{
}

// *****************************************************************************
OpenCL_Kernel::~OpenCL_Kernel()
{
    if (kernel)  clReleaseKernel(kernel);
    if (program) clReleaseProgram(program);

    delete[] global_work_size;
    delete[] local_work_size;
}

// *****************************************************************************
void OpenCL_Kernel::Build(std::string _kernel_name, std::string _compiler_options)
{
    compiler_options = _compiler_options;
    kernel_name      = _kernel_name;

    // **********************************************************
    // Load and build the kernel
    Load_Program_From_File();

    // Create the kernel.
    kernel = clCreateKernel(program, kernel_name.c_str(), &err);
    OpenCL_Test_Success(err, "clCreateKernel");

    // **********************************************************
    // Get the maximum work group size
    //err = clGetKernelWorkGroupInfo(kernel_md, list.Prefered_OpenCL_Device(), CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &maxWorkSize, NULL);
    //std_cout << "Maximum kernel work group size: " << maxWorkSize << "\n";
    //OpenCL_Test_Success(err, "clGetKernelWorkGroupInfo");
}

// *****************************************************************************
void OpenCL_Kernel::Compute_Work_Size(int N)
/**
 * When this method is used the local work size is choosed by OpenCL.
 * Thus, the global work size is set to N.
 * Don't use this function if using MT.
 */
{
    dimension = 1;

    global_work_size = new size_t[dimension];
    local_work_size = new size_t[dimension];

    global_work_size[0] = N;
    local_work_size = NULL;
}

// *****************************************************************************
void OpenCL_Kernel::Compute_Work_Size(int N, int _p, int _q)
/**
 * Use this method for a kernel who benefits from MT.
 */
{
    dimension = 2;

    global_work_size = new size_t[dimension];
    local_work_size = new size_t[dimension];

    if (_p * _q > MAX_LOCAL_WORK_SIZE)
    {
        _p = MAX_LOCAL_WORK_SIZE / _q;
    }
    else if (_q == 1 && N < _p)
    {
        _p = N;
    }

    local_work_size[0] = _p;
    local_work_size[1] = _q;

    global_work_size[0] = Get_Multiple_Of_Work_Size(N, _p*_q);
    global_work_size[1] = _q;

    p = _p;
    q = _q;

    //std_cout << "Number of dimension of problem space: " << dimension << "\n";
    //std_cout << "global_workGroupSize[" << 0 << "]: " << globalWorkSizes[id][0] << " \n";
    //for(int d = 0 ; d < dimension ; d++)
    //    "local_workGroupSize[" << d << "]: " << localWorkSizes[id][d] << "\n";
}

// *****************************************************************************
cl_kernel OpenCL_Kernel::Get_Kernel() const
{
    return kernel;
}

// *****************************************************************************
size_t *OpenCL_Kernel::Get_Global_Work_Size() const
{
    return global_work_size;
}

// *****************************************************************************
size_t *OpenCL_Kernel::Get_Local_Work_Size() const
{
    return local_work_size;
}

// *****************************************************************************
int OpenCL_Kernel::Get_Dimension() const
{
    return dimension;
}

// *****************************************************************************
bool OpenCL_Kernel::Uses_MT() const
{
    return use_mt;
}

// *****************************************************************************
void OpenCL_Kernel::Load_Program_From_File()
{
    // Program Setup
    int pl;
    size_t program_length;
    std_cout << "Loading OpenCL program from \"" << filename << "\"...\n";

    // Loads the contents of the file at the given path
    char* cSourceCL = read_opencl_kernel(filename, &pl);
    program_length = (size_t) pl;

    // create the program
    program = clCreateProgramWithSource(context, 1, (const char **) &cSourceCL, &program_length, &err);
    OpenCL_Test_Success(err, "clCreateProgramWithSource");

    Build_Executable();
}

// *****************************************************************************
void OpenCL_Kernel::Build_Executable()
/**
 * Build the program executable
 */
{
    std_cout << "Building the program..." << std::flush;

#ifdef YDEBUG
    // Include debugging symbols in kernel compilation
#ifndef MACOSX
    compiler_options += "-g ";
#endif // #ifndef MACOSX
#endif // #ifdef YDEBUG
    // Verbose compilation? Does not do much... And it may break kernel compilation
    // with invalid kernel name error.
    //compiler_options += "-cl-nv-verbose";

    std_cout << "\nOpenCL Compiler Options: " << compiler_options.c_str() << "\n" << std::flush;

    err = clBuildProgram(program, 0, NULL, compiler_options.c_str(), NULL, NULL);

    char *build_log;
    size_t ret_val_size;
    err = clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &ret_val_size);
    build_log = new char[ret_val_size+1];
    err = clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, ret_val_size, build_log, NULL);
    build_log[ret_val_size] = '\0';
    OpenCL_Test_Success(err, "1. clGetProgramBuildInfo");
    std_cout << "OpenCL kernels file compilation log: \n" << build_log << "\n";

    if (err != CL_SUCCESS)
    {
        cl_build_status build_status;
        err = clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &build_status, NULL);
        OpenCL_Test_Success(err, "0. clGetProgramBuildInfo");

        //char *build_log;
        //size_t ret_val_size;
        err = clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &ret_val_size);
        OpenCL_Test_Success(err, "1. clGetProgramBuildInfo");
        //build_log = new char[ret_val_size+1];
        err = clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, ret_val_size, build_log, NULL);
        build_log[ret_val_size] = '\0';

        OpenCL_Test_Success(err, "2. clGetProgramBuildInfo");
        std_cout << "Build log: \n" << build_log << "\n";
        std_cout << "Kernel did not built correctly. Exiting.\n";

        std_cout << std::flush;
        abort();
    }

    delete[] build_log;

    std_cout << "done.\n";
}

// *****************************************************************************
int OpenCL_Kernel::Get_Multiple_Of_Work_Size(int n, int _p)
{
    int multipleOfWorkSize = 0;

    if (n < _p)
    {
        multipleOfWorkSize = n;
    }
    else if (n % _p == 0)
    {
        multipleOfWorkSize = n;
    }
    else
    {
        multipleOfWorkSize = _p*std::floor(n/_p) + _p;
    }

    return multipleOfWorkSize;
}

// ********** End of file ******************************************************
