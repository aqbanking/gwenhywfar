# $Id$
# (c) 2002 Martin Preuss<martin@libchipcard.de>
# These functions search for KDE 2-3


AC_DEFUN([AQ_CHECK_KDE],[
dnl $1 = operating system name ("linux", "freebsd", "windows")
dnl $2 = subdirs to include when KDE is available
dnl searches a dir for some files
dnl You may preset the return variables.
dnl All variables which already have a value will not be altered
dnl returns some variables:
dnl  kde_generation either 1,2 or 3
dnl  kde_includes path to includes
dnl  kde_libs path to libraries
dnl  kde_install_dir

ops="$1"
lsd="$2"

dnl check if kde apps are desired
AC_MSG_CHECKING(if KDE applications should be compiled)
AC_ARG_ENABLE(kdeapps,
  [  --enable-kdeapps         enable compilation of kde applications (default=yes)],
  enable_kdeapps="$enableval",
  enable_kdeapps="yes")
AC_MSG_RESULT($enable_kdeapps)

if test "$enable_kdeapps" = "no"; then
   kde_libs=""
   kde_includes=""
   kde_app=""
else

dnl paths for kde
AC_ARG_WITH(kde-dir, 
  [  --with-kde-dir=DIR      uses kde from given dir],
  [local_kde_paths="$withval"],
  [local_kde_paths="\
  	$KDEDIR \
        /opt/kde3 \
      	/usr/local/lib/kde3 \
        /usr/lib/kde3 \
        /lib/kde3 \
        /opt/kde2 \
        /usr/local/lib/kde2 \
        /usr/lib/kde2 \
        /lib/kde2 \
        /opt/kde1 \
        /usr/local/lib/kde1 \
        /usr/lib/kde1 \
        /lib/kde1 \
        /opt/kde \
        /usr/local/lib/kde \
        /usr/lib/kde \
        /lib/kde \
        /usr/local \
        /usr \
        / \
        "])

dnl check for library
AC_MSG_CHECKING(for kde libraries)
dnl check for 3
if test -z "$kde_libs" ; then
	AQ_SEARCH_FOR_PATH([lib/libkdeui.so.4],[$local_kde_paths])
	if test -n "$found_dir" ; then
        	kde_dir="$found_dir"
    		kde_libs="-L$found_dir/lib"
                kde_generation="3"
	fi
fi

dnl check for 2
if test -z "$kde_libs"; then
	AQ_SEARCH_FOR_PATH([lib/libkdeui.so.3],[$local_kde_paths])
	if test -n "$found_dir" ; then
        	kde_dir="$found_dir"
    		kde_libs="-L$found_dir/lib"
                kde_generation="2"
	fi
fi

dnl check for 1
if test -z "$kde_libs"; then
	AQ_SEARCH_FOR_PATH([lib/libkdeui.so],[$local_kde_paths])
	if test -n "$found_dir" ; then
        	kde_dir="$found_dir"
    		kde_libs="-L$found_dir/lib"
                kde_generation="1"
	fi
fi
if test -z "$kde_libs"; then
	AC_MSG_RESULT(not found)
else
	AC_MSG_RESULT($kde_libs)
fi

dnl check for includes
AC_MSG_CHECKING(for kde includes)
if test -z "$kde_includes"; then
       	AQ_SEARCH_FOR_PATH([include/kpushbutton.h],[$kde_dir $local_kde_paths])
       	if test -n "$found_dir" ; then
       		kde_includes="-I$found_dir/include"
       	fi
fi
if test -z "$kde_includes"; then
       	AQ_SEARCH_FOR_PATH([include/kde3/kpushbutton.h],[$kde_dir $local_kde_paths])
       	if test -n "$found_dir" ; then
       		kde_includes="-I$found_dir/include/kde3"
       	fi
fi
if test -z "$kde_includes"; then
       	AQ_SEARCH_FOR_PATH([include/kde2/kpushbutton.h],[$kde_dir $local_kde_paths])
       	if test -n "$found_dir" ; then
       		kde_includes="-I$found_dir/include/kde2"
       	fi
fi
if test -z "$kde_includes"; then
       	AQ_SEARCH_FOR_PATH([include/kde1/kpushbutton.h],[$kde_dir $local_kde_paths])
       	if test -n "$found_dir" ; then
       		kde_includes="-I$found_dir/include/kde1"
       	fi
fi
if test -z "$kde_includes"; then
       	AQ_SEARCH_FOR_PATH([include/kde/kpushbutton.h],[$kde_dir $local_kde_paths])
       	if test -n "$found_dir" ; then
       		kde_includes="-I$found_dir/include/kde"
       	fi
fi
if test -z "$kde_includes"; then
	AC_MSG_RESULT(not found)
else
	AC_MSG_RESULT($kde_includes)
fi


# check if all necessary kde components where found
if test -n "$kde_includes" && \
   test -n "$kde_libs"; then
   kde_app="$lsd"
   AC_DEFINE_UNQUOTED(KDE_GENERATION,$kde_generation,[KDE generation])
   AC_MSG_CHECKING(for kde install prefix)
   AC_ARG_WITH(kde-prefix, 
  	[  --with-kde-prefix=DIR      install kde apps to prefix],
  	[kde_install_dir="$withval"],
  	[kde_install_dir="$kde_dir"])
   AC_MSG_RESULT($kde_install_dir)
else
   kde_libs=""
   kde_includes=""
   kde_app=""
   if test "$enable_kdeapps" = "yes"; then
        AC_MSG_ERROR([
 Compilation of KDE applications is enabled but I could not find some KDE
 components (see which are missing in messages above).
 If you don't want to compile KDE applications please use "--disable-kdeapps".
 ])
   fi
fi

dnl end of if "$enable_kdeapps"
fi

AC_SUBST(kde_dir)
AC_SUBST(kde_app)
AC_SUBST(kde_libs)
AC_SUBST(kde_includes)
AC_SUBST(kde_generation)
AC_SUBST(kde_install_dir)
])
