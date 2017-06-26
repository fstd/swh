/* fsm_init_hp.c - FSM eating initial chat from HP gear, handled by sc
 * swh - switch ssh front-end - (C) 2017, Timo Buhrmester
 * See README for contact-, COPYING for license information. */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#define LOG_MOD MOD_BACK_HP_FSM_INIT_HP

#include "../fsm_init.h"
#include "common_hp.h"
#include "../../common/common.h"
#include "../../common/log.h"

#define S_STA 0 /* Starting state */
#define S_LIC 1 /* license/copyright bullshit */
#define S_CM1 2 /* escape-sequences following the copyright stuff */
#define S_ANY 3 /* please press the any key */
#define S_CM2 4 /* escape-sequences following the anykey prompt */
#define S_LAS 5 /* your last login was ... */
#define S_CM3 6 /* escape-sequences following the last login message */
#define S_PS1 7 /* prompt */
#define S_CM4 8 /* escape-sequences following the prompt */
#define S_FIN 9 /* accepting state */
#define S_ERR 10 /* error state */
#define NUM_STATES 11

#define T_ESEQ 0 /* ANSI escape sequence */
#define T_REST 1 /* anything except escape seq and EOF */
#define T_EOF  2 /* end of data */
#define NUM_TOKENS 3

#define PS1BUFSZ 256


static fsm *s_fsm; /* the actual state machine */

static char *s_ps1buf; /* hold prompt */
static size_t s_ps1bufsz, s_ps1bufcnt;

static bool s_anykey; /* flag - the any key has to be pressed */
static bool s_report; /* flag - cursor position^W^Wterminal size report */


static void reset(void);
static void act_nop(int c);
static void act_ak(int c);
static void act_report(int c);
static void act_recps(int c);
static int mktok(const uint8_t *in, size_t len, size_t *toklen);
static fsm *init(void);
static const char *ps1buf(void);
static bool anykey(void);
static bool report(void);

/* A table entry {S_FOO, act_bar} at row S_ROW and column T_COL means
 * that if we are in state S_ROW, for an input token T_COL we'll
 * transition to state S_FOO and call act_bar() in the process */

#define ERR {S_ERR, act_nop}
/* eat the initial conversation (license, press the any key, etc) */
static struct fsm_trans delta[NUM_STATES * NUM_TOKENS] = {
/* (init)   T_ESEQ              T_REST                T_EOF */
/* S_STA */ ERR,                {S_LIC, act_nop},     ERR,
/* S_LIC */ {S_CM1, act_nop},   {S_LIC, act_nop},     ERR,
/* S_CM1 */ {S_CM1, act_nop},   {S_ANY, act_nop},     ERR,
/* S_ANY */ {S_CM2, act_ak},    {S_ANY, act_nop},     ERR,
/* S_CM2 */ {S_CM2, act_nop},   {S_LAS, act_report},  ERR,
/* S_LAS */ {S_CM3, act_nop},   {S_LAS, act_nop},     ERR,
/* S_CM3 */ {S_CM3, act_nop},   {S_PS1, act_recps},   ERR,
/* S_PS1 */ {S_CM4, act_nop},   {S_PS1, act_recps},   ERR,
/* S_CM4 */ {S_CM4, act_nop},   ERR,                  {S_FIN, act_nop},
/* S_FIN */ ERR,                ERR,                  ERR,
/* S_ERR */ ERR,                ERR,                  ERR,
};
#undef ERR


static void
reset(void)
{
	s_ps1bufcnt = 0;
	s_ps1buf[0] = '\0';
	s_anykey = false;
	s_report = false;
	return;
}

static void
act_nop(int c)
{
	(void)c;
	return;
}

static void
act_ak(int c)
{
	(void)c;
	s_anykey = true;
	return;
}

static void
act_report(int c)
{
	(void)c;
	s_report = true;
	return;
}

/* record this character to PS1 buffer */
static void
act_recps(int c)
{
	if ((s_ps1bufcnt+1) >= s_ps1bufsz)
		s_ps1buf = xrealloc(s_ps1buf, s_ps1bufsz *= 2);

	s_ps1buf[s_ps1bufcnt++] = c;
	s_ps1buf[s_ps1bufcnt] = '\0';
	return;
}

static int
mktok(const uint8_t *in, size_t len, size_t *toklen)
{
	return fsm_hp_mktok(in, len, toklen, T_ESEQ, T_REST, T_EOF);
}

static fsm *
init(void)
{
	s_fsm = fsm_new(NUM_STATES, NUM_TOKENS, S_ERR, S_STA,
	                (int[]){S_FIN}, 1, delta, mktok, reset);
	(s_ps1buf = xmalloc(s_ps1bufsz = PS1BUFSZ))[0] = '\0';
	return s_fsm;
}

static const char *
ps1buf(void)
{
	return s_ps1buf;
}

static bool
anykey(void)
{
	bool b = s_anykey;
	s_anykey = false;
	return b;
}

static bool
report(void)
{
	bool b = s_report;
	s_report = false;
	return b;
}


void
fsm_init_hp_attach(struct fsm_init_if *ifc)
{
	/* attach pointers to the above functions to the interface
	 * struct pointed to by `ifc` */
	ifc->f_init = init;
	ifc->f_ps1buf = ps1buf;
	ifc->f_anykey = anykey;
	ifc->f_report = report;
	I("fsm_init_hp attached");
	return;
}
