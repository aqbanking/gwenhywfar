INCLUDES=-I$(top_srcdir) \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/crypt \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/net \
 -I$(top_srcdir)/src/parser \
 -I$(top_srcdir)/src/ui @all_includes@

typefiles=crypttoken.xml


localedir=$(datadir)/locale
DEFS=-DLOCALEDIR=\"$(localedir)\" @DEFS@

iheaderdir=${includedir}/gwenhywfar
iheader_HEADERS=\
  crypttoken.h \
  crypttoken_context.h \
  crypttoken_cryptinfo.h \
  crypttoken_keyinfo.h \
  crypttoken_plugin.h \
  crypttoken_signinfo.h \
  crypttoken_user.h \
  ct_file.h

noinst_LTLIBRARIES=libcrypt2.la
noinst_HEADERS=\
 crypttoken_p.h \
  crypttoken_user_l.h \
  crypttoken_user_p.h \
  ct_file_p.h

libcrypt2_la_SOURCES=\
 crypttoken.c \
 crypttoken_user.c \
 ct_file.c

types: $(typefiles)
	typemaker -d GWENHYWFAR_API $(typefiles)

