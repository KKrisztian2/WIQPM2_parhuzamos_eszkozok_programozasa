#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <CL/cl.h>

#define MAX_SOURCE_SIZE (0x100000)

struct chars{
    char c;
    int freq;
};

void countChars(const char *str, struct chars **ch, int *size){
    while (*str){
        int count = 0;
        unsigned char currentChar = (unsigned char)(*str);
        for(int i = 0; i < *size; i++){
            if((*ch)[i].c == currentChar){
                count++;
                (*ch)[i].freq++;
                i = *size;
            }
        }
        if (count == 0) {
            *ch = realloc(*ch, (*size+1) * sizeof(struct chars));
            (*ch)[*size].c = currentChar;
            (*ch)[*size].freq = 1;
            (*size)++;
        }
        str++;
    }
}

int sort(const void *a, const void *b) {
    const struct chars *ca = (const struct chars *)a;
    const struct chars *cb = (const struct chars *)b;
    return ca->freq - cb->freq;
}

struct TreeNode {
    char data;
    unsigned freq;
    struct TreeNode *left, *right;
};

struct Tree_kernel {
    char data;
    unsigned freq;
    int left, right;
};

struct TreeNode* newNode(char data, unsigned freq) {
    struct TreeNode* newNode = (struct TreeNode*)malloc(sizeof(struct TreeNode));
    newNode->data = data;
    newNode->freq = freq;
    newNode->left = newNode->right = NULL;
    return newNode;
}

struct MinHeapNode {
    struct TreeNode* data;
    struct MinHeapNode* next;
};

struct MinHeap {
    struct MinHeapNode *head;
};

struct MinHeapNode* newMinHeapNode(struct TreeNode* data) {
    struct MinHeapNode* newNode = (struct MinHeapNode*)malloc(sizeof(struct MinHeapNode));
    newNode->data = data;
    newNode->next = NULL;
    return newNode;
}

struct MinHeap* createMinHeap() {
    struct MinHeap* minHeap = (struct MinHeap*)malloc(sizeof(struct MinHeap));
    minHeap->head = NULL;
    return minHeap;
}

void insertMinHeap(struct MinHeap* minHeap, struct TreeNode* data) {
    struct MinHeapNode* newNode = newMinHeapNode(data);

    if (minHeap->head == NULL) {
        minHeap->head = newNode;
        return;
    }

    struct MinHeapNode* temp = minHeap->head;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = newNode;
}

struct TreeNode* extractMin(struct MinHeap* minHeap) {
    if (minHeap->head == NULL) {
        return NULL;
    }

    struct MinHeapNode* minNode = minHeap->head;
    struct MinHeapNode* prev = NULL;
    struct MinHeapNode* temp = minHeap->head;

    while (temp->next != NULL) {
        if (temp->next->data->freq < minNode->data->freq) {
            prev = temp;
            minNode = temp->next;
        }
        temp = temp->next;
    }

    if (prev == NULL) {
        minHeap->head = minNode->next;
    } else {
        prev->next = minNode->next;
    }

    struct TreeNode* data = minNode->data;
    free(minNode);
    return data;
}

struct TreeNode* buildHuffmanTree(struct chars **ch, int size) {
    struct TreeNode *left, *right, *top;
    struct MinHeap* minHeap = createMinHeap();
    for (int i = 0; i < size; ++i) {
        printf("%d ", i);
    printf("%c %d\n", (*ch)[i].c, (*ch)[i].freq);
        insertMinHeap(minHeap, newNode((*ch)[i].c, (*ch)[i].freq));
    }

    while (minHeap->head->next != NULL) {
        left = extractMin(minHeap);
        right = extractMin(minHeap);
        top = newNode('$', left->freq + right->freq);
        top->left = left;
        top->right = right;
        insertMinHeap(minHeap, top);
    }

    return extractMin(minHeap);
}

