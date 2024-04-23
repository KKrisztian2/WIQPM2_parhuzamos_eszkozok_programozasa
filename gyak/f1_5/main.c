#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <CL/cl.h>

#define MAX_SOURCE_SIZE (0x100000)

void replaceMissing(int* input, int size) {
    // Load the kernel source code
    FILE *fp;
    char *source_str;
    size_t source_size;

    fp = fopen("kernels/hello_kernel.cl", "r");
    if (!fp) {
        fprintf(stderr, "Failed to load kernel.\n");
        exit(1);
    }
    source_str = (char*)malloc(MAX_SOURCE_SIZE);
    source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose(fp);

    cl_int err;

    // Get platform
    cl_uint n_platforms;
    cl_platform_id platform_id;
    err = clGetPlatformIDs(1, &platform_id, &n_platforms);
    if (err != CL_SUCCESS) {
        printf("[ERROR] Error calling clGetPlatformIDs. Error code: %d\n", err);
        return;
    }

    // Get device
    cl_device_id device_id;
    cl_uint n_devices;
    err = clGetDeviceIDs(
        platform_id,
        CL_DEVICE_TYPE_GPU,
        1,
        &device_id,
        &n_devices
    );
    if (err != CL_SUCCESS) {
        printf("[ERROR] Error calling clGetDeviceIDs. Error code: %d\n", err);
        return;
    }

    // Create OpenCL context
    cl_context context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);
    if (!context || err != CL_SUCCESS) {
        printf("[ERROR] Error creating OpenCL context. Error code: %d\n", err);
        return;
    }

    // Build the program
    cl_program program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &err);
    if (!program || err != CL_SUCCESS) {
        printf("[ERROR] Error creating OpenCL program. Error code: %d\n", err);
        return;
    }
    err = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
        printf("Build error! Code: %d\n", err);
        size_t len;
        char buffer[2048];
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
        printf("%s\n", buffer);
        return;
    }

    cl_kernel kernel = clCreateKernel(program, "replace_missing", &err);
    if (!kernel || err != CL_SUCCESS) {
        printf("[ERROR] Error creating kernel. Error code: %d\n", err);
        return;
    }

    // Create the host buffer and initialize it
    int* host_buffer = (int*)malloc(size * sizeof(int));
    for (int i = 0; i < size; ++i) {
        host_buffer[i] = input[i];
    }

    // Create the device buffer
    cl_mem device_input = clCreateBuffer(context, CL_MEM_READ_ONLY, size * sizeof(int), NULL, NULL);
    cl_mem device_output = clCreateBuffer(context, CL_MEM_WRITE_ONLY, size * sizeof(int), NULL, NULL);

    // Set kernel arguments
    clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&device_input);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*)&device_output);
    clSetKernelArg(kernel, 2, sizeof(int), (void*)&size);

    // Create the command queue
    cl_command_queue command_queue = clCreateCommandQueue(context, device_id, NULL, NULL);

    // Host buffer -> Device buffer
    clEnqueueWriteBuffer(
        command_queue,
        device_input,
        CL_FALSE,
        0,
        size * sizeof(int),
        host_buffer,
        0,
        NULL,
        NULL
    );

    // Size specification
    size_t global_work_size = size;

    // Apply the kernel on the range
    clEnqueueNDRangeKernel(
        command_queue,
        kernel,
        1,
        NULL,
        &global_work_size,
        NULL,
        0,
        NULL,
        NULL
    );

    // Host buffer <- Device buffer
    clEnqueueReadBuffer(
        command_queue,
        device_output,
        CL_TRUE,
        0,
        size * sizeof(int),
        host_buffer,
        0,
        NULL,
        NULL
    );

    printf("Array after replacing missing elements:\n");
    for (int i = 0; i < size; ++i) {
        printf("%d ", host_buffer[i]);
    }
    printf("\n");

    // Release the OpenCL resources
    clReleaseMemObject(device_input);
    clReleaseMemObject(device_output);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(command_queue);
    clReleaseContext(context);

    // Free the host memory
    free(source_str);
    free(host_buffer);
}

int main() {
    srand(time(NULL));
    int s = 100;
    int array[s];
    int size = sizeof(array) / sizeof(array[0]);
    int r = 0;
    for(int i = 0; i < s; i++){
        array[i] = rand() % 100 + 1;
        r = rand() % 10 + 1;
        if(r == 1 && i > 0 && i < (s-1)){
            if(array[i-1] != -1){
                array[i] = -1;
            }
        }
    }

    replaceMissing(array, size);

    return 0;
}
