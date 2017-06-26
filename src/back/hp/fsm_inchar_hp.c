/* fsm_inchar_hp.c - FSM eating input char echo from HP gear, handled by sc
 * swh - switch ssh front-end - (C) 2017, Timo Buhrmester
 * See README for contact-, COPYING for license information. */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#define LOG_MOD MOD_BACK_HP_FSM_INCHAR_HP

#include "../fsm_inchar.h"
#include "common_hp.h"
#include "../../common/common.h"
#include "../../common/log.h"

#define S_STA 0 /* Starting state */
#define S_CM1 1 /* escape-sequences preceding the character echo */
#define S_INC 2 /* input character echo */
#define S_CM2 3 /* escape-sequences following the character echo */
#define S_FIN 4 /* accepting state */
#define S_ERR 5 /* error state */
#define NUM_STATES 6

#define T_ESEQ 0 /* ANSI escape sequence */
#define T_REST 1 /* anything except escape seq and EOF */
#define T_EOF  2 /* end of data */
#define NUM_TOKENS 3


static fsm *s_fsm; /* the actual state machine */


static void reset(void);
static void act_nop(int c);
static int mktok(const uint8_t *in, size_t len, size_t *toklen);
static fsm *init(void);

/* A table entry {S_FOO, act_bar} at row S_ROW and column T_COL means
 * that if we are in state S_ROW, for an input token T_COL we'll
 * transition to state S_FOO and call act_bar() in the process */

#define ERR {S_ERR, act_nop}
/* eat the echo we get per input character */
static struct fsm_trans delta[NUM_STATES * NUM_TOKENS] = {
/* (inchar) T_ESEQ              T_REST                T_EOF */
/* S_STA */ {S_CM1, act_nop},   ERR,                  ERR,
/* S_CM1 */ ERR,                {S_INC, act_nop},     ERR,
/* S_INC */ {S_CM2, act_nop},   ERR,                  ERR,
/* S_CM2 */ {S_CM2, act_nop},   ERR,                  {S_FIN, act_nop},
/* S_FIN */ ERR,                ERR,                  ERR,
/* S_ERR */ ERR,                ERR,                  ERR,
};
#undef ERR


static void
reset(void)
{
	return;
}

static void
act_nop(int c)
{
	(void)c;
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
	return s_fsm;
}


void
fsm_inchar_hp_attach(struct fsm_inchar_if *ifc)
{
	/* attach pointers to the above functions to the interface
	 * struct pointed to by `ifc` */
	ifc->f_init = init;
	I("fsm_inchar_hp attached");
	return;
}
