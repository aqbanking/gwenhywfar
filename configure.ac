#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# info
GWENHYWFAR_AUTHOR="Martin Preuss<martin@libchipcard.de>"
GWENHYWFAR_BUGREPORT_ADDR="<martin@libchipcard.de>"


AC_PREREQ(2.56)
AC_INIT
AC_CANONICAL_TARGET([])
AC_CONFIG_SRCDIR([src/gwenhywfarapi.h])
AC_CONFIG_HEADER([config.h])

#
# versions
#
GWENHYWFAR_VERSION_MAJOR=0
GWENHYWFAR_VERSION_MINOR=9
GWENHYWFAR_VERSION_PATCHLEVEL=11
GWENHYWFAR_VERSION_BUILD=15
dnl "stable", "rcX", "betaX", "cvs"
GWENHYWFAR_VERSION_TAG="cvs"
GWENHYWFAR_VERSION_FULL_STRING="$GWENHYWFAR_VERSION_MAJOR.$GWENHYWFAR_VERSION_MINOR.$GWENHYWFAR_VERSION_PATCHLEVEL.$GWENHYWFAR_VERSION_BUILD${GWENHYWFAR_VERSION_TAG}"
GWENHYWFAR_VERSION_STRING="$GWENHYWFAR_VERSION_MAJOR.$GWENHYWFAR_VERSION_MINOR.$GWENHYWFAR_VERSION_PATCHLEVEL"


#
# Create release string
#
case "$GWENHYWFAR_VERSION_TAG" in
  cvs)
     GWENHYWFAR_VERSION_RELEASE_STRING="$GWENHYWFAR_VERSION_FULL_STRING"
     ;;
  stable)
     GWENHYWFAR_VERSION_RELEASE_STRING="$GWENHYWFAR_VERSION_MAJOR.$GWENHYWFAR_VERSION_MINOR"
     # should we add patchlevel ?
     if test "$GWENHYWFAR_VERSION_PATCHLEVEL" != "0"; then
       GWENHYWFAR_VERSION_RELEASE_STRING="$GWENHYWFAR_VERSION_RELEASE_STRING.$GWENHYWFAR_VERSION_PATCHLEVEL"
     fi
     ;;
  *)
     GWENHYWFAR_VERSION_RELEASE_STRING="$GWENHYWFAR_VERSION_MAJOR.$GWENHYWFAR_VERSION_MINOR"
     # should we add patchlevel ?
     if test "$GWENHYWFAR_VERSION_PATCHLEVEL" != "0"; then
       GWENHYWFAR_VERSION_RELEASE_STRING="$GWENHYWFAR_VERSION_RELEASE_STRING.$GWENHYWFAR_VERSION_PATCHLEVEL"
     fi
     
     # add TAG
     GWENHYWFAR_VERSION_RELEASE_STRING="$GWENHYWFAR_VERSION_RELEASE_STRING${GWENHYWFAR_VERSION_TAG}"
     ;;
esac
   

AC_SUBST(GWENHYWFAR_VERSION_MAJOR)
AC_SUBST(GWENHYWFAR_VERSION_MINOR)
AC_SUBST(GWENHYWFAR_VERSION_PATCHLEVEL)
AC_SUBST(GWENHYWFAR_VERSION_BUILD)
AC_SUBST(GWENHYWFAR_VERSION_TAG)
AC_SUBST(GWENHYWFAR_VERSION_FULL_STRING)
AC_SUBST(GWENHYWFAR_VERSION_STRING)
AC_SUBST(GWENHYWFAR_VERSION_RELEASE_STRING)

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
GWENHYWFAR_SO_CURRENT="13"
GWENHYWFAR_SO_REVISION="0"
GWENHYWFAR_SO_AGE="0"
GWENHYWFAR_SO_EFFECTIVE="`echo \$(($GWENHYWFAR_SO_CURRENT-$GWENHYWFAR_SO_AGE))`"

AC_SUBST(GWENHYWFAR_SO_CURRENT)
AC_SUBST(GWENHYWFAR_SO_REVISION)
AC_SUBST(GWENHYWFAR_SO_AGE)
AC_SUBST(GWENHYWFAR_SO_EFFECTIVE)

AM_INIT_AUTOMAKE(gwenhywfar, $GWENHYWFAR_VERSION_RELEASE_STRING)


