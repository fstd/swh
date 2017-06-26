/* uc.h - User communication subsystem; handled by core
 * swh - switch ssh front-end - (C) 2017, Timo Buhrmester
 * See README for contact-, COPYING for license information. */

#ifndef FRONT_UC_H
#define FRONT_UC_H

#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

#include <sys/types.h>

/* The uc interface and call dispatch struct */
struct uc_if {
	void    (*f_init)(void);
	int     (*f_hasdata)(bool block);
	ssize_t (*f_getdata)(char *dest, size_t destsz);
	bool    (*f_putdata)(const void *data, size_t datalen);
	void    (*f_dump)(void);
};

/* This is the interface core uses to talk to whatever frontend attached */
/* Initialize the uc */
void uc_init(void);

/* 0: no, 1: yes, -1: offline */
int uc_hasdata(bool block);

/* 0: no data, >0: data length, -1: offline */
ssize_t uc_getdata(char *dest, size_t destsz);

/* true: ok, false: offline */
bool uc_putdata(const void *data, size_t datalen);

/* Dump state for debugging */
void uc_dump(void);


/* Load uc by name */
void uc_attach(const char *ifname);

#endif
