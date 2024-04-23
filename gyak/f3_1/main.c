#include <stdio.h>
#include <stdlib.h>

#include <CL/cl.h>

#define MAX_SOURCE_SIZE (0x100000)

const int SAMPLE_SIZE = 1000;

// Callback függvény a buffer kiolvasásához
void CL_CALLBACK readBufferCallback(cl_event event, cl_int event_status, void *user_data) {
    if (event_status == CL_COMPLETE) {
        printf("Buffer read callback: Buffer reading completed successfully.\n");
    } else {
        printf("Buffer read callback: Buffer reading failed.\n");
    }
}

int main(void) {
    FILE *fp;
    char *source_str;
    size_t source_size;

    // Load the kernel source code
    fp = fopen("kernels/hello_kernel.cl", "r");
    if (!fp) {
        fprintf(stderr, "Failed to load kernel.\n");
        exit(1);
    }
    source_str = (char*)malloc(MAX_SOURCE_SIZE);
    source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
    puts("Kernel kod:");
    puts(source_str);
    fclose(fp);

    cl_int err;

    // Get platform
    cl_uint n_platforms;
    cl_platform_id platform_id;
    err = clGetPlatformIDs(1, &platform_id, &n_platforms);
    if (err != CL_SUCCESS) {
        printf("[ERROR] Error calling clGetPlatformIDs. Error code: %d\n", err);
        return 0;
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
        return 0;
    }

    // Create OpenCL context
    cl_context context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);
    if (!context || err != CL_SUCCESS) {
        printf("[ERROR] Error creating OpenCL context. Error code: %d\n", err);
        return 0;
    }

    // Build the program
    cl_program program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &err);
    if (!program || err != CL_SUCCESS) {
        printf("[ERROR] Error creating OpenCL program. Error code: %d\n", err);
        return 0;
    }
    err = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
        printf("Build error! Code: %d\n", err);
        size_t len;
        char buffer[2048];
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
        printf("%s\n", buffer);
        return 0;
    }

    cl_kernel kernel = clCreateKernel(program, "hello_kernel", &err);
    if (!kernel || err != CL_SUCCESS) {
        printf("[ERROR] Error creating kernel. Error code: %d\n", err);
        return 0;
    }

    // Create the host buffer and initialize it
    int* host_buffer = (int*)malloc(SAMPLE_SIZE * sizeof(int));
    for (int i = 0; i < SAMPLE_SIZE; ++i) {
        host_buffer[i] = i;
    }

    // Create the device buffer
    cl_mem device_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, SAMPLE_SIZE * sizeof(int), NULL, NULL);

    // Set kernel arguments
    clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&device_buffer);
    clSetKernelArg(kernel, 1, sizeof(int), (void*)&SAMPLE_SIZE);

    // Create the command queue
    cl_command_queue command_queue = clCreateCommandQueue(context, device_id, CL_QUEUE_PROFILING_ENABLE, &err);

    // Host buffer -> Device buffer
    clEnqueueWriteBuffer(
        command_queue,
        device_buffer,
        CL_FALSE,
        0,
        SAMPLE_SIZE * sizeof(int),
        host_buffer,
        0,
        NULL,
        NULL
    );

    // Size specification
    size_t local_work_size = 256;
    size_t n_work_groups = (SAMPLE_SIZE + local_work_size - 1) / local_work_size;
    size_t global_work_size = n_work_groups * local_work_size;

    // Apply the kernel on the range
    cl_event kernel_event;
    clEnqueueNDRangeKernel(
        command_queue,
        kernel,
        1,
        NULL,
        &global_work_size,
        &local_work_size,
        0,
        NULL,
        &kernel_event
    );

    // Set up callback for buffer read event
    cl_event read_event;
    err = clEnqueueReadBuffer(
        command_queue,
        device_buffer,
        CL_TRUE,
        0,
        SAMPLE_SIZE * sizeof(int),
        host_buffer,
        0,
        NULL,
        &read_event
    );
    if (err != CL_SUCCESS) {
        printf("Error enqueueing buffer read: %d\n", err);
        return 0;
    }
    clSetEventCallback(read_event, CL_COMPLETE, readBufferCallback, NULL);

    // Wait for kernel and read events to complete
    clWaitForEvents(1, &kernel_event);
    clWaitForEvents(1, &read_event);

    // Print results
    for (int i = 0; i < SAMPLE_SIZE; ++i) {
        printf("[%d] = %d, ", i, host_buffer[i]);
    }

    // Release the resources
    clReleaseEvent(kernel_event);
    clReleaseEvent(read_event);
    clReleaseMemObject(device_buffer);
    clReleaseCommandQueue(command_queue);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseContext(context);
    clReleaseDevice(device_id);
    free(source_str);
    free(host_buffer);

    return 0;
}
