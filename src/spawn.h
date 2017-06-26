/* spawn.h - fork/exec ssh, provide pipes to stdout/in; handled by sc
 * swh - switch ssh front-end - (C) 2017, Timo Buhrmester
 * See README for contact-, COPYING for license information. */

#ifndef SPAWN_H
#define SPAWN_H

void spawn_init(char **envp);
void spawn_operate(void);
int spawn_launch(const char *host, int *stin, int *stout, int *sterr);
void spawn_kill(void);

#endif
