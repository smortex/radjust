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

void	 receive_file(const int fd, const char *filename, const struct file_info *remote_info);
void	 adjust_file(const int fd, struct file_info *local_info, const struct file_info *remote_info);

int	 file_recv_content(const int fd, struct file_info *file);

int	 recv_changed_chunks(const int fd, struct file_info *file);
int	 recv_whole_file_content(const int fd, struct file_info *file);

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

    receive_file(client_sock, argv[1], remote_info);

    unlink("socket");
    file_info_free(remote_info);

    exit(EXIT_SUCCESS);
}

void
receive_file(const int fd, const char *filename, const struct file_info *remote_info)
{
    struct file_info *local_info;

    char answer;

    if ((local_info = file_info_new(filename))) {
	if (0 == file_info_cmp(local_info, remote_info)) {
	    answer = ADJUST_FILE_UPTODATE;
	    warnx("file match");
	    return;
	} else {
	    answer = ADJUST_FILE_MISMATCH;
	    local_info->transfer_mode = TM_DELTA;
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
	    err(EXIT_FAILURE, "file_info_new");
	}
    }

    send(fd, &answer, 1, 0);

    adjust_file(fd, local_info, remote_info);
    file_info_free(local_info);
}

void
adjust_file(const int fd, struct file_info *local_info, const struct file_info *remote_info)
{
    if (file_open(local_info, O_RDWR | O_CREAT) < 0)
	err(EXIT_FAILURE, "open");

    file_set_size(local_info, remote_info->size);

    file_recv_content(fd, local_info);

    file_set_mtime(local_info, remote_info->mtime);

    file_close(local_info);
}


int
file_recv_content(const int fd, struct file_info *file)
{
    switch (file->transfer_mode) {
    case TM_DELTA:
	return recv_changed_chunks(fd, file);
	break;
    case TM_WHOLE_FILE:
	return recv_whole_file_content(fd, file);
	break;
    }

    return -1; /* NOTREACHED */
}

int
recv_changed_chunks(const int fd, struct file_info *file)
{
    if (file_map_first_block(file) < 0)
	return -1;

    recv_changed_block_chunks(fd, file);

    bool finished = false;
    while (!finished) {
	switch (file_map_next_block(file)) {
	case -1:
	    return -1;
	    break;
	case 0:
	    finished = true;
	    break;
	case 1:
	    recv_changed_block_chunks(fd, file);
	    break;
	}
    }

    return 0;
}

int
recv_whole_file_content(const int fd, struct file_info *file)
{
    char buffer[BUFSIZ];
    int total = 0;

    while (total < file->size) {
	int n = recv(fd, buffer, sizeof(buffer), 0);
	if (write(file->fd, buffer, n) != n)
	    err(EXIT_FAILURE, "write");

	total += n;
    }

    return 0;
}
