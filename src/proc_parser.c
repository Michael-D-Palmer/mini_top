#include "proc_parser.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>

#ifdef __linux__
// Linux-specific /proc implementation

static int is_number(const char *s)
{
  for (; *s; s++)
  {
    if (!isdigit(*s))
      return 0;
  }
  return 1;
}

int read_process_info(int pid, process_info_t *info)
{
  char path[256];

  // 1. Read /proc/<pid>/comm (process name)
  snprintf(path, sizeof(path), "/proc/%d/comm", pid);
  FILE *f = fopen(path, "r");
  if (!f)
    return -1;

  if (!fgets(info->name, sizeof(info->name), f))
  {
    fclose(f);
    return -1;
  }
  info->name[strcspn(info->name, "\n")] = 0; // remove newline
  fclose(f);

  // 2. Read /proc/<pid>/statm for memory
  snprintf(path, sizeof(path), "/proc/%d/statm", pid);
  f = fopen(path, "r");
  if (f)
  {
    long pages;
    if (fscanf(f, "%ld", &pages) == 1)
    {
      info->memory_kb = pages * getpagesize() / 1024;
    }
    else
    {
      info->memory_kb = 0;
    }
    fclose(f);
  }
  else
  {
    info->memory_kb = 0;
  }

  // 3. Basic CPU placeholder for now
  // Real CPU usage requires reading /proc/stat or saving deltas.
  info->cpu_usage = 0.0;

  info->pid = pid;

  return 0;
}

int get_process_list(process_info_t *list, size_t max_count)
{
  DIR *d = opendir("/proc");
  if (!d)
    return -1;

  struct dirent *ent;
  size_t count = 0;

  while ((ent = readdir(d)) != NULL)
  {
    if (!is_number(ent->d_name))
      continue;

    if (count >= max_count)
      break;

    int pid = atoi(ent->d_name);

    if (read_process_info(pid, &list[count]) == 0)
    {
      count++;
    }
  }

  closedir(d);
  return (int)count;
}
#elif defined(__APPLE__)
// macOS-specific implementation
#include <libproc.h>
#include <sys/sysctl.h>

int read_process_info(int pid, process_info_t *info)
{
  char path[256];

  // 1. Read /proc/<pid>/comm (process name)
  snprintf(path, sizeof(path), "/proc/%d/comm", pid);
  FILE *f = fopen(path, "r");
  if (!f)
    return -1;

  if (!fgets(info->name, sizeof(info->name), f))
  {
    fclose(f);
    return -1;
  }
  info->name[strcspn(info->name, "\n")] = 0; // remove newline
  fclose(f);

  // 2. Read /proc/<pid>/statm for memory
  snprintf(path, sizeof(path), "/proc/%d/statm", pid);
  f = fopen(path, "r");
  if (f)
  {
    long pages;
    if (fscanf(f, "%ld", &pages) == 1)
    {
      info->memory_kb = pages * 4; // assume 4 KB page size
    }
    else
    {
      info->memory_kb = 0;
    }
    fclose(f);
  }
  else
  {
    info->memory_kb = 0;
  }

  // 3. Basic CPU placeholder for now
  // Real CPU usage requires reading /proc/stat or saving deltas.
  info->cpu_usage = 0.0;

  info->pid = pid;

  return 0;
}

int get_process_list(process_info_t *list, size_t max)
{
  pid_t pids[2048];

  int count = proc_listpids(PROC_ALL_PIDS, 0, pids, sizeof(pids));
  if (count <= 0)
    return -1;

  size_t n = 0;
  for (size_t i = 0; i < (size_t)count && n < max; i++)
  {
    pid_t pid = pids[i];
    struct proc_taskinfo pti;

    if (proc_pidinfo(pid, PROC_PIDTASKINFO, 0, &pti, sizeof(pti)) > 0)
    {
      list[n].pid = pid;
      list[n].cpu_usage = 0.0;

      list[n].memory_kb = pti.pti_resident_size / 1024;
      n++;
    }
  }
  return n;
}
#else
#error "Unsupported OS"
#endif
