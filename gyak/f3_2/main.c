#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>

#define ARRAY_SIZE 1000
#define RANGE 101 // 0-100 intervallumhoz

const char *kernelSource = "__kernel void calculateFrequency(__global int *input, __global int *output) {\n"
                           "    int gid = get_global_id(0);\n"
                           "    atomic_inc(&output[input[gid]]);\n"
                           "}\n";

int main() {
    // Generate random array
    int input[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        input[i] = rand() % RANGE; // Generating random numbers in [0, 100]
    }

    // Load kernel source
    cl_uint numPlatforms;
    clGetPlatformIDs(0, NULL, &numPlatforms);
    cl_platform_id platform;
    clGetPlatformIDs(1, &platform, NULL);

    cl_uint numDevices;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, NULL, &numDevices);
    cl_device_id device;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, NULL);

    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);
    cl_command_queue queue = clCreateCommandQueue(context, device, 0, NULL);

    cl_mem inputBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * ARRAY_SIZE, input, NULL);
    cl_mem outputBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int) * RANGE, NULL, NULL);

    cl_program program = clCreateProgramWithSource(context, 1, (const char **)&kernelSource, NULL, NULL);
    clBuildProgram(program, 1, &device, NULL, NULL, NULL);

    cl_kernel kernel = clCreateKernel(program, "calculateFrequency", NULL);
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &inputBuffer);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &outputBuffer);

    size_t globalSize = ARRAY_SIZE;
    clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &globalSize, NULL, 0, NULL, NULL);

    int output[RANGE] = {0};
    clEnqueueReadBuffer(queue, outputBuffer, CL_TRUE, 0, sizeof(int) * RANGE, output, 0, NULL, NULL);

    // Display frequencies
    printf("Frequencies:\n");
    for (int i = 0; i < RANGE; ++i) {
        printf("%d: %d\n", i, output[i]);
    }

    // Clean up
    clReleaseMemObject(inputBuffer);
    clReleaseMemObject(outputBuffer);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    return 0;
}
