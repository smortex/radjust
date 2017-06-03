#include <stdio.h>

#include "adjust.h"
#include "adjust_internal.h"

struct libadjust_stats stats;

void
libadjust_stats_print(FILE *stream)
{
    fprintf(stream, "synchronized %ld bytes\n", stats.bytes_synchronized);
    fprintf(stream, "sent %ld bytes, received %ld bytes\n", stats.bytes_send, stats.bytes_recv);

}
