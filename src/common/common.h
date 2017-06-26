/* common.h - Misc/utility functions available to all subsystems
 * swh - switch ssh front-end - (C) 2017, Timo Buhrmester
 * See README for contact-, COPYING for license information. */

#ifndef COMMON_COMMON_H
#define COMMON_COMMON_H

#include <stdbool.h>
#include <stddef.h>

#include <sys/types.h>

#define COUNTOF(ARR) (sizeof (ARR) / sizeof (ARR)[0])

void shiftbuf(char *buf, size_t *bufcnt, size_t n);
void growbuf(char **buf, size_t *bufsz, size_t minsz);
int selectfd(int fd, bool block);
void *xmalloc(size_t n);
void *xrealloc(void *p, size_t n);
void xwrite(int fd, const char *data, size_t len);
ssize_t xread(int fd, void *dest, size_t destsz);
void tscribe(const char *name, const char *data, size_t len, bool reading);
void msleep(unsigned long ms);
void setblocking(int fd, bool blocking);
void hexdump(const void *data, size_t len, const char *name);

#endif
