/*
 * Enables POSIX.1-2008 features in glibc.
 * Must precede any system header includes to expose getline().
 */
#define _POSIX_C_SOURCE 200809L
#include <arpa/inet.h>  /* inet_ntop(), INET_ADDRSTRLEN */
#include <netinet/in.h> /* struct in_addr, AF_INET */
#include <stddef.h>     /* size_t */
#include <stdint.h>     /* uint32_t, uint16_t */
#include <stdio.h>      /* fopen(), getline(), sscanf(), printf(), perror() */
#include <stdlib.h>     /* free(), EXIT_SUCCESS */
#include <sys/socket.h> /* Socket domain constants */
#include <sys/types.h>

/*
 * Holds the parsed fields from a /proc/net/tcp record.
 * IP addresses and connection states are stored as 32-bit hex values.
 */

/*
 * Translates numeric TCP states into standard human-readable labels.
 */
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

struct TcpConnection {
  uint32_t local_ip_hex; /* Local IPv4 address in hex format (little-endian on
                            x86/ARM) */
  uint32_t local_port; /* Local port (converted from hex to decimal integer) */
  uint32_t rem_ip_hex; /* Remote IPv4 address in hex format */
  uint32_t rem_port;   /* Remote port (converted from hex to decimal integer) */
  uint32_t state; /* Numeric TCP connection state (e.g., 0x01 = ESTABLISHED,
                     0x0A = LISTEN) */
};

int main(int argc, char *argv[]) {
  /*
   * Open the kernel pseudo-file containing active TCP connection tables.
   * The kernel generates this content on-the-fly upon read requests.
   */
  FILE *fp = fopen("/proc/net/tcp", "r");
  if (fp == NULL) {
    perror("File not found !");
    return 1;
  }

  /*
   * Buffers for getline().
   * Setting line to NULL and len to 0 directs getline() to handle memory
   * allocation and resizing (malloc/realloc) dynamically.
   */
  char *line = NULL;
  size_t len = 0;
  struct TcpConnection conn;

  /*
   * Read and discard the first line (column headers):
   * "  sl  local_address rem_address   st tx_queue rx_queue tr tm->when
   * retrnsmt   uid  timeout inode"
   */
  if (getline(&line, &len, fp) != -1) {
    /* Iterate over each connection record until reaching End Of File (EOF) */
    while (getline(&line, &len, fp) != -1) {
      /*
       * Parse positional tokens using sscanf:
       * - %*d:     Matches and discards the sequence index and trailing colon
       * (e.g., "   0:")
       * - %X:%X    Extracts local IP (hex) and local port (hex) separated by
       * ':'
       * - %X:%X    Extracts remote IP (hex) and remote port (hex) separated
       * by
       * ':'
       * - %X       Extracts the connection state code (hex)
       */
      int parsed = sscanf(line, "%*d: %X:%X %X:%X %X", &conn.local_ip_hex,
                          &conn.local_port, &conn.rem_ip_hex, &conn.rem_port,
                          &conn.state);

      /* Ensure all 5 requested tokens were matched and stored */
      if (parsed == 5) {
        /*
         * in_addr wraps a single 32-bit uint32_t (s_addr).
         * On little-endian architectures, reading the raw hex integer from
         * /proc produces the exact byte layout required by network
         * presentation utilities.
         */
        struct in_addr addr;          // local ip
        struct in_addr addr_dest;     // remote ip
        char ip_str[INET_ADDRSTRLEN]; /* 16-byte buffer for IPv4
                                         dotted-decimal string */
        char ip_str_dest[INET_ADDRSTRLEN];
        addr.s_addr = conn.local_ip_hex;
        addr_dest.s_addr = conn.rem_ip_hex;
        /*
         * inet_ntop (Network to Presentation): converts the 32-bit binary
         * address to a standard null-terminated dotted-decimal string
         * ("xxx.xxx.xxx.xxx").
         */

        // ip source
        inet_ntop(AF_INET, &addr, ip_str, INET_ADDRSTRLEN);
        // ip dest
        inet_ntop(AF_INET, &addr_dest, ip_str_dest, INET_ADDRSTRLEN);

        // the "-5" is used to tabulate by 5 characters
        /* Print the connection details */
        printf("Local: %s:%-5u -> Remote: %s:%-5u [%s]\n", ip_str,
               conn.local_port, ip_str_dest, conn.rem_port,
               tcp_state_to_str(conn.state));
      }
    }
  }

  /* Release allocated system resources */
  free(line); /* Free the buffer managed internally by getline() */
  fclose(fp); /* Close the open file descriptor */

  return EXIT_SUCCESS;
}
