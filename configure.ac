#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

#

###-------------------------------------------------------------------------
#
# The following line makes sure that this file is always touched when a new
# revision is checked out.
#
# $Rev$
#

# info
GWENHYWFAR_AUTHOR="Martin Preuss<martin@libchipcard.de>"
GWENHYWFAR_BUGREPORT_ADDR="<martin@libchipcard.de>"


AC_PREREQ(2.56)
AC_INIT
AC_CANONICAL_TARGET([])
AC_CONFIG_SRCDIR([src/gwenhywfarapi.h])
AC_CONFIG_HEADERS([config.h])

###-------------------------------------------------------------------------
#
# versions
#
GWENHYWFAR_VERSION_MAJOR=2
GWENHYWFAR_VERSION_MINOR=3
GWENHYWFAR_VERSION_PATCHLEVEL=0
dnl "stable", "rcX", "betaX", "cvs"
GWENHYWFAR_VERSION_TAG="svn"



###-------------------------------------------------------------------------
#
# Generate version strings and BUILD
#

GWENHYWFAR_VERSION_BUILD="0"
case "$GWENHYWFAR_VERSION_TAG" in svn|cvs)
  SVN=`which svn`
  case "x$SVN" in
    x|xno)
      GWENHYWFAR_VERSION_BUILD="0"
      ;;
    *)
      # Let svn write the revision number directly into here on
      # each svn update. Note: The trailing space is necessary for
      # cvs versions.
      GWENHYWFAR_VERSION_BUILD=`echo "$Rev$ " | cut -d' ' -f2`
      if test "x$GWENHYWFAR_VERSION_BUILD" = "x" ; then
	GWENHYWFAR_VERSION_BUILD="0"
      fi
      ;;
  esac
  ;;
esac
GWENHYWFAR_VERSION_FULL_STRING="$GWENHYWFAR_VERSION_MAJOR.$GWENHYWFAR_VERSION_MINOR.$GWENHYWFAR_VERSION_PATCHLEVEL-${GWENHYWFAR_VERSION_TAG}-r${GWENHYWFAR_VERSION_BUILD}"
GWENHYWFAR_VERSION_STRING="$GWENHYWFAR_VERSION_MAJOR.$GWENHYWFAR_VERSION_MINOR.$GWENHYWFAR_VERSION_PATCHLEVEL"

# The version number for the plugin/crypttoken/ohbci/ohbci.la
OHBCI_VERSION_STRING="0.1.0"

###-------------------------------------------------------------------------
#
# SO version for Gwenhywfar
#
GWENHYWFAR_SO_CURRENT="44"
GWENHYWFAR_SO_AGE="6"
GWENHYWFAR_SO_REVISION="0"
GWENHYWFAR_SO_EFFECTIVE="`echo \$(($GWENHYWFAR_SO_CURRENT-$GWENHYWFAR_SO_AGE))`"



###-------------------------------------------------------------------------
#
# Create release strings
#
case "$GWENHYWFAR_VERSION_TAG" in
  cvs|svn)
     GWENHYWFAR_VERSION_RELEASE_STRING="$GWENHYWFAR_VERSION_MAJOR.$GWENHYWFAR_VERSION_MINOR.$GWENHYWFAR_VERSION_PATCHLEVEL.r${GWENHYWFAR_VERSION_BUILD}"
     ;;
  stable)
     GWENHYWFAR_VERSION_RELEASE_STRING="$GWENHYWFAR_VERSION_MAJOR.$GWENHYWFAR_VERSION_MINOR.$GWENHYWFAR_VERSION_PATCHLEVEL"
     ;;
  *)
     GWENHYWFAR_VERSION_RELEASE_STRING="$GWENHYWFAR_VERSION_MAJOR.$GWENHYWFAR_VERSION_MINOR.$GWENHYWFAR_VERSION_PATCHLEVEL"
     
     # add TAG
     GWENHYWFAR_VERSION_RELEASE_STRING="$GWENHYWFAR_VERSION_RELEASE_STRING-${GWENHYWFAR_VERSION_TAG}"
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
AC_SUBST(OHBCI_VERSION_STRING)

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


