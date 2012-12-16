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
 gui_l.h gui_p.h cgui_l.h \
 widget_l.h widget_p.h \
 progressdata_l.h progressdata_p.h \
 dlg_progress_l.h dlg_progress_p.h \
 dlg_input_l.h dlg_input_p.h \
 dlg_message_l.h dlg_message_p.h \
 dlg_showbox_l.h dlg_showbox_p.h \
 passwdstore_p.h

iheaderdir=$(gwenhywfar_headerdir_am)/gwenhywfar
iheader_HEADERS=\
 cgui.h \
 dialog.h dialog_be.h \
 gui.h gui_be.h \
 widget_be.h \
 passwdstore.h

libgui_la_SOURCES=\
 cgui.c \
 cprogress.c \
 dialog.c \
 gui.c \
 widget.c \
 progressdata.c \
 dlg_progress.c \
 dlg_input.c \
 dlg_message.c \
 dlg_showbox.c \
 passwdstore.c


dlgdatadir=$(pkgdatadir)/dialogs
dlgdata_DATA=\
  dlg_input.dlg \
  dlg_message.dlg \
  dlg_progress.dlg \
  dlg_showbox.dlg

EXTRA_DIST=$(dlgdata_DATA)


sources:
	for f in $(libgui_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(dlgdata_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/dlgsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

