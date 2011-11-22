OclUtils
================================

Content
-------------------------

C++ library to initialize a list of OpenCL platforms and the available (sorted) list of devices on each platforms.

Compilation
-------------------------

To compile, you'll need [CMake version 2.4](http://www.cmake.org/) or up.

``` bash
$ git clone git://github.com/nbigaouette/oclutils.git
$ cd oclutils
$ mkdir build
$ cd build
$ cmake ../ -DCMAKE_INSTALL_PREFIX="/usr"
$ make
```
If cmake fails because it can't find the OpenCL headers or library (because they are
installed some non-standard place), define the environment variables OpenCL_INCPATH
and OpenCL_LIBPATH. For example:

``` bash
$ OpenCL_INCPATH=/opt/cuda/4.0/include OpenCL_LIBPATH=/usr/lib64/nvidia cmake ../ -DCMAKE_INSTALL_PREFIX="/usr"
$ make
```

Then to install:

``` bash
$ sudo make install
```

This will install the include file into /usr/include and the
libraries into /usr/lib.

To use it in you code, just include OclUtils.hpp:

``` C++
#include <OclUtils.hpp>
```

and link to either the dynamic lib:

``` bash
$ g++ [...] -loclutils [...]
```

or static lib:

``` bash
$ g++ [...] /usr/lib/liboclutils.a [...]
```

Usage
-------------------------
Take a look at example/Example.cpp for how to use. It should be quite straightforward:

``` C++
#include <OclUtils.hpp>
int main(int argc, char *argv[])
{
    OpenCL_platforms_list platforms_list;
    platforms_list.Initialize("nvidia", true);
    const std::string platform = platforms_list.Get_Running_Platform();
    platforms_list[platform].Lock_Best_Device();
    platforms_list.Print();
    return EXIT_SUCCESS;
}
```

With a single device present, running a second instance of the example will abort. Try it!