AC_SUBST(GWENHYWFAR_SO_CURRENT)
AC_SUBST(GWENHYWFAR_SO_REVISION)
AC_SUBST(GWENHYWFAR_SO_AGE)
AC_SUBST(GWENHYWFAR_SO_EFFECTIVE)

AM_INIT_AUTOMAKE(gwenhywfar, $GWENHYWFAR_VERSION_RELEASE_STRING)



###-------------------------------------------------------------------------
#
# SO version for GwenUI
#
GWENUI_SO_CURRENT="1"
GWENUI_SO_REVISION="1"
GWENUI_SO_AGE="0"
GWENUI_SO_EFFECTIVE="`echo \$(($GWENUI_SO_CURRENT-$GWENUI_SO_AGE))`"

AC_SUBST(GWENUI_SO_CURRENT)
AC_SUBST(GWENUI_SO_REVISION)
AC_SUBST(GWENUI_SO_AGE)
AC_SUBST(GWENUI_SO_EFFECTIVE)



###-------------------------------------------------------------------------
#
# SO version for GwenPP
#
GWENPP_SO_CURRENT="16"
GWENPP_SO_REVISION="0"
GWENPP_SO_AGE="0"
GWENPP_SO_EFFECTIVE="`echo \$(($GWENPP_SO_CURRENT-$GWENPP_SO_AGE))`"

AC_SUBST(GWENPP_SO_CURRENT)
AC_SUBST(GWENPP_SO_REVISION)
AC_SUBST(GWENPP_SO_AGE)
AC_SUBST(GWENPP_SO_EFFECTIVE)



###-------------------------------------------------------------------------
# check for OS

AQ_CHECK_OS


###-------------------------------------------------------------------------
# prefix handling

AC_PREFIX_DEFAULT(/usr)
if test "x$prefix" = "xNONE"; then
  prefix=$ac_default_prefix
  ac_configure_args="$ac_configure_args --prefix $prefix"
fi
AC_SUBST(prefix)



###-------------------------------------------------------------------------
#
# static/shared handling
#
AC_ENABLE_STATIC(no)
AC_ENABLE_SHARED(yes)


###-------------------------------------------------------------------------
#
# Checks for programs.
#
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL

AC_LIBTOOL_WIN32_DLL
AC_LIBTOOL_RC
AC_PROG_LIBTOOL
AC_PROG_AWK
AC_CHECK_PROG(USE_DOT,dot,YES,NO)
AC_CHECK_PROG(SED,sed,sed)


###-------------------------------------------------------------------------
#
# Checks for libraries.
#
AC_CHECK_LIB(dl, dlopen)
# note: this AC_CHECK_LIB will also add -ldl to LIBS.


###-------------------------------------------------------------------------
#
# Checks for header files.
#
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h])
AC_CHECK_HEADERS([stdlib.h string.h strings.h sys/socket.h sys/time.h])
AC_CHECK_HEADERS([syslog.h unistd.h])
AC_CHECK_HEADERS([unistd.h])
AC_CHECK_HEADERS([dlfcn.h])
AC_CHECK_HEADERS([signal.h termios.h iconv.h])
AC_CHECK_HEADERS([time.h locale.h])
AC_CHECK_HEADERS([syslog.h])
AC_CHECK_HEADERS([sys/stat.h])
AC_CHECK_HEADERS([sys/types.h])
AC_CHECK_HEADERS([sys/wait.h])
AC_CHECK_HEADERS([regex.h])


###-------------------------------------------------------------------------
#
# Checks for typedefs, structures, and compiler characteristics.
#
AC_C_CONST
AC_HEADER_TIME
AC_STRUCT_TM


###-------------------------------------------------------------------------
#
# Checks for library functions.
#
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_MKTIME
AC_FUNC_SELECT_ARGTYPES
AC_CHECK_FUNCS([gethostbyaddr gethostbyname inet_ntoa isascii getpid])
AC_CHECK_FUNCS([memmove memset])
AC_CHECK_FUNCS([select socket])
AC_CHECK_FUNCS([strcasecmp strchr strdup strerror])
AC_CHECK_FUNCS([snprintf])
AC_CHECK_FUNCS([symlink setlocale])
AC_CHECK_FUNCS([srandom random])


