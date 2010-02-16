INCLUDES= -I$(gwenhywfar_symlinkdir) \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/parser @all_includes@

AM_CFLAGS=@visibility_cflags@ -DBUILDING_GWENHYWFAR

noinst_LTLIBRARIES=libgui.la

noinst_HEADERS=\
 cgui_p.h \
 cprogress_l.h cprogress_p.h \
 dialog_p.h \
 gui_p.h cgui_l.h \
 widget_l.h widget_p.h \
 progressdata_l.h progressdata_p.h \
 dlg_progress_l.h dlg_progress_p.h

iheaderdir=$(gwenhywfar_headerdir_am)/gwenhywfar
iheader_HEADERS=\
 cgui.h \
 dialog.h dialog_be.h \
 gui.h gui_be.h \
 widget_be.h

libgui_la_SOURCES=\
 cgui.c \
 cprogress.c \
 dialog.c \
 gui.c \
 widget.c \
 progressdata.c \
 dlg_progress.c

dlgdatadir=$(pkgdatadir)/dialogs
dlgdata_DATA=\
  dlg_progress.dlg


sources:
	for f in $(libgui_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

