#ifndef _ADJUST_INTERNAL_H
#define _ADJUST_INTERNAL_H

#include <sys/stat.h>

#define LARGE_BLOCK_SIZE (16 * 1024 * 1024)
#define SMALL_BLOCK_SIZE (4 * 1024)

struct file_info {
    char *filename;
    off_t size;
    struct timespec mtime;

    enum {TM_WHOLE_FILE, TM_ADJUST} transfer_mode;

    int fd;
    off_t offset;
    void *data;
    size_t data_size;
    int mmap_mode;
};

int		 file_open(struct file_info *file, int mode) __attribute__((warn_unused_result));
int		 file_close(struct file_info *file) __attribute__((warn_unused_result));

void		 sha256(const void *data, const size_t length, unsigned char digest[32]);

int		 send_file_adjustments(const int fd, struct file_info *file) __attribute__((warn_unused_result));
int		 recv_file_adjustments(const int fd, struct file_info *file) __attribute__((warn_unused_result));

void		 send_block_adjustments(const int fd, const struct file_info *file);
void		 recv_block_adjustments(const int fd, const struct file_info *file);

int		 file_map_first_block(struct file_info *file) __attribute__((warn_unused_result));
int		 file_map_next_block(struct file_info *file) __attribute__((warn_unused_result));
int		 file_set_size(struct file_info *file, const off_t size) __attribute__((warn_unused_result));
int		 file_set_mtime(const struct file_info *file, const struct timespec mtime) __attribute__((warn_unused_result));

#endif /* !_ADJUST_INTERNAL_H */
