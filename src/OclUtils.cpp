/*
 Copyright 2011 Nicolas Bigaouette <nbigaouette@gmail.com>
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program. If not, see <http://www.gnu.org/licenses/>.

 https://github.com/nbigaouette/oclutils
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>

#include <cerrno>       // errno, EWOULDBLOCK
#include <cstring>      // strlen()
#include <cmath>
#include <algorithm>    // std::ostringstream
#include <sstream>
#include <unistd.h>     // getpid()

#include <sys/time.h> // timeval

#include "OclUtils.hpp"


// *****************************************************************************
// Quote something, usefull to quote a macro's value
#ifndef _QUOTEME
#define _QUOTEME(x) #x
#endif // #ifndef _QUOTEME
#ifndef QUOTEME
#define QUOTEME(x) _QUOTEME(x)
#endif // #ifndef QUOTEME

#define assert(x)                                       \
    if (!(x)) {                                         \
        std_cout                                        \
            << "##########################"             \
            << "##########################"             \
            << "##########################\n"           \
            << "Assertion failed in \"" << __FILE__     \
            << "\", line " << __LINE__ << ": "          \
            << "!(" << QUOTEME(x) << ")\n"              \
            << "##########################"             \
            << "##########################"             \
            << "##########################\n"           \
            << std::flush;                              \
        abort();                                        \
    }

// *****************************************************************************
const double B_to_KiB   = 9.76562500000000e-04;
const double B_to_MiB   = 9.53674316406250e-07;
const double B_to_GiB   = 9.31322574615479e-10;
const double KiB_to_B   = 1024.0;
const double KiB_to_MiB = 9.76562500000000e-04;
const double KiB_to_GiB = 9.53674316406250e-07;
const double MiB_to_B   = 1048576.0;
const double MiB_to_KiB = 1024.0;
const double MiB_to_GiB = 9.76562500000000e-04;
const double GiB_to_B   = 1073741824.0;
const double GiB_to_KiB = 1048576.0;
const double GiB_to_MiB = 1024.0;

// *****************************************************************************
// **************** Local functions prototypes *********************************
void Print_N_Times(const std::string x, const int N, const bool newline = true);
std::string Get_Lock_Filename(const int device_id, const int platform_id_offset,
                              const std::string &platform_name, const std::string &device_name);
int Lock_File(const char *path, const bool quiet = false);
void Unlock_File(int f, const bool quiet = false);
void Wait(const double duration_sec);

void * calloc_and_check(uint64_t nb, size_t s, std::string msg = "");

// **************************************************************
void * calloc_and_check(uint64_t nb, size_t s, std::string msg)
{
    void *p = NULL;
    const uint64_t nb_s = nb * s;
    p = calloc(nb, s);
    if (p == NULL)
    {
        std_cout << "ERROR!!!\n";
        std_cout << "    Allocation of ";
        std_cout << nb << " x " << s << " bytes = " << nb_s << " bytes\n";
        std_cout << "                                               (";
        std_cout
            << nb_s * B_to_KiB << " KiB, "
            << nb_s * B_to_KiB << " MiB, "
            << nb_s * B_to_GiB << " GiB)\n"
            << "    FAILED!!!\n";
        if (msg != "")
        {
            std_cout << "Comment: " << msg << std::endl;
        }
        std_cout << "Aborting.\n" << std::flush;
        abort();
    }

    return p;
}


// *****************************************************************************
inline std::string Bytes_in_String(const uint64_t bytes)
{
    std::ostringstream MyStream;
    MyStream
        << bytes << " bytes ("
            << B_to_KiB * bytes << " KiB, "
            << B_to_MiB * bytes << " MiB, "
            << B_to_GiB * bytes << " GiB)"
        << std::flush;
    return (MyStream.str());
}

// *****************************************************************************
void Print_N_Times(const std::string x, const int N, const bool newline)
{
    for (int i = 0 ; i < N ; i++)
    {
        std_cout << x;
    }

    if (newline)
        std_cout << "\n";
}

// *****************************************************************************
std::string Get_Lock_Filename(const int device_id, const int platform_id_offset,
                              const std::string &platform_name, const std::string &device_name)
{
    std::string f = "/tmp/OpenCL_"; // Beginning of lock filename
    char t[4096];
    sprintf(t, "Platform%d_Device%d__%s_%s", platform_id_offset, device_id, platform_name.c_str(), device_name.c_str()); //generate string filename
    unsigned int len = (unsigned int) strlen(t);
    for (unsigned int i = 0; i < len; i++)
    {
        // Replace all non alphanumeric characters with underscore
        if (!isalpha(t[i]) && !isdigit(t[i]))
        {
            t[i] = '_';
        }
    }
    f += t;
    f += ".lck"; // File suffix
    return f;
}

// *****************************************************************************
int Lock_File(const char *path, const bool quiet)
/**
 * Attempt to lock file, and check lock status on lock file
 * @return      file handle if locked, or -1 if failed
 */
{
    if (not quiet)
        std_cout << "OpenCL: Attempt to acquire lock on file " << path << "..." << std::flush;

    // Open file
    int f = open(path, O_CREAT | O_TRUNC, 0666);
    if (f == -1)
    {
        if (not quiet)
            std_cout << "Could not open lock file!\n" << std::flush;
        return -1; // Open failed
    }

    // Set file's permissions.
    // Needed so that multi-user systems can share the lock file.
    // WARNING: The call's return value is not tested. We know it would
    //          fail if the file is owned by another process. So just try
    //          to do it, but don't test it. Anyway, what is important
    //          is the locking with flock().
    fchmod(f, 0666);

    // Aquire the lock. Since the locking might fail (because another process is checking the lock too)
    // we try a maximum of 5 times, with a random delay  between 1 and 10 seconds between tries.
    pid_t pid = getpid();
    const int max_retry = 5;
    srand(pid * (unsigned int)time(NULL));
    int err;
    for (int i = 0 ; i < max_retry ; i++)
    {
        // Try to acquire lock
        err = flock(f, LOCK_EX | LOCK_NB);

        // If it succeeds, exist the loop
        if (err != -1)
            break;

        // If it it did not succeeds, sleep for a random
        // time (between 1 and 10 seconds) and retry
        const double delay = ((double(rand()) / double(RAND_MAX)) * 9.0) + 1.0;
        char delay_string[64];
        sprintf(delay_string, "%.4f", delay);
        std_cout
            << "\nOpenCL: WARNING: Failed to acquire a lock on file '" << path << "'.\n"
            << "                 Waiting " << delay_string << " seconds before retrying (" << i+1 << "/" << max_retry << ")...\n" << std::flush;
        Wait(delay);
        std_cout << "                 Done waiting.";
        if (i+1 < max_retry)
            std_cout << " Retrying.";
        std_cout << "\n";
    }

    if (err == -1)
    {
        if (errno == EWOULDBLOCK)
        {
            close(f);
            if (not quiet)
                std_cout << "Lock file is already locked!\n";
            return -1; // File is locked
        }
        else
        {
            std_cout << "File lock operation failed!\n";
            close(f);
            return -1; // Another error occurred
        }
    }

    if (not quiet)
        std_cout << "Success!\n" << std::flush;

    return f;
}

// *****************************************************************************
void Unlock_File(int f, const bool quiet)
/**
 * Unlock file
 */
{
    if (not quiet)
        std_cout << "Closing lock file.\n";
    close(f); // Close file automatically unlocks file
}

// *****************************************************************************
void Wait(const double duration_sec)
{
    double delay = 0.0;
    timeval initial, now;
    gettimeofday(&initial, NULL);
    while (delay <= duration_sec)
    {
        gettimeofday(&now, NULL);
        // Transform time into double delay
        delay = double(now.tv_sec - initial.tv_sec) + 1.0e-6*double(now.tv_usec - initial.tv_usec);
        //printf("Delay = %.6f   max = %.6f\n", delay, duration_sec);
    }
}

// *****************************************************************************
bool Verify_if_Device_is_Used(const int device_id, const int platform_id_offset,
                              const std::string &platform_name, const std::string &device_name)
{
    int check = Lock_File(Get_Lock_Filename(device_id, platform_id_offset, platform_name, device_name).c_str());

    if (check == -1)
    {
        return true;                // Device is used
    }
    else
    {
        Unlock_File(check, true);   // Close file, quiet == true
        return false;               // Device not in use
    }
}

