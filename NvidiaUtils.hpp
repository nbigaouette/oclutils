/**
 * Comes from oclUtils.{cpp,h} and shrUtils.{cpp,h} from Nvidia's CUDA SDK
 */

#ifndef INC_NVIDIAUTILS_hpp
#define INC_NVIDIAUTILS_hpp

#include <CL/cl.h>

#include <string.h> // strstr

#include <StdCout.hpp>


// Helper function to get OpenCL error string from constant
// *********************************************************************
extern "C" const char* oclErrorString(cl_int error);



#endif // INC_NVIDIAUTILS_hpp

// ********** End of file ***************************************