###-------------------------------------------------------------------------
#
# OS dependant settings
#

case "$OS_TYPE" in
    posix)
        gwenhywfar_sys_is_windows=0
        gwenhywfar_ldflags="-L\${libdir}"
        gwenhywfar_libs="-lgwenhywfar"
	gwenhywfar_internal_libname="libgwenhywfar.la"
        gwenhywfar_headerdir_am="\$(includedir)"
        gwenhywfar_headerdir="\${includedir}"
        gwenhywfar_plugindir="\${libdir}/gwenhywfar/plugins/${GWENHYWFAR_SO_EFFECTIVE}"
        gwenhywfar_os_lib="os/posix/libos.la"
        GWEN_OS="posix"
        
        # add special libraries for solaris
        if test "$OSYSTEM" = "solaris"; then
          all_libraries="${all_libraries} -lresolv -lsocket"
          PROG_LIBRARIES="${PROG_LIBRARIES} -lresolv -lsocket"
        fi
        
        # add special libraries for darwin
        if test "$OSYSTEM" = "osx"; then
	  # Add -L/sw/lib here since it is needed on darwin, but
	  # it should be avoided when linking the test program in
	  # test/.
          all_libraries="-L/sw/lib ${all_libraries} -lintl"
          # hmm, this should be detected by the GETTEXT macro...
          PROG_LIBRARIES="${PROG_LIBRARIES} -lintl"
        fi
    	;;
    windows)
        gwenhywfar_sys_is_windows=1
        ACX_WINDOWS_PATHS
        gwenhywfar_ldflags="-L${WIN_PATH_WINDOWS_MINGW}"

        #gwenhywfar_libs="-lgwenhywfar32_${GWENHYWFAR_SO_EFFECTIVE}"
	#gwenhywfar_internal_libname="gwenhywfar32_${GWENHYWFAR_SO_EFFECTIVE}.dll"

        gwenhywfar_libs="-lgwenhywfar-${GWENHYWFAR_SO_EFFECTIVE}"
	gwenhywfar_internal_libname="libgwenhywfar.la"

        gwenhywfar_headerdir_am="\$(includedir)"
        gwenhywfar_headerdir="\${includedir}"
        gwenhywfar_plugindir="\${libdir}/gwenhywfar/plugins/${GWENHYWFAR_SO_EFFECTIVE}"
        gwenhywfar_os_lib="os/windows/libos.la"
        GWEN_OS="windows"
        PROG_LIBRARIES="-L/mingw/lib ${PROG_LIBRARIES}"
	all_libraries="${all_libraries} -lregex -lwsock32"
    	;;
    *)
    	AC_MSG_ERROR(**** Unknown operating system type)
        ;;
esac

AM_CONDITIONAL(IS_WINDOWS, [test "$OS_TYPE" = "windows"])
AC_SUBST(gwenhywfar_sys_is_windows)


dnl # Check for x86_64 architecture; potentially set lib-directory suffix
dnl if test "$target_cpu" = "x86_64"; then
dnl   libdirsuffix="64"
dnl else
dnl   libdirsuffix=""
dnl fi
dnl AC_DEFINE_UNQUOTED(LIBDIRSUFFIX, ["$libdirsuffix"], [Defined to the suffix string of the library directory])
dnl --- no longer needed since $libdir is used directly


###-------------------------------------------------------------------------
#
# check for some types and printf-templates
#
AQ_TYPE_UINT32(unsigned int)
AQ_TYPE_UINT64(unsigned long long)
GWEN_TYPE_UINT32=$aq_type_uint32
AC_SUBST(GWEN_TYPE_UINT32)
GWEN_TYPE_TMPL_UINT32=$aq_type_tmpl_uint32
AC_SUBST(GWEN_TYPE_TMPL_UINT32)
GWEN_TYPE_UINT64=$aq_type_uint64
AC_SUBST(GWEN_TYPE_UINT64)
GWEN_TYPE_TMPL_UINT64=$aq_type_tmpl_uint64
AC_SUBST(GWEN_TYPE_TMPL_UINT64)


###-------------------------------------------------------------------------
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



