# $Id$
# (c) 2002 Martin Preuss<martin@libchipcard.de>
# These functions search for QT 2-3


AC_DEFUN(AQ_CHECK_QT,[
dnl $1 = operating system name ("linux", "freebsd", "windows")
dnl $2 = subdirs to include when QT is available
dnl searches a dir for some files
dnl You may preset the return variables.
dnl All variables which already have a value will not be altered
dnl returns some variables:
dnl  qt_generation either 1,2 or 3
dnl  qt_includes path to includes
dnl  qt_libs path to libraries
dnl  qt_moc path to moc
dnl  qt_uic path to uic

ops="$1"
lsd="$2"

dnl check if qt apps are desired
AC_MSG_CHECKING(if QT applications should be compiled)
AC_ARG_ENABLE(qtapps,
  [  --enable-qtapps         enable compilation of qt applications (default=yes)],
  enable_qtapps="$enableval",
  enable_qtapps="yes")
AC_MSG_RESULT($enable_qtapps)

if test "$enable_qtapps" = "no"; then
   qt_libs=""
   qt_includes=""
   qt_app=""
   qt_moc=""
   qt_uic=""
else

dnl paths for qt
AC_ARG_WITH(qt-dir, 
  [  --with-qt-dir=DIR      uses qt from given dir],
  [local_qt_paths="$withval"],
  [local_qt_paths="\
  	$QTDIR \
      	/usr/local/lib/qt3 \
        /usr/lib/qt3 \
        /lib/qt3 \
        /usr/local/lib/qt2 \
        /usr/lib/qt2 \
        /lib/qt2 \
        /usr/local/lib/qt1 \
        /usr/lib/qt1 \
        /lib/qt1 \
        /usr/local/lib/qt \
        /usr/lib/qt \
        /lib/qt \
        /usr/local \
        /usr \
        /usr/X11R6 \
        / \
        "])

AC_MSG_CHECKING(if threaded qt may be used)
AC_ARG_ENABLE(qt-threads,
  [  --enable-qt-threads         enable qt-mt library (default=yes)],
  enable_qt_threads="$enableval",
  enable_qt_threads="yes")
AC_MSG_RESULT($enable_qt_threads)

dnl check for library
AC_MSG_CHECKING(for qt libraries)
dnl check for 3
if test -z "$qt_libs" && test "$enable_qt_threads" != "no"; then
	AQ_SEARCH_FOR_PATH([lib/libqt-mt.so.3],[$local_qt_paths])
	if test -n "$found_dir" ; then
        	qt_dir="$found_dir"
    		qt_libs="-L$found_dir/lib -lqt-mt"
                if test "$ops" = "freebsd"; then
                	qt_libs="$qt_libs -lc_r"
                fi
                qt_generation="3"
	fi
fi
if test -z "$qt_libs"; then
	AQ_SEARCH_FOR_PATH([lib/libqt.so.3],[$local_qt_paths])
	if test -n "$found_dir" ; then
        	qt_dir="$found_dir"
    		qt_libs="-L$found_dir/lib -lqt"
                qt_generation="3"
	fi
fi

dnl check for 2
if test -z "$qt_libs" && test "$enable_qt_threads" != "no"; then
	AQ_SEARCH_FOR_PATH([lib/libqt-mt.so.2],[$local_qt_paths])
	if test -n "$found_dir" ; then
        	qt_dir="$found_dir"
    		qt_libs="-L$found_dir/lib -lqt-mt"
                qt_generation="2"
	fi
fi
if test -z "$qt_libs"; then
	AQ_SEARCH_FOR_PATH([lib/libqt.so.2],[$local_qt_paths])
	if test -n "$found_dir" ; then
        	qt_dir="$found_dir"
    		qt_libs="-L$found_dir/lib -lqt"
                qt_generation="2"
	fi
fi

dnl check for 1
if test -z "$qt_libs" && test "$enable_qt_threads" != "no"; then
	AQ_SEARCH_FOR_PATH([lib/libqt-mt.so.1],[$local_qt_paths])
	if test -n "$found_dir" ; then
        	qt_dir="$found_dir"
    		qt_libs="-L$found_dir/lib -lqt-mt"
                qt_generation="1"
	fi
fi
if test -z "$qt_libs"; then
	AQ_SEARCH_FOR_PATH([lib/libqt.so.1],[$local_qt_paths])
	if test -n "$found_dir" ; then
        	qt_dir="$found_dir"
    		qt_libs="-L$found_dir/lib -lqt"
                qt_generation="1"
	fi
fi
if test -z "$qt_libs"; then
	AC_MSG_RESULT(not found)
else
	AC_MSG_RESULT($qt_libs)
fi

dnl check for includes
AC_MSG_CHECKING(for qt includes)
if test -z "$qt_includes"; then
       	AQ_SEARCH_FOR_PATH([include/qt.h],[$qt_dir $local_qt_paths])
       	if test -n "$found_dir" ; then
       		qt_includes="-I$found_dir/include"
       	fi
fi
if test -z "$qt_includes"; then
       	AQ_SEARCH_FOR_PATH([include/qt3/qt.h],[$qt_dir $local_qt_paths])
       	if test -n "$found_dir" ; then
       		qt_includes="-I$found_dir/include/qt3"
       	fi
fi
if test -z "$qt_includes"; then
       	AQ_SEARCH_FOR_PATH([include/qt2/qt.h],[$qt_dir $local_qt_paths])
       	if test -n "$found_dir" ; then
       		qt_includes="-I$found_dir/include/qt2"
       	fi
fi
if test -z "$qt_includes"; then
       	AQ_SEARCH_FOR_PATH([include/qt1/qt.h],[$qt_dir $local_qt_paths])
       	if test -n "$found_dir" ; then
       		qt_includes="-I$found_dir/include/qt1"
       	fi
fi
if test -z "$qt_includes"; then
       	AQ_SEARCH_FOR_PATH([include/qt/qt.h],[$qt_dir $local_qt_paths])
       	if test -n "$found_dir" ; then
       		qt_includes="-I$found_dir/include/qt"
       	fi
fi
if test -z "$qt_includes"; then
	AC_MSG_RESULT(not found)
else
	AC_MSG_RESULT($qt_includes)
fi

dnl check for moc
AC_MSG_CHECKING(for qt moc)
if test -z "$qt_moc"; then
       	AQ_SEARCH_FOR_PATH([bin/moc],[$qt_dir $local_qt_paths])
       	if test -n "$found_dir" ; then
       		qt_moc="$found_dir/bin/moc"
       	fi
fi
if test -z "$qt_moc"; then
       	AQ_SEARCH_FOR_PATH([bin/moc2],[$qt_dir $local_qt_paths])
       	if test -n "$found_dir" ; then
       		qt_moc="$found_dir/bin/moc2"
       	fi
fi
if test -z "$qt_moc"; then
	AC_MSG_RESULT(not found)
else
	AC_MSG_RESULT($qt_moc)
fi

dnl check for uic
AC_MSG_CHECKING(for qt uic)
if test -z "$qt_uic"; then
       	AQ_SEARCH_FOR_PATH([bin/uic],[$qt_dir $local_qt_paths])
       	if test -n "$found_dir" ; then
       		qt_uic="$found_dir/bin/uic"
       	fi
fi
if test -z "$qt_uic"; then
	AC_MSG_RESULT(not found)
else
	AC_MSG_RESULT($qt_uic)
fi

# check if all necessary qt components where found
if test -n "$qt_includes" && \
   test -n "$qt_libs" && \
   test -n "$qt_moc" && \
   test -n "$qt_uic"; then
   qt_app="$lsd"
   AC_DEFINE_UNQUOTED(QT_GENERATION,$qt_generation, [QT generation])
else
   qt_libs=""
   qt_includes=""
   qt_app=""
   qt_moc=""
   qt_uic=""
   if test "$enable_qtapps" = "yes"; then
        AC_MSG_ERROR([
 Compilation of QT applications is enabled but I could not find some QT
 components (see which are missing in messages above).
 If you don't want to compile QT applications please use "--disable-qtapps".
 ])
   fi
fi

dnl end of if "$enable_qtapps"
fi

AC_SUBST(qt_dir)
AC_SUBST(qt_app)
AC_SUBST(qt_libs)
AC_SUBST(qt_includes)
AC_SUBST(qt_moc)
AC_SUBST(qt_uic)
AC_SUBST(qt_generation)
])