// *****************************************************************************
char *read_opencl_kernel(const std::string filename, int *length)
{
    FILE *f = fopen(filename.c_str(), "r");
    void *buffer;

    if (!f)
    {
        std_cout << "OpenCL: Unable to open " << filename << " for reading\n";
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
    id      = NULL;
    vendor  = "Not set";
    name    = "Not set";
    version = "Not set";
    extensions = "Not set";
    profile = "Not set";
    id_offset  = 0;
}

// *****************************************************************************
void OpenCL_platform::Initialize(const std::string _key, int _id_offset, cl_platform_id _id,
                                 OpenCL_platforms_list *_platform_list,
                                 const std::string preferred_platform)
{
    key             = _key;
    id_offset       = _id_offset;
    id              = _id;
    platform_list   = _platform_list;

    cl_int err;
    char tmp_string[4096];

    // Query platform information
    err = clGetPlatformInfo(id, CL_PLATFORM_PROFILE, sizeof(tmp_string), &tmp_string, NULL);
    OpenCL_Test_Success(err, "clGetPlatformInfo (CL_PLATFORM_PROFILE)");
    profile = std::string(tmp_string);

    err = clGetPlatformInfo(id, CL_PLATFORM_VERSION, sizeof(tmp_string), &tmp_string, NULL);
    OpenCL_Test_Success(err, "clGetPlatformInfo (CL_PLATFORM_VERSION)");
    version = std::string(tmp_string);

    err = clGetPlatformInfo(id, CL_PLATFORM_NAME, sizeof(tmp_string), &tmp_string, NULL);
    OpenCL_Test_Success(err, "clGetPlatformInfo (CL_PLATFORM_NAME)");
    name = std::string(tmp_string);

    err = clGetPlatformInfo(id, CL_PLATFORM_VENDOR, sizeof(tmp_string), &tmp_string, NULL);
    OpenCL_Test_Success(err, "clGetPlatformInfo (CL_PLATFORM_VENDOR)");
    vendor = std::string(tmp_string);

    err = clGetPlatformInfo(id, CL_PLATFORM_EXTENSIONS, sizeof(tmp_string), &tmp_string, NULL);
    OpenCL_Test_Success(err, "clGetPlatformInfo (CL_PLATFORM_EXTENSIONS)");
    extensions = std::string(tmp_string);

    // Initialize the platform's devices
    devices_list.Initialize(*this, preferred_platform);
}

// *****************************************************************************
void OpenCL_platform::Print_Preferred() const
{
    Print_N_Times("-", 109);
    std_cout << "OpenCL: Platform and device to be used:\n";
    std_cout << "OpenCL: Platform's name:             " << Name() << "\n";
    std_cout << "OpenCL: Platform's best device:      " << devices_list.preferred_device->Get_Name() << " (id = "
                                                        << devices_list.preferred_device->Get_ID()   << ")\n";
    Print_N_Times("-", 109);
}
// *****************************************************************************
void OpenCL_platform::Lock_Best_Device()
{
    if (Preferred_OpenCL().Is_Lockable())
    {
        Preferred_OpenCL().Lock();
    }
}

// *****************************************************************************
void OpenCL_platform::Print() const
{
    std_cout
        << "    Platform information:\n"
        << "        vendor:     " << vendor << "\n"
        << "        name:       " << name << "\n"
        << "        version:    " << version << "\n"
        << "        extensions: " << extensions << "\n"
        << "        id:         " << id << "\n"
        << "        profile:    " << profile << "\n"
        << "        key:        " << key << "\n"
        << "        list:       " << platform_list << "\n"
    ;

    std_cout
        << "    Available OpenCL devices on platform:\n";
    devices_list.Print();
}

// *****************************************************************************
void OpenCL_platforms_list::Initialize(const std::string &_preferred_platform, const bool _use_locking)
{
    preferred_platform = _preferred_platform;
    use_locking = _use_locking;
    if (use_locking)
        std_cout << "OpenCL: File locking mechanism enabled. Will probably fail if run under a queueing system.\n" << std::flush;
    else
        std_cout << "OpenCL: File locking mechanism disabled. Must be disabled when using queueing system.\n" << std::flush;

    cl_int err;
    cl_uint nb_platforms;

    Print_N_Times("-", 109);
    std_cout << "OpenCL: Getting a list of platform(s)..." << std::flush;

    // Get number of platforms available
    err = clGetPlatformIDs(0, NULL, &nb_platforms);
    OpenCL_Test_Success(err, "clGetPlatformIDs");

    if (nb_platforms == 0)
    {
        std_cout << "\nERROR: No OpenCL platform found! Exiting.\n";
        abort();
    }

    // Get a list of the OpenCL platforms available.
    cl_platform_id *tmp_platforms;
    tmp_platforms = new cl_platform_id[nb_platforms];
    err = clGetPlatformIDs(nb_platforms, tmp_platforms, NULL);
    OpenCL_Test_Success(err, "clGetPlatformIDs");

    std_cout << " done.\n";

    if (nb_platforms == 1)
        std_cout << "OpenCL: Initializing the available platform...\n";
    else
        std_cout << "OpenCL: Initializing the " << nb_platforms << " available platforms...\n";

    char tmp_string[4096];

    // Print the platform list first
    for (unsigned int i = 0 ; i < nb_platforms ; i++)
    {
        cl_platform_id tmp_platform_id = tmp_platforms[i];

        err = clGetPlatformInfo(tmp_platform_id, CL_PLATFORM_VENDOR, sizeof(tmp_string), &tmp_string, NULL);
        OpenCL_Test_Success(err, "clGetPlatformInfo (CL_PLATFORM_VENDOR)");

        std_cout << "        (" << i+1 << "/" << nb_platforms << ") " << tmp_string << "\n";
    }

    // This offset allows distinguishing in LOCK_FILE the devices that can appear in different platforms.
    int platform_id_offset = 0;

    // Add every platforms to the map
    for (unsigned int i = 0 ; i < nb_platforms ; i++)
    {
        cl_platform_id tmp_platform_id = tmp_platforms[i];

        err = clGetPlatformInfo(tmp_platform_id, CL_PLATFORM_VENDOR, sizeof(tmp_string), &tmp_string, NULL);
        OpenCL_Test_Success(err, "clGetPlatformInfo (CL_PLATFORM_VENDOR)");

        std::string platform_vendor = std::string(tmp_string);
        std::transform(platform_vendor.begin(), platform_vendor.end(), platform_vendor.begin(), tolower);
        std::string key;
        if      (platform_vendor.find("nvidia") != std::string::npos)
            key = OPENCL_PLATFORMS_NVIDIA;
        else if (platform_vendor.find("advanced micro devices") != std::string::npos or platform_vendor.find("amd") != std::string::npos)
            key = OPENCL_PLATFORMS_AMD;
        else if (platform_vendor.find("intel") != std::string::npos)
            key = OPENCL_PLATFORMS_INTEL;
        else if (platform_vendor.find("apple") != std::string::npos)
            key = OPENCL_PLATFORMS_APPLE;
        else
        {
            std_cout << "ERROR: Unknown OpenCL platform \"" << platform_vendor << "\"! Exiting.\n" << std::flush;
            abort();
        }

        OpenCL_platform &platform = platforms[key];

        platform.Initialize(key, platform_id_offset, tmp_platforms[i], this, preferred_platform);

        ++platform_id_offset;
    }

    delete[] tmp_platforms;

    /*
    // Debugging: Add dummy platform
    {
        platforms["test"].vendor    = "Dummy Vendor";
        platforms["test"].name      = "Dummy platform";
        platforms["test"].version   = "Dummy Version 3.1415";
        platforms["test"].extensions= "Dummy Extensions";
        platforms["test"].profile   = "Dummy Profile";
        ++nb_platforms;
    }
    */

    // If the preferred platform is not specified, set it to the first one.
    if (preferred_platform == "-1" or preferred_platform == "")
    {
        preferred_platform = platforms.begin()->first;
    }

    // Initialize the best device on the preferred platform.
    platforms[preferred_platform].devices_list.Set_Preferred_OpenCL();
}

// *****************************************************************************
void OpenCL_platforms_list::Print() const
{
    std_cout << "OpenCL: Available platforms:\n";
    std::map<std::string,OpenCL_platform>::const_iterator it = platforms.begin();
    for (unsigned int i = 0 ; i < platforms.size() ; i++, it++)
    {
        it->second.Print();
    }

    Print_Preferred();
}

// *****************************************************************************
void OpenCL_platforms_list::Print_Preferred() const
{
    std::map<std::string,OpenCL_platform>::const_iterator it;
    it = platforms.find(preferred_platform);
    if (it == platforms.end())
    {
        std_cout << "ERROR: Cannot find platform '" << preferred_platform << "'. Aborting.\n" << std::flush;
        abort();
    }
    assert(it->second.devices_list.preferred_device != NULL);

    it->second.Print_Preferred();
}

// *****************************************************************************
OpenCL_platform & OpenCL_platforms_list::operator[](const std::string key)
{
    std::map<std::string,OpenCL_platform>::iterator it;

    if (key == "-1" or key == "")
    {
        if (platforms.size() == 0)
        {
            std_cout << "ERROR: Trying to access a platform but the list is uninitialized! Aborting.\n" << std::flush;
            abort();
        }
        // Just take the first one.
        it = platforms.begin();
    }
    else
    {
        // Find the right one
        it = platforms.find(key);
        if (it == platforms.end())
        {
            Print();
            std_cout << "Cannot find platform \"" << key << "\"! Aborting.\n" << std::flush;
            abort();
        }
    }
    return it->second;
}

// *****************************************************************************
void OpenCL_platforms_list::Set_Preferred_OpenCL(const int _preferred_device)
{
    platforms[preferred_platform].devices_list.Set_Preferred_OpenCL(_preferred_device);
}

// *****************************************************************************
OpenCL_device::OpenCL_device()
{
    object_is_initialized       = false;
    parent_platform             = NULL;
    name                        = "";
    device_id                   = -1;
    device_is_gpu               = false;
    max_compute_units           = 0;
    device                      = NULL;
    context                     = NULL;
    device_is_in_use            = false;
    is_lockable                 = true;
    file_locked                 = false;
}

// *****************************************************************************
OpenCL_device::~OpenCL_device()
{
    Destructor();
}

// *****************************************************************************
void OpenCL_device::Destructor()
{
    if (context)
        clReleaseContext(context);

    Unlock();
}

// *****************************************************************************
void OpenCL_device::Set_Information(const int _id, cl_device_id _device,
                                    const int platform_id_offset,
                                    const std::string &platform_name,
                                    const bool _device_is_gpu,
                                    const OpenCL_platform * const _parent_platform                                   )
{
    object_is_initialized = true;
    device_id       = _id;
    device          = _device;
    device_is_gpu   = _device_is_gpu;
    parent_platform = _parent_platform;

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

        is_nvidia                               = true;
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

    queue_properties_string = "";
    if (queue_properties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE)
        queue_properties_string += "CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, ";
    if (queue_properties & CL_QUEUE_PROFILING_ENABLE)
        queue_properties_string += "CL_QUEUE_PROFILING_ENABLE, ";

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

    assert(parent_platform                  != NULL);
    assert(parent_platform->Platform_List() != NULL);
    if (parent_platform->Platform_List()->Use_Locking())
    {
        device_is_in_use = Verify_if_Device_is_Used(device_id, platform_id_offset, platform_name, name);
        is_lockable = true;
    }
    else
    {
        device_is_in_use = false;
        is_lockable = false;
    }
}

// *****************************************************************************
cl_int OpenCL_device::Set_Context()
{
    cl_int err = CL_SUCCESS+1;
    pid_t pid = getpid();
    const int max_retry = 5;
    srand(pid * (unsigned int)time(NULL));
    for (int i = 0 ; i < max_retry ; i++)
    {
        // Try to set an OpenCL context on the device
        context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);

        // If it succeeds, exist the loop
        if (err == CL_SUCCESS)
            break;

        // If it it did not succeeds, sleep for a random
        // time (between 1 and 10 seconds) and retry
        const double delay = ((double(rand()) / double(RAND_MAX)) * 9.0) + 1.0;
        char delay_string[64];
        sprintf(delay_string, "%.4f", delay);
        std_cout
            << "\nOpenCL: WARNING: Failed to set an OpenCL context on the device.\n"
            << "                 Waiting " << delay_string << " seconds before retrying (" << i+1 << "/" << max_retry << ")...\n" << std::flush;
        Wait(delay);
        std_cout << "                 Done waiting.";
        if (i+1 < max_retry)
            std_cout << " Retrying.";
        std_cout << "\n";
    }
    return err;
}

