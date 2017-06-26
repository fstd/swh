/* common_hp.c - Common routines for the HP FSMs, available to fsm_*_hp
 * swh - switch ssh front-end - (C) 2017, Timo Buhrmester
 * See README for contact-, COPYING for license information. */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#define LOG_MOD MOD_BACK_HP_COMMON_HP

#include "common_hp.h"

#include "../ansiseq.h"
#include "../../common/common.h"
#include "../../common/log.h"


void
fsm_hp_common_init(void)
{
	return;
}

int
fsm_hp_mktok(const uint8_t *in, size_t len, size_t *toklen,
             int t_eseq, int t_rest, int t_eof)
{
	if (!in)
		return t_eof;

	int c = in[0];

	if (c < 0 || c > 255)
		C("character out of range: %d", c);

	if (c == '\033') {
		int r = ansiseq_eatone(in, len, NULL);
		if (r == -1)
			return -1;

		*toklen = r;

		return t_eseq;
	}

	*toklen = 1;
	return t_rest;
}
