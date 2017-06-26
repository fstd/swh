/* uc_noop.c - Dummy no-op user interface, use as a template for new ones.
 * swh - switch ssh front-end - (C) 2017, Timo Buhrmester
 * See README for contact-, COPYING for license information. */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#define LOG_MOD MOD_FRONT_NOOP_UC_NOOP

#include "../../common/log.h"
#include "../../common/common.h"
#include "../uc.h"


void
uc_noop_init(void)
{
	/* initialize the user frontend.  if there's a problem, abort
	 * using the C or CE macro. */
	I("uc-noop initialized"); /* No-op has nothing to initialize */
	return;
}

int
uc_noop_hasdata(bool block)
{
	(void)block;
	/* return 1 if there's data available through uc_*_getdata,
	 * return -1 if the user frontend has gone fubar,
	 * return 0 if there isn't yet data, unless `block` is true,
	 * in which case we're supposed to wait until we can return
	 * 1 or -1 */
	return 0; /* No-op never has data */
}

ssize_t
uc_noop_getdata(char *dest, size_t destsz)
{
	(void)dest, (void)destsz;
	/* attempt to retrieve data from the user frontend -- typically
	 * a command to be invoked on the switch like 'show vlans\n'.
	 * return 0 if there isn't any data to get, -1 if the user
	 * frontend is dead.  If there is data to be read, return
	 * the number of bytes written into `*dest`, which may be
	 * no more than `destsz`.  */
	return 0; /* No-op never has data */
}

bool
uc_noop_putdata(const void *data, size_t datalen)
{
	(void)data, (void)datalen;
	/* write `datalen` bytes worth of data (typically a response from
	 * the switch) to the user frontend.
	 * return true if all data has been written, false if there's
	 * a write error or if we're dead */
	return true; /* No-op discards everything */
}

void
uc_noop_dump(void)
{
	/* dump to stderr as much state as possible for debug purposes */
	A("uc-noop exhaustive state dump:"); /* No-op truly is useless */
	return;
}


void
uc_noop_attach(struct uc_if *ifc)
{
	/* attach pointers to the above functions to the interface
	 * struct pointed to by `ifc` */
	ifc->f_init = uc_noop_init;
	ifc->f_hasdata = uc_noop_hasdata;
	ifc->f_getdata = uc_noop_getdata;
	ifc->f_putdata = uc_noop_putdata;
	ifc->f_dump = uc_noop_dump;
	I("uc-noop attached");
	return;
}
