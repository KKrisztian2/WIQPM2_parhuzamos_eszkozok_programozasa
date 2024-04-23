__kernel void count_frequencies(__global int* array, __global int* frequencies, int size) {
    int gid = get_global_id(0);
    if (gid < size) {
        atomic_inc(&frequencies[array[gid]]);
    }
}