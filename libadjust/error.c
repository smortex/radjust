#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

struct error {
    char *message;
    struct error *next;
};

struct error *error_head = NULL, *error_tail = NULL;

int
error_push(const char *file, const int lineno, const char *function, const bool print_errno, const char *format, ...)
{
    char *message;
    char application_error[BUFSIZ];

    va_list ap;
    va_start(ap, format);
    vsnprintf(application_error, sizeof(application_error), format, ap);
    va_end(ap);

    if (print_errno) {
	asprintf(&message, "%s:%d %s: %s: %s", file, lineno, function, application_error, strerror(errno));
    } else {
	asprintf(&message, "%s:%d %s: %s", file, lineno, function, application_error);
    }

    if (error_tail) {
	error_tail->next = malloc(sizeof(struct error));
	error_tail = error_tail->next;
    } else {
	error_head = error_tail = malloc(sizeof(struct error));
    }

    error_tail->next = NULL;
    error_tail->message = message;

    return 0;
}

void
libadjust_error_print(FILE *stream)
{
    while (error_head) {
	struct error *next = error_head->next;;

	fprintf(stream, "%s\n", error_head->message);
	free(error_head->message);
	free(error_head);
	error_head = next;
    }
}
