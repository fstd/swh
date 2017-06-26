#!/bin/sh
exec 2>/dev/null
make clean
make distclean
rm Makefile Makefile.in aclocal.m4 configure.scan autoscan*.log config.h \
    config.log config.status stamp-h1 config.h.in 'config.h.in~' configure

rm -r autom4te.cache build-aux m4 src/gen

find src \( -name .deps -o -name Makefile -o -name Makefile.in \) -print0 \
    | xargs -0 rm -r
