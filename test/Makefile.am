INCLUDES=@all_includes@

if USE_NCURSES
libsui = -lgwenui
else
libsui =
endif

noinst_PROGRAMS=gwentest
gwentest_SOURCES=gwentest.c
gwentest_LDADD=\
  -L/mingw/lib -L../src -lgwenhywfar \
  -L../gwenui $(libsui)

EXTRA_DIST = README.MSVC gwentest.vcproj
