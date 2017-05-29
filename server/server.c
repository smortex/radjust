#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>

#include "adjust.h"
#include "adjust_internal.h"

void	 receive_file(const char *filename, const struct file_info *remote_info, int client);
void	 adjust_file(struct file_info *local_info, const struct file_info *remote_info, int client);
void	 adjust_file_size(struct file_info *file, const off_t size);
void	 adjust_file_content(struct file_info *file, int client);
void	 adjust_file_mtime(const struct file_info *file, const struct timespec mtime);

void	 recv_changed_chunks(struct file_info *file, int client);
void	 recv_whole_file_content(struct file_info *file, int client);

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

    receive_file(argv[1], remote_info, client_sock);

    unlink("socket");
    file_info_free(remote_info);

    exit(EXIT_SUCCESS);
}

void
receive_file(const char *filename, const struct file_info *remote_info, int client)
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

    send(client, &answer, 1, 0);

    adjust_file(local_info, remote_info, client);
    file_info_free(local_info);
}

void
adjust_file(struct file_info *local_info, const struct file_info *remote_info, int client)
{
    if (file_open(local_info, O_RDWR | O_CREAT) < 0)
	err(EXIT_FAILURE, "open");

    adjust_file_size(local_info, remote_info->size);

    adjust_file_content(local_info, client);

    adjust_file_mtime(local_info, remote_info->mtime);

    file_close(local_info);
}

void
adjust_file_size(struct file_info *file, const off_t size)
{
    if (ftruncate(file->fd, size) < 0)
	err(EXIT_FAILURE, "ftruncate");

    file->size = size;
}

void
adjust_file_content(struct file_info *file, int client)
{
    switch (file->transfer_mode) {
    case TM_DELTA:
	recv_changed_chunks(file, client);
	break;
    case TM_WHOLE_FILE:
	recv_whole_file_content(file, client);
	break;
    }
}

void
recv_changed_chunks(struct file_info *file, int client)
{
    map_first_block(file);
    recv_changed_block_chunks(client, file);

    while (map_next_block(file))
	recv_changed_block_chunks(client, file);
}

void
recv_whole_file_content(struct file_info *file, int client)
{
    char buffer[BUFSIZ];
    int total = 0;

    while (total < file->size) {
	int n = recv(client, buffer, sizeof(buffer), 0);
	if (write(file->fd, buffer, n) != n)
	    err(EXIT_FAILURE, "write");

	total += n;
    }
}

void
adjust_file_mtime(const struct file_info *file, const struct timespec mtime)
{
    fsync(file->fd);

    struct timespec times[] = {
	{ 0, UTIME_OMIT },
	mtime,
    };

    if (futimens(file->fd, times) < 0)
	err(EXIT_FAILURE, "futimens");
}
