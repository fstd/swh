/* fsm_init.h - FSM abstraction for the initial chat, handled by sc
 * swh - switch ssh front-end - (C) 2017, Timo Buhrmester
 * See README for contact-, COPYING for license information. */

#ifndef BACK_FSM_INIT_H
#define BACK_FSM_INIT_H

#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

#include <sys/types.h>

#include "fsm.h"

/* The backend init-fsm interface and call dispatch struct */
struct fsm_init_if {
	fsm *(*f_init)(void);
	const char *(*f_ps1buf)(void);
	bool (*f_anykey)(void);
	bool (*f_report)(void);
};

/* This is the interface core uses to talk to whatever backend attached */
fsm *fsm_init_init(void);
const char *fsm_init_ps1buf(void);
bool fsm_init_anykey(void);
bool fsm_init_report(void);

/* Load backend fsm by name */
void fsm_init_attach(const char *ifname);

#endif
