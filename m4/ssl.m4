# $Id$
# (c) 2002 Martin Preuss<martin@libchipcard.de>
# This function checks if SSL is wanted and locates it

AC_DEFUN(AQ_CHECK_SSL,[
dnl PREREQUISITES:
dnl   - AQ_CHECK_OS must becalled before
dnl IN: 
dnl   nothing
dnl OUT:
dnl   Variables:
dnl     ssl_libraries: Path to the SSL libraries (subst)
dnl     ssl_lib: SSL libraries to link against (subst)
dnl     ssl_includes: Path to the SSL includes (subst)
dnl     ssl_available: "yes" if OpenSSL is available
dnl   Defines:

dnl check if ssl is desired
AC_MSG_CHECKING(if OpenSSL should be used)
AC_ARG_ENABLE(ssl,
  [  --enable-ssl             enable encryption (default=yes)],
  enable_ssl="$enableval",
  enable_ssl="yes")
AC_MSG_RESULT($enable_ssl)

if test "$enable_ssl" != "no"; then

dnl ******* openssl includes ***********
AC_MSG_CHECKING(for openssl includes)
AC_ARG_WITH(openssl-includes, [  --with-openssl-includes=DIR adds openssl include path],
  [ssl_search_inc_dirs="$withval"],
  [ssl_search_inc_dirs="/usr/include\
    		       /usr/local/include\
		       /usr/local/ssl/include\
  		       /usr/ssl/include"])

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
AC_ARG_WITH(openssl-libname, [  --with-openssl-libname=NAME  specify the name of the openssl library],
  [ssl_search_lib_names="$withval"],
  [ssl_search_lib_names="libcrypto.so \
	                 libcrypto.so.* \
	                 libcrypto.a"])

AC_ARG_WITH(openssl-libs, [  --with-openssl-libs=DIR  adds openssl library path],
  [ssl_search_lib_dirs="$withval"],
  [ssl_search_lib_dirs="/usr/lib \
		       /usr/local/lib \
		       /usr/lib/ssl/lib \
		       /usr/lib/openssl/lib \
		       /usr/local/ssl/lib \
		       /usr/local/openssl/lib \
		       /lib"])

dnl search for openssl libs
if test "$OSYSTEM" != "windows" ; then
   for d in $ssl_search_lib_dirs; do
     AQ_SEARCH_FILES("$d",$ssl_search_lib_names)
     if test -n "$found_file" ; then
        ssl_libraries="-L$d"
        ssl_lib="-l`echo $found_file | sed 's/lib//;s/\.so*//;s/\.a//'`"
        AC_MSG_RESULT($ssl_libraries ${ssl_lib})
        break
     fi
   done
   else
     ssl_libraries="-L/c/windows"
     ssl_lib="-llibeay32 -llibssl32"
     AC_MSG_RESULT($ssl_libraries ${ssl_lib})
fi

AC_MSG_CHECKING(whether openssl is usable)
if test -z "$ssl_libraries" -o -z "$ssl_lib" -o -z "$ssl_includes"; then
    ssl_available="no"
    AC_MSG_WARN(ssl libraries not found.)
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

