#include <string.h>
#include <inttypes.h>

#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "adjust.h"
#include "adjust_internal.h"

static void		 recv_data(int fd, void *data, size_t length);
static void		 send_data(int fd, void *data, size_t length);

static int byte_send = 0;
static int byte_recv = 0;

void
send_changed_block_chunks(int fd, struct file_info *file)
{
    unsigned char local_sha256[32];
    sha256(file->data, file->data_size, local_sha256);

    send_data(fd, local_sha256, 32);

    char buffer;
    recv_data(fd, &buffer, 1);
    if (buffer != 0) {
	unsigned char remote_sha256[32];
	for (uint32_t i = 0; i < (LARGE_BLOCK_SIZE / SMALL_BLOCK_SIZE) && file->offset + i * SMALL_BLOCK_SIZE < file->size; i++) {
	    int this_block_size = MIN(SMALL_BLOCK_SIZE, file->size - file->offset + i * SMALL_BLOCK_SIZE);
	    recv_data(fd, remote_sha256, 32);
	    sha256(file->data + i * SMALL_BLOCK_SIZE, this_block_size, local_sha256);

	    if (memcmp(local_sha256, remote_sha256, 32) == 0) {
		send_data(fd, "\0", 1);
	    } else {
		send_data(fd, "\1", 1);
		send_data(fd, file->data + i * SMALL_BLOCK_SIZE, this_block_size);
	    }
	}
    }
}

void
recv_changed_block_chunks(int fd, struct file_info *file)
{
    unsigned char local_sha256[32];
    sha256(file->data, file->data_size, local_sha256);

    unsigned char remote_sha256[32];
    recv_data(fd, remote_sha256, 32);

    if (memcmp(local_sha256, remote_sha256, 32) == 0) {
	send_data(fd, "\0", 1);
    } else {
	send_data(fd, "\1", 1);

	for (uint32_t i = 0; i < (LARGE_BLOCK_SIZE / SMALL_BLOCK_SIZE) && file->offset + i * SMALL_BLOCK_SIZE < file->size; i++) {
	    int this_block_size = MIN(SMALL_BLOCK_SIZE, file->size - file->offset + i * SMALL_BLOCK_SIZE);
	    sha256(file->data + i * SMALL_BLOCK_SIZE, this_block_size, local_sha256);
	    send_data(fd, local_sha256, 32);

	    char buffer;
	    recv_data(fd, &buffer, 1);
	    if (buffer == 1) {
		recv_data(fd, file->data + i * SMALL_BLOCK_SIZE, this_block_size);
	    }
	}
    }
}

static void
send_data(int fd, void *data, size_t length)
{
    send(fd, data, length, 0);
    byte_send += length;
}

static void
recv_data(int fd, void *data, size_t length)
{
    recv(fd, data, length, 0);
    byte_recv += length;
}

void
get_xfer_stats(int *send, int *recv)
{
    if (send)
	*send = byte_send;
    if (recv)
	*recv = byte_recv;
}
