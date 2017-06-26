/* common.c - Misc/utility functions available to all subsystems
 * swh - switch ssh front-end - (C) 2017, Timo Buhrmester
 * See README for contact-, COPYING for license information. */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#define LOG_MOD MOD_COMMON_COMMON

#include "common.h"

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <signal.h>
#include <inttypes.h>
#include <time.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>

#include "log.h"


static void ywrite(int fd, const char *data, size_t len, bool transscribe);


/* shift data to the left by `n` elements */
void
shiftbuf(char *buf, size_t *bufcnt, size_t n)
{
	if (!n)
		return;
	*bufcnt -= n;
	memmove(buf, buf + n, *bufcnt);
	return;
}

/* ensure buffer pointed to by `*buf` can stomach at least `minsz` bytes */
void
growbuf(char **buf, size_t *bufsz, size_t minsz)
{
	if (minsz < *bufsz)
		return;

	size_t nsz = *bufsz;
	while (nsz < minsz)
		nsz *= 2;

	*buf = xrealloc(*buf, nsz);
	*bufsz = nsz;
	return;
}

/* tell if a fd is readable, optionally wait until it is
 * returns 1: readable, 0: not readable (cannot happen if `block` is true)
 * panics on error */
int
selectfd(int fd, bool block)
{
	int r;
	for (;;) {
		struct timeval tv = {0, 0};
		fd_set fds;
		FD_ZERO(&fds);
		if (fd >= 0) FD_SET(fd, &fds);
		D("waiting for fd %d to become readable", fd);
		r = select(fd+1, &fds, NULL, NULL, block ? NULL : &tv);

		if (r == -1) {
			if (errno == EINTR) {
				WE("select");
				continue;
			}

			CE("select");
		}
		if (r == 0 && block) {
			D("nothing");
			continue;
		}

		return r;
	}
	return 1;
}

/* wrapper around malloc(3), panics on error */
void *
xmalloc(size_t n)
{
	void *r = malloc(n);
	if (!r)
		C("malloc");
	return r;
}

/* wrapper around realloc(3), panics on error */
void *
xrealloc(void *p, size_t n)
{
	V("called for p=%p", p);
	void *r = realloc(p, n);
	if (!r)
		C("realloc");
	return r;
}

/* Like ywrite with transcription enabled */
void
xwrite(int fd, const char *data, size_t len)
{
	ywrite(fd, data, len, true);
	return;
}

/* Wrapper around read(2), panics on both error and EOF (for now, XXX),
 * but not on EAGAIN.  Restarts the call on EINTR (for now XXX)
 * returns the number of bytes read (> 0) or -1 on EAGAIN */
ssize_t
xread(int fd, void *dest, size_t destsz)
{
	char tsnam[16];
	snprintf(tsnam, sizeof tsnam, "swh_ts.fd%d", fd);
	ssize_t r;
	for (;;) {
		r = read(fd, dest, destsz);
		if (r == -1) {
			if (errno == EINTR) {
				WE("read");
				continue;
			}
			if (errno != EAGAIN) {
				tscribe(tsnam, "[READ ERROR]", 12, true);
				CE("read");
			}
		} else if (r == 0) {
			tscribe(tsnam, "[EOF]", 5, true);
			C("read: EOF"); // XXX
		} else
			tscribe(tsnam, dest, r, true);

		break;
	}

	return r;
}

/* transscribe read/write data for debugging purposes XXX */
void
tscribe(const char *name, const char *data, size_t len, bool reading)
{
	char path[256];
	static unsigned long s_seq;
	struct timeval tv;
	if (gettimeofday(&tv, NULL) == -1)
		CE("gettimeofday");
	char timestr[32];
	snprintf(timestr, sizeof timestr, "%"PRIu64".%04u/%lu: ",
	    (uint64_t)tv.tv_sec, (unsigned)(tv.tv_usec / 1000u), s_seq++);
	snprintf(path, sizeof path, "/tmp/transcript.%s", name);
	int fd = open(path, O_WRONLY | O_APPEND | O_CREAT, 0700);
	if (fd == -1)
		C("cannot open transcript file /tmp/transcript.%s", name);

	const char *s;
	if (reading)
		s = "READ data+2x newline:\n";
	else
		s = "WRITE data+2x newline:\n";

	ywrite(fd, timestr, strlen(timestr), false);
	ywrite(fd, s, strlen(s), false);
	ywrite(fd, data, len, false);
	s = "\n\n";
	ywrite(fd, s, strlen(s), false);
	if (close(fd) == -1)
		CE("close");
	return;
}

