/* log.h - Logging subsystem
 * swh - switch ssh front-end - (C) 2017, Timo Buhrmester
 * See README for contact-, COPYING for license information. */

/* This provides macros logging diagnostics of several log levels to
 * stderr.  The log levels are:
 *
 * C(ritical): Fatal errors, complain and dump core.
 * E(rror):    Non-fatal (but typically functionality-breaking) errors
 * W(arning):  Suboptimal conditions
 * N(otice):   Noteworthy conditions of potential interest to users
 * I(nfo):     Noteworthy conditions of potential interest to developers
 * D(ebug):    Debug output, not totally flooding stderr
 * V(ivi):     Highly verbose debug output, flooding stderr
 * H(ex):      Hexdumps of read/written data.
 * T(race):    Trace calls.  Only available when NOTRACE isn't defined.
 *
 * Messages of these severities can be produced using the [CEWNIDVHT]()-
 * macros; using the [CEWNIDVHT]E()-macros (note the extra
 * 'E') will additionally print errno-related information.
 * CN() and CNE() are like C() and CE(), except they don't dump core.
 */
#ifndef COMMON_LOG_H
#define COMMON_LOG_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

enum {
#define X(MOD, NAMESTR) MOD,
#include "gen/logmods.h"
#undef X
	MOD_UNKNOWN,
	NUM_MODS,
};

/* CRIT - DEBUG are defined in terms of syslogd's loglevel */
#define LOG_CRIT 2
#define LOG_ERR 3
#define LOG_WARNING 4
#define LOG_NOTICE 5
#define LOG_INFO 6
#define LOG_DEBUG 7
#define LOG_VIVI 8
#define LOG_HEX 9
#define LOG_TRACE 10

#ifndef LOG_MOD
# define LOG_MOD MOD_UNKNOWN
#endif

#define COREDUMP do { fprintf(stderr, "dumping core\n"); \
                      if (!fork()) raise(SIGABRT); } while(0)

/* ----- logging interface ----- */

#define H(...) \
 log_log(LOG_MOD,LOG_HEX,-1,__FILE__,__LINE__,__func__,__VA_ARGS__)

#define V(...) \
 log_log(LOG_MOD,LOG_VIVI,-1,__FILE__,__LINE__,__func__,__VA_ARGS__)

#define VE(...) \
 log_log(LOG_MOD,LOG_VIVI,errno,__FILE__,__LINE__,__func__,__VA_ARGS__)

#define D( ...) \
 log_log(LOG_MOD,LOG_DEBUG,-1,__FILE__,__LINE__,__func__,__VA_ARGS__)

#define DE(...) \
 log_log(LOG_MOD,LOG_DEBUG,errno,__FILE__,__LINE__,__func__,__VA_ARGS__)

#define I(...) \
 log_log(LOG_MOD,LOG_INFO,-1,__FILE__,__LINE__,__func__,__VA_ARGS__)

#define IE(...) \
 log_log(LOG_MOD,LOG_INFO,errno,__FILE__,__LINE__,__func__,__VA_ARGS__)

#define N(...) \
 log_log(LOG_MOD,LOG_NOTICE,-1,__FILE__,__LINE__,__func__,__VA_ARGS__)

#define NE(...) \
 log_log(LOG_MOD,LOG_NOTICE,errno,__FILE__,__LINE__,__func__,__VA_ARGS__)

#define W(...) \
 log_log(LOG_MOD,LOG_WARNING,-1,__FILE__,__LINE__,__func__,__VA_ARGS__)

#define WE(...) \
 log_log(LOG_MOD,LOG_WARNING,errno,__FILE__,__LINE__,__func__,__VA_ARGS__)

#define E(...) \
 log_log(LOG_MOD,LOG_ERR,-1,__FILE__,__LINE__,__func__,__VA_ARGS__)

#define EE(...) \
 log_log(LOG_MOD,LOG_ERR,errno,__FILE__,__LINE__,__func__,__VA_ARGS__)

#define CN(...) do{ \
 log_log(LOG_MOD,LOG_CRIT,-1,__FILE__,__LINE__,__func__,__VA_ARGS__); \
 fflush(stderr); exit(EXIT_FAILURE); } while (0)

#define CNE(...) do { \
 log_log(LOG_MOD,LOG_CRIT,errno,__FILE__,__LINE__,__func__,__VA_ARGS__); \
 fflush(stderr); exit(EXIT_FAILURE); } while (0)

#define C(...) do { \
 log_log(LOG_MOD,LOG_CRIT,-1,__FILE__,__LINE__,__func__,__VA_ARGS__); \
 fflush(stderr); COREDUMP; exit(EXIT_FAILURE); } while (0)

#define CE(...) do { \
 log_log(LOG_MOD,LOG_CRIT,errno,__FILE__,__LINE__,__func__,__VA_ARGS__); \
 fflush(stderr); COREDUMP; exit(EXIT_FAILURE); } while (0)

/* special: always printed, never decorated */
#define A(...) \
 log_log(-1,INT_MIN,-1,__FILE__,__LINE__,__func__,__VA_ARGS__)

/* tracing */
#if NOTRACE
# define T(...) do{}while(0)
# define TC(...) do{}while(0)
# define TR(...) do{}while(0)
#else
# define T(...) \
 log_log(LOG_MOD,LOG_TRACE,-1,__FILE__,__LINE__,__func__,__VA_ARGS__)

# define TC(...) \
 do{ \
 log_log(LOG_MOD,LOG_TRACE,-1,__FILE__,__LINE__,__func__,__VA_ARGS__); \
 log_tcall(); \
 } while (0)

# define TR(...) \
 do{ \
 log_tret(); \
 log_log(LOG_MOD,LOG_TRACE,-1,__FILE__,__LINE__,__func__,__VA_ARGS__); \
 } while (0)
#endif

/* ----- logger control interface ----- */

void log_setlvl(int mod, int lvl);
void log_setlvl_all(int lvl);
int log_getlvl(int mod);

void log_setfancy(bool fancy);
bool log_getfancy(void);

void log_set_ourname(const char *name);

void log_tret(void);
void log_tcall(void);

/* ----- backend ----- */

void log_log(int mod, int lvl, int errn, const char *file, int line,
             const char *func, const char *fmt, ...)
#ifdef __GNUC__
    __attribute__ ((format (printf, 7, 8)))
#endif
    ;

void log_init(void);

#endif
