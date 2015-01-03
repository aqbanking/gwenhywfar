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


AC_PREREQ(2.60)
AC_INIT
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CONFIG_SRCDIR([src/gwenhywfarapi.h])
AC_CONFIG_HEADERS([config.h])

###-------------------------------------------------------------------------
#
# versions
#
GWENHYWFAR_VERSION_MAJOR=4
GWENHYWFAR_VERSION_MINOR=13
GWENHYWFAR_VERSION_PATCHLEVEL=1
GWENHYWFAR_VERSION_BUILD=0
dnl "stable", "rcX", "betaX", "cvs"
GWENHYWFAR_VERSION_TAG="stable"



###-------------------------------------------------------------------------
#
# SO version for Gwenhywfar
#
GWENHYWFAR_SO_CURRENT="73"
GWENHYWFAR_SO_AGE="13"
GWENHYWFAR_SO_REVISION="1"
GWENHYWFAR_SO_EFFECTIVE="`echo \$(($GWENHYWFAR_SO_CURRENT-$GWENHYWFAR_SO_AGE))`"



###-------------------------------------------------------------------------
#
# Generate version strings and BUILD
#

#GWENHYWFAR_VERSION_BUILD="0"
# Let svn write the revision number directly into here on
# each svn update. Note: The trailing space is necessary for
# cvs versions.
#GWENHYWFAR_VERSION_BUILD=`echo "$Rev$ " | cut -d' ' -f2`
#if test "x$GWENHYWFAR_VERSION_BUILD" = "x" ; then
#  GWENHYWFAR_VERSION_BUILD="0"
#fi
GWENHYWFAR_VERSION_FULL_STRING="$GWENHYWFAR_VERSION_MAJOR.$GWENHYWFAR_VERSION_MINOR.${GWENHYWFAR_VERSION_PATCHLEVEL}${GWENHYWFAR_VERSION_TAG}-${GWENHYWFAR_VERSION_BUILD}"
GWENHYWFAR_VERSION_STRING="$GWENHYWFAR_VERSION_MAJOR.$GWENHYWFAR_VERSION_MINOR.$GWENHYWFAR_VERSION_PATCHLEVEL"

# The version number for the plugin/ct/ohbci/ohbci.la
OHBCI_VERSION_STRING="0.1.0"



###-------------------------------------------------------------------------
#
# Create release strings
#
case "$GWENHYWFAR_VERSION_TAG" in
  cvs|svn|git)
     GWENHYWFAR_VERSION_RELEASE_STRING="$GWENHYWFAR_VERSION_MAJOR.$GWENHYWFAR_VERSION_MINOR.$GWENHYWFAR_VERSION_PATCHLEVEL.r${GWENHYWFAR_VERSION_BUILD}"
     ;;
  stable)
     GWENHYWFAR_VERSION_RELEASE_STRING="$GWENHYWFAR_VERSION_MAJOR.$GWENHYWFAR_VERSION_MINOR.$GWENHYWFAR_VERSION_PATCHLEVEL"
     ;;
  *)
     GWENHYWFAR_VERSION_RELEASE_STRING="$GWENHYWFAR_VERSION_MAJOR.$GWENHYWFAR_VERSION_MINOR.$GWENHYWFAR_VERSION_PATCHLEVEL"
     
     # add TAG
     GWENHYWFAR_VERSION_RELEASE_STRING="${GWENHYWFAR_VERSION_RELEASE_STRING}${GWENHYWFAR_VERSION_TAG}"
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


if test "$OSYSTEM" = "osx"; then
  COCOA_FRAMEWORK="-framework Cocoa"
  FOUNDATION_FRAMEWORK="-framework CoreFoundation"
  SYSCONF_FRAMEWORK="-framework SystemConfiguration"
  APPKIT_FRAMEWORK="-framework AppKit"
  COREDATA_FRAMEWORK="-framework CoreData"
  WEBKIT_FRAMEWORK="-framework WebKit"
fi
AC_SUBST(COCOA_FRAMEWORK)
AC_SUBST(FOUNDATION_FRAMEWORK)
AC_SUBST(SYSCONF_FRAMEWORK)
AC_SUBST(APPKIT_FRAMEWORK)
AC_SUBST(COREDATA_FRAMEWORK)
AC_SUBST(WEBKIT_FRAMEWORK)


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
AC_PROG_OBJC
#AC_PROG_OBJCXX
#AC_PROG_OBJCXXCPP
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_LIBTOOL_WIN32_DLL
AC_LIBTOOL_RC
# Newer libtool doesn't have AC_LIBTOOL_RC anymore.  Need to check for
# $(RC) by ourselves.
AC_CHECK_TOOL([RC], [windres], [no])
AC_PROG_AWK
AC_CHECK_PROG(USE_DOT,dot,YES,NO)
AC_CHECK_PROG(SED,sed,sed)

###-------------------------------------------------------------------------
#
# Checks for libraries.
#
AC_CHECK_LIB(dl, dlopen)
# note: this AC_CHECK_LIB will also add -ldl to LIBS.

AC_CHECK_LIB(rt, sem_init, 
  [ 
    rt_libs="-lrt -pthread"
    AC_DEFINE(HAVE_LIBRT, "1", [whether librt is available])
  ], [], [-pthread])
AC_SUBST(rt_libs)

