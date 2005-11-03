INCLUDES= -I$(top_srcdir)/gwenhywfar2 \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/crypt \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/os \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/net2 \
 -I$(top_srcdir)/src/parser @all_includes@

SUBDIRS = base crypt io ipc os parser crypt2 net2

DEFS += \
  -DGWEN_INSTALL_DIR=\"$(prefix)\" \
  -DGWEN_EFFECTIVE_VERSTR=\"${GWENHYWFAR_SO_EFFECTIVE}\" \
  -DGWEN_SYSCONF_DIR=\"$(sysconfdir)\"


EXTRA_DIST=\
 types.h.in \
 version.h.in

iheaderdir=$(gwenhywfar_headerdir_am)/gwenhywfar
iheader_HEADERS=\
  gwenhywfar.h \
  gwenhywfarapi.h \
  types.h \
  version.h

lib_LTLIBRARIES=libgwenhywfar.la

libgwenhywfar_la_SOURCES=\
  gwenhywfar.c

libgwenhywfar_la_LIBADD=\
  base/libbase.la \
  crypt/libcrypt.la \
  crypt2/libcrypt2.la \
  io/libio.la \
  ipc/libipc.la \
  net2/libnet2.la \
  os/libos.la \
  parser/libparser.la $(ssl_libraries) $(ssl_lib) $(all_libraries)

libgwenhywfar_la_LDFLAGS= \
  -no-undefined $(STRIPALL) $(PROFILING_LDFLAGS) -version-info \
   $(GWENHYWFAR_SO_CURRENT):$(GWENHYWFAR_SO_REVISION):$(GWENHYWFAR_SO_AGE)

# Before compiling the sources, first built these files
BUILT_SOURCES = types.h version.h


# dll stuff
if IS_WINDOWS

# you only need to setup these variables here, the rest is done automatically
LIBRARY_NAME = gwenhywfar32_$(GWENHYWFAR_SO_EFFECTIVE)
DLLLDLIBS = $(ssl_libraries) $(ssl_lib) -lwsock32 -lregex
DLLLDFLAGS = $(STRIPALL) $(LDFLAGS)


###------------------------------------------------------------------------
# You should not have to make any modifications below this point
#

DLLNAME = $(LIBRARY_NAME).dll
DLLEXP_LIB = $(LIBRARY_NAME).lib
DLLEXP_DEF = $(LIBRARY_NAME).def
DLLSRC_LIB = $(lib_LTLIBRARIES:.la=.a)
DLLSRC_LIBLA = $(lib_LTLIBRARIES)

all-local: dll

install-exec-local: dll-install

DLLTOOL=dlltool
DLLWRAP=dllwrap
WINDRES=windres


dll: $(DLLNAME)

dll-install: dll
	$(INSTALL) -D $(DLLNAME) $(DESTDIR)$(WIN_PATH_WINDOWS_MINGW)/$(DLLNAME)

ressource.o: ressource.rc
	$(WINDRES) -i $< -o $@


$(DLLNAME): $(DLLSRC_LIBLA) ressource.o
	mkdir -p dlldir && \
	( \
	  if test -r $(DLLSRC_LIB); then \
	    SRCLIB=$(DLLSRC_LIB); \
	  else \
	    SRCLIB=.libs/$(DLLSRC_LIB); \
	  fi; \
	  ( cd dlldir && ar x "../$${SRCLIB}" && \
	   ( ls ../.libs/*.o && cp ../.libs/*.o .) ) && \
	 $(DLLTOOL) -e exports.o *.o && \
	 $(DLLWRAP) \
	   --export-all \
	   --output-def $(DLLEXP_DEF) \
	   --implib $(DLLEXP_LIB) \
	   --driver-name $(CC) \
	   -o $(DLLNAME) \
	     dlldir/*.o exports.o \
	     ressource.o $(DLLLDFLAGS) $(DLLLDLIBS) \
	) && rm -rf dlldir

CLEANFILES = $(DLLNAME) $(DLLEXP_LIB) $(DLLEXP_DEF)

endif
# IS_WINDOWS