###-------------------------------------------------------------------------
#
# check whether profiling is to be supported
#
AC_MSG_CHECKING(if profiling is to be supported)
AC_ARG_ENABLE(profiling,
  [  --enable-profiling         enable profiling (default=no)],
  enable_profiling="$enableval",
  enable_profiling="no")
AC_MSG_RESULT($enable_profiling)
if test "x$enable_profiling" == "xyes"; then
  PROFILING_CFLAGS="-pg"
  PROFILING_LDFLAGS="-pg"
else
  PROFILING_CFLAGS=""
  PROFILING_LDFLAGS=""
fi
AC_SUBST(PROFILING_CFLAGS)
AC_SUBST(PROFILING_LDFLAGS)


###-------------------------------------------------------------------------
#
# check for encryption library
#
AQ_CHECK_SSL
if test "$ssl_available" = "yes"; then
  all_includes="$all_includes $ssl_includes"
  GWEN_CRYPTO="ossl"
  AC_DEFINE_UNQUOTED(GWEN_SSL_CRYPTO, 1, [Defined if OpenSSL crypto module is built])
  USE_CRYPT="yes"
else
  GWEN_CRYPTO="none"
  USE_CRYPT="no"
fi
AM_CONDITIONAL(GWEN_SSL_CRYPTO, [test "$GWEN_CRYPTO" = "ossl"])
AC_SUBST(USE_CRYPT)



###-------------------------------------------------------------------------
#
# check for LibXML
#
AC_MSG_CHECKING([whether LibXML2 should be used])
if test "x`which xml2-config`" != "x"; then
  AC_DEFINE(USE_LIBXML2, 1, [whether LibXML2 should be used])
  xml_includes="`xml2-config --cflags`"
  xml_libraries="`xml2-config --libs`"
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
fi
AC_SUBST(xml_includes)
AC_SUBST(xml_libraries)



###-------------------------------------------------------------------------
#
# check for I18N
#
HAVE_I18N="yes"
AC_CHECK_PROG(MSGFMT,msgfmt,msgfmt,missing)
AC_CHECK_PROG(XGETTEXT,xgettext,xgettext, missing)

if test "$XGETTEXT" != "missing" -a "$MSGFMT" != "missing" ; then
  AC_CHECK_HEADERS([locale.h libintl.h], [], [HAVE_I18N="no"])
  AC_SEARCH_LIBS(gettext, intl, [], [HAVE_I18N="no"])
else
  HAVE_I18N="no"
  AC_MSG_WARN([xgettext or msgfmt is missing. Locale suport is disabled.])
fi
AC_SUBST(HAVE_I18N)
if test "$HAVE_I18N" = "yes"; then
  AC_DEFINE_UNQUOTED(HAVE_I18N, 1, [whether I18N is available])
  AC_DEFINE_UNQUOTED(ENABLE_NLS, 1, [whether I18N is available])
fi
AM_CONDITIONAL(USE_I18N, [test "$HAVE_I18N" = "yes"])


###-------------------------------------------------------------------------
#
# create header directory, make symlinks
#
GWEN_HEADER_DIRS="src src/base src/net2 src/crypt src/crypt/${GWEN_CRYPTO} src/crypt2 src/io src/ipc src/ipc/request src/os src/os/${GWEN_OS} src/storage"
GWEN_HEADER_DIRS="${GWEN_HEADER_DIRS} src/parser"
lprefix="${srcdir}"
lprefix=`( cd ${lprefix} ; pwd )`

rm -Rf gwenhywfar 2>/dev/null
mkdir gwenhywfar
GWEN_HEADER_FILES="doc/apidoc.h"
for d in ${GWEN_HEADER_DIRS}; do
  hfiles=`( cd "${srcdir}/${d}" && ls *.h 2>/dev/null )`
  for f in ${hfiles}; do
    case ${f} in
      *_l.h | *_p.h)
      	 ;;
      *)
         GWEN_HEADER_FILES="${GWEN_HEADER_FILES} ${d}/${f}"
    	 ln -s "${lprefix}/${d}/${f}" "gwenhywfar/${f}"
         ;;
    esac
  done
