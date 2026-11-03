#include "../include/function.h"
#include <arpa/inet.h>
#include <ctype.h>  /* isdigit() */
#include <dirent.h> /* opendir(), readdir(), closedir(), DIR, struct dirent */
#include <netinet/in.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h> /* strncmp(), strlen() */
#include <sys/socket.h>
#include <unistd.h> /* readlink() */

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
//
static int is_pid_dir(const char *name) {
  if (*name == '\0') {
    return 0;
  }
  // Iterate through the string until the null terminator ('\0') is reached.
  // *name checks the current character, while name++ moves the pointer to the
  // next character.
  for (; *name; ++name) {
    if (!isdigit((unsigned char)*name)) {
      return 0;
    }
  }
  return 1;
}

static void get_process_name(pid_t pid, char *dest, size_t max_len) {
  char comm_path[64];

  snprintf(comm_path, sizeof(comm_path), "/proc/%d/comm", pid);

  FILE *fp = fopen(comm_path, "r");
  if (fp == NULL) {
    // If opening fails, stores "-" in dest
    snprintf(dest, max_len, "-");
    return;
  }

  // Reads the process name from the file and stores it in dest
  if (fgets(dest, (int)max_len, fp) != NULL) {
    /* Strip trailing newline character stored by the kernel */
    size_t len = strlen(dest);
    // Checks if the last character is a newline
    if (len > 0 && dest[len - 1] == '\n') {
      // Replaces the newline with the string terminator
      dest[len - 1] = '\0';
    }
  } else {
    snprintf(dest, max_len, "-");
  }
  fclose(fp);
}

void resolve_process_info(struct TcpConnection *conn) {}

int fetch_tcp_connections(struct TcpConnection *list, size_t max_items) {
  FILE *fp = fopen("/proc/net/tcp", "r");
  if (fp == NULL) {
    perror("Failed to open /proc/net/tcp");
    return -1;
  }

  char *line = NULL; /* Buffer managed by getline(); initialized to NULL for
                        automatic malloc */
  size_t len = 0;    /* Allocated buffer capacity */
  size_t count = 0;  /* Number of valid connections successfully parsed */

  /* Skip the first line containing table column headers */
  if (getline(&line, &len, fp) != -1) {
    /* Read subsequent lines until EOF or until the output array capacity is
     * reached */
    while (getline(&line, &len, fp) != -1 && count < max_items) {
      uint32_t loc_hex, rem_hex;
      uint32_t loc_port, rem_port;
      uint32_t state;
      long unsigned inode;

      /* Parsing of /proc/net/tcp parameters */
      int parsed =
          sscanf(line, "%*d: %X:%X %X:%X %X %*X:%*X %*X:%*X %*X %*d %*d %lu",
                 &loc_hex, &loc_port, &rem_hex, &rem_port, &state, &inode);

      if (parsed == 6) {
        struct in_addr addr_loc = {.s_addr = loc_hex};
        struct in_addr addr_rem = {.s_addr = rem_hex};

        /* Convert IP addresses to dotted-decimal strings */
        inet_ntop(AF_INET, &addr_loc, list[count].local_ip,
                  sizeof(list[count].local_ip));
        inet_ntop(AF_INET, &addr_rem, list[count].rem_ip,
                  sizeof(list[count].rem_ip));

        /* Assign ports and connection state */
        list[count].local_port = (uint16_t)loc_port;
        list[count].rem_port = (uint16_t)rem_port;
        list[count].state = state;
        list[count].inode = inode;

        count++;
      }
    }
  }
  /* Release allocated system resources */
  free(line); /* Free the buffer managed internally by getline() */
  fclose(fp); /* Close the open file descriptor */
  return (int)count;
}
