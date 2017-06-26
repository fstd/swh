/* fsm_cmdout_hp.c - FSM eating cmd output from HP gear, handled by sc
 * swh - switch ssh front-end - (C) 2017, Timo Buhrmester
 * See README for contact-, COPYING for license information. */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#define LOG_MOD MOD_BACK_HP_FSM_CMDOUT_HP

#include <string.h>

#include "../fsm_cmdout.h"
#include "common_hp.h"
#include "../../common/common.h"
#include "../../common/log.h"

#define S_STA 0 /* Starting state */
#define S_CM1 1 /* escape-sequence munching state 1 */
#define S_OUT 2 /* command output, prompt or question */
#define S_CM2 3 /* escape-sequence munching state 2 */
#define S_PS1 4 /* prompt */
#define S_CM3 5 /* escape-sequence munching state 3 */
#define S_FIN 6 /* accepting state */
#define S_ERR 7 /* error state */
#define NUM_STATES 8

#define T_ESEQ 0 /* ANSI escape sequence */
#define T_REST 1 /* anything except escape seq and EOF */
#define T_EOF  2 /* end of data */
#define NUM_TOKENS 3

#define OUTBUFSZ 4096
#define PS1BUFSZ 256


static fsm *s_fsm; /* the actual state machine */

static char *s_outbuf, *s_ps1buf; /* hold output and prompt respectively */
static size_t s_outbufsz, s_ps1bufsz, s_outbufcnt, s_ps1bufcnt;


static void reset(void);
static void act_nop(int c);
static void act_noout(int c);
static void act_rec(int c);
static void act_recps(int c);
static int mktok(const uint8_t *in, size_t len, size_t *toklen);
static fsm *init(void);
static const char *ps1buf(void);
static const char *outbuf(void);
static size_t outbufcnt(void);

/* A table entry {S_FOO, act_bar} at row S_ROW and column T_COL means
 * that if we are in state S_ROW, for an input token T_COL we'll
 * transition to state S_FOO and call act_bar() in the process */

#define ERR {S_ERR, act_nop}
/* eat command output, a question, or possibly just a prompt change */
static struct fsm_trans delta[NUM_STATES * NUM_TOKENS] = {
/* (cmdout) T_ESEQ              T_REST                T_EOF */
/* S_STA */ {S_CM1, act_nop},   ERR,                  ERR,
/* S_CM1 */ {S_CM1, act_nop},   {S_OUT, act_rec},     ERR,
/* S_OUT */ {S_CM2, act_nop},   {S_OUT, act_rec},     ERR,
/* S_CM2 */ {S_CM2, act_nop},   {S_PS1, act_recps},   {S_FIN, act_noout},
/* S_PS1 */ {S_CM3, act_nop},   {S_PS1, act_recps},   ERR,
/* S_CM3 */ {S_CM3, act_nop},   ERR,                  {S_FIN, act_nop},
/* S_FIN */ ERR,                ERR,                  ERR,
/* S_ERR */ ERR,                ERR,                  ERR,
};
#undef ERR


static void
reset(void)
{
	s_outbufcnt = 0;
	s_outbuf[0] = '\0';
	s_ps1bufcnt = 0;
	s_ps1buf[0] = '\0';
	return;
}

static void
act_nop(int c)
{
	(void)c;
	return;
}

/* done, we have a question, or just a PS1 change but no real output */
static void
act_noout(int c)
{
	(void)c;
	size_t nsz = s_ps1bufsz;
	while (s_outbufcnt >= nsz)
		nsz *= 2;

	if (nsz > s_ps1bufsz) {
		s_ps1buf = xrealloc(s_ps1buf, nsz);
		s_ps1bufsz = nsz;
	}

	memcpy(s_ps1buf, s_outbuf, s_outbufcnt);
	s_ps1bufcnt = s_outbufcnt;
	s_ps1buf[s_ps1bufcnt] = '\0';
	s_outbufcnt = 0;
	s_outbuf[0] = '\0';
	return;
}

/* record this character to output buffer */
static void
act_rec(int c)
{
	if ((s_outbufcnt+1) >= s_outbufsz) {
		s_outbuf = xrealloc(s_outbuf, s_outbufsz * 2);
		s_outbuf[s_outbufcnt] = '\0';
		s_outbufsz *= 2;
	}

	s_outbuf[s_outbufcnt++] = c;
	s_outbuf[s_outbufcnt] = '\0';
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
	(s_outbuf = xmalloc(s_outbufsz = OUTBUFSZ))[0] = '\0';
	return s_fsm;
}

static const char *
ps1buf(void)
{
	return s_ps1buf;
}

static const char *
outbuf(void)
{
	return s_outbuf;
}

static size_t
outbufcnt(void)
{
	return s_outbufcnt;
}


void
fsm_cmdout_hp_attach(struct fsm_cmdout_if *ifc)
{
	/* attach pointers to the above functions to the interface
	 * struct pointed to by `ifc` */
	ifc->f_init = init;
	ifc->f_ps1buf = ps1buf;
	ifc->f_outbuf = outbuf;
	ifc->f_outbufcnt = outbufcnt;
	I("fsm_cmdout_hp attached");
	return;
}
