#ifndef _ADJUST_H
#define _ADJUST_H

#include <stdio.h>

#include <sys/types.h>

int			 libadjust_socket_open_out(const int port) __attribute__((warn_unused_result));
int			 libadjust_socket_open_in(void) __attribute__((warn_unused_result));
int			 libadjust_socket_open_in_accept(void) __attribute__((warn_unused_result));

int			 libadjust_send_file(char *filename) __attribute__((warn_unused_result));
int			 libadjust_recv_file(char *filename) __attribute__((warn_unused_result));

void			 libadjust_socket_close(void);

void			 get_xfer_stats(size_t *bytes);
void			 get_networking_stats(size_t *send, size_t *recv);

void			 libadjust_error_print(FILE *stream);

#endif /* !_ADJUST_H */
