#include <stdio.h>
#include <stdlib.h>

#include <sys/stat.h>

#include "adjust.h"
#include "adjust_internal.h"

int
main(int argc, char *argv[])
{
    if (argc != 2) {
	fprintf(stderr, "usage: %s filename\n", argv[0]);
	exit(EXIT_FAILURE);
    }

    struct file_info *info;
    info = file_info_new(argv[1]);

    printf("%s:%ld:%ld.%9ld\n", info->filename, info->size, info->mtime.tv_sec, info->mtime.tv_nsec);

    file_info_free(info);

    exit(EXIT_SUCCESS);
}
