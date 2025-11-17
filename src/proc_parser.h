#ifndef PROC_PARSER_H
#define PROC_PARSER_H

#include <sys/types.h> // for pid_t
#include <stdint.h>    // for uint64_t
#include <stddef.h>

typedef struct
{
  int pid;
  char name[256];
  double cpu_usage; // %
  long memory_kb;   // approximated from /proc/<pid>/statm
} process_info_t;

/**
 * Reads all processes in /proc and fills the supplied array with results.
 *
 * Returns: number of processes found, or -1 on error.
 */
int get_process_list(process_info_t *list, size_t max_count);

/**
 * Reads a specific PID and fills a process_info_t struct.
 *
 * Returns 0 on success, -1 on failure.
 */
int read_process_info(int pid, process_info_t *info);

#endif