// *****************************************************************************
void OpenCL_device::Print() const
{
    std_cout << "    "; Print_N_Times("-", 105);

    std_cout
        << "    name: " << name << "\n"
        << "        id:                             " << device_id << "\n"
        << "        parent platform:                " << (parent_platform != NULL ? parent_platform->Name() : "") << "\n"
        << "        device_is_used:                 " << (device_is_in_use ? "yes" : "no ") << "\n"
        << "        max_compute_unit:               " << max_compute_units << "\n"
        << "        device is GPU?                  " << (device_is_gpu ? "yes" : "no ") << "\n"

        << "        address_bits:                   " << address_bits << "\n"
        << "        available:                      " << (available ? "yes" : "no") << "\n"
        << "        compiler_available:             " << (compiler_available ? "yes" : "no") << "\n"
        //<< "        double_fp_config:               " << double_fp_config << "\n"
        << "        endian_little:                  " << (endian_little ? "yes" : "no") << "\n"
        << "        error_correction_support:       " << (error_correction_support ? "yes" : "no") << "\n"
        << "        execution_capabilities:         " << execution_capabilities << "\n"
        << "        global_mem_cache_size:          " << Bytes_in_String(global_mem_cache_size) << "\n"
        << "        global_mem_cache_type:          " << global_mem_cache_type << "\n"
        << "        global_mem_cacheline_size:      " << Bytes_in_String(global_mem_cacheline_size) << "\n"
        << "        global_mem_size:                " << Bytes_in_String(global_mem_size) << "\n"
        //<< "        half_fp_config:                 " << half_fp_config << "\n"
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
    std_cout << "        Available memory (global):   " << Bytes_in_String(global_mem_size) << "\n";

    // Avialable local memory on device
    std_cout << "        Available memory (local):    " << Bytes_in_String(local_mem_size) << "\n";

    // Avialable constant memory on device
    std_cout << "        Available memory (constant): " << Bytes_in_String(max_constant_buffer_size) << "\n";
}

// *****************************************************************************
void OpenCL_device::Lock()
{
    lock_file = Lock_File(Get_Lock_Filename(device_id, parent_platform->Id_Offset(), parent_platform->Name(), name).c_str());
    if (lock_file == -1)
    {
        std_cout << "An error occurred locking the file!\n" << std::flush;
        abort();
    }
    file_locked = true; // File is now locked
}

// *****************************************************************************
void OpenCL_device::Unlock()
{
    if (file_locked == true)
    {
        Unlock_File(lock_file);
        file_locked = false;
    }
}

// *****************************************************************************
bool OpenCL_device::operator<(const OpenCL_device &other)
{
    // Start by checking if ones not in use. When this is the case give it priority.
    // Then compare the maximum number of compute unit
    // NOTE: We want a sorted list where device with higher compute units
    //       are located at the top (front). We thus invert the test here.
    bool result = false;

    if      (this->device_is_in_use == false && other.device_is_in_use == true)  // "this" wins (it is not in use).
        result = true;
    else if (this->device_is_in_use == true  && other.device_is_in_use == false) // "other" wins (it is not in use).
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
    is_initialized      = false;
    platform            = NULL;
    nb_cpu              = 0;
    nb_gpu              = 0;
    err                 = 0;
    preferred_device    = NULL;
}

// *****************************************************************************
OpenCL_devices_list::~OpenCL_devices_list()
{
    if (not is_initialized)
        return;
}

// *****************************************************************************
OpenCL_device & OpenCL_devices_list::Preferred_OpenCL()
{
    if (preferred_device == NULL)
    {
        std_cout << "ERROR: No OpenCL device is present!\n"
        << "Make sure you call OpenCL_platforms.platforms[<WANTED PLATFORM>] with a valid (i.e. created) platform!\n" << std::flush;
        abort();
    }

    return *preferred_device;
}

// *****************************************************************************
void OpenCL_devices_list::Print() const
{
    if (device_list.size() == 0)
    {
        std_cout << "        None" << "\n";
    }
    else
    {
        for (std::list<OpenCL_device>::const_iterator it = device_list.begin() ; it != device_list.end() ; ++it)
            it->Print();

        std_cout << "        "; Print_N_Times("*", 101);
        std_cout << "        Order of preference for OpenCL devices for this platform:\n";
        int i = 0;
        for (std::list<OpenCL_device>::const_iterator it = device_list.begin() ; it != device_list.end() ; ++it)
        {
            std_cout << "        " << i++ << ".   " << it->Get_Name() << " (id = " << it->Get_ID() << ")\n";
        }
        std_cout << "        "; Print_N_Times("*", 101);
    }
}

