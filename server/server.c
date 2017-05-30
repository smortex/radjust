#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#include "adjust.h"

int
main(int argc, char *argv[])
{
    if (argc != 2) {
	fprintf(stderr, "usage: %s filename\n", argv[0]);
	exit(EXIT_FAILURE);
    }

    if (libadjust_socket_open_in() < 0)
	err(EXIT_FAILURE, "libadjust_socket_open_in");

    if (libadjust_recv_file(argv[1]) < 0)
	err(EXIT_FAILURE, "libadjust_recv_file");

    libadjust_socket_close();

    size_t byte_synchronized, byte_send, byte_recv;

    get_xfer_stats(&byte_synchronized);
    get_networking_stats(&byte_send, &byte_recv);
    printf("server: synchronized %ld bytes\n", byte_synchronized);
    printf("server: sent %ld bytes, received %ld bytes\n", byte_send, byte_recv);

    exit(EXIT_SUCCESS);
}
