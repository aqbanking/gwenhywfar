#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# info
GWENHYWFAR_AUTHOR="Martin Preuss<martin@libchipcard.de>"
GWENHYWFAR_BUGREPORT_ADDR="<gwenhywfar-bugs@libchipcard.de>"


AC_PREREQ(2.57)
AC_INIT
AC_CANONICAL_TARGET([])
AM_INIT_AUTOMAKE(gwenhywfar, 0.9beta9)
AC_CONFIG_SRCDIR([src/gwenhywfarapi.h])
AC_CONFIG_HEADER([config.h])


#
# versions
#
GWENHYWFAR_VERSION_MAJOR=0
GWENHYWFAR_VERSION_MINOR=9
GWENHYWFAR_VERSION_PATCHLEVEL=4
GWENHYWFAR_VERSION_BUILD=10
GWENHYWFAR_VERSION_TAG="cvs"
GWENHYWFAR_VERSION_FULL_STRING="$GWENHYWFAR_VERSION_MAJOR.$GWENHYWFAR_VERSION_MINOR.$GWENHYWFAR_VERSION_PATCHLEVEL.$GWENHYWFAR_VERSION_BUILD($GWENHYWFAR_VERSION_TAG)"
GWENHYWFAR_VERSION_STRING="$GWENHYWFAR_VERSION_MAJOR.$GWENHYWFAR_VERSION_MINOR.$GWENHYWFAR_VERSION_PATCHLEVEL($GWENHYWFAR_VERSION_TAG)"
rpm_version="$VERSION"
rpm_release="1"

AC_SUBST(GWENHYWFAR_VERSION_MAJOR)
AC_SUBST(GWENHYWFAR_VERSION_MINOR)
AC_SUBST(GWENHYWFAR_VERSION_PATCHLEVEL)
AC_SUBST(GWENHYWFAR_VERSION_BUILD)
AC_SUBST(GWENHYWFAR_VERSION_TAG)
AC_SUBST(GWENHYWFAR_VERSION_FULL_STRING)
AC_SUBST(GWENHYWFAR_VERSION_STRING)
AC_SUBST(rpm_version)
AC_SUBST(rpm_release)

AC_DEFINE_UNQUOTED(GWENHYWFAR_VERSION_MAJOR,$GWENHYWFAR_VERSION_MAJOR, 
                   [major version])
AC_DEFINE_UNQUOTED(GWENHYWFAR_VERSION_MINOR,$GWENHYWFAR_VERSION_MINOR, 
                   [minor version])
AC_DEFINE_UNQUOTED(GWENHYWFAR_VERSION_PATCHLEVEL,$GWENHYWFAR_VERSION_PATCHLEVEL, 
                   [patchlevel])
AC_DEFINE_UNQUOTED(GWENHYWFAR_VERSION_BUILD,$GWENHYWFAR_VERSION_BUILD, [build])
AC_DEFINE_UNQUOTED(GWENHYWFAR_VERSION_STRING,"$GWENHYWFAR_VERSION_STRING", 
                   [version string])
AC_DEFINE_UNQUOTED(GWENHYWFAR_VERSION_FULL_STRING,"$GWENHYWFAR_VERSION_FULL_STRING",
                   [full version string])
AC_DEFINE_UNQUOTED(GWENHYWFAR_VERSION_TAG,$GWENHYWFAR_VERSION_TAG, [tag])

#
# SO version
#
GWENHYWFAR_SO_CURRENT=3
GWENHYWFAR_SO_REVISION=2
GWENHYWFAR_SO_AGE=1
AC_SUBST(GWENHYWFAR_SO_CURRENT)
AC_SUBST(GWENHYWFAR_SO_REVISION)
AC_SUBST(GWENHYWFAR_SO_AGE)



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
        gwenhywfar_libs="-L`eval echo $prefix/lib` -lgwenhywfar"
        gwenhywfar_includes="-I`eval echo $prefix/include`"
        gwenhywfar_os_lib="os/posix/libos.la"
        GWEN_OS="posix"
    	;;
    windows)
        AC_DEFINE_UNQUOTED(BUILDING_GWEN_DLL,1, [Define if DLL is built])
        gwenhywfar_libs="-L/c/windows -lgwenhywfar32"
        gwenhywfar_includes="-I`eval echo $prefix/include`"
        gwenhywfar_os_lib="os/windows/libos.la"
        GWEN_MAKE_DLL_TARGET="dll"
        GWEN_INSTALL_DLL_TARGET="dll-install"
        GWEN_OS="windows"
    	;;
esac


