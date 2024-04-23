#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <CL/cl.h>

#define MAX_SOURCE_SIZE (0x100000)

const int VECTOR_SIZE = 1000;


void vectorAdd(float *a, float *b, float *result, int size) {
    cl_int err;

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

    cl_kernel kernel = clCreateKernel(program, "vector_add", &err);
    if (!kernel || err != CL_SUCCESS) {
        printf("[ERROR] Error creating kernel. Error code: %d\n", err);
        return;
    }

    // Create the host buffer and initialize it randomly
    float *host_a = (float*)malloc(size * sizeof(float));
    float *host_b = (float*)malloc(size * sizeof(float));
    float *host_result = (float*)malloc(size * sizeof(float));
    srand(time(NULL)); // Initialize random seed
    for (int i = 0; i < size; i++) {
        host_a[i] = (float)rand() / RAND_MAX; // Random values between 0 and 1
        host_b[i] = (float)rand() / RAND_MAX; // Random values between 0 and 1
    }

    // Create the device buffers
    cl_mem device_a = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, size * sizeof(float), host_a, &err);
    cl_mem device_b = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, size * sizeof(float), host_b, &err);
    cl_mem device_result = clCreateBuffer(context, CL_MEM_WRITE_ONLY, size * sizeof(float), NULL, &err);

    // Set kernel arguments
    clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&device_a);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*)&device_b);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), (void*)&device_result);
    clSetKernelArg(kernel, 3, sizeof(int), (void*)&size);

    // Create the command queue
    cl_command_queue command_queue = clCreateCommandQueue(context, device_id, 0, &err);
    if (!command_queue || err != CL_SUCCESS) {
        printf("[ERROR] Error creating command queue. Error code: %d\n", err);
        return;
    }

    // Execute the kernel
    size_t global_size = size;
    err = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_size, NULL, 0, NULL, NULL);
    if (err != CL_SUCCESS) {
        printf("[ERROR] Error enqueueing kernel. Error code: %d\n", err);
        return;
    }

    // Read the result back to host memory
    err = clEnqueueReadBuffer(command_queue, device_result, CL_TRUE, 0, size * sizeof(float), host_result, 0, NULL, NULL);
    if (err != CL_SUCCESS) {
        printf("[ERROR] Error reading buffer. Error code: %d\n", err);
        return;
    }

    // Print the result
    printf("Result of vector addition:\n");
    for (int i = 0; i < size; ++i) {
        printf("%.2f + %.2f = %.2f\n", host_a[i], host_b[i], host_result[i]);
    }

    // Release the OpenCL resources
    clReleaseMemObject(device_a);
    clReleaseMemObject(device_b);
    clReleaseMemObject(device_result);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(command_queue);
    clReleaseContext(context);

    // Free the host memory
    free(source_str);
    free(host_a);
    free(host_b);
    free(host_result);
}

int main() {
    vectorAdd(NULL, NULL, NULL, VECTOR_SIZE);

    return 0;
}
