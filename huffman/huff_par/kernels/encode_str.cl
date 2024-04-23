
struct Tree_kernel {
    char data;
    unsigned freq;
    int left, right;
};


void encode(struct Tree_kernel* huff_tree, int root_index, char data, __global char *output, int offset, int depth, __private char *encoded) {
    if (root_index == -1) return;
	
    if (huff_tree[root_index].left == -1 && huff_tree[root_index].right == -1 && huff_tree[root_index].data == data) {
		for(int i = 0; i < depth; i++){
			output[i+offset] = encoded[i];
		}
		for(int i = offset + depth; i < offset + 10; i++){
			output[i] = '-';
		}
        return;
    }

    if (huff_tree[root_index].left != -1) {
        encoded[depth] = '0';
        encode(huff_tree, huff_tree[root_index].left, data, output, offset, depth + 1, encoded);
    }

    if (huff_tree[root_index].right != -1) {
        encoded[depth] = '1';
        encode(huff_tree, huff_tree[root_index].right, data, output, offset, depth + 1, encoded);
    }
}


__kernel void huffmanEncode(__global const char *input, __global char *output, int strLength, __global struct Tree_kernel* huff_tree) {
    int gid = get_global_id(0);
	
	int numThreads = get_global_size(0);
    int itemsPerThread = strLength / numThreads;

    int startIdx = gid * itemsPerThread;
    int endIdx = startIdx + itemsPerThread;
	if(strLength - endIdx - 1 < endIdx - startIdx){
		endIdx = strLength;
	}

    int offset = startIdx * 10;
	
	__private char encoded[10];
    for (int i = startIdx; i < endIdx; i++) {
        encode(huff_tree, 0, input[i], output, offset, 0, encoded);
        offset += 10;
    }
	
}