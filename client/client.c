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

    if (libadjust_connect() < 0)
	err(EXIT_FAILURE, "libadjust_connect");

    if (libadjust_send_file(argv[1]) < 0)
	err(EXIT_FAILURE, "xfer_file");

    libadjust_terminate();

    exit(EXIT_SUCCESS);
}
