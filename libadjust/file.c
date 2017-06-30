#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/param.h>

#ifdef HAVE_MD
#  include <sha256.h>
#endif

#ifdef HAVE_OPENSSL
#  include <openssl/sha.h>
#endif

#include "adjust.h"
#include "adjust_internal.h"

static int		 map_current_block(struct file_info *file);
static int		 unmap_current_block(struct file_info *file);
static int		 receive_file_data(const int fd, const char *filename, const struct file_info *remote_info);
static int		 create_directory_for(const char *filename);
static int		 send_file_or_directory(const char *local_filename, const char *remote_filename);

extern int sock;

int
libadjust_send_files(int argc, char *argv[])
{
    for (int i = 0; i < argc; i++) {
	if (send_file_or_directory(argv[i], NULL) < 0)
	    FAILX(-1, "send_file_or_directory");
    }

    return 0;
}

int
libadjust_recv_files(char *filename)
{
    char data;
    while (peek_data(sock, &data, 1) > 0) {
	if (recv_file(filename) < 0)
	    FAILX(-1, "recv_file");
    }

    return 0;
}

int
send_file_or_directory(const char *local_filename, const char *remote_filename)
{
    struct file_info *info;
    if (!(info = file_info_new(local_filename)))
	FAILX(-1, "file_info_new");

    if (info->type == T_DIRECTORY) {
	char combined_local_filename[BUFSIZ];
	char combined_remote_filename[BUFSIZ];

	DIR *d;

	if ((d = opendir(local_filename)) == NULL)
	    FAIL(-1, "opendir");

	struct dirent *dp;
	while ((dp = readdir(d))) {
	    if (strcmp(dp->d_name, ".") == 0)
		continue;
	    if (strcmp(dp->d_name, "..") == 0)
		continue;

	    if (!remote_filename) {
		if (local_filename[strlen(local_filename) - 1] == '/') {
		    sprintf(combined_remote_filename, "%s", dp->d_name);
		} else {
		    sprintf(combined_remote_filename, "%s/%s", basename(local_filename), dp->d_name);
		}
	    } else {
		sprintf(combined_remote_filename, "%s/%s", remote_filename, dp->d_name);
	    }

	    sprintf(combined_local_filename, "%s/%s", local_filename, dp->d_name);

	    send_file_or_directory(combined_local_filename, combined_remote_filename);
	}

	closedir(d);
    } else {
	if (send_file(info, remote_filename) < 0)
	    FAILX(-1, "send_file");
    }

    file_info_free(info);
    return 0;
}

int
send_file(struct file_info *info, const char *remote_filename)
{
    if (!remote_filename)
	remote_filename = basename(info->filename);

    if (file_open(info, O_RDONLY) < 0)
	FAIL(-1, "file_open");

    char buffer[BUFSIZ];
    sprintf(buffer, "%s:%ld:%ld.%9ld\n", remote_filename, info->size, info->mtime.tv_sec, info->mtime.tv_nsec);

    if (send_data(sock, buffer, strlen(buffer)) != (int) strlen(buffer))
	FAILX(-1, "send_data");

    struct file_info *remote;
    remote = file_info_alloc();

    if (recv_line(buffer, sizeof(buffer)) < 0)
	FAILX(-1, "read_line");

    if (sscanf(buffer, "%ld", &remote->size) != 1)
	FAILX(-1, "sscanf");

    if (file_send(sock, info, remote) < 0)
	FAILX(-1, "file_send");

    if (file_close(info) < 0)
	FAILX(-1, "file_close");

    stats.bytes_synchronized += info->size;

    stats.files_synchronized++;

    return 0;
}

int
recv_file(const char *filename)
{
    char buffer[BUFSIZ];
    if (recv_line(buffer, sizeof(buffer)) < 0)
	FAILX(-1, "read_line");

    char remote_filename[BUFSIZ];

    struct file_info *remote_info;
    remote_info = file_info_alloc();

    if (sscanf(buffer, "%[^:]:%ld:%ld.%9ld", remote_filename, &remote_info->size, &remote_info->mtime.tv_sec, &remote_info->mtime.tv_nsec) != 4)
	FAILX(-1, "sscanf");

    remote_info->filename = strdup(remote_filename);

    if (receive_file_data(sock, filename, remote_info) < 0)
	FAILX(-1, "receive_file_data");

    stats.bytes_synchronized += remote_info->size;
    file_info_free(remote_info);

    stats.files_synchronized++;

    return 0;
}

