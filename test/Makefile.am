INCLUDES=@all_includes@

if USE_NCURSES
libsui = -lgwenui
lgwen = -lgwenhywfar
else
libsui =
lgwen = -lgwenhywfar32_$(GWENHYWFAR_SO_EFFECTIVE)
endif

noinst_PROGRAMS=gwentest
gwentest_SOURCES=gwentest.c
gwentest_LDADD=\
  -L/mingw/lib -L../src $(lgwen) \
  -L../gwenui $(libsui)

EXTRA_DIST = README.MSVC gwentest.vcproj