# Some systems have iconv in libc, some have it in libiconv (OSF/1 and
# those with the standalone portable GNU libiconv installed). Check
# which one is available. The define ICONV_CONST will be "" or "const"
# accordingly.
AM_ICONV

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
AC_CHECK_HEADERS([signal.h termios.h langinfo.h localcharset.h iconv.h])
AC_CHECK_HEADERS([time.h locale.h])
AC_CHECK_HEADERS([syslog.h])
AC_CHECK_HEADERS([sys/stat.h])
AC_CHECK_HEADERS([sys/types.h])
AC_CHECK_HEADERS([sys/wait.h])
AC_CHECK_HEADERS([regex.h])


###-------------------------------------------------------------------------
#
# Checks for declarations
#
AC_CHECK_DECLS([sigprocmask], [], [], [
  #ifdef HAVE_SIGNAL_H
  #  include <signal.h>
  #endif
])


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

# Init binreloc
AM_BINRELOC



###-------------------------------------------------------------------------
#
# check whether gwen is a subproject to another one
#

AC_MSG_CHECKING(if gwen is a subproject)
AC_ARG_ENABLE(gwen-subproject,
  [  --enable-gwen-subproject       make gwen only a subproject (default=no)],
  enable_gwen_subproject="$enableval",
  enable_gwen_subproject="no")
AC_MSG_RESULT($enable_gwen_subproject)

if test "$enable_gwen_subproject" != "no"; then
  AC_DEFINE(GWENHYWFAR_IS_SUBPROJECT, 1, [whether gwen is a subproject])
fi



###-------------------------------------------------------------------------
#
# check whether 'make check' should run checks that depend on network connectivity
#

AC_MSG_CHECKING(network connectivity can be assumed available during 'make check')
AC_ARG_ENABLE(network-checks,
  [  --disable-network-checks       disables checks that assume available network during build (default=enabled)],
  enable_gwen_network_checks="$enableval",
  enable_gwen_network_checks="yes")
AC_MSG_RESULT($enable_gwen_network_checks)

if test "$enable_gwen_network_checks" = "no"; then
  AC_DEFINE(GWENHYWFAR_SKIP_NETWORK_CHECKS, 1, [whether to run network-dependent checks during build])
fi



###-------------------------------------------------------------------------
#
# check whether local installation mode is enabled
#

AC_MSG_CHECKING(whether local installation mode is wanted)
AC_ARG_ENABLE(local-install,
  [  --enable-local-install         allow local installation mode (default=no)],
  enable_local_install="$enableval",
  enable_local_install="no")
AC_MSG_RESULT($enable_local_install)

if test "$enable_local_install" != "no"; then
  AC_DEFINE(ENABLE_LOCAL_INSTALL, 1, [whether local install is wanted])
fi



###-------------------------------------------------------------------------
#
# OS dependant settings
#

# Common settings for all OSs
gwenhywfar_bindir="\${bindir}"
gwenhywfar_libs="-lgwenhywfar"
gwenhywfar_ldflags="-L\${libdir}"
gwenhywfar_internal_libname="libgwenhywfar.la"
gwenhywfar_headerdir_am="\$(includedir)/gwenhywfar4"
gwenhywfar_headerdir="\${includedir}/gwenhywfar4"
gwenhywfar_plugindir="\${libdir}/gwenhywfar/plugins/${GWENHYWFAR_SO_EFFECTIVE}"
gwenhywfar_symlinkdir="\$(top_builddir)/gwenhywfar4"
gwenhywfar_datadir="\${datadir}/gwenhywfar"
gwenhywfar_sysdatadir="\${datadir}"

# Now the specific settings
case "$OS_TYPE" in
    posix)
        gwenhywfar_sys_is_windows=0
        gwenhywfar_os_lib="os/posix/libos.la"
        GWEN_OS="posix"
        
        # add special libraries for solaris
        if test "$OSYSTEM" = "solaris"; then
          all_libraries="${all_libraries} -lresolv -lsocket"
          PROG_LIBRARIES="${PROG_LIBRARIES} -lresolv -lsocket"
        fi
        
        # add special libraries for darwin
        if test "$OSYSTEM" = "osx"; then
          # hmm, this should be detected by the GETTEXT macro...
          #PROG_LIBRARIES="${PROG_LIBRARIES} -lintl"
          # for local install mode on Mac OS X link with the CoreFoundation framework 
          if test "$enable_local_install" != "no"; then 
            all_libraries="${all_libraries} -framework CoreFoundation"
          fi  
        fi
    	;;
    windows)
        gwenhywfar_sys_is_windows=1
        dnl ACX_WINDOWS_PATHS

        gwenhywfar_os_lib="os/windows/libos.la"
        GWEN_OS="windows"
	all_libraries="${all_libraries} -lwsock32 -lintl"
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
# check plugin search path
#
AC_MSG_CHECKING(plugin search dir)
AC_ARG_WITH(plugin-searchdir, [  --with-plugin-searchdir=DIR where to search for plugins],
  [gwenhywfar_plugin_searchdir="$withval"],
  [gwenhywfar_plugin_searchdir=""])

if test -z "${gwenhywfar_plugin_searchdir}"; then
  if test "$OSYSTEM" = "windows"; then
    gwenhywfar_plugin_searchdir="lib/gwenhywfar/plugins/${GWENHYWFAR_SO_EFFECTIVE}"
  else
    if test "$enable_local_install" != "no"; then
      gwenhywfar_plugin_searchdir="lib/gwenhywfar/plugins/${GWENHYWFAR_SO_EFFECTIVE}"
    else
      gwenhywfar_plugin_searchdir="$gwenhywfar_plugindir"
    fi
  fi
