/* fsm_init.c - FSM abstraction for the initial chat, handled by sc
 * swh - switch ssh front-end - (C) 2017, Timo Buhrmester
 * See README for contact-, COPYING for license information. */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#define LOG_MOD MOD_BACK_FSM_INIT

#include "fsm_init.h"

#include <string.h>

#include "../common/common.h"
#include "../common/log.h"


/* Generate prototypes for all the attach functions that better exist... */
#define X(IFNAME) \
	void fsm_init_ ## IFNAME ## _attach(struct fsm_init_if *ifc);
#include "backends.h"
#undef X

/* Generate a name -> attach function lookup table for by-name attaching */
static struct fsm_init_type {
	char name[32];
	void (*attach_fn)(struct fsm_init_if *ifc);
} s_backends[] = {
#define X(IFNAME) {#IFNAME, fsm_init_ ## IFNAME ## _attach},
#include "backends.h"
#undef X
};

/* Attach point + call dispatch to whatever backend was attached */
static struct fsm_init_if s_backend;


/* Delegate the backend interface to whatever attached to `s_backend` */
fsm *
fsm_init_init(void)
{
	return s_backend.f_init();
}

const char *
fsm_init_ps1buf(void)
{
	return s_backend.f_ps1buf();
}

bool
fsm_init_anykey(void)
{
	return s_backend.f_anykey();
}

bool
fsm_init_report(void)
{
	return s_backend.f_report();
}


/* Look up backend by name and attach it */
void
fsm_init_attach(const char *ifname)
{
	for (size_t i = 0; i < COUNTOF(s_backends); i++)
		if (strcmp(s_backends[i].name, ifname) == 0) {
			D("attaching '%s', ifname", ifname);
			s_backends[i].attach_fn(&s_backend);
			return;
		}
	C("could not find '%s' to attach", ifname);
	return;
}
