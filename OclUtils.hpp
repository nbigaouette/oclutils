
#include <string>

#include <CL/cl.h>

#include <StdCout.hpp>


#define OpenCL_Test_Success(err, fct_name)                          \
if ((err) != CL_SUCCESS)                                            \
{                                                                   \
    std_cout                                                        \
        << "ERROR calling " << fct_name << "() ("                   \
        << __FILE__ << " line " << __LINE__ << "): "                \
        << oclErrorString(err) << "\n" << std::flush;               \
    abort();                                                        \
}

#define OpenCL_Release_Kernel(err, opencl_kernel)                   \
{                                                                   \
    if ((opencl_kernel)) err = clReleaseKernel((opencl_kernel));    \
    OpenCL_Test_Success(err, "clReleaseKernel");                    \
}

#define OpenCL_Release_Program(err, opencl_program)                 \
{                                                                   \
    if ((opencl_program)) err = clReleaseProgram((opencl_program)); \
    OpenCL_Test_Success(err, "clReleaseProgram");                   \
}

#define OpenCL_Release_CommandQueue(err, opencl_cqueue)             \
{                                                                   \
    if ((opencl_cqueue)) err = clReleaseCommandQueue((opencl_cqueue));\
    OpenCL_Test_Success(err, "clReleaseCommandQueue");              \
}

#define OpenCL_Release_Memory(err, opencl_array)                    \
{                                                                   \
    if ((opencl_array)) err = clReleaseMemObject((opencl_array));   \
    OpenCL_Test_Success(err, "clReleaseMemObject");                 \
}

#define OpenCL_Release_Context(err, opencl_context)                 \
{                                                                   \
    if ((opencl_context)) err = clReleaseContext((opencl_context)); \
    OpenCL_Test_Success(err, "clReleaseContext");                   \
}

/******************************************
* cl_nv_device_attribute_query extension *
******************************************/
/* cl_nv_device_attribute_query extension - no extension #define since it has no functions */
#ifndef CL_DEVICE_COMPUTE_CAPABILITY_MAJOR_NV
#define CL_DEVICE_COMPUTE_CAPABILITY_MAJOR_NV       0x4000
#endif

#ifndef CL_DEVICE_COMPUTE_CAPABILITY_MINOR_NV
#define CL_DEVICE_COMPUTE_CAPABILITY_MINOR_NV       0x4001
#endif

#ifndef CL_DEVICE_REGISTERS_PER_BLOCK_NV
#define CL_DEVICE_REGISTERS_PER_BLOCK_NV            0x4002
#endif

#ifndef CL_DEVICE_WARP_SIZE_NV
#define CL_DEVICE_WARP_SIZE_NV                      0x4003
#endif

#ifndef CL_DEVICE_GPU_OVERLAP_NV
#define CL_DEVICE_GPU_OVERLAP_NV                    0x4004
#endif

#ifndef CL_DEVICE_KERNEL_EXEC_TIMEOUT_NV
#define CL_DEVICE_KERNEL_EXEC_TIMEOUT_NV            0x4005
#endif

#ifndef CL_DEVICE_INTEGRATED_MEMORY_NV
#define CL_DEVICE_INTEGRATED_MEMORY_NV              0x4006
#endif


char *read_opencl_kernel(const std::string filename, int *length);

// Beginning of GPU Architecture definitions
inline int ConvertSMVer2Cores(int major, int minor)
{
    // Defines for GPU Architecture types (using the SM version to determine the # of cores per SM
    typedef struct {
        int SM; // 0xMm (hexidecimal notation), M = SM Major version, and m = SM minor version
        int Cores;
    } sSMtoCores;

    sSMtoCores ConvertSMVer2Cores_nGpuArchCoresPerSM[] =
    { { 0x10,  8 },
      { 0x11,  8 },
      { 0x12,  8 },
      { 0x13,  8 },
      { 0x20, 32 },
      { 0x21, 48 },
      {   -1, -1 }
    };

    int index = 0;
    while (ConvertSMVer2Cores_nGpuArchCoresPerSM[index].SM != -1) {
        if (ConvertSMVer2Cores_nGpuArchCoresPerSM[index].SM == ((major << 4) + minor) ) {
            return ConvertSMVer2Cores_nGpuArchCoresPerSM[index].Cores;
        }
        index++;
    }
    printf("MapSMtoCores undefined SMversion %d.%d!\n", major, minor);
    return -1;
}
// end of GPU Architecture definitions