fi

AC_SUBST(gwenhywfar_plugin_searchdir)
AC_MSG_RESULT($gwenhywfar_plugin_searchdir)



###-------------------------------------------------------------------------
#
# check data search dir
#
AC_MSG_CHECKING(data search dir)
AC_ARG_WITH(data-searchdir, [  --with-data-searchdir=DIR where to search for gwen's data files],
  [gwenhywfar_data_searchdir="$withval"],
  [gwenhywfar_data_searchdir=""])

if test -z "${gwenhywfar_data_searchdir}"; then
  if test "$OSYSTEM" = "windows"; then
    gwenhywfar_data_searchdir="share/gwenhywfar"
  else
    if test "$enable_local_install" != "no"; then
      gwenhywfar_data_searchdir="share/gwenhywfar"
    else
      gwenhywfar_data_searchdir="$gwenhywfar_datadir"
    fi
  fi
fi

AC_SUBST(gwenhywfar_data_searchdir)
AC_MSG_RESULT($gwenhywfar_data_searchdir)



###-------------------------------------------------------------------------
#
# check system data search dir
#
AC_MSG_CHECKING(system data search dir)
AC_ARG_WITH(sysdata-searchdir, [  --with-sysdata-searchdir=DIR where to search for data files],
  [gwenhywfar_sysdata_searchdir="$withval"],
  [gwenhywfar_sysdata_searchdir=""])

if test -z "${gwenhywfar_sysdata_searchdir}"; then
  if test "$OSYSTEM" = "windows"; then
    gwenhywfar_sysdata_searchdir="share"
  else
    if test "$enable_local_install" != "no"; then
      gwenhywfar_sysdata_searchdir="share"
    else
      gwenhywfar_sysdata_searchdir="$gwenhywfar_sysdatadir"
    fi
  fi
fi

AC_SUBST(gwenhywfar_sysdata_searchdir)
AC_MSG_RESULT($gwenhywfar_sysdata_searchdir)



###-------------------------------------------------------------------------
#
# check locale search dir
#
AC_MSG_CHECKING(locale search dir)
AC_ARG_WITH(locale-searchdir, [  --with-locale-searchdir=DIR where to search for locale files],
  [gwenhywfar_locale_searchdir="$withval"],
  [gwenhywfar_locale_searchdir=""])

if test -z "${gwenhywfar_locale_searchdir}"; then
  if test "$OSYSTEM" = "windows"; then
    gwenhywfar_locale_searchdir="share/locale"
  else
    if test "$enable_local_install" != "no"; then
      gwenhywfar_locale_searchdir="share/locale"
    else
      gwenhywfar_locale_searchdir="\$(localedir)"
    fi
  fi
fi

AC_SUBST(gwenhywfar_locale_searchdir)
AC_MSG_RESULT($gwenhywfar_locale_searchdir)



###-------------------------------------------------------------------------
#
# check cfg search dir
#
AC_MSG_CHECKING(cfg search dir)
AC_ARG_WITH(cfg-searchdir, [  --with-cfg-searchdir=DIR where to search for cfg files],
  [gwenhywfar_cfg_searchdir="$withval"],
  [gwenhywfar_cfg_searchdir=""])

if test -z "$gwenhywfar_cfg_searchdir"; then
  if test "$OSYSTEM" = "windows"; then
    gwenhywfar_cfg_searchdir="etc"
  else
    if test "$enable_local_install" != "no"; then
      gwenhywfar_cfg_searchdir="etc"
    else
      gwenhywfar_cfg_searchdir="\$(sysconfdir)"
    fi
  fi
fi


AC_SUBST(gwenhywfar_cfg_searchdir)
AC_MSG_RESULT($gwenhywfar_cfg_searchdir)



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
if test "x$enable_variadic_macros" = "xno"; then
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
if test "x$enable_profiling" = "xyes"; then
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
# check for Debian multiarch paths
#
AC_PATH_TOOL([DPKG_ARCH], [dpkg-architecture])
AC_MSG_CHECKING([for multiarch paths])
multiarch_triplet=""
DEBIAN_MULTIARCH_DIRS=""
if test -n "$DPKG_ARCH"; then
  multiarch_triplet=$($DPKG_ARCH -qDEB_HOST_MULTIARCH 2> /dev/null)
  if test -z "$multiarch_triplet"; then
    multiarch_triplet=$($DPKG_ARCH -qDEB_HOST_GNU_TYPE)
  fi
fi
if test -n "$multiarch_triplet"; then
  DEBIAN_MULTIARCH_DIRS="/lib/$multiarch_triplet /usr/lib/$multiarch_triplet"
  AC_MSG_RESULT([$DEBIAN_MULTIARCH_DIRS])
else
  AC_MSG_RESULT([not found])
fi



###-------------------------------------------------------------------------
#
# check for libgcrypt
#
NEED_LIBGCRYPT_API=1
NEED_LIBGCRYPT_VERSION=1.2.0
AC_DEFINE_UNQUOTED(NEED_LIBGCRYPT_VERSION, "$NEED_LIBGCRYPT_VERSION",
                                     [Required version of Libgcrypt])
case "$OS_TYPE" in
  windows)
    LIBGCRYPT_CFLAGS=""
    LIBGCRYPT_LIBS="-lgcrypt"
    AC_SUBST(LIBGCRYPT_CFLAGS)
    AC_SUBST(LIBGCRYPT_LIBS)
    ;;
  *)
    AM_PATH_LIBGCRYPT("$NEED_LIBGCRYPT_API:$NEED_LIBGCRYPT_VERSION",
                      [have_gcrypt="yes"], [have_gcrypt="no"])
    if test "$have_gcrypt" != "yes"; then
      AC_MSG_ERROR([
    **** Libgcrypt is required for Gwenhywfar. Please install it (including devel packages)
    **** (at least version $NEED_LIBGCRYPT_VERSION using API $NEED_LIBGCRYPT_API is required.)])
    fi
    ;;
