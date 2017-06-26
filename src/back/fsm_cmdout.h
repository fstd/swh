/* fsm_cmdout.h - FSM abstraction for command output, handled by sc
 * swh - switch ssh front-end - (C) 2017, Timo Buhrmester
 * See README for contact-, COPYING for license information. */

#ifndef BACK_FSM_CMDOUT_H
#define BACK_FSM_CMDOUT_H

#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

#include <sys/types.h>

#include "fsm.h"

/* The backend cmdout-fsm interface and call dispatch struct */
struct fsm_cmdout_if {
	fsm *(*f_init)(void);
	const char *(*f_ps1buf)(void);
	bool (*f_anykey)(void);
	bool (*f_report)(void);
	const char *(*f_outbuf)(void);
	size_t (*f_outbufcnt)(void);
};

/* This is the interface core uses to talk to whatever backend attached */
fsm *fsm_cmdout_init(void);
const char *fsm_cmdout_ps1buf(void);
const char *fsm_cmdout_outbuf(void);
size_t fsm_cmdout_outbufcnt(void);
bool fsm_cmdout_anykey(void);
bool fsm_cmdout_report(void);

/* Load backend fsm by name */
void fsm_cmdout_attach(const char *ifname);

#endif
