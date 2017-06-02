#include <err.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void		 usage(void);

const char *progname;
struct {
    struct {
	int port;
	char host[255];
	int hostport;
    } R;
    char *user;
    char *hostname;
} options;

int
main(int argc, char *argv[])
{
    int ch;
    progname = argv[0];

    while ((ch = getopt(argc, argv, "R:")) != -1) {
	switch (ch) {
	case 'R':
	    if (sscanf(optarg, "%d:%[^:]:%d", &options.R.port, options.R.host, &options.R.hostport) != 3)
		exit(EXIT_FAILURE);

	    if (options.R.port == 0) {
		if (strcmp(options.R.host, "127.0.0.1") == 0) {
		    fprintf(stderr, "Allocated port %d for remote forward to %s:%d\n", options.R.hostport, options.R.host, options.R.hostport);
		} else {
		    fprintf(stderr, "Fake -R only supports 127.0.0.1 as host.\n");
		    exit(EXIT_FAILURE);
		}
	    }
	    break;
	case '?':
	default:
	    usage();
	    break;
	}
    }

    argc -= optind;
    argv += optind;

    if (argc < 2)
	usage();

    char *p = strchr(argv[0], '@');
    if (p) {
	options.user = argv[0];
	options.hostname = p + 1;
    } else {
	options.user = NULL;
	options.hostname = argv[0];
    }

    argc -= 1;
    argv += 1;

    if (execvp(argv[0], argv) < 0) {
	err(EXIT_FAILURE, "execvp");
    }

    /* NOTREACHED */
    exit(EXIT_SUCCESS);
}

void
usage(void)
{
    fprintf(stderr, "usage: %s [-R port:host:hostport] [user@]hostname [command]\n", progname);
    exit(EXIT_FAILURE);
}
