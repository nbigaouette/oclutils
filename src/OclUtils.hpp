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

#ifndef INC_OCLUTILS_hpp
#define INC_OCLUTILS_hpp

#include <string>
#include <list>
#include <map>
#include <climits>

#include <CL/cl.h>

#ifndef std_cout
#define std_cout std::cout
#include <iostream>
#include <fstream>
#include <fstream>
#endif // #ifndef std_cout

const std::string OPENCL_PLATFORMS_NVIDIA("nvidia");
const std::string OPENCL_PLATFORMS_AMD("amd");
const std::string OPENCL_PLATFORMS_INTEL("intel");
const std::string OPENCL_PLATFORMS_APPLE("apple");

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
bool Verify_if_Device_is_Used(const int device_id, const int platform_id_offset,
                              const std::string &platform_name, const std::string &device_name);

// *****************************************************************************
char *read_opencl_kernel(const std::string filename, int *length);

// **************************************************************

namespace OclUtils {
template <class Integer>
std::string Integer_in_String_Binary(Integer n)
/**
 * Prints binary representation of an integer if any size.
 * Inspired by http://www.exploringbinary.com/displaying-the-raw-fields-of-a-floating-point-number/
 * WARNING: In C/C++, logical right shift of SIGNED integers is compiler dependant. GCC keeps the
 *          sign bit intact (instead of putting a 0).
 *          So ">>" is an arithmetic shift when the integer is signed. Unsigned are not
 *          affected (arithmetic and logical shifts are the same for unsigned integers).
 *          See http://en.wikipedia.org/wiki/Bitwise_operation#Arithmetic_shift
 */
{
                                        // Example 32 bits integers, converted from
                                        // http://www.binaryconvert.com/convert_unsigned_int.html
    const Integer i_zero    =  Integer(0);  // 00000000 00000000 00000000 00000000
    //const Integer i_ones    = ~i_zero;      // 11111111 11111111 11111111 11111111
    const Integer i_one     =  Integer(1);  // 00000000 00000000 00000000 00000001
    //const Integer i_two     =  Integer(2);  // 00000000 00000000 00000000 00000010
    //const Integer i_eigth   =  Integer(8);  // 00000000 00000000 00000000 00001000
    const Integer nb_bits_per_byte    = CHAR_BIT; // Normaly, it is 8, but could be different.
    const Integer nb_bits_per_Integer = sizeof(n)*nb_bits_per_byte;

    // Starting from the LSB being index "0", the MSB is at index "msb_position"
    const Integer msb_position  = nb_bits_per_Integer - i_one;
    const Integer msb           = i_one << msb_position;
    const Integer or_msb        = Integer(~msb);

    std::string integer_in_binary(nb_bits_per_Integer, ' ');
    Integer counter = 0;

    // Note that right shifting a signed integer migth keep the sign bit intact
    // (instead of setting it to 0) because C/C++ is implementation dependant
    // regarding right shift applied to negative signed integers. GCC will do
    // an "arithmetic right shift", meaning dividing the integer by 2. This will
    // keep the number negative (if it was). Because of this, the mask can get
    // screwed. If the Integer type is signed, first right shifting of the
    // mask of one (having an initial value of "msb" == 10000... and thus a
    // negative value) will keep the sign bit (leading to mask == 11000...) but
    // what we want is just to move the mask's bit, not keep the integer
    // reprentation "valid" (we want mask == 01000...). To fix that, after
    // right shifting the mask by one, we "AND" it (using "&") with "or_msb"
    // (or_msb == 01111...) to make sure we forget the sign bit.
    for (Integer mask = msb ; mask != i_zero ; mask = ((mask >> i_one) & or_msb ))
    {
        // If "n"'s bit at position of the mask is 0, print 0, else print 1.
        if ((mask & n) == i_zero) integer_in_binary[counter++] = '0';
        else                      integer_in_binary[counter++] = '1';
    }

    return integer_in_binary;
}

// **************************************************************
template <class Pointer>
void free_me(Pointer &p)
{
    if (p != NULL)
    {
        // Free memory
        free(p);
    }
    p = NULL;
}

};

// *****************************************************************************
class OpenCL_device
{
    private:
        bool                            object_is_initialized;
        int                             device_id;
        cl_device_id                    device;
        cl_context                      context;
        bool                            device_is_gpu;
        bool                            device_is_in_use;

        // OpenCL device's information.
        // See http://www.khronos.org/registry/cl/sdk/1.0/docs/man/xhtml/clGetDeviceInfo.html
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

        // A lock can be acquired on the device only if another program
        // did not acquired one before. If the program detects that the device
        // was is used by another process, it won't try to lock or unlock the device.
        bool                            is_lockable;
        bool                            file_locked;
        int                             lock_file;

