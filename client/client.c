#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>

#include "adjust.h"
#include "adjust_internal.h"

int		 xfer_file(const char *filename) __attribute__((warn_unused_result));
int		 send_file(const int fd, struct file_info *file) __attribute__((warn_unused_result));

int		 file_send_content(const int fd, struct file_info *file) __attribute__((warn_unused_result));

int
main(int argc, char *argv[])
{
    if (argc != 2) {
	fprintf(stderr, "usage: %s filename\n", argv[0]);
	exit(EXIT_FAILURE);
    }

    if (xfer_file(argv[1]) < 0)
	err(EXIT_FAILURE, "xfer_file");

    exit(EXIT_SUCCESS);
}

int
xfer_file(const char *filename)
{
    struct file_info *info;
    if (!(info = file_info_new(filename)))
	return -1;

    if (file_open(info, O_RDONLY) < 0)
	return -1;

    int sock = socket(PF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un address;

    address.sun_family = PF_UNIX;
    strcpy(address.sun_path, "socket");

    int res = connect(sock, (struct sockaddr *)&address, sizeof(address));

    if (res < 0)
	err(EXIT_FAILURE, "connect");


    char buffer[BUFSIZ];
    sprintf(buffer, "%s:%ld:%ld.%9ld\n", info->filename, info->size, info->mtime.tv_sec, info->mtime.tv_nsec);

    send(sock, buffer, strlen(buffer), 0);

    if (send_file(sock, info) < 0)
	err(EXIT_FAILURE, "send_file");

    if (file_close(info) < 0)
	return -1;

    file_info_free(info);
    close(sock);

    int byte_send, byte_recv;

    get_xfer_stats(&byte_send, &byte_recv);
    printf("client: synchronized %ld bytes\n", info->size);
    printf("client: sent %d bytes, received %d bytes\n", byte_send, byte_recv);

    return 0;
}

int
send_file(const int fd, struct file_info *file)
{
    char buffer;
    recv(fd, &buffer, 1, 0);
    switch (buffer) {
    case ADJUST_FILE_UPTODATE:
	break;
    case ADJUST_FILE_MISMATCH:
	file->transfer_mode = TM_ADJUST;
	break;
    case ADJUST_FILE_MISSING:
	file->transfer_mode = TM_WHOLE_FILE;
	break;
    }

    return file_send_content(fd, file);
}

int
file_send_content(const int fd, struct file_info *file)
{
    switch (file->transfer_mode) {
    case TM_ADJUST:
	return send_file_adjustments(fd, file);
	break;
    case TM_WHOLE_FILE:
	return send_whole_file_content(fd, file);
	break;
    }

    return -1; /* NOTREACHED */
}
