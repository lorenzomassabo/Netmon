#include "../include/function.h"
#include <arpa/inet.h>
#include <stdint.h>
#include <sys/socket.h>

const char *tcp_state_to_str(uint32_t state) {
  switch (state) {
  case 0x01:
    return "ESTABLISHED";
  case 0x02:
    return "SYN_SENT";
  case 0x03:
    return "SYN_RECV";
  case 0x04:
    return "FIN_WAIT1";
  case 0x05:
    return "FIN_WAIT2";
  case 0x06:
    return "TIME_WAIT";
  case 0x07:
    return "CLOSE";
  case 0x08:
    return "CLOSE_WAIT";
  case 0x09:
    return "LAST_ACK";
  case 0x0A:
    return "LISTEN";
  case 0x0B:
    return "CLOSING";
  case 0x0C:
    return "NEW_SYNC_RECV";
  default:
    return "UNKNOWN";
  }
}

int fetch_tcp_connections(struct TcpConnection *list, size_t max_items) {
  FILE *fp = fopen("/proc/net/tcp", "r");
  if (fp == NULL) {
    perror("Failed to open /proc/net/tcp");
    return -1;
  }

  char *line = NULL;
  size_t len = 0;
  size_t count = 0;
  struct TcpConnection conn;

  if (getline(&line, &len, fp) != -1) {
    while (getline(&line, &len, fp) != -1 && count < max_items) {
      uint32_t loc_hex, rem_hex;
      uint32_t loc_port, rem_port;
      uint32_t state;

      int parsed = sscanf(line, "%*d: %X:%X %X:%X %X", &loc_hex, &loc_port,
                          &rem_hex, &rem_port, &state);

      if (parsed == 5) {
        struct in_addr addr_loc = {.s_addr = loc_hex}; // local ip
        struct in_addr addr_rem = {.s_addr = rem_hex};

        inet_ntop(AF_INET, &addr_loc, list[count].local_ip,
                  sizeof(list[count].local_ip));
      }
    }

    /* Release allocated system resources */
    free(line); /* Free the buffer managed internally by getline() */
    fclose(fp); /* Close the open file descriptor */
  }
  return (int)count;
}
