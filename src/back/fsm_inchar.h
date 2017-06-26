/* fsm_inchar.h - FSM abstraction for input char echo, handled by sc
 * swh - switch ssh front-end - (C) 2017, Timo Buhrmester
 * See README for contact-, COPYING for license information. */

#ifndef BACK_FSM_INCHAR_H
#define BACK_FSM_INCHAR_H

#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

#include <sys/types.h>

#include "fsm.h"

/* The backend inchar-fsm interface and call dispatch struct */
struct fsm_inchar_if {
	fsm *(*f_init)(void);
};

/* This is the interface core uses to talk to whatever backend attached */
fsm *fsm_inchar_init(void);

/* Load backend fsm by name */
void fsm_inchar_attach(const char *ifname);

#endif
