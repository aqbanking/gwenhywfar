# $Id$
# (c) 2002 Martin Preuss<martin@libchipcard.de>
# This function checks for libgwenhywfar

AC_DEFUN(AC_GWENHYWFAR, [
dnl searches for gwenhywfar
dnl Arguments: 
dnl   $1: major version minimum
dnl   $2: minor version minimum
dnl   $3: patchlevel version minimum
dnl   $4: build version minimum
dnl Returns: gwenhywfar_dir
dnl          gwenhywfar_libs
dnl          gwenhywfar_includes
dnl          have_gwenhywfar

if test -z "$1"; then vma="0"; else vma="$1"; fi
if test -z "$2"; then vmi="1"; else vmi="$2"; fi
if test -z "$3"; then vpl="0"; else vpl="$3"; fi
if test -z "$4"; then vbld="0"; else vbld="$4"; fi

AC_MSG_CHECKING(if gwenhywfar support desired)
AC_ARG_ENABLE(gwenhywfar,
  [  --enable-gwenhywfar      enable gwenhywfar support (default=yes)],
  enable_gwenhywfar="$enableval",
  enable_gwenhywfar="yes")
AC_MSG_RESULT($enable_gwenhywfar)

have_gwenhywfar="no"
gwenhywfar_dir=""
gwenhywfar_libs=""
gwenhywfar_includes=""
if test "$enable_gwenhywfar" != "no"; then
  AC_MSG_CHECKING(for gwenhywfar)
  AC_ARG_WITH(gwen-dir, [  --with-gwen-dir=DIR
                            uses gwenhywfar from given dir],
    [lcc_dir="$withval"],
    [lcc_dir="/usr/local \
             /usr \
             /"])

  for li in $lcc_dir; do
      if test -x "$li/bin/gwenhywfar-config"; then
          gwenhywfar_dir="$li"
      fi
  done
  if test -z "$gwenhywfar_dir"; then
      AC_MSG_RESULT([not found ])
  else
      AC_MSG_RESULT($gwenhywfar_dir)
      AC_MSG_CHECKING(for gwen libs)
      gwenhywfar_libs="`$gwenhywfar_dir/bin/gwenhywfar-config --libraries`"
      AC_MSG_RESULT($gwenhywfar_libs)
      AC_MSG_CHECKING(for gwen includes)
      gwenhywfar_includes="`$gwenhywfar_dir/bin/gwenhywfar-config --includes`"
      AC_MSG_RESULT($gwenhywfar_includes)
  fi
  AC_MSG_CHECKING(if gwenhywfar test desired)
  AC_ARG_ENABLE(gwenhywfar,
    [  --enable-gwenhywfar-test   enable gwenhywfar-test (default=yes)],
     enable_gwenhywfar_test="$enableval",
     enable_gwenhywfar_test="yes")
  AC_MSG_RESULT($enable_gwenhywfar_test)
  AC_MSG_CHECKING(for Gwenhywfar version >=$vma.$vmi.$vpl.$vbld)
  if test "$enable_gwenhywfar_test" != "no"; then
    if test "$vma" -gt "`$gwenhywfar_dir/bin/gwenhywfar-config --vmajor`"; then
      AC_MSG_ERROR([Your Gwenhywfar version is way too old.
      Please update from www.aquamaniac.de])
    elif test "$vma" = "`$gwenhywfar_dir/bin/gwenhywfar-config --vmajor`"; then
      if test "$vmi" -gt "`$gwenhywfar_dir/bin/gwenhywfar-config --vminor`"; then
        AC_MSG_ERROR([Your Gwenhywfar version is too old.
          Please update from www.aquamaniac.de])
      elif test "$vmi" = "`$gwenhywfar_dir/bin/gwenhywfar-config --vminor`"; then
          if test "$vpl" -gt "`$gwenhywfar_dir/bin/gwenhywfar-config --vpatchlevel`"; then
            AC_MSG_ERROR([Your Gwenhywfar version is a little bit too old.
            Please update from www.aquamaniac.de])
          elif test "$vpl" = "`$gwenhywfar_dir/bin/gwenhywfar-config --vpatchlevel`"; then
            if test "$vbld" -gt "`$gwenhywfar_dir/bin/gwenhywfar-config --vbuild`"; then
              AC_MSG_ERROR([Your Gwenhywfar version is a little bit too old.
              Please update from www.aquamaniac.de])
             fi
           fi
      fi
    fi
    have_gwenhywfar="yes"
    AC_MSG_RESULT(yes)
  else
    have_gwenhywfar="yes"
    AC_MSG_RESULT(assuming yes)
  fi
dnl end of "if enable-gwenhywfar"
fi

AC_SUBST(gwenhywfar_dir)
AC_SUBST(gwenhywfar_libs)
AC_SUBST(gwenhywfar_includes)
])
