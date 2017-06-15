#include <stdio.h>

#include "adjust.h"
#include "adjust_internal.h"

#define PLURALIZE(n) ((n) == 1 ? "" : "s")

struct libadjust_stats stats;

void
libadjust_stats_print(FILE *stream)
{
    fprintf(stream, "synchronized %ld byte%s in %d file%s\n", stats.bytes_synchronized, PLURALIZE(stats.bytes_synchronized), stats.files_synchronized, PLURALIZE(stats.files_synchronized));
    fprintf(stream, "%ld byte%s in %d chunk%s in %d block%s where adjusted\n", stats.bytes_adjusted, PLURALIZE(stats.bytes_adjusted), stats.adjusted_chunks, PLURALIZE(stats.adjusted_chunks), stats.adjusted_blocks, PLURALIZE(stats.adjusted_blocks));
    fprintf(stream, "%ld byte%s where send raw\n", stats.bytes_send_raw, PLURALIZE(stats.bytes_send_raw));
    fprintf(stream, "sent %ld byte%s, received %ld byte%s\n", stats.bytes_send, PLURALIZE(stats.bytes_send), stats.bytes_recv, PLURALIZE(stats.bytes_recv));

}
