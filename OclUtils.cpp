
#include <stdio.h>
#include <stdlib.h> // abort()
#include <string>   // std::string
#include <string.h> // strstr

#include <StdCout.hpp>

#include "OclUtils.hpp"

// **************************************************************
char *read_opencl_kernel(const std::string filename, int *length)
{
    FILE *f = fopen(filename.c_str(), "r");
    void *buffer;

    if (!f) {
        std_cout << "Unable to open " << filename << " for reading\n";
        abort();
    }

    fseek(f, 0, SEEK_END);
    *length = ftell(f);
    fseek(f, 0, SEEK_SET);

    buffer = malloc(*length+1);
    *length = fread(buffer, 1, *length, f);
    fclose(f);
    ((char*)buffer)[*length] = '\0';

    return (char*)buffer;
}

/**
 * Comes from oclUtils.{cpp,h} and shrUtils.{cpp,h} from Nvidia's CUDA SDK
 */

//////////////////////////////////////////////////////////////////////////////
//! Gets the platform ID for NVIDIA if available, otherwise default
//!
//! @return the id
//! @param clSelectedPlatformID         OpenCL platoform ID
//////////////////////////////////////////////////////////////////////////////
cl_int oclGetPlatformID(cl_platform_id* clSelectedPlatformID)
{
    char chBuffer[1024];
    cl_uint num_platforms;
    cl_platform_id* clPlatformIDs;
    cl_int ciErrNum;
    *clSelectedPlatformID = NULL;

    // Get OpenCL platform count
    ciErrNum = clGetPlatformIDs (0, NULL, &num_platforms);
    if (ciErrNum != CL_SUCCESS)
    {
        std_cout << " Error " << ciErrNum << " in clGetPlatformIDs Call !!!\n";
        return -1000;
    }
    else
    {
        if(num_platforms == 0)
        {
            std_cout << "No OpenCL platform found!\n";
            return -2000;
        }
        else
        {
            // if there's a platform or more, make space for ID's
            if ((clPlatformIDs = (cl_platform_id*)malloc(num_platforms * sizeof(cl_platform_id))) == NULL)
            {
                std_cout << "Failed to allocate memory for cl_platform ID's!\n";
                return -3000;
            }

            // get platform info for each platform and trap the NVIDIA platform if found
            ciErrNum = clGetPlatformIDs (num_platforms, clPlatformIDs, NULL);
            for(cl_uint i = 0; i < num_platforms; ++i)
            {
                ciErrNum = clGetPlatformInfo (clPlatformIDs[i], CL_PLATFORM_NAME, 1024, &chBuffer, NULL);
                if(ciErrNum == CL_SUCCESS)
                {
                    if(strstr(chBuffer, "NVIDIA") != NULL)
                    {
                        *clSelectedPlatformID = clPlatformIDs[i];
                        break;
                    }
                }
            }

            // default to zeroeth platform if NVIDIA not found
            if(*clSelectedPlatformID == NULL)
            {
                std_cout << "WARNING: NVIDIA OpenCL platform not found - defaulting to first platform!\n";
                *clSelectedPlatformID = clPlatformIDs[0];
            }

            free(clPlatformIDs);
        }
    }

    return CL_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//! Print the device name
//!
//! @param iLogMode       enum LOGBOTH, LOGCONSOLE, LOGFILE
//! @param device         OpenCL id of the device
//////////////////////////////////////////////////////////////////////////////
void oclPrintDevName(cl_device_id device)
{
    char device_string[1024];
    clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_string), &device_string, NULL);
    std_cout << "Device name: " << device_string << "\n";
}

