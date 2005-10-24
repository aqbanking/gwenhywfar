iheaderdir=${includedir}/gwenpp
iheader_HEADERS=\
  waitcallback.h

lib_LTLIBRARIES=libgwenpp.la

libgwenpp_la_SOURCES=\
  waitcallback.cpp

if IS_WINDOWS
  PROVIDED_GWEN_LIB=$(top_builddir)/src/gwenhywfar32_$(GWENHYWFAR_SO_EFFECTIVE).dll
else
  PROVIDED_GWEN_LIB=$(top_builddir)/src/libgwenhywfar.la
endif

libgwenpp_la_LDFLAGS= \
  @STRIPALL@ -version-info \
  @GWENPP_SO_CURRENT@:@GWENPP_SO_REVISION@:@GWENPP_SO_AGE@

libgwenpp_la_LIBADD=\
  -lstdc++ $(PROVIDED_GWEN_LIB)

