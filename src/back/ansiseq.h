/* ansiseq.h - State machine eating ANSI escape seqs
 * swh - switch ssh front-end - (C) 2017, Timo Buhrmester
 * See README for contact-, COPYING for license information. */

#ifndef BACK_ANSISEQ_H
#define BACK_ANSISEQ_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limits.h>

#define MAX_ANSISEQ_PARAMS 8
#define MAX_OSC_STR_SZ 256
#define ABSENT INT_MIN

struct ansiseq {
	uint8_t cls;
	uint8_t cmd;
	uint8_t strarg[MAX_OSC_STR_SZ];
	bool ext;
	int argv[MAX_ANSISEQ_PARAMS];
	int argc;
};

void ansiseq_init(void);

int ansiseq_eatone(const uint8_t *data, size_t len, struct ansiseq *dst);

void ansiseq_dump(struct ansiseq *cs);

#endif
