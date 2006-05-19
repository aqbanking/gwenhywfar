EXTRA_DIST= ressource.rc.in localfiles.xml.in

INCLUDES= -I$(top_srcdir)/gwenhywfar2 \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/crypt \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/net2 \
 -I$(top_srcdir)/src/os \
 -I$(top_srcdir)/src/parser \
 -I$(top_srcdir)/src/storage @all_includes@

AM_CFLAGS=$(visibility_cflags)


storageplugindir = $(gwenhywfar_plugindir)/storage

storageplugin_LTLIBRARIES=localfiles.la
storageplugin_DATA=localfiles.xml

noinst_LTLIBRARIES=liblocalfiles.la

noinst_HEADERS=\
  lf_object_l.h lf_object_p.h \
  lf_type_l.h lf_type_p.h \
  lf_storage_l.h lf_storage_p.h

liblocalfiles_la_SOURCES=\
  lf_object.c \
  lf_type.c \
  lf_storage.c

liblocalfiles_la_LIBADD=$(top_builddir)/src/libgwenhywfar.la
liblocalfiles_la_DEPENDENCIES=$(top_builddir)/src/libgwenhywfar.la

localfiles_la_SOURCES=plugin.c
localfiles_la_LIBADD=$(top_builddir)/src/libgwenhywfar.la liblocalfiles.la
localfiles_la_DEPENDENCIES=$(localfiles_la_LIBADD)
localfiles_la_LDFLAGS = -no-undefined $(STRIPALL) $(ressource_ldflags) \
    -module -version-info 0:1:0

noinst_PROGRAMS=testlib
testlib_SOURCES=testlib.c
testlib_LDADD=$(top_builddir)/src/libgwenhywfar.la liblocalfiles.la

# if IS_WINDOWS
if IS_WINDOWS
localfiles_la_DEPENDENCIES += ressource.o
ressource_ldflags=-Wl,ressource.o
endif
# IS_WINDOWS


sources:
	for f in $(localfiles_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

ressource.o: ressource.rc
	$(RC) -o $@ $<

