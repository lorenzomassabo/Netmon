#include "../include/function.h"

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

int fetch_tcp_connections(struct TcpConnection *list, size_t max_items) {}
