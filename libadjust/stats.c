#include <stdio.h>

#include "adjust.h"
#include "adjust_internal.h"

struct libadjust_stats stats;

void
libadjust_stats_print(FILE *stream)
{
    fprintf(stream, "synchronized %ld bytes\n", stats.bytes_synchronized);
    fprintf(stream, "%d chunks in %d blocks where adjusted\n", stats.adjusted_chunks, stats.adjusted_blocks);
    fprintf(stream, "sent %ld bytes, received %ld bytes\n", stats.bytes_send, stats.bytes_recv);

}
