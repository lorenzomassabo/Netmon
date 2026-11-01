#ifndef FUNCTION_H
#define FUNCTION_H

#define _POSIX_C_SOURCE 200809L

#include <arpa/inet.h>  /* inet_ntop(), INET_ADDRSTRLEN */
#include <netinet/in.h> /* struct in_addr, AF_INET */
#include <stddef.h>     /* size_t */
#include <stdint.h>     /* uint32_t, uint16_t */
#include <stdio.h>      /* fopen(), getline(), sscanf(), printf(), perror() */
#include <stdlib.h>     /* free(), EXIT_SUCCESS */
#include <sys/socket.h> /* Socket domain constants */
#include <sys/types.h>  /* For pid_t */

struct TcpConnection {
  char local_ip[INET_ADDRSTRLEN]; /* Buffer to hold formatted dotted-decimal
                                     IPv4 string */
  uint16_t local_port;
  char rem_ip[INET_ADDRSTRLEN]; /* Buffer to hold formatted dotted-decimal IPv4
                                   string */
  uint16_t rem_port;
  uint32_t state;
  unsigned long inode;
  pid_t pid;
  char process_name[64];
};

/* Resolve PID and process name by matching socket inode against /proc/[pid]/fd
 * entries */
void resolve_process_info(struct TcpConnection *conn);

int fetch_tcp_connections(struct TcpConnection *list, size_t max_items);

const char *tcp_state_to_str(uint32_t state);

#endif // !FUNCTION_H
