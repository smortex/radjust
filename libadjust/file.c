#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/param.h>

#if defined(__FreeBSD__)
#include <sha256.h>
#elif defined(__linux__)
#  include <openssl/sha.h>
#endif

#include "adjust.h"
#include "adjust_internal.h"

static int		 map_current_block(struct file_info *file);
static int		 unmap_current_block(struct file_info *file);
static int		 receive_file_data(const int fd, const char *filename, const struct file_info *remote_info);

extern int sock;
static size_t synchronized;

int
libadjust_send_file(char *filename)
{
    struct file_info *info;
    if (!(info = file_info_new(filename)))
	return -1;

    if (file_open(info, O_RDONLY) < 0)
	return -1;

    char buffer[BUFSIZ];
    sprintf(buffer, "%s:%ld:%ld.%9ld\n", info->filename, info->size, info->mtime.tv_sec, info->mtime.tv_nsec);

    if (send_data(sock, buffer, strlen(buffer)) != (int) strlen(buffer))
	return -1;

    if (file_send(sock, info) < 0)
	return -1;

    if (file_close(info) < 0)
	return -1;

    synchronized += info->size;
    file_info_free(info);

    return 0;
}

int
libadjust_recv_file(char *filename)
{
    char buffer[BUFSIZ];
    char *p = buffer;

    if (recv_data(sock, p, 1) != 1)
	return -1;

    while (*p != '\n') {
	p++;
	if (recv_data(sock, p, 1) != 1)
	    return -1;
    }
    *p = '\0';

    char remote_filename[BUFSIZ];

    struct file_info *remote_info;
    remote_info = file_info_alloc();

    if (sscanf(buffer, "%[^:]:%ld:%ld.%9ld", remote_filename, &remote_info->size, &remote_info->mtime.tv_sec, &remote_info->mtime.tv_nsec) != 4)
	return -1;

    remote_info->filename = strdup(filename);

    if (receive_file_data(sock, filename, remote_info) < 0)
	return -1;

    libadjust_terminate();
    unlink("socket");
    synchronized += remote_info->size;
    file_info_free(remote_info);

    return 0;
}

int
receive_file_data(const int fd, const char *filename, const struct file_info *remote_info)
{
    int res = 0;
    struct file_info *local_info;
    char answer;

    if ((local_info = file_info_new(filename))) {
	if (0 == file_info_cmp(local_info, remote_info)) {
	    answer = ADJUST_FILE_UPTODATE;
	    // warnx("file match");
	    return res;
	} else {
	    answer = ADJUST_FILE_MISMATCH;
	    local_info->transfer_mode = TM_ADJUST;
	    // warnx("need adjusting");
	}
    } else {
	if (errno == ENOENT) {
	    answer = ADJUST_FILE_MISSING;
	    local_info = file_info_alloc();
	    local_info->filename = strdup(filename);
	    local_info->transfer_mode = TM_WHOLE_FILE;
	    // warnx("destination file does not exist");
	} else {
	    return -1;
	}
    }

    if (send_data(fd, &answer, 1) != 1)
	return -1;

    if (file_recv(fd, local_info, remote_info) < 0)
	res = -1;

    file_info_free(local_info);

    return res;
}

void
sha256(const void *data, const size_t length, unsigned char digest[32])
{
    SHA256_CTX sha256;

    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data, length);
    SHA256_Final(digest, &sha256);
}

int
file_open(struct file_info *file, int mode)
{
    if ((file->fd = open(file->filename, mode, 0666))) {
	file->mmap_mode = PROT_READ;
	if ((mode & O_RDWR) == O_RDWR)
	    file->mmap_mode |= PROT_WRITE;
    }
    return file->fd;
}

static int
map_current_block(struct file_info *file)
{
    file->data = mmap(NULL, file->data_size, file->mmap_mode, MAP_SHARED, file->fd, file->offset);

    if (file->data == MAP_FAILED)
	return -1;

    return 0;
}

static int
unmap_current_block(struct file_info *file)
{
    return munmap(file->data, file->data_size);
}

int
file_map_first_block(struct file_info *file)
{
    file->data_size = MIN(LARGE_BLOCK_SIZE, file->size);
    file->offset = 0;
    return map_current_block(file);
}

int
file_map_next_block(struct file_info *file)
{
    if (unmap_current_block(file) < 0)
	return -1;

    file->offset += file->data_size;

    if (file->offset == file->size)
	return 0;

    file->data_size = MIN(LARGE_BLOCK_SIZE, file->size - file->offset);
    if (map_current_block(file) < 0)
	return -1;

    return 1;
}

int
file_close(struct file_info *file)
{
    unmap_current_block(file);
    return close(file->fd);
}

int
file_set_size(struct file_info *file, const off_t size)
{
    if (ftruncate(file->fd, size) < 0)
	return -1;

    file->size = size;

    return 0;
}

int
file_set_mtime(const struct file_info *file, const struct timespec mtime)
{
    if (fsync(file->fd) < 0)
	return -1;

    struct timespec times[] = {
	{ 0, UTIME_OMIT },
	mtime,
    };

    if (futimens(file->fd, times) < 0)
	return -1;

    return 0;
}

int
file_send(const int fd, struct file_info *file)
{
    char buffer;
    if (recv_data(fd, &buffer, 1) < 0)
	return -1;

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
file_recv(const int fd, struct file_info *local, const struct file_info *remote)
{
    if (file_open(local, O_RDWR | O_CREAT) < 0)
	return -1;

    if (file_set_size(local, remote->size) < 0)
	return -1;

    if (file_recv_content(fd, local) < 0)
	return -1;

    if (file_set_mtime(local, remote->mtime) < 0)
	return -1;

    if (file_close(local) < 0)
	return -1;

    return 0;
}

int
file_send_content(const int fd, struct file_info *file)
{
    int res;

    switch (file->transfer_mode) {
    case TM_ADJUST:
	res = send_file_adjustments(fd, file);
	break;
    case TM_WHOLE_FILE:
	res = send_whole_file_content(fd, file);
	break;
    }

    return res;
}

int
file_recv_content(const int fd, struct file_info *file)
{
    int res;
    switch (file->transfer_mode) {
    case TM_ADJUST:
	res = recv_file_adjustments(fd, file);
	break;
    case TM_WHOLE_FILE:
	res = recv_whole_file_content(fd, file);
	break;
    }

    return res;
}

void
get_xfer_stats(size_t *bytes)
{
    *bytes = synchronized;
}
