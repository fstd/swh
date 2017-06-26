/* sc.c - Switch communication subsystem; handled by core
 * swh - switch ssh front-end - (C) 2017, Timo Buhrmester
 * See README for contact-, COPYING for license information. */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#define LOG_MOD MOD_SC

#include "sc.h"

#include <stdint.h>
#include <string.h>

#include <unistd.h>

#include "common/log.h"
#include "common/common.h"
#include "spawn.h"
#include "back/fsm.h"
#include "back/fsm_init.h"
#include "back/fsm_cmdout.h"
#include "back/fsm_inchar.h"

#define BUSY 0
#define READY 1
#define WRITING 2
#define OFFLINE 3

// initial sizes, buffers will grow on demand
#define READBUFSZ 4096
#define OUTBUFSZ 4096
#define PS1BUFSZ 128
#define WRITEBUFSZ 4096


static bool s_nodataflag;
static int s_state;
static int s_stdin, s_stdout, s_stderr;

static char *s_readbuf;
static size_t s_readbufsz, s_readbufcnt;

static char *s_writebuf;
static size_t s_writebufsz, s_writebufcnt;

static char *s_ps1buf;
static size_t s_ps1bufsz, s_ps1bufcnt;

static char *s_outbuf;
static size_t s_outbufsz, s_outbufcnt;

static fsm *s_fsm_init;
static fsm *s_fsm_cmdout;
static fsm *s_fsm_inchar;
static fsm *s_curfsm;


static ssize_t read_more(void);
static int oper_writing(void);
static int oper_busy(void);


void
sc_init(const char *backend)
{
	V("allocating and initializing buffers");
	(s_readbuf = xmalloc(s_readbufsz = READBUFSZ))[0] = '\0';
	(s_writebuf = xmalloc(s_writebufsz = WRITEBUFSZ))[0] = '\0';
	(s_ps1buf = xmalloc(s_ps1bufsz = PS1BUFSZ))[0] = '\0';
	(s_outbuf = xmalloc(s_outbufsz = OUTBUFSZ))[0] = '\0';

	fsm_init_attach(backend);
	fsm_cmdout_attach(backend);
	fsm_inchar_attach(backend);

	s_fsm_init = fsm_init_init();
	s_fsm_cmdout = fsm_cmdout_init();
	s_fsm_inchar = fsm_inchar_init();

	s_curfsm = s_fsm_init;

	//fsm_init();
	//V("resetting fsm, program init");
	//fsm_reset(0); // loads init program

	I("sc initialized");
	return;
}

int
sc_start(const char *host)
{
	D("calling spawn to launch ssh");
	if (spawn_launch(host, &s_stdin, &s_stdout, &s_stderr) != 0)
		C("could not spawn ssh");

	D("going nonblocking");
	setblocking(s_stdout, false);

	D("spawned ssh");
	return 0;
}

int
sc_operate(void)
{
	switch (s_state) {
	case WRITING:
		return oper_writing();
	case BUSY:
		return oper_busy();
	case READY:
		C("cannot operate while ready");
	case OFFLINE:
		C("cannot operate while offline");
	default:
		C("invalid state %d", s_state);
	}
}

void
sc_write(const char *str, size_t len)
{
	if (s_state != READY)
		C("attempt to write outside READY state");
	if (s_writebufcnt)
		C("write buffer not empty");

	growbuf(&s_writebuf, &s_writebufsz, s_writebufcnt + len + 1);
	memcpy(s_writebuf + s_writebufcnt, str, len);
	s_writebufcnt += len;
	s_writebuf[s_writebufcnt] = '\0';

	D("queued %zu bytes (%.*s) to switch", len, (int)len, str);
	V("state changed to WRITING");
	s_state = WRITING;
	V("purging outbuf");
	s_outbufcnt = 0;
	s_outbuf[0] = '\0';
	return;
}

bool
sc_hasreply(void)
{
	return s_outbufcnt;
}

const char *
sc_getreply(void)
{
	return s_outbuf;
}

const char *
sc_getps1(void)
{
	return s_ps1buf;
}

bool
sc_busy(void)
{
	return !sc_ready() && !sc_offline();
}

