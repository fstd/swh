/* fsm_cmdout.c - FSM abstraction for command output, handled by sc
 * swh - switch ssh front-end - (C) 2017, Timo Buhrmester
 * See README for contact-, COPYING for license information. */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#define LOG_MOD MOD_BACK_FSM_CMDOUT

#include "fsm_cmdout.h"

#include <string.h>

#include "../common/common.h"
#include "../common/log.h"


/* Generate prototypes for all the attach functions that better exist... */
#define X(IFNAME) \
	void fsm_cmdout_ ## IFNAME ## _attach(struct fsm_cmdout_if *ifc);
#include "backends.h"
#undef X

/* Generate a name -> attach function lookup table for by-name attaching */
static struct fsm_cmdout_type {
	char name[32];
	void (*attach_fn)(struct fsm_cmdout_if *ifc);
} s_backends[] = {
#define X(IFNAME) {#IFNAME, fsm_cmdout_ ## IFNAME ## _attach},
#include "backends.h"
#undef X
};

/* Attach point + call dispatch to whatever backend was attached */
static struct fsm_cmdout_if s_backend;


/* Delegate the backend interface to whatever attached to `s_backend` */
fsm *
fsm_cmdout_init(void)
{
	return s_backend.f_init();
}

const char *
fsm_cmdout_ps1buf(void)
{
	return s_backend.f_ps1buf();
}

const char *
fsm_cmdout_outbuf(void)
{
	return s_backend.f_outbuf();
}

size_t
fsm_cmdout_outbufcnt(void)
{
	return s_backend.f_outbufcnt();
}

bool
fsm_cmdout_anykey(void)
{
	return s_backend.f_anykey();
}

bool
fsm_cmdout_report(void)
{
	return s_backend.f_report();
}


/* Look up backend by name and attach it */
void
fsm_cmdout_attach(const char *ifname)
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
