/* init.c - Initialization, argument processing, entry point
 * swh - switch ssh front-end - (C) 2017, Timo Buhrmester
 * See README for contact-, COPYING for license information. */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#define LOG_MOD MOD_INIT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <getopt.h>

#include "common/log.h"
#include "common/common.h"
#include "nami.h"
#include "core.h"


/* selected user front-end (currently there's only one) */
static char s_ux[32] = "auto";

/* selected switch back-end (currently there's only one) */
static char s_sx[32] = "auto";

/* host to connect to */
static char s_host[256];


static void process_args(int argc, char **argv);
static void init(int argc, char **argv, char **envp);
static void usage(FILE *str, const char *a0, int ec);
static void update_logger(int verb, int fancy);


static void
process_args(int argc, char **argv)
{
	char *a0 = argv[0];

	for(int ch; (ch = getopt(argc, argv, "Xx:Ss:cvqh")) != -1;) {
		switch (ch) {
		case 's':
			snprintf(s_sx, sizeof s_sx, "%s", optarg);
			break;
		case 'S':
			nami_backends(stdout);
			exit(0);
		case 'x':
			snprintf(s_ux, sizeof s_ux, "%s", optarg);
			break;
		case 'X':
			nami_frontends(stdout);
			exit(0);
		case 'c':
			update_logger(0, 1);
			break;
		case 'v':
			update_logger(1, -1);
			break;
		case 'q':
			update_logger(-1, -1);
			break;
		case 'h':
			usage(stdout, a0, EXIT_SUCCESS);
			break;
		case '?':
		default:
			usage(stderr, a0, EXIT_FAILURE);
		}
	}

	argc -= optind;
	argv += optind;

	if (argc == 0)
		C("argument missing (switch hostname or address)");

	snprintf(s_host, sizeof s_host, "%s", argv[0]);

	if (strcmp(s_ux, "auto") == 0)
		strcpy(s_ux, "ia"); // There's just one uc for now

	if (strcmp(s_sx, "auto") == 0)
		strcpy(s_sx, "hp"); // There's just one backend for now
}

static void
init(int argc, char **argv, char **envp)
{
	log_init();
	log_set_ourname(PACKAGE_NAME);

	process_args(argc, argv);

	core_init(s_ux, s_sx, envp);

	N("all subsystems initialized");
}

static void
usage(FILE *str, const char *a0, int ec)
{
	#define U(STR) fputs(STR "\n", str)
	U("================");
	U("== "PACKAGE_NAME" v"PACKAGE_VERSION" ==");
	U("================");
	fprintf(str, "usage: %s [-x <frontend>] [-s <backend>] [-XScvqh]\n", a0);
	U("");
	U("\t-x <frontend>: Use user interface <frontend> (default: auto)");
	U("\t-X: List known user interfaces types and exit");
	U("\t-s <backend>: Use switch interface <backend> (default: auto)");
	U("\t-S: List known switch interfaces types and exit");
	U("\t-c: Use ANSI color sequences on stderr");
	U("\t-v: Be more verbose (multiple are OK)");
	U("\t-q: Be less verbose (multiple are OK)");
	U("\t-h: Display brief usage statement and terminate");
	U("");
	U("(C) 2017, Timo Buhrmester <van.fstd@gmail.com>");
	#undef U
	exit(ec);
}

/* verb: relative; fancy: 1=on 0=off -1=don't change */
static void
update_logger(int verb, int fancy)
{
	int v = log_getlvl(MOD_INIT) + verb;
	if (v < 0)
		v = 0;

	if (fancy == -1)
		fancy = log_getfancy();

	log_setfancy(fancy);
	log_setlvl_all(v);
}


int
main(int argc, char **argv, char **envp)
{
	init(argc, argv, envp);

	return core_run(s_host);
}