esac



###-------------------------------------------------------------------------
#
# check for library for common error values and messages in GnuPG components
#
AM_PATH_GPG_ERROR



###-------------------------------------------------------------------------
#
# check for libgnutls
#
case "$OS_TYPE" in
  windows)
    LIBGNUTLS_CFLAGS=""
    LIBGNUTLS_LIBS="-lgnutls -lgpg-error"
    # we use 1.6.3 or higher, so we do have gnutls_transport_set_error
    HAVE_GNUTLS_TRANSPORT_SET_ERRNO="yes"
    AC_SUBST(LIBGNUTLS_CFLAGS)
    AC_SUBST(LIBGNUTLS_LIBS)
    ;;
    
  *)
    PKG_CHECK_MODULES([LIBGNUTLS], [gnutls >= 2.9.8])
    # check for 1.6 or higher
    oldlibs="$LIBS"
    LIBS="${LIBGNUTLS_LIBS}"
    AC_CHECK_LIB(gnutls,gnutls_transport_set_errno, 
      [HAVE_GNUTLS_TRANSPORT_SET_ERRNO="yes"], 
      [HAVE_GNUTLS_TRANSPORT_SET_ERRNO="no"])
    LIBS="$oldlibs"
  ;;
esac
if test "x${HAVE_GNUTLS_TRANSPORT_SET_ERRNO}" = "xyes"; then
  AC_DEFINE(HAVE_GNUTLS_TRANSPORT_SET_ERRNO, 1, [only in gnutls>=1.6])
fi



###-------------------------------------------------------------------------
#
# check for OpenSSL
#
AQ_CHECK_SSL
if test "$ssl_available" = "yes"; then
  AC_DEFINE_UNQUOTED(HAVE_OPENSSL, 1, [Defined if OpenSSL is available])
  USE_CRYPT="yes"
else
  GWEN_CRYPTO="none"
  USE_CRYPT="no"
fi
AM_CONDITIONAL(IF_HAVE_OPENSSL, [test "$ssl_available" = "yes"])
AC_SUBST(ssl_available)



###-------------------------------------------------------------------------
# check for Distribution

AQ_DISTRIBUTION



###-------------------------------------------------------------------------
#
# check for I18N
#


# check whether i18n files are to be created
AC_CHECK_PROG(XGETTEXT,xgettext,xgettext, missing)

MAKE_I18N_FILES="yes"
if test "$XGETTEXT" = "missing"; then
  MAKE_I18N_FILES="no"
  AC_MSG_WARN([xgettext is missing. Locale suport is disabled.])
fi

AC_PATH_PROG([MSGFMT], [msgfmt], [missing])
if test "$MSGFMT" = "missing"; then
  MAKE_I18N_FILES="no"
  AC_MSG_WARN([msgfmt is missing. Locale suport is disabled.])
fi

AC_SUBST(MAKE_I18N_FILES)
AM_CONDITIONAL(IF_MAKE_I18N_FILES, [test "$MAKE_I18N_FILES" = "yes"])


# then check whether i18n support is to be compiled in
i18n_libs=""
HAVE_I18N="yes"
AC_CHECK_HEADERS([locale.h libintl.h], [], [HAVE_I18N="no"])

if test "$OS_TYPE" != "windows"; then
  oldlibs="$LIBS"
  LIBS=""
  AC_SEARCH_LIBS(gettext, intl, [], [HAVE_I18N="no"])
  i18n_libs="$LIBS"
  LIBS="$oldlibs"
else
  i18n_libs="-lintl"
fi

if test "$HAVE_I18N" != "NO"; then
  AC_DEFINE_UNQUOTED(HAVE_I18N, 1, [whether I18N is available])
fi

AC_SUBST(HAVE_I18N)
AC_SUBST(i18n_libs)



###-------------------------------------------------------------------------
#
# check for iconv
#

AC_ARG_WITH(iconv, [  --with-iconv=PATH take iconv from PATH],
  [ICONV_DIR="$withval"],
  [ICONV_DIR="/usr"])

if test "$ICONV_DIR" = "no" ; then
    echo Disabling ICONV support