done
test -f gwenhywfar/version.h || ln -s "${lprefix}/src/version.h" "gwenhywfar/version.h" 2>/dev/null
test -f gwenhywfar/types.h || ln -s "${lprefix}/src/types.h" "gwenhywfar/types.h" 2>/dev/null
test -f gwenhywfar/system.h || ln -s "${lprefix}/src/os/$OS_TYPE/system.h" "gwenhywfar/system.h" 2>/dev/null

AC_SUBST(GWEN_HEADER_FILES)


###-------------------------------------------------------------------------
#
# check docpath
#
AC_MSG_CHECKING(docpath)
AC_ARG_WITH(docpath, [  --with-docpath=DIR where to store the apidoc],
  [docpath="$withval"],
  [docpath="${HOME}/apidoc"])
AC_SUBST(docpath)
AC_MSG_RESULT($docpath)


###-------------------------------------------------------------------------
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
	DOXYGEN_INPUT="listdoc.h src"
        DOXYGEN_DEFINE=""
else
	DOXYGEN_DEFINE="DOXYGEN_HIDE"
	DOXYGEN_INPUT="listdoc.h ${GWEN_HEADER_FILES}"
fi
AC_SUBST(DOXYGEN_INPUT)
AC_SUBST(DOXYGEN_DEFINE)
AC_MSG_RESULT($enable_fulldoc)


###-------------------------------------------------------------------------
#
# search for tag files
#
AC_MSG_CHECKING(doxygen tag files)
DOXYGEN_TAGFILES=""
if test -d "${docpath}"; then
  DOXYGEN_TAGFILES="`cd ${docpath} && ls *.tag`"
  if test -n "${DOXYGEN_TAGFILES}"; then
   DOXYGEN_TAGFILES="`echo ${DOXYGEN_TAGFILES} | ${SED} -e s/${PACKAGE}.tag//`"
  fi
  realfiles=""
  for ff in ${DOXYGEN_TAGFILES}; do
    realfiles="${realfiles} ${docpath}/${ff}"
  done
  DOXYGEN_TAGFILES="${realfiles}"
fi
if test -z "${DOXYGEN_TAGFILES}"; then
  AC_MSG_RESULT(none)
else
  AC_MSG_RESULT(found)
fi
AC_SUBST(DOXYGEN_TAGFILES)



###-------------------------------------------------------------------------
#
# Search for dlopen
#
LIBS=""
AC_SEARCH_LIBS(dlopen, c dl)
if test "$LIBS" = "-lc"; then
    LIBS=""
fi
ld_library="$LIBS"
LIBS=""
all_libraries="$all_libraries $ld_library"


###-------------------------------------------------------------------------
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



###-------------------------------------------------------------------------
#
# GCC version (check for usability)
#
AC_MSG_CHECKING(if symbol visibility is supported)
visibility_cflags=""
case `basename $CC` in 
  gcc*)
     dnl Determine version number (watch out: the outside square
     dnl parenteses are vitally important here!)
     gccversion=["` ${CC} --version | head -1 | ${SED} 's/.*\([0-9]\.[0-9]\.[0-9]*\).*/\1/' `"]
     case $gccversion in
       4.*)
	 # Activate -fvisibility=hidden only for explicit 4.x gcc
	 # versions. We never now what future gcc's might do with
	 # that argument, and we also can't be sure that other
	 # gcc's happen to show very weird version
	 # numbers. Therefore leave the default at non-activated.
         visibility_supported="yes, gcc-${gccversion}"
         AC_DEFINE(GCC_WITH_VISIBILITY_ATTRIBUTE, 1, [visibility])
         visibility_cflags="-fvisibility=hidden"
         ;;
       *)
	 # Default: -fvisibility=hidden not activated.
         visibility_supported="no (needs gcc >=4.0, you have gcc-${gccversion})"
         ;;
      esac
      ;;
  *)
    visibility_supported="no (needs gcc >=4.0, you use $CC)"
    ;;
esac

# -fvisibility is at least not available on MinGW/gcc-3.4.4 (will
# give an "unrecognized cmdline option"). Also, unfortunately I
# don't know an easy way to ask the compiler here. Maybe
# http://autoconf-archive.cryp.to/ax_cflags_gcc_option.html
case "$OS_TYPE" in
  windows)
        visibility_supported="no (not yet on MinGW/Windows)"
        visibility_cflags=""
	;;
  *)
	;;
