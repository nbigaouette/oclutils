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

License
-------------------------

This code is distributed under the terms of the [GNU General Public License v3 (GPLv3)](http://www.gnu.org/licenses/gpl.html) and is Copyright 2011 Nicolas Bigaouette.


Example
-------------------------
Here's the output of the example code on a machine with the three (AMD APP, NVIDIA CUDA and Intel OpenCL) SDK
installed. Both AMD and Intel's SDK expose the CPU, while the NVIDIA only expose the video card (GT 220).
If there was an AMD/ATI video card in it, the AMD APP would show it too:

```
$ ./example/OclUtilsExample
-------------------------------------------------------------------------------------------------------------
OpenCL: Getting a list of platform(s)... done.
OpenCL: Initializing the 3 available platforms...
OpenCL: Initialize platform "Intel(R) OpenCL"'s device(s)
OpenCL: WARNING: Can't find a usable GPU!
OpenCL: Trying to set an context on Intel(R) Core(TM)2 Quad CPU    Q9500  @ 2.83GHz (id = 0)... Success!
OpenCL: Initialize platform "NVIDIA CUDA"'s device(s)
OpenCL: WARNING: Can't find a usable CPU!
OpenCL: Trying to set an context on GeForce GT 220 (id = 0)... Success!
OpenCL: Initialize platform "AMD Accelerated Parallel Processing"'s device(s)
OpenCL: WARNING: Can't find a usable GPU!
OpenCL: Trying to set an context on Intel(R) Core(TM)2 Quad CPU    Q9500  @ 2.83GHz (id = 0)... Success!
OpenCL: Available platforms:
    Platform information:
        vendor:     Advanced Micro Devices, Inc.
        name:       AMD Accelerated Parallel Processing
        version:    OpenCL 1.1 AMD-APP-SDK-v2.4 (595.10)
        extensions: cl_khr_icd cl_amd_event_callback cl_amd_offline_devices
        id:         0x7f5f75fcc800
        profile:    FULL_PROFILE
        key:        amd
        list:       0x7fffd5fd7490
    Available OpenCL devices on platform:
    ---------------------------------------------------------------------------------------------------------
    name: Intel(R) Core(TM)2 Quad CPU    Q9500  @ 2.83GHz
        id:                             0
        parent platform:                AMD Accelerated Parallel Processing
        device_is_used:                 yes
        max_compute_unit:               4
        device is GPU?                  no
        address_bits:                   64
        available:                      yes
        compiler_available:             yes
        endian_little:                  yes
        error_correction_support:       no
        execution_capabilities:         3
        global_mem_cache_size:          32768 bytes (32 KiB, 0.03125 MiB, 3.05176e-05 GiB)
        global_mem_cache_type:          2
        global_mem_cacheline_size:      64 bytes (0.0625 KiB, 6.10352e-05 MiB, 5.96046e-08 GiB)
        global_mem_size:                4158504960 bytes (4.06104e+06 KiB, 3965.86 MiB, 3.87291 GiB)
        image_support:                  yes
        image2d_max_height:             8192
        image2d_max_width:              8192
        image3d_max_depth:              2048
        image3d_max_height:             2048
        image3d_max_width:              2048
        local_mem_size:                 32768 bytes (32 KiB, 0.03125 MiB, 3.05176e-05 GiB)
        local_mem_type:                 2
        max_clock_frequency:            2826 MHz
        max_compute_units:              4
        max_constant_args:              8
        max_constant_buffer_size:       65536 bytes (64 KiB, 0.0625 MiB, 6.10352e-05 GiB)
        max_mem_alloc_size:             2147483648 bytes (2.09715e+06 KiB, 2048 MiB, 2 GiB)
        max_parameter_size:             4096 bytes (4 KiB, 0.00390625 MiB, 3.8147e-06 GiB)
        max_read_image_args:            128
        max_samplers:                   16
        max_work_group_size:            1024 bytes (1 KiB, 0.000976562 MiB, 9.53674e-07 GiB)
        max_work_item_dimensions:       3
        max_work_item_sizes:            (1024, 1024, 1024)
        max_write_image_args:           8
        mem_base_addr_align:            1024
        min_data_type_align_size:       128 bytes (0.125 KiB, 0.00012207 MiB, 1.19209e-07 GiB)
        platform:                       0x7f5f75fcc800
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
        vendor:                         GenuineIntel
        version:                        OpenCL 1.1 AMD-APP-SDK-v2.4 (595.10)
        driver_version:                 2.0
        GPU is NOT from NVidia
        Available memory (global):   4158504960 bytes (4.06104e+06 KiB, 3965.86 MiB, 3.87291 GiB)
        Available memory (local):    32768 bytes (32 KiB, 0.03125 MiB, 3.05176e-05 GiB)
        Available memory (constant): 65536 bytes (64 KiB, 0.0625 MiB, 6.10352e-05 GiB)
        *****************************************************************************************************
        Order of preference for OpenCL devices for this platform:
        0.   Intel(R) Core(TM)2 Quad CPU    Q9500  @ 2.83GHz (id = 0)
        *****************************************************************************************************
    Platform information:
        vendor:     Intel(R) Corporation
        name:       Intel(R) OpenCL
        version:    OpenCL 1.1 LINUX
        extensions: cl_khr_fp64 cl_khr_global_int32_base_atomics cl_khr_global_int32_extended_atomics cl_khr_local_int32_base_atomics cl_khr_local_int32_extended_atomics cl_khr_gl_sharing cl_khr_byte_addressable_store cl_intel_printf cl_ext_device_fission cl_khr_icd
        id:         0x1414cf0
        profile:    FULL_PROFILE
        key:        intel
        list:       0x7fffd5fd7490
    Available OpenCL devices on platform:
    ---------------------------------------------------------------------------------------------------------
    name: Intel(R) Core(TM)2 Quad CPU    Q9500  @ 2.83GHz
        id:                             0
        parent platform:                Intel(R) OpenCL
        device_is_used:                 no
        max_compute_unit:               4
        device is GPU?                  no
        address_bits:                   64
        available:                      yes
        compiler_available:             yes
        endian_little:                  yes
        error_correction_support:       no
        execution_capabilities:         3
        global_mem_cache_size:          3145728 bytes (3072 KiB, 3 MiB, 0.00292969 GiB)
        global_mem_cache_type:          2
        global_mem_cacheline_size:      64 bytes (0.0625 KiB, 6.10352e-05 MiB, 5.96046e-08 GiB)
        global_mem_size:                4158504960 bytes (4.06104e+06 KiB, 3965.86 MiB, 3.87291 GiB)
        image_support:                  yes
        image2d_max_height:             8192
        image2d_max_width:              8192
        image3d_max_depth:              2048
        image3d_max_height:             2048
        image3d_max_width:              2048
        local_mem_size:                 32768 bytes (32 KiB, 0.03125 MiB, 3.05176e-05 GiB)
        local_mem_type:                 2
        max_clock_frequency:            2830 MHz
        max_compute_units:              4
        max_constant_args:              128
        max_constant_buffer_size:       131072 bytes (128 KiB, 0.125 MiB, 0.00012207 GiB)
        max_mem_alloc_size:             1039626240 bytes (1.01526e+06 KiB, 991.465 MiB, 0.968227 GiB)
        max_parameter_size:             1024 bytes (1 KiB, 0.000976562 MiB, 9.53674e-07 GiB)
        max_read_image_args:            128
        max_samplers:                   128
        max_work_group_size:            1024 bytes (1 KiB, 0.000976562 MiB, 9.53674e-07 GiB)
        max_work_item_dimensions:       3
        max_work_item_sizes:            (1024, 1024, 1024)
        max_write_image_args:           128
        mem_base_addr_align:            1024
        min_data_type_align_size:       128 bytes (0.125 KiB, 0.00012207 MiB, 1.19209e-07 GiB)
        platform:                       0x1414cf0
        preferred_vector_width_char:    16
        preferred_vector_width_short:   8
        preferred_vector_width_int:     4
        preferred_vector_width_long:    2
        preferred_vector_width_float:   4
        preferred_vector_width_double:  2
        profiling_timer_resolution:     353857 ns
        queue_properties:               CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, CL_QUEUE_PROFILING_ENABLE,  (3)
        single_fp_config:               CL_FP_DENORM, CL_FP_INF_NAN, CL_FP_ROUND_TO_NEAREST,  (7)
        type:                           CL_DEVICE_TYPE_CPU (2)
        vendor_id:                      32902
        extensions:                     cl_khr_fp64 cl_khr_global_int32_base_atomics cl_khr_global_int32_extended_atomics cl_khr_local_int32_base_atomics cl_khr_local_int32_extended_atomics cl_khr_gl_sharing cl_khr_byte_addressable_store cl_intel_printf cl_ext_device_fission
        profile:                        FULL_PROFILE
        vendor:                         Intel(R) Corporation
        version:                        OpenCL 1.1 (Build 13785.5219)
        driver_version:                 1.1
        GPU is NOT from NVidia
        Available memory (global):   4158504960 bytes (4.06104e+06 KiB, 3965.86 MiB, 3.87291 GiB)
        Available memory (local):    32768 bytes (32 KiB, 0.03125 MiB, 3.05176e-05 GiB)
        Available memory (constant): 131072 bytes (128 KiB, 0.125 MiB, 0.00012207 GiB)
        *****************************************************************************************************
        Order of preference for OpenCL devices for this platform:
        0.   Intel(R) Core(TM)2 Quad CPU    Q9500  @ 2.83GHz (id = 0)
        *****************************************************************************************************
    Platform information:
        vendor:     NVIDIA Corporation
        name:       NVIDIA CUDA
        version:    OpenCL 1.0 CUDA 4.0.1
        extensions: cl_khr_byte_addressable_store cl_khr_icd cl_khr_gl_sharing cl_nv_compiler_options cl_nv_device_attribute_query cl_nv_pragma_unroll
        id:         0x3ddd960
        profile:    FULL_PROFILE
        key:        nvidia
        list:       0x7fffd5fd7490
    Available OpenCL devices on platform:
    ---------------------------------------------------------------------------------------------------------
    name: GeForce GT 220
        id:                             0
        parent platform:                NVIDIA CUDA
        device_is_used:                 no
        max_compute_unit:               6
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
        global_mem_size:                1073020928 bytes (1.04787e+06 KiB, 1023.31 MiB, 0.999329 GiB)
        image_support:                  yes
        image2d_max_height:             32768
        image2d_max_width:              4096
        image3d_max_depth:              2048
        image3d_max_height:             2048
        image3d_max_width:              2048
        local_mem_size:                 16384 bytes (16 KiB, 0.015625 MiB, 1.52588e-05 GiB)
        local_mem_type:                 1
        max_clock_frequency:            1360 MHz
        max_compute_units:              6
        max_constant_args:              9
        max_constant_buffer_size:       65536 bytes (64 KiB, 0.0625 MiB, 6.10352e-05 GiB)
        max_mem_alloc_size:             268255232 bytes (261968 KiB, 255.828 MiB, 0.249832 GiB)
        max_parameter_size:             4352 bytes (4.25 KiB, 0.00415039 MiB, 4.05312e-06 GiB)
        max_read_image_args:            128
        max_samplers:                   16
        max_work_group_size:            512 bytes (0.5 KiB, 0.000488281 MiB, 4.76837e-07 GiB)
        max_work_item_dimensions:       3
        max_work_item_sizes:            (512, 512, 64)
        max_write_image_args:           8
        mem_base_addr_align:            2048
        min_data_type_align_size:       128 bytes (0.125 KiB, 0.00012207 MiB, 1.19209e-07 GiB)
        platform:                       0x3ddd960
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
        driver_version:                 275.09.07
        GPU is from NVidia
            nvidia_device_compute_capability_major: 1
            nvidia_device_compute_capability_minor: 2
            nvidia_device_registers_per_block:      16384
            nvidia_device_warp_size:                32
            nvidia_device_gpu_overlap:              yes
            nvidia_device_kernel_exec_timeout:      yes
            nvidia_device_integrated_memory:        no
        Available memory (global):   1073020928 bytes (1.04787e+06 KiB, 1023.31 MiB, 0.999329 GiB)
        Available memory (local):    16384 bytes (16 KiB, 0.015625 MiB, 1.52588e-05 GiB)
        Available memory (constant): 65536 bytes (64 KiB, 0.0625 MiB, 6.10352e-05 GiB)
        *****************************************************************************************************
        Order of preference for OpenCL devices for this platform:
        0.   GeForce GT 220 (id = 0)
        *****************************************************************************************************
-------------------------------------------------------------------------------------------------------------
OpenCL: Prefered platform's name:          AMD Accelerated Parallel Processing
OpenCL: Prefered platform's best device:   Intel(R) Core(TM)2 Quad CPU    Q9500  @ 2.83GHz
-------------------------------------------------------------------------------------------------------------
```

