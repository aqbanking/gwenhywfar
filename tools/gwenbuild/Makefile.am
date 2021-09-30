SUBDIRS=types parser tools buildctx builders templates


INCLUDES=@all_includes@ -I$(top_builddir) -I../ -I$(top_srcdir)/tools -I$(gwenhywfar_symlinkdir)



bin_PROGRAMS=gwbuild
gwbuild_SOURCES=\
  main.c \
  utils.c \
  c_setup.c \
  c_prepare.c \
  c_build.c \
  c_install.c \
  c_clean.c \
  c_dist.c
  
noinst_HEADERS=\
  utils.h \
  c_setup.h \
  c_prepare.h \
  c_build.h \
  c_install.h \
  c_clean.h \
  c_dist.h \
  filenames.h


gwbuild_LDADD = \
  $(top_builddir)/src/$(gwenhywfar_internal_libname) \
  types/libgwb_types.la \
  parser/libgwb_parser.la \
  tools/libgwb_tools.la \
  builders/libgwb_builders.la \
  buildctx/libgwb_build.la \
  $(PROG_LIBRARIES)

