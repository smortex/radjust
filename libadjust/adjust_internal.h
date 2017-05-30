#ifndef _ADJUST_INTERNAL_H
#define _ADJUST_INTERNAL_H

#include <sys/stat.h>

#define LARGE_BLOCK_SIZE (16 * 1024 * 1024)
#define SMALL_BLOCK_SIZE (4 * 1024)

struct file_info {
    char *filename;
    off_t size;
    struct timespec mtime;

    enum {TM_WHOLE_FILE, TM_DELTA} transfer_mode;

    int fd;
    off_t offset;
    void *data;
    size_t data_size;
    int mmap_mode;
};

int		 file_open(struct file_info *file, int mode);
int		 file_close(struct file_info *file);

void		 sha256(void *data, size_t length, unsigned char digest[32]);

void		 send_changed_block_chunks(int fd, struct file_info *file);
void		 recv_changed_block_chunks(int fd, struct file_info *file);

int		 file_map_first_block(struct file_info *file);
int		 file_map_next_block(struct file_info *file);
int		 file_set_size(struct file_info *file, const off_t size);
int		 file_set_mtime(const struct file_info *file, const struct timespec mtime);

#endif /* !_ADJUST_INTERNAL_H */
