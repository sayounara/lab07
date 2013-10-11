#if __linux
#define _GNU_SOURCE
#include <sched.h>
#endif

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>

#ifndef MAX
#define MAX(a,b) (a < b ? b : a)
#endif

#if defined(__i386__)
inline uint64_t currentcycles() {
    uint64_t result;
    __asm__ __volatile__ ("rdtsc" : "=A" (result));
    return result;
}
#elif defined(__x86_64__)
static __inline__ uint64_t currentcycles(void)
{
     uint32_t a, d; 
     asm volatile("rdtsc" : "=a" (a), "=d" (d)); 
     return ((uint64_t)a) | (((uint64_t)d) << 32); 
}
#endif

inline uint64_t cyclediff(uint64_t before, uint64_t after) {
    /* handle wrap-around in difference */
    if (before > after) {
#if defined(__i386)
        uint64_t residual = UINT32_MAX - before;
#elif defined(__x86_64__)
        uint64_t residual = UINT64_MAX - before;
#endif
        return (after - 0) + residual;
    } else {
        return after - before;
    }
}

int main(int argc, char **argv) {
    
    int page_size = getpagesize();
    int array_size = 0;
    printf("# page size is %d\n", page_size);

    if (argc == 2) {
        array_size = atoi(argv[1]);
    } else {
        fprintf(stderr, "usage: %s array_size\n", argv[0]);
        return -1;
    }

    if (array_size <= 0) {
        fprintf(stderr, "Non-positive array size?  C'mon.\n");
        return -1;
    }

#if __linux
    #define CPUSETSIZE 8 
    // pin this thread to one CPU
    int cpu = sched_getcpu();
    printf("# running on CPU %d; pinning to that CPU\n", cpu); 
    cpu_set_t *cpu_set = CPU_ALLOC(CPUSETSIZE);
    size_t size = CPU_ALLOC_SIZE(CPUSETSIZE);
    CPU_ZERO_S(size, cpu_set);
    CPU_SET_S(cpu, size, cpu_set);
    pid_t pid = getpid();

    int rv = sched_setaffinity(pid, CPU_ALLOC_SIZE(CPUSETSIZE), cpu_set);
    if (rv < 0) {
        fprintf(stderr, "Error pinning thread to a CPU: %s\n", strerror(errno));
    }
#endif // __linux


    // ensure a power of 2
    int width = 0;
    while (array_size > 0) {
        array_size >>= 1;
        width++;
    }
    array_size = 1 << width; 


    printf ("# array size %d (bytes)\n", array_size);
    printf ("# stride_length    cycles\n");
    printf ("# -------------    ------\n");
    const int max_index = array_size / sizeof(float);
    float *array = (float*)malloc(sizeof(float)*max_index);
    int i = 0;
    for ( ; i < max_index; i++) {
        array[i] = 0;
    }

    uint64_t before, after, diff;

    int stride_length = 2;
    while (stride_length <= max_index/2) {
        int PASSES = MAX(10000, (double)stride_length / (double)max_index * 100000000);
	uint64_t sample_count = 0;
        diff = 0ULL;
        int passes = 0;
        for (; passes < PASSES; passes++) {
            i = 0;
            while (i < max_index) {
                before = currentcycles();
                float tmp = array[i]; // time the read
                after = currentcycles();
                diff += cyclediff(before, after);
                sample_count += 1;
                array[i] = tmp + 1;   // but don't count write in timing
                i += stride_length;
            }
        }

        uint64_t cycle_average = diff / sample_count;
        printf("\t%d\t\t%llu\n", stride_length << 2, (long long unsigned int)cycle_average);
        stride_length <<= 1;
    }

    free (array);
#if __linux
    CPU_FREE(cpu_set);
#endif

    return 0;
}