// *****************************************************************************
void OpenCL_devices_list::Initialize(const OpenCL_platform &_platform,
                                     const std::string &preferred_platform)
{
    std_cout << "OpenCL: Initialize platform \"" << _platform.Name() << "\"'s device(s)\n";

    platform            = &_platform;

    // Get the number of GPU devices available to the platform
    // Number of GPU
    err = clGetDeviceIDs(platform->Id(), CL_DEVICE_TYPE_GPU, 0, NULL, &nb_gpu);
    if (err == CL_DEVICE_NOT_FOUND)
    {
        std_cout << "OpenCL: WARNING: Can't find a usable GPU!\n";
        err = CL_SUCCESS;
    }
    OpenCL_Test_Success(err, "clGetDeviceIDs()");

    // Number of CPU
    err = clGetDeviceIDs(platform->Id(), CL_DEVICE_TYPE_CPU, 0, NULL, &nb_cpu);
    if (err == CL_DEVICE_NOT_FOUND)
    {
        std_cout << "OpenCL: WARNING: Can't find a usable CPU!\n";
        err = CL_SUCCESS;
    }
    OpenCL_Test_Success(err, "clGetDeviceIDs()");
    assert(nb_devices() >= 1);

    // Create the device list
    device_list.resize(nb_devices());
    // Temporary list
    cl_device_id *tmp_devices;

    std::list<OpenCL_device>::iterator it = device_list.begin();

    are_all_devices_in_use = true; // We want to know if all devices are in use.

    // Add CPUs to list
    if (nb_cpu >= 1)
    {
        tmp_devices = new cl_device_id[nb_cpu];
        err = clGetDeviceIDs(platform->Id(), CL_DEVICE_TYPE_CPU, nb_cpu, tmp_devices, NULL);
        OpenCL_Test_Success(err, "clGetDeviceIDs()");

        for (unsigned int i = 0 ; i < nb_cpu ; ++i, ++it)
        {
            it->Set_Information(i, tmp_devices[i], _platform.Id_Offset(), platform->Name().c_str(), false, &_platform); // device_is_gpu == false

            // When one device is not in use... One device is not in use!
            if (!it->Is_In_Use())
                are_all_devices_in_use = false;

        }
        delete[] tmp_devices;
    }

    // Add GPUs to list
    if (nb_gpu >= 1)
    {
        tmp_devices = new cl_device_id[nb_gpu];
        err = clGetDeviceIDs(platform->Id(), CL_DEVICE_TYPE_GPU, nb_gpu, tmp_devices, NULL);
        OpenCL_Test_Success(err, "clGetDeviceIDs()");
        for (unsigned int i = 0 ; i < nb_gpu ; ++i, ++it)
        {
            it->Set_Information(nb_cpu+i, tmp_devices[i], _platform.Id_Offset(), platform->Name().c_str(), true, &_platform); // device_is_gpu == true

            // When one device is not in use... One device is not in use!
            if (!it->Is_In_Use())
                are_all_devices_in_use = false;
        }
        delete[] tmp_devices;
    }

    assert(it == device_list.end());

    // When all devices are in use we abort the program
    if (are_all_devices_in_use == true)
    {
        std_cout << "All devices on platform '" << _platform.Name() << "' are in use!\n" << std::flush;
        abort();
    }

    preferred_device = NULL;    // The preferred device is unknown for now.

    is_initialized = true;
}

// *****************************************************************************
void OpenCL_devices_list::Set_Preferred_OpenCL(const int _preferred_device)
{
    std::list<OpenCL_device>::iterator it = device_list.begin();
    if (_preferred_device == -1)
    {
        // Sort the list. The order is defined by "OpenCL_device::operator<"
        device_list.sort();

        // Initialize context on a device
        for (it = device_list.begin() ; it != device_list.end() ; ++it)
        {
            std_cout << "OpenCL: Trying to set a context on " << it->Get_Name() << " (id = " << it->Get_ID() << ")...";
            if (it->Set_Context() == CL_SUCCESS)
            {
                std_cout << " Success!\n";
                preferred_device = &(*it);

                break;
            }
            else
            {
                std_cout << " Failed. Maybe next one will work?\n";
            }
        }
    }
    else
    {
        if (_preferred_device >= int(device_list.size()))
        {
            std_cout << "OpenCL: ERROR: the device requested is out of range. Exiting.\n";
            abort();
        }

        // Release any allocated context
        for (it = device_list.begin() ; it != device_list.end() ; ++it)
        {
            it->Destructor();
        }

        for (it = device_list.begin() ; it != device_list.end() ; ++it)
        {
            if (_preferred_device == it->Get_ID())
            {
                std_cout << "OpenCL: Found preferred device (" << it->Get_Parent_Platform()->Name() << ", " << it->Get_Name() << ", id = " << it->Get_ID() << "). Trying to set an context on it...\n";
                if (it->Set_Context() == CL_SUCCESS)
                {
                    std_cout << " Success!\n";
                    preferred_device = &(*it);

                    break;
                }
            }
        }
    }

    if (preferred_device == NULL)
    {
        std_cout << "ERROR: Cannot set an OpenCL context on any of the available devices!\nExiting" << std::flush;
        abort();
    }
}

// *****************************************************************************
OpenCL_Kernel::OpenCL_Kernel()
{
    filename        = "";
    context         = NULL;
    device_id       = NULL;
    compiler_options= "";
    kernel_name     = "";
    dimension       = 0;
    p               = 0;
    q               = 0;
    program         = NULL;
    kernel          = NULL;
    global_work_size= NULL;
    local_work_size = NULL;
    err             = 0;
    event           = NULL;
}

// *****************************************************************************
OpenCL_Kernel::OpenCL_Kernel(std::string _filename, const cl_context &_context,
                             const cl_device_id &_device_id)
{
    Initialize(_filename, _context, _device_id);
}

// *****************************************************************************
void OpenCL_Kernel::Initialize(std::string _filename, const cl_context &_context,
                               const cl_device_id &_device_id)
{
    filename        = _filename;
    context         = _context;
    device_id       = _device_id;
    kernel          = NULL;
    program         = NULL;
    compiler_options= "";

    dimension = 2; // Always use two dimensions.

    // Create array once
    global_work_size = new size_t[dimension];
    local_work_size  = new size_t[dimension];
}

// *****************************************************************************
OpenCL_Kernel::~OpenCL_Kernel()
{
    if (kernel)  clReleaseKernel(kernel);
    if (program) clReleaseProgram(program);

    if (global_work_size) delete[] global_work_size;
    if (local_work_size)  delete[] local_work_size;

    kernel           = NULL;
    program          = NULL;
    global_work_size = NULL;
    local_work_size  = NULL;
}

// *****************************************************************************
void OpenCL_Kernel::Build(std::string _kernel_name)
{
    kernel_name      = _kernel_name;

    // **********************************************************
    // Load and build the kernel
    Load_Program_From_File();

    // Create the kernel.
    kernel = clCreateKernel(program, kernel_name.c_str(), &err);
    OpenCL_Test_Success(err, "clCreateKernel");

    // **********************************************************
    // Get the maximum work group size
    //err = clGetKernelWorkGroupInfo(kernel_md, list.Preferred_OpenCL_Device(), CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &maxWorkSize, NULL);
    //std_cout << "Maximum kernel work group size: " << maxWorkSize << "\n";
    //OpenCL_Test_Success(err, "clGetKernelWorkGroupInfo");
}

