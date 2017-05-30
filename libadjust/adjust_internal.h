#ifndef _ADJUST_INTERNAL_H
#define _ADJUST_INTERNAL_H

#include <sys/stat.h>

#define ADJUST_FILE_UPTODATE 0
#define ADJUST_FILE_MISMATCH 1
#define ADJUST_FILE_MISSING  2

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

struct file_info	*file_info_alloc(void) __attribute__((warn_unused_result));
struct file_info	*file_info_new(const char *filename) __attribute__((warn_unused_result));

int			 file_info_cmp(const struct file_info *left, const struct file_info *right) __attribute__((warn_unused_result));

void			 file_info_free(struct file_info *info);

int			 file_open(struct file_info *file, int mode) __attribute__((warn_unused_result));
int			 file_close(struct file_info *file) __attribute__((warn_unused_result));

int			 send_whole_file_content(const int fd, struct file_info *file) __attribute__((warn_unused_result));
int			 recv_whole_file_content(const int fd, struct file_info *file) __attribute__((warn_unused_result));

int			 send_file_adjustments(const int fd, struct file_info *file) __attribute__((warn_unused_result));
int			 recv_file_adjustments(const int fd, struct file_info *file) __attribute__((warn_unused_result));

int			 send_block_adjustments(const int fd, const struct file_info *file) __attribute__((warn_unused_result));
int			 recv_block_adjustments(const int fd, const struct file_info *file) __attribute__((warn_unused_result));

int			 file_map_first_block(struct file_info *file) __attribute__((warn_unused_result));
int			 file_map_next_block(struct file_info *file) __attribute__((warn_unused_result));
int			 file_set_size(struct file_info *file, const off_t size) __attribute__((warn_unused_result));
int			 file_set_mtime(const struct file_info *file, const struct timespec mtime) __attribute__((warn_unused_result));

int			 file_send(const int fd, struct file_info *file) __attribute__((warn_unused_result));
int			 file_recv(const int fd, struct file_info *local, const struct file_info *remote) __attribute__((warn_unused_result));

int			 file_send_content(const int fd, struct file_info *file) __attribute__((warn_unused_result));
int			 file_recv_content(const int fd, struct file_info *file) __attribute__((warn_unused_result));

void			 sha256(const void *data, const size_t length, unsigned char digest[32]);

int			 recv_data(int fd, void *data, size_t length) __attribute__((warn_unused_result));
int			 send_data(int fd, void *data, size_t length) __attribute__((warn_unused_result));

#endif /* !_ADJUST_INTERNAL_H */
