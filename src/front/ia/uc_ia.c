/* uc_ia.c - Interactive user interface; handled by core (via uc)
 * swh - switch ssh front-end - (C) 2017, Timo Buhrmester
 * See README for contact-, COPYING for license information. */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#define LOG_MOD MOD_FRONT_IA_UC_IA

#include <stdio.h>
#include <string.h>

#include <unistd.h>

#include "../../common/log.h"
#include "../../common/common.h"
#include "../uc.h"

#define READBUFSZ 4095


static char s_readbuf[READBUFSZ + 1];
static size_t s_readbufcnt = 0;


static ssize_t read_more(void);


void
uc_ia_init(void)
{
	if (setvbuf(stdout, NULL, _IONBF, 0) != 0)
		WE("setvbuf");
	I("uc-ia initialized");
	return;
}

int
uc_ia_hasdata(bool block)
{
	V("do we have a whole line? (block: %d)", block);
	if (strchr(s_readbuf, '\n')) {
		V("yes, right there");
		return 1;
	}

	V("can we maybe read some more?");
	while (selectfd(0, block)) {
		V("select(2) says we can read");
		read_more();
		if (strchr(s_readbuf, '\n')) {
			V("now there's a whole line -> success");
			return 1;
		} else
			V("still no whole line");
	}

	V("nothing more to read for now, returning");

	return 0;
}

ssize_t
uc_ia_getdata(char *dest, size_t destsz)
{
	char *p = strchr(s_readbuf, '\n');
	if (!p) {
		V("no data to hand out (rbc %zu)", s_readbufcnt);
		return 0;
	}
	size_t len = p - s_readbuf + 1;
	size_t copy = len;
	if (copy >= destsz) {
		W("data truncated, you're likely SOL"); // XXX
		copy = destsz - 1;
	}
	D("handing out %zu/%zu bytes of data", copy, len);
	memcpy(dest, s_readbuf, copy);
	shiftbuf(s_readbuf, &s_readbufcnt, len);
	s_readbuf[s_readbufcnt] = '\0';
	return len;
}

bool
uc_ia_putdata(const void *data, size_t datalen)
{
	D("received %zu bytes of data, printing to stdout", datalen);
	printf("%.*s", (int)datalen, (const char *)data);
	fflush(stdout);
	return true;
}

void
uc_ia_dump(void)
{
	A("uc-ia dump");
	A("s_readbufcnt: %zu", s_readbufcnt);
	A("uc-ia end of dump");
	return;
}



static ssize_t
read_more(void)
{
	size_t remain = READBUFSZ - s_readbufcnt;
	V("read(2)ing up to %zu bytes, blockingly", remain);
	ssize_t r = xread(0, s_readbuf + s_readbufcnt, remain);
	if (r == -1)
		CE("read");
	if (r == 0) {
		C("read: EOF"); // XXX
	}

	D("read %zd bytes from user", r);
	s_readbufcnt += (size_t)r;
	s_readbuf[s_readbufcnt] = '\0';
	hexdump(s_readbuf, s_readbufcnt, "readbuf");
	return r;
}


void
uc_ia_attach(struct uc_if *ifc)
{
	ifc->f_init = uc_ia_init;
	ifc->f_hasdata = uc_ia_hasdata;
	ifc->f_getdata = uc_ia_getdata;
	ifc->f_putdata = uc_ia_putdata;
	ifc->f_dump = uc_ia_dump;
	I("uc-ia attached");
	return;
}
