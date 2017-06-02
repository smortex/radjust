#include <err.h>
#include <fcntl.h>
#include <getopt.h>
#include <paths.h>
#include <signal.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/wait.h>

#include "adjust.h"

const char *progname;

void		 usage(void);
int		 start_client(int argc, char *argv[]);
int		 start_server(int argc, char *argv[]);
bool		 is_remote(const char *subject);
void		 extract_remote_host_and_filename(char *subject, char **remote_host, char **filename);
pid_t		 run_external_command(char *command, int *in, int *out, int *err);
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

void
sigchld_handler(int sig, siginfo_t *info, void *ucontext)
{
    (void) sig;
    (void) ucontext ;

    if (info->si_code == CLD_EXITED && info->si_status != 0) {
	fprintf(stderr, "child process exited with error code %d\n", info->si_status);
	exit(EXIT_FAILURE);
    }
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
    asprintf(&cmd, "%s -R 0:127.0.0.1:%d -- %s radjust --client %s %s", options.rsh, local_port, remote_host, client_flags, remote_filename);

    int client_pid;
    int in_fd, out_fd, err_fd;

    struct sigaction act = {
	.sa_sigaction = sigchld_handler,
	.sa_flags = SA_SIGINFO,
    };
    struct sigaction oact;
    sigfillset(&act.sa_mask);
    sigaction(SIGCHLD, &act, &oact);

    if ((client_pid = run_external_command(cmd, &in_fd, &out_fd, &err_fd)) < 0) {
	err(EXIT_FAILURE, "run_external_command\n");
    }

    char buffer[BUFSIZ];
    if (read(err_fd, buffer, sizeof(buffer)) < 1) {
	perror("read");
	goto fail;
    }

    int remote_port;
    if (sscanf(buffer, "Allocated port %d for remote forward to", &remote_port) != 1) {
	fprintf(stderr, "can't read port number\n");
	goto fail;
    }

    sprintf(buffer, "%d\n", remote_port);
    write(in_fd, buffer, strlen(buffer));

    if (libadjust_socket_open_in_accept() < 0)
	errx(EXIT_FAILURE, "libadjust_socket_open_in_accept");

    sigaction(SIGCHLD, &oact, NULL);

    if (transmit(local_filename) < 0)
	errx(EXIT_FAILURE, "transmit");

fail:
    libadjust_socket_close();

    int exit_code;
    if (waitpid(client_pid, &exit_code, 0) < 0) {
	err(EXIT_FAILURE, "waitpid");
    }
    if (exit_code != 0)
	err(EXIT_FAILURE, "client exited with error");
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

extern char **environ;

pid_t
run_external_command(char *command, int *in_fd, int *out_fd, int *err_fd)
{
    pid_t res;
    char *argv[4];
    int pipes[3][2];

    if (pipe2(pipes[0], O_CLOEXEC) < 0)
	err(EXIT_FAILURE, "pipe2");
    if (pipe2(pipes[1], O_CLOEXEC) < 0)
	err(EXIT_FAILURE, "pipe2");
    if (pipe2(pipes[2], O_CLOEXEC) < 0)
	err(EXIT_FAILURE, "pipe2");

    switch (res = fork()) {
    case -1:
	err(EXIT_FAILURE, "fork");
	break;
    case 0:
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	dup2(pipes[0][0], STDIN_FILENO);
	dup2(pipes[1][1], STDOUT_FILENO);
	dup2(pipes[2][1], STDERR_FILENO);

	argv[0] = "sh";
	argv[1] = "-c";
	argv[2] = command;
	argv[3] = NULL;

	execve(_PATH_BSHELL, argv, environ);

	err(EXIT_FAILURE, "execve");
	return -1;
	break;
    default:
	break;
    }

    close(pipes[0][0]);
    close(pipes[1][1]);
    close(pipes[2][1]);

    *in_fd  = pipes[0][1];
    *out_fd = pipes[1][0];
    *err_fd = pipes[2][0];

    return res;
}
