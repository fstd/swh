/* fsm.c - finite state machine core, handled by sc and fsm_*
 * swh - switch ssh front-end - (C) 2017, Timo Buhrmester
 * See README for contact-, COPYING for license information. */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#define LOG_MOD MOD_BACK_FSM

#include "fsm.h"

#include <string.h>

#include "common/common.h"
#include "common/log.h"

//ind = sta * num_tok + tok
//sta = ind / num_tok
//tok = ind % num_tok
#define IND(FSM, STA, TOK) ((STA)*((FSM)->ntok)+(TOK))


static bool isaccepting(fsm *f, int state);


fsm *
fsm_new(size_t nsta, size_t ntok, int errst, int inist, int *accst,
        size_t naccst, struct fsm_trans *delta, fsm_mktok_fn f_mktok,
        fsm_reset_fn f_reset)
{
	fsm *f = xmalloc(sizeof *f);

	f->nsta = nsta;
	f->ntok = ntok;
	f->errst = errst;
	f->inist = inist;
	f->naccst = naccst;
	f->f_mktok = f_mktok;
	f->f_reset = f_reset;

	f->accst = xmalloc(naccst * sizeof *f->accst);
	memcpy(f->accst, accst, naccst * sizeof *f->accst);

	f->delta = xmalloc(ntok * nsta * sizeof *f->delta);
	memcpy(f->delta, delta, ntok * nsta * sizeof *f->delta);

	f->curst = f->inist;

	return f;
}

void
fsm_destroy(fsm *f)
{
	free(f->accst);
	free(f->delta);
	free(f);
	return;
}


size_t
fsm_feed(fsm *f, const uint8_t *data, size_t len)
{
	size_t i = 0;

	struct fsm_trans tr;

	if (f->curst == f->errst)
		return 0;

	/* if data is NULL, we want the loop body to execute once,
	 * feeding an EOF token into the machine. */
	while (!data || i < len) {
		int c, t;
		size_t toklen = 1;
		if (data) {
			c = data[i];
			t = f->f_mktok(data + i, len - i, &toklen);
			if (t == -1) { //not enough data
				W("not enough data");
				return i;
			}
		} else {
			c = EOF;
			t = f->f_mktok(NULL, 0, &toklen);
		}

		i += toklen;
		tr = f->delta[IND(f, f->curst, t)];
		if (!len) //probe only
			return isaccepting(f, tr.state);

		tr.action(c);
		f->curst = tr.state;

		if (f->curst == f->errst || !data)
			break;
	}

	return i;
}

void
fsm_reset(fsm *f)
{
	f->curst = f->inist;
	if (f->f_reset)
		f->f_reset();
	return;
}

bool
fsm_error(fsm *f)
{
	return f->curst == f->errst;
}

bool
fsm_accepting(fsm *f)
{
	return isaccepting(f, f->curst);
}

void
fsm_dump(fsm *f)
{
	A("curst=%d, errst=%d, ntok=%zu, nsta=%zu, delta=%p, "
	  "accst=%p, naccst=%zu,  inist=%d", f->curst,
	  f->errst, f->ntok, f->nsta, (void *)f->delta,
	  (void *)f->accst, f->naccst, f->inist);
	for (size_t i = 0; i < f->naccst; i++)
		A("accst[%zu]=%d", i, f->accst[i]);
	return;
}

static bool
isaccepting(fsm *f, int state)
{
	for (size_t i = 0; i < f->naccst; i++)
		if (state == f->accst[i])
			return true;
	return false;
}
