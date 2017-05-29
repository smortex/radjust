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
void	 adjust_file(const char *filename, const struct file_info *remote_info, int client);
void	 adjust_file_size(const int fd, const off_t size);
void	 adjust_file_content(const int fd, const int size, int client);
void	 adjust_file_mtime(const int fd, const struct timespec mtime);

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
	    warnx("need adjusting");
	}
	file_info_free(local_info);
    } else {
	if (errno == ENOENT) {
	    answer = ADJUST_FILE_MISSING;
	    warnx("destination file does not exist");
	} else {
	    err(EXIT_FAILURE, "file_info_new");
	}
    }

    send(client, &answer, 1, 0);

    adjust_file(filename, remote_info, client);
}

void
adjust_file(const char *filename, const struct file_info *remote_info, int client)
{
    int fd;
    if ((fd = open(filename, O_RDWR | O_CREAT, 0666)) < 0)
	err(EXIT_FAILURE, "open");

    adjust_file_size(fd, remote_info->size);
    adjust_file_content(fd, remote_info->size, client);
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
adjust_file_content(const int fd, const int size, int client)
{
    char buffer[BUFSIZ];
    int total = 0;

    while (total < size) {
	int n = recv(client, buffer, sizeof(buffer), 0);
	if (write(fd, buffer, n) != n)
	    err(EXIT_FAILURE, "write");

	total += n;
    }
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
