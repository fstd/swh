/* uc.c - User communication subsystem; handled by core
 * swh - switch ssh front-end - (C) 2017, Timo Buhrmester
 * See README for contact-, COPYING for license information. */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#define LOG_MOD MOD_FRONT_UC

#include "uc.h"

#include <stdio.h>
#include <string.h>

#include "common/log.h"
#include "common/common.h"


/* Maps uc names to their respective attach functions */
struct uctype {
	char name[32];
	void (*attach_fn)(struct uc_if *ifc);
};

/* Prototypes for all the attach functions that better be there... */
#define X(IFNAME) void uc_ ## IFNAME ## _attach(struct uc_if *ifc);
#include "front/frontends.h"
#undef X

/* Generate array containing all known ucs for by-name lookup */
static struct uctype s_ucs[] = {
#define X(IFNAME) {#IFNAME, uc_ ## IFNAME ## _attach},
#include "front/frontends.h"
#undef X
};

/* Attach point + call dispatch to whatever uc was attached */
static struct uc_if s_uc;


/* Delegate the uc interface to whatever attached to `s_uc` */
void
uc_init(void)
{
	s_uc.f_init();
	return;
}

int
uc_hasdata(bool block)
{
	return s_uc.f_hasdata(block);
}

ssize_t
uc_getdata(char *dest, size_t destsz)
{
	return s_uc.f_getdata(dest, destsz);
}

bool
uc_putdata(const void *data, size_t datalen)
{
	return s_uc.f_putdata(data, datalen);
}

void
uc_dump(void)
{
	s_uc.f_dump();
	return;
}


/* Look up uc by name and attach it */
void
uc_attach(const char *ifname)
{
	for (size_t i = 0; i < COUNTOF(s_ucs); i++)
		if (strcmp(s_ucs[i].name, ifname) == 0) {
			D("attaching uc '%s', ifname", ifname);
			s_ucs[i].attach_fn(&s_uc);
			return;
		}
	C("could not find uc '%s' to attach", ifname);
	return;
}
