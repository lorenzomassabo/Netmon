#include "../include/function.h"
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Maximum capacity of the connection buffer allocated on the stack */
#define MAX_CONNECTIONS 128

/* Volatile atomic flag to guarantee safe modification from signal handlers */
static volatile sig_atomic_t keep_running = 1;

/* Signal handler for graceful termination on SIGINT (Ctrl+C) */
static void handle_sigint(int sig) {
  (void)sig;
  keep_running = 0;
}

int main(void) {
  struct sigaction sa;

  sa.sa_handler = handle_sigint;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  if (sigaction(SIGINT, &sa, NULL) == -1) {
    perror("sigaction");
    return EXIT_FAILURE;
  }

  struct TcpConnection conns[MAX_CONNECTIONS];

  /* Configure sleep duration: 1 second, 0 nanoseconds */
  struct timespec req = {.tv_sec = 1, .tv_nsec = 0};
  while (keep_running) {
    int count = fetch_tcp_connections(conns, MAX_CONNECTIONS);
    if (count < 0) {
      fprintf(stderr, "Error while reading TCP connections\n");
      return EXIT_FAILURE;
    }
    printf("\033[H\033[J"); // clear the terminal
    printf("Active TCP connections found: %d\n\n", count);

    /* Format header columns with fixed widths for clean alignment */
    printf("%-24s %-24s %-14s %-24s\n", "LOCAL ENDPOINT", "REMOTE ENDPOINT",
           "STATE", "PROGRAM (PID)");
    printf("-------------------------------------------------------------------"
           "----------------\n");

    for (int i = 0; i < count; ++i) {
      /* Intermediate formatting buffers to hold combined strings */
      char local_buf[32];
      char rem_buf[32];
      char proc_buf[48];

      /* Format local endpoint combining dotted-decimal IP and host-order port
       */
      snprintf(local_buf, sizeof(local_buf), "%s:%u", conns[i].local_ip,
               conns[i].local_port);
      /* Format remote endpoint combining dotted-decimal IP and host-order port
       */
      snprintf(rem_buf, sizeof(rem_buf), "%s:%u", conns[i].rem_ip,
               conns[i].rem_port);

      /* Format program name and PID into a single clean string */
      if (conns[i].pid != -1) {
        snprintf(proc_buf, sizeof(proc_buf), "%s (%d)", conns[i].process_name,
                 conns[i].pid);
      } else {
        snprintf(proc_buf, sizeof(proc_buf), "-");
      }

      /* Output formatted connection record to standard output */
      printf("%-24s %-24s %-14s %-24s\n", local_buf, rem_buf,
             tcp_state_to_str(conns[i].state), proc_buf);
    }
    /*
     * Suspend process execution for the duration specified in req.
     * Passing NULL as the second argument ignores any remaining time if
     * interrupted.
     */
    nanosleep(&req, NULL);
  }
  printf("\nExiting cleanly...\n");
  return EXIT_SUCCESS;
}
