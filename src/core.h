/* core.h - Main loop, ties everything together; handled by init
 * swh - switch ssh front-end - (C) 2017, Timo Buhrmester
 * See README for contact-, COPYING for license information. */

#ifndef CORE_H
#define CORE_H

void core_init(const char *frontend, const char *backend, char **envp);
int core_run(const char *host);

#endif
