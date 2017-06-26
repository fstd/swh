/* nami.h - front-/backend name information, handled by init
 * swh - switch ssh front-end - (C) 2017, Timo Buhrmester
 * See README for contact-, COPYING for license information. */

#ifndef NAMI_H
#define NAMI_H

#include <stdio.h>

void nami_backends(FILE *str);  /* print known backend names */
void nami_frontends(FILE *str); /* print known frontend names */

#endif
