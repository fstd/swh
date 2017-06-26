/* nami.c - front-/backend name information, handled by init
 * swh - switch ssh front-end - (C) 2017, Timo Buhrmester
 * See README for contact-, COPYING for license information. */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#define LOG_MOD MOD_NAMI

#include "nami.h"

#include "common/common.h"
#include "common/log.h"


/* Dump a list of all known backends to the given stream */
void
nami_backends(FILE *str)
{
#define X(IFNAME) fprintf(str, "%s\n", #IFNAME);
#include "back/backends.h"
#undef X
	return;
}

/* Dump a list of all known ucs to the given stream */
void
nami_frontends(FILE *str)
{
#define X(IFNAME) fprintf(str, "%s\n", #IFNAME);
#include "front/frontends.h"
#undef X
	return;
}
