INCLUDES=@all_includes@ -I$(top_builddir) -I../ -I$(gwenhywfar_symlinkdir)

bin_PROGRAMS=gwbuild

gwbuild_SOURCES=\
  main.c \
  context.c \
  file.c \
  generator.c \
  keyvaluepair.c


noinst_HEADERS= \
  context.h \
  context_p.h \
  file.h \
  file_p.h \
  generator.h \
  generator_p.h \
  keyvaluepair.h \
  keyvaluepair_p.h


gwbuild_LDADD = \
  $(top_builddir)/src/$(gwenhywfar_internal_libname) \
  $(PROG_LIBRARIES)