else
    if test "$with_iconv" != "yes" -a "$with_iconv" != "" ; then
	CPPFLAGS="${CPPFLAGS} -I$with_iconv/include"
	# Export this since our headers include iconv.h
	ICONV_INCLUDES="-I$ICONV_DIR/include"
	ICONV_LIBS="-L$ICONV_DIR/lib"
    fi

    AC_CHECK_HEADER(iconv.h,
	AC_MSG_CHECKING(for iconv)
	AC_TRY_LINK([#include <stdlib.h>
#include <iconv.h>],[
iconv_t cd = iconv_open ("","");
iconv (cd, NULL, NULL, NULL, NULL);],[
	    AC_MSG_RESULT(yes)
	    WITH_ICONV=1],[
	    AC_MSG_RESULT(no)
	    AC_MSG_CHECKING(for iconv in -liconv)

	    _ldflags="${LDFLAGS}"
	    _libs="${LIBS}"
	    LDFLAGS="${LDFLAGS} ${ICONV_LIBS}"
	    LIBS="${LIBS} -liconv"

	    AC_TRY_LINK([#include <stdlib.h>
#include <iconv.h>],[
iconv_t cd = iconv_open ("","");
iconv (cd, NULL, NULL, NULL, NULL);],[
		AC_MSG_RESULT(yes)
		WITH_ICONV=1
		ICONV_LIBS="${ICONV_LIBS} -liconv"
		LIBS="${_libs}"
		LDFLAGS="${_ldflags}"],[
		AC_MSG_RESULT(no)
		LIBS="${_libs}"
		LDFLAGS="${_ldflags}"])]))
fi
AC_DEFINE_UNQUOTED(HAVE_ICONV, $WITH_ICONV, [Defined if libiconv is available])
AC_SUBST(WITH_ICONV)
AC_SUBST(ICONV_LIBS)



###-------------------------------------------------------------------------
#
# guis
#

AC_ARG_WITH(guis, 
  [  --with-guis=list      space separated list of GUI implementations (default:all)],
  [gwenhywfar_guis="$withval"],
  [gwenhywfar_guis="qt4 gtk2"]
)
if test "$gwenhywfar_guis" = "none"; then
  gwenhywfar_guis=""
fi
if test "$gwenhywfar_guis" = "yes"; then
  gwenhywfar_guis=""
fi
AC_SUBST(gwenhywfar_guis)




###-------------------------------------------------------------------------
# Check for fox toolkit
#


case "$gwenhywfar_guis" in 
  *fox16*)
     AQ_CHECK_FOX
     if test "$have_fox" != "yes"; then
       AC_MSG_ERROR([
**** FOX Toolkit missing.
**** If you want to use the FOX-GUI you will have to install the package
**** libfox-1.6 and libfox-1.6-devel on Debian-like systems.
**** On RPM systems like SuSE those packages might have different names.
**** If you don't need the FOX-GUI you might want to use the option
**** --with-guis])
     fi
     AC_DEFINE(BUILD_GUI_FOX16, 1, [if gwen gui for fox16 is built])
     with_gui_fox16="yes"
     ;;
esac

AC_SUBST(with_gui_fox16)
AM_CONDITIONAL(WITH_GUI_FOX16, [test "$with_gui_fox16" = "yes"])



###-------------------------------------------------------------------------
# Check for GTK+
#


case "$gwenhywfar_guis" in 
  *gtk2*)
     # We need at least GTK 2.17.5 because we use functions like
     # gtk_widget_get_sensitive() and gtk_widget_has_focus()
     pkg_modules="gtk+-2.0 >= 2.17.5"
     dnl $4=true => don't die in case of failure.
     PKG_CHECK_MODULES(GTK2, [$pkg_modules], [], true)
     AC_DEFINE(BUILD_GUI_GTK2, 1, [if gwen gui for gtk2 is built])
     with_gui_gtk2="yes"
     ;;
esac

AC_SUBST(with_gui_gtk2)
AM_CONDITIONAL(WITH_GUI_GTK2, [test "$with_gui_gtk2" = "yes"])



###-------------------------------------------------------------------------
#
# QT4
#

UIC4="NO"
QT4_BINDIR=""

case "$gwenhywfar_guis" in 
  *qt4*)
    AQ_CHECK_QT4
    if test "$have_qt4" = "yes"; then
      # Define this macro to disable the cast from QString to const char
      # which implicitly uses latin1()!
      AC_DEFINE(QT_NO_ASCII_CAST,[1],[Disable the cast from QString to const char.])
    fi
    
    dnl windows/mingw doesnt have the yes program, so add this emulation
    AC_CHECK_PROG(YES, yes, yes, [[echo -e 'y\ny\ny\ny\n']])
    
    dnl Store the bin dir for the windows dlls
    QT4_BINDIR="` echo ${qt4_moc} | ${SED} 's-/moc--' | ${SED} 's-/c/-c:\\\\-' | ${SED} 's-/-\\\\-g' `"
    UIC4="${qt4_uic}"

    AC_DEFINE(BUILD_GUI_QT4, 1, [if gwen gui for qt4 is built])
    with_gui_qt4="yes"
    ;;
  *)
    ;;
esac

dnl Store the bin dir for the windows dlls
AC_SUBST(QT4_BINDIR)
AC_SUBST(with_gui_qt4)
AM_CONDITIONAL(WITH_GUI_QT4, [test "$with_gui_qt4" = "yes"])



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
AC_ARG_ENABLE(visibility,
  [  --enable-visibility             enable visibility (default=no)],
  enable_visibility="$enableval",
  enable_visibility="yes")
AC_MSG_RESULT($enable_visibility)

if test "$enable_visibility" = "yes"; then
  AC_MSG_CHECKING(if symbol visibility is supported)
  visibility_cflags=""
  case `$CC --version|head -n1|cut -f1 -d " "` in 
    *gcc*)
       dnl Determine version number (watch out: the outside square
       dnl parenteses are vitally important here!)
       gccversion=["` ${CC} -dumpversion `"]
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
else
  visibility_supported="no (not wanted)"
  visibility_cflags=""
fi


