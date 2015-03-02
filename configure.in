# http://www.linuxselfhelp.com/gnu/autoconf/html_chapter/autoconf_4.html

AC_PREREQ(2.50)
AC_INIT([supervisor], [0.0.1])
AC_REVISION([m4_esyscmd_s([git describe --always])])

AC_ARG_ENABLE(name_heuristic,AC_HELP_STRING([--enable-name-heuristic],[Enable name heuristic (default: no)]))
AS_IF([test "x$enable_name_heuristic" = "xyes"], [
	AC_DEFINE([NAME_HEURISTIC], [1], [Use part of supervisor file name to find config and log files])
])
AC_SUBST(NAME_HEURISTIC)

AC_ARG_ENABLE(syslog, AC_HELP_STRING([--enable-syslog], [Enable logging via syslog (default: enabled)]))
AS_IF([test "x$enable_syslog" != "xno"], [
	AC_DEFINE([USE_SYSLOG], [1], [Enable logging via syslog])
])
AC_SUBST(USE_SYSLOG)

# AC_CONFIG_FILES, AC_CONFIG_HEADERS, AC_CONFIG_COMMANDS and AC_CONFIG_LINK
AC_CONFIG_FILES([Makefile])
AC_CONFIG_HEADERS([defines.h])

AC_OUTPUT

