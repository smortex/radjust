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

static void		 map_current_block(struct file_info *file);
static void		 unmap_current_block(struct file_info *file);

void
sha256(void *data, size_t length, unsigned char digest[32])
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

static void
map_current_block(struct file_info *file)
{
    file->data = mmap(NULL, file->data_size, file->mmap_mode, MAP_SHARED, file->fd, file->offset);
}

static void
unmap_current_block(struct file_info *file)
{
    munmap(file->data, file->data_size);
}

void
map_first_block(struct file_info *file)
{
    file->data_size = MIN(LARGE_BLOCK_SIZE, file->size);
    file->offset = 0;
    map_current_block(file);
}

int
map_next_block(struct file_info *file)
{
    unmap_current_block(file);
    file->offset += file->data_size;

    if (file->offset == file->size)
	return 0;

    file->data_size = MIN(LARGE_BLOCK_SIZE, file->size - file->offset);
    map_current_block(file);

    return 1;
}

int
file_close(struct file_info *file)
{
    unmap_current_block(file);
    return close(file->fd);
}