What's new
-------------------------
* v1.0 Library will try 5 times to acquire context and locks with random delay. Useful if first try fails.
* v0.9 Locking has been re-written. Before, a single file "/tmp/gpu_usage.txt" would contain OpenCL devices used. This was prone to errors as if the
program did not exist correctly, the line containing the used device would be kept in the file, preventing other programs from using the device. Instead,
a file named with the device id, platform and name is opened, and a [flock()](http://linux.die.net/man/2/flock) is acquired on the file. Another process
trying to acquire the same lock would fail. On program termination, crash or simply close(), the lock is released automatically. The file is unfortunately
not deleted, but since it's in /tmp it shouldn't matter much.
Note that this locking will NOT work over NFS! Please use another directory as temporary folder.


License
-------------------------

This code is distributed under the terms of the [GNU General Public License v3 (GPLv3)](http://www.gnu.org/licenses/gpl.html) and is Copyright 2011 Nicolas Bigaouette.


Example
-------------------------
Here's the output of the example code on a machine with the three (AMD APP, NVIDIA CUDA and Intel OpenCL) SDK
installed. Both AMD and Intel's SDK expose the CPU, while the NVIDIA SDK only expose the video cards (two GTX 580, one GT 220).
If there was an AMD/ATI video card in it, the AMD APP would show it too:

```
$ ./example/OclUtilsExample
-------------------------------------------------------------------------------------------------------------
OpenCL: Getting a list of platform(s)... done.
OpenCL: Initializing the 3 available platforms...
        1/3) Intel(R) Corporation
        2/3) NVIDIA Corporation
        3/3) Advanced Micro Devices, Inc.
OpenCL: Initialize platform "Intel(R) OpenCL"'s device(s)
OpenCL: WARNING: Can't find a usable GPU!
OpenCL: Attempt to open and lock file /tmp/OpenCL_Platform0_Device0__Intel_R__OpenCL_Quad_Core_AMD_Opteron_tm__Processor_2389.lck
OpenCL: Closing lock file!
OpenCL: Trying to set an context on Quad-Core AMD Opteron(tm) Processor 2389 (id = 0)... Success!
OpenCL: Initialize platform "NVIDIA CUDA"'s device(s)
OpenCL: WARNING: Can't find a usable CPU!
OpenCL: Attempt to open and lock file /tmp/OpenCL_Platform1_Device0__NVIDIA_CUDA_GeForce_GTX_580.lck
OpenCL: Closing lock file!
OpenCL: Attempt to open and lock file /tmp/OpenCL_Platform1_Device1__NVIDIA_CUDA_GeForce_210.lck
OpenCL: Closing lock file!
OpenCL: Attempt to open and lock file /tmp/OpenCL_Platform1_Device2__NVIDIA_CUDA_GeForce_GTX_580.lck
OpenCL: Closing lock file!
OpenCL: Trying to set an context on GeForce GTX 580 (id = 0)... Success!
OpenCL: Initialize platform "AMD Accelerated Parallel Processing"'s device(s)
OpenCL: WARNING: Can't find a usable GPU!
OpenCL: Attempt to open and lock file /tmp/OpenCL_Platform2_Device0__AMD_Accelerated_Parallel_Processing_Quad_Core_AMD_Opteron_tm__Processor_2389.lck
OpenCL: Closing lock file!
OpenCL: Trying to set an context on Quad-Core AMD Opteron(tm) Processor 2389 (id = 0)... Success!
OpenCL: Attempt to open and lock file /tmp/OpenCL_Platform2_Device0__AMD_Accelerated_Parallel_Processing_Quad_Core_AMD_Opteron_tm__Processor_2389.lck
OpenCL: Available platforms:
    Platform information:
        vendor:     Advanced Micro Devices, Inc.
        name:       AMD Accelerated Parallel Processing
        version:    OpenCL 1.1 AMD-APP-SDK-v2.5 (684.213)
        extensions: cl_khr_icd cl_amd_event_callback cl_amd_offline_devices
        id:         0x7fac0ba92060
        profile:    FULL_PROFILE
        key:        amd
        list:       0x7fffc31c5d40
    Available OpenCL devices on platform:
    ---------------------------------------------------------------------------------------------------------
    name: Quad-Core AMD Opteron(tm) Processor 2389
        id:                             0
        parent platform:                AMD Accelerated Parallel Processing
        device_is_used:                 no
        max_compute_unit:               8
        device is GPU?                  no
        address_bits:                   64
        available:                      yes
        compiler_available:             yes
        endian_little:                  yes
        error_correction_support:       no
        execution_capabilities:         3
        global_mem_cache_size:          65536 bytes (64 KiB, 0.0625 MiB, 6.10352e-05 GiB)
        global_mem_cache_type:          2
        global_mem_cacheline_size:      64 bytes (0.0625 KiB, 6.10352e-05 MiB, 5.96046e-08 GiB)
        global_mem_size:                16863936512 bytes (1.64687e+07 KiB, 16082.7 MiB, 15.7058 GiB)
        image_support:                  yes
        image2d_max_height:             8192
        image2d_max_width:              8192
        image3d_max_depth:              2048
        image3d_max_height:             2048
        image3d_max_width:              2048
        local_mem_size:                 32768 bytes (32 KiB, 0.03125 MiB, 3.05176e-05 GiB)
        local_mem_type:                 2
        max_clock_frequency:            2900 MHz
        max_compute_units:              8
        max_constant_args:              8
        max_constant_buffer_size:       65536 bytes (64 KiB, 0.0625 MiB, 6.10352e-05 GiB)
        max_mem_alloc_size:             4215984128 bytes (4.11717e+06 KiB, 4020.68 MiB, 3.92644 GiB)
        max_parameter_size:             4096 bytes (4 KiB, 0.00390625 MiB, 3.8147e-06 GiB)
        max_read_image_args:            128
        max_samplers:                   16
        max_work_group_size:            1024 bytes (1 KiB, 0.000976562 MiB, 9.53674e-07 GiB)
        max_work_item_dimensions:       3
        max_work_item_sizes:            (1024, 1024, 1024)
        max_write_image_args:           8
        mem_base_addr_align:            1024
        min_data_type_align_size:       128 bytes (0.125 KiB, 0.00012207 MiB, 1.19209e-07 GiB)
        platform:                       0x7fac0ba92060
        preferred_vector_width_char:    16
        preferred_vector_width_short:   8
        preferred_vector_width_int:     4
        preferred_vector_width_long:    2
        preferred_vector_width_float:   4
        preferred_vector_width_double:  0
        profiling_timer_resolution:     1 ns
        queue_properties:               CL_QUEUE_PROFILING_ENABLE,  (2)
        single_fp_config:               CL_FP_DENORM, CL_FP_INF_NAN, CL_FP_ROUND_TO_NEAREST, CL_FP_ROUND_TO_ZERO, CL_FP_ROUND_TO_INF,  (31)
        type:                           CL_DEVICE_TYPE_CPU (2)
        vendor_id:                      4098
        extensions:                     cl_khr_fp64 cl_amd_fp64 cl_khr_global_int32_base_atomics cl_khr_global_int32_extended_atomics cl_khr_local_int32_base_atomics cl_khr_local_int32_extended_atomics cl_khr_int64_base_atomics cl_khr_int64_extended_atomics cl_khr_byte_addressable_store cl_khr_gl_sharing cl_ext_device_fission cl_amd_device_attribute_query cl_amd_vec3 cl_amd_media_ops cl_amd_popcnt cl_amd_printf
        profile:                        FULL_PROFILE
        vendor:                         AuthenticAMD
        version:                        OpenCL 1.1 AMD-APP-SDK-v2.5 (684.213)
        driver_version:                 2.0
        GPU is NOT from NVidia
        Available memory (global):   16863936512 bytes (1.64687e+07 KiB, 16082.7 MiB, 15.7058 GiB)
        Available memory (local):    32768 bytes (32 KiB, 0.03125 MiB, 3.05176e-05 GiB)
        Available memory (constant): 65536 bytes (64 KiB, 0.0625 MiB, 6.10352e-05 GiB)
        *****************************************************************************************************
        Order of preference for OpenCL devices for this platform:
        0.   Quad-Core AMD Opteron(tm) Processor 2389 (id = 0)
        *****************************************************************************************************
    Platform information:
        vendor:     Intel(R) Corporation
        name:       Intel(R) OpenCL
        version:    OpenCL 1.1 LINUX
        extensions: cl_khr_fp64 cl_khr_global_int32_base_atomics cl_khr_global_int32_extended_atomics cl_khr_local_int32_base_atomics cl_khr_local_int32_extended_atomics cl_khr_gl_sharing cl_khr_byte_addressable_store cl_intel_printf cl_ext_device_fission cl_khr_icd
        id:         0x1a14720
        profile:    FULL_PROFILE
        key:        intel
        list:       0x7fffc31c5d40
    Available OpenCL devices on platform:
    ---------------------------------------------------------------------------------------------------------
    name: Quad-Core AMD Opteron(tm) Processor 2389
        id:                             0
        parent platform:                Intel(R) OpenCL
        device_is_used:                 no
        max_compute_unit:               8
        device is GPU?                  no
        address_bits:                   64
        available:                      yes
        compiler_available:             yes
        endian_little:                  yes
        error_correction_support:       no
        execution_capabilities:         3
        global_mem_cache_size:          524288 bytes (512 KiB, 0.5 MiB, 0.000488281 GiB)
        global_mem_cache_type:          2
        global_mem_cacheline_size:      64 bytes (0.0625 KiB, 6.10352e-05 MiB, 5.96046e-08 GiB)
        global_mem_size:                16863936512 bytes (1.64687e+07 KiB, 16082.7 MiB, 15.7058 GiB)
        image_support:                  yes
        image2d_max_height:             8192
        image2d_max_width:              8192
        image3d_max_depth:              2048
        image3d_max_height:             2048
        image3d_max_width:              2048
        local_mem_size:                 32768 bytes (32 KiB, 0.03125 MiB, 3.05176e-05 GiB)
        local_mem_type:                 2
        max_clock_frequency:            0 MHz
        max_compute_units:              8
        max_constant_args:              128
        max_constant_buffer_size:       131072 bytes (128 KiB, 0.125 MiB, 0.00012207 GiB)
        max_mem_alloc_size:             4215984128 bytes (4.11717e+06 KiB, 4020.68 MiB, 3.92644 GiB)
        max_parameter_size:             1024 bytes (1 KiB, 0.000976562 MiB, 9.53674e-07 GiB)
        max_read_image_args:            128
        max_samplers:                   128
        max_work_group_size:            1024 bytes (1 KiB, 0.000976562 MiB, 9.53674e-07 GiB)
        max_work_item_dimensions:       3
        max_work_item_sizes:            (1024, 1024, 1024)
        max_write_image_args:           128
        mem_base_addr_align:            1024
        min_data_type_align_size:       128 bytes (0.125 KiB, 0.00012207 MiB, 1.19209e-07 GiB)
        platform:                       0x1a14720
        preferred_vector_width_char:    16
        preferred_vector_width_short:   8
        preferred_vector_width_int:     4
        preferred_vector_width_long:    2
        preferred_vector_width_float:   4
        preferred_vector_width_double:  2
        profiling_timer_resolution:     344827 ns
        queue_properties:               CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, CL_QUEUE_PROFILING_ENABLE,  (3)
        single_fp_config:               CL_FP_DENORM, CL_FP_INF_NAN, CL_FP_ROUND_TO_NEAREST,  (7)
        type:                           CL_DEVICE_TYPE_CPU (2)
        vendor_id:                      32902
        extensions:                     cl_khr_fp64 cl_khr_global_int32_base_atomics cl_khr_global_int32_extended_atomics cl_khr_local_int32_base_atomics cl_khr_local_int32_extended_atomics cl_khr_gl_sharing cl_khr_byte_addressable_store cl_intel_printf cl_ext_device_fission
        profile:                        FULL_PROFILE
        vendor:                         Intel(R) Corporation
        version:                        OpenCL 1.1
        driver_version:                 1.1
        GPU is NOT from NVidia
        Available memory (global):   16863936512 bytes (1.64687e+07 KiB, 16082.7 MiB, 15.7058 GiB)
        Available memory (local):    32768 bytes (32 KiB, 0.03125 MiB, 3.05176e-05 GiB)
        Available memory (constant): 131072 bytes (128 KiB, 0.125 MiB, 0.00012207 GiB)
        *****************************************************************************************************
        Order of preference for OpenCL devices for this platform:
        0.   Quad-Core AMD Opteron(tm) Processor 2389 (id = 0)
        *****************************************************************************************************
    Platform information:
        vendor:     NVIDIA Corporation
        name:       NVIDIA CUDA
        version:    OpenCL 1.1 CUDA 4.1.1
        extensions: cl_khr_byte_addressable_store cl_khr_icd cl_khr_gl_sharing cl_nv_compiler_options cl_nv_device_attribute_query cl_nv_pragma_unroll
        id:         0x483c990
        profile:    FULL_PROFILE
        key:        nvidia
        list:       0x7fffc31c5d40
    Available OpenCL devices on platform:
    ---------------------------------------------------------------------------------------------------------
    name: GeForce GTX 580
        id:                             0
        parent platform:                NVIDIA CUDA
        device_is_used:                 no
        max_compute_unit:               16
        device is GPU?                  yes
        address_bits:                   32
        available:                      yes
        compiler_available:             yes
        endian_little:                  yes
        error_correction_support:       no
        execution_capabilities:         1
        global_mem_cache_size:          262144 bytes (256 KiB, 0.25 MiB, 0.000244141 GiB)
        global_mem_cache_type:          2
        global_mem_cacheline_size:      128 bytes (0.125 KiB, 0.00012207 MiB, 1.19209e-07 GiB)
        global_mem_size:                3220897792 bytes (3.14541e+06 KiB, 3071.69 MiB, 2.99969 GiB)
        image_support:                  yes
        image2d_max_height:             32768
        image2d_max_width:              32768
        image3d_max_depth:              2048
        image3d_max_height:             2048
        image3d_max_width:              2048
        local_mem_size:                 49152 bytes (48 KiB, 0.046875 MiB, 4.57764e-05 GiB)
        local_mem_type:                 1
        max_clock_frequency:            1544 MHz
        max_compute_units:              16
        max_constant_args:              9
        max_constant_buffer_size:       65536 bytes (64 KiB, 0.0625 MiB, 6.10352e-05 GiB)
        max_mem_alloc_size:             805224448 bytes (786352 KiB, 767.922 MiB, 0.749924 GiB)
        max_parameter_size:             4352 bytes (4.25 KiB, 0.00415039 MiB, 4.05312e-06 GiB)
        max_read_image_args:            128
        max_samplers:                   16
        max_work_group_size:            1024 bytes (1 KiB, 0.000976562 MiB, 9.53674e-07 GiB)
        max_work_item_dimensions:       3
        max_work_item_sizes:            (1024, 1024, 64)
        max_write_image_args:           8
        mem_base_addr_align:            4096
        min_data_type_align_size:       128 bytes (0.125 KiB, 0.00012207 MiB, 1.19209e-07 GiB)
        platform:                       0x483c990
        preferred_vector_width_char:    1
        preferred_vector_width_short:   1
        preferred_vector_width_int:     1
        preferred_vector_width_long:    1
        preferred_vector_width_float:   1
        preferred_vector_width_double:  1
        profiling_timer_resolution:     1000 ns
        queue_properties:               CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, CL_QUEUE_PROFILING_ENABLE,  (3)
        single_fp_config:               CL_FP_DENORM, CL_FP_INF_NAN, CL_FP_ROUND_TO_NEAREST, CL_FP_ROUND_TO_ZERO, CL_FP_ROUND_TO_INF, CL_FP_FMA,  (63)
        type:                           CL_DEVICE_TYPE_GPU (4)
        vendor_id:                      4318
        extensions:                     cl_khr_byte_addressable_store cl_khr_icd cl_khr_gl_sharing cl_nv_compiler_options cl_nv_device_attribute_query cl_nv_pragma_unroll  cl_khr_global_int32_base_atomics cl_khr_global_int32_extended_atomics cl_khr_local_int32_base_atomics cl_khr_local_int32_extended_atomics cl_khr_fp64
        profile:                        FULL_PROFILE
        vendor:                         NVIDIA Corporation
        version:                        OpenCL 1.1 CUDA
        driver_version:                 285.05.09
        GPU is from NVidia
            nvidia_device_compute_capability_major: 2
            nvidia_device_compute_capability_minor: 0
            nvidia_device_registers_per_block:      32768
            nvidia_device_warp_size:                32
            nvidia_device_gpu_overlap:              yes
            nvidia_device_kernel_exec_timeout:      no
            nvidia_device_integrated_memory:        no
        Available memory (global):   3220897792 bytes (3.14541e+06 KiB, 3071.69 MiB, 2.99969 GiB)
        Available memory (local):    49152 bytes (48 KiB, 0.046875 MiB, 4.57764e-05 GiB)
        Available memory (constant): 65536 bytes (64 KiB, 0.0625 MiB, 6.10352e-05 GiB)
    ---------------------------------------------------------------------------------------------------------
    name: GeForce GTX 580
        id:                             2
        parent platform:                NVIDIA CUDA
        device_is_used:                 no
        max_compute_unit:               16
        device is GPU?                  yes
        address_bits:                   32
        available:                      yes
        compiler_available:             yes
        endian_little:                  yes
        error_correction_support:       no
        execution_capabilities:         1
        global_mem_cache_size:          262144 bytes (256 KiB, 0.25 MiB, 0.000244141 GiB)
        global_mem_cache_type:          2
        global_mem_cacheline_size:      128 bytes (0.125 KiB, 0.00012207 MiB, 1.19209e-07 GiB)
        global_mem_size:                3220897792 bytes (3.14541e+06 KiB, 3071.69 MiB, 2.99969 GiB)
        image_support:                  yes
        image2d_max_height:             32768
        image2d_max_width:              32768
        image3d_max_depth:              2048
        image3d_max_height:             2048
        image3d_max_width:              2048
        local_mem_size:                 49152 bytes (48 KiB, 0.046875 MiB, 4.57764e-05 GiB)
        local_mem_type:                 1
        max_clock_frequency:            1544 MHz
        max_compute_units:              16
        max_constant_args:              9
        max_constant_buffer_size:       65536 bytes (64 KiB, 0.0625 MiB, 6.10352e-05 GiB)
        max_mem_alloc_size:             805224448 bytes (786352 KiB, 767.922 MiB, 0.749924 GiB)
        max_parameter_size:             4352 bytes (4.25 KiB, 0.00415039 MiB, 4.05312e-06 GiB)
        max_read_image_args:            128
        max_samplers:                   16
        max_work_group_size:            1024 bytes (1 KiB, 0.000976562 MiB, 9.53674e-07 GiB)
        max_work_item_dimensions:       3
        max_work_item_sizes:            (1024, 1024, 64)
        max_write_image_args:           8
        mem_base_addr_align:            4096
        min_data_type_align_size:       128 bytes (0.125 KiB, 0.00012207 MiB, 1.19209e-07 GiB)
        platform:                       0x483c990
        preferred_vector_width_char:    1
        preferred_vector_width_short:   1
        preferred_vector_width_int:     1
        preferred_vector_width_long:    1
        preferred_vector_width_float:   1
        preferred_vector_width_double:  1
        profiling_timer_resolution:     1000 ns
        queue_properties:               CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, CL_QUEUE_PROFILING_ENABLE,  (3)
        single_fp_config:               CL_FP_DENORM, CL_FP_INF_NAN, CL_FP_ROUND_TO_NEAREST, CL_FP_ROUND_TO_ZERO, CL_FP_ROUND_TO_INF, CL_FP_FMA,  (63)
        type:                           CL_DEVICE_TYPE_GPU (4)
        vendor_id:                      4318
        extensions:                     cl_khr_byte_addressable_store cl_khr_icd cl_khr_gl_sharing cl_nv_compiler_options cl_nv_device_attribute_query cl_nv_pragma_unroll  cl_khr_global_int32_base_atomics cl_khr_global_int32_extended_atomics cl_khr_local_int32_base_atomics cl_khr_local_int32_extended_atomics cl_khr_fp64
        profile:                        FULL_PROFILE
        vendor:                         NVIDIA Corporation
        version:                        OpenCL 1.1 CUDA
        driver_version:                 285.05.09
        GPU is from NVidia
            nvidia_device_compute_capability_major: 2
            nvidia_device_compute_capability_minor: 0
            nvidia_device_registers_per_block:      32768
            nvidia_device_warp_size:                32
            nvidia_device_gpu_overlap:              yes
            nvidia_device_kernel_exec_timeout:      no
            nvidia_device_integrated_memory:        no
        Available memory (global):   3220897792 bytes (3.14541e+06 KiB, 3071.69 MiB, 2.99969 GiB)
        Available memory (local):    49152 bytes (48 KiB, 0.046875 MiB, 4.57764e-05 GiB)
        Available memory (constant): 65536 bytes (64 KiB, 0.0625 MiB, 6.10352e-05 GiB)
    ---------------------------------------------------------------------------------------------------------
    name: GeForce 210
        id:                             1
        parent platform:                NVIDIA CUDA
        device_is_used:                 no
        max_compute_unit:               2
        device is GPU?                  yes
        address_bits:                   32
        available:                      yes
        compiler_available:             yes
        endian_little:                  yes
        error_correction_support:       no
        execution_capabilities:         1
        global_mem_cache_size:          0 bytes (0 KiB, 0 MiB, 0 GiB)
        global_mem_cache_type:          0
        global_mem_cacheline_size:      0 bytes (0 KiB, 0 MiB, 0 GiB)
        global_mem_size:                536674304 bytes (524096 KiB, 511.812 MiB, 0.499817 GiB)
        image_support:                  yes
        image2d_max_height:             16383
        image2d_max_width:              4096
        image3d_max_depth:              2048
        image3d_max_height:             2048
        image3d_max_width:              2048
        local_mem_size:                 16384 bytes (16 KiB, 0.015625 MiB, 1.52588e-05 GiB)
        local_mem_type:                 1
        max_clock_frequency:            1230 MHz
        max_compute_units:              2
        max_constant_args:              9
        max_constant_buffer_size:       65536 bytes (64 KiB, 0.0625 MiB, 6.10352e-05 GiB)
        max_mem_alloc_size:             134217728 bytes (131072 KiB, 128 MiB, 0.125 GiB)
        max_parameter_size:             4352 bytes (4.25 KiB, 0.00415039 MiB, 4.05312e-06 GiB)
        max_read_image_args:            128
        max_samplers:                   16
        max_work_group_size:            512 bytes (0.5 KiB, 0.000488281 MiB, 4.76837e-07 GiB)
        max_work_item_dimensions:       3
        max_work_item_sizes:            (512, 512, 64)
        max_write_image_args:           8
        mem_base_addr_align:            2048
        min_data_type_align_size:       128 bytes (0.125 KiB, 0.00012207 MiB, 1.19209e-07 GiB)
        platform:                       0x483c990
        preferred_vector_width_char:    1
        preferred_vector_width_short:   1
        preferred_vector_width_int:     1
        preferred_vector_width_long:    1
        preferred_vector_width_float:   1
        preferred_vector_width_double:  0
        profiling_timer_resolution:     1000 ns
        queue_properties:               CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, CL_QUEUE_PROFILING_ENABLE,  (3)
        single_fp_config:               CL_FP_INF_NAN, CL_FP_ROUND_TO_NEAREST, CL_FP_ROUND_TO_ZERO, CL_FP_ROUND_TO_INF, CL_FP_FMA,  (62)
        type:                           CL_DEVICE_TYPE_GPU (4)
        vendor_id:                      4318
        extensions:                     cl_khr_byte_addressable_store cl_khr_icd cl_khr_gl_sharing cl_nv_compiler_options cl_nv_device_attribute_query cl_nv_pragma_unroll  cl_khr_global_int32_base_atomics cl_khr_global_int32_extended_atomics cl_khr_local_int32_base_atomics cl_khr_local_int32_extended_atomics
        profile:                        FULL_PROFILE
        vendor:                         NVIDIA Corporation
        version:                        OpenCL 1.0 CUDA
        driver_version:                 285.05.09
        GPU is from NVidia
            nvidia_device_compute_capability_major: 1
            nvidia_device_compute_capability_minor: 2
            nvidia_device_registers_per_block:      16384
            nvidia_device_warp_size:                32
            nvidia_device_gpu_overlap:              yes
            nvidia_device_kernel_exec_timeout:      yes
            nvidia_device_integrated_memory:        no
        Available memory (global):   536674304 bytes (524096 KiB, 511.812 MiB, 0.499817 GiB)
        Available memory (local):    16384 bytes (16 KiB, 0.015625 MiB, 1.52588e-05 GiB)
        Available memory (constant): 65536 bytes (64 KiB, 0.0625 MiB, 6.10352e-05 GiB)
        *****************************************************************************************************
        Order of preference for OpenCL devices for this platform:
        0.   GeForce GTX 580 (id = 0)
        1.   GeForce GTX 580 (id = 2)
        2.   GeForce 210 (id = 1)
        *****************************************************************************************************
-------------------------------------------------------------------------------------------------------------
OpenCL: Prefered platform's name:          AMD Accelerated Parallel Processing
OpenCL: Prefered platform's best device:   Quad-Core AMD Opteron(tm) Processor 2389
-------------------------------------------------------------------------------------------------------------
Press enter to exit example. The OpenCL device will be released.

OpenCL: Closing lock file!
```

