AC_INIT([lispy], [1.0], [scott@scottreynolds.us])
AM_INIT_AUTOMAKE([foreign -Wall -Wextra])
AC_PROG_CC_C99
AC_PROG_INSTALL
AM_PROG_CC_C_O
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile tests/Makefile])
AC_CHECK_LIB([edit], [readline])
AC_CHECK_LIB([m], [fabs])
AC_OUTPUT
