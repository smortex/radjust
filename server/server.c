#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/stat.h>

#include "adjust.h"
#include "adjust_internal.h"

void	 receive_file(const char *filename, const struct file_info *remote_info);
void	 adjust_file(const char *filename, const struct file_info *remote_info);
void	 adjust_file_size(const int fd, const off_t size);
void	 adjust_file_content(const int fd);
void	 adjust_file_mtime(const int fd, const struct timespec mtime);

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

    receive_file(argv[1], remote_info);

    exit(EXIT_SUCCESS);
}

void
receive_file(const char *filename, const struct file_info *remote_info)
{
    struct file_info *local_info;

    if ((local_info = file_info_new(filename))) {
	if (0 == file_info_cmp(local_info, remote_info)) {
	    warnx("file match");
	    return;
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

    adjust_file(filename, remote_info);
}

void
adjust_file(const char *filename, const struct file_info *remote_info)
{
    int fd;
    if ((fd = open(filename, O_RDWR | O_CREAT, 0666)) < 0)
	err(EXIT_FAILURE, "open");

    adjust_file_size(fd, remote_info->size);
    adjust_file_content(fd);
    adjust_file_mtime(fd, remote_info->mtime);

    close(fd);
}

void
adjust_file_size(const int fd, const off_t size)
{
    if (ftruncate(fd, size) < 0)
	err(EXIT_FAILURE, "ftruncate");
}

void
adjust_file_content(const int fd)
{
    (void) fd;
    /* FIXME */
}

void
adjust_file_mtime(const int fd, const struct timespec mtime)
{
    struct timespec times[] = {
	{ 0, UTIME_OMIT },
	mtime,
    };

    if (futimens(fd, times) < 0)
	err(EXIT_FAILURE, "futimens");
}
