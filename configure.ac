#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# info
GWENHYFWAR_AUTHOR="Martin Preuss<martin@libchipcard.de>"
GWENHYFWAR_BUGREPORT_ADDR="<gwenhyfwar-bugs@libchipcard.de>"


AC_PREREQ(2.57)
AC_INIT
AC_CANONICAL_TARGET([])
AM_INIT_AUTOMAKE(Gwenhyfwar, 0.9beta3)
AC_CONFIG_SRCDIR([src/gwenhyfwarapi.h])
AC_CONFIG_HEADER([config.h])



#
# versions
#
GWENHYFWAR_VERSION_MAJOR=0
GWENHYFWAR_VERSION_MINOR=9
GWENHYFWAR_VERSION_PATCHLEVEL=1
GWENHYFWAR_VERSION_BUILD=6
GWENHYFWAR_VERSION_TAG="cvs"
GWENHYFWAR_VERSION_FULL_STRING="$GWENHYFWAR_VERSION_MAJOR.$GWENHYFWAR_VERSION_MINOR.$GWENHYFWAR_VERSION_PATCHLEVEL.$GWENHYFWAR_VERSION_BUILD($GWENHYFWAR_VERSION_TAG)"
GWENHYFWAR_VERSION_STRING="$GWENHYFWAR_VERSION_MAJOR.$GWENHYFWAR_VERSION_MINOR.$GWENHYFWAR_VERSION_PATCHLEVEL($GWENHYFWAR_VERSION_TAG)"

AC_SUBST(GWENHYFWAR_VERSION_MAJOR)
AC_SUBST(GWENHYFWAR_VERSION_MINOR)
AC_SUBST(GWENHYFWAR_VERSION_PATCHLEVEL)
AC_SUBST(GWENHYFWAR_VERSION_BUILD)
AC_SUBST(GWENHYFWAR_VERSION_TAG)
AC_SUBST(GWENHYFWAR_VERSION_FULL_STRING)
AC_SUBST(GWENHYFWAR_VERSION_STRING)

AC_DEFINE_UNQUOTED(GWENHYFWAR_VERSION_MAJOR,$GWENHYFWAR_VERSION_MAJOR, 
                   [major version])
AC_DEFINE_UNQUOTED(GWENHYFWAR_VERSION_MINOR,$GWENHYFWAR_VERSION_MINOR, 
                   [minor version])
AC_DEFINE_UNQUOTED(GWENHYFWAR_VERSION_PATCHLEVEL,$GWENHYFWAR_VERSION_PATCHLEVEL, 
                   [patchlevel])
AC_DEFINE_UNQUOTED(GWENHYFWAR_VERSION_BUILD,$GWENHYFWAR_VERSION_BUILD, [build])
AC_DEFINE_UNQUOTED(GWENHYFWAR_VERSION_STRING,"$GWENHYFWAR_VERSION_STRING", 
                   [version string])
AC_DEFINE_UNQUOTED(GWENHYFWAR_VERSION_FULL_STRING,"$GWENHYFWAR_VERSION_FULL_STRING",
                   [full version string])
AC_DEFINE_UNQUOTED(GWENHYFWAR_VERSION_TAG,$GWENHYFWAR_VERSION_TAG, [tag])

#
# SO version
#
GWENHYFWAR_SO_CURRENT=2
GWENHYFWAR_SO_REVISION=0
GWENHYFWAR_SO_AGE=0
AC_SUBST(GWENHYFWAR_SO_CURRENT)
AC_SUBST(GWENHYFWAR_SO_REVISION)
AC_SUBST(GWENHYFWAR_SO_AGE)



# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PROG_AWK


# Checks for libraries.
AC_CHECK_LIB(dl,dlopen)


# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h])
AC_CHECK_HEADERS([stdlib.h string.h sys/socket.h sys/time.h])
AC_CHECK_HEADERS([syslog.h unistd.h])
AC_CHECK_HEADERS([unistd.h])
AC_CHECK_HEADERS([dlfcn.h])
AC_CHECK_HEADERS([signal.h])
AC_CHECK_HEADERS([time.h])
AC_CHECK_HEADERS([syslog.h])
AC_CHECK_HEADERS([sys/stat.h])
AC_CHECK_HEADERS([sys/types.h])
AC_CHECK_HEADERS([sys/wait.h])


# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_HEADER_TIME
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_SELECT_ARGTYPES
AC_CHECK_FUNCS([gethostbyaddr gethostbyname inet_ntoa isascii])
AC_CHECK_FUNCS([memmove memset])
AC_CHECK_FUNCS([select socket])
AC_CHECK_FUNCS([strcasecmp strchr strdup strerror])
AC_CHECK_FUNCS([snprintf])


dnl prefix handling
AC_PREFIX_DEFAULT(/usr)
if test "x$prefix" = "xNONE"; then
  prefix=$ac_default_prefix
  ac_configure_args="$ac_configure_args --prefix $prefix"
fi
AC_SUBST(prefix)


dnl OS dependant settings
AQ_CHECK_OS

case "$OS_TYPE" in
    posix)
        gwenhyfwar_libs="-L`eval echo $prefix` -lgwenhyfwar"
        gwenhyfwar_includes="-I`eval echo $prefix/include`"
        gwenhyfwar_os_lib="os/posix/libos.la"
        GWEN_OS="posix"
    	;;
    windows)
        AC_DEFINE_UNQUOTED(BUILDING_CHAMELEON_DLL,1, [Define if DLL is built])
        gwenhyfwar_libs=-L`eval echo $prefix` -lgwenhyfwar
        gwenhyfwar_includes="-I`eval echo $prefix/include`"
        gwenhyfwar_os_lib="os/windows/libos.la"
        GWEN_OS="posix"
    	;;
esac


#
# check for encryption library
#
AQ_CHECK_SSL
if test "$ssl_available" = "yes"; then
  all_libraries="${all_libraries} ${ssl_libraries} ${ssl_lib}"
  all_includes="$all_includes $ssl_includes"
  gwenhyfwar_includes="$gwenhyfwar_includes $ssl_includes"
  gwenhyfwar_crypt_lib="crypt/ossl/libcrypt.la"
  GWEN_CRYPTO="ossl"
fi



#
# create header directory, make symlinks
#
GWEN_HEADER_DIRS="src src/base src/crypt src/crypt/${GWEN_CRYPTO} src/io src/os src/os/${GWEN_OS} src/ipc src/parser src/impl src/impl/ipc"
lprefix="${srcdir}"
lprefix=`( cd ${lprefix} ; pwd )`

rm -Rf gwenhyfwar
mkdir gwenhyfwar
GWEN_HEADER_FILES=""
for d in ${GWEN_HEADER_DIRS}; do
  hfiles=`( cd "${srcdir}/${d}" && ls *.h 2>/dev/null )`
  for f in ${hfiles}; do
    GWEN_HEADER_FILES="${GWEN_HEADER_FILES} ${lprefix}/${d}/${f}"
    case ${f} in
      *_p.h)
      		;;
      *)
    		ln -s "${lprefix}/${d}/${f}" "gwenhyfwar/${f}"
                ;;
    esac
  done
done
ln -s "${lprefix}/src/version.h" "gwenhyfwar/version.h"

#
# substitute vars
#
AS_SCRUB_INCLUDE(gwenhyfwar_includes)
AC_SUBST(gwenhyfwar_includes)
AC_SUBST(gwenhyfwar_libs)
AC_SUBST(gwenhyfwar_needed_libs)
AC_SUBST(GWEN_OS)
AC_SUBST(gwenhyfwar_os_lib)
AC_SUBST(GWENHYFWAR_LIB)
AC_SUBST(gwenhyfwar_crypt_lib)
AC_SUBST(GWEN_CRYPTO)
AC_SUBST(GWEN_HEADER_FILES)
AC_SUBST(all_includes)
AC_SUBST(all_libraries)


#
# list of files to create
#
AC_CONFIG_FILES([Makefile
                 m4/Makefile
                 gwenhywfar-config.in
                 src/Makefile
                 src/version.h
                 src/base/Makefile
                 src/crypt/Makefile
                 src/crypt/ossl/Makefile
                 src/io/Makefile
                 src/ipc/Makefile
                 src/os/Makefile
                 src/os/posix/Makefile
                 src/parser/Makefile
                 src/impl/Makefile
                 src/impl/ipc/Makefile
                 test/Makefile])
AC_OUTPUT
