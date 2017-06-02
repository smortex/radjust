#include <err.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "adjust.h"

#include <unistd.h>

const char *progname;

void		 usage(void);
int		 start_client(int argc, char *argv[]);
int		 start_server(int argc, char *argv[]);
bool		 is_remote(const char *subject);
void		 extract_remote_host_and_filename(char *subject, char **remote_host, char **filename);
int		 transmit(char *filename);

struct {
    int client;
    int recv;
    int send;
    char *rsh;
} options = {
    0,
    0,
    0,
    "ssh",
};

static struct option longopts[] = {
    { "client", no_argument,       &options.client, 1,   },
    { "recv",   no_argument,       &options.recv,   1,   },
    { "send",   no_argument,       &options.send,   1,   },
    { "rsh",    required_argument, NULL,            'e', },
    { NULL,     0,                 NULL,            0,   },
};

int
main(int argc, char *argv[])
{
    progname = argv[0];

    int ch;
    while ((ch = getopt_long(argc, argv, "e:", longopts, NULL)) != -1) {
	switch (ch) {
	case 0:
	    break;
	case 'e':
	    options.rsh = optarg;
	    break;
	default:
	    usage();
	    break;
	}
    }
    argc -= optind;
    argv += optind;

    if (!options.send && !options.recv) {
	if (is_remote(argv[0]))
	    options.recv = 1;
	if (is_remote(argv[argc - 1]))
	    options.send = 1;
    }

    if (options.send && options.recv) {
	fprintf(stderr, "Options --send and --recv are mutualy exclusive\n");
	exit(EXIT_FAILURE);
    }

    if (options.client) {
	start_client(argc, argv);
    } else {
	start_server(argc, argv);
    }

    exit(EXIT_SUCCESS);
}

void
usage(void)
{
    fprintf(stderr, "usage: %s [options] source destination\n", progname);
    exit(EXIT_FAILURE);
}

int
start_client(int argc, char *argv[])
{
    if (argc != 1)
	usage();

    char buffer[BUFSIZ];
    fgets(buffer, sizeof(buffer), stdin);

    int port;
    if (sscanf(buffer, "%d\n", &port) < 0)
	return -1;

    if (libadjust_socket_open_out(port) < 0)
	errx(EXIT_FAILURE, "libadjust_socket_open_out");

    if (transmit(argv[0]) < 0)
	errx(EXIT_FAILURE, "transmit");

    libadjust_socket_close();

    return 0;
}

int
start_server(int argc, char *argv[])
{
    if (argc != 2)
	usage();

    int local_port = libadjust_socket_open_in();
    if (local_port < 0)
	errx(EXIT_FAILURE, "libadjust_socket_open_in");

    char *remote_host, *remote_filename;
    char *local_filename;

    char *client_flags = "";
    if (options.send) {
	client_flags = "--recv";
	local_filename = argv[0];
	extract_remote_host_and_filename(argv[1], &remote_host, &remote_filename);
    } else {
	client_flags = "--send";
	extract_remote_host_and_filename(argv[0], &remote_host, &remote_filename);
	local_filename = argv[1];
    }

    char *cmd;
    asprintf(&cmd, "%s -R 0:127.0.0.1:%d %s radjust --client %s %s 2>&1", options.rsh, local_port, remote_host, client_flags, remote_filename);

    FILE *f;

    if (!(f = popen(cmd, "r+"))) {
	fprintf(stderr, "popen\n");
	goto fail;
    }

    char buffer[BUFSIZ];
    if (!fgets(buffer, sizeof(buffer), f)) {
	fprintf(stderr, "fgets\n");
	goto fail;
    }

    int remote_port;
    if (sscanf(buffer, "Allocated port %d for remote forward to", &remote_port) != 1) {
	fprintf(stderr, "can't read port number\n");
	goto fail;
    }

    fprintf(f, "%d\n", remote_port);
    fflush(f);

    if (libadjust_socket_open_in_accept() < 0)
	errx(EXIT_FAILURE, "libadjust_socket_open_in_accept");

    if (transmit(local_filename) < 0)
	errx(EXIT_FAILURE, "transmit");

fail:
    libadjust_socket_close();
    if (pclose(f) < 0) {
	err(EXIT_FAILURE, "pclose");
    }
    free(cmd);

    return 0;
}

int
transmit(char *filename)
{
    int res = -1;

    if (options.send)
	res = libadjust_send_file(filename);

    if (options.recv)
	res = libadjust_recv_file(filename);

    if (res < 0)
	libadjust_error_print(stderr);

    return res;
}

void
extract_remote_host_and_filename(char *subject, char **remote_host, char **filename)
{
    if (is_remote(subject)) {
	char *colon = strchr(subject, ':');

	*colon = '\0';
	*remote_host = subject;
	*filename = colon + 1;
    } else {
	*remote_host = NULL;
	*filename = subject;
    }
}

bool
is_remote(const char *subject)
{
    char *slash = strchr(subject, '/');
    char *colon = strchr(subject, ':');

    if ((colon && !slash) || (colon && slash && colon < slash))
	return true;
    else
	return false;
}
