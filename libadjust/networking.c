#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>

#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "adjust.h"
#include "adjust_internal.h"

static int byte_send = 0;
static int byte_recv = 0;

int sock;

int
libadjust_connect(void)
{
    if ((sock = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
	return -1;

    struct sockaddr_un address;

    address.sun_family = PF_UNIX;
    strcpy(address.sun_path, "socket");

    if (connect(sock, (struct sockaddr *)&address, sizeof(address)) < 0)
	return -1;

    return 0;
}

int
libadjust_serve(void)
{
    if ((sock = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
	return -1;

    struct sockaddr_un server_address;
    server_address.sun_family = PF_UNIX;
    strcpy(server_address.sun_path, "socket");

    if (bind(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
	return -1;

    if (listen(sock, 1) < 0)
	return -1;

    struct sockaddr_un client_address;
    socklen_t client_len = sizeof(client_address);
    int client_sock = accept(sock, (struct sockaddr *)&client_address, &client_len);

    close(sock);
    sock = client_sock;

    return 0;
}

void
libadjust_terminate(void)
{
    close(sock);
}

int
send_whole_file_content(const int fd, struct file_info *file)
{
    off_t data_sent = 0;

    char buffer[4 * 1024];

    while (data_sent < file->size) {

	int res = read(file->fd, buffer, MIN((off_t)sizeof(buffer), file->size - data_sent));

	if (send_data(fd, buffer, res) != res)
	    return -1;

	data_sent += res;
    }

    return 0;
}

int
recv_whole_file_content(const int fd, struct file_info *file)
{
    char buffer[4 * 1024];
    int total = 0;

    while (total < file->size) {
	int n = recv(fd, buffer, sizeof(buffer), 0);
	if (write(file->fd, buffer, n) != n)
	    return -1;

	total += n;
    }

    return 0;
}

int
send_file_adjustments(const int fd, struct file_info *file)
{
    if (file_map_first_block(file) < 0)
	return -1;

    if (send_block_adjustments(fd, file) < 0)
	return -1;

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
	    if (send_block_adjustments(fd, file) < 0)
		return -1;
	    break;
	}
    }

    return 0;
}

int
recv_file_adjustments(const int fd, struct file_info *file)
{
    if (file_map_first_block(file) < 0)
	return -1;

    if (recv_block_adjustments(fd, file) < 0)
	return -1;

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
	    if (recv_block_adjustments(fd, file) < 0)
		return -1;
	    break;
	}
    }

    return 0;
}

int
send_block_adjustments(const int fd, const struct file_info *file)
{
    unsigned char local_sha256[32];
    sha256(file->data, file->data_size, local_sha256);

    if (send_data(fd, local_sha256, 32) != 32)
	return -1;

    char buffer;
    if (recv_data(fd, &buffer, 1) != 1)
	return -1;

    if (buffer != 0) {
	unsigned char remote_sha256[32];
	for (uint32_t i = 0; i < (LARGE_BLOCK_SIZE / SMALL_BLOCK_SIZE) && file->offset + i * SMALL_BLOCK_SIZE < file->size; i++) {
	    int this_block_size = MIN(SMALL_BLOCK_SIZE, file->size - file->offset + i * SMALL_BLOCK_SIZE);
	    if (recv_data(fd, remote_sha256, 32) != 32)
		return -1;
	    sha256(file->data + i * SMALL_BLOCK_SIZE, this_block_size, local_sha256);

	    if (memcmp(local_sha256, remote_sha256, 32) == 0) {
		if (send_data(fd, "\0", 1) != 1)
		    return -1;
	    } else {
		if (send_data(fd, "\1", 1) != 1)
		    return -1;

		if (send_data(fd, file->data + i * SMALL_BLOCK_SIZE, this_block_size) != this_block_size)
		    return -1;
	    }
	}
    }

    return 0;
}

int
recv_block_adjustments(const int fd, const struct file_info *file)
{
    unsigned char local_sha256[32];
    sha256(file->data, file->data_size, local_sha256);

    unsigned char remote_sha256[32];
    if (recv_data(fd, remote_sha256, 32) != 32)
	return -1;

    if (memcmp(local_sha256, remote_sha256, 32) == 0) {
	if (send_data(fd, "\0", 1) != 1)
	    return -1;
    } else {
	if (send_data(fd, "\1", 1) != 1)
	    return -1;

	for (uint32_t i = 0; i < (LARGE_BLOCK_SIZE / SMALL_BLOCK_SIZE) && file->offset + i * SMALL_BLOCK_SIZE < file->size; i++) {
	    int this_block_size = MIN(SMALL_BLOCK_SIZE, file->size - file->offset + i * SMALL_BLOCK_SIZE);
	    sha256(file->data + i * SMALL_BLOCK_SIZE, this_block_size, local_sha256);
	    if (send_data(fd, local_sha256, 32) != 32)
		return -1;

	    char buffer;
	    if (recv_data(fd, &buffer, 1) != 1)
		return -1;

	    if (buffer == 1) {
		if (recv_data(fd, file->data + i * SMALL_BLOCK_SIZE, this_block_size) != this_block_size)
		    return -1;
	    }
	}
    }

    return 0;
}

int
send_data(int fd, void *data, size_t length)
{
    int res = send(fd, data, length, 0);
    byte_send += res;
    return res;
}

int
recv_data(int fd, void *data, size_t length)
{
    int res = recv(fd, data, length, 0);
    byte_recv += res;
    return res;
}

void
get_xfer_stats(int *send, int *recv)
{
    if (send)
	*send = byte_send;
    if (recv)
	*recv = byte_recv;
}