// *****************************************************************************
void OpenCL_Kernel::Compute_Work_Size(size_t _global_x, size_t _global_y, size_t _local_x, size_t _local_y)
/**
 * @param _global_x: The global work size in dimension x.
 * @param _global_y: The global work size in dimension y.
 * @param _local_x : The local  work size in dimension x.
 * @param _local_y : The local  work size in dimension y.
 */
{
    assert(_global_x >= _local_x);
    assert(_global_y >= _local_y);

    assert(_global_x % _local_x == 0);
    assert(_global_y % _local_y == 0);

    global_work_size[0] = _global_x;
    global_work_size[1] = _global_y;

    local_work_size[0] = _local_x;
    local_work_size[1] = _local_y;

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
void OpenCL_Kernel::Append_Compiler_Option(const std::string option)
{
    compiler_options += option;
    if (option[option.size()-1] != ' ')
        compiler_options += " ";
}

// *****************************************************************************
void OpenCL_Kernel::Launch(const cl_command_queue &command_queue)
{
    err = clEnqueueNDRangeKernel(command_queue, Get_Kernel(), Get_Dimension(), NULL,
                                 Get_Global_Work_Size(), Get_Local_Work_Size(),
                                 0, NULL, NULL);
    OpenCL_Test_Success(err, "clEnqueueNDRangeKernel");
}

// *****************************************************************************
int OpenCL_Kernel::Get_Multiple(int n, int base)
{
    int multipleOfWorkSize = 0;

    if (n < base)
    {
        multipleOfWorkSize = base;
    }
    else if (n % base == 0)
    {
        multipleOfWorkSize = n;
    }
    else
    {
        multipleOfWorkSize = base*(int)std::floor(n/base) + base;
    }

    return multipleOfWorkSize;
}

// *****************************************************************************
void OpenCL_Kernel::Load_Program_From_File()
{
    // Program Setup
    int pl;
    size_t program_length;
    char* cSourceCL;

    // Test of file exists
    std::ifstream input_file(filename.c_str());
    if (input_file.is_open())
    {
        std_cout << "Loading OpenCL program from \"" << filename << "\"...\n";

        // Loads the contents of the file at the given path
        cSourceCL = read_opencl_kernel(filename, &pl);
        program_length = (size_t) pl;

        input_file.close();
    }
    else
    {
        cSourceCL = (char *)filename.c_str();
        program_length = filename.size();
    }

    // create the program
    program = clCreateProgramWithSource(context, 1, (const char **) &cSourceCL, &program_length, &err);
    OpenCL_Test_Success(err, "clCreateProgramWithSource");

    Build_Executable(true);
}

// *****************************************************************************
void OpenCL_Kernel::Build_Executable(const bool verbose)
/**
 * Build the program executable
 */
{
    if (verbose)
    {
        std_cout << "Building the program..." << std::flush;
        std_cout << "\nOpenCL Compiler Options: " << compiler_options << "\n" << std::flush;
    }

    err = clBuildProgram(program, 0, NULL, compiler_options.c_str(), NULL, NULL);

    char *build_log;
    size_t ret_val_size;
    err = clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &ret_val_size);
    build_log = new char[ret_val_size+1];
    err = clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, ret_val_size, build_log, NULL);
    build_log[ret_val_size] = '\0';
    OpenCL_Test_Success(err, "1. clGetProgramBuildInfo");
    if (verbose)
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

    if (verbose)
        std_cout << "done.\n";
}

// *****************************************************************************
std::string OpenCL_Error_to_String(cl_int error)
/**
 * Helper function to get OpenCL error string from constant
 */
{
    const std::string errorString[] = {
        "CL_SUCCESS",
        "CL_DEVICE_NOT_FOUND",
        "CL_DEVICE_NOT_AVAILABLE",
        "CL_COMPILER_NOT_AVAILABLE",
        "CL_MEM_OBJECT_ALLOCATION_FAILURE",
        "CL_OUT_OF_RESOURCES",
        "CL_OUT_OF_HOST_MEMORY",
        "CL_PROFILING_INFO_NOT_AVAILABLE",
        "CL_MEM_COPY_OVERLAP",
        "CL_IMAGE_FORMAT_MISMATCH",
        "CL_IMAGE_FORMAT_NOT_SUPPORTED",
        "CL_BUILD_PROGRAM_FAILURE",
        "CL_MAP_FAILURE",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "CL_INVALID_VALUE",
        "CL_INVALID_DEVICE_TYPE",
        "CL_INVALID_PLATFORM",
        "CL_INVALID_DEVICE",
        "CL_INVALID_CONTEXT",
        "CL_INVALID_QUEUE_PROPERTIES",
        "CL_INVALID_COMMAND_QUEUE",
        "CL_INVALID_HOST_PTR",
        "CL_INVALID_MEM_OBJECT",
        "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR",
        "CL_INVALID_IMAGE_SIZE",
        "CL_INVALID_SAMPLER",
        "CL_INVALID_BINARY",
        "CL_INVALID_BUILD_OPTIONS",
        "CL_INVALID_PROGRAM",
        "CL_INVALID_PROGRAM_EXECUTABLE",
        "CL_INVALID_KERNEL_NAME",
        "CL_INVALID_KERNEL_DEFINITION",
        "CL_INVALID_KERNEL",
        "CL_INVALID_ARG_INDEX",
        "CL_INVALID_ARG_VALUE",
        "CL_INVALID_ARG_SIZE",
        "CL_INVALID_KERNEL_ARGS",
        "CL_INVALID_WORK_DIMENSION",
        "CL_INVALID_WORK_GROUP_SIZE",
        "CL_INVALID_WORK_ITEM_SIZE",
        "CL_INVALID_GLOBAL_OFFSET",
        "CL_INVALID_EVENT_WAIT_LIST",
        "CL_INVALID_EVENT",
        "CL_INVALID_OPERATION",
        "CL_INVALID_GL_OBJECT",
        "CL_INVALID_BUFFER_SIZE",
        "CL_INVALID_MIP_LEVEL",
        "CL_INVALID_GLOBAL_WORK_SIZE",
    };

    const int errorCount = sizeof(errorString) / sizeof(errorString[0]);

    const int index = -error;

    return (index >= 0 && index < errorCount) ? errorString[index] : "Unspecified Error";
}

// *****************************************************************************
template <class T>
OpenCL_Array<T>::OpenCL_Array()
{
    array_is_padded             = false;
    N                           = 0;
    sizeof_element              = 0;
    new_array_size_bytes        = 0;
    host_array                  = NULL;
    nb_1024bits_blocks          = 0;
    device_array                = NULL;
    context                     = NULL;
    command_queue               = NULL;
}

// *****************************************************************************
template <class T>
void OpenCL_Array<T>::Initialize(int _N, const size_t _sizeof_element,
                                 T *&_host_array,
                                 cl_context &_context, cl_mem_flags flags,
                                 std::string _platform,
                                 cl_command_queue &_command_queue,
                                 cl_device_id &_device,
                                 const bool _checksum_array)
{
    assert(_host_array != NULL);

    N               = _N;
    sizeof_element  = _sizeof_element;
    context         = _context;
    command_queue   = _command_queue;
    device          = _device;
    host_array      = _host_array;
    platform        = _platform;
    new_array_size_bytes = N * sizeof_element;

    memset(host_checksum,   0, 64);
    memset(device_checksum, 0, 64);

#ifdef OpenCLSHA512Checksum
    if (_checksum_array)
    {
        array_is_padded = true;

        void * array = (void *) _host_array;
        uint64_t new_array_size_bits = new_array_size_bytes*CHAR_BIT;
        OpenCL_SHA512::Prepare_Array_for_Checksuming(&array, sizeof_element, new_array_size_bits);
        new_array_size_bytes = new_array_size_bits / CHAR_BIT;
        _host_array = (T *)array;
        host_array  = (T *)array;

        std::string kernel_source = reinterpret_cast<const char*>(kernel_SHA512_Checksum);
        kernel_checksum.Initialize(kernel_source, context, device);


        kernel_checksum.Append_Compiler_Option("-DYDEBUG");
        // Include debugging symbols in kernel compilation
#ifndef MACOSX
        if (platform != OPENCL_PLATFORMS_NVIDIA)
        {
            kernel_checksum.Append_Compiler_Option("-g");
        }
#endif // #ifndef MACOSX

        if      (platform == OPENCL_PLATFORMS_AMD)
        {
            kernel_checksum.Append_Compiler_Option("-DOPENCL_AMD");
        }
        else if (platform == OPENCL_PLATFORMS_INTEL)
        {
            kernel_checksum.Append_Compiler_Option("-DOPENCL_INTEL");
        }
        else if (platform == OPENCL_PLATFORMS_NVIDIA)
        {
            kernel_checksum.Append_Compiler_Option("-DOPENCL_NVIDIA");
            // Verbose compilation? Does not do much... And it may break kernel compilation
            // with invalid kernel name error.
            kernel_checksum.Append_Compiler_Option("-cl-nv-verbose");
        }
        else if (platform == OPENCL_PLATFORMS_APPLE)
        {
            kernel_checksum.Append_Compiler_Option("-DOPENCL_APPLE");
        }

        kernel_checksum.Build("SHA512_Checksum");
        kernel_checksum.Compute_Work_Size(1, 1, 1, 1);

        // Allocate memory on device
        device_array      = clCreateBuffer(context, flags,           new_array_size_bytes, NULL, &err); OpenCL_Test_Success(err, "clCreateBuffer()");
        //cl_array_size_bit = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(int),         NULL, &err); OpenCL_Test_Success(err, "clCreateBuffer()");
        cl_sha512sum      = clCreateBuffer(context, CL_MEM_READ_WRITE, buff_size_checksum, NULL, &err); OpenCL_Test_Success(err, "clCreateBuffer()");

        // Set kernel arguments
        err  = clSetKernelArg(kernel_checksum.Get_Kernel(), 0, sizeof(cl_mem), (void *) &device_array);
        //err |= clSetKernelArg(kernel_checksum.Get_Kernel(), 1, sizeof(cl_mem), (void *) &cl_array_size_bit);
        err |= clSetKernelArg(kernel_checksum.Get_Kernel(), 1, sizeof(int),    (void *) &new_array_size_bits);
        err |= clSetKernelArg(kernel_checksum.Get_Kernel(), 2, sizeof(cl_mem), (void *) &cl_sha512sum);
        OpenCL_Test_Success(err, "clSetKernelArg()");

    }
    else
#endif // #ifdef OpenCLSHA512Checksum
    {
        // Allocate memory on the device
        device_array = clCreateBuffer(context, flags, new_array_size_bytes, NULL, &err);
        OpenCL_Test_Success(err, "clCreateBuffer()");
    }

    // Transfer data from host to device (cpu to gpu)
    Host_to_Device();

    if (_checksum_array)
        Validate_Data();

    err = clFinish(command_queue);
    OpenCL_Test_Success(err, "clFinish");
}