AC_ENABLE_STATIC(no)
AC_ENABLE_SHARED(yes)


# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PROG_AWK
AC_CHECK_PROG(USE_DOT,dot,YES,NO)


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
AC_FUNC_MKTIME
AC_FUNC_SELECT_ARGTYPES
AC_CHECK_FUNCS([gethostbyaddr gethostbyname inet_ntoa isascii getpid])
AC_CHECK_FUNCS([memmove memset])
AC_CHECK_FUNCS([select socket])
AC_CHECK_FUNCS([strcasecmp strchr strdup strerror])
AC_CHECK_FUNCS([snprintf])
AC_CHECK_FUNCS([symlink])

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
        gwenhywfar_headers="`eval echo $prefix/include/gwenhywfar`"
        gwenhywfar_sections=""
        gwenhywfar_plugins="`eval echo $prefix/lib`/gwenhywfar/plugins/${GWENHYWFAR_SO_EFFECTIVE}"
        gwenhywfar_os_lib="os/posix/libos.la"
        GWEN_OS="posix"
        # check for some types
        AQ_TYPE_UINT32(unsigned int)
        GWEN_TYPE_UINT32=$aq_type_uint32
        AC_SUBST(GWEN_TYPE_UINT32)
        AQ_TYPE_UINT64(unsigned long long)
        GWEN_TYPE_UINT64=$aq_type_uint64
        AC_SUBST(GWEN_TYPE_UINT64)
        
        # add special libraries for solaris
        if test "$OSYSTEM" = "solaris"; then
          all_libraries="${all_libraries} -lresolv -lsocket"
        fi
    	;;
    windows)
        AC_DEFINE_UNQUOTED(BUILDING_GWEN_DLL,1, [Define if DLL is built])
        ACX_WINDOWS_PATHS
        gwenhywfar_libs="-L${WIN_PATH_WINDOWS_MINGW} -lgwenhywfar32_${GWENHYWFAR_SO_EFFECTIVE}"
        gwenhywfar_includes="-I`eval echo $prefix/include`"
        gwenhywfar_headers="`eval echo $prefix/include/gwenhywfar`"
        gwenhywfar_sections=""
        gwenhywfar_plugins="`eval echo $prefix/lib`/gwenhywfar/plugins/${GWENHYWFAR_SO_EFFECTIVE}"
        gwenhywfar_os_lib="os/windows/libos.la"
        GWEN_MAKE_DLL_TARGET="dll"
        GWEN_INSTALL_DLL_TARGET="dll-install"
        GWEN_OS="windows"
        # check for some types
        AQ_TYPE_UINT32(unsigned int)
        GWEN_TYPE_UINT32=$aq_type_uint32
        AC_SUBST(GWEN_TYPE_UINT32)
        AQ_TYPE_UINT64(unsigned long long)
        GWEN_TYPE_UINT64=$aq_type_uint64
        AC_SUBST(GWEN_TYPE_UINT64)
    	;;
esac

AM_CONDITIONAL(IS_WINDOWS, [test "$OS_TYPE" = "windows"])

#
# check whether variadic macros should be supported
#
AC_MSG_CHECKING(if compiling with variadic macros)
AC_ARG_ENABLE(variadic-macros,
  [  --enable-variadic-macros         enable variadic macros (default=yes)],
  enable_variadic_macros="$enableval",
  enable_variadic_macros="yes")
AC_MSG_RESULT($enable_variadic_macros)
if test "x$enable_variadic_macros" == "xno"; then
  AC_DEFINE(NO_VARIADIC_MACROS, 1, [Define if variadic macros should be disabled])
fi

#
# check for encryption library
#
AQ_CHECK_SSL
if test "$ssl_available" = "yes"; then
  all_libraries="${all_libraries} ${ssl_libraries} ${ssl_lib}"
  all_includes="$all_includes $ssl_includes"
  gwenhywfar_includes="$gwenhywfar_includes $ssl_includes"
  GWEN_CRYPTO="ossl"
  AC_DEFINE_UNQUOTED(GWEN_SSL_CRYPTO, 1, [Defined if OpenSSL crypto module is built])
  USE_CRYPT="yes"
else
  GWEN_CRYPTO="none"
  USE_CRYPT="no"
fi
AM_CONDITIONAL(GWEN_SSL_CRYPTO, [test "$GWEN_CRYPTO" = "ossl"])
AC_SUBST(USE_CRYPT)


