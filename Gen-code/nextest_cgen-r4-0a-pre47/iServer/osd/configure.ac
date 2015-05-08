dnl Process this file with autoconf to produce a configure script.
AC_INIT(include/nxosd.h)
AC_CONFIG_AUX_DIR(config)
AM_INIT_AUTOMAKE(osd, 1.1)
AM_CONFIG_HEADER(include/tmpconfig.h)
AX_PREFIX_CONFIG_H(include/osdconfig.h,,include/tmpconfig.h)
AC_CANONICAL_HOST

AH_TEMPLATE([HAVE_FUNC_GETHOSTBYNAME_R_6],
            [Define if target system has gethostbyname_r 
             which takes six arguments])

AH_TEMPLATE([HAVE_FUNC_GETHOSTBYNAME_R_5],
            [Define if target system has gethostbyname_r 
             which takes five arguments])

AH_TEMPLATE([HAVE_FUNC_GETHOSTBYNAME_R_3],
            [Define if target system has gethostbyname_r 
             which takes three arguments])

AH_TEMPLATE([HAVE_FUNC_GETRPCBYNAME_R_5],
            [Define if target system has getrpcbyname_r 
             which takes five arguments])

AH_TEMPLATE([HAVE_FUNC_GETRPCBYNAME_R_4],
            [Define if target system has getrpcbyname_r 
             which takes four arguments])
             
dnl Checks for programs.

AC_PROG_CC
AC_PROG_RANLIB

dnl Checks for libraries.
AC_CHECK_LIB(check, suite_create,[],[
        echo "Warning: You need to have libcheck installed for running the tests"
        ])

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS( sys/ioctl.h sys/filio.h sys/sockio.h \
                  rpc/netdb.h rpc/rpcent.h libgen.h rpc/rpc.h )

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_CHECK_TYPES([longlong_t, uchar_t, hrtime_t, ulong_t])

dnl Checks for library functions.
AC_CHECK_LIB([gen],[mkdirp])
AC_SEARCH_LIBS([getrpcbyname_r],[nsl socket])
AC_SEARCH_LIBS([gethostbyname_r],[nsl socket])

AC_CHECK_FUNCS([gethrtime] [strlcpy] [strlcat] \
               [sig2str] [strsignal] [mkdirp] \
               [svctcp_create] [svc_create] [priocntl]) 

AC_FUNC_WHICH_GETHOSTBYNAME_R
AC_FUNC_WHICH_GETRPCBYNAME_R
dnl run make in tests subdir only if libcheck is installed
AM_CONDITIONAL(MAKE_TESTS, [test "$ac_cv_lib_check_suite_create" = yes])
AC_CONFIG_FILES(Makefile src/Makefile tests/Makefile )
AC_OUTPUT


