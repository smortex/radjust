#ifndef _ADJUST_H
#define _ADJUST_H

struct file_info;

struct file_info	*file_info_alloc(void);
struct file_info	*file_info_new(char *filename);

int			 file_info_cmp(struct file_info *left, struct file_info *right);

void			 file_info_free(struct file_info *info);

#endif /* !_ADJUST_H */
