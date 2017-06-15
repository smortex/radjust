#ifndef _ADJUST_H
#define _ADJUST_H

#include <stdio.h>

#include <sys/types.h>

int			 libadjust_socket_open_out(const int port) __attribute__((warn_unused_result));
int			 libadjust_socket_open_in(void) __attribute__((warn_unused_result));
int			 libadjust_socket_open_in_accept(void) __attribute__((warn_unused_result));

int			 libadjust_send_files(int argc, char *argv[]) __attribute__((warn_unused_result));
int			 libadjust_recv_files(char *filename) __attribute__((warn_unused_result));

void			 libadjust_socket_close(void);

void			 libadjust_stats_print(FILE *stream);

void			 libadjust_error_print(FILE *stream);

#endif /* !_ADJUST_H */
