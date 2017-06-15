#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#include "adjust.h"

void		 log_error_and_exit(char *origin);

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
	log_error_and_exit("libadjust_socket_open_out");

    if (libadjust_send_files(1, argv + 2) < 0)
	log_error_and_exit("libadjust_send_files");

    libadjust_socket_close();

    libadjust_stats_print(stdout);

    exit(EXIT_SUCCESS);
}

void
log_error_and_exit(char *origin)
{
    fprintf(stderr, "Failure in %s.  Error backtrace:\n", origin);
    libadjust_error_print(stderr);
    exit(EXIT_FAILURE);
}
