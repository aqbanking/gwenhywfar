INCLUDES=@all_includes@

if USE_NCURSES
libsui = -lgwenui
else
libsui =
endif

noinst_PROGRAMS=gwentest
gwentest_SOURCES=gwentest.c
gwentest_LDADD=\
  -L/mingw/lib -L$(top_builddir)/src @gwenhywfar_lib@ \
  -L$(top_builddir)/gwenui $(libsui)

EXTRA_DIST = README.MSVC gwentest.vcproj
