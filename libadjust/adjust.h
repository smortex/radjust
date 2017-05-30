#ifndef _ADJUST_H
#define _ADJUST_H

#include <sys/types.h>

int			 libadjust_connect(void) __attribute__((warn_unused_result));
int			 libadjust_serve(void) __attribute__((warn_unused_result));

int			 libadjust_send_file(char *filename) __attribute__((warn_unused_result));
int			 libadjust_recv_file(char *filename) __attribute__((warn_unused_result));

void			 libadjust_terminate(void);

void			 get_xfer_stats(size_t *bytes);
void			 get_networking_stats(size_t *send, size_t *recv);

#endif /* !_ADJUST_H */