void printCodes(struct TreeNode* root, int arr[], int top, struct Tree_kernel **huff_tree, int *kernel_arr_size, int parent_index) {
    *huff_tree = (struct Tree_kernel *)realloc(*huff_tree, (*kernel_arr_size + 1) * sizeof(struct Tree_kernel));
    (*huff_tree)[*kernel_arr_size].data = root->data;
    (*huff_tree)[*kernel_arr_size].freq = root->freq;
    (*huff_tree)[*kernel_arr_size].left = -1;
    (*huff_tree)[*kernel_arr_size].right = -1;

    if (parent_index != -1) {
        if (arr[top - 1] == 0) {
            (*huff_tree)[parent_index].left = *kernel_arr_size;
        } else {
            (*huff_tree)[parent_index].right = *kernel_arr_size;
        }
    }

    int current_index = *kernel_arr_size;
    *kernel_arr_size = *kernel_arr_size + 1;

    if (root->left) {
        arr[top] = 0;
        printCodes(root->left, arr, top + 1, huff_tree, kernel_arr_size, current_index);
    }
    if (root->right) {
        arr[top] = 1;
        printCodes(root->right, arr, top + 1, huff_tree, kernel_arr_size, current_index);
    }

    if (!(root->left) && !(root->right)) {
        printf("%c: ", root->data);
        for (int i = 0; i < top; ++i) {
            printf("%d", arr[i]);
        }
        printf("\n");
    }
}

