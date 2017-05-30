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

    file_open(info, O_RDONLY);

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

    recv(sock, buffer, 1, 0);
    switch (buffer[0]) {
    case ADJUST_FILE_UPTODATE:
	break;
    case ADJUST_FILE_MISMATCH: {
	if (file_map_first_block(info) < 0)
	    err(EXIT_FAILURE, "file_map_first_block");

	send_changed_block_chunks(sock, info);

	bool finished = false;
	while (!finished) {
	    switch (file_map_next_block(info)) {
	    case -1:
		err(EXIT_FAILURE, "file_map_next_block");
		break;
	    case 0:
		finished = true;
		break;
	    case 1:
		send_changed_block_chunks(sock, info);
		break;
	    }
	}
    }
    break;
    case ADJUST_FILE_MISSING: {
	off_t data_sent = 0;
	int fd = open(filename, O_RDONLY);

	while (data_sent < info->size) {

	    int res = read(fd, buffer, MIN((off_t)sizeof(buffer), info->size - data_sent));

	    if (send(sock, buffer, res, 0) != res)
		err(EXIT_FAILURE, "send");
	    data_sent += res;
	}

	close(fd);
    }

    break;
    }

    int byte_send, byte_recv;

    get_xfer_stats(&byte_send, &byte_recv);
    printf("client: synchronized %ld bytes\n", info->size);
    printf("client: sent %d bytes, received %d bytes\n", byte_send, byte_recv);

    file_close(info);
    file_info_free(info);
    close(sock);

    return 0;
}