AC_MSG_RESULT(${visibility_supported})
AC_SUBST(visibility_cflags)



###-------------------------------------------------------------------------
#
# enable export
#
AC_MSG_CHECKING(if symbol visibility is supported)
AC_ARG_ENABLE(export,
  [  --enable-export             enable export (default=yes)],
  enable_export="$enableval",
  enable_export="yes")
AC_MSG_RESULT($enable_export)

gwen_enable_export=0
if test "$enable_export" = "yes"; then
  gwen_enable_export=1
fi
AC_SUBST(gwen_enable_export)



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
# check buffer hard limit
#
AC_MSG_CHECKING(default buffer hard limit)
AC_ARG_WITH(buffer-hardlimit, [  --with-buffer-hardlimit=VALUE new default buffer hard limit],
  [gwenhywfar_buffer_hardlimit="$withval"],
  [gwenhywfar_buffer_hardlimit=""])

if test -z "${gwenhywfar_buffer_hardlimit}"; then
  gwenhywfar_buffer_hardlimit="(16*1024*1024)"
fi

AC_SUBST(gwenhywfar_buffer_hardlimit)
AC_MSG_RESULT($gwenhywfar_buffer_hardlimit)


###-------------------------------------------------------------------------
#
# compile program flags
#

PROG_LIBRARIES="${PROG_LIBRARIES} ${ssl_libraries} ${ssl_lib} ${ncurses_libs}"

# Clean up include variables
AS_SCRUB_INCLUDE(ssl_includes)
AS_SCRUB_INCLUDE(gwenhywfar_includes)
AS_SCRUB_INCLUDE(all_includes)



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
	DOXYGEN_INPUT="listdoc.h src gui"
        DOXYGEN_DEFINE=""
else
	DOXYGEN_DEFINE="DOXYGEN_HIDE"
	DOXYGEN_INPUT="listdoc.h gwenhywfar4"
fi
AC_SUBST(DOXYGEN_INPUT)
AC_SUBST(DOXYGEN_DEFINE)
AC_MSG_RESULT($enable_fulldoc)



# Debug arguments for compilation
ACX_COMPILE_WARN()



###-------------------------------------------------------------------------
#
# substitute vars
#
AC_SUBST(GWEN_OS)
AC_SUBST(PROG_LIBRARIES)
AC_SUBST(all_includes)
AC_SUBST(all_libraries)
AC_SUBST(gwenhywfar_crypt_lib)
AC_SUBST(gwenhywfar_ldflags)
AC_SUBST(gwenhywfar_bindir)
AC_SUBST(gwenhywfar_libs)
AC_SUBST(gwenhywfar_internal_libname)
AC_SUBST(gwenhywfar_libspp)
AC_SUBST(gwenhywfar_libsui)
AC_SUBST(gwenhywfar_os_lib)
AC_SUBST(gwenhywfar_pkgincludedir)
AC_SUBST(gwenhywfar_plugindir)
AC_SUBST(gwenhywfar_headerdir_am)
AC_SUBST(gwenhywfar_headerdir)
AC_SUBST(gwenhywfar_symlinkdir)
AC_SUBST(gwenhywfar_datadir)



###-------------------------------------------------------------------------
#
AC_MSG_CHECKING(crypt token plugins)
AC_ARG_WITH(plugins-ct, [  --with-plugins-ct=LIST list of CT plugins to build],
  [plugins="$withval"],
  [plugins="all"])

if test "$plugins" = "all"; then
  plugins="ohbci";
fi

gwenhywfar_plugins_ct_dirs=""
gwenhywfar_plugins_ct_libs=""
for plugin in $plugins; do
  case "$plugin" in
    ohbci)
      gwenhywfar_plugins_ct_dirs="$gwenhywfar_plugins_ct_dirs ohbci"
      gwenhywfar_plugins_ct_libs="$gwenhywfar_plugins_ct_libs ohbci/libct_ohbci.la"
      AC_DEFINE(GWENHYWFAR_WITH_PLUGIN_CT_OHBCI, 1, [plugin availability])
      ;;
    *)
      AC_MSG_ERROR("ERROR: Unknown plugin \"$plugin\"")
      ;;
  esac
done

AC_SUBST(gwenhywfar_plugins_ct_dirs)
AC_SUBST(gwenhywfar_plugins_ct_libs)
AC_MSG_RESULT($plugins)



###-------------------------------------------------------------------------
#
AC_MSG_CHECKING(DBIO plugins)
AC_ARG_WITH(plugins-dbio, [  --with-plugins-dbio=LIST list of DBIO plugins to build],
  [plugins="$withval"],
  [plugins="all"])

if test "$plugins" = "all"; then
  plugins="csv olddb xmldb";
fi

