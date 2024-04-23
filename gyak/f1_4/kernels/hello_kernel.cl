__kernel void vector_add(__global const float* a, __global const float* b, __global float* result, const int size) {
    int global_id = get_global_id(0);

    if (global_id < size) {
        result[global_id] = a[global_id] + b[global_id];
    }
}