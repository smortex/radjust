#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>

#include "adjust.h"
#include "adjust_internal.h"

int	 receive_file(const int fd, const char *filename, const struct file_info *remote_info) __attribute__((warn_unused_result));
int	 adjust_file(const int fd, struct file_info *local_info, const struct file_info *remote_info) __attribute__((warn_unused_result));

int	 file_recv_content(const int fd, struct file_info *file) __attribute__((warn_unused_result));

int
main(int argc, char *argv[])
{
    if (argc != 2) {
	fprintf(stderr, "usage: %s filename\n", argv[0]);
	exit(EXIT_FAILURE);
    }

    int server_sock = socket(PF_UNIX, SOCK_STREAM, 0);

    struct sockaddr_un server_address;
    server_address.sun_family = PF_UNIX;
    strcpy(server_address.sun_path, "socket");

    bind(server_sock, (struct sockaddr *)&server_address, sizeof(server_address));

    listen(server_sock, 1);

    struct sockaddr_un client_address;
    socklen_t client_len = sizeof(client_address);
    int client_sock = accept(server_sock, (struct sockaddr *)&client_address, &client_len);

    char buffer[BUFSIZ];
    char *p = buffer;

    recv(client_sock, p, 1, 0);
    while (*p != '\n') {
	p++;
	recv(client_sock, p, 1, 0);
    }


    char filename[BUFSIZ];

    struct file_info *remote_info;
    remote_info = file_info_alloc();
    remote_info->filename = strdup(filename);

    if (sscanf(buffer, "%[^:]:%ld:%ld.%9ld", filename, &remote_info->size, &remote_info->mtime.tv_sec, &remote_info->mtime.tv_nsec) != 4) {
	perror("sscanf");
	exit(EXIT_FAILURE);
    }

    if (receive_file(client_sock, argv[1], remote_info) < 0)
	err(EXIT_FAILURE, "receive_file");

    unlink("socket");
    file_info_free(remote_info);

    exit(EXIT_SUCCESS);
}

int
receive_file(const int fd, const char *filename, const struct file_info *remote_info)
{
    int res = 0;
    struct file_info *local_info;
    char answer;

    if ((local_info = file_info_new(filename))) {
	if (0 == file_info_cmp(local_info, remote_info)) {
	    answer = ADJUST_FILE_UPTODATE;
	    warnx("file match");
	    return res;
	} else {
	    answer = ADJUST_FILE_MISMATCH;
	    local_info->transfer_mode = TM_ADJUST;
	    warnx("need adjusting");
	}
    } else {
	if (errno == ENOENT) {
	    answer = ADJUST_FILE_MISSING;
	    local_info = file_info_alloc();
	    local_info->filename = strdup(filename);
	    local_info->transfer_mode = TM_WHOLE_FILE;
	    warnx("destination file does not exist");
	} else {
	    return -1;
	}
    }

    send(fd, &answer, 1, 0);

    if (adjust_file(fd, local_info, remote_info) < 0)
	res = -1;

    file_info_free(local_info);

    return res;
}

int
adjust_file(const int fd, struct file_info *local_info, const struct file_info *remote_info)
{
    if (file_open(local_info, O_RDWR | O_CREAT) < 0)
	err(EXIT_FAILURE, "open");

    if (file_set_size(local_info, remote_info->size) < 0)
	return -1;

    if (file_recv_content(fd, local_info) < 0)
	return -1;

    if (file_set_mtime(local_info, remote_info->mtime) < 0)
	return -1;

    if (file_close(local_info) < 0)
	return -1;

    return 0;
}


int
file_recv_content(const int fd, struct file_info *file)
{
    switch (file->transfer_mode) {
    case TM_ADJUST:
	return recv_file_adjustments(fd, file);
	break;
    case TM_WHOLE_FILE:
	return recv_whole_file_content(fd, file);
	break;
    }

    return -1; /* NOTREACHED */
}
