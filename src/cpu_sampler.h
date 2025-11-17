#ifndef CPU_SAMPLER_H
#define CPU_SAMPLER_H

#include <stddef.h>
#include "proc_parser.h"
#include <pthread.h>

extern volatile int running; // tells other .c files it exists somewhere

/**
 * Start the CPU sampling thread.
 * @param list Array of process_info_t to update
 * @param count Number of processes in the array
 * @return pthread_t handle of the sampler thread
 */
pthread_t start_cpu_sampler(process_info_t *list, size_t count);

#endif
