/**
 * Comes from oclUtils.{cpp,h} and shrUtils.{cpp,h} from Nvidia's CUDA SDK
 */


#include "NvidiaUtils.hpp"

// Defines for GPU Architecture types (using the SM version to determine the # of cores per SM
static int nGpuArchCoresPerSM[] = { -1, 8, 32 };
// end of GPU Architecture definitions

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
    char device_string[1024];
    bool nv_device_attibute_query = false;

    const int iLogMode = LOGCONSOLE;

    // CL_DEVICE_NAME
    clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_string), &device_string, NULL);
    shrLogEx(iLogMode, 0, "  CL_DEVICE_NAME: \t\t\t%s\n", device_string);

    // CL_DEVICE_VENDOR
    clGetDeviceInfo(device, CL_DEVICE_VENDOR, sizeof(device_string), &device_string, NULL);
    shrLogEx(iLogMode, 0, "  CL_DEVICE_VENDOR: \t\t\t%s\n", device_string);

    // CL_DRIVER_VERSION
    clGetDeviceInfo(device, CL_DRIVER_VERSION, sizeof(device_string), &device_string, NULL);
    shrLogEx(iLogMode, 0, "  CL_DRIVER_VERSION: \t\t\t%s\n", device_string);

    // CL_DEVICE_INFO
    cl_device_type type;
    clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(type), &type, NULL);
    if( type & CL_DEVICE_TYPE_CPU )
        shrLogEx(iLogMode, 0, "  CL_DEVICE_TYPE:\t\t\t%s\n", "CL_DEVICE_TYPE_CPU");
    if( type & CL_DEVICE_TYPE_GPU )
        shrLogEx(iLogMode, 0, "  CL_DEVICE_TYPE:\t\t\t%s\n", "CL_DEVICE_TYPE_GPU");
    if( type & CL_DEVICE_TYPE_ACCELERATOR )
        shrLogEx(iLogMode, 0, "  CL_DEVICE_TYPE:\t\t\t%s\n", "CL_DEVICE_TYPE_ACCELERATOR");
    if( type & CL_DEVICE_TYPE_DEFAULT )
        shrLogEx(iLogMode, 0, "  CL_DEVICE_TYPE:\t\t\t%s\n", "CL_DEVICE_TYPE_DEFAULT");

    // CL_DEVICE_MAX_COMPUTE_UNITS
    cl_uint compute_units;
    clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(compute_units), &compute_units, NULL);
    shrLogEx(iLogMode, 0, "  CL_DEVICE_MAX_COMPUTE_UNITS:\t\t%u\n", compute_units);

    // CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS
    size_t workitem_dims;
    clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(workitem_dims), &workitem_dims, NULL);
    shrLogEx(iLogMode, 0, "  CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS:\t%u\n", workitem_dims);

    // CL_DEVICE_MAX_WORK_ITEM_SIZES
    size_t workitem_size[3];
    clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(workitem_size), &workitem_size, NULL);
    shrLogEx(iLogMode, 0, "  CL_DEVICE_MAX_WORK_ITEM_SIZES:\t%u / %u / %u \n", workitem_size[0], workitem_size[1], workitem_size[2]);

    // CL_DEVICE_MAX_WORK_GROUP_SIZE
    size_t workgroup_size;
    clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(workgroup_size), &workgroup_size, NULL);
    shrLogEx(iLogMode, 0, "  CL_DEVICE_MAX_WORK_GROUP_SIZE:\t%u\n", workgroup_size);

    // CL_DEVICE_MAX_CLOCK_FREQUENCY
    cl_uint clock_frequency;
    clGetDeviceInfo(device, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(clock_frequency), &clock_frequency, NULL);
    shrLogEx(iLogMode, 0, "  CL_DEVICE_MAX_CLOCK_FREQUENCY:\t%u MHz\n", clock_frequency);

    // CL_DEVICE_ADDRESS_BITS
    cl_uint addr_bits;
    clGetDeviceInfo(device, CL_DEVICE_ADDRESS_BITS, sizeof(addr_bits), &addr_bits, NULL);
    shrLogEx(iLogMode, 0, "  CL_DEVICE_ADDRESS_BITS:\t\t%u\n", addr_bits);

    // CL_DEVICE_MAX_MEM_ALLOC_SIZE
    cl_ulong max_mem_alloc_size;
    clGetDeviceInfo(device, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(max_mem_alloc_size), &max_mem_alloc_size, NULL);
    shrLogEx(iLogMode, 0, "  CL_DEVICE_MAX_MEM_ALLOC_SIZE:\t\t%u MByte\n", (unsigned int)(max_mem_alloc_size / (1024 * 1024)));

    // CL_DEVICE_GLOBAL_MEM_SIZE
    cl_ulong mem_size;
    clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(mem_size), &mem_size, NULL);
    shrLogEx(iLogMode, 0, "  CL_DEVICE_GLOBAL_MEM_SIZE:\t\t%u MByte\n", (unsigned int)(mem_size / (1024 * 1024)));

    // CL_DEVICE_ERROR_CORRECTION_SUPPORT
    cl_bool error_correction_support;
    clGetDeviceInfo(device, CL_DEVICE_ERROR_CORRECTION_SUPPORT, sizeof(error_correction_support), &error_correction_support, NULL);
    shrLogEx(iLogMode, 0, "  CL_DEVICE_ERROR_CORRECTION_SUPPORT:\t%s\n", error_correction_support == CL_TRUE ? "yes" : "no");

    // CL_DEVICE_LOCAL_MEM_TYPE
    cl_device_local_mem_type local_mem_type;
    clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_TYPE, sizeof(local_mem_type), &local_mem_type, NULL);
    shrLogEx(iLogMode, 0, "  CL_DEVICE_LOCAL_MEM_TYPE:\t\t%s\n", local_mem_type == 1 ? "local" : "global");

    // CL_DEVICE_LOCAL_MEM_SIZE
    clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(mem_size), &mem_size, NULL);
    shrLogEx(iLogMode, 0, "  CL_DEVICE_LOCAL_MEM_SIZE:\t\t%u KByte\n", (unsigned int)(mem_size / 1024));

    // CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE
    clGetDeviceInfo(device, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof(mem_size), &mem_size, NULL);
    shrLogEx(iLogMode, 0, "  CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE:\t%u KByte\n", (unsigned int)(mem_size / 1024));

    // CL_DEVICE_QUEUE_PROPERTIES
    cl_command_queue_properties queue_properties;
    clGetDeviceInfo(device, CL_DEVICE_QUEUE_PROPERTIES, sizeof(queue_properties), &queue_properties, NULL);
    if( queue_properties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE )
        shrLogEx(iLogMode, 0, "  CL_DEVICE_QUEUE_PROPERTIES:\t\t%s\n", "CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE");
    if( queue_properties & CL_QUEUE_PROFILING_ENABLE )
        shrLogEx(iLogMode, 0, "  CL_DEVICE_QUEUE_PROPERTIES:\t\t%s\n", "CL_QUEUE_PROFILING_ENABLE");

    // CL_DEVICE_IMAGE_SUPPORT
    cl_bool image_support;
    clGetDeviceInfo(device, CL_DEVICE_IMAGE_SUPPORT, sizeof(image_support), &image_support, NULL);
    shrLogEx(iLogMode, 0, "  CL_DEVICE_IMAGE_SUPPORT:\t\t%u\n", image_support);

    // CL_DEVICE_MAX_READ_IMAGE_ARGS
    cl_uint max_read_image_args;
    clGetDeviceInfo(device, CL_DEVICE_MAX_READ_IMAGE_ARGS, sizeof(max_read_image_args), &max_read_image_args, NULL);
    shrLogEx(iLogMode, 0, "  CL_DEVICE_MAX_READ_IMAGE_ARGS:\t%u\n", max_read_image_args);

    // CL_DEVICE_MAX_WRITE_IMAGE_ARGS
    cl_uint max_write_image_args;
    clGetDeviceInfo(device, CL_DEVICE_MAX_WRITE_IMAGE_ARGS, sizeof(max_write_image_args), &max_write_image_args, NULL);
    shrLogEx(iLogMode, 0, "  CL_DEVICE_MAX_WRITE_IMAGE_ARGS:\t%u\n", max_write_image_args);

    // CL_DEVICE_SINGLE_FP_CONFIG
    cl_device_fp_config fp_config;
    clGetDeviceInfo(device, CL_DEVICE_SINGLE_FP_CONFIG, sizeof(cl_device_fp_config), &fp_config, NULL);
    shrLogEx(iLogMode, 0, "  CL_DEVICE_SINGLE_FP_CONFIG:\t\t%s%s%s%s%s%s\n",
        fp_config & CL_FP_DENORM ? "denorms " : "",
        fp_config & CL_FP_INF_NAN ? "INF-quietNaNs " : "",
        fp_config & CL_FP_ROUND_TO_NEAREST ? "round-to-nearest " : "",
        fp_config & CL_FP_ROUND_TO_ZERO ? "round-to-zero " : "",
        fp_config & CL_FP_ROUND_TO_INF ? "round-to-inf " : "",
        fp_config & CL_FP_FMA ? "fma " : "");

    // CL_DEVICE_IMAGE2D_MAX_WIDTH, CL_DEVICE_IMAGE2D_MAX_HEIGHT, CL_DEVICE_IMAGE3D_MAX_WIDTH, CL_DEVICE_IMAGE3D_MAX_HEIGHT, CL_DEVICE_IMAGE3D_MAX_DEPTH
    size_t szMaxDims[5];
    shrLogEx(iLogMode, 0, "\n  CL_DEVICE_IMAGE <dim>");
    clGetDeviceInfo(device, CL_DEVICE_IMAGE2D_MAX_WIDTH, sizeof(size_t), &szMaxDims[0], NULL);
    shrLogEx(iLogMode, 0, "\t\t\t2D_MAX_WIDTH\t %u\n", szMaxDims[0]);
    clGetDeviceInfo(device, CL_DEVICE_IMAGE2D_MAX_HEIGHT, sizeof(size_t), &szMaxDims[1], NULL);
    shrLogEx(iLogMode, 0, "\t\t\t\t\t2D_MAX_HEIGHT\t %u\n", szMaxDims[1]);
    clGetDeviceInfo(device, CL_DEVICE_IMAGE3D_MAX_WIDTH, sizeof(size_t), &szMaxDims[2], NULL);
    shrLogEx(iLogMode, 0, "\t\t\t\t\t3D_MAX_WIDTH\t %u\n", szMaxDims[2]);
    clGetDeviceInfo(device, CL_DEVICE_IMAGE3D_MAX_HEIGHT, sizeof(size_t), &szMaxDims[3], NULL);
    shrLogEx(iLogMode, 0, "\t\t\t\t\t3D_MAX_HEIGHT\t %u\n", szMaxDims[3]);
    clGetDeviceInfo(device, CL_DEVICE_IMAGE3D_MAX_DEPTH, sizeof(size_t), &szMaxDims[4], NULL);
    shrLogEx(iLogMode, 0, "\t\t\t\t\t3D_MAX_DEPTH\t %u\n", szMaxDims[4]);

    // CL_DEVICE_EXTENSIONS: get device extensions, and if any then parse & log the string onto separate lines
    clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, sizeof(device_string), &device_string, NULL);
    if (device_string != 0)
    {
        shrLogEx(iLogMode, 0, "\n  CL_DEVICE_EXTENSIONS:");
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
                shrLogEx(iLogMode, 0, "\t\t");
            }
            shrLogEx(iLogMode, 0, "\t\t\t%s\n", stdDevString.substr(szOldPos, szSpacePos - szOldPos).c_str());

            do {
                szOldPos = szSpacePos + 1;
                szSpacePos = stdDevString.find(' ', szOldPos);
            } while (szSpacePos == szOldPos);
        }
        shrLogEx(iLogMode, 0, "\n");
    }
    else
    {
        shrLogEx(iLogMode, 0, "  CL_DEVICE_EXTENSIONS: None\n");
    }

    if(nv_device_attibute_query)
    {
        cl_uint compute_capability_major, compute_capability_minor;
        clGetDeviceInfo(device, CL_DEVICE_COMPUTE_CAPABILITY_MAJOR_NV, sizeof(cl_uint), &compute_capability_major, NULL);
        clGetDeviceInfo(device, CL_DEVICE_COMPUTE_CAPABILITY_MINOR_NV, sizeof(cl_uint), &compute_capability_minor, NULL);
        shrLogEx(iLogMode, 0, "\n  CL_DEVICE_COMPUTE_CAPABILITY_NV:\t%u.%u\n", compute_capability_major, compute_capability_minor);

        shrLogEx(iLogMode, 0, "  NUMBER OF MULTIPROCESSORS:\t\t%u\n", compute_units); // this is the same value reported by CL_DEVICE_MAX_COMPUTE_UNITS
        shrLogEx(iLogMode, 0, "  NUMBER OF CUDA CORES:\t\t\t%u\n", nGpuArchCoresPerSM[compute_capability_major] * compute_units);

        cl_uint regs_per_block;
        clGetDeviceInfo(device, CL_DEVICE_REGISTERS_PER_BLOCK_NV, sizeof(cl_uint), &regs_per_block, NULL);
        shrLogEx(iLogMode, 0, "  CL_DEVICE_REGISTERS_PER_BLOCK_NV:\t%u\n", regs_per_block);

        cl_uint warp_size;
        clGetDeviceInfo(device, CL_DEVICE_WARP_SIZE_NV, sizeof(cl_uint), &warp_size, NULL);
        shrLogEx(iLogMode, 0, "  CL_DEVICE_WARP_SIZE_NV:\t\t%u\n", warp_size);

        cl_bool gpu_overlap;
        clGetDeviceInfo(device, CL_DEVICE_GPU_OVERLAP_NV, sizeof(cl_bool), &gpu_overlap, NULL);
        shrLogEx(iLogMode, 0, "  CL_DEVICE_GPU_OVERLAP_NV:\t\t%s\n", gpu_overlap == CL_TRUE ? "CL_TRUE" : "CL_FALSE");

        cl_bool exec_timeout;
        clGetDeviceInfo(device, CL_DEVICE_KERNEL_EXEC_TIMEOUT_NV, sizeof(cl_bool), &exec_timeout, NULL);
        shrLogEx(iLogMode, 0, "  CL_DEVICE_KERNEL_EXEC_TIMEOUT_NV:\t%s\n", exec_timeout == CL_TRUE ? "CL_TRUE" : "CL_FALSE");

        cl_bool integrated_memory;
        clGetDeviceInfo(device, CL_DEVICE_INTEGRATED_MEMORY_NV, sizeof(cl_bool), &integrated_memory, NULL);
        shrLogEx(iLogMode, 0, "  CL_DEVICE_INTEGRATED_MEMORY_NV:\t%s\n", integrated_memory == CL_TRUE ? "CL_TRUE" : "CL_FALSE");
    }

    // CL_DEVICE_PREFERRED_VECTOR_WIDTH_<type>
    shrLogEx(iLogMode, 0, "  CL_DEVICE_PREFERRED_VECTOR_WIDTH_<t>\t");
    cl_uint vec_width [6];
    clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, sizeof(cl_uint), &vec_width[0], NULL);
    clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, sizeof(cl_uint), &vec_width[1], NULL);
    clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT, sizeof(cl_uint), &vec_width[2], NULL);
    clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG, sizeof(cl_uint), &vec_width[3], NULL);
    clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, sizeof(cl_uint), &vec_width[4], NULL);
    clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, sizeof(cl_uint), &vec_width[5], NULL);
    shrLogEx(iLogMode, 0, "CHAR %u, SHORT %u, INT %u, LONG %u, FLOAT %u, DOUBLE %u\n\n\n",
           vec_width[0], vec_width[1], vec_width[2], vec_width[3], vec_width[4], vec_width[5]);
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

