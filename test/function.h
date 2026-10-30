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
#include <sys/types.h>

struct TcpConnection {
  uint32_t local_ip_hex; /* Local IPv4 address in hex format (little-endian on
                            x86/ARM) */
  uint32_t local_port; /* Local port (converted from hex to decimal integer) */
  uint32_t rem_ip_hex; /* Remote IPv4 address in hex format */
  uint32_t rem_port;   /* Remote port (converted from hex to decimal integer) */
  uint32_t state; /* Numeric TCP connection state (e.g., 0x01 = ESTABLISHED,
                     0x0A = LISTEN) */
};

int fetch_tcp_connections(struct TcpConnection *list, size_t max_items);

#endif // !function.h
