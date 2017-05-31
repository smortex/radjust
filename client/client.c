#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#include "adjust.h"

int
main(int argc, char *argv[])
{
    if (argc != 3) {
	fprintf(stderr, "usage: %s port filename\n", argv[0]);
	exit(EXIT_FAILURE);
    }

    int port;
    sscanf(argv[1], "%d", &port);

    if (libadjust_socket_open_out(port) < 0)
	err(EXIT_FAILURE, "libadjust_socket_open_out");

    if (libadjust_send_file(argv[2]) < 0)
	err(EXIT_FAILURE, "libadjust_send_file");

    libadjust_socket_close();

    size_t byte_synchronized, byte_send, byte_recv;

    get_xfer_stats(&byte_synchronized);
    get_networking_stats(&byte_send, &byte_recv);
    printf("client: synchronized %ld bytes\n", byte_synchronized);
    printf("client: sent %ld bytes, received %ld bytes\n", byte_send, byte_recv);

    exit(EXIT_SUCCESS);
}