// *****************************************************************************
template <class T>
void OpenCL_Array<T>::Set_as_Kernel_Argument(cl_kernel &kernel, const int order)
{
    err = clSetKernelArg(kernel, order, sizeof(cl_mem), &device_array);
    OpenCL_Test_Success(err, "clSetKernelArg()");
}

// *****************************************************************************
template <class T>
void OpenCL_Array<T>::Release_Memory()
{
    if (device_array)
        clReleaseMemObject(device_array);
}

// *****************************************************************************
template <class T>
std::string OpenCL_Array<T>::Host_Checksum()
{
    return OpenCL_SHA512::Checksum_to_String(host_checksum);
}

// *****************************************************************************
template <class T>
std::string OpenCL_Array<T>::Device_Checksum()
{
    return OpenCL_SHA512::Checksum_to_String(device_checksum);
}

// *****************************************************************************
template <class T>
void OpenCL_Array<T>::Validate_Data()
{
#ifdef OpenCLSHA512Checksum
    /*
    std_cout << "Array in binary:\n" << OpenCL_SHA512::String_Binary(host_array, new_array_size_bytes*CHAR_BIT) << "\n";
    std_cout << "Array in hexa:\n"   << OpenCL_SHA512::String_Hexadecimal(host_array, new_array_size_bytes*CHAR_BIT) << "\n";
    */

    // Wait for queue to finish
    err = clFinish(command_queue);
    OpenCL_Test_Success(err, "clFinish()");

    // Calculate checksum of host memory
    OpenCL_SHA512::Calculate_Checksum(host_array, new_array_size_bytes*CHAR_BIT, host_checksum);

    // Calculate checksum of device memory
    kernel_checksum.Launch(command_queue);
    // Wait for kernel to finish
    err = clFinish(command_queue);
    OpenCL_Test_Success(err, "clFinish()");

    // Transfer back checksum
    err = clEnqueueReadBuffer(command_queue, cl_sha512sum, CL_FALSE, 0, buff_size_checksum, device_checksum, 0, NULL, NULL);
    OpenCL_Test_Success(err, "clEnqueueReadBuffer");
    err = clFinish(command_queue);
    OpenCL_Test_Success(err, "clFinish()");

    /*
    std_cout << "Host_Checksum()   = " << Host_Checksum() << "\n";
    std_cout << "Device_Checksum() = " << Device_Checksum() << "\n";
    */

    if (Host_Checksum() != Device_Checksum())
    {
        std_cout << "ERROR: Checksums don't match!\n";
        std_cout << "Host_Checksum()   = " << Host_Checksum() << "\n";
        std_cout << "Device_Checksum() = " << Device_Checksum() << "\n";
        std_cout << "Array in hexa:\n"   << OpenCL_SHA512::String_Hexadecimal(host_array, new_array_size_bytes*CHAR_BIT) << "\n";
    }
//     else
//     {
//         std_cout << "Checksums do match.\n";
//         std_cout << "Host_Checksum()   = " << Host_Checksum() << "\n";
//         std_cout << "Device_Checksum() = " << Device_Checksum() << "\n";
//         std_cout << "Array in hexa:\n"   << OpenCL_SHA512::String_Hexadecimal(host_array, new_array_size_bytes*CHAR_BIT) << "\n";
//     }
    assert(Host_Checksum() == Device_Checksum());

#endif // #ifdef OpenCLSHA512Checksum
}

// *****************************************************************************
template <class T>
void OpenCL_Array<T>::Host_to_Device()
{
    err = clEnqueueWriteBuffer(command_queue,       // Command queue
                               device_array,        // Memory buffer to write to
                               CL_TRUE,             // Non-Blocking read
                               0,                   // Offset in the buffer object to read from
                               new_array_size_bytes,// Size in bytes of data being read
                               host_array,          // Pointer to buffer on device to store write data
                               0,                   // Number of event in the event list
                               NULL,                // List of events that needs to complete before this executes
                               NULL);               // Event object to return on completion
    OpenCL_Test_Success(err, "clEnqueueWriteBuffer()");
}

// *****************************************************************************
template <class T>
void OpenCL_Array<T>::Device_to_Host()
{
    assert(device_array != NULL);
    err = clEnqueueReadBuffer(command_queue,        // Command queue
                              device_array,         // Memory buffer to read from
                              CL_FALSE,             // Non-Blocking read
                              0,                    // Offset in the buffer object to read from
                              new_array_size_bytes, // Size in bytes of data being read
                              host_array,           // Pointer to buffer in RAM to store read data
                              0,                    // Number of event in the event list
                              NULL,                 // List of events that needs to complete before this executes
                              NULL);                // Event object to return on completion
    OpenCL_Test_Success(err, "clEnqueueReadBuffer()");
}

