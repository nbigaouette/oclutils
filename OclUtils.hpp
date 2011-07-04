#ifndef INC_OCLUTILS_hpp
#define INC_OCLUTILS_hpp

#include <string>
#include <list>

#include <CL/cl.h>

#include <StdCout.hpp>

#include "NvidiaUtils.hpp"


#define MAX_LOCAL_WORK_SIZE 256


// *****************************************************************************
#define OpenCL_Test_Success(err, fct_name)                          \
if ((err) != CL_SUCCESS)                                            \
{                                                                   \
    std_cout                                                        \
        << "ERROR calling " << fct_name << "() ("                   \
        << __FILE__ << " line " << __LINE__ << "): "                \
        << oclErrorString(err) << "\n" << std::flush;               \
    abort();                                                        \
}

// *****************************************************************************
#define OpenCL_Release_Kernel(err, opencl_kernel)                   \
{                                                                   \
    if ((opencl_kernel)) err = clReleaseKernel((opencl_kernel));    \
    OpenCL_Test_Success(err, "clReleaseKernel");                    \
}

// *****************************************************************************
#define OpenCL_Release_Program(err, opencl_program)                 \
{                                                                   \
    if ((opencl_program)) err = clReleaseProgram((opencl_program)); \
    OpenCL_Test_Success(err, "clReleaseProgram");                   \
}

// *****************************************************************************
#define OpenCL_Release_CommandQueue(err, opencl_cqueue)             \
{                                                                   \
    if ((opencl_cqueue)) err = clReleaseCommandQueue((opencl_cqueue));\
    OpenCL_Test_Success(err, "clReleaseCommandQueue");              \
}

// *****************************************************************************
#define OpenCL_Release_Memory(err, opencl_array)                    \
{                                                                   \
    if ((opencl_array)) err = clReleaseMemObject((opencl_array));   \
    OpenCL_Test_Success(err, "clReleaseMemObject");                 \
}

// *****************************************************************************
#define OpenCL_Release_Context(err, opencl_context)                 \
{                                                                   \
    if ((opencl_context)) err = clReleaseContext((opencl_context)); \
    OpenCL_Test_Success(err, "clReleaseContext");                   \
}

class OpenCL_platform;
class OpenCL_platforms_list;
class OpenCL_device;
class OpenCL_devices_list;
class OpenCL_Kernel;

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
        bool            device_is_used;
        cl_ulong        available_memory_global;
        cl_ulong        available_memory_local;
        cl_ulong        available_memory_constant;

    public:
        const OpenCL_platform *parent_platform;

        OpenCL_device();
        ~OpenCL_device();

        const OpenCL_platform *   Get_Parent_Platform() { return parent_platform;    }
        std::string         Get_Name()      { return name;      }
        cl_uint             Get_Compute_Unit() { return max_compute_unit;      }
        int                 Get_ID()        { return id;        }
        cl_device_id    &   Get_Device()    { return device;    }
        cl_context      &   Get_Context()   { return context;   }

        void Set_Information(const int _id, cl_device_id _device, const bool _device_is_gpu);

        cl_int Set_Context();

        void Print();
        bool Is_In_Use()    {return device_is_used;}
        int Get_Id() const  {return id;}

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

    // If the device usage is forced (when all devices are used) we dont want to
    // write anything to /tmp.
    bool                        write_to_tmp;

    OpenCL_device               *preferred_device;

public:
    OpenCL_devices_list();
    ~OpenCL_devices_list();

    int nb_devices() { return nb_cpu + nb_gpu; }

    void Print();

    inline OpenCL_device &  Prefered_OpenCL()                { return *preferred_device; }
    inline cl_device_id  &  Prefered_OpenCL_Device()         { return Prefered_OpenCL().Get_Device(); }
    inline cl_context    &  Prefered_OpenCL_Device_Context() { return Prefered_OpenCL().Get_Context(); }

    void Initialize(const OpenCL_platform &_platform);

};

// *****************************************************************************
class OpenCL_platform
{
private:
public:
    cl_platform_id              id;
    std::string                 profile;
    std::string                 version;
    std::string                 name;
    std::string                 vendor;
    std::string                 extensions;
    OpenCL_devices_list         devices_list;

    OpenCL_platform();

    void Print();
};

// *****************************************************************************
class OpenCL_platforms_list
{
private:
public:
    std::list<OpenCL_platform> platforms;

    OpenCL_platforms_list();

    void Initialize();
    void Print();
};

// **************************************************************
class OpenCL_Kernel
{
    public:

        OpenCL_Kernel(std::string _filename, bool _use_mt, cl_context _context, cl_device_id _device_id);
        ~OpenCL_Kernel();

        void Build(std::string _kernel_name, std::string _compiler_options);

        void Compute_Work_Size(int N);
        void Compute_Work_Size(int N, int _p, int _q);

        cl_kernel Get_Kernel() const;

        size_t *Get_Global_Work_Size() const;
        size_t *Get_Local_Work_Size() const;

        int Get_Dimension() const;
        bool Uses_MT() const;

    private:

        std::string filename;
        bool use_mt;
        cl_context context;
        cl_device_id device_id;

        std::string compiler_options;
        std::string kernel_name;

        int dimension;
        int p;
        int q;

        cl_program program;

        cl_kernel kernel;
        size_t *global_work_size;
        size_t *local_work_size;

        // Debugging variables
        cl_int err;
        cl_event event;

        // Load an OpenCL program from a file
        void Load_Program_From_File();

        // Build runtime executable from a program
        void Build_Executable();

        int Get_Multiple_Of_Work_Size(int n, int _p);
};

#endif // INC_OCLUTILS_hpp

// ********** End of file ***************************************
