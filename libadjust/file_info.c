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
    return res;
}

struct file_info *
file_info_new(char *filename)
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
    }

    return res;
}

int
file_info_cmp(struct file_info *left, struct file_info *right)
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
