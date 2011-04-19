
#include <stdio.h>
#include <stdlib.h> // abort()
#include <string>   // std::string
#include <cstdarg>  // va_arg, va_list, etc.

#include <Memory.hpp> // Print_N_Times()

#include "OclUtils.hpp"


// *****************************************************************************
char *read_opencl_kernel(const std::string filename, int *length)
{
    FILE *f = fopen(filename.c_str(), "r");
    void *buffer;

    if (!f) {
        std_cout << "Unable to open " << filename << " for reading\n";
        abort();
    }

    fseek(f, 0, SEEK_END);
    *length = ftell(f);
    fseek(f, 0, SEEK_SET);

    buffer = malloc(*length+1);
    *length = fread(buffer, 1, *length, f);
    fclose(f);
    ((char*)buffer)[*length] = '\0';

    return (char*)buffer;
}



// *****************************************************************************
OpenCL_device::OpenCL_device()
{
    name                        = "";
    id                          = -1;
    device_is_gpu               = false;
    max_compute_unit            = 0;
    available_memory_global     = 0;
    available_memory_local      = 0;
    available_memory_constant   = 0;

    device  = NULL;
    context = NULL;
}

// *****************************************************************************
OpenCL_device::~OpenCL_device()
{
    if (context)
        clReleaseContext(context);
}

// *****************************************************************************
void OpenCL_device::Set_Information(const int _id, cl_device_id _device, const bool _device_is_gpu)
{
    id              = _id;
    device          = _device;
    device_is_gpu   = _device_is_gpu;

    char tmp_name[4096];

    cl_int err;
    err  = clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS,         sizeof(cl_uint),  &max_compute_unit, NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE,           sizeof(cl_ulong), &available_memory_global, NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE,            sizeof(cl_ulong), &available_memory_local,  NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE,  sizeof(cl_ulong), &available_memory_constant,  NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_NAME,                      sizeof(tmp_name), &tmp_name, NULL);
    name = std::string(tmp_name);

    OpenCL_Test_Success(err, "OpenCL_device::Set_Information()");
}

// *****************************************************************************
void OpenCL_device::Set_Context()
{
    int err;
    context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    if (err == CL_DEVICE_NOT_FOUND)
    {
        std_cout << "WARNING: Can't find an OpenCL device!\n" << std::flush;
        abort();
    }
    OpenCL_Test_Success(err, "OpenCL_device::Set_Context()");
}

// *****************************************************************************
void OpenCL_device::Print()
{
    Print_N_Times("-", 109);

    std_cout
        << "Available OpenCL device:\n"
        << "    name: " << name << "\n"
        << "    id:   " << id << "\n"
        << "    max_compute_unit: " << max_compute_unit << "\n"
        << "    device is GPU? " << (device_is_gpu ? "yes" : "no ") << "\n";


    std_cout << "oclPrintDevInfo():\n";
    oclPrintDevInfo(device);
    std_cout << "Device capability (only nvidia): " << oclGetDevCap(device) << "\n";

    // Avialable global memory on device
    std_cout.Format(0, 3, 'g');
    std_cout
        << "Available memory (global):   "
        << available_memory_global << " bytes, "
        << Bytes_to_KiBytes(available_memory_global) << " KiB, "
        << Bytes_to_MiBytes(available_memory_global) << " MiB, "
        << Bytes_to_GiBytes(available_memory_global) << " GiB\n";

    // Avialable local memory on device
    std_cout.Format(0, 3, 'g');
    std_cout
        << "Available memory (local):    "
        << available_memory_local << " bytes, "
        << Bytes_to_KiBytes(available_memory_local) << " KiB, "
        << Bytes_to_MiBytes(available_memory_local) << " MiB, "
        << Bytes_to_GiBytes(available_memory_local) << " GiB\n";

    // Avialable constant memory on device
    std_cout.Format(0, 3, 'g');
    std_cout
        << "Available memory (constant): "
        << available_memory_constant << " bytes, "
        << Bytes_to_KiBytes(available_memory_constant) << " KiB, "
        << Bytes_to_MiBytes(available_memory_constant) << " MiB, "
        << Bytes_to_GiBytes(available_memory_constant) << " GiB\n";
}

// *****************************************************************************
bool OpenCL_device::operator<(const OpenCL_device &b)
{
    // Compare the maximum number of compute unit
    return (this->max_compute_unit < b.max_compute_unit) ? true : false;
}


// *****************************************************************************
OpenCL_devices_list::OpenCL_devices_list()
{
    nb_cpu  = 0;
    nb_gpu  = 0;
    err     = 0;
}

// *****************************************************************************
OpenCL_devices_list::~OpenCL_devices_list()
{
}

void OpenCL_devices_list::Print()
{
    for (it = device_list.begin() ; it != device_list.end() ; ++it)
        it->Print();
}

// *****************************************************************************
void OpenCL_devices_list::Initialize()
{
    std_cout << "Initialize OpenCL object and context\n" << std::flush;

    err = oclGetPlatformID(&platform_id);
    OpenCL_Test_Success(err, "oclGetPlatformID");

    // Get the number of GPU devices available to the platform
    // Number of GPU
    err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 0, NULL, &nb_gpu);
    if (err == CL_DEVICE_NOT_FOUND)
    {
        std_cout << "WARNING: Can't find a usable GPU!\n" << std::flush;
        err = CL_SUCCESS;
    }
    OpenCL_Test_Success(err, "clGetDeviceIDs()");

    // Number of CPU
    err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_CPU, 0, NULL, &nb_cpu);
    if (err == CL_DEVICE_NOT_FOUND)
    {
        std_cout << "WARNING: Can't find a usable CPU!\n" << std::flush;
        err = CL_SUCCESS;
    }
    OpenCL_Test_Success(err, "clGetDeviceIDs()");
    assert(nb_devices() >= 1);

    // Create the device list
    device_list.resize(nb_devices());
    // Temporary list
    cl_device_id *tmp_devices;

    it = device_list.begin();

    // Add CPUs to list
    if (nb_cpu >= 1)
    {
        tmp_devices = new cl_device_id[nb_cpu];
        err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_CPU, nb_cpu, tmp_devices, NULL);
        OpenCL_Test_Success(err, "clGetDeviceIDs()");

        for (unsigned int i = 0 ; i < nb_cpu ; ++i, ++it)
        {
            it->Set_Information(i, tmp_devices[i], false); // device_is_gpu == false
        }
        delete[] tmp_devices;
    }

    // Add GPUs to list
    if (nb_gpu >= 1)
    {
        tmp_devices = new cl_device_id[nb_gpu];
        err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, nb_gpu, tmp_devices, NULL);
        OpenCL_Test_Success(err, "clGetDeviceIDs()");
        for (unsigned int i = 0 ; i < nb_gpu ; ++i, ++it)
        {
            it->Set_Information(nb_cpu+i, tmp_devices[i], true); // device_is_gpu == true
        }
        delete[] tmp_devices;
    }

    assert(it == device_list.end());

    // Sort the list. Sorting is define as "OpenCL_device::operator<" (line 112)
    device_list.sort();

    // Initialize context on preferred device's
    Prefered_OpenCL().Set_Context();

    Print();
}

