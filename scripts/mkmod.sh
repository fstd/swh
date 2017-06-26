#!/bin/sh
# mkmod.sh - Convenience script to create new source/header pairs
# swh - switch ssh front-end - (C) 2017, Timo Buhrmester
# See README for contact-, COPYING for license information. */

[ -f src/init.c ] || Nuke "Please run from source root directory"

[ -n "$1" ] || { echo "Usage: $0 <modname>" >&2; exit 1; }

modnam="$1"
umodnam="$(echo "$modnam" | tr 'a-z' 'A-Z')"

sed -e "s/@@UMODNAM@@/$umodnam/g" -e "s/@@MODNAM@@/$modnam/g" \
    scripts/mod.c.tmpl >src/$modnam.c

sed -e "s/@@UMODNAM@@/$umodnam/g" -e "s/@@MODNAM@@/$modnam/g" \
    scripts/mod.h.tmpl >src/$modnam.h
