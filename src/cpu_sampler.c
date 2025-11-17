#include "cpu_sampler.h"
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __linux__
#include <stdio.h>
#include <string.h>
#endif

#ifdef __APPLE__
#include <libproc.h>
#endif

typedef struct
{
  pid_t pid;
  uint64_t last_cpu_time_us;
} cpu_snapshot_t;

// Platform-specific CPU time retrieval
static uint64_t get_process_cpu_time(pid_t pid)
{
#ifdef __linux__
  char path[256];
  snprintf(path, sizeof(path), "/proc/%d/stat", pid);
  FILE *f = fopen(path, "r");
  if (!f)
    return 0;

  // /proc/<pid>/stat: utime=14th, stime=15th
  long utime, stime;
  int ignored;
  char buffer[1024];
  if (!fgets(buffer, sizeof(buffer), f))
  {
    fclose(f);
    return 0;
  }
  fclose(f);

  sscanf(buffer, "%*d %*s %*c "
                 "%*d %*d %*d %*d %*d "
                 "%*u %*u %*u %*u %*u "
                 "%ld %ld",
         &utime, &stime);

  long ticks_per_sec = sysconf(_SC_CLK_TCK);
  return (uint64_t)((utime + stime) * 1000000 / ticks_per_sec); // microseconds
#elif __APPLE__
  struct proc_taskinfo pti;
  if (proc_pidinfo(pid, PROC_PIDTASKINFO, 0, &pti, sizeof(pti)) <= 0)
    return 0;
  return pti.pti_total_user + pti.pti_total_system; // microseconds
#else
  return 0;
#endif
}

typedef struct
{
  process_info_t *list;
  size_t count;
} sampler_arg_t;

static void *cpu_sampler_thread(void *arg)
{
  sampler_arg_t *sarg = (sampler_arg_t *)arg;
  process_info_t *list = sarg->list;
  size_t count = sarg->count;

  cpu_snapshot_t snapshots[count];
  for (size_t i = 0; i < count; i++)
  {
    snapshots[i].pid = list[i].pid;
    snapshots[i].last_cpu_time_us = get_process_cpu_time(list[i].pid);
    list[i].cpu_usage = 0.0;
  }

  const double interval_sec = 1.0;

  while (running)
  {
    sleep((unsigned int)interval_sec);

    for (size_t i = 0; i < count; i++)
    {
      uint64_t new_time = get_process_cpu_time(list[i].pid);
      uint64_t delta_us = new_time - snapshots[i].last_cpu_time_us;
      snapshots[i].last_cpu_time_us = new_time;

      // CPU usage %
      list[i].cpu_usage = 100.0 * (delta_us / (interval_sec * 1e6));
    }
  }
  return NULL;
}

pthread_t start_cpu_sampler(process_info_t *list, size_t count)
{
  pthread_t tid;
  sampler_arg_t *arg = malloc(sizeof(sampler_arg_t));
  arg->list = list;
  arg->count = count;
  pthread_create(&tid, NULL, cpu_sampler_thread, arg);
  return tid;
}
