INCLUDES= -I$(top_builddir)/gwenhywfar3 $(GPG_ERROR_CFLAGS)

EXTRA_DIST=README

noinst_LTLIBRARIES=libinherit.la

libinherit_la_SOURCES=baseclass.c derivedclass.c
noinst_HEADERS=baseclass.h baseclass_p.h derivedclass.h derivedclass_p.h
libinherit_la_LIBADD=$(top_builddir)/src/libgwenhywfar.la

noinst_PROGRAMS = testinherit
testinherit_SOURCES = testinherit.c
testinherit_LDADD = libinherit.la $(top_builddir)/src/libgwenhywfar.la \
  $(GPG_ERROR_LIBS)
TESTS = testinherit
