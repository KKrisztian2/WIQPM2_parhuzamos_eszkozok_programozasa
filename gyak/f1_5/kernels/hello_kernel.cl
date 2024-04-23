__kernel void replace_missing(__global const int* input, __global int* output, const int size) {
    int gid = get_global_id(0);

    if (gid < size) {
        // A hiányzó elemek pótlása a szomszédos elemek átlagával
        if (input[gid] == -1) {
            if (gid > 0 && gid < size - 1) {
                output[gid] = (input[gid - 1] + input[gid + 1]) / 2;
            } else if (gid == 0) {
                output[gid] = input[gid + 1];
            } else if (gid == size - 1) {
                output[gid] = input[gid - 1];
            }
        } else {
            output[gid] = input[gid];
        }
    }
}