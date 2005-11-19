INCLUDES= -I$(top_srcdir)/gwenhywfar2 \
 -I$(top_srcdir)/src/base \
 -I$(top_srcdir)/src/crypt \
 -I$(top_srcdir)/src/io \
 -I$(top_srcdir)/src/ipc \
 -I$(top_srcdir)/src/net2 \
 -I$(top_srcdir)/src/os \
 -I$(top_srcdir)/src/parser @all_includes@

noinst_PROGRAMS=gwentest
gwentest_SOURCES=gwentest.c
gwentest_LDADD=$(top_builddir)/src/libgwenhywfar.la

EXTRA_DIST = README.MSVC gwentest.vcproj

sources:
