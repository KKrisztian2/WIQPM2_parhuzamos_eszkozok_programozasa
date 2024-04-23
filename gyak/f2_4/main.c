#include <stdio.h>
#include <stdlib.h>

#include <CL/cl.h>

#define MAX_SOURCE_SIZE (0x100000)

// Transzponálás
void transposeMatrix(cl_context context, cl_command_queue command_queue, cl_program program, cl_kernel kernel, float* matrix, int rows, int cols) {
    cl_mem input_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, rows * cols * sizeof(float), NULL, NULL);
    cl_mem output_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, rows * cols * sizeof(float), NULL, NULL);

    clEnqueueWriteBuffer(command_queue, input_buffer, CL_TRUE, 0, rows * cols * sizeof(float), matrix, 0, NULL, NULL);

    clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_buffer);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &output_buffer);
    clSetKernelArg(kernel, 2, sizeof(int), &rows);
    clSetKernelArg(kernel, 3, sizeof(int), &cols);

    size_t global_work_size[2] = {cols, rows};
    clEnqueueNDRangeKernel(command_queue, kernel, 2, NULL, global_work_size, NULL, 0, NULL, NULL);


    clEnqueueReadBuffer(command_queue, output_buffer, CL_TRUE, 0, rows * cols * sizeof(float), matrix, 0, NULL, NULL);

    clReleaseMemObject(input_buffer);
    clReleaseMemObject(output_buffer);
}

// Szorzás
void multiplyMatrices(cl_context context, cl_command_queue command_queue, cl_program program, cl_kernel kernel, float* A, float* B, float* C, int rowsA, int colsA, int colsB) {
    cl_mem bufferA = clCreateBuffer(context, CL_MEM_READ_ONLY, rowsA * colsA * sizeof(float), NULL, NULL);
    cl_mem bufferB = clCreateBuffer(context, CL_MEM_READ_ONLY, colsA * colsB * sizeof(float), NULL, NULL);
    cl_mem bufferC = clCreateBuffer(context, CL_MEM_WRITE_ONLY, rowsA * colsB * sizeof(float), NULL, NULL);

    clEnqueueWriteBuffer(command_queue, bufferA, CL_TRUE, 0, rowsA * colsA * sizeof(float), A, 0, NULL, NULL);
    clEnqueueWriteBuffer(command_queue, bufferB, CL_TRUE, 0, colsA * colsB * sizeof(float), B, 0, NULL, NULL);

    clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufferA);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &bufferB);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &bufferC);
    clSetKernelArg(kernel, 3, sizeof(int), &rowsA);
    clSetKernelArg(kernel, 4, sizeof(int), &colsA);
    clSetKernelArg(kernel, 5, sizeof(int), &colsB);

    size_t global_work_size[2] = {colsB, rowsA};
    clEnqueueNDRangeKernel(command_queue, kernel, 2, NULL, global_work_size, NULL, 0, NULL, NULL);

    clEnqueueReadBuffer(command_queue, bufferC, CL_TRUE, 0, rowsA * colsB * sizeof(float), C, 0, NULL, NULL);

    clReleaseMemObject(bufferA);
    clReleaseMemObject(bufferB);
    clReleaseMemObject(bufferC);
}

// Oszlopösszeg számítás
void calculateColumnSum(cl_context context, cl_command_queue command_queue, cl_program program, cl_kernel kernel, float* matrix, float* result, int rows, int cols) {
    cl_mem input_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, rows * cols * sizeof(float), NULL, NULL);
    cl_mem output_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, cols * sizeof(float), NULL, NULL);

    clEnqueueWriteBuffer(command_queue, input_buffer, CL_TRUE, 0, rows * cols * sizeof(float), matrix, 0, NULL, NULL);

    clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_buffer);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &output_buffer);
    clSetKernelArg(kernel, 2, sizeof(int), &rows);
    clSetKernelArg(kernel, 3, sizeof(int), &cols);

    size_t global_work_size[1] = {cols};
    clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);

    clEnqueueReadBuffer(command_queue, output_buffer, CL_TRUE, 0, cols * sizeof(float), result, 0, NULL, NULL);

    clReleaseMemObject(input_buffer);
    clReleaseMemObject(output_buffer);
}

// Sorösszeg számítás
void calculateRowSum(cl_context context, cl_command_queue command_queue, cl_program program, cl_kernel kernel, float* matrix, float* result, int rows, int cols) {
    cl_mem input_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, rows * cols * sizeof(float), NULL, NULL);
    cl_mem output_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, rows * sizeof(float), NULL, NULL);

    clEnqueueWriteBuffer(command_queue, input_buffer, CL_TRUE, 0, rows * cols * sizeof(float), matrix, 0, NULL, NULL);

    clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_buffer);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &output_buffer);
    clSetKernelArg(kernel, 2, sizeof(int), &rows);
    clSetKernelArg(kernel, 3, sizeof(int), &cols);

    size_t global_work_size[1] = {rows};
    clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);

    clEnqueueReadBuffer(command_queue, output_buffer, CL_TRUE, 0, rows * sizeof(float), result, 0, NULL, NULL);

    clReleaseMemObject(input_buffer);
    clReleaseMemObject(output_buffer);
}