// *********************************************************************
// Function to log standardized information to console, file or both
static int shrLogV(int iLogMode, int iErrNum, const char* cFormatString, va_list vaArgList)
{
    /*
    static FILE* pFileStream0 = NULL;
    static FILE* pFileStream1 = NULL;
    size_t szNumWritten = 0;
    char cFileMode [3];

    // if the sample log file is closed and the call incudes a "write-to-file", open file for writing
    if ((pFileStream0 == NULL) && (iLogMode & LOGFILE))
    {
        // if the default filename has not been overriden, set to default
        if (cLogFilePathAndName == NULL)
        {
            shrSetLogFileName(DEFAULTLOGFILE);
        }

        #ifdef _WIN32   // Windows version
            // set the file mode
            if (iLogMode & APPENDMODE)  // append to prexisting file contents
            {
                sprintf_s (cFileMode, 3, "a+");
            }
            else                        // replace prexisting file contents
            {
                sprintf_s (cFileMode, 3, "w");
            }

            // open the individual sample log file in the requested mode
            errno_t err = fopen_s(&pFileStream0, cLogFilePathAndName, cFileMode);

            // if error on attempt to open, be sure the file is null or close it, then return negative error code
            if (err != 0)
            {
                if (pFileStream0)
                {
                    fclose (pFileStream0);
                }
                return -err;
            }
        #else           // Linux & Mac version
            // set the file mode
            if (iLogMode & APPENDMODE)  // append to prexisting file contents
            {
                sprintf (cFileMode, "a+");
            }
            else                        // replace prexisting file contents
            {
                sprintf (cFileMode, "w");
            }

            // open the file in the requested mode
            if ((pFileStream0 = fopen(cLogFilePathAndName, cFileMode)) == 0)
            {
                // if error on attempt to open, be sure the file is null or close it, then return negative error code
                if (pFileStream0)
                {
                    fclose (pFileStream0);
                }
                return -1;
            }
        #endif
    }
    */

    /*
    // if the master log file is closed and the call incudes a "write-to-file" and MASTER, open master logfile file for writing
    if ((pFileStream1 == NULL) && (iLogMode & LOGFILE) && (iLogMode & MASTER))
    {
        #ifdef _WIN32   // Windows version
            // open the master log file in append mode
            errno_t err = fopen_s(&pFileStream1, MASTERLOGFILE, "a+");

            // if error on attempt to open, be sure the file is null or close it, then return negative error code
            if (err != 0)
            {
                if (pFileStream1)
                {
                    fclose (pFileStream1);
                }
                return -err;
            }
        #else           // Linux & Mac version

            // open the file in the requested mode
            if ((pFileStream1 = fopen(MASTERLOGFILE, "a+")) == 0)
            {
                // if error on attempt to open, be sure the file is null or close it, then return negative error code
                if (pFileStream1)
                {
                    fclose (pFileStream1);
                }
                return -1;
            }
        #endif

        // If master log file length has become excessive, empty/reopen
        fseek(pFileStream1, 0L, SEEK_END);
        if (ftell(pFileStream1) > 50000L)
        {
            fclose (pFileStream1);
        #ifdef _WIN32   // Windows version
            fopen_s(&pFileStream1, MASTERLOGFILE, "w");
        #else
            pFileStream1 = fopen(MASTERLOGFILE, "w");
        #endif
        }
    }
    */

    /*
    // Handle special Error Message code
    if (iLogMode & ERRORMSG)
    {
        // print string to console if flagged
        if (iLogMode & LOGCONSOLE)
        {
            szNumWritten = printf ("\n !!! Error # %i at ", iErrNum);                           // console
        }
        // print string to file if flagged
        if (iLogMode & LOGFILE)
        {
            szNumWritten = fprintf (pFileStream0, "\n !!! Error # %i at ", iErrNum);            // sample log file
        }
    }
    */

    // Vars used for variable argument processing
    const char*     pStr;
    const char*     cArg;
    int             iArg;
    double          dArg;
    unsigned int    uiArg;
    std::string sFormatSpec;
    const std::string sFormatChars = " -+#0123456789.dioufnpcsXxEeGgAa";
    const std::string sTypeChars = "dioufnpcsXxEeGgAa";
    char cType = 'c';

    // Start at the head of the string and scan to the null at the end
    for (pStr = cFormatString; *pStr; ++pStr)
    {
        // Check if the current character is not a formatting specifier ('%')
        if (*pStr != '%')
        {
            /*
            // character is not '%', so print it verbatim to console and/or files as flagged
            if (iLogMode & LOGCONSOLE)
            {
                szNumWritten = putc(*pStr, stdout);                                             // console
            }
            if (iLogMode & LOGFILE)
            {
                szNumWritten  = putc(*pStr, pFileStream0);                                      // sample log file
                if (iLogMode & MASTER)
                {
                    szNumWritten = putc(*pStr, pFileStream1);                                   // master log file
                }
            }
            */
            std_cout << *pStr;
        }
        else
        {
            // character is '%', so skip over it and read the full format specifier for the argument
            ++pStr;
            sFormatSpec = '%';

            // special handling for string of %%%%
            bool bRepeater = (*pStr == '%');
            if (bRepeater)
            {
                cType = '%';
            }

            // chars after the '%' are part of format if on list of constants... scan until that isn't true or NULL is found
            while (pStr && ((sFormatChars.find(*pStr) != std::string::npos) || bRepeater))
            {
                sFormatSpec += *pStr;

                // If the char is a type specifier, trap it and stop scanning
                // (a type specifier char is always the last in the format except for string of %%%)
                if (sTypeChars.find(*pStr) != std::string::npos)
                {
                    cType = *pStr;
                    break;
                }

                // Special handling for string of %%%
                // If a string of %%% was started and then it ends, break (There won't be a typical type specifier)
                if (bRepeater && (*pStr != '%'))
                {
                    break;
                }

                pStr++;
            }

            // Now handle the arg according to type
            switch (cType)
            {
                case '%':   // special handling for string of %%%%
                {
                    if (iLogMode & LOGCONSOLE)
                    {
                        //szNumWritten = printf(sFormatSpec.c_str());                             // console
                        std_cout << sFormatSpec;
                    }
                    /*
                    if (iLogMode & LOGFILE)
                    {
                        szNumWritten = fprintf (pFileStream0, sFormatSpec.c_str());             // sample log file
                        if (iLogMode & MASTER)
                        {
                            szNumWritten  = fprintf(pFileStream1, sFormatSpec.c_str());         // master log file
                        }
                    }
                    */
                    continue;
                }
                case 'c':   // single byte char
                case 's':   // string of single byte chars
                {
                    // Set cArg as the next value in list and print to console and/or files if flagged
                    cArg = va_arg(vaArgList, char*);
                    if (iLogMode & LOGCONSOLE)
                    {
                        //szNumWritten = printf(sFormatSpec.c_str(), cArg);                       // console
                        std_cout << cArg;
                    }
                    /*
                    if (iLogMode & LOGFILE)
                    {
                        szNumWritten = fprintf (pFileStream0, sFormatSpec.c_str(), cArg);       // sample log file
                        if (iLogMode & MASTER)
                        {
                            szNumWritten  = fprintf(pFileStream1, sFormatSpec.c_str(), cArg);   // master log file
                        }
                    }
                    */
                    continue;
                }
                case 'd':   // signed decimal integer
                case 'i':   // signed decimal integer
                {
                    // set iArg as the next value in list and print to console and/or files if flagged
                    iArg = va_arg(vaArgList, int);
                    if (iLogMode & LOGCONSOLE)
                    {
                        //szNumWritten = printf(sFormatSpec.c_str(), iArg);                       // console
                        std_cout << iArg;
                    }
                    /*
                    if (iLogMode & LOGFILE)
                    {
                        szNumWritten = fprintf (pFileStream0, sFormatSpec.c_str(), iArg);       // sample log file
                        if (iLogMode & MASTER)
                        {
                            szNumWritten  = fprintf(pFileStream1, sFormatSpec.c_str(), iArg);   // master log file
                        }
                    }
                    */
                    continue;
                }
                case 'u':   // unsigned decimal integer
                case 'o':   // unsigned octal integer
                case 'x':   // unsigned hexadecimal integer using "abcdef"
                case 'X':   // unsigned hexadecimal integer using "ABCDEF"
                {
                    // set uiArg as the next value in list and print to console and/or files if flagged
                    uiArg = va_arg(vaArgList, unsigned int);
                    if (iLogMode & LOGCONSOLE)
                    {
                        //szNumWritten = printf(sFormatSpec.c_str(), uiArg);                      // console
                        std_cout << uiArg;
                    }
                    /*
                    if (iLogMode & LOGFILE)
                    {
                        szNumWritten = fprintf (pFileStream0, sFormatSpec.c_str(), uiArg);      // sample log file
                        if (iLogMode & MASTER)
                        {
                            szNumWritten  = fprintf(pFileStream1, sFormatSpec.c_str(), uiArg);  // master log file
                        }
                    }
                    */
                    continue;
                }
                case 'f':   // float/double
                case 'e':   // scientific double/float
                case 'E':   // scientific double/float
                case 'g':   // scientific double/float
                case 'G':   // scientific double/float
                case 'a':   // signed hexadecimal double precision float
                case 'A':   // signed hexadecimal double precision float
                {
                    // set dArg as the next value in list and print to console and/or files if flagged
                    dArg = va_arg(vaArgList, double);
                    if (iLogMode & LOGCONSOLE)
                    {
                        //szNumWritten = printf(sFormatSpec.c_str(), dArg);                       // console
                        std_cout << dArg;
                    }
                    /*
                    if (iLogMode & LOGFILE)
                    {
                        szNumWritten = fprintf (pFileStream0, sFormatSpec.c_str(), dArg);       // sample log file
                        if (iLogMode & MASTER)
                        {
                            szNumWritten  = fprintf(pFileStream1, sFormatSpec.c_str(), dArg);   // master log file
                        }
                    }
                    */
                    continue;
                }
                default:
                {
                    // print arg of unknown/unsupported type to console and/or file if flagged
                    if (iLogMode & LOGCONSOLE)                          // console
                    {
                        //szNumWritten = putc(*pStr, stdout);
                        std_cout << *pStr;
                    }
                    /*
                    if (iLogMode & LOGFILE)
                    {
                        szNumWritten  = putc(*pStr, pFileStream0);      // sample log file
                        if (iLogMode & MASTER)
                        {
                            szNumWritten  = putc(*pStr, pFileStream1);  // master log file
                        }
                    }
                    */
                }
            }
        }
    }

    /*
    // end the sample log with a horizontal line if closing
    if (iLogMode & CLOSELOG)
    {
        if (iLogMode & LOGCONSOLE)
        {
            printf(HDASHLINE);
        }
        if (iLogMode & LOGFILE)
        {
            fprintf(pFileStream0, HDASHLINE);
        }
    }
    */

    // flush console and/or file buffers if updated
    if (iLogMode & LOGCONSOLE)
    {
        //fflush(stdout);
        std_cout.Flush();
    }
    /*
    if (iLogMode & LOGFILE)
    {
        fflush (pFileStream0);

        // if the master log file has been updated, flush it too
        if (iLogMode & MASTER)
        {
            fflush (pFileStream1);
        }
    }
    */

    /*
    // If the log file is open and the caller requests "close file", then close and NULL file handle
    if ((pFileStream0) && (iLogMode & CLOSELOG))
    {
        fclose (pFileStream0);
        pFileStream0 = NULL;
    }
    if ((pFileStream1) && (iLogMode & CLOSELOG))
    {
        fclose (pFileStream1);
        pFileStream1 = NULL;
    }
    */

    // return error code or OK
    if (iLogMode & ERRORMSG)
    {
        return iErrNum;
    }
    else
    {
        return 0;
    }
}

// *********************************************************************
// Function to log standardized information to console, file or both
int shrLogEx(int iLogMode = LOGCONSOLE, int iErrNum = 0, const char* cFormatString = "", ...)
{
    va_list vaArgList;

    // Prepare variable agument list
    va_start(vaArgList, cFormatString);
    int ret = shrLogV(iLogMode, iErrNum, cFormatString, vaArgList);

    // end variable argument handler
    va_end(vaArgList);

    return ret;
}

// *********************************************************************
// Function to log standardized information to console, file or both
int shrLog(const char* cFormatString = "", ...)
{
    va_list vaArgList;

    // Prepare variable agument list
    va_start(vaArgList, cFormatString);
    //int ret = shrLogV(LOGBOTH, 0, cFormatString, vaArgList);
    int ret = shrLogV(LOGCONSOLE, 0, cFormatString, vaArgList);

    // end variable argument handler
    va_end(vaArgList);

    return ret;
}
