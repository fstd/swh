/* common_hp.h - Common routines for the HP FSMs, available to fsm_*_hp
 * swh - switch ssh front-end - (C) 2017, Timo Buhrmester
 * See README for contact-, COPYING for license information. */

#ifndef BACK_HP_COMMON_HP_H
#define BACK_HP_COMMON_HP_H

#include <stddef.h>
#include <stdint.h>

#include "../../common/common.h"

void fsm_hp_common_init(void);
int fsm_hp_mktok(const uint8_t *in, size_t len, size_t *toklen,
                 int t_eseq, int t_rest, int t_eof);

#endif