// *****************************************************************************
namespace OpenCL_SHA512
{
    // *************************************************************************
    void Prepare_Array_for_Checksuming(void **_array, const uint64_t sizeof_element,
                                       uint64_t &array_size_bit)
    /**
     * Prepare array to be SHA512 checksumed.
     * Will re-allocate memory for the array to next multiple of 1024 and add the
     * necessary padding required by SHA512.
     */
    {
        // Calculate how much padding is necessary for the SHA512 checksum
        // See http://www.iwar.org.uk/comsec/resources/cipher/sha256-384-512.pdf
        // | initial array | |                                  |                                   |
        //                  ^ 1 bit      ^ padding bits (0)      ^ 128 bits (initial array length)

        const uint64_t padding_bits         = (896 - (array_size_bit + 1)) % 1024;
        const uint64_t new_array_size_bit   = array_size_bit + 1 + padding_bits + 128;
        //const uint64_t new_array_size_bytes = new_array_size_bit / CHAR_BIT;

        char *carray = *((char **)_array);
        void * array = (void *) carray;

        assert(new_array_size_bit % 1024 == 0);

        // Allocate new array with padded 0s
        const uint64_t N = array_size_bit / (sizeof_element * CHAR_BIT);
        const uint64_t new_array_N = new_array_size_bit / 32; // 32-bit integers
        uint32_t *new_array        = (uint32_t *) calloc_and_check(new_array_N, sizeof(uint32_t));

        /*
        std_cout << "Initial array: " << carray << "\n";
        std_cout << "Initial array (binary):\n" << String_Binary(carray, array_size_bit) << "\n";
        std_cout << "Initial array (hexadecimal):\n" << String_Hexadecimal(carray, array_size_bit) << "\n";
        const uint64_t new_array_size       = new_array_size_bytes / sizeof_element;
        std_cout << "N                  = " << N << "\n";
        std_cout << "sizeof_element     = " << sizeof_element << "\n";
        std_cout << "array_size_bytes   = " << array_size_bit / CHAR_BIT << "\n";
        std_cout << "array_size_bit     = " << array_size_bit << "\n";
        std_cout << "padding_bits       = " << padding_bits << "\n";
        std_cout << "new_array_size_bit = " << new_array_size_bit << "\n";
        std_cout << "new_array_size_bytes = " << new_array_size_bytes << "\n";
        std_cout << "new_array_size     = " << new_array_size << "\n";
        std_cout << "sizeof(uint32_t)   = " << sizeof(uint32_t) << "\n";
        std_cout << "new_array_size_bit/32= " << new_array_size_bit/32 << "\n";
        */

        // Copy the original array to the new array
        memcpy(new_array, array, array_size_bit/CHAR_BIT);

        // Set next bit to 1 to start padding
        // To do so, set the most significant bit of the array's next element to 1.
        if      (sizeof_element*CHAR_BIT == 8)
            ((uint8_t *)new_array)[N] = 0x80; // 8 bits (char)
        else if (sizeof_element*CHAR_BIT == 16)
            ((uint16_t *)new_array)[N] = 0x8000; // 16 bits
        else if (sizeof_element*CHAR_BIT == 32)
            ((uint32_t *)new_array)[N] = 0x80000000; // 32 bits (floats)
        else if (sizeof_element*CHAR_BIT == 64)
            ((uint64_t *)new_array)[N] = 0x8000000000000000; // 64 bits (double)
        else
        {
            std_cout << "ERROR: sizeof(array) == " << sizeof_element << " unsupported! Aborting.\n" << std::flush;
            abort();
        }

        // Because calloc_and_check() is used to allocate the new array, it is filled with 0.
        // There is no need thus to set the padding to 0.

        // Now set the last 128 bits of the array as being a (big-endian) 128-bits integer
        // representing the original array size.
        // NOTE: There is no "uint128_t" type, so we ignore the 64 most significant bit
        //       of this value.
        // WARNING: According to Section 2 of the standard (https://tools.ietf.org/html/rfc4634#section-2),
        //          big-endian representation is used. This means that "the most significant bit
        //          is shown in the left-most bit position". So the deciman "24" is represented as:
        //          "00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
        //           00000000 00000000 00000000 00000000 00000000 00000000 00000000 00011000"
        //          and not:
        //          "00011000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
        //           00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000"

        // Change endianess
        // http://www.codeguru.com/forum/showthread.php?t=292902
        const uint64_t bigendian_array_size_bit =
                                 (array_size_bit >> 56)                       |
                                ((array_size_bit << 40) & 0x00FF000000000000) |
                                ((array_size_bit << 24) & 0x0000FF0000000000) |
                                ((array_size_bit <<  8) & 0x000000FF00000000) |
                                ((array_size_bit >>  8) & 0x00000000FF000000) |
                                ((array_size_bit >> 24) & 0x0000000000FF0000) |
                                ((array_size_bit >> 40) & 0x000000000000FF00) |
                                 (array_size_bit << 56);
        assert(new_array_N % 2 == 0);
        ((uint64_t *)new_array)[new_array_N/2-1] = bigendian_array_size_bit;

        /*
        std_cout << "Padded array (binary):\n" << String_Binary(new_array, new_array_size_bit) << "\n";
        std_cout << "Padded array (hexadecimal):\n" << String_Hexadecimal(new_array, new_array_size_bit) << "\n";
        */

        // Free the old array
        free(carray);

        *_array = new_array;
        array_size_bit = new_array_size_bit;
    }

    // *************************************************************************
    void Calculate_Checksum(const void *_array, uint64_t size_bits, uint8_t *sha512sum)
    {
        const uint8_t *array = (uint8_t *) _array;

        uint64_t N = size_bits / 1024;
        uint64_t H[8];

        H[0] = 0x6A09E667F3BCC908ll;
        H[1] = 0xBB67AE8584CAA73Bll;
        H[2] = 0x3C6EF372FE94F82Bll;
        H[3] = 0xA54FF53A5F1D36F1ll;
        H[4] = 0x510E527FADE682D1ll;
        H[5] = 0x9B05688C2B3E6C1Fll;
        H[6] = 0x1F83D9ABFB41BD6Bll;
        H[7] = 0x5BE0CD19137E2179ll;

        static const uint64_t K[80] =
        {
            0x428A2F98D728AE22ll, 0x7137449123EF65CDll, 0xB5C0FBCFEC4D3B2Fll,
            0xE9B5DBA58189DBBCll, 0x3956C25BF348B538ll, 0x59F111F1B605D019ll,
            0x923F82A4AF194F9Bll, 0xAB1C5ED5DA6D8118ll, 0xD807AA98A3030242ll,
            0x12835B0145706FBEll, 0x243185BE4EE4B28Cll, 0x550C7DC3D5FFB4E2ll,
            0x72BE5D74F27B896Fll, 0x80DEB1FE3B1696B1ll, 0x9BDC06A725C71235ll,
            0xC19BF174CF692694ll, 0xE49B69C19EF14AD2ll, 0xEFBE4786384F25E3ll,
            0x0FC19DC68B8CD5B5ll, 0x240CA1CC77AC9C65ll, 0x2DE92C6F592B0275ll,
            0x4A7484AA6EA6E483ll, 0x5CB0A9DCBD41FBD4ll, 0x76F988DA831153B5ll,
            0x983E5152EE66DFABll, 0xA831C66D2DB43210ll, 0xB00327C898FB213Fll,
            0xBF597FC7BEEF0EE4ll, 0xC6E00BF33DA88FC2ll, 0xD5A79147930AA725ll,
            0x06CA6351E003826Fll, 0x142929670A0E6E70ll, 0x27B70A8546D22FFCll,
            0x2E1B21385C26C926ll, 0x4D2C6DFC5AC42AEDll, 0x53380D139D95B3DFll,
            0x650A73548BAF63DEll, 0x766A0ABB3C77B2A8ll, 0x81C2C92E47EDAEE6ll,
            0x92722C851482353Bll, 0xA2BFE8A14CF10364ll, 0xA81A664BBC423001ll,
            0xC24B8B70D0F89791ll, 0xC76C51A30654BE30ll, 0xD192E819D6EF5218ll,
            0xD69906245565A910ll, 0xF40E35855771202All, 0x106AA07032BBD1B8ll,
            0x19A4C116B8D2D0C8ll, 0x1E376C085141AB53ll, 0x2748774CDF8EEB99ll,
            0x34B0BCB5E19B48A8ll, 0x391C0CB3C5C95A63ll, 0x4ED8AA4AE3418ACBll,
            0x5B9CCA4F7763E373ll, 0x682E6FF3D6B2B8A3ll, 0x748F82EE5DEFB2FCll,
            0x78A5636F43172F60ll, 0x84C87814A1F0AB72ll, 0x8CC702081A6439ECll,
            0x90BEFFFA23631E28ll, 0xA4506CEBDE82BDE9ll, 0xBEF9A3F7B2C67915ll,
            0xC67178F2E372532Bll, 0xCA273ECEEA26619Cll, 0xD186B8C721C0C207ll,
            0xEADA7DD6CDE0EB1Ell, 0xF57D4F7FEE6ED178ll, 0x06F067AA72176FBAll,
            0x0A637DC5A2C898A6ll, 0x113F9804BEF90DAEll, 0x1B710B35131C471Bll,
            0x28DB77F523047D84ll, 0x32CAAB7B40C72493ll, 0x3C9EBE0A15C9BEBCll,
            0x431D67C49C100D4Cll, 0x4CC5D4BECB3E42B6ll, 0x597F299CFC657E2All,
            0x5FCB6FAB3AD6FAECll, 0x6C44198C4A475817ll
        };

        int t8 = 0;

        for (uint64_t i = 0 ; i < N ; i++)
        {
            uint64_t W[80]; // Word sequence.

            for (int t = 0 ; t < 16 ; t++, t8 += 8)
                W[t] = ((uint64_t)(array[t8  ]) << 56) |
                    ((uint64_t)(array[t8 + 1]) << 48) |
                    ((uint64_t)(array[t8 + 2]) << 40) |
                    ((uint64_t)(array[t8 + 3]) << 32) |
                    ((uint64_t)(array[t8 + 4]) << 24) |
                    ((uint64_t)(array[t8 + 5]) << 16) |
                    ((uint64_t)(array[t8 + 6]) << 8) |
                    ((uint64_t)(array[t8 + 7]));

            for (int t = 16 ; t < 80 ; t++)
                W[t] = SHA512_sigma1(W[t-2]) + W[t-7] + SHA512_sigma0(W[t-15]) + W[t-16];

            uint64_t a = H[0];
            uint64_t b = H[1];
            uint64_t c = H[2];
            uint64_t d = H[3];
            uint64_t e = H[4];
            uint64_t f = H[5];
            uint64_t g = H[6];
            uint64_t h = H[7];

            for(int t = 0 ; t < 80 ; t++)
            {
                uint64_t T1 = h + SHA512_SIGMA1(e) + SHA_Ch(e,f,g) + K[t] + W[t];
                uint64_t T2 = SHA512_SIGMA0(a) + SHA_Maj(a,b,c);

                h = g;
                g = f;
                f = e;
                e = d + T1;
                d = c;
                c = b;
                b = a;
                a = T1 + T2;
            }

            H[0] += a;
            H[1] += b;
            H[2] += c;
            H[3] += d;
            H[4] += e;
            H[5] += f;
            H[6] += g;
            H[7] += h;
        }

        uint8_t message_digest[64];

        for (int i = 0 ; i < 64 ; ++i)
            message_digest[i] = (uint8_t)(H[i>>3] >> 8 * ( 7 - ( i % 8 ) ));

        for(int i = 0 ; i < 64 ; i++)
        {
            sha512sum[i] = message_digest[i];
        }
    }

