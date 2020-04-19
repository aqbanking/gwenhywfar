AM_CPPFLAGS= -I$(gwenhywfar_symlinkdir) \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/os \
 -I$(top_srcdir)/src/parser @all_includes@

AM_CFLAGS=@visibility_cflags@ @WERROR@ -DBUILDING_GWENHYWFAR

noinst_LTLIBRARIES=libxmlcmd.la

noinst_HEADERS=\
 xmlcmd_p.h \
 xmlcmd_gxml_p.h



iheaderdir=$(gwenhywfar_headerdir_am)/gwenhywfar
iheader_HEADERS=\
 xmlcmd.h \
 xmlcmd_gxml.h \
 xmlcmd_gxml_todb.h \
 xmlcmd_gxml_fromdb.h


libxmlcmd_la_SOURCES=\
 xmlcmd.c \
 xmlcmd_gxml.c \
 xmlcmd_gxml_todb.c \
 xmlcmd_gxml_fromdb.c





if WITH_LIBXML2_CODE

AM_CPPFLAGS += $(LIBXML_CFLAGS)


noinst_HEADERS += \
 xmlcmd_lxml_p.h


iheader_HEADERS += \
  xmlcmd_lxml.h \
  xmlcmd_lxml_todb.h

libxmlcmd_la_SOURCES += \
  xmlcmd_lxml.c \
  xmlcmd_lxml_todb.c

endif # WITH_LIBXML2_CODE



sources:
	for f in $(libxmlcmd_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done