/* sleep for `ms` millisecs */
void
msleep(unsigned long ms)
{
	time_t secs = ms / 1000;
	long ns = (ms % 1000) * 1000000l;
	struct timespec treq = { secs, ns };
	nanosleep(&treq, NULL);
	return;
}

/* enable or disable blocking mode on fd `fd` */
void
setblocking(int fd, bool blocking)
{
	int fl = fcntl(fd, F_GETFL);
	if (fl == -1)
		CE("fcntl F_GETFL fd %d", fd);

	if (blocking)
		fl &= ~O_NONBLOCK;
	else
		fl |= O_NONBLOCK;

	if (fcntl(fd, F_SETFL, fl) == -1)
		CE("fcntl F_SETFL %x fd %d", (unsigned)fl, fd);
	return;
}

/* prints nothing at all, unless the verbosity level is pushed all
 * the way up to the hex-level (-vvvvv), in which case it prints
 * pretty hexdumps in hexdump(1) -C style */
void
hexdump(const void *data, size_t len, const char *name)
{
	if (!name) name = "unnamed";

	H("Hexdump '%s', %zu bytes:", name, len);

#define BPL 16
	size_t lines = len / BPL;

	char line[2+8+1+ 3*BPL+1 +1+BPL+1+1];
	char *lnend;

	const uint8_t *d = data;
	size_t co = 0; // current offset
	for (size_t l = 0; l < lines; l++) {
		lnend = line;
		lnend += sprintf(lnend, "%#08x ", (unsigned)co);
		for (size_t c = 0; c < BPL; c++) {
			lnend += sprintf(lnend, " %02x", d[co+c]);
			if (c % 8 == 7)
				*lnend++ = ' ';
		}

		*lnend++ = ' ';
		for (size_t c = 0; c < BPL; c++)
			*lnend++ = isprint(d[co+c]) ? d[co+c] : '_';

		*lnend = '\0';
		co += BPL;
		H("%s", line);
	}

	if (len % BPL) {
		size_t remain = len % BPL;
		lnend = line;
		lnend += sprintf(lnend, "%#08x ", (unsigned)co);
		for (size_t c = 0; c < BPL; c++) {
			if (c < remain)
				lnend += sprintf(lnend, " %02x", d[co+c]);
			else
				lnend += sprintf(lnend, " --");

			if (c % 8 == 7)
				*lnend++ = ' ';
		}

		*lnend++ = ' ';
		for (size_t c = 0; c < remain ; c++)
			*lnend++ = isprint(d[co+c]) ? d[co+c] : '_';

		*lnend = '\0';
		co += remain;
		H("%s", line);

	}

	H("%#08x (end)", (unsigned)co);
	return;
}



/* Wrapper around write(2), won't stop until `len` bytes are written,
 * panics on error, optionally transscribes written data to a files.
 * This function exists because the transcribe function itself uses
 * this interface and thus needs a way to disable transcribing while
 * writing the transcript. */
static void
ywrite(int fd, const char *data, size_t len, bool transscribe)
{
	char tsnam[16];
	snprintf(tsnam, sizeof tsnam, "swh_ts.fd%d", fd);

	size_t bc = 0;
	while (bc < len) {
		ssize_t r = write(fd, data + bc, len - bc);
		if (r < 0) {
			if (errno == EINTR) {
				WE("write");
				continue;
			}
			if (transscribe)
				tscribe(tsnam, "[WRITE ERR]", 13, false);
			CE("write");
		} else if (r == 0) {
			if (transscribe)
				tscribe(tsnam, "[WRITE 0]", 9, false);
			C("write: 0");
		}
		if (transscribe)
			tscribe(tsnam, data + bc, (size_t)r, false);
		V("wrote %zd bytes to fd %d", r, fd);
		bc += (size_t)r;
	}
	return;
}