int main()
{
    char str[] = "A_DEAD_DAD_CEDED_A_BAD_BABE_A_BEADED_ABACA_BEDAC";
    //char str[] = "hfdfdfdffddfsfhdfbDHSNDHjcxDNJDXHCBR_DHWCJEHfjdskfncFHEnjdvq";
    //char str[] = "hfdfdfdffddfsfhdfbDHSNDHjcxDNJDXHCBR_DHWCJEHfjdskfncFEDJFS_fejdwfvijcwefhvjgfvcdclgbrfFDSDFdjedwkdfcvuierfwodcdhtrrzeegsdvjmghnfFHEnjdvq";
    //char str[] = "abcdefghijklmfghijklmnopqrstuvwxyzabcdefghijklmnopqabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
    //char str[] = "abcdefghijklmfghbcdefghijklmnocdefghijklmnopqrstuvwxyzabcfsdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcfghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
    //char str[] = "zWziWl81V7ZwSWjmJdPgV49fa4Lca5MIp6gbdqzXuzArY0nrVK7W627D2ZHyKgowkcUErseAWQB6Tr1XnY3WgPx9qDTBKeX1hvN2QmMtIDcotFF3Ix3k55X2YrxxRRCPDggzuM2L2RwiIBObGMpwDVccMzBQR3KI6qfzsXgnSqBSrqQKyHqXecPCmESWcADwust43JJX41jgg8SdB2o5a8W2NHyxg9oHba27PXEpoS9vbV4nEk3dCR8jsHPWPAcK6cNsah1SqFY4XpwE0QrNK13Fv0aPzMsyWdvDSERCWNH5CgtKK5Gmr2QNif38aD3pm8lhnHJmh3ZzHoBUFoUh42O8grRMKr8W6w175Xc8ahrUL210IAoMWRStAvaJnrThrjJ0q0IBvdyUQRhxl2GeFvwrHlRudef63AOKRxy3Cb7zS5HG2QNCB6yafJY2P9dSDHqF6rPHQ0Vr6DIYpTHCP8q1R1H5KXjLaSbNoeZD0i8qJMSn3RRB";
    //char str[] = "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";

    printf("%s \n", str);
    struct chars *ch = calloc(0, sizeof(struct chars));
    int size = 0;
    countChars(str, &ch, &size);

    qsort(ch, size, sizeof(struct chars), sort);

    printf("Character frequencies:\n");
    for (int i = 0; i < size; i++) {
        printf("%c: %d\n", ch[i].c, ch[i].freq);
    }
    struct TreeNode* root = buildHuffmanTree(&ch, size);

    int arr[size], top = 0;
    struct Tree_kernel *huff_tree = calloc(0, sizeof(struct Tree_kernel));
    int kernel_arr_size = 0;
    printf("Huffman Codes:\n");
    printCodes(root, arr, top, &huff_tree, &kernel_arr_size, 0);
    printf("\n\n\n %d\n", kernel_arr_size);
    for(int i = 0; i < kernel_arr_size; i++){
        printf("%c, %d, %d, %d \n", huff_tree[i].data, huff_tree[i].freq, huff_tree[i].left, huff_tree[i].right);
    }

    cl_int err;

    FILE *fp;
    char *source_str;
    size_t source_size;
    fp = fopen("kernels/encode_str.cl", "r");
    if (!fp) {
        fprintf(stderr, "Failed to load kernel.\n");
        exit(1);
    }
    source_str = (char*)malloc(MAX_SOURCE_SIZE);
    source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose(fp);

    cl_uint n_platforms;
    cl_platform_id platform_id;
    err = clGetPlatformIDs(1, &platform_id, &n_platforms);
    if (err != CL_SUCCESS) {
        printf("[ERROR] Error calling clGetPlatformIDs. Error code: %d\n", err);
        return;
    }

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

    cl_context context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);
    if (!context || err != CL_SUCCESS) {
        printf("[ERROR] Error creating OpenCL context. Error code: %d\n", err);
        return;
    }

    cl_program program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &err);
    if (!program || err != CL_SUCCESS) {
        printf("[ERROR] Error creating OpenCL program. Error code: %d\n", err);
        return;
    }
    err = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
        printf("Build error! Code: %d\n", err);
        size_t len;
        char buffer[100000];
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
        printf("%s\n", buffer);
        return;
    }

    cl_kernel kernel = clCreateKernel(program, "huffmanEncode", &err);
    if (!kernel || err != CL_SUCCESS) {
        printf("[ERROR] Error creating kernel. Error code: %d\n", err);
        return;
    }

    int strLength = strlen(str);
    size_t root_size = sizeof(struct TreeNode);

    char *host_result = (char*)malloc(10000000 * sizeof(char));

    cl_mem device_a = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, strLength * sizeof(char), &str, &err);
    cl_mem device_encoded = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(char) * 10000000, NULL, &err);
    cl_mem device_huff_tree = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, kernel_arr_size * sizeof(struct Tree_kernel), huff_tree, &err);

    clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&device_a);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*)&device_encoded);
    clSetKernelArg(kernel, 2, sizeof(int), &strLength);
    clSetKernelArg(kernel, 3, sizeof(cl_mem), (void*)&device_huff_tree);

    cl_command_queue command_queue = clCreateCommandQueue(context, device_id, CL_QUEUE_PROFILING_ENABLE , &err);
    if (!command_queue || err != CL_SUCCESS) {
        printf("[ERROR] Error creating command queue. Error code: %d\n", err);
        return;
    }

    size_t local_work_size = 2;
    size_t global_work_size = strLength/local_work_size;


    cl_event event;
    err = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_work_size, &local_work_size, 0, NULL, &event);
    if (err != CL_SUCCESS) {
        printf("[ERROR] Error enqueueing kernel. Error code: %d\n", err);
        return;
    }

    clFinish(command_queue);


    clWaitForEvents(1, &event);
    cl_ulong start, end;
    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, NULL);
    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, NULL);
    double elapsed_time = (end - start) * 1.0e-6;
    printf("Kernel execution time: %0.3f milliseconds\n", elapsed_time);

    err = clEnqueueReadBuffer(command_queue, device_encoded, CL_TRUE, 0, 10000000 * sizeof(char), host_result, 0, NULL, NULL);
    if (err != CL_SUCCESS) {
        printf("[ERROR] Error reading buffer. Error code: %d\n\n", err);
        return;
    }

    printf("\n\nEredmeny:\n");
    char code[1000000];
    int codedLength = 0;
    for(int i = 0; i < strLength; i++){
        for(int j = 0; j < 10; j++){
            if(host_result[i*10+j] != '-'){
                code[codedLength] = host_result[i*10+j];
                printf("%c", code[codedLength]);
                codedLength++;
            }
        }
    }

    clReleaseMemObject(device_a);
    clReleaseMemObject(device_encoded);
    clReleaseMemObject(device_huff_tree);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(command_queue);
    clReleaseContext(context);

    free(source_str);
    free(host_result);
    free(ch);

    return 0;
}