int main() {
    // Mátrix inicializálása és mérete
    float A[] = {1.0, 2.0, 3.0, 4.0,
                 5.0, 6.0, 7.0, 8.0,
                 9.0, 10.0, 11.0, 12.0};
    int rowsA = 3;
    int colsA = 4;

    float B[] = {1.0, 2.0, 3.0,
                 4.0, 5.0, 6.0,
                 7.0, 8.0, 9.0,
                 10.0, 11.0, 12.0};
    int rowsB = 4;
    int colsB = 3;

    // OpenCL inicializálása
    cl_int err;
    cl_platform_id platform_id = NULL;
    cl_device_id device_id = NULL;
    cl_uint n_platforms, n_devices;
    cl_context context;
    cl_command_queue command_queue;
    cl_program program;
    cl_kernel kernel_transpose, kernel_multiply, kernel_column_sum, kernel_row_sum;

    // Platform lekérdezése
    err = clGetPlatformIDs(1, &platform_id, &n_platforms);
    if (err != CL_SUCCESS) {
        printf("Error: Failed to find an OpenCL platform!\n");
        return EXIT_FAILURE;
    }

    // Eszköz lekérdezése
    err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, &n_devices);
    if (err != CL_SUCCESS) {
        printf("Error: Failed to find a GPU device!\n");
        return EXIT_FAILURE;
    }

    // OpenCL kontextus létrehozása
    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);
    if (!context || err != CL_SUCCESS) {
        printf("Error: Failed to create an OpenCL context!\n");
        return EXIT_FAILURE;
    }

    // OpenCL parancssor létrehozása
    command_queue = clCreateCommandQueue(context, device_id, 0, &err);
    if (!command_queue || err != CL_SUCCESS) {
        printf("Error: Failed to create an OpenCL command queue!\n");
        return EXIT_FAILURE;
    }

    // Kernel fájl beolvasása
    FILE *fp;
    char *source_str;
    size_t source_size;

    fp = fopen("kernels/hello_kernel.cl", "r");
    if (!fp) {
        printf("Error: Failed to load kernel source!\n");
        return EXIT_FAILURE;
    }

    source_str = (char*)malloc(MAX_SOURCE_SIZE);
    source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose(fp);

    // Kernel létrehozása
    program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &err);
    if (!program || err != CL_SUCCESS) {
        printf("Error: Failed to create OpenCL program!\n");
        return EXIT_FAILURE;
    }

    err = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
        printf("Error: Failed to build OpenCL program!\n");
        return EXIT_FAILURE;
    }

    kernel_transpose = clCreateKernel(program, "transpose", &err);
    kernel_multiply = clCreateKernel(program, "matrix_multiply", &err);
    kernel_column_sum = clCreateKernel(program, "column_sum", &err);
    kernel_row_sum = clCreateKernel(program, "row_sum", &err);

    // Mátrix műveletek végrehajtása
    float *result_transpose = (float*)malloc(rowsA * colsA * sizeof(float));
    transposeMatrix(context, command_queue, program, kernel_transpose, A, rowsA, colsA);
    printf("Transposed matrix:\n");
    for (int i = 0; i < colsA; ++i) {
        for (int j = 0; j < rowsA; ++j) {
            printf("%.2f ", A[i * rowsA + j]);
        }
        printf("\n");
    }

    float *result_multiply = (float*)malloc(rowsA * colsB * sizeof(float));
    multiplyMatrices(context, command_queue, program, kernel_multiply, A, B, result_multiply, rowsA, colsA, colsB);
    printf("Matrix multiplication result:\n");
    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            printf("%.2f ", result_multiply[i * colsB + j]);
        }
        printf("\n");
    }

    float *result_column_sum = (float*)malloc(colsA * sizeof(float));
    calculateColumnSum(context, command_queue, program, kernel_column_sum, A, result_column_sum, rowsA, colsA);
    printf("Column sums:\n");
    for (int i = 0; i < colsA; ++i) {
        printf("%.2f ", result_column_sum[i]);
    }
    printf("\n");

    float *result_row_sum = (float*)malloc(rowsA * sizeof(float));
    calculateRowSum(context, command_queue, program, kernel_row_sum, A, result_row_sum, rowsA, colsA);
    printf("Row sums:\n");
    for (int i = 0; i < rowsA; ++i) {
        printf("%.2f ", result_row_sum[i]);
    }
    printf("\n");

    // Resszvények felszabadítása
    free(result_transpose);
    free(result_multiply);
    free(result_column_sum);
    free(result_row_sum);

    clReleaseKernel(kernel_transpose);
    clReleaseKernel(kernel_multiply);
    clReleaseKernel(kernel_column_sum);
    clReleaseKernel(kernel_row_sum);

    clReleaseProgram(program);
    clReleaseCommandQueue(command_queue);
    clReleaseContext(context);

    return 0;
}
