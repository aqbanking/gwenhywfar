INCLUDES=@all_includes@ -I$(top_srcdir) \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/crypt \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/net \
 -I$(top_srcdir)/src/os \
 -I$(top_srcdir)/src/parser \
 -I$(top_srcdir)/src/ui
 

EXTRA_DIST=README 

lib_LTLIBRARIES=libgwenui.la

iheaderdir=${includedir}/gwenhywfar/ui
iheader_HEADERS=\
  button.h \
  checkbox.h \
  dropdownbox.h \
  editbox.h \
  event.h \
  filedialog.h \
  hline.h \
  hslider.h \
  loader.h \
  messagebox.h \
  scrollwidget.h \
  tablewidget.h \
  textwidget.h \
  ui.h \
  vslider.h \
  widget.h \
  window.h

noinst_HEADERS=\
  button_p.h \
  checkbox_p.h \
  dropdownbox_p.h \
  editbox_p.h \
  event_l.h \
  event_p.h \
  filedialog_p.h \
  hline_p.h \
  hslider_p.h \
  loader_p.h \
  messagebox_p.h \
  scrollwidget_p.h \
  tablewidget_p.h \
  textwidget_p.h \
  ui_l.h \
  ui_p.h \
  vslider_p.h \
  widget_l.h \
  widget_p.h \
  window_p.h

libgwenui_la_SOURCES=\
  button.c \
  checkbox.c \
  dropdownbox.c \
  editbox.c \
  event.c \
  filedialog.c \
  hline.c \
  hslider.c \
  loader.c \
  messagebox.c \
  scrollwidget.c \
  tablewidget.c \
  textwidget.c \
  ui.c \
  vslider.c \
  widget.c \
  window.c

if IS_WINDOWS
  PROVIDED_GWEN_LIB=-lgwenhywfar32_$(GWENHYWFAR_SO_EFFECTIVE)
else
  PROVIDED_GWEN_LIB=-lgwenhywfar
endif

libgwenui_la_LIBADD=-L/mingw/lib -L$(top_builddir)/src $(PROVIDED_GWEN_LIB) @ncurses_libs@

libgwenui_la_LDFLAGS= \
  @STRIPALL@ -version-info \
   @GWENUI_SO_CURRENT@:@GWENUI_SO_REVISION@:@GWENUI_SO_AGE@







