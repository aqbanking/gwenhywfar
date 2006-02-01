# $Id$
# (c) 2002 Martin Preuss<martin@libchipcard.de>
# This function checks if SSL is wanted and locates it

AC_DEFUN([AQ_CHECK_SSL],[
dnl PREREQUISITES:
dnl   - AQ_CHECK_OS must be called before
dnl   - For windows, AQ_WINDOZE must be called beforehand
dnl IN: 
dnl   nothing
dnl OUT:
dnl   Variables:
dnl     ssl_libraries: Path to the SSL libraries (subst)
dnl     ssl_lib: SSL libraries to link against (subst)
dnl     ssl_includes: Path to the SSL includes (subst)
dnl     ssl_available: "yes" if OpenSSL is available
dnl   Defines:
AC_REQUIRE([AQ_CHECK_OS])

dnl check if ssl is desired
AC_MSG_CHECKING(if OpenSSL should be used)
AC_ARG_ENABLE(ssl,
  [  --enable-ssl             enable SSL features (default=yes)],
  enable_ssl="$enableval",
  enable_ssl="yes")
AC_MSG_RESULT($enable_ssl)

if test "$enable_ssl" != "no"; then

dnl ******* openssl includes ***********
AC_MSG_CHECKING(for openssl includes)
AC_ARG_WITH(openssl-includes, [  --with-openssl-includes=DIR specify include directory for openssl includes],
  [ssl_search_inc_dirs="$withval"],
  [ssl_search_inc_dirs="/usr/include\
    		       /usr/local/include\
		       /usr/local/ssl/include\
  		       /usr/ssl/include\
                       /more/include\
                       /sw/include"\
                       ])

dnl search for ssl
AQ_SEARCH_FOR_PATH([openssl/des.h],[$ssl_search_inc_dirs])
if test -z "$found_dir" ; then
    AC_MSG_RESULT(not found)
else
  ssl_includes="-I$found_dir"
  AC_MSG_RESULT($ssl_includes)
fi


dnl ******* openssl lib ***********
AC_MSG_CHECKING(for openssl libs)
AC_ARG_WITH(openssl-libs, [  --with-openssl-libs=DIR    specify lib directory for openssl libraries],
  [ssl_search_lib_dirs="$withval"],
  [ssl_search_lib_dirs="/usr/lib \
		       /usr/local/lib \
		       /usr/lib/ssl/lib \
		       /usr/lib/openssl/lib \
		       /usr/local/ssl/lib \
		       /usr/local/openssl/lib \
                       /sw/lib \
		       /lib"])

dnl search for openssl libs
if test "$OSYSTEM" != "windows" ; then
dnl POSIX systems
   for d in $ssl_search_lib_dirs; do
     # Look for the library files; the dylib one is for Darwin/Mac OS.
     AQ_SEARCH_FILES("$d", [libssl.so libssl.so.* libssl.a libssl.dylib])
     if test -n "$found_file" ; then
        ssl_libraries="-L$d"
        ssl_lib="-lssl -lcrypto"
        break
     fi
   done
   AC_MSG_RESULT($ssl_libraries ${ssl_lib})
else
dnl Windoze systems
   if test -z "$WIN_PATH_WINDOWS_MINGW"; then
     AC_ERROR([Error in configure.ac: The macro aq_windoze did not set a windows system path -- maybe this macro has not yet been called.])
   fi
   # Check for the directory of the installed OpenSSL DLLs
   for d in "$WIN_PATH_WINDOWS_MINGW" "$WIN_PATH_SYSTEM_MINGW"; do
      AQ_SEARCH_FILES("$d", "libssl32.dll")
      if test -n "$found_file"; then
         ssl_libraries="-L$d"
         break
      fi
   done
   ssl_lib="-leay32 -lssl32"
   AC_MSG_RESULT($ssl_libraries ${ssl_lib})
fi

AC_MSG_CHECKING(whether openssl is usable)
if test -z "$ssl_libraries" -o -z "$ssl_lib" -o -z "$ssl_includes"; then
    ssl_available="no"
    AC_MSG_ERROR(ssl libraries not found.)
else
    ssl_available="yes"
    AC_MSG_RESULT(yes)
fi


# end of "if enable-ssl"
fi
AC_SUBST(ssl_includes)
AC_SUBST(ssl_libraries)
AC_SUBST(ssl_lib)
])

