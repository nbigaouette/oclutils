
#include <stdio.h>
#include <stdlib.h> // abort()
#include <string>   // std::string
#include <cstdarg>  // va_arg, va_list, etc.
#include <cmath>

#include <Memory.hpp> // Print_N_Times()

#include "OclUtils.hpp"

#define TMP_FILE "/tmp/gpu_usage.txt"


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
    *length = int(ftell(f));
    fseek(f, 0, SEEK_SET);

    buffer = malloc(*length+1);
    *length = int(fread(buffer, 1, *length, f));
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

    device_is_used = false;
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

    std::ifstream file(TMP_FILE, std::ios::in);

    if(file)
    {
        std::string line;

        while(std::getline(file, line))
        {
            if(line.find("Device = " + id) != std::string::npos)
            {
                device_is_used = true;
            }
        }

        file.close();
    }
    else
    {
        device_is_used = false;
    }

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
cl_int OpenCL_device::Set_Context()
{
    cl_int err;
    context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    return err;
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
    // Start by checking if ones not in use. When this is the case give it priority.
    // Then compare the maximum number of compute unit
    // NOTE: We want a sorted list where device with higher compute units
    //       are located at the top (front). We thus invert the test here.
    bool result = false;

    if(this->device_is_used == false && b.device_is_used == true) // this wins (it is not in use).
        result = true;
    else if(this->device_is_used == true && b.device_is_used == false) // b wins (it is not in use).
        result = false;
    else // both are used or not used. Thus, we must compare the ammount of compute units.
    {
        if(this->max_compute_unit > b.max_compute_unit) // this wins (having more compute units).
            result = true;
        else                                            // b wins (having more or equal compute units).
            result = false;
    }

    return result;
}

// *****************************************************************************
OpenCL_devices_list::OpenCL_devices_list()
{
    nb_cpu  = 0;
    nb_gpu  = 0;
    err     = 0;
    preferred_device = NULL;
    write_to_tmp = true;
}

// *****************************************************************************
OpenCL_devices_list::~OpenCL_devices_list()
{
    if(write_to_tmp)
    {
        std::string file_content; // Write the data from the file.

        std::ifstream file_read(TMP_FILE, std::ios::in);

        if(file_read)
        {
            std::string line;

            while(std::getline(file_read, line))
            {
                // Read every line except the one corresponding to the current device.
                if(line.find("Device = " + preferred_device->Get_Id()) == std::string::npos)
                {
                    file_content += line + "\n"; // Add the lines to the string.
                }
            }

            file_read.close();
        }

        // Write back the string to file (the current device being deleted).
        std::ofstream file_write(TMP_FILE, std::ios::out | std::ios::trunc);

        if(file_write)
        {
            file_write << file_content;

            file_write.close();
        }
    }
}

