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

    // Initialize the array, fill L1 cache
    for (int i = 0; i < L1_CACHE_LINES; i++) {          // for each line
        for (int j = 0; j < NUM_INTS_IN_LINE; j++) {    // for every int in the line
            secret_array[i * NUM_INTS_IN_LINE + j] = j; 
        }
    }

    //** STEP 2: Flush Phase - Clear cache using clflsuh from utility.h */
    // We need to flush out every line and ensure that the line doesn't exist in any way in the cache as well
    for (int i = 0; i < L1_CACHE_LINES; i++) {
        clflush(&secret_array[i * NUM_INTS_IN_LINE]);
    }

    //** STEP 3: Victim Load Phase - Load value(s) from HugePage to the cache */
    int secret_data = secret_array[47];

    //** STEP 4: Reload mmap to see load times */
    uint64_t result[L1_CACHE_LINES] = {0};
    for (int i = 0; i < L1_CACHE_LINES; i++) {
        uint64_t time = measure_one_block_access_time((uint64_t)&secret_array[i * NUM_INTS_IN_LINE]);
        if (time < L1_THRESHOLD) {
            printf("Load time for line %d: %ld cycles\n", i, time);
            for (int entry = 0; entry < NUM_INTS_IN_LINE; entry++) {
                printf("secret_array[%ld]: %d\n", i * NUM_INTS_IN_LINE + entry, secret_array[i * NUM_INTS_IN_LINE + entry]);
            }
        }
    }

    return 0;
}