gwenhywfar_plugins_dbio_dirs=""
gwenhywfar_plugins_dbio_libs=""
for plugin in $plugins; do
  case "$plugin" in
    csv)
      gwenhywfar_plugins_dbio_dirs="$gwenhywfar_plugins_dbio_dirs csv"
      gwenhywfar_plugins_dbio_libs="$gwenhywfar_plugins_dbio_libs csv/libdbio_csv.la"
      AC_DEFINE(GWENHYWFAR_WITH_PLUGIN_DBIO_CSV, 1, [plugin availability])
      ;;
    olddb)
      gwenhywfar_plugins_dbio_dirs="$gwenhywfar_plugins_dbio_dirs olddb"
      gwenhywfar_plugins_dbio_libs="$gwenhywfar_plugins_dbio_libs olddb/libdbio_olddb.la"
      AC_DEFINE(GWENHYWFAR_WITH_PLUGIN_DBIO_OLDDB, 1, [plugin availability])
      ;;
    xmldb)
      gwenhywfar_plugins_dbio_dirs="$gwenhywfar_plugins_dbio_dirs xmldb"
      gwenhywfar_plugins_dbio_libs="$gwenhywfar_plugins_dbio_libs xmldb/libdbio_xmldb.la"
      AC_DEFINE(GWENHYWFAR_WITH_PLUGIN_DBIO_XMLDB, 1, [plugin availability])
      ;;
    *)
      AC_MSG_ERROR("ERROR: Unknown plugin \"$plugin\"")
      ;;
  esac
done

AC_SUBST(gwenhywfar_plugins_dbio_dirs)
AC_SUBST(gwenhywfar_plugins_dbio_libs)
AC_MSG_RESULT($plugins)



###-------------------------------------------------------------------------
#
AC_MSG_CHECKING(ConfigManager plugins)
AC_ARG_WITH(plugins-cfgmgr, [  --with-plugins-cfgmgr=LIST list of ConfigManager plugins to build],
  [plugins="$withval"],
  [plugins="all"])

if test "$plugins" = "all"; then
  plugins="dir";
fi

gwenhywfar_plugins_cfgmgr_dirs=""
gwenhywfar_plugins_cfgmgr_libs=""
for plugin in $plugins; do
  case "$plugin" in
    dir)
      gwenhywfar_plugins_cfgmgr_dirs="$gwenhywfar_plugins_cfgmgr_dirs dir"
      gwenhywfar_plugins_cfgmgr_libs="$gwenhywfar_plugins_cfgmgr_libs dir/libcfgmgr_dir.la"
      AC_DEFINE(GWENHYWFAR_WITH_PLUGIN_CFGMGR_DIR, 1, [plugin availability])
      ;;
    *)
      AC_MSG_ERROR("ERROR: Unknown plugin \"$plugin\"")
      ;;
  esac
done

AC_SUBST(gwenhywfar_plugins_cfgmgr_dirs)
AC_SUBST(gwenhywfar_plugins_cfgmgr_libs)
AC_MSG_RESULT($plugins)



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
                 gwenhywfar-config.cmake
                 gwenhywfar-config-version.cmake
                 gwenhywfar.spec
                 gwenhywfar.iss
                 checks/Makefile
                 data/Makefile
                 doc/Makefile
                 doc/dialog/Makefile
                 doc/dialog/images/Makefile
                 doc/inherit/Makefile
		 doc/plugindescr.xsd
                 src/Makefile
                 src/types.h
                 src/ressource.rc
                 src/version.h
                 src/base/Makefile
                 src/crypt3/Makefile
                 src/cryptmsg/Makefile
                 src/crypttoken/Makefile
                 src/gui/Makefile
                 src/sio/Makefile
                 src/html/Makefile
                 src/os/Makefile
                 src/os/posix/Makefile
                 src/os/posix/system.h
                 src/os/windows/Makefile
                 src/os/windows/system.h
                 src/parser/Makefile
                 src/gparser/Makefile
                 src/sar/Makefile
                 plugins/Makefile
                 plugins/dbio/Makefile
                 plugins/dbio/csv/Makefile
                 plugins/dbio/csv/csv.xml
                 plugins/dbio/csv/ressource.rc
                 plugins/dbio/olddb/Makefile
                 plugins/dbio/olddb/olddb.xml
                 plugins/dbio/olddb/ressource.rc
                 plugins/dbio/xmldb/Makefile
                 plugins/dbio/xmldb/xmldb.xml
                 plugins/dbio/xmldb/ressource.rc
                 plugins/ct/Makefile
                 plugins/ct/ohbci/Makefile
                 plugins/ct/ohbci/ohbci.xml
                 plugins/ct/ohbci/ressource.rc
                 plugins/configmgr/Makefile
                 plugins/configmgr/dir/Makefile
                 plugins/configmgr/dir/dir.xml
                 tools/Makefile
                 tools/xmlmerge/Makefile
                 tools/typemaker/Makefile
                 tools/typemaker2/Makefile
                 tools/typemaker2/types/Makefile
                 tools/typemaker2/types/c/Makefile
                 tools/gcttool/Makefile
                 tools/gsa/Makefile
                 test/Makefile
                 gui/Makefile
                 gui/cpp/Makefile
                 gui/qt4/Makefile
                 gui/qt4/gwengui-qt4.pc
                 gui/qt4/gwengui-qt4-config.cmake
                 gui/qt4/gwengui-qt4-config-version.cmake
                 gui/fox16/Makefile
                 gui/fox16/gwengui-fox16.pc
                 gui/gtk2/Makefile
                 gui/gtk2/gwengui-gtk2.pc
                 gui/cocoa/Makefile
                 gui/testdialogs/Makefile
		])
AC_OUTPUT




###-------------------------------------------------------------------------
#
# create header directory, make symlinks
#
HEADER_DIRS="src src/base src/crypt3 src/cryptmsg src/crypttoken src/sio src/os src/os/${GWEN_OS} src/gui src/html"
HEADER_DIRS="${HEADER_DIRS} src/parser src/gparser src/sar"
lprefix="${srcdir}"
lprefix=`( cd ${lprefix} ; pwd )`

