#!/bin/sh
# mklogmods.sh - Ham-fistedly generate an X-macro for the logger
# swh - switch ssh front-end - (C) 2017, Timo Buhrmester
# See README for contact-, COPYING for license information. */

dest='src/gen/logmods.h'

Nuke()
{
	printf '%s: ERROR: %s\n' "$0" "$*"
	exit 1
}

[ -f src/init.c ] || Nuke "Please run from source root directory"
mkdir -p src/gen

printf '//This file was generated on %s by %s\n' "$(date)" "$0" >$dest
for f in $(grep -rl '^#include "\([./]*common/\)*log.h"' src/*.c src/*/*.c src/*/*/*.c \
    | sed -e 's/^src\///' -e 's/\.c$//' \
    | grep -vF -e log -e tmpl); do
	ucase="$(printf '%s\n' "$f" | tr a-z A-Z | sed 's,/,_,g')"
	printf 'X(MOD_%s, "%s")\n' "$ucase" "$f"
done >>$dest