esac

AC_MSG_RESULT(${visibility_supported})
AC_SUBST(visibility_cflags)


###-------------------------------------------------------------------------
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
  #AC_DEFINE(GWEN_MEMTRACE, 1, [if memory trace is wanted])
  AC_DEFINE(GWEN_MEMORY_DEBUG, 1, [if memory debugging is wanted])
fi


# Debug arguments for compilation
ACX_COMPILE_WARN()


###-------------------------------------------------------------------------
#
# compile program flagss
#

PROG_LIBRARIES="${PROG_LIBRARIES} ${ssl_libraries} ${ssl_lib} ${ncurses_libs}"

# Clean up include variables
AS_SCRUB_INCLUDE(ssl_includes)
AS_SCRUB_INCLUDE(gwenhywfar_includes)
AS_SCRUB_INCLUDE(all_includes)


###-------------------------------------------------------------------------
#
# substitute vars
#
AC_SUBST(GWEN_CRYPTO)
AC_SUBST(GWEN_HEADER_FILES)
AC_SUBST(GWEN_OS)
AC_SUBST(PROG_LIBRARIES)
AC_SUBST(all_includes)
AC_SUBST(all_libraries)
AC_SUBST(gwenhywfar_crypt_lib)
AC_SUBST(gwenhywfar_ldflags)
AC_SUBST(gwenhywfar_libs)
AC_SUBST(gwenhywfar_internal_libname)
AC_SUBST(gwenhywfar_libspp)
AC_SUBST(gwenhywfar_libsui)
AC_SUBST(gwenhywfar_os_lib)
AC_SUBST(gwenhywfar_pkgincludedir)
AC_SUBST(gwenhywfar_plugindir)
AC_SUBST(gwenhywfar_headerdir_am)
AC_SUBST(gwenhywfar_headerdir)


###-------------------------------------------------------------------------
#
# list of files to create
#
AC_CONFIG_FILES([Doxyfile
                 Makefile
                 admin/Makefile
                 m4/Makefile
                 po/Makefile
                 gwenhywfar-config.in
		 gwenhywfar.pc
		 gwenhywfar.spec
		 gwenhywfar.iss
                 doc/Makefile
                 doc/inherit/Makefile
		 doc/plugindescr.xsd
                 src/Makefile
                 src/types.h
                 src/ressource.rc
                 src/version.h
                 src/base/Makefile
                 src/crypt/Makefile
                 src/crypt/ossl/Makefile
                 src/crypt/none/Makefile
                 src/crypt2/Makefile
                 src/io/Makefile
                 src/ipc/Makefile
                 src/ipc/request/Makefile
                 src/os/Makefile
                 src/os/posix/Makefile
                 src/os/posix/system.h
                 src/os/windows/Makefile
                 src/os/windows/system.h
                 src/parser/Makefile
                 src/net2/Makefile
                 src/storage/Makefile
                 plugins/Makefile
                 plugins/dbio/Makefile
                 plugins/dbio/csv/Makefile
                 plugins/dbio/csv/csv.xml
                 plugins/dbio/csv/ressource.rc
                 plugins/dbio/olddb/Makefile
                 plugins/dbio/olddb/olddb.xml
                 plugins/dbio/olddb/ressource.rc
                 plugins/dbio/rfc822/Makefile
                 plugins/dbio/rfc822/rfc822.xml
                 plugins/dbio/rfc822/ressource.rc
                 plugins/dbio/xmldb/Makefile
                 plugins/dbio/xmldb/xmldb.xml
                 plugins/dbio/xmldb/ressource.rc
                 plugins/crypttoken/Makefile
                 plugins/crypttoken/ohbci/Makefile
                 plugins/crypttoken/ohbci/ohbci.xml
                 plugins/crypttoken/ohbci/ressource.rc
                 plugins/storage/Makefile
                 plugins/storage/localfiles/Makefile
                 plugins/storage/localfiles/localfiles.xml
                 plugins/storage/localfiles/ressource.rc
                 tools/Makefile
                 tools/xmlmerge/Makefile
                 tools/typemaker/Makefile
                 tools/gct-tool/Makefile
                 test/Makefile
		])
AC_OUTPUT