int
receive_file_data(const int fd, const char *filename, const struct file_info *remote_info)
{
    int res = 0;
    struct file_info *local_info;
    char answer;

    if ((local_info = file_info_new(filename))) {
	if (local_info->type == T_DIRECTORY) {
	    char buffer[BUFSIZ];
	    sprintf(buffer, "%s/%s", filename, remote_info->filename);
	    if (receive_file_data(fd, buffer, remote_info) < 0)
		FAILX(-1, "receive_file_data");
	    return 0;
	}
	if (0 == file_info_cmp(local_info, remote_info)) {
	    answer = ADJUST_FILE_UPTODATE;
	} else {
	    answer = ADJUST_FILE_MISMATCH;
	}
    } else {
	if (errno == ENOENT) {
	    answer = ADJUST_FILE_MISSING;
	    local_info = file_info_alloc();
	    local_info->filename = strdup(filename);
	} else {
	    FAIL(-1, "file_info_new");
	}
    }

    char buffer[BUFSIZ];
    sprintf(buffer, "%ld\n", local_info->size);

    if (send_data(fd, buffer, strlen(buffer)) < 0)
	FAIL(-1, "send_data");

    if (send_data(fd, &answer, 1) != 1)
	FAILX(-1, "send_data");

    if (answer == ADJUST_FILE_UPTODATE)
	return 0;

    if (file_recv(fd, local_info, remote_info) < 0)
	FAILX(-1, "file_recv");

    file_info_free(local_info);

    return res;
}

void
sha256(const void *data, const size_t length, unsigned char digest[32])
{
    SHA256_CTX sha256;

    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data, length);
    SHA256_Final(digest, &sha256);
}

int
file_open(struct file_info *file, int mode)
{
    if ((file->fd = open(file->filename, mode, 0666))) {
	file->mmap_mode = PROT_READ;
	if ((mode & O_RDWR) == O_RDWR)
	    file->mmap_mode |= PROT_WRITE;
    }

    if (file->fd < 0 && errno == ENOENT) {
	if (create_directory_for(file->filename) < 0)
	    return -1;

	return file_open(file, mode);
    }

    return file->fd;
}

int
create_directory_for(const char *filename)
{
    char *directory;

    if (!(directory = strdup(dirname(filename))))
	FAIL(-1, "strdup");

    struct stat sb;
    if (stat(directory, &sb) < 0) {
	if (create_directory_for(directory) < 0)
	    FAILX(-1, "create_directory_for");

	if (mkdir(directory, 0777) < 0)
	    FAIL(-1, "mkdir");
    }

    free(directory);
    return 0;
}

static int
map_current_block(struct file_info *file)
{
    file->data = mmap(NULL, file->data_size, file->mmap_mode, MAP_SHARED, file->fd, file->offset);

    if (file->data == MAP_FAILED)
	FAIL(-1, "mmap");

    return 0;
}

static int
unmap_current_block(struct file_info *file)
{
    return munmap(file->data, file->data_size);
}

int
file_map_first_block(struct file_info *file)
{
    file->data_size = MIN(LARGE_BLOCK_SIZE, file->size);
    file->offset = 0;
    return map_current_block(file);
}

int
file_map_next_block(struct file_info *file)
{
    if (unmap_current_block(file) < 0)
	FAILX(-1, "unmap_current_block");

    file->offset += file->data_size;

    if (file->offset == file->size)
	return 0;

    file->data_size = MIN(LARGE_BLOCK_SIZE, file->size - file->offset);
    if (map_current_block(file) < 0)
	FAILX(-1, "map_current_block");

    return 1;
}

int
file_close(struct file_info *file)
{
    unmap_current_block(file);
    return close(file->fd);
}

int
file_set_size(struct file_info *file, const off_t size)
{
    if (file->size > size) {
	if (ftruncate(file->fd, size) < 0)
	    FAIL(-1, "ftruncate");
    }

    file->size = size;

    return 0;
}

int
file_set_mtime(const struct file_info *file, const struct timespec mtime)
{
    if (fsync(file->fd) < 0)
	FAIL(-1, "fsync");

    struct timespec times[] = {
	{ 0, UTIME_OMIT },
	mtime,
    };

    if (futimens(file->fd, times) < 0)
	FAIL(-1, "futimens");

    return 0;
}

int
file_send(const int fd, struct file_info *local, const struct file_info *remote)
{
    char buffer;
    if (recv_data(fd, &buffer, 1) != 1)
	FAILX(-1, "recv_data");

    if (buffer == ADJUST_FILE_UPTODATE)
	return 0;

    return file_send_content(fd, local, remote);
}

int
file_recv(const int fd, struct file_info *local, const struct file_info *remote)
{
    if (file_open(local, O_RDWR | O_CREAT) < 0)
	FAIL(-1, "file_open");

    if (file_recv_content(fd, local, remote) < 0)
	FAILX(-1, "file_recv_content");

    if (file_set_mtime(local, remote->mtime) < 0)
	FAILX(-1, "file_set_mtime");

    if (file_close(local) < 0)
	FAILX(-1, "file_close");

    return 0;
}

int
file_send_content(const int fd, struct file_info *local, const struct file_info *remote)
{
    if (send_file_adjustments(fd, local, remote) < 0)
	FAILX(-1, "send_file_adjustments");

    if (send_end_of_file(fd, local) < 0)
	FAILX(-1, "send_end_of_file");

    return 0;
}

int
file_recv_content(const int fd, struct file_info *local, const struct file_info *remote)
{
    if (recv_file_adjustments(fd, local, remote) < 0)
	FAILX(-1, "recv_file_adjustments");

    if (file_set_size(local, remote->size) < 0)
	FAILX(-1, "file_set_size");

    if (recv_end_of_file(fd, local) < 0)
	FAILX(-1, "recv_end_of_file");

    return 0;
}
