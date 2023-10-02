#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>

#define PRINT 1

int SZ = 8;
int *v;

// bufV is the memory buffer used by the OpenCL runtime for the kernel programs
// In this program the data in vector v is copied into bufV as part of the initialisation in setup_kernel_memory()
cl_mem bufV;

// device_id is an OpenCL identifier for a compute device (e.g. CPU, GPU)
cl_device_id device_id;

// context is an OpenCL identifier for the selected context on the host (GPU > CPU)
cl_context context;

// program is an object containing the compiled OpenCL C program, which is vector_ops.cl in our case.
cl_program program;

// kernel is a function (i.e. kernel) which is executed by the OpenCL runtime.
// kernel object contains a specific kernel function (e.g. square_magnitude) declared in the vector_ops.cl program
// along with the parameters for the kernel function.
cl_kernel kernel;

// queue is an OpenCL command queue object. queue is used for sending operations to a host or device within
// a context. Operations in a queue are executed sequentially.
cl_command_queue queue;
cl_event event = NULL;

int err;

// create_device() returns an OpenGL device 
cl_device_id create_device();

// The purpose of this function is to configure and initialise an OpenCL Context, Queue and Kernel object
void setup_openCL_device_context_queue_kernel(char *filename, char *kernelname);

// build_program reads the kernel code (vector_ops.cl), compiles it and returns an executable program
cl_program build_program(cl_context ctx, cl_device_id dev, const char *filename);

// This function allocates memory (i.e. creates a buffer) before copying
// the initialised vector data into an OpenCL buffer (bufV)
void setup_kernel_memory();

// copy_kernel_args() configures the kernel arguments by parameter index
// These being the buffer size, followed by the buffer
void copy_kernel_args();

// free_memory() is responsible for releasing resources once the program has completed
void free_memory();

void init(int *&A, int size);
void print(int *A, int size);

int main(int argc, char **argv)
{
    if (argc > 1)
        SZ = atoi(argv[1]);

    init(v, SZ);


    // global is an array of size 1, containing the number of work-items to be executed by OpenCL
    size_t global[1] = {(size_t)SZ};

    //initial vector
    print(v, SZ);

    setup_openCL_device_context_queue_kernel((char *)"./vector_ops.cl", (char *)"square_magnitude");

    setup_kernel_memory();
    copy_kernel_args();

    // clEnqueueNDRangeKernel pushes a kernel onto the command queue to be executed by a device
    // Its arguments are:
    // cl_command_queue command_queue (queue to push the kernel onto)
    // cl_kernel kernel (kernel containing the program instructions and parameters)
    // cl_uint work_dim - the number of dimensions in the global work group
    // const size_t *global_work_offset - offset/start buffer index
    // const size_t *global_work_size - size of
    // const size_t *local_work_size
    // cl_uint num_events_in_wait_list
    // const cl_event *event_wait_list
    // cl_event *event
    clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global, NULL, 0, NULL, &event);
    clWaitForEvents(1, &event);

    // todo
    clEnqueueReadBuffer(queue, bufV, CL_TRUE, 0, SZ * sizeof(int), &v[0], 0, NULL, NULL);

    //result vector
    print(v, SZ);

    //frees memory for device, kernel, queue, etc.
    //you will need to modify this to free your own buffers
    free_memory();
}

void init(int *&A, int size)
{
    A = (int *)malloc(sizeof(int) * size);

    for (long i = 0; i < size; i++)
    {
        A[i] = rand() % 100; // any number less than 100
    }
}

void print(int *A, int size)
{
    if (PRINT == 0)
    {
        return;
    }

    if (PRINT == 1 && size > 15)
    {
        for (long i = 0; i < 5; i++)
        {                        //rows
            printf("%d ", A[i]); // print the cell value
        }
        printf(" ..... ");
        for (long i = size - 5; i < size; i++)
        {                        //rows
            printf("%d ", A[i]); // print the cell value
        }
    }
    else
    {
        for (long i = 0; i < size; i++)
        {                        //rows
            printf("%d ", A[i]); // print the cell value
        }
    }
    printf("\n----------------------------\n");
}

void free_memory()
{
    //free the buffers
    clReleaseMemObject(bufV);

    //free opencl objects
    clReleaseKernel(kernel);
    clReleaseCommandQueue(queue);
    clReleaseProgram(program);
    clReleaseContext(context);

    free(v);
}


void copy_kernel_args()
{
    // clSetKernelArg is used to set a value for a kernel (i.e. function) parameter at a specific index.
    // The first argument to clSetKernelArg is the kernel
    // The second argument is the specific index of the kernel parameter
    // The third argument is the data size of the parameter
    // The fourth argument is a generic pointer cast to the data used by the kernel parameter
    clSetKernelArg(kernel, 0, sizeof(int), (void *)&SZ);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&bufV);

    if (err < 0)
    {
        perror("Couldn't create a kernel argument");
        printf("error = %d", err);
        exit(1);
    }
}

