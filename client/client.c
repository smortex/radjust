#include <stdio.h>
#include <stdlib.h>

#include <sys/stat.h>

#include "adjust.h"
#include "adjust_internal.h"

int		 send_file(const char *filename);

int
main(int argc, char *argv[])
{
    if (argc != 2) {
	fprintf(stderr, "usage: %s filename\n", argv[0]);
	exit(EXIT_FAILURE);
    }

    send_file(argv[1]);

    exit(EXIT_SUCCESS);
}

int
send_file(const char *filename)
{
    struct file_info *info;
    if (!(info = file_info_new(filename)))
	return -1;

    printf("%s:%ld:%ld.%9ld\n", info->filename, info->size, info->mtime.tv_sec, info->mtime.tv_nsec);

    file_info_free(info);
    return 0;
}
