#ifndef _ADJUST_H
#define _ADJUST_H

#define ADJUST_FILE_UPTODATE 0
#define ADJUST_FILE_MISMATCH 1
#define ADJUST_FILE_MISSING  2

struct file_info;

struct file_info	*file_info_alloc(void);
struct file_info	*file_info_new(const char *filename);

int			 file_info_cmp(const struct file_info *left, const struct file_info *right);

void			 file_info_free(struct file_info *info);

#endif /* !_ADJUST_H */