#
# check for encryption library
#
AQ_CHECK_SSL
if test "$ssl_available" = "yes"; then
  all_libraries="${all_libraries} ${ssl_libraries} ${ssl_lib}"
  all_includes="$all_includes $ssl_includes"
  gwenhywfar_includes="$gwenhywfar_includes $ssl_includes"
  GWEN_CRYPTO="ossl"
else
  GWEN_CRYPTO="none"
fi



#
# create header directory, make symlinks
#
GWEN_HEADER_DIRS="src src/base src/crypt src/crypt/${GWEN_CRYPTO} src/io src/os src/os/${GWEN_OS} src/ipc src/parser"
lprefix="${srcdir}"
lprefix=`( cd ${lprefix} ; pwd )`

rm -Rf gwenhywfar
mkdir gwenhywfar
GWEN_HEADER_FILES=""
for d in ${GWEN_HEADER_DIRS}; do
  hfiles=`( cd "${srcdir}/${d}" && ls *.h 2>/dev/null )`
  for f in ${hfiles}; do
    GWEN_HEADER_FILES="${GWEN_HEADER_FILES} ${lprefix}/${d}/${f}"
    case ${f} in
      *_p.h)
      		;;
      *)
    		ln -s "${lprefix}/${d}/${f}" "gwenhywfar/${f}"
                ;;
    esac
  done
done
ln -s "${lprefix}/src/version.h" "gwenhywfar/version.h" &>/dev/null


#
# check docpath
#
AC_MSG_CHECKING(docpath)
AC_ARG_WITH(docpath, [  --with-docpath=DIR where to store the apidoc],
  [docpath="$withval"],
  [docpath="apidoc"])
AC_SUBST(docpath)
AC_MSG_RESULT($docpath)


#
# check for doc type
#
AC_MSG_CHECKING(if full docu should be created)
AC_ARG_ENABLE(full-doc,
  [  --enable-full-doc         enable creating full apidoc (default=no)],
  [ case "${enableval}" in
    yes) enable_fulldoc="yes";;
    no)  enable_fulldoc="no";;
    *) AC_MSG_ERROR(bad value ${enableval} for --enable-full-doc);;
    esac
  ],
  enable_fulldoc="no")

if test "$enable_fulldoc" = "yes"; then
	DOXYGEN_INPUT="src"
        DOXYGEN_DEFINE=""
else
	DOXYGEN_DEFINE="DOXYGEN_HIDE"
	DOXYGEN_INPUT="doc/apidoc.h gwenhywfar"
fi
AC_SUBST(DOXYGEN_INPUT)
AC_SUBST(DOXYGEN_DEFINE)
AC_MSG_RESULT($enable_fulldoc)



#
# check for MEMTRACE
#
AC_MSG_CHECKING(if MEMTRACE is wanted)
AC_ARG_ENABLE(memtrace,
  [  --enable-memtrace             enable memory trace (default=no)],
  enable_memtrace="$enableval",
  enable_memtrace="no")
AC_MSG_RESULT($enable_memtrace)

if test "$enable_memtrace" != "no"; then
  AC_DEFINE(GWEN_MEMTRACE, 1, [if memory trace is wanted])
fi

# Debug arguments for compilation
ACX_COMPILE_WARN()


#
# substitute vars
#
AS_SCRUB_INCLUDE(gwenhywfar_includes)
AC_SUBST(gwenhywfar_includes)
AC_SUBST(gwenhywfar_libs)
AC_SUBST(gwenhywfar_needed_libs)
AC_SUBST(GWEN_OS)
AC_SUBST(GWEN_MAKE_DLL_TARGET)
AC_SUBST(GWEN_INSTALL_DLL_TARGET)
AC_SUBST(gwenhywfar_os_lib)
AC_SUBST(GWENHYWFAR_LIB)
AC_SUBST(gwenhywfar_crypt_lib)
AC_SUBST(GWEN_CRYPTO)
AC_SUBST(GWEN_HEADER_FILES)
AC_SUBST(all_includes)
AC_SUBST(all_libraries)


#
# list of files to create
#
AC_CONFIG_FILES([Doxyfile
                 Makefile
                 m4/Makefile
                 gwenhywfar-config.in
		 gwenhywfar.spec
                 doc/Makefile
                 src/Makefile
                 src/ressource.rc
                 src/version.h
                 src/base/Makefile
                 src/crypt/Makefile
                 src/crypt/ossl/Makefile
                 src/crypt/none/Makefile
                 src/io/Makefile
                 src/ipc/Makefile
                 src/os/Makefile
                 src/os/posix/Makefile
                 src/os/windows/Makefile
                 src/parser/Makefile
                 test/Makefile])
AC_OUTPUT
