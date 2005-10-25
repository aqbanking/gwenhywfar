iheaderdir=$(includedir)/gwenpp
iheader_HEADERS=\
  waitcallback.h

lib_LTLIBRARIES=libgwenpp.la

libgwenpp_la_SOURCES=\
  waitcallback.cpp

libgwenpp_la_LDFLAGS= \
  @STRIPALL@ -version-info \
  @GWENPP_SO_CURRENT@:@GWENPP_SO_REVISION@:@GWENPP_SO_AGE@

libgwenpp_la_LIBADD=\
  $(top_builddir)/src/$(gwenhywfar_internal_libname) \
  -lstdc++