bool
sc_offline(void)
{
	return s_state == OFFLINE;
}

bool
sc_ready(void)
{
	return s_state == READY;
}

int
sc_getfd(void)
{
	return s_stdout;
}



static ssize_t
read_more(void)
{
	size_t remain;
	remain = s_readbufsz - s_readbufcnt;
	if (!remain) {
		N("growing readbuf");
		growbuf(&s_readbuf, &s_readbufsz, s_readbufsz * 2);
	}
	remain = s_readbufsz - s_readbufcnt;

	V("trying to read more data (%zu bytes space in readbuf)", remain);
	ssize_t r = xread(s_stdout, s_readbuf + s_readbufcnt, remain);
	if (r == -1) {
		V("EAGAIN (rbc %zu)", s_readbufcnt);
		return 0;
	}
	if (r == 0)
		C("read: EOF");

	D("read from switch: %zd bytes", r);
	s_readbufcnt += (size_t)r;
	hexdump(s_readbuf, s_readbufcnt, "readbuf");
	return r;
}

static int
oper_writing(void)
{
	V("operate in WRITING state");
	if (!s_writebufcnt)
		C("line from user didn't end in newline"); //XXX so what

	char c = s_writebuf[0];
	V("writing 0x%02x aka '%c'", c, c);
	xwrite(s_stdin, &c, 1);
	shiftbuf(s_writebuf, &s_writebufcnt, 1);// XXX inefficient
	if (c == '\n') {
		V("resetting fsm, program cmdout");
		s_curfsm = s_fsm_cmdout;
		fsm_reset(s_curfsm);
	} else {
		V("resetting fsm, program inchar");
		s_curfsm = s_fsm_inchar;
		fsm_reset(s_curfsm);
	}
	V("state changed to BUSY");
	s_state = BUSY;
	return 1;
}

static int
oper_busy(void)
{
	V("operate in BUSY state");

	if (read_more() || s_readbufcnt) {
		s_nodataflag = false;
		size_t r = fsm_feed(s_curfsm, (const uint8_t *)s_readbuf,
		                    s_readbufcnt);
		if (r == 0) {
			W("fsm needs more data");
			return 0; //need more data
		}
		D("fsm ate %zu/%zu", r, s_readbufcnt);

		if (s_curfsm == s_fsm_init) {
			if (fsm_init_anykey())
				xwrite(s_stdin, "x", 1);

			if (fsm_init_report()) {
				xwrite(s_stdin, "\033[9999;130R", 11);
				msleep(100); //XXX HP quirk, move this
			}
		}

		shiftbuf(s_readbuf, &s_readbufcnt, r);

		if (fsm_error(s_curfsm))
			C("fsm in error state");

		return 1;
	}

	if (fsm_feed(s_curfsm, NULL, 0)) {
		fsm_feed(s_curfsm, NULL, 1);
	} else if (!s_nodataflag) {
		s_nodataflag = true;
		return 0;
	} else {
		C("meh"); //XXX
	}

	if (s_writebufcnt) {
		V("state changed to WRITING");
		s_state = WRITING;
	} else {
		if (s_outbufcnt)
			C("why isn't the outbuf empty here"); //XXX

		const char *out = "";
		if (s_curfsm == s_fsm_cmdout)
			out = fsm_cmdout_outbuf();

		size_t len = strlen(out);
		growbuf(&s_outbuf, &s_outbufsz, len+1);
		memcpy(s_outbuf, out, len);
		s_outbuf[len] = '\0';
		s_outbufcnt = len;

		const char *ps1 = "";
		if (s_curfsm == s_fsm_init)
			ps1 = fsm_init_ps1buf();
		else if (s_curfsm == s_fsm_cmdout)
			ps1 = fsm_cmdout_ps1buf();

		len = strlen(ps1);
		growbuf(&s_ps1buf, &s_ps1bufsz, len+1);
		memcpy(s_ps1buf, ps1, len);
		s_ps1buf[len] = '\0';
		s_ps1bufcnt = len;

		V("state changed to READY");
		s_state = READY;
	}

	return 1;
}
