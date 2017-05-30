#include <fcntl.h>
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
