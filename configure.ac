#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([swh], [0.0.0], [van.fstd@gmail.com])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/init.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

# Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_CC_C99

AC_EGREP_CPP(posix_200112L_supported,
        [#define _POSIX_C_SOURCE 200112L
        #include <unistd.h>
        #ifdef _POSIX_VERSION
        #if _POSIX_VERSION >= 200112L
        posix_200112L_supported
        #endif
        #endif
        ],
        [AC_DEFINE([_POSIX_C_SOURCE], [200112L], [Request POSIX.1-2001])],
        [AC_MSG_FAILURE([The system needs to conform to POSIX.1-2001])])

# Checks for header files.
AC_CHECK_HEADERS([ctype.h errno.h fcntl.h getopt.h inttypes.h limits.h \
                  signal.h stdarg.h stdbool.h stddef.h stdint.h stdio.h \
                  stdlib.h string.h sys/select.h sys/types.h sys/wait.h \
                  sys/time.h time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT8_T
AC_TYPE_UINT64_T
AC_CHECK_TYPES([time_t])
AC_CHECK_TYPE([struct timespec], [AC_DEFINE([HAVE_STRUCT_TIMESPEC], [1],
              [struct timespec (select)])], [], [[#include <sys/time.h>]])
AC_CHECK_TYPE([struct timeval], [AC_DEFINE([HAVE_STRUCT_TIMEVAL], [1],
              [struct timeval (select)])], [], [[#include <sys/select.h>]])


# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STRERROR_R
AC_CHECK_FUNCS([close ctime_r dup2 execve exit fclose fcntl fflush fopen \
                fork fprintf fputs getenv getopt gettimeofday isdigit \
		isprint malloc memcpy memmove memset nanosleep open pipe \
		printf read realloc select setvbuf signal snprintf \
		sprintf strchr strcmp strcpy strerror_r strlen strncpy \
		strtok strtol time vsnprintf wait write])


AC_CONFIG_FILES([Makefile
                 src/common/Makefile
                 src/back/Makefile
                 src/back/hp/Makefile
                 src/front/Makefile
                 src/front/ia/Makefile
                 src/front/noop/Makefile
                 src/Makefile])
AC_OUTPUT

echo \
"-------------------------------------------------

 ${PACKAGE_NAME} Version ${PACKAGE_VERSION}

 Prefix: '${prefix}'.
 Compiler: '${CC} ${CFLAGS}'
 Libraries: '${LIBS}'

--------------------------------------------------"
scripts/mklogmods.sh
