dnl Provides a test to determine the correct 
dnl way to call getrpcbyname_r
dnl
dnl defines HAVE_FUNC_GETRPCBYNAME_R_5 if it needs 5 arguments (e.g linux)
dnl defines HAVE_FUNC_GETRPCBYNAME_R_4 if it needs 4 arguments (e.g. solaris)
dnl

AC_DEFUN(AC_FUNC_WHICH_GETRPCBYNAME_R,
[AC_CACHE_CHECK(for which type of getrpcbyname_r, ac_cv_func_which_getrpcname_r, [
AC_CHECK_FUNC(getrpcbyname_r, [
	AC_TRY_COMPILE([
#		include <rpc/rpcent.h> 
  	], 	[

        char *name;
        struct rpcent *re;
	char buffer[2048];
	int buflen = 2048;
        (void) getrpcbyname_r(name, re, buffer, buflen)
		],ac_cv_func_which_getrpcname_r=four, 
			[

  AC_TRY_COMPILE([
#   include <rpc/netdb.h>
  ], [
	char *name;
	struct rpcent *re, *res;
	char buffer[2048];
	int buflen = 2048;
	(void) getrpcbyname_r(name, re, buffer, buflen, &res)
  ],ac_cv_func_which_getrpcname_r=five,
    ac_cv_func_which_getrpcname_r=no)
  ]
  
  )
  		]
	,ac_cv_func_which_getrpcname_r=no)])

if test $ac_cv_func_which_getrpcname_r = five; then
  AC_DEFINE(HAVE_FUNC_GETRPCBYNAME_R_5)
elif test $ac_cv_func_which_getrpcname_r = four; then
 AC_DEFINE(HAVE_FUNC_GETRPCBYNAME_R_4)
fi

])
