INCLUDES=@all_includes@ -Ios/@GWEN_OS@

SUBDIRS=base crypt io ipc os parser impl

EXTRA_DIST=\
 version.h.in

lib_LTLIBRARIES=libgwenhywfar.la
iheaderdir=${includedir}/gwenhywfar
iheader_HEADERS=\
  gwenhywfar.h \
  gwenhywfarapi.h \
  version.h

libgwenhywfar_la_SOURCES=\
  gwenhywfar.c

libgwenhywfar_la_LIBADD=\
  base/libbase.la \
  crypt/libcrypt.la \
  io/libio.la \
  ipc/libipc.la \
  os/libos.la \
  parser/libparser.la \
  impl/ipc/libimplipc.la \
  impl/msgengine/libimplmsg.la  @all_libraries@
libgwenhywfar_la_LDFLAGS= \
  -version-info \
   @GWENHYFWAR_SO_CURRENT@:@GWENHYFWAR_SO_REVISION@:@GWENHYFWAR_SO_AGE@