//////////////////////////////////////////////////////////////////////////////
//! Print info about the device
//!
//! @param device         OpenCL id of the device
//////////////////////////////////////////////////////////////////////////////
void oclPrintDevInfo(cl_device_id device)
{
    std_cout << "Device information:\n";
    char device_string[1024];
    bool nv_device_attibute_query = false;

    // CL_DEVICE_NAME
    clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_string), &device_string, NULL);
    std_cout << "  CL_DEVICE_NAME: \t\t\t" << device_string << "\n";

    // CL_DEVICE_VENDOR
    clGetDeviceInfo(device, CL_DEVICE_VENDOR, sizeof(device_string), &device_string, NULL);
    std_cout << "  CL_DEVICE_VENDOR: \t\t\t" << device_string << "\n";

    // CL_DRIVER_VERSION
    clGetDeviceInfo(device, CL_DRIVER_VERSION, sizeof(device_string), &device_string, NULL);
    std_cout << "  CL_DRIVER_VERSION: \t\t\t"  << device_string << "\n";

    // CL_DEVICE_VERSION
    clGetDeviceInfo(device, CL_DEVICE_VERSION, sizeof(device_string), &device_string, NULL);
    std_cout << "  CL_DEVICE_VERSION: \t\t\t" << device_string << "\n";

    // CL_DEVICE_OPENCL_C_VERSION (if CL_DEVICE_VERSION version > 1.0)
    if(strncmp("OpenCL 1.0", device_string, 10) != 0)
    {
        // This code is unused for devices reporting OpenCL 1.0, but a def is needed anyway to allow compilation using v 1.0 headers
        // This constant isn't #defined in 1.0
        #ifndef CL_DEVICE_OPENCL_C_VERSION
            #define CL_DEVICE_OPENCL_C_VERSION 0x103D
        #endif

        clGetDeviceInfo(device, CL_DEVICE_OPENCL_C_VERSION, sizeof(device_string), &device_string, NULL);
        std_cout << "  CL_DEVICE_OPENCL_C_VERSION: \t\t" << device_string << "\n";
    }

    // CL_DEVICE_TYPE
    cl_device_type type;
    clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(type), &type, NULL);
    if( type & CL_DEVICE_TYPE_CPU )
        std_cout << "  CL_DEVICE_TYPE:\t\t\tCL_DEVICE_TYPE_CPU\n";
    if( type & CL_DEVICE_TYPE_GPU )
        std_cout << "  CL_DEVICE_TYPE:\t\t\tCL_DEVICE_TYPE_GPU\n";
    if( type & CL_DEVICE_TYPE_ACCELERATOR )
        std_cout << "  CL_DEVICE_TYPE:\t\t\tCL_DEVICE_TYPE_ACCELERATOR\n";
    if( type & CL_DEVICE_TYPE_DEFAULT )
        std_cout << "  CL_DEVICE_TYPE:\t\t\tCL_DEVICE_TYPE_DEFAULT\n";

    // CL_DEVICE_MAX_COMPUTE_UNITS
    cl_uint compute_units;
    clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(compute_units), &compute_units, NULL);
    std_cout << "  CL_DEVICE_MAX_COMPUTE_UNITS:\t\t" << compute_units << "\n";

    // CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS
    size_t workitem_dims;
    clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(workitem_dims), &workitem_dims, NULL);
    std_cout << "  CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS:\t" << workitem_dims << "\n";

    // CL_DEVICE_MAX_WORK_ITEM_SIZES
    size_t workitem_size[3];
    clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(workitem_size), &workitem_size, NULL);
    std_cout << "  CL_DEVICE_MAX_WORK_ITEM_SIZES:\t" << workitem_size[0] << " / " << workitem_size[1] << " / " << workitem_size[2] << "\n";

    // CL_DEVICE_MAX_WORK_GROUP_SIZE
    size_t workgroup_size;
    clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(workgroup_size), &workgroup_size, NULL);
    std_cout << "  CL_DEVICE_MAX_WORK_GROUP_SIZE:\t" << workgroup_size << "\n";

    // CL_DEVICE_MAX_CLOCK_FREQUENCY
    cl_uint clock_frequency;
    clGetDeviceInfo(device, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(clock_frequency), &clock_frequency, NULL);
    std_cout << "  CL_DEVICE_MAX_CLOCK_FREQUENCY:\t" << clock_frequency << " MHz\n";

    // CL_DEVICE_ADDRESS_BITS
    cl_uint addr_bits;
    clGetDeviceInfo(device, CL_DEVICE_ADDRESS_BITS, sizeof(addr_bits), &addr_bits, NULL);
    std_cout << "  CL_DEVICE_ADDRESS_BITS:\t\t" << addr_bits << "\n";

    // CL_DEVICE_MAX_MEM_ALLOC_SIZE
    cl_ulong max_mem_alloc_size;
    clGetDeviceInfo(device, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(max_mem_alloc_size), &max_mem_alloc_size, NULL);
    std_cout << "  CL_DEVICE_MAX_MEM_ALLOC_SIZE:\t\t" << (unsigned int)(max_mem_alloc_size / (1024 * 1024)) << " MBytes\n";

    // CL_DEVICE_GLOBAL_MEM_SIZE
    cl_ulong mem_size;
    clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(mem_size), &mem_size, NULL);
    std_cout << "  CL_DEVICE_GLOBAL_MEM_SIZE:\t\t" << (unsigned int)(mem_size / (1024 * 1024)) << " MBytes\n";

    // CL_DEVICE_ERROR_CORRECTION_SUPPORT
    cl_bool error_correction_support;
    clGetDeviceInfo(device, CL_DEVICE_ERROR_CORRECTION_SUPPORT, sizeof(error_correction_support), &error_correction_support, NULL);
    std_cout << "  CL_DEVICE_ERROR_CORRECTION_SUPPORT:\t" << (error_correction_support == CL_TRUE ? "yes" : "no") << "\n";

    // CL_DEVICE_LOCAL_MEM_TYPE
    cl_device_local_mem_type local_mem_type;
    clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_TYPE, sizeof(local_mem_type), &local_mem_type, NULL);
    std_cout << "  CL_DEVICE_LOCAL_MEM_TYPE:\t\t" << (local_mem_type == 1 ? "local" : "global") << "\n";

    // CL_DEVICE_LOCAL_MEM_SIZE
    clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(mem_size), &mem_size, NULL);
    std_cout << "  CL_DEVICE_LOCAL_MEM_SIZE:\t\t" << (unsigned int)(mem_size / 1024) << " KByte\n";

    // CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE
    clGetDeviceInfo(device, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof(mem_size), &mem_size, NULL);
    std_cout << "  CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE:\t" << (unsigned int)(mem_size / 1024) << " KByte\n";

    // CL_DEVICE_QUEUE_PROPERTIES
    cl_command_queue_properties queue_properties;
    clGetDeviceInfo(device, CL_DEVICE_QUEUE_PROPERTIES, sizeof(queue_properties), &queue_properties, NULL);
    if( queue_properties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE )
        std_cout << "  CL_DEVICE_QUEUE_PROPERTIES:\t\tCL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE\n";
    if( queue_properties & CL_QUEUE_PROFILING_ENABLE )
        std_cout << "  CL_DEVICE_QUEUE_PROPERTIES:\t\tCL_QUEUE_PROFILING_ENABLE\n";

    // CL_DEVICE_IMAGE_SUPPORT
    cl_bool image_support;
    clGetDeviceInfo(device, CL_DEVICE_IMAGE_SUPPORT, sizeof(image_support), &image_support, NULL);
    std_cout << "  CL_DEVICE_IMAGE_SUPPORT:\t\t" << image_support << "\n";

    // CL_DEVICE_MAX_READ_IMAGE_ARGS
    cl_uint max_read_image_args;
    clGetDeviceInfo(device, CL_DEVICE_MAX_READ_IMAGE_ARGS, sizeof(max_read_image_args), &max_read_image_args, NULL);
    std_cout << "  CL_DEVICE_MAX_READ_IMAGE_ARGS:\t" << max_read_image_args << "\n";

    // CL_DEVICE_MAX_WRITE_IMAGE_ARGS
    cl_uint max_write_image_args;
    clGetDeviceInfo(device, CL_DEVICE_MAX_WRITE_IMAGE_ARGS, sizeof(max_write_image_args), &max_write_image_args, NULL);
    std_cout << "  CL_DEVICE_MAX_WRITE_IMAGE_ARGS:\t" << max_write_image_args << "\n";

    // CL_DEVICE_SINGLE_FP_CONFIG
    cl_device_fp_config fp_config;
    clGetDeviceInfo(device, CL_DEVICE_SINGLE_FP_CONFIG, sizeof(cl_device_fp_config), &fp_config, NULL);
    std_cout << "  CL_DEVICE_SINGLE_FP_CONFIG:\t\t" <<
        (fp_config & CL_FP_DENORM ? "denorms " : "") <<
        (fp_config & CL_FP_INF_NAN ? "INF-quietNaNs " : "") <<
        (fp_config & CL_FP_ROUND_TO_NEAREST ? "round-to-nearest " : "") <<
        (fp_config & CL_FP_ROUND_TO_ZERO ? "round-to-zero " : "") <<
        (fp_config & CL_FP_ROUND_TO_INF ? "round-to-inf " : "") <<
        (fp_config & CL_FP_FMA ? "fma " : "") << "\n";

    // CL_DEVICE_IMAGE2D_MAX_WIDTH, CL_DEVICE_IMAGE2D_MAX_HEIGHT, CL_DEVICE_IMAGE3D_MAX_WIDTH, CL_DEVICE_IMAGE3D_MAX_HEIGHT, CL_DEVICE_IMAGE3D_MAX_DEPTH
    size_t szMaxDims[5];
    std_cout << "  CL_DEVICE_IMAGE <dim>";
    clGetDeviceInfo(device, CL_DEVICE_IMAGE2D_MAX_WIDTH, sizeof(size_t), &szMaxDims[0], NULL);
    std_cout << "\t\t\t2D_MAX_WIDTH\t " << szMaxDims[0] << "\n";
    clGetDeviceInfo(device, CL_DEVICE_IMAGE2D_MAX_HEIGHT, sizeof(size_t), &szMaxDims[1], NULL);
    std_cout << "\t\t\t\t\t2D_MAX_HEIGHT\t " << szMaxDims[1] << "\n";
    clGetDeviceInfo(device, CL_DEVICE_IMAGE3D_MAX_WIDTH, sizeof(size_t), &szMaxDims[2], NULL);
    std_cout << "\t\t\t\t\t3D_MAX_WIDTH\t " << szMaxDims[2] << "\n";
    clGetDeviceInfo(device, CL_DEVICE_IMAGE3D_MAX_HEIGHT, sizeof(size_t), &szMaxDims[3], NULL);
    std_cout << "\t\t\t\t\t3D_MAX_HEIGHT\t " << szMaxDims[3] << "\n";
    clGetDeviceInfo(device, CL_DEVICE_IMAGE3D_MAX_DEPTH, sizeof(size_t), &szMaxDims[4], NULL);
    std_cout << "\t\t\t\t\t3D_MAX_DEPTH\t " << szMaxDims[4] << "\n";

    // CL_DEVICE_EXTENSIONS: get device extensions, and if any then parse & log the string onto separate lines
    clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, sizeof(device_string), &device_string, NULL);
    if (device_string != 0)
    {
        std_cout << "  CL_DEVICE_EXTENSIONS:";
        std::string stdDevString;
        stdDevString = std::string(device_string);
        size_t szOldPos = 0;
        size_t szSpacePos = stdDevString.find(' ', szOldPos); // extensions string is space delimited
        while (szSpacePos != stdDevString.npos)
        {
            if( strcmp("cl_nv_device_attribute_query", stdDevString.substr(szOldPos, szSpacePos - szOldPos).c_str()) == 0 )
                nv_device_attibute_query = true;

            if (szOldPos > 0)
            {
                std_cout << "\t\t";
            }
            std_cout << "\t\t\t" << stdDevString.substr(szOldPos, szSpacePos - szOldPos).c_str() << "\n";

            do {
                szOldPos = szSpacePos + 1;
                szSpacePos = stdDevString.find(' ', szOldPos);
            } while (szSpacePos == szOldPos);
        }
    }
    else
    {
        std_cout << "  CL_DEVICE_EXTENSIONS: None\n";
    }

    if(nv_device_attibute_query)
    {
        cl_uint compute_capability_major, compute_capability_minor;
        clGetDeviceInfo(device, CL_DEVICE_COMPUTE_CAPABILITY_MAJOR_NV, sizeof(cl_uint), &compute_capability_major, NULL);
        clGetDeviceInfo(device, CL_DEVICE_COMPUTE_CAPABILITY_MINOR_NV, sizeof(cl_uint), &compute_capability_minor, NULL);
        std_cout << "\n  CL_DEVICE_COMPUTE_CAPABILITY_NV:\t" << compute_capability_major << "." << compute_capability_minor << "\n";

        std_cout << "  NUMBER OF MULTIPROCESSORS:\t\t" << compute_units << "\n"; // this is the same value reported by CL_DEVICE_MAX_COMPUTE_UNITS
        std_cout << "  NUMBER OF CUDA CORES:\t\t\t" << ConvertSMVer2Cores(compute_capability_major, compute_capability_minor) * compute_units << "\n";

        cl_uint regs_per_block;
        clGetDeviceInfo(device, CL_DEVICE_REGISTERS_PER_BLOCK_NV, sizeof(cl_uint), &regs_per_block, NULL);
        std_cout << "  CL_DEVICE_REGISTERS_PER_BLOCK_NV:\t" << regs_per_block << "\n";

        cl_uint warp_size;
        clGetDeviceInfo(device, CL_DEVICE_WARP_SIZE_NV, sizeof(cl_uint), &warp_size, NULL);
        std_cout << "  CL_DEVICE_WARP_SIZE_NV:\t\t" << warp_size << "\n";

        cl_bool gpu_overlap;
        clGetDeviceInfo(device, CL_DEVICE_GPU_OVERLAP_NV, sizeof(cl_bool), &gpu_overlap, NULL);
        std_cout << "  CL_DEVICE_GPU_OVERLAP_NV:\t\t" << (gpu_overlap == CL_TRUE ? "CL_TRUE" : "CL_FALSE") << "\n";

        cl_bool exec_timeout;
        clGetDeviceInfo(device, CL_DEVICE_KERNEL_EXEC_TIMEOUT_NV, sizeof(cl_bool), &exec_timeout, NULL);
        std_cout << "  CL_DEVICE_KERNEL_EXEC_TIMEOUT_NV:\t" << (exec_timeout == CL_TRUE ? "CL_TRUE" : "CL_FALSE") << "\n";

        cl_bool integrated_memory;
        clGetDeviceInfo(device, CL_DEVICE_INTEGRATED_MEMORY_NV, sizeof(cl_bool), &integrated_memory, NULL);
        std_cout << "  CL_DEVICE_INTEGRATED_MEMORY_NV:\t" << (integrated_memory == CL_TRUE ? "CL_TRUE" : "CL_FALSE") << "\n";
    }

    // CL_DEVICE_PREFERRED_VECTOR_WIDTH_<type>
    std_cout << "  CL_DEVICE_PREFERRED_VECTOR_WIDTH_<t>\t";
    cl_uint vec_width [6];
    clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, sizeof(cl_uint), &vec_width[0], NULL);
    clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, sizeof(cl_uint), &vec_width[1], NULL);
    clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT, sizeof(cl_uint), &vec_width[2], NULL);
    clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG, sizeof(cl_uint), &vec_width[3], NULL);
    clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, sizeof(cl_uint), &vec_width[4], NULL);
    clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, sizeof(cl_uint), &vec_width[5], NULL);
    std_cout << "CHAR " << vec_width[0] << ", SHORT " << vec_width[1] << ", INT " << vec_width[2] << ", LONG " << vec_width[3] << ", FLOAT " << vec_width[4] << ", DOUBLE " << vec_width[5] << "\n";
}

