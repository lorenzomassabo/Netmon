#include "../include/function.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_CONNECTIONS 128

int main(void) {
  struct TcpConnection conns[MAX_CONNECTIONS];

  int count = fetch_tcp_connections(conns, MAX_CONNECTIONS);
  if (count < 0) {
    fprintf(stderr, "Error wihle reading TCP connections\n");
    return EXIT_FAILURE;
  }

  printf("Active TCP connections found: %d\n\n", count);
  printf("%-20s %-20s %-12s\n", "LOCAL ENDPORT", "REMOTE ENDPOINT", "STATE");
  printf("------------------------------------------------------------\n");

  for (int i = 0; i < count; ++i) {
    char local_buf[32];
    char rem_buf[32];

    snprintf(local_buf, sizeof(local_buf), "%s:%u", conns[i].local_ip,
             conns[i].local_port);
    snprintf(rem_buf, sizeof(rem_buf), "%s:%u", conns[i].rem_ip,
             conns[i].rem_port);

    printf("%-20s %-20s 0x%02X\n", local_buf, rem_buf, conns[i].state);
  }

  return EXIT_SUCCESS;
}
