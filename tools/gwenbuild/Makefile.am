SUBDIRS=parser


INCLUDES=@all_includes@ -I$(top_builddir) -I../ -I$(gwenhywfar_symlinkdir)



bin_PROGRAMS=gwbuild



gwbuild_SOURCES=\
  main.c \
  builder.c \
  context.c \
  file.c \
  gwenbuild.c \
  keyvaluepair.c \
  target.c \
  project.c



noinst_HEADERS= \
  builder.h \
  builder_p.h \
  context.h \
  context_p.h \
  file.h \
  file_p.h \
  gwenbuild.h \
  gwenbuild_p.h \
  keyvaluepair.h \
  keyvaluepair_p.h \
  target.h \
  target_p.h \
  project.h \
  project_p.h



gwbuild_LDADD = \
  $(top_builddir)/src/$(gwenhywfar_internal_libname) \
  $(PROG_LIBRARIES)