//////////////////////////////////////////////////////////////////////////////
//! Gets the platform ID for NVIDIA if available, otherwise default to platform 0
//!
//! @return the id
//! @param clSelectedPlatformID         OpenCL platform ID
//////////////////////////////////////////////////////////////////////////////
extern "C" cl_int oclGetPlatformID(cl_platform_id* clSelectedPlatformID);

// Helper function to get OpenCL error string from constant
// *********************************************************************
extern "C" const char* oclErrorString(cl_int error);

//////////////////////////////////////////////////////////////////////////////
//! Print info about the device
//!
//! @param iLogMode       enum LOGBOTH, LOGCONSOLE, LOGFILE
//! @param device         OpenCL id of the device
//////////////////////////////////////////////////////////////////////////////
extern "C" void oclPrintDevInfo(cl_device_id device);

//////////////////////////////////////////////////////////////////////////////
//! Print the device name
//!
//! @param device         OpenCL id of the device
//////////////////////////////////////////////////////////////////////////////
extern "C" void oclPrintDevName(cl_device_id device);


// Defines and enum for use with logging functions
// *********************************************************************
#define DEFAULTLOGFILE "SdkConsoleLog.txt"
#define MASTERLOGFILE "SdkMasterLog.csv"
enum LOGMODES
{
    LOGCONSOLE = 1, // bit to signal "log to console"
    //LOGFILE    = 2, // bit to signal "log to file"
    //LOGBOTH    = 3, // convenience union of first 2 bits to signal "log to both"
    //APPENDMODE = 4, // bit to set "file append" mode instead of "replace mode" on open
    //MASTER     = 8, // bit to signal master .csv log output
    ERRORMSG   = 16 // bit to signal "pre-pend Error"
    //CLOSELOG   = 32  // bit to close log file, if open, after any requested file write
};

//////////////////////////////////////////////////////////////////////////////
//! Get and return device capability
//!
//! @return the 2 digit integer representation of device Cap (major minor). return -1 if NA
//! @param device         OpenCL id of the device
//////////////////////////////////////////////////////////////////////////////
extern "C" int oclGetDevCap(cl_device_id device);

// *********************************************************************
// Helper function to log standardized information to Console, to File or to both
//! Examples: shrLogEx(LOGBOTH, 0, "Function A\n");
//!         : shrLogEx(LOGBOTH | ERRORMSG, ciErrNum, STDERROR);
//!
//! Automatically opens file and stores handle if needed and not done yet
//! Closes file and nulls handle on request
//!
//! @param 0 iLogMode: LOGCONSOLE, LOGFILE, LOGBOTH, APPENDMODE, MASTER, ERRORMSG, CLOSELOG.
//!          LOGFILE and LOGBOTH may be | 'd  with APPENDMODE to select file append mode instead of overwrite mode
//!          LOGFILE and LOGBOTH may be | 'd  with CLOSELOG to "write and close"
//!          First 3 options may be | 'd  with MASTER to enable independent write to master data log file
//!          First 3 options may be | 'd  with ERRORMSG to start line with standard error message
//! @param 2 dValue:
//!          Positive val = double value for time in secs to be formatted to 6 decimals.
//!          Negative val is an error code and this give error preformatting.
//! @param 3 cFormatString: String with formatting specifiers like printf or fprintf.
//!          ALL printf flags, width, precision and type specifiers are supported with this exception:
//!              Wide char type specifiers intended for wprintf (%S and %C) are NOT supported
//!              Single byte char type specifiers (%s and %c) ARE supported
//! @param 4... variable args: like printf or fprintf.  Must match format specifer type above.
//! @return 0 if OK, negative value on error or if error occurs or was passed in.
// *********************************************************************
extern "C" int shrLogEx(int iLogMode, int iErrNum, const char* cFormatString, ...);

extern "C" int shrLog(const char* cFormatString, ...);
