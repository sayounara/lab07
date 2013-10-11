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


uint64_t measure_page_access(int page_stride, int max_index, float *array) {
    uint64_t before, after, diff;
    uint64_t sample_count = 0;
    diff = 0ULL;
    int passes = 0;
    const int PASSES = 1000000;
    int stride = getpagesize() / sizeof(float);
    for (; passes < PASSES; passes++) {
        int i = 0;
        for ( ; i < (page_stride * stride); i += stride) {
            before = currentcycles();
            float tmp = array[i]; // time the read
            after = currentcycles();
            diff += cyclediff(before, after);
            sample_count += 1;
            array[i] = tmp + 1;   // but don't count write in timing
        }
    }

    uint64_t cycle_average = diff / sample_count;
    return cycle_average;
}


int main(int argc, char **argv) {
    int page_size = getpagesize();

    printf("# page size is %d\n", page_size);

    int max_pages = 1024;
    if (argc == 2) {
        max_pages = atoi(argv[1]);
    } else {
        fprintf(stderr, "usage: %s max_pages\n", argv[0]);
        return -1;
    }

    if (max_pages <= 0) {
        fprintf(stderr, "Non-positive maximum number of pages?  C'mon.\n");
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

    int array_size = (page_size * max_pages);
    printf ("# array size %d (bytes)\n", array_size);
    printf ("# numpages         access time\n");
    printf ("# -------------    -----------\n");

    const int max_index = array_size / sizeof(float);
    float *array = (float*)malloc(array_size);
    int i = 0;
    for ( ; i < max_index; i++) {
        array[i] = 0;
    }

    int page_stride = 1;
    for ( ; page_stride <= max_pages; page_stride <<= 1) {
        uint64_t access_average = measure_page_access(page_stride, max_index, array);
        printf("%d\t%llu\n", page_stride, (long long unsigned int)access_average);
    }

    free (array);
#if __linux
    CPU_FREE(cpu_set);
#endif

    return 0;
}