//////////////////////////////////////////////////////////////////////////////
//! Get and return device capability
//!
//! @return the 2 digit integer representation of device Cap (major minor). return -1 if NA
//! @param device         OpenCL id of the device
//////////////////////////////////////////////////////////////////////////////
int oclGetDevCap(cl_device_id device)
{
    char cDevString[1024];
    bool bDevAttributeQuery = false;
    int iDevArch = -1;

    // Get device extensions, and if any then search for cl_nv_device_attribute_query
    clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, sizeof(cDevString), &cDevString, NULL);
    if (cDevString != 0)
    {
        std::string stdDevString;
        stdDevString = std::string(cDevString);
        size_t szOldPos = 0;
        size_t szSpacePos = stdDevString.find(' ', szOldPos); // extensions string is space delimited
        while (szSpacePos != stdDevString.npos)
        {
            if( strcmp("cl_nv_device_attribute_query", stdDevString.substr(szOldPos, szSpacePos - szOldPos).c_str()) == 0 )
            {
                bDevAttributeQuery = true;
            }

            do {
                szOldPos = szSpacePos + 1;
                szSpacePos = stdDevString.find(' ', szOldPos);
            } while (szSpacePos == szOldPos);
        }
    }

    // if search succeeded, get device caps
    if(bDevAttributeQuery)
    {
        cl_int iComputeCapMajor, iComputeCapMinor;
        clGetDeviceInfo(device, CL_DEVICE_COMPUTE_CAPABILITY_MAJOR_NV, sizeof(cl_uint), (void*)&iComputeCapMajor, NULL);
        clGetDeviceInfo(device, CL_DEVICE_COMPUTE_CAPABILITY_MINOR_NV, sizeof(cl_uint), (void*)&iComputeCapMinor, NULL);
        iDevArch = (10 * iComputeCapMajor) + iComputeCapMinor;
    }

    return iDevArch;
}

// Helper function to get OpenCL error string from constant
// *********************************************************************
const char* oclErrorString(cl_int error)
{
    static const char* errorString[] = {
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

