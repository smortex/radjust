#ifndef _ADJUST_INTERNAL_H
#define _ADJUST_INTERNAL_H

#include <sys/stat.h>

struct file_info {
    char *filename;
    off_t size;
    struct timespec mtime;
};

#endif /* !_ADJUST_INTERNAL_H */
