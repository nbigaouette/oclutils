#ifndef INC_OCLUTILS_hpp
#define INC_OCLUTILS_hpp

#include <string>
#include <list>

#include <CL/cl.h>

#include <StdCout.hpp>

#include "NvidiaUtils.hpp"

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

// *****************************************************************************
char *read_opencl_kernel(const std::string filename, int *length);

// *****************************************************************************
class OpenCL_device
{
    private:
        std::string     name;
        int             id;
        cl_device_id    device;
        cl_context      context;
        cl_uint         max_compute_unit;
        bool            device_is_gpu;
        cl_ulong        available_memory_global;
        cl_ulong        available_memory_local;
        cl_ulong        available_memory_constant;

    public:
        OpenCL_device();
        ~OpenCL_device();

        std::string         Get_Name()      { return name;      }
        cl_uint             Get_Compute_Unit() { return max_compute_unit;      }
        cl_device_id    &   Get_Device()    { return device;    }
        cl_context      &   Get_Context()   { return context;   }

        void Set_Information(const int _id, cl_device_id _device, const bool _device_is_gpu);

        cl_int Set_Context();

        void Print();

        bool operator<(const OpenCL_device &b);
};

// *****************************************************************************
class OpenCL_devices_list
{
private:
    cl_platform_id              platform_id;
    std::list<OpenCL_device>    device_list;
    std::list<OpenCL_device>::iterator it;
    cl_uint                     nb_cpu;
    cl_uint                     nb_gpu;
    int                         err;

    OpenCL_device               *preferred_device;

public:
    OpenCL_devices_list();
    ~OpenCL_devices_list();

    int nb_devices() { return nb_cpu + nb_gpu; }

    void Print();

    inline OpenCL_device &  Prefered_OpenCL()                { return *preferred_device; }
    inline cl_device_id  &  Prefered_OpenCL_Device()         { return Prefered_OpenCL().Get_Device(); }
    inline cl_context    &  Prefered_OpenCL_Device_Context() { return Prefered_OpenCL().Get_Context(); }

    void Initialize();

};



#endif // INC_OCLUTILS_hpp

// ********** End of file ***************************************
