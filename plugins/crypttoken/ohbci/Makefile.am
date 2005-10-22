EXTRA_DIST = ressource.rc.in

INCLUDES = \
 -I$(top_srcdir) \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/crypt \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/net \
 -I$(top_srcdir)/src/os \
 -I$(top_srcdir)/src/parser \
 $(all_includes)

AM_CFLAGS=@PROFILING_CFLAGS@

crypttokenplugindir = $(gwenhywfar_plugindir)/crypttoken

crypttokenplugin_LTLIBRARIES=ohbci.la
crypttokenplugin_DATA=ohbci.xml
noinst_HEADERS=ohbci_l.h ohbci_p.h tag16_l.h tag16_p.h

ohbci_la_SOURCES=ohbci.c tag16.c
ohbci_la_LIBADD=$(top_builddir)/src/libgwenhywfar.la
ohbci_la_LDFLAGS = $(STRIPALL) @PROFILING_LDFLAGS@ -module -version-info 0:1:0


# dll stuff
if IS_WINDOWS

# you only need to setup these variables here, the rest is done automatically
LIBRARY_NAME = ohbci
DLLLDLIBS = $(top_builddir)/src/libgwenhywfar32_$(GWENHYWFAR_SO_EFFECTIVE).la
DLLLDFLAGS = $(STRIPALL) $(LDFLAGS)


###------------------------------------------------------------------------
# You should not have to make any modifications below this point
#

DLLNAME = $(LIBRARY_NAME).dll
DLLEXP_LIB = $(LIBRARY_NAME).lib
DLLEXP_DEF = $(LIBRARY_NAME).def
DLLSRC_LIB = $(crypttokenplugin_LTLIBRARIES:.la=.a)
DLLSRC_LIBLA = $(lib_LTLIBRARIES)

all-local: dll
#@GWEN_MAKE_DLL_TARGET@

install-exec-local: dll-install
#@GWEN_INSTALL_DLL_TARGET@

DLLTOOL=dlltool
DLLWRAP=dllwrap
WINDRES=windres


dll: $(DLLNAME)

dll-install: dll
	$(INSTALL) -D $(DLLNAME) $(DESTDIR)$(crypttokenplugindir)/$(DLLNAME)

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