    public:

        const OpenCL_platform          *parent_platform;

        OpenCL_device();
        ~OpenCL_device();
        void Destructor();

        const OpenCL_platform *         Get_Parent_Platform()       { return parent_platform;   }
        std::string                     Get_Name() const            { return name;              }
        cl_uint                         Get_Compute_Units() const   { return max_compute_units; }
        int                             Get_ID() const              { return device_id;         }
        cl_device_id &                  Get_Device()                { return device;            }
        cl_context &                    Get_Context()               { return context;           }
        bool                            Is_In_Use()                 { return device_is_in_use;  }
        bool                            Is_Lockable()               { return is_lockable;       }
        void                            Set_Lockable(const bool _is_lockable) { is_lockable = _is_lockable; }

        void                            Set_Information(const int _id, cl_device_id _device, const int platform_id_offset,
                                                        const std::string &platform_name, const bool _device_is_gpu,
                                                        const OpenCL_platform * const _parent_platform);

        cl_int                          Set_Context();
        void                            Print() const;
        void                            Lock();
        void                            Unlock();
        bool                            operator<(const OpenCL_device &b);
};

// *****************************************************************************
class OpenCL_devices_list
{
    private:
        bool                            is_initialized;
        const OpenCL_platform          *platform;
        std::list<OpenCL_device>        device_list;
        cl_uint                         nb_cpu;
        cl_uint                         nb_gpu;
        int                             err;
        bool                            are_all_devices_in_use;

    public:

        OpenCL_device                  *preferred_device;

        OpenCL_devices_list();
        ~OpenCL_devices_list();

        void                            Set_Preferred_OpenCL(const int _preferred_device = -1);
        OpenCL_device &                 Preferred_OpenCL();
        cl_device_id &                  Preferred_OpenCL_Device()         { return Preferred_OpenCL().Get_Device(); }
        cl_context &                    Preferred_OpenCL_Device_Context() { return Preferred_OpenCL().Get_Context(); }
        int                             nb_devices()                     { return nb_cpu + nb_gpu; }
        void                            Print() const;
        void                            Initialize(const OpenCL_platform &_platform,
                                                   const std::string &preferred_platform);

};

// *****************************************************************************
class OpenCL_platform
{
    private:
        cl_platform_id                  id;
        std::string                     profile;
        std::string                     version;
        std::string                     name;
        std::string                     vendor;
        std::string                     extensions;
        std::string                     key;
        OpenCL_platforms_list           *platform_list;
        int                             id_offset;
    public:
        OpenCL_devices_list             devices_list;
        OpenCL_platform();

        void                            Initialize(std::string _key, int id_offset, cl_platform_id _id,
                                                   OpenCL_platforms_list *_platform_list, const std::string preferred_platform);
        OpenCL_device &                 Preferred_OpenCL()                   { return devices_list.Preferred_OpenCL(); }
        cl_device_id &                  Preferred_OpenCL_Device()            { return devices_list.Preferred_OpenCL_Device(); }
        cl_context &                    Preferred_OpenCL_Device_Context()    { return devices_list.Preferred_OpenCL_Device_Context(); }
        OpenCL_platforms_list *         Platform_List() const               { return platform_list; }
        void                            Print_Preferred() const;
        std::string                     Key() const                         { return key; }
        std::string   const             Name() const                        { return name; }
        cl_platform_id                  Id() const                          { return id; }
        int                             Id_Offset() const                   { return id_offset; }
        void                            Lock_Best_Device();
        void                            Print() const;
};

// *****************************************************************************
class OpenCL_platforms_list
{
    private:
        std::map<std::string,OpenCL_platform>   platforms;
        std::string                     preferred_platform;
        bool                            use_locking;
    public:
        void                            Initialize(const std::string &_preferred_platform, const bool _use_locking = true);
        void                            Print() const;
        void                            Print_Preferred() const;
        std::string                     Get_Running_Platform()              { return preferred_platform; }
        bool                            Use_Locking() const                 { return use_locking; }

        OpenCL_platform & operator[](const std::string key);
        void                            Set_Preferred_OpenCL(const int _preferred_device = -1);
};

// **************************************************************
class OpenCL_Kernel
{
    public:

        OpenCL_Kernel();
        OpenCL_Kernel(std::string _filename, const cl_context &_context,
                      const cl_device_id &_device_id);
        ~OpenCL_Kernel();
        void Initialize(std::string _filename, const cl_context &_context,
                        const cl_device_id &_device_id);

        void Build(std::string _kernel_name);

        // By default global_y is one, local_x is MAX_WORK_SIZE and local_y is one.
        void Compute_Work_Size(size_t _global_x, size_t _global_y, size_t _local_x, size_t _local_y);