rm -Rf gwenhywfar4 2>/dev/null
mkdir -p gwenhywfar4/gwenhywfar
for d in ${HEADER_DIRS}; do
  hfiles=`( cd "${srcdir}/${d}" && ls *.h 2>/dev/null )`
  for f in ${hfiles}; do
    case ${f} in
      *_l.h | *_p.h)
      	 ;;
      *)
    	 ln -s "${lprefix}/${d}/${f}" "gwenhywfar4/gwenhywfar/${f}"
         ;;
    esac
  done
done
#ln -s "${lprefix}/src/os/$OS_TYPE/system.h" "gwenhywfar4/gwenhywfar/system.h" 2>/dev/null



# cpp
#
rm -Rf gwenhywfar4/gwen-gui-cpp 2>/dev/null
mkdir -p gwenhywfar4/gwen-gui-cpp
for d in gui/cpp; do
  hfiles=`( cd "${srcdir}/${d}" && ls *.hpp *.h 2>/dev/null )`
  for f in ${hfiles}; do
    case ${f} in
      *_l.hpp | *_p.hpp)
      	 ;;
      *)
    	 ln -s "${lprefix}/${d}/${f}" "gwenhywfar4/gwen-gui-cpp/${f}"
         ;;
    esac
  done
done



# qt4
#
rm -Rf gwenhywfar4/gwen-gui-qt4 2>/dev/null
mkdir -p gwenhywfar4/gwen-gui-qt4
for d in gui/qt4; do
  hfiles=`( cd "${srcdir}/${d}" && ls *.hpp *.h 2>/dev/null )`
  for f in ${hfiles}; do
    case ${f} in
      *_l.hpp | *_p.hpp | qt4_gui_*.hpp)
      	 ;;
      *)
    	 ln -s "${lprefix}/${d}/${f}" "gwenhywfar4/gwen-gui-qt4/${f}"
         ;;
    esac
  done
done


# fox16
#
rm -Rf gwenhywfar4/gwen-gui-fox16 2>/dev/null
mkdir -p gwenhywfar4/gwen-gui-fox16
for d in gui/fox16; do
  hfiles=`( cd "${srcdir}/${d}" && ls *.hpp *.h 2>/dev/null )`
  for f in ${hfiles}; do
    case ${f} in
      *_l.hpp | *_p.hpp | fox16_gui_*.hpp)
      	 ;;
      *)
    	 ln -s "${lprefix}/${d}/${f}" "gwenhywfar4/gwen-gui-fox16/${f}"
         ;;
    esac
  done
done


# gtk2
#
rm -Rf gwenhywfar4/gwen-gui-gtk2 2>/dev/null
mkdir -p gwenhywfar4/gwen-gui-gtk2
for d in gui/gtk2; do
  hfiles=`( cd "${srcdir}/${d}" && ls *.h 2>/dev/null )`
  for f in ${hfiles}; do
    case ${f} in
      *_l.h | *_p.h | gtk2_gui_*.h)
      	 ;;
      *)
    	 ln -s "${lprefix}/${d}/${f}" "gwenhywfar4/gwen-gui-gtk2/${f}"
         ;;
    esac
  done
done



# cocoa
#
rm -Rf gwenhywfar4/gwen-gui-cocoa 2>/dev/null
mkdir -p gwenhywfar4/gwen-gui-cocoa
for d in gui/cocoa; do
  hfiles=`( cd "${srcdir}/${d}" && ls *.h 2>/dev/null )`
  for f in ${hfiles}; do
    case ${f} in
      *_l.h | *_p.h | gtk2_gui_*.h)
      	 ;;
      *)
    	 ln -s "${lprefix}/${d}/${f}" "gwenhywfar4/gwen-gui-cocoa/${f}"
         ;;
    esac
  done
done




###-------------------------------------------------------------------------
#
# summary
#

echo
echo "Summary"
echo "============================================="
echo "Version                       : $GWENHYWFAR_VERSION_FULL_STRING"
echo "System                        : $OSYSTEM"
echo "Installation Folder           : $prefix"
echo "Local Installation Mode       : $enable_local_install"
echo "Plugin Installation Folder    : $gwenhywfar_plugindir"
echo "Plugin Search Folder          : $gwenhywfar_plugin_searchdir"
echo "Data Search Folder            : $gwenhywfar_data_searchdir"
echo "Locale Search Folder          : $gwenhywfar_locale_searchdir"
echo "Config Search Folder          : $gwenhywfar_cfg_searchdir"
echo "Buffer Default Hard Limit     : $gwenhywfar_buffer_hardlimit"
echo "Symbol Visibility             : $visibility_supported"
echo "OpenSSL Support for gct-tool  : $ssl_available"
echo "GUI Support                   : $gwenhywfar_guis"

if test "$HAVE_GNUTLS_TRANSPORT_SET_ERRNO" = "yes"; then
  echo "GNUTLS                        : 1.6.1 or higher"
else
  echo "GNUTLS                        : pre-1.6.1"
fi

echo
echo "Plugins"
echo "============================================="
echo "  Crypt token plugins         :$gwenhywfar_plugins_ct_dirs"
echo "  DBIO plugins                :$gwenhywfar_plugins_dbio_dirs"
echo "  ConfigManager plugins       :$gwenhywfar_plugins_cfgmgr_dirs"

echo
echo "You can now use the command 'make' (or on *BSD 'gmake') to "
echo "build Gwenhywfar."
echo


