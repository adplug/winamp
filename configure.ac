# Tell autoconf we're compiling a C++ DLL, using automake & libtool
AC_INIT(in_adlib,1.8.3)
AC_CONFIG_SRCDIR(src/plugin.cpp)
AC_CONFIG_FILES([Makefile src/Makefile in_adlib.psc])
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE
AC_LIBTOOL_WIN32_DLL
AC_LIBTOOL_RC
AM_DISABLE_STATIC
AM_PROG_LIBTOOL
AC_LANG(C++)

# Check for a sane C/C++ build environment.
AC_PROG_MAKE_SET
AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_CXX

# Check for needed libraries.
AC_CHECK_LIB(stdc++,main,,AC_MSG_ERROR([libstdc++ not installed]))
#AC_CHECK_LIB(binio,main,,AC_MSG_ERROR([libbinio not installed]))
#AC_CHECK_LIB(adplug,main,,AC_MSG_ERROR([AdPlug not installed]))
dnl PKG_CHECK_MODULES([adplug], [adplug >= 1.6])

PKG_CHECK_MODULES([adplug], [adplug >= 2.0],,[
AC_MSG_WARN([You seem to be using a version of AdPlug prior to 2.0. \
I will try to do the old-style library search for which i cannot check \
versions. Please bear in mind that i am requiring at least version 1.4.])
AC_CHECK_LIB(adplug,main,,AC_MSG_ERROR([*** AdPlug not installed ***]))])

# Sanitize some compiler features, which may be broken...
AC_C_CONST
AC_C_INLINE

# Enable debugging on user request.
AC_ARG_ENABLE([debug],AC_HELP_STRING([--enable-debug],
[Compile with debug logging support (default is to disable debug logging)]),
	AC_DEFINE(DEBUG))

AC_OUTPUT
