iheaderdir=${includedir}/gwenpp
iheader_HEADERS=\
  waitcallback.h

lib_LTLIBRARIES=libgwenpp.la

libgwenpp_la_SOURCES=\
  waitcallback.cpp

libgwenpp_la_LDFLAGS= \
  @STRIPALL@ -version-info \
  @GWENHYWFAR_SO_CURRENT@:@GWENHYWFAR_SO_REVISION@:@GWENHYWFAR_SO_AGE@

libgwenpp_la_LIBADD=\
  -lstdc++ -L/mingw/lib -L../src @gwenhywfar_libs@