    // *************************************************************************
    void Print_Checksum(const uint8_t checksum[64])
    {
        for(int i = 0 ; i < 64 ; i++)
            printf("%02x", checksum[i] & 0xff);
        printf("\n");
    }

    // *************************************************************************
    std::string Checksum_to_String(const uint8_t checksum[64])
    {
        std::string string_checksum("");
        char two_char[3];
        memset(two_char, 0, 3*sizeof(char));
        for(int i = 0 ; i < 64 ; i++)
        {
            sprintf(two_char, "%02x", checksum[i] & 0xff);
            string_checksum += two_char;
        }

        return string_checksum;
    }

    // *************************************************************************
    std::string String_Hexadecimal(const void *array, uint64_t size_bits)
    {
        // 1 hexa == 4 bits
        const uint64_t hexa_to_bit = 4;
        assert(size_bits % CHAR_BIT == 0);
        std::string array_in_hexa("");
        //assert(size_bits % (CHAR_BIT * 2) == 0);
        uint8_t *array_as_int = (uint8_t *) array;
        char tmp[10];
        // When printed, a single hexadecimal value shows up as two characters.
        for (uint64_t i = 0 ; i < size_bits / (2*hexa_to_bit) ; i++)
        {
            sprintf(tmp, "%02x", array_as_int[i]);
            array_in_hexa += tmp;
            if ((i+1) % 4 == 0)
                array_in_hexa += " ";
            if ((i+1) % (4*8) == 0)
                array_in_hexa += "\n";
        }

        return array_in_hexa;
    }

    // *************************************************************************
    std::string String_Binary(const void *array, uint64_t size_bits)
    {
        // Print in chunk of 8 bits
        assert(size_bits % 8 == 0);

        std::string array_in_binary("");
        const uint8_t *const array_8bit = (uint8_t *) array;
        for (uint64_t i = 0 ; i < size_bits / 8 ; i++)
        {
            array_in_binary += OclUtils::Integer_in_String_Binary(array_8bit[i]);
            array_in_binary += " ";
            if ((i+1) % 8 == 0)
                array_in_binary += "\n";
        }

        return array_in_binary;
    }

    // *************************************************************************
    void Validation()
    {
        const int l = 1000;
        char *char_array;
        std::string precalculated_checksum, calculated_checksum;
        uint64_t array_size_bit;
        uint8_t checksum[64];

        // *********************************************************************
        // Examples from http://www.iwar.org.uk/comsec/resources/cipher/sha256-384-512.pdf
        char_array = (char *) calloc_and_check(l, sizeof(char));
        sprintf(char_array, "%s", "abc");
        array_size_bit = strlen(char_array)*CHAR_BIT;
        //std_cout << "Validation() String: " << char_array << "\n";
        Prepare_Array_for_Checksuming((void **)&char_array, sizeof(char), array_size_bit);
        precalculated_checksum = "ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f";
        Calculate_Checksum(char_array, array_size_bit, checksum);
        //std_cout << "Validation() Pre-calculate checksum: " << precalculated_checksum << "\n";
        //std_cout << "Validation() Calculate checksum:     " << Checksum_to_String(checksum) << "\n";
        assert(precalculated_checksum == Checksum_to_String(checksum));
        OclUtils::free_me(char_array);

        // *********************************************************************
        // Examples from Wikipedia
        // https://secure.wikimedia.org/wikipedia/en/wiki/Sha512#Examples_of_SHA-2_variants_.28SHA224.2C_SHA256.2C_SHA384_and_SHA512.29
        char_array = (char *) calloc_and_check(l, sizeof(char));
        sprintf(char_array, "%s", "The quick brown fox jumps over the lazy dog");
        //std_cout << "Validation() String: " << char_array << "\n";
        array_size_bit = strlen(char_array)*CHAR_BIT;
        Prepare_Array_for_Checksuming((void **)&char_array, sizeof(char), array_size_bit);
        precalculated_checksum = "07e547d9586f6a73f73fbac0435ed76951218fb7d0c8d788a309d785436bbb642e93a252a954f23912547d1e8a3b5ed6e1bfd7097821233fa0538f3db854fee6";
        Calculate_Checksum(char_array, array_size_bit, checksum);
        //std_cout << "Pre-calculate checksum: " << precalculated_checksum << "\n";
        //std_cout << "Calculate checksum:     " << Checksum_to_String(checksum) << "\n";
        assert(precalculated_checksum == Checksum_to_String(checksum));
        OclUtils::free_me(char_array);

        char_array = (char *) calloc_and_check(l, sizeof(char));
        sprintf(char_array, "%s", "The quick brown fox jumps over the lazy dog.");
        //std_cout << "Validation() String: " << char_array << "\n";
        array_size_bit = strlen(char_array)*CHAR_BIT;
        Prepare_Array_for_Checksuming((void **)&char_array, sizeof(char), array_size_bit);
        precalculated_checksum = "91ea1245f20d46ae9a037a989f54f1f790f0a47607eeb8a14d12890cea77a1bbc6c7ed9cf205e67b7f2b8fd4c7dfd3a7a8617e45f3c463d481c7e586c39ac1ed";
        Calculate_Checksum(char_array, array_size_bit, checksum);
        //std_cout << "Pre-calculate checksum: " << precalculated_checksum << "\n";
        //std_cout << "Calculate checksum:     " << Checksum_to_String(checksum) << "\n";
        assert(precalculated_checksum == Checksum_to_String(checksum));
        OclUtils::free_me(char_array);

        // *********************************************************************
        // Examples from http://csrc.nist.gov/publications/fips/fips180-2/fips180-2.pdf
        // Example C.2
        char_array = (char *) calloc_and_check(l, sizeof(char));
        sprintf(char_array, "%s", "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu");
        //std_cout << "Validation() String: " << char_array << "\n";
        array_size_bit = strlen(char_array)*CHAR_BIT;
        Prepare_Array_for_Checksuming((void **)&char_array, sizeof(char), array_size_bit);
        precalculated_checksum = "8e959b75dae313da8cf4f72814fc143f8f7779c6eb9f7fa17299aeadb6889018501d289e4900f7e4331b99dec4b5433ac7d329eeb6dd26545e96e55b874be909";
        Calculate_Checksum(char_array, array_size_bit, checksum);
        //std_cout << "Pre-calculate checksum: " << precalculated_checksum << "\n";
        //std_cout << "Calculate checksum:     " << Checksum_to_String(checksum) << "\n";
        assert(precalculated_checksum == Checksum_to_String(checksum));
        OclUtils::free_me(char_array);

        // Example C.3
        char_array = (char *) calloc_and_check(1000100, sizeof(char));
        //sprintf(char_array, "%s", "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu");
        for (int i = 0 ; i < 1000000 ; i++)
            char_array[i] = 'a';
        //std_cout << "Validation() String: " << char_array << "\n";
        array_size_bit = strlen(char_array)*CHAR_BIT;
        Prepare_Array_for_Checksuming((void **)&char_array, sizeof(char), array_size_bit);
        precalculated_checksum = "e718483d0ce769644e2e42c7bc15b4638e1f98b13b2044285632a803afa973ebde0ff244877ea60a4cb0432ce577c31beb009c5c2c49aa2e4eadb217ad8cc09b";
        Calculate_Checksum(char_array, array_size_bit, checksum);
        //std_cout << "Pre-calculate checksum: " << precalculated_checksum << "\n";
        //std_cout << "Calculate checksum:     " << Checksum_to_String(checksum) << "\n";
        assert(precalculated_checksum == Checksum_to_String(checksum));
        OclUtils::free_me(char_array);
    }
}

template class OpenCL_Array<float>;
template class OpenCL_Array<double>;
template class OpenCL_Array<int>;
template class OpenCL_Array<char>;


// ********** End of file ******************************************************
