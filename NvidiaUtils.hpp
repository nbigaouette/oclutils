/**
 * Comes from oclUtils.{cpp,h} and shrUtils.{cpp,h} from Nvidia's CUDA SDK
 */

#ifndef INC_NVIDIAUTILS_hpp
#define INC_NVIDIAUTILS_hpp

#include <CL/cl.h>

#include <string.h> // strstr

#include <StdCout.hpp>


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



#endif // INC_NVIDIAUTILS_hpp

// ********** End of file ***************************************
