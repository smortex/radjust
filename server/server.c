#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#include "adjust.h"

void		 log_error_and_exit(char *origin);

int
main(int argc, char *argv[])
{
    if (argc != 2) {
	fprintf(stderr, "usage: %s filename\n", argv[0]);
	exit(EXIT_FAILURE);
    }

    int port = libadjust_socket_open_in();
    if (port < 0)
	log_error_and_exit("libadjust_socket_open_in");

    fprintf(stdout, "%d\n", port);
    fflush(stdout);

    if (libadjust_socket_open_in_accept() < 0)
	log_error_and_exit("libadjust_socket_open_in_accept");

    if (libadjust_recv_file(argv[1]) < 0)
	log_error_and_exit("libadjust_recv_file");

    libadjust_socket_close();

    size_t byte_synchronized, byte_send, byte_recv;

    get_xfer_stats(&byte_synchronized);
    get_networking_stats(&byte_send, &byte_recv);
    printf("server: synchronized %ld bytes\n", byte_synchronized);
    printf("server: sent %ld bytes, received %ld bytes\n", byte_send, byte_recv);

    exit(EXIT_SUCCESS);
}

void
log_error_and_exit(char *origin)
{
    fprintf(stderr, "Failure in %s.  Error backtrace:\n", origin);
    libadjust_error_print(stderr);
    exit(EXIT_FAILURE);
}
