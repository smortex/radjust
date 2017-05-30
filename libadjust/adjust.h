#ifndef _ADJUST_H
#define _ADJUST_H

#define ADJUST_FILE_UPTODATE 0
#define ADJUST_FILE_MISMATCH 1
#define ADJUST_FILE_MISSING  2

int			 libadjust_connect(void) __attribute__((warn_unused_result));
int			 libadjust_serve(void) __attribute__((warn_unused_result));

int			 libadjust_send_file(char *filename);
int			 libadjust_recv_file(char *filename);

void			 libadjust_terminate(void);

struct file_info;

struct file_info	*file_info_alloc(void) __attribute__((warn_unused_result));
struct file_info	*file_info_new(const char *filename) __attribute__((warn_unused_result));

int			 file_info_cmp(const struct file_info *left, const struct file_info *right) __attribute__((warn_unused_result));

void			 file_info_free(struct file_info *info);

void			 get_xfer_stats(size_t *bytes);
void			 get_networking_stats(size_t *send, size_t *recv);

#endif /* !_ADJUST_H */
