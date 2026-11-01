#include "../include/function.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/* Maximum capacity of the connection buffer allocated on the stack */
#define MAX_CONNECTIONS 128

int main(void) {
  struct TcpConnection conns[MAX_CONNECTIONS];

  int count = fetch_tcp_connections(conns, MAX_CONNECTIONS);
  if (count < 0) {
    fprintf(stderr, "Error wihle reading TCP connections\n");
    return EXIT_FAILURE;
  }

  printf("Active TCP connections found: %d\n\n", count);
  /*
   * Format header columns:
   * %-22s: Left-aligned string with a fixed field width of 22 characters
   * %-12s: Left-aligned string with a fixed field width of 12 characters
   */
  printf("%-20s %-20s %-12s\n", "LOCAL ENDPORT", "REMOTE ENDPOINT", "STATE");
  printf("------------------------------------------------------------\n");

  for (int i = 0; i < count; ++i) {
    /* Intermediate formatting buffers to hold combined "IP:PORT" strings */
    char local_buf[32];
    char rem_buf[32];

    /* Format local endpoint combining dotted-decimal IP and host-order port */
    snprintf(local_buf, sizeof(local_buf), "%s:%u", conns[i].local_ip,
             conns[i].local_port);
    /* Format remote endpoint combining dotted-decimal IP and host-order port */
    snprintf(rem_buf, sizeof(rem_buf), "%s:%u", conns[i].rem_ip,
             conns[i].rem_port);

    /*
     * Output formatted connection record to standard output.
     * State is converted from raw numeric code to a descriptive string via
     * tcp_state_to_str().
     */
    printf("%-20s %-20s 0x%02X\n", local_buf, rem_buf, conns[i].state);
  }

  return EXIT_SUCCESS;
}
