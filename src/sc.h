/* sc.h - Switch communication subsystem; handled by core
 * swh - switch ssh front-end - (C) 2017, Timo Buhrmester
 * See README for contact-, COPYING for license information. */

#ifndef SC_H
#define SC_H

#include <stdbool.h>
#include <stddef.h>
void sc_init(const char *backend);

int sc_start(const char *host);
int sc_operate(void);
void sc_write(const char *str, size_t len);

bool sc_hasreply(void);
const char *sc_getreply(void);
const char *sc_getps1(void);

bool sc_busy(void);
bool sc_offline(void);
bool sc_ready(void);

int sc_getfd(void);

#endif