// *****************************************************************************
void OpenCL_devices_list::Print()
{
    for (it = device_list.begin() ; it != device_list.end() ; ++it)
        it->Print();

    Print_N_Times("*", 109);
    std_cout << "Order of preference for OpenCL devices:\n";
    int i = 0;
    for (it = device_list.begin() ; it != device_list.end() ; ++it)
    {
        std_cout << i++ << ".   " << it->Get_Name() << "\n";
    }
    Print_N_Times("*", 109);
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

    bool is_all_devices_in_use = true; // We want to know if all devices are in use.

    // Add CPUs to list
    if (nb_cpu >= 1)
    {
        tmp_devices = new cl_device_id[nb_cpu];
        err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_CPU, nb_cpu, tmp_devices, NULL);
        OpenCL_Test_Success(err, "clGetDeviceIDs()");

        for (unsigned int i = 0 ; i < nb_cpu ; ++i, ++it)
        {
            it->Set_Information(i, tmp_devices[i], false); // device_is_gpu == false

            // When one device is not in use... One device is not in use!
            if(!it->Is_In_Use())
                is_all_devices_in_use = false;

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

            // When one device is not in use... One device is not in use!
            if(!it->Is_In_Use())
                is_all_devices_in_use = false;
        }
        delete[] tmp_devices;
    }

    assert(it == device_list.end());

    // When all devices are in use we ask the user if he would like
    // his simulation to be forced.
    if(is_all_devices_in_use == true)
    {
        bool correct_answer = false;

        // Don't write to tmp. This would suppress lines created by other program running.
        write_to_tmp = false;

        while(!correct_answer)
        {
            // Ask the user if he still wants to execute the program.
            std_cout << "It seem's that all devices are in use. Do you want to continue?\n";
            std::string answer;
            std::cin >> answer;

            if(answer == "yes" || answer == "Y" || answer == "y")
            {
                correct_answer = true;
                std_cout << "Proceeding... \n";
            }
            else if(answer == "no" || answer == "N" || answer == "n")
            {
                correct_answer = true;
                std_cout << "Exiting... \n";
                abort();
            }
            else
            {
                correct_answer = false;
                std_cout << "You entered an invalid answer!\n";
            }
        }
    }

    // Sort the list. The order is defined by "OpenCL_device::operator<" (line 112)
    device_list.sort();

    Print();

    // Initialize context on a device
    preferred_device = NULL;    // The preferred device is unknown for now.
    for (it = device_list.begin() ; it != device_list.end() ; ++it)
    {
        std_cout << "Trying to set an OpenCL context on " << it->Get_Name() << "...";
        if (it->Set_Context() == CL_SUCCESS)
        {
            std_cout << " Success!\n";
            preferred_device = &(*it);

            if(write_to_tmp)
            {
                std::ofstream file(TMP_FILE, std::ios::out | std::ios::app);

                if(file)
                {
                    file << "Device = " << preferred_device->Get_Id() << std::endl;

                    file.close();
                }
            }

            break;
        }
        else
        {
            std_cout << " Failed. Maybe next one will work?\n";
        }
    }
    if (preferred_device == NULL)
    {
        std_cout << "ERROR: Cannot set an OpenCL context on any of the available devices!\nExiting" << std::flush;
        abort();
    }
}


// *****************************************************************************

OpenCL_Kernel::OpenCL_Kernel(std::string _filename, bool _use_mt, cl_context _context, cl_device_id _device_id):
                           filename(_filename), use_mt(_use_mt), context(_context), device_id(_device_id)
{
}


// *****************************************************************************
OpenCL_Kernel::~OpenCL_Kernel()
{
    if (kernel)  clReleaseKernel(kernel);
    if (program) clReleaseProgram(program);

    delete[] global_work_size;
    delete[] local_work_size;
}


// *****************************************************************************
void OpenCL_Kernel::Build(std::string _kernel_name, std::string _compiler_options)
{
    compiler_options = _compiler_options;
    kernel_name      = _kernel_name;

    // **********************************************************
    // Load and build the kernel
    Load_Program_From_File();

    // Create the kernel.
    kernel = clCreateKernel(program, kernel_name.c_str(), &err);
    OpenCL_Test_Success(err, "clCreateKernel");

    // **********************************************************
    // Get the maximum work group size
    //err = clGetKernelWorkGroupInfo(kernel_md, list.Prefered_OpenCL_Device(), CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &maxWorkSize, NULL);
    //std_cout << "Maximum kernel work group size: " << maxWorkSize << "\n";
    //OpenCL_Test_Success(err, "clGetKernelWorkGroupInfo");
}


// *****************************************************************************
void OpenCL_Kernel::Compute_Work_Size(int N)
/**
 * When this method is used the local work size is choosed by OpenCL.
 * Thus, the global work size is set to N.
 * Don't use this function if using MT.
 */
{
    dimension = 1;

    global_work_size = new size_t[dimension];
    local_work_size = new size_t[dimension];

    global_work_size[0] = N;
    local_work_size = NULL;
}


// *****************************************************************************
void OpenCL_Kernel::Compute_Work_Size(int N, int _p, int _q)
/**
 * Use this method for a kernel who benefits from MT.
 */
{
    dimension = 2;

    global_work_size = new size_t[dimension];
    local_work_size = new size_t[dimension];

    if(_p * _q > MAX_LOCAL_WORK_SIZE)
    {
        _p = MAX_LOCAL_WORK_SIZE / _q;
    }
    else if(_q == 1 && N < _p)
    {
        _p = N;
    }

    local_work_size[0] = _p;
    local_work_size[1] = _q;

    global_work_size[0] = Get_Multiple_Of_Work_Size(N, _p*_q);
    global_work_size[1] = _q;

    p = _p;
    q = _q;

    //std_cout << "Number of dimension of problem space: " << dimension << "\n";
    //std_cout << "global_workGroupSize[" << 0 << "]: " << globalWorkSizes[id][0] << " \n";
    //for(int d = 0 ; d < dimension ; d++)
    //    "local_workGroupSize[" << d << "]: " << localWorkSizes[id][d] << "\n";
}


