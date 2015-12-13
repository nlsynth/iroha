AC_INIT([iroha], [0.1.0], [tabata.yusuke@gmail.com])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([foreign])
# see m4/m4_ax_check_compile_flag
AX_CHECK_COMPILE_FLAG([-std=c++11],[CXXFLAGS="$CXXFLAGS -std=c++11"],[],[])
AC_PROG_CC
AC_PROG_CXX
LT_INIT

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
