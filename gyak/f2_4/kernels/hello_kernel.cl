// matrix_operations.cl

// Transzponálás
__kernel void transpose(__global const float* input, __global float* output, const int rows, const int cols) {
    int global_id_x = get_global_id(0);
    int global_id_y = get_global_id(1);
    
    if (global_id_x < cols && global_id_y < rows) {
        output[global_id_x * rows + global_id_y] = input[global_id_y * cols + global_id_x];
    }
}

// Szorzás
__kernel void matrix_multiply(__global const float* A, __global const float* B, __global float* C, const int rowsA, const int colsA, const int colsB) {
    int global_id_x = get_global_id(0);
    int global_id_y = get_global_id(1);

    if (global_id_x < colsB && global_id_y < rowsA) {
        float sum = 0.0;
        for (int k = 0; k < colsA; ++k) {
            sum += A[global_id_y * colsA + k] * B[k * colsB + global_id_x];
        }
        C[global_id_y * colsB + global_id_x] = sum;
    }
}

// Oszlopösszeg számítás
__kernel void column_sum(__global const float* input, __global float* output, const int rows, const int cols) {
    int global_id_x = get_global_id(0);
    
    if (global_id_x < cols) {
        float sum = 0.0;
        for (int i = 0; i < rows; ++i) {
            sum += input[i * cols + global_id_x];
        }
        output[global_id_x] = sum;
    }
}

// Sorösszeg számítás
__kernel void row_sum(__global const float* input, __global float* output, const int rows, const int cols) {
    int global_id_y = get_global_id(0);
    
    if (global_id_y < rows) {
        float sum = 0.0;
        for (int i = 0; i < cols; ++i) {
            sum += input[global_id_y * cols + i];
        }
        output[global_id_y] = sum;
    }
}