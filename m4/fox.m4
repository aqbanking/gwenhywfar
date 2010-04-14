# $Id: fox.m4,v 1.4 2006/01/18 09:44:19 cstim Exp $
# (c) 2009 Martin Preuss<martin@libchipcard.de>
# These functions search for FOX


AC_DEFUN([AQ_CHECK_FOX],[
dnl PREREQUISITES:
dnl   AQ_CHECK_OS must be called before this
dnl IN: 
dnl   All variables which already have a value will not be altered
dnl OUT:
dnl   Variables:
dnl     have_fox - set to "yes" if FOX exists
dnl     fox_includes - path to includes
dnl     fox_libs - path to libraries
dnl   Defines:
dnl     HAVE_FOX

AC_MSG_CHECKING(if FOX is allowed)
AC_ARG_ENABLE(fox,
  [  --enable-fox         enable fox (default=yes)],
  enable_fox="$enableval",
  enable_fox="yes")
AC_MSG_RESULT($enable_fox)

if test "$enable_fox" = "no"; then
   fox_libs=""
   fox_includes=""
   have_fox="no"
else


dnl paths for fox includes
AC_MSG_CHECKING(for fox includes)
AC_ARG_WITH(fox-includes, 
  [  --with-fox-includes=DIR      uses fox includes from given dir],
  [local_fox_includes="$withval"],
  [local_fox_includes="\
        /usr/include/fox-1.6 \
        /usr/local/include/fox-1.6 \
        /opt/fox-1.6/include \
        "
  ]
)

if test -z "$fox_includes"; then
	for i in $local_fox_includes; do
		if test -z "$fox_includes"; then
                  if test -f "$i/fxver.h"; then
                    fox_includes="-I$i"
                    break;
                  fi
 		fi
        done
fi
if test -n "$fox_includes"; then
	AC_MSG_RESULT($fox_includes)
else
	AC_MSG_RESULT(not found)
fi


# Check for x86_64 architecture; potentially set lib-directory suffix
if test "$target_cpu" = "x86_64"; then
  libdirsuffix="64"
else
  libdirsuffix=""
fi

dnl paths for fox libs
AC_MSG_CHECKING(for fox libraries)
AC_ARG_WITH(fox-libs, 
  [  --with-fox-libs=SPEC      uses given fox libs ],
  [fox_libs="$withval"],
  [
      AC_LANG(C++)
      SAVED_LDFLAGS=$LDFLAGS
      SAVED_CXXFLAGS=$CXXFLAGS
      LDFLAGS="$LDFLAGS -lFOX-1.6"
      CXXFLAGS="$CXXFLAGS $fox_includes"
      AC_LINK_IFELSE(
        [AC_LANG_PROGRAM([#include <fx.h>],
          [FX::FXDate dummy])],
        [fox_libs="-lFOX-1.6"])
      LDFLAGS=$SAVED_LDFLAGS
      CXXFLAGS=$SAVED_CXXFLAGS
  ]
)



if test -z "$fox_libs"; then
   AQ_SEARCH_FOR_PATH([$qt_searchname],[$local_fox_libs])
   if test -n "$found_dir" ; then
     fox_libs="-L$found_dir -lFOX-1.6"
       test "$OSYSTEM" = "freebsd" && \
          fox_libs="$fox_libs -lc_r"
   fi
fi

if test -n "$fox_libs"; then
	AC_MSG_RESULT($fox_libs)
else
	AC_MSG_RESULT(not found)
fi


# check if all necessary fox components where found
if test -z "$fox_includes" || \
   test -z "$fox_libs"; then
	fox_libs=""
   	fox_includes=""
   	have_fox="no"
else
   have_fox="yes"
   AC_DEFINE(HAVE_FOX, 1, [whether FOX is available])
fi


dnl end of if "$enable_fox"
fi

AS_SCRUB_INCLUDE(fox_includes)
AC_SUBST(fox_libs)
AC_SUBST(fox_includes)

])
