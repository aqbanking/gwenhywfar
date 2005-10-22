iheaderdir=${includedir}/gwenpp
iheader_HEADERS=\
  waitcallback.h

lib_LTLIBRARIES=libgwenpp.la

libgwenpp_la_SOURCES=\
  waitcallback.cpp

if IS_WINDOWS
  PROVIDED_GWEN_LIB=libgwenhywfar32_$(GWENHYWFAR_SO_EFFECTIVE).la
else
  PROVIDED_GWEN_LIB=libgwenhywfar.la
endif

libgwenpp_la_LDFLAGS= \
  @STRIPALL@ -version-info \
  @GWENPP_SO_CURRENT@:@GWENPP_SO_REVISION@:@GWENPP_SO_AGE@

libgwenpp_la_LIBADD=\
  -lstdc++ $(top_builddir)/src/$(PROVIDED_GWEN_LIB)

