__kernel void hello_kernel(__global int* output) {
    // Az indexek beállítása
    int global_id = get_global_id(0);
    int local_id = get_local_id(0);
    int group_id = get_group_id(0);
    int group_size = get_local_size(0);
    int global_size = get_global_size(0);

    // Az eredménytömbbe beállítja a globális és lokális indexeket
	int reversed_index = global_size - global_id - 1;
    output[reversed_index] = global_id;
    output[global_size + local_id] = local_id;
	
	// Cseréli a szomszédos, páros és páratlan indexeken lévő elemeket
    if (global_id < global_size - 1) {
        // Csere a szomszédos indexeken
        int temp = output[global_id];
        output[global_id] = output[global_id + 1];
        output[global_id + 1] = temp;
    }

    if (global_id % 2 == 0 && global_id + 1 < global_size) {
        // Csere a páros indexeken
        int temp = output[global_id];
        output[global_id] = output[global_id + 1];
        output[global_id + 1] = temp;
    }

    if (global_id % 2 != 0 && global_id + 1 < global_size) {
        // Csere a páratlan indexeken
        int temp = output[global_id];
        output[global_id] = output[global_id + 1];
        output[global_id + 1] = temp;
    }
}