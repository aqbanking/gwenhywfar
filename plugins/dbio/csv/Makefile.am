EXTRA_DIST=README ressource.rc.in

dbioplugindir = $(gwenhywfar_plugindir)/dbio

localedir=$(datadir)/locale
DEFS += -D LOCALEDIR=\"$(localedir)\"

dbioplugin_LTLIBRARIES=csv.la
dbioplugin_DATA=csv.xml
noinst_HEADERS=csv_p.h

csv_la_SOURCES=csv.c
csv_la_LIBADD=-L$(top_builddir)/src -lgwenhywfar
csv_la_LDFLAGS = $(STRIPALL) -module -version-info 0:1:0


# dll stuff
if IS_WINDOWS

# you only need to setup these variables here, the rest is done automatically
LIBRARY_NAME = csv
DLLLDLIBS = -L/mingw/lib -L$(top_builddir)/src -lgwenhywfar32_$(GWENHYWFAR_SO_EFFECTIVE)
DLLLDFLAGS = $(STRIPALL) $(LDFLAGS)


###------------------------------------------------------------------------
# You should not have to make any modifications below this point
#

DLLNAME = $(LIBRARY_NAME).dll
DLLEXP_LIB = $(LIBRARY_NAME).lib
DLLEXP_DEF = $(LIBRARY_NAME).def
DLLSRC_LIB = $(dbioplugin_LTLIBRARIES:.la=.a)
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
	$(INSTALL) -D $(DLLNAME) $(DESTDIR)$(dbioplugindir)/$(DLLNAME)

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


