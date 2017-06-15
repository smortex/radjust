#include <stdlib.h>
#include <string.h>

#include "adjust.h"

#include "adjust_internal.h"

struct file_info *
file_info_alloc(void)
{
    struct file_info *res;
    res = malloc(sizeof(*res));
    res->filename = NULL;
    res->fd = 0;
    res->size = 0;
    res->offset = 0;
    res->data = NULL;
    return res;
}

struct file_info *
file_info_new(const char *filename)
{
    struct stat sb;
    if (stat(filename, &sb) < 0) {
	return NULL;
    }

    struct file_info *res;
    if ((res = file_info_alloc())) {
	res->filename = strdup(filename);
	res->size = sb.st_size;
	res->mtime = sb.st_mtim;
	res->type = S_ISDIR(sb.st_mode) ? T_DIRECTORY : T_FILE;
    }

    return res;
}

int
file_info_cmp(const struct file_info *left, const struct file_info *right)
{
    if (left->size != right->size)
	return left->size - right->size;
    if (left->mtime.tv_sec != right->mtime.tv_sec)
	return left->mtime.tv_sec - right->mtime.tv_sec;
    if (left->mtime.tv_nsec != right->mtime.tv_nsec)
	return left->mtime.tv_nsec - right->mtime.tv_nsec;
    return 0;
}

void
file_info_free(struct file_info *info)
{
    free(info->filename);
    free(info);
}
