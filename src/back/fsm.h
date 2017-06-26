/* fsm.h - finite state machine core, handled by sc and fsm_*
 * swh - switch ssh front-end - (C) 2017, Timo Buhrmester
 * See README for contact-, COPYING for license information. */

#ifndef BACK_FSM_H
#define BACK_FSM_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef void (*fsm_reset_fn)(void);
typedef int (*fsm_mktok_fn)(const uint8_t *in, size_t inlen, size_t *tlen);

struct fsm {
	size_t nsta;              /* number of states */
	size_t ntok;              /* number of tokens */
	int curst;                /* current state */
	int errst;                /* error state */
	int inist;                /* initial state */
	int *accst;               /* accepting states */
	size_t naccst;            /* number of accepting states */
	struct fsm_trans *delta;  /* transition table (flattened) */
	fsm_mktok_fn f_mktok;     /* tokenizer function */
	fsm_reset_fn f_reset;     /* reset callback (optional) */
};

typedef struct fsm fsm;

struct fsm_trans {
	int state;
	void (*action)(int);
};

fsm *fsm_new(size_t nsta, size_t ntok, int errst, int inist, int *accst,
             size_t naccst, struct fsm_trans *delta, fsm_mktok_fn f_mktok,
             fsm_reset_fn f_reset);

void fsm_destroy(fsm *f);

size_t fsm_feed(fsm *f, const uint8_t *data, size_t len);
void fsm_reset(fsm *f);

bool fsm_error(fsm *f);
bool fsm_accepting(fsm *f);

void fsm_dump(fsm *f);

#endif
