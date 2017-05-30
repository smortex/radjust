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

    if (libadjust_serve() < 0)
	err(EXIT_FAILURE, "libadjust_serve");

    if (libadjust_recv_file(argv[1]) < 0)
	err(EXIT_FAILURE, "libadjust_recv_file");

    libadjust_terminate();

    exit(EXIT_SUCCESS);
}
