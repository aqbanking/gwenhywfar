INCLUDES=@all_includes@ -Ios/@GWEN_OS@

SUBDIRS=base crypt io ipc os parser impl

EXTRA_DIST=\
 version.h.in

lib_LTLIBRARIES=libgwenhyfwar.la
iheaderdir=${includedir}/gwenhyfwar
iheader_HEADERS=\
  gwenhyfwar.h \
  gwenhyfwarapi.h \
  version.h

libgwenhyfwar_la_SOURCES=\
  gwenhyfwar.c

libgwenhyfwar_la_LIBADD=\
  base/libbase.la \
  crypt/libcrypt.la \
  io/libio.la \
  ipc/libipc.la \
  @gwenhyfwar_os_lib@ \
  parser/libparser.la \
  impl/ipc/libimplipc.la
libgwenhyfwar_la_LDFLAGS= \
  -version-info \
   @GWENHYFWAR_SO_CURRENT@:@GWENHYFWAR_SO_REVISION@:@GWENHYFWAR_SO_AGE@