void setup_kernel_memory()
{
    // The clCreateBuffer function creates a buffer object to be used by the OpenCL runtime
    // The parameters are a context (context), memory access flags (flags), buffer size to allocate (size),
    // optional pointer to an existing buffer already allocated (host_ptr), and a return error code
    // The second parameter of the clCreateBuffer is cl_mem_flags flags. Check the OpenCL documention to find out what is it's purpose and read the List of supported memory flag values
    // Supported memory flags:
    // CL_MEM_READ_WRITE (Read and write permission, default setting)
    // CL_MEM_WRITE_ONLY (Write only permission, reading results in undefined behaviour)
    // CL_MEM_READ_ONLY (Read only permission, writing results in undefined behaviour)
    // R/W, WO, RO are mutually exclusive options
    // CL_MEM_USE_HOST_PTR (If host_ptr is not null, tell OpenCL runtime to use memory at host_ptr)
    // CL_MEM_ALLOC_HOST_PTR (Allocate memory to host_ptr using host memory)
    // CL_MEM_COPY_HOST_PTR (If host_ptr is not null, copy the buffer at host_ptr and return a new pointer)
    // CL_MEM_HOST_WRITE_ONLY (Tell the host that it can only write to the memory object)
    // CL_MEM_HOST_READ_ONLY (Tell host that the memory object is read only)
    // CL_MEM_HOST_NO_ACCESS (Tell host to not read or write the memory object)
    bufV = clCreateBuffer(context, CL_MEM_READ_WRITE, SZ * sizeof(int), NULL, NULL);

    // Copy matrices to the GPU
    clEnqueueWriteBuffer(queue, bufV, CL_TRUE, 0, SZ * sizeof(int), &v[0], 0, NULL, NULL);
}

void setup_openCL_device_context_queue_kernel(char *filename, char *kernelname)
{
    device_id = create_device();
    cl_int err;

    // clCreateContext creates an OpenCL context using one or more devices found within a platform
    // The platform in this case being the first found GPU
    // In this instance we are creating a context using a single device ID
    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);
    if (err < 0)
    {
        perror("Couldn't create a context");
        exit(1);
    }

    program = build_program(context, device_id, filename);

    // clCreateCommandQueueWithProperties simply creates a Command Queue within an existing context for a specific device
    // using an optional set of properties
    queue = clCreateCommandQueueWithProperties(context, device_id, 0, &err);
    if (err < 0)
    {
        perror("Couldn't create a command queue");
        exit(1);
    };


    kernel = clCreateKernel(program, kernelname, &err);
    if (err < 0)
    {
        perror("Couldn't create a kernel");
        printf("error =%d", err);
        exit(1);
    };
}

cl_program build_program(cl_context ctx, cl_device_id dev, const char *filename)
{

    cl_program program;
    FILE *program_handle;
    char *program_buffer, *program_log;
    size_t program_size, log_size;

    /* Read program file and place content into buffer */
    program_handle = fopen(filename, "r");
    if (program_handle == NULL)
    {
        perror("Couldn't find the program file");
        exit(1);
    }
    fseek(program_handle, 0, SEEK_END);
    program_size = ftell(program_handle);
    rewind(program_handle);
    program_buffer = (char *)malloc(program_size + 1);
    program_buffer[program_size] = '\0';
    fread(program_buffer, sizeof(char), program_size, program_handle);
    fclose(program_handle);

    // clCreateProgramWithSource(context, count, strings, lengths, errcode_ret);
    // clCreateProgramWithSource creates and returns a cl_program object for a context.
    // The function parameters are: 
    // context: OpenGL context,
    // count: the number of strings (null terminated),
    // strings: Array of strings containing source code
    // lengths: An array of each string length
    // errcode_ret: and a return error code
    program = clCreateProgramWithSource(ctx, 1,
                                        (const char **)&program_buffer, &program_size, &err);
    if (err < 0)
    {
        perror("Couldn't create the program");
        exit(1);
    }
    free(program_buffer);

    /* Build program 

   The fourth parameter accepts options that configure the compilation. 
   These are similar to the flags used by gcc. For example, you can 
   define a macro with the option -DMACRO=VALUE and turn off optimization 
   with -cl-opt-disable.
   */
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (err < 0)
    {

        /* Find size of log and print to std output */
        clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG,
                              0, NULL, &log_size);
        program_log = (char *)malloc(log_size + 1);
        program_log[log_size] = '\0';
        clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG,
                              log_size + 1, program_log, NULL);
        printf("%s\n", program_log);
        free(program_log);
        exit(1);
    }

    return program;
}

cl_device_id create_device() {

   cl_platform_id platform;
   cl_device_id dev;
   int err;

   /* Identify a platform */
   err = clGetPlatformIDs(1, &platform, NULL);
   if(err < 0) {
      perror("Couldn't identify a platform");
      exit(1);
   } 

   // Access a device
   // GPU
   err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
   if(err == CL_DEVICE_NOT_FOUND) {
      // CPU
      printf("GPU not found\n");
      err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
   }
   if(err < 0) {
      perror("Couldn't access any devices");
      exit(1);   
   }

   return dev;
}