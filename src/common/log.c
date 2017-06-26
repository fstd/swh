/* log.c - Logging subsystem
 * swh - switch ssh front-end - (C) 2017, Timo Buhrmester
 * See README for contact-, COPYING for license information. */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include "log.h"

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DEF_LVL LOG_WARNING

#define COL_REDINV "\033[07;31;01m"
#define COL_RED "\033[31;01m"
#define COL_YELLOW "\033[33;01m"
#define COL_GREEN "\033[32;01m"
#define COL_WHITE "\033[37;01m"
#define COL_WHITEINV "\033[07;37;01m"
#define COL_GRAY "\033[30;01m"
#define COL_GRAYINV "\033[07;30;01m"
#define COL_LBLUE "\033[34;01m"
#define COL_RST "\033[0m"

#define COUNTOF(ARR) (sizeof (ARR) / sizeof (ARR)[0])


static const char *modnams[NUM_MODS] = {
#define X(MOD, NAMESTR) [MOD] = NAMESTR,
#include "gen/logmods.h"
#undef X
	[MOD_UNKNOWN] = "(??" "?)"
};

static bool s_fancy;
static bool s_init;
static int s_lvlarr[NUM_MODS];

static int s_w_modnam = 8;
static int s_w_file = 15;
static int s_w_line = 4;
static int s_w_func = 19;

static int s_calldepth = 0;
static char s_ourname[32];


static const char *lvlnam(int lvl);
static const char *lvlcol(int lvl);
static bool isdigitstr(const char *p);
static int getenv_m(const char *nam, char *dest, size_t destsz);


void
log_setlvl(int mod, int lvl)
{
	s_lvlarr[mod] = lvl;
}

void
log_setlvl_all(int lvl)
{
	for (size_t i = 0; i < NUM_MODS; i++)
		s_lvlarr[i] = lvl;
}

int
log_getlvl(int mod)
{
	return s_lvlarr[mod];
}

void
log_setfancy(bool fancy)
{
	s_fancy = fancy;
}

bool
log_getfancy(void)
{
	return s_fancy;
}

void
log_set_ourname(const char *name)
{
	strncpy(s_ourname, name, sizeof s_ourname - 1);
	s_ourname[sizeof s_ourname - 1] = '\0';
}

void
log_log(int mod, int lvl, int errn, const char *file, int line,
        const char *func, const char *fmt, ...)
{
	if (!s_init)
		log_init();

	bool always = lvl == INT_MIN;

	if (lvl > s_lvlarr[mod])
		return;

	char resmsg[4096];

	char payload[4096];
	va_list vl;
	va_start(vl, fmt);

	vsnprintf(payload, sizeof payload, fmt, vl);
	char *c = payload;
	while (*c) {
		if (*c == '\n' || *c == '\r')
			*c = '$';
		c++;
	}

	char errmsg[256];
	errmsg[0] = '\0';
	if (errn >= 0) {
		errmsg[0] = ':';
		errmsg[1] = ' ';
		strerror_r(errn, errmsg + 2, sizeof errmsg - 2);
	}

	if (always) {
		fputs(payload, stderr);
		fputs("\n", stderr);
	} else {
		char pad[256];
		if (lvl == LOG_TRACE) {
			size_t d = s_calldepth * 2;
			if (d > sizeof pad)
				d = sizeof pad - 1;
			memset(pad, ' ', d);
			pad[d] = '\0';
		} else
			pad[0] = '\0';

		char timebuf[27];
		time_t t = time(NULL);
		if (!ctime_r(&t, timebuf))
			strcpy(timebuf, "(ctime() failed)");
		char *ptr = strchr(timebuf, '\n');
		if (ptr)
			*ptr = '\0';

		snprintf(resmsg, sizeof resmsg,
		    "%s%s: %s %s: %*s:%*d:%*s(): %s%s%s%s\n",
		    s_fancy ? lvlcol(lvl) : "",
		    timebuf,
		    s_ourname,
		    lvlnam(lvl),
		    s_w_file, file,
		    s_w_line, line,
		    s_w_func, func,
		    pad,
		    payload,
		    errmsg,
		    s_fancy ? COL_RST : "");

		fputs(resmsg, stderr);
		fflush(stderr);
	}

	va_end(vl);
}