# ---------------------------------------------------------------------------
# check for ncurses
#

USE_NCURSES="yes"
AC_CHECK_LIB(ncurses, newwin, [], [USE_NCURSES="no"])
AC_CHECK_LIB(panel, new_panel, [], [USE_NCURSES="no"])
AC_CHECK_HEADERS([ncurses.h], [], [USE_NCURSES="no"])
AC_CHECK_HEADERS([panel.h], [], [USE_NCURSES="no"])
if test "$USE_NCURSES" != "yes"; then
  AC_MSG_WARN([
*** ncurses not found, the text interface will not be available.
*** It is strongly recommended to compile Gwenhywfar with ncurses support
*** since at least my projects (Libchipcard, AqHBCI) will rely on it...
***
*** Please make sure that the ncurses-devel packages are also installed.])
  NCURSES_TOOLS=""
else
  AC_DEFINE(USE_NCURSES, 1, [whether to use ncurses])
  all_libraries="$all_libraries -lpanel -lncurses"
  NCURSES_TOOLS=""
fi
AC_SUBST(NCURSES_TOOLS)
AC_SUBST(USE_NCURSES)
AM_CONDITIONAL(USE_NCURSES, [test "$USE_NCURSES" = "yes"])


#
# create header directory, make symlinks
#
GWEN_HEADER_DIRS="src src/base src/net src/crypt src/crypt/${GWEN_CRYPTO} src/io src/ipc src/os src/os/${GWEN_OS}"
GWEN_MODULES="ui"
lprefix="${srcdir}"
lprefix=`( cd ${lprefix} ; pwd )`

rm -Rf gwenhywfar &>/dev/null
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
test -f gwenhywfar/version.h || ln -s "${lprefix}/src/version.h" "gwenhywfar/version.h" &>/dev/null
test -f gwenhywfar/types.h || ln -s "${lprefix}/src/types.h" "gwenhywfar/types.h" &>/dev/null
test -f gwenhywfar/system.h || ln -s "${lprefix}/src/os/$OS_TYPE/system.h" "gwenhywfar/system.h" &>/dev/null


GWEN_MODULES="ui"
lprefix="${srcdir}"
lprefix=`( cd ${lprefix} ; pwd )`
for d in ${GWEN_MODULES}; do
  rm -Rf gwenhywfar/$d &>/dev/null
  mkdir gwenhywfar/$d
  hfiles=`( cd "${srcdir}/src/${d}" && ls *.h 2>/dev/null )`
  for f in ${hfiles}; do
    GWEN_HEADER_FILES="${GWEN_HEADER_FILES} ${lprefix}/${d}/${f}"
    case ${f} in
      *_p.h | *_l.h)
      		;;
      *)
    		ln -s "${lprefix}/src/${d}/${f}" "gwenhywfar/${d}/${f}"
                ;;
    esac
  done
done


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


dnl Search for dlopen
LIBS=""
AC_SEARCH_LIBS(dlopen, c dl)
if test "$LIBS" = "-lc"; then
    LIBS=""
fi
ld_library="$LIBS"
LIBS=""
all_libraries="$all_libraries $ld_library"


#
# check for release
#
AC_MSG_CHECKING(whether this is an official release)
AC_ARG_ENABLE(release,
  [  --enable-release         make this an official release (default=no)],
  [ case "${enableval}" in
    yes) enable_release="yes";;
    no)  enable_release="no";;
    *) AC_MSG_ERROR(bad value ${enableval} for --enable-release);;
    esac
  ],
  enable_release="no")

if test "$enable_release" = "yes"; then
  STRIPALL="-s"
else
  STRIPALL=""
fi
AC_SUBST(STRIPALL)
AC_MSG_RESULT($enable_release)



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
AC_SUBST(gwenhywfar_plugins)
AC_SUBST(gwenhywfar_headers)
AC_SUBST(gwenhywfar_sections)
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
                 src/types.h
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
                 src/os/posix/system.h
                 src/os/windows/Makefile
                 src/os/windows/system.h
                 src/parser/Makefile
                 src/net/Makefile
                 src/ui/Makefile
                 plugins/Makefile
                 plugins/dbio/Makefile
                 plugins/dbio/csv/Makefile
                 tools/Makefile
                 tools/xmlmerge/Makefile
                 test/Makefile
		])
AC_OUTPUT


