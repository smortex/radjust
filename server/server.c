#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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

    char buffer[BUFSIZ];
    fgets(buffer, sizeof(buffer), stdin);

    char filename[BUFSIZ];

    struct file_info *remote_info;
    remote_info = file_info_alloc();
    remote_info->filename = filename;

    if (sscanf(buffer, "%[^:]:%ld:%ld.%9ld", filename, &remote_info->size, &remote_info->mtime.tv_sec, &remote_info->mtime.tv_nsec) != 4) {
	perror("sscanf");
	exit(EXIT_FAILURE);
    }

    struct file_info *local_info;

    if ((local_info = file_info_new(argv[1]))) {
	if (0 == file_info_cmp(local_info, remote_info)) {
	    warnx("file match");
	    exit(EXIT_SUCCESS);
	} else {
	    warnx("need adjusting");
	}
    } else {
	if (errno == ENOENT) {
	    warnx("destination file does not exist");
	} else {
	    err(EXIT_FAILURE, "file_info_new");
	}
    }

    int fd;
    if ((fd = open(argv[1], O_RDWR | O_CREAT, 0666)) < 0)
	err(EXIT_FAILURE, "open");

    if (ftruncate(fd, remote_info->size) < 0)
	err(EXIT_FAILURE, "ftruncate");

    /* FIXME Sync content */

    struct timespec times[] = {
	{ 0, UTIME_OMIT },
	remote_info->mtime,
    };

    futimens(fd, times);
    close(fd);

    exit(EXIT_SUCCESS);
}
