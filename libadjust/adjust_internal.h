#ifndef _ADJUST_INTERNAL_H
#define _ADJUST_INTERNAL_H

#include <stdbool.h>

#include <sys/stat.h>

#define ADJUST_FILE_UPTODATE 0
#define ADJUST_FILE_MISMATCH 1
#define ADJUST_FILE_MISSING  2

#define LARGE_BLOCK_SIZE (16 * 1024 * 1024)
#define SMALL_BLOCK_SIZE (4 * 1024)

#define _FAIL(eval, print_errno, format, ...) do { error_push(__FILE__, __LINE__, __func__, print_errno, format, ## __VA_ARGS__); return eval; } while (0)
#define FAIL(eval, format, ...) do { _FAIL(eval, true, format, ## __VA_ARGS__); } while (0)
#define FAILX(eval, format, ...) do { _FAIL(eval, false, format, ## __VA_ARGS__); } while (0)

struct file_info {
    char *filename;
    off_t size;
    struct timespec mtime;

    int fd;
    off_t offset;
    void *data;
    size_t data_size;
    int mmap_mode;
};

struct libadjust_stats {
    size_t bytes_synchronized;
    size_t bytes_send;
    size_t bytes_recv;
    int adjusted_blocks;
    int adjusted_chunks;
};

extern struct libadjust_stats stats;

struct file_info	*file_info_alloc(void) __attribute__((warn_unused_result));
struct file_info	*file_info_new(const char *filename) __attribute__((warn_unused_result));

int			 file_info_cmp(const struct file_info *left, const struct file_info *right) __attribute__((warn_unused_result));

void			 file_info_free(struct file_info *info);

int			 file_open(struct file_info *file, int mode) __attribute__((warn_unused_result));
int			 file_close(struct file_info *file) __attribute__((warn_unused_result));

int			 send_whole_file_content(const int fd, struct file_info *local) __attribute__((warn_unused_result));
int			 recv_whole_file_content(const int fd, struct file_info *local) __attribute__((warn_unused_result));

int			 send_file_adjustments(const int fd, struct file_info *local, const struct file_info *remote) __attribute__((warn_unused_result));
int			 recv_file_adjustments(const int fd, struct file_info *local, const struct file_info *remote) __attribute__((warn_unused_result));

int			 send_block_adjustments(const int fd, const struct file_info *file) __attribute__((warn_unused_result));
int			 recv_block_adjustments(const int fd, const struct file_info *file) __attribute__((warn_unused_result));

int			 file_map_first_block(struct file_info *file) __attribute__((warn_unused_result));
int			 file_map_next_block(struct file_info *file) __attribute__((warn_unused_result));
int			 file_set_size(struct file_info *file, const off_t size) __attribute__((warn_unused_result));
int			 file_set_mtime(const struct file_info *file, const struct timespec mtime) __attribute__((warn_unused_result));

int			 file_send(const int fd, struct file_info *local, const struct file_info *remote) __attribute__((warn_unused_result));
int			 file_recv(const int fd, struct file_info *local, const struct file_info *remote) __attribute__((warn_unused_result));

int			 file_send_content(const int fd, struct file_info *local, const struct file_info *remote) __attribute__((warn_unused_result));
int			 file_recv_content(const int fd, struct file_info *local, const struct file_info *remote) __attribute__((warn_unused_result));

void			 sha256(const void *data, const size_t length, unsigned char digest[32]);

int			 recv_line(char *buffer, size_t length);

int			 recv_data(int fd, void *data, size_t length) __attribute__((warn_unused_result));
int			 send_data(int fd, void *data, size_t length) __attribute__((warn_unused_result));

int			 error_push(const char *file, const int lineno, const char *function, const bool print_errno, const char *format, ...);

#endif /* !_ADJUST_INTERNAL_H */
