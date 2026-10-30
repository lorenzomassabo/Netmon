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

/*
 * Holds the parsed fields from a /proc/net/tcp record.
 * IP addresses and connection states are stored as 32-bit hex values.
 */
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
       * - %X:%X    Extracts remote IP (hex) and remote port (hex) separated by
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
         * /proc produces the exact byte layout required by network presentation
         * utilities.
         */
        struct in_addr addr;
        char ip_str[INET_ADDRSTRLEN]; /* 16-byte buffer for IPv4 dotted-decimal
                                         string */

        addr.s_addr = conn.local_ip_hex;

        /*
         * inet_ntop (Network to Presentation): converts the 32-bit binary
         * address to a standard null-terminated dotted-decimal string
         * ("xxx.xxx.xxx.xxx").
         */
        inet_ntop(AF_INET, &addr, ip_str, INET_ADDRSTRLEN);

        /* Print the connection details */
        printf("Local: %s:%u -> Remote Port: %u (State: 0x%X)\n", ip_str,
               conn.local_port, conn.rem_port, conn.state);
      }
    }
  }

  /* Release allocated system resources */
  free(line); /* Free the buffer managed internally by getline() */
  fclose(fp); /* Close the open file descriptor */

  return EXIT_SUCCESS;
}
