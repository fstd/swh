/* core.c - Main loop, ties everything together; handled by init
 * swh - switch ssh front-end - (C) 2017, Timo Buhrmester
 * See README for contact-, COPYING for license information. */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#define LOG_MOD MOD_CORE

#include "core.h"

#include <stdio.h>
#include <string.h>

#include "common/common.h"
#include "common/log.h"
#include "sc.h"
#include "spawn.h"
#include "front/uc.h"


/* initialize subsystems, attach user front end and switch back end */
void
core_init(const char *frontend, const char *backend, char **envp)
{
	spawn_init(envp);
	sc_init(backend);
	uc_attach(frontend);
	uc_init();
	I("core initialized");
	return;
}

/* main loop, `host` is the switch we're going to talk to.
 * does not return */
int
core_run(const char *host)
{
	char buf[512];

	if (sc_start(host) != 0)
		C("could not start sc");

	for (;;) {
		spawn_operate();

		while (sc_busy())
			if (!sc_operate())
				selectfd(sc_getfd(), true);

		if (sc_offline())
			C("sc offline");

		if (sc_hasreply()) {
			const char *rep = sc_getreply();
			uc_putdata(rep, strlen(rep));
		}

		const char *ps1 = sc_getps1();
		uc_putdata(ps1, strlen(ps1));

		if (uc_hasdata(true) == -1)
			C("uc shat itself");

		ssize_t n = uc_getdata(buf, sizeof buf);
		if (n < 0)
			C("uc massively shat itself");
		else if (n == 0)
			C("bug: uc doesn't have data yet claims to do");

		sc_write(buf, n);
	}
}
