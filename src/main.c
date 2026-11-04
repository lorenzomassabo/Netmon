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

  // This function will be executed when the program receives SIGINT (Ctrl+C).
  sa.sa_handler = handle_sigint;

  // Initialize an empty signal mask.
  // No additional signals will be blocked while the signal handler executes.
  sigemptyset(&sa.sa_mask);

  // Set signal handling flags to 0.
  // This means no additional options are enabled.
  sa.sa_flags = 0;

  // Register the signal handler for SIGINT.
  // Returns -1 if the operation fails.
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
    /*
     * Format header columns:
     * %-22s: Left-aligned string with a fixed field width of 22 characters
     * %-12s: Left-aligned string with a fixed field width of 12 characters
     */
    printf("%-20s %-20s %-19s %s\n", "LOCAL ENDPORT", "REMOTE ENDPOINT",
           "STATE", "INODE");
    printf("-------------------------------------------------------------------"
           "-\n");

    for (int i = 0; i < count; ++i) {
      /* Intermediate formatting buffers to hold combined "IP:PORT" strings */
      char local_buf[32];
      char rem_buf[32];

      /* Format local endpoint combining dotted-decimal IP and host-order port
       */
      snprintf(local_buf, sizeof(local_buf), "%s:%u", conns[i].local_ip,
               conns[i].local_port);
      /* Format remote endpoint combining dotted-decimal IP and host-order port
       */
      snprintf(rem_buf, sizeof(rem_buf), "%s:%u", conns[i].rem_ip,
               conns[i].rem_port);
      /*
       * Output formatted connection record to standard output.
       * State is converted from raw numeric code to a descriptive string via
       * tcp_state_to_str().
       */
      printf("%-20s %-20s %-20s", local_buf, rem_buf,
             tcp_state_to_str(conns[i].state));
      printf("%-20lu\n", conns[i].inode);
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