// *****************************************************************************
cl_kernel OpenCL_Kernel::Get_Kernel() const
{
    return kernel;
}


// *****************************************************************************
size_t *OpenCL_Kernel::Get_Global_Work_Size() const
{
    return global_work_size;
}


// *****************************************************************************
size_t *OpenCL_Kernel::Get_Local_Work_Size() const
{
    return local_work_size;
}


// *****************************************************************************
int OpenCL_Kernel::Get_Dimension() const
{
    return dimension;
}


// *****************************************************************************
bool OpenCL_Kernel::Uses_MT() const
{
    return use_mt;
}


// *****************************************************************************
void OpenCL_Kernel::Load_Program_From_File()
{
    // Program Setup
    int pl;
    size_t program_length;
    std_cout << "Loading OpenCL program from \"" << filename << "\"...\n";

    // Loads the contents of the file at the given path
    char* cSourceCL = read_opencl_kernel(filename, &pl);
    program_length = (size_t) pl;

    // create the program
    program = clCreateProgramWithSource(context, 1, (const char **) &cSourceCL, &program_length, &err);
    OpenCL_Test_Success(err, "clCreateProgramWithSource");

    Build_Executable();
}


// *****************************************************************************
void OpenCL_Kernel::Build_Executable()
/**
 * Build the program executable
 */
{
    std_cout << "Building the program..." << std::flush;

#ifdef YDEBUG
    // Include debugging symbols in kernel compilation
#ifndef MACOSX
    compiler_options += "-g ";
#endif // #ifndef MACOSX
#endif // #ifdef YDEBUG
    // Verbose compilation? Does not do much... And it may break kernel compilation
    // with invalid kernel name error.
    //OpenCLCompilerOptions += "-cl-nv-verbose";

    std_cout << "\nOpenCL Compiler Options: " << compiler_options.c_str() << "\n" << std::flush;

    err = clBuildProgram(program, 0, NULL, compiler_options.c_str(), NULL, NULL);

    char *build_log;
    size_t ret_val_size;
    err = clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &ret_val_size);
    build_log = new char[ret_val_size+1];
    err = clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, ret_val_size, build_log, NULL);
    build_log[ret_val_size] = '\0';
    OpenCL_Test_Success(err, "1. clGetProgramBuildInfo");
    std_cout << "OpenCL kernels file compilation log: \n" << build_log << "\n";

    if (err != CL_SUCCESS)
    {
        cl_build_status build_status;
        err = clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &build_status, NULL);
        OpenCL_Test_Success(err, "0. clGetProgramBuildInfo");

        //char *build_log;
        //size_t ret_val_size;
        err = clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &ret_val_size);
        OpenCL_Test_Success(err, "1. clGetProgramBuildInfo");
        //build_log = new char[ret_val_size+1];
        err = clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, ret_val_size, build_log, NULL);
        build_log[ret_val_size] = '\0';

        OpenCL_Test_Success(err, "2. clGetProgramBuildInfo");
        std_cout << "Build log: \n" << build_log << "\n";
        std_cout << "Kernel did not built correctly. Exiting.\n";

        std_cout << std::flush;
        abort();
    }

    delete[] build_log;

    std_cout << "done.\n";
}



// *****************************************************************************
int OpenCL_Kernel::Get_Multiple_Of_Work_Size(int n, int _p)
{
    int multipleOfWorkSize = 0;

    if (n < _p)
    {
        multipleOfWorkSize = n;
    }
    else if (n % _p == 0)
    {
        multipleOfWorkSize = n;
    }
    else
    {
        multipleOfWorkSize = _p*std::floor(n/_p) + _p;
    }

    return multipleOfWorkSize;
}

// ********** End of file ******************************************************
