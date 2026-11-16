#include "../include/function.h"
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
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

int main(int argc, char* argv[]) {
    struct NetmonArg arguments;
    struct sigaction sa;

    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }
    parseArguments(argc, argv, &arguments);

    struct TcpConnection conns[MAX_CONNECTIONS];

    /* Configure sleep duration: 1 second, 0 nanoseconds */
    struct timespec req = {.tv_sec = 1, .tv_nsec = 0};
    while (keep_running) {
        int count = fetch_tcp_connections(conns, MAX_CONNECTIONS);
        if (count < 0) {
            fprintf(stderr, "Error while reading TCP connections\n");
            return EXIT_FAILURE;
        }

        // Print processes
        printConnections(conns, count, &arguments);

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
