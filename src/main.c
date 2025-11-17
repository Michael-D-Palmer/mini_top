#include "proc_parser.h"
#include "cpu_sampler.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <ncurses.h>
#include <unistd.h>

// Global running flag, shared with sampler thread
volatile int running = 1;

int main()
{
  // Initialize ncurses
  initscr();             // start ncurses mode
  cbreak();              // disable line buffering
  noecho();              // don't echo keypresses
  nodelay(stdscr, TRUE); // make getch() non-blocking
  curs_set(0);           // hide cursor

  process_info_t list[1024];
  int count = get_process_list(list, 1024);
  if (count < 0)
  {
    endwin(); // cleanup ncurses
    fprintf(stderr, "Failed to get process list\n");
    return 1;
  }

  // Start CPU sampler thread
  pthread_t sampler = start_cpu_sampler(list, count);

  // Main UI loop
  while (running)
  {
    clear(); // clear screen

    mvprintw(0, 0, "PID     Name                  Memory     CPU");
    mvprintw(1, 0, "---------------------------------------------");

    for (int i = 0; i < count; i++)
    {
      mvprintw(i + 2, 0, "%5d  %-20s %8ld KB CPU: %.1f%%",
               list[i].pid, list[i].name, list[i].memory_kb, list[i].cpu_usage);
    }

    refresh();  // flush to screen
    napms(500); // sleep 500 ms

    int ch = getch(); // non-blocking input
    if (ch == 'q' || ch == 'Q')
      running = 0;
  }

  // Wait for sampler thread to finish
  pthread_join(sampler, NULL);

  // Cleanup ncurses
  endwin();

  return 0;
}
