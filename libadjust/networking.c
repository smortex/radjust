#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>

#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "adjust.h"
#include "adjust_internal.h"

int sock;

int
libadjust_socket_open_out(const int port)
{
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	FAIL(-1, "socket");

    struct sockaddr_in address;
    bzero(&address, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    int val = 1;
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val));

    if (connect(sock, (struct sockaddr *)&address, sizeof(address)) < 0)
	FAIL(-1, "connect");

    return 0;
}

int
libadjust_socket_open_in(void)
{
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	FAIL(-1, "socket");

    struct sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_port = 0;
    server_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    int val = 1;
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val));

    if (bind(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
	FAIL(-1, "bind");

    if (listen(sock, 1) < 0)
	FAIL(-1, "listen");

    socklen_t n = sizeof(server_address);
    if (getsockname(sock, (struct sockaddr *) &server_address, &n) < 0)
	FAIL(-1, "getsockname");

    return ntohs(server_address.sin_port);
}

int
libadjust_socket_open_in_accept(void)
{
    struct sockaddr_in client_address;
    socklen_t client_len = sizeof(client_address);
    int client_sock = accept(sock, (struct sockaddr *)&client_address, &client_len);

    if (client_sock < 0)
	FAIL(-1, "accept");

    close(sock);
    sock = client_sock;

    return 0;
}

void
libadjust_socket_close(void)
{
    close(sock);
}

int
send_end_of_file(const int fd, struct file_info *local)
{
    char buffer[4 * 1024];

    while (local->offset < local->size) {

	int res = read(local->fd, buffer, MIN((off_t)sizeof(buffer), local->size - local->offset));

	if (send_data(fd, buffer, res) != res)
	    FAILX(-1, "send_data");

	local->offset += res;
	stats.bytes_send_raw += res;
    }

    return 0;
}

int
recv_end_of_file(const int fd, struct file_info *local)
{
    char buffer[4 * 1024];

    while (local->offset < local->size) {
	int expect = MIN((off_t)sizeof(buffer), local->size - local->offset);
	int res;

	if ((res = recv_data(fd, buffer, expect)) != expect)
	    FAILX(-1, "recv_data");

	if (write(local->fd, buffer, res) != res)
	    FAIL(-1, "write");

	local->offset += res;
	stats.bytes_send_raw += res;
    }

    return 0;
}

int
send_file_adjustments(const int fd, struct file_info *local, const struct file_info *remote)
{
    if (!remote->size)
	return 0;

    off_t actual_size = local->size;
    local->size = MIN(local->size, remote->size);

    if (file_map_first_block(local) < 0)
	FAILX(-1, "file_map_first_block");

    if (send_block_adjustments(fd, local) < 0)
	FAILX(-1, "send_block_adjustments");

    bool finished = false;
    while (!finished) {
	switch (file_map_next_block(local)) {
	case -1:
	    FAILX(-1, "file_map_next_block");
	    break;
	case 0:
	    finished = true;
	    break;
	case 1:
	    if (send_block_adjustments(fd, local) < 0)
		FAILX(-1, "send_block_adjustments");
	    break;
	}
    }

    if (lseek(local->fd, local->size, SEEK_SET) != local->size)
	FAIL(-1, "lseek");

    local->size = actual_size;

    return 0;
}

int
recv_file_adjustments(const int fd, struct file_info *local, const struct file_info *remote)
{
    if (!local->size)
	return 0;

    off_t actual_size = local->size;
    local->size = MIN(local->size, remote->size);

    if (file_map_first_block(local) < 0)
	FAILX(-1, "file_map_first_block");

    if (recv_block_adjustments(fd, local) < 0)
	FAILX(-1, "recv_block_adjustments");

    bool finished = false;
    while (!finished) {
	switch (file_map_next_block(local)) {
	case -1:
	    FAILX(-1, "file_map_next_block");
	    break;
	case 0:
	    finished = true;
	    break;
	case 1:
	    if (recv_block_adjustments(fd, local) < 0)
		FAILX(-1, "recv_block_adjustments");
	    break;
	}
    }

    if (lseek(local->fd, local->size, SEEK_SET) != local->size)
	FAIL(-1, "lseek");

    local->size = actual_size;

    return 0;
}

int
send_block_adjustments(const int fd, const struct file_info *file)
{
    unsigned char local_sha256[32];
    sha256(file->data, file->data_size, local_sha256);

    if (send_data(fd, local_sha256, 32) != 32)
	FAILX(-1, "send_data");

    char buffer;
    if (recv_data(fd, &buffer, 1) != 1)
	FAILX(-1, "recv_data");

    if (buffer != 0) {
	unsigned char remote_sha256[32];
	for (int i = 0; i < (LARGE_BLOCK_SIZE / SMALL_BLOCK_SIZE) && file->offset + i * SMALL_BLOCK_SIZE < file->size; i++) {
	    int this_block_size = MIN(SMALL_BLOCK_SIZE, file->size - file->offset + i * SMALL_BLOCK_SIZE);
	    if (recv_data(fd, remote_sha256, 32) != 32)
		FAILX(-1, "recv_data");
	    sha256(file->data + i * SMALL_BLOCK_SIZE, this_block_size, local_sha256);

	    if (memcmp(local_sha256, remote_sha256, 32) == 0) {
		if (send_data(fd, "\0", 1) != 1)
		    FAILX(-1, "send_data");
	    } else {
		if (send_data(fd, "\1", 1) != 1)
		    FAILX(-1, "send_data");

		if (send_data(fd, file->data + i * SMALL_BLOCK_SIZE, this_block_size) != this_block_size)
		    FAILX(-1, "send_data");

		stats.adjusted_chunks += 1;
		stats.bytes_adjusted += this_block_size;
	    }
	}
	stats.adjusted_blocks += 1;
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
	FAILX(-1, "recv_data");

    if (memcmp(local_sha256, remote_sha256, 32) == 0) {
	if (send_data(fd, "\0", 1) != 1)
	    FAILX(-1, "send_data");
    } else {
	if (send_data(fd, "\1", 1) != 1)
	    FAILX(-1, "send_data");

	for (int i = 0; i < (LARGE_BLOCK_SIZE / SMALL_BLOCK_SIZE) && file->offset + i * SMALL_BLOCK_SIZE < file->size; i++) {
	    int this_block_size = MIN(SMALL_BLOCK_SIZE, file->size - file->offset + i * SMALL_BLOCK_SIZE);
	    sha256(file->data + i * SMALL_BLOCK_SIZE, this_block_size, local_sha256);
	    if (send_data(fd, local_sha256, 32) != 32)
		FAILX(-1, "send_data");

	    char buffer;
	    if (recv_data(fd, &buffer, 1) != 1)
		FAILX(-1, "recv_data");

	    if (buffer == 1) {
		if (recv_data(fd, file->data + i * SMALL_BLOCK_SIZE, this_block_size) != this_block_size)
		    FAILX(-1, "recv_data");

		stats.adjusted_chunks += 1;
		stats.bytes_adjusted += this_block_size;
	    }
	}
	stats.adjusted_blocks += 1;
    }

    return 0;
}

int
recv_line(char *buffer, size_t length)
{
    char *p = buffer;
    char *e = buffer + length;

    if (recv_data(sock, p, 1) != 1)
	FAILX(-1, "recv_data");

    while (*p != '\n') {
	p++;

	if (p > e)
	    FAILX(-1, "end of buffer reached");

	if (recv_data(sock, p, 1) != 1)
	    FAILX(-1, "recv_data");
    }
    *p = '\0';

    return 0;
}

int
send_data(int fd, void *data, size_t length)
{
    assert(length > 0);

    int res = send(fd, data, length, 0);
    if (res < 0)
	FAIL(-1, "send");
    stats.bytes_send += res;
    return res;
}

int
peek_data(int fd, void *data, size_t length)
{
    assert(length > 0);

    int res = recv(fd, data, length, MSG_WAITALL | MSG_PEEK);
    if (res < 0)
	FAIL(-1, "recv");
    stats.bytes_recv += res;
    return res;
}

int
recv_data(int fd, void *data, size_t length)
{
    assert(length > 0);

    int res = recv(fd, data, length, MSG_WAITALL);
    if (!res)
	FAILX(-1, "recv: Connection reset by peer");
    if (res < 0)
	FAIL(-1, "recv");
    stats.bytes_recv += res;
    return res;
}
