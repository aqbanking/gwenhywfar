INCLUDES=@all_includes@

if USE_NCURSES
libsui = $(gwenhywfar_libsui)
else
libsui =
endif

noinst_PROGRAMS=gwentest
gwentest_SOURCES=gwentest.c
gwentest_LDADD=\
  -L/mingw/lib -L$(top_builddir)/src $(gwenhywfar_libs) \
  -L$(top_builddir)/gwenui $(libsui)

EXTRA_DIST = README.MSVC gwentest.vcproj