void
log_init(void)
{
	int deflvl = DEF_LVL;
	for (size_t i = 0; i < COUNTOF(s_lvlarr); i++)
		s_lvlarr[i] = INT_MIN;

	strncpy(s_ourname, PACKAGE_NAME, sizeof s_ourname - 1);
	s_ourname[sizeof s_ourname - 1] = '\0';

	char v[128];
	if (getenv_m(PACKAGE_NAME"_DEBUG", v, sizeof v) == 0 && v[0]) {
		for (char *t = strtok(v, " "); t; t = strtok(NULL, " ")) {
			char *eq = strchr(t, '=');
			if (eq) {
				if (t[0] == '=' || !isdigitstr(eq+1))
					continue;

				*eq = '\0';
				size_t mod = 0;
				for (; mod < COUNTOF(modnams); mod++)
					if (strcmp(modnams[mod], t) == 0)
						break;

				if (mod < COUNTOF(s_lvlarr))
					s_lvlarr[mod] =
					    (int)strtol(eq+1, NULL, 10);

				*eq = '=';
				continue;
			}
			eq = strchr(t, ':');
			if (eq) {
				if (t[0] == ':' || !isdigitstr(eq+1))
					continue;

				*eq = '\0';
				int val = (int)strtol(eq+1, NULL, 10);
				if (strcmp(t, "modnam") == 0) {
					s_w_modnam = val;
				} else if (strcmp(t, "file") == 0) {
					s_w_file = val;
				} else if (strcmp(t, "line") == 0) {
					s_w_line = val;
				} else if (strcmp(t, "func") == 0) {
					s_w_func = val;
				}

				*eq = ':';
				continue;
			}
			if (isdigitstr(t)) {
				/* special case: a stray number
				 * means set default loglevel */
				deflvl = (int)strtol(t, NULL, 10);
			}
		}
	}

	for (size_t i = 0; i < COUNTOF(s_lvlarr); i++)
		if (s_lvlarr[i] == INT_MIN)
			s_lvlarr[i] = deflvl;

	const char *vv = getenv(PACKAGE_NAME"_DEBUG_FANCY");
	if (vv && vv[0] != '0')
		log_setfancy(true);
	else
		log_setfancy(false);

	s_init = true;
}

void
log_tret(void)
{
	if (s_calldepth == 0) {
		fprintf(stderr, "call depth would be <0\n");
		return;
	}
	s_calldepth--;
}

void
log_tcall(void)
{
	s_calldepth++;
}



static const char *
lvlnam(int lvl)
{
	return lvl == LOG_DEBUG ? "DBG" :
	       lvl == LOG_TRACE ? "TRC" :
	       lvl == LOG_HEX ? "HEX" :
	       lvl == LOG_VIVI ? "VIV" :
	       lvl == LOG_INFO ? "INF" :
	       lvl == LOG_NOTICE ? "NOT" :
	       lvl == LOG_WARNING ? "WRN" :
	       lvl == LOG_CRIT ? "CRT" :
	       lvl == LOG_ERR ? "ERR" : "???";
}

static const char *
lvlcol(int lvl)
{
	return lvl == LOG_DEBUG ? COL_GRAY :
	       lvl == LOG_TRACE ? COL_LBLUE :
	       lvl == LOG_HEX ? COL_WHITEINV :
	       lvl == LOG_VIVI ? COL_GRAYINV :
	       lvl == LOG_INFO ? COL_WHITE :
	       lvl == LOG_NOTICE ? COL_GREEN :
	       lvl == LOG_WARNING ? COL_YELLOW :
	       lvl == LOG_CRIT ? COL_REDINV :
	       lvl == LOG_ERR ? COL_RED : COL_WHITEINV;
}

static bool
isdigitstr(const char *p)
{
	while (*p)
		if (!isdigit((unsigned char)*p++))
			return false;
	return true;
}

static int
getenv_m(const char *nam, char *dest, size_t destsz)
{
	const char *v = getenv(nam);
	if (!v)
		return -1;

	snprintf(dest, destsz, "%s", v);
	return 0;
}
