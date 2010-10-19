
#include <CL/opencl.h>

// Beginning of GPU Architecture definitions
inline int ConvertSMVer2Cores(int major, int minor)
{
    // Defines for GPU Architecture types (using the SM version to determine the # of cores per SM
    typedef struct {
        int SM; // 0xMm (hexidecimal notation), M = SM Major version, and m = SM minor version
        int Cores;
    } sSMtoCores;

    sSMtoCores nGpuArchCoresPerSM[] =
    { { 0x10,  8 },
      { 0x11,  8 },
      { 0x12,  8 },
      { 0x13,  8 },
      { 0x20, 32 },
      { 0x21, 48 },
      {   -1, -1 }
    };

    int index = 0;
    while (nGpuArchCoresPerSM[index].SM != -1) {
        if (nGpuArchCoresPerSM[index].SM == ((major << 4) + minor) ) {
            return nGpuArchCoresPerSM[index].Cores;
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

//////////////////////////////////////////////////////////////////////////////
//! Get and return device capability
//!
//! @return the 2 digit integer representation of device Cap (major minor). return -1 if NA
//! @param device         OpenCL id of the device
//////////////////////////////////////////////////////////////////////////////
extern "C" int oclGetDevCap(cl_device_id device);

