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


static void process_args(int argc, char **argv);
static void init(int argc, char **argv);
static void usage(FILE *str, const char *a0, int ec);
static void update_logger(int verb, int fancy);


static void
process_args(int argc, char **argv)
{
	char *a0 = argv[0];

	for(int ch; (ch = getopt(argc, argv, "cvqh")) != -1;) {
		switch (ch) {
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
}

static void
init(int argc, char **argv)
{
	log_init();
	log_set_ourname(PACKAGE_NAME);

	process_args(argc, argv);

	N("all subsystems initialized");
}

static void
usage(FILE *str, const char *a0, int ec)
{
	#define U(STR) fputs(STR "\n", str)
	U("================");
	U("== "PACKAGE_NAME" v"PACKAGE_VERSION" ==");
	U("================");
	fprintf(str, "usage: %s [-cvqh]\n", a0);
	U("");
	U("\t-c: Use ANSI color sequences on stderr");
	U("\t-v: Be more verbose (multiple are OK)");
	U("\t-q: Be less verbose (multiple are OK)");
	U("\t-h: Display brief usage statement and terminate");
	U("");
	U("(C) 2017, Timo Buhrmester");
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
main(int argc, char **argv)
{
	init(argc, argv);

	return 0;
}