        cl_kernel Get_Kernel() const;

        size_t *Get_Global_Work_Size() const;
        size_t *Get_Local_Work_Size() const;

        int Get_Dimension() const;
        void Append_Compiler_Option(const std::string option);

        void Launch(const cl_command_queue &command_queue);

        static int Get_Multiple(int n, int base);

    private:

        std::string filename;
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
        void Build_Executable(const bool verbose = true);
};


// *****************************************************************************
template <class T>
class OpenCL_Array
{
private:
    bool array_is_padded;               // Will the array need to be padded for checksumming?
    int N;                              // Number of elements in array
    size_t sizeof_element;              // Size of each array elements
    uint64_t new_array_size_bytes;      // Size (bytes) of new padded array
    T     *host_array;                  // Pointer to start of host array, INCLUDIǸG padding
    uint64_t nb_1024bits_blocks;        // Number of 1024 bits blocks in padded array
    std::string platform;               // OpenCL platform
    cl_context context;                 // OpenCL context
    cl_command_queue command_queue;     // OpenCL command queue
    cl_device_id device;                // OpenCL device
    cl_int err;                         // Error code

    uint8_t host_checksum[64];          // SHA512 checksum on host memory (512 bits)
    uint8_t device_checksum[64];        // SHA512 checksum on device memory (512 bits)
    static const int buff_size_checksum = sizeof(uint8_t) * 64;

    OpenCL_Kernel kernel_checksum;      // Kernel for checksum calculation

    // Allocated memory on device
    cl_mem device_array;                // Memory of device
    cl_mem cl_array_size_bit;
    cl_mem cl_sha512sum;

public:
    OpenCL_Array();
    void Initialize(int _N, const size_t _sizeof_element,
                    T *&host_array,
                    cl_context &_context, cl_mem_flags flags,
                    std::string _platform,
                    cl_command_queue &_command_queue,
                    cl_device_id &_device,
                    const bool _checksum_array);
    void Release_Memory();
    void Host_to_Device();
    void Device_to_Host();
    std::string Host_Checksum();
    std::string Device_Checksum();
    void Validate_Data();

    inline cl_mem * Get_Device_Array() { return &device_array; }
    inline T *      Get_Host_Pointer() { return  host_array;   }
    void Set_as_Kernel_Argument(cl_kernel &kernel, const int order);
};

// *****************************************************************************
namespace OpenCL_SHA512
{
    // Following code comes from http://tools.ietf.org/html/rfc4634.
    /*
    * These definitions are potentially faster equivalents for the ones
    * used in FIPS-180-2, section 4.1.3.
    *   ((x & y) ^ (~x & z)) becomes
    *   ((x & (y ^ z)) ^ z)
    */
    #define SHA_Ch(x,y,z)        (((x) & (y)) ^ ((~(x)) & (z)))


    /*
    *   ((x & y) ^ (x & z) ^ (y & z)) becomes
    *   ((x & (y | z)) | (y & z))
    */
    #define SHA_Maj(x,y,z)       (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

    /* Define the SHA shift, rotate left and rotate right macro */
    #define SHA512_SHR(bits,word)  (((uint64_t)(word)) >> (bits))
    #define SHA512_ROTR(bits,word) ((((uint64_t)(word)) >> (bits)) | (((uint64_t)(word)) << (64-(bits))))

    /* Define the SHA SIGMA and sigma macros */
    #define SHA512_SIGMA0(word)     (SHA512_ROTR(28,word) ^ SHA512_ROTR(34,word) ^ SHA512_ROTR(39,word))
    #define SHA512_SIGMA1(word)     (SHA512_ROTR(14,word) ^ SHA512_ROTR(18,word) ^ SHA512_ROTR(41,word))
    #define SHA512_sigma0(word)     (SHA512_ROTR( 1,word) ^ SHA512_ROTR( 8,word) ^ SHA512_SHR( 7,word))
    #define SHA512_sigma1(word)     (SHA512_ROTR(19,word) ^ SHA512_ROTR(61,word) ^ SHA512_SHR( 6,word))

    void Prepare_Array_for_Checksuming(void **array, const uint64_t sizeof_element,
                                       uint64_t &array_size_bit);
    void Calculate_Checksum(const void *_message, uint64_t length, uint8_t *_message_digest);
    void Print_Checksum(const uint8_t checksum[64]);
    std::string Checksum_to_String(const uint8_t checksum[64]);
    std::string String_Hexadecimal(const void *array, uint64_t size_bits);
    std::string String_Binary(const void *array, uint64_t size_bits);

    void Validation();
}

#endif // INC_OCLUTILS_hpp

// ********** End of file ***************************************
