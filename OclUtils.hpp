#ifndef INC_OCLUTILS_hpp
#define INC_OCLUTILS_hpp

#include <string>
#include <list>
#include <map>

#include <CL/cl.h>

#include <StdCout.hpp>

#include "NvidiaUtils.hpp"


const std::string OPENCL_PLATFORMS_NVIDIA("nvidia");
const std::string OPENCL_PLATFORMS_AMD("amd");
const std::string OPENCL_PLATFORMS_INTEL("intel");

#define MAX_LOCAL_WORK_SIZE 256


// *****************************************************************************
#define OpenCL_Test_Success(err, fct_name)                          \
if ((err) != CL_SUCCESS)                                            \
{                                                                   \
    std_cout                                                        \
        << "ERROR calling " << fct_name << "() ("                   \
        << __FILE__ << " line " << __LINE__ << "): "                \
        << OpenCL_Error_to_String(err) << "\n" << std::flush;               \
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
// Nvidia extensions. On non-nvidia, needs to define those.
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

// *****************************************************************************
std::string OpenCL_Error_to_String(cl_int error);

// *****************************************************************************
char *read_opencl_kernel(const std::string filename, int *length);

// *****************************************************************************
class OpenCL_device
{
    private:
        int             id;
        cl_device_id    device;
        cl_context      context;
        bool            device_is_gpu;
        bool            device_is_used;

        cl_uint                         address_bits;
        cl_bool                         available;
        cl_bool                         compiler_available;
        cl_device_fp_config             double_fp_config;
        cl_bool                         endian_little;
        cl_bool                         error_correction_support;
        cl_device_exec_capabilities     execution_capabilities;
        cl_ulong                        global_mem_cache_size;
        cl_device_mem_cache_type        global_mem_cache_type;
        cl_uint                         global_mem_cacheline_size;
        cl_ulong                        global_mem_size;
        cl_device_fp_config             half_fp_config;
        cl_bool                         image_support;
        size_t                          image2d_max_height;
        size_t                          image2d_max_width;
        size_t                          image3d_max_depth;
        size_t                          image3d_max_height;
        size_t                          image3d_max_width;
        cl_ulong                        local_mem_size;
        cl_device_local_mem_type        local_mem_type;
        cl_uint                         max_clock_frequency;
        cl_uint                         max_compute_units;
        cl_uint                         max_constant_args;
        cl_ulong                        max_constant_buffer_size;
        cl_ulong                        max_mem_alloc_size;
        size_t                          max_parameter_size;
        cl_uint                         max_read_image_args;
        cl_uint                         max_samplers;
        size_t                          max_work_group_size;
        cl_uint                         max_work_item_dimensions;
        size_t                          max_work_item_sizes[3];
        cl_uint                         max_write_image_args;
        cl_uint                         mem_base_addr_align;
        cl_uint                         min_data_type_align_size;
        cl_platform_id                  platform;
        cl_uint                         preferred_vector_width_char;
        cl_uint                         preferred_vector_width_short;
        cl_uint                         preferred_vector_width_int;
        cl_uint                         preferred_vector_width_long;
        cl_uint                         preferred_vector_width_float;
        cl_uint                         preferred_vector_width_double;
        size_t                          profiling_timer_resolution;
        cl_command_queue_properties     queue_properties;
        cl_device_fp_config             single_fp_config;
        cl_device_type                  type;
        cl_uint                         vendor_id;

        std::string                     extensions;
        std::string                     name;
        std::string                     profile;
        std::string                     vendor;
        std::string                     version;
        std::string                     driver_version;
        std::string                     type_string;
        std::string                     queue_properties_string;
        std::string                     single_fp_config_string;

        // Nvidia specific extensions
        // http://developer.download.nvidia.com/compute/cuda/3_2_prod/toolkit/docs/OpenCL_Extensions/cl_nv_device_attribute_query.txt
        bool                            is_nvidia;
        cl_uint                         nvidia_device_compute_capability_major;
        cl_uint                         nvidia_device_compute_capability_minor;
        cl_uint                         nvidia_device_registers_per_block;
        cl_uint                         nvidia_device_warp_size;
        cl_bool                         nvidia_device_gpu_overlap;
        cl_bool                         nvidia_device_kernel_exec_timeout;
        cl_bool                         nvidia_device_integrated_memory;

    public:
        const OpenCL_platform *parent_platform;

        OpenCL_device();
        ~OpenCL_device();

        const OpenCL_platform *   Get_Parent_Platform() { return parent_platform;    }
        std::string         Get_Name()      { return name;      }
        cl_uint             Get_Compute_Units() { return max_compute_units;      }
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
    std::map<std::string,OpenCL_platform>   platforms;


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
