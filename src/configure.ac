dnl Process this file with autoconf to produce a configure script.

AC_INIT([Kanji Quiz], [0.0.1])
AC_PREREQ([2.5])

AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE([1.8])
AM_PROG_CC_C_O
AC_PROG_CXX

AC_CONFIG_HEADERS([config.h])

dnl Check for C compiler
AC_LANG([C])
AC_ISC_POSIX

dnl Checks for various programs
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

dnl Various checks just so we're really sane checking (proposed by AzaToth)
AC_C_VOLATILE

AC_TYPE_MODE_T
AC_TYPE_PID_T

AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STAT

dnl Next four lines is a hack to prevent libtool checking for C++/F77
m4_undefine([AC_PROG_CXX])
m4_defun([AC_PROG_CXX],[])
m4_undefine([AC_PROG_F77])
m4_defun([AC_PROG_F77],[])

dnl pkg-config
PKG_PROG_PKG_CONFIG

dnl GTK
PKG_CHECK_MODULES([GTK], [gtk+-2.0 >= 2.12.0])
PKG_CHECK_MODULES([GTHREAD], [gthread-2.0])
PKG_CHECK_MODULES([SQLITE], [sqlite3])
PKG_CHECK_MODULES([GSTREAMER], [gstreamer-0.10])


dnl CFLAGS
CFLAGS="$CFLAGS -Wall -Werror -std=c99 -fno-strict-aliasing -fmessage-length=0 -D_FORTIFY_SOURCE=2 -O3 -msse"

AC_CONFIG_FILES([
    Makefile
])

AC_OUTPUT

