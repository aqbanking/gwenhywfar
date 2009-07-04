SUBDIRS=types

INCLUDES=@all_includes@ -I$(gwenhywfar_symlinkdir)


typefiles=\
  tm_item.xml \
  tm_enum.xml \
  tm_define.xml


bin_PROGRAMS=typemaker2
noinst_HEADERS=\
  tm_type_p.h tm_type.h \
  tm_member_p.h tm_member.h \
  tm_header_p.h tm_header.h \
  tm_typemanager_p.h tm_typemanager.h \
  tm_builder_p.h tm_builder.h \
  tm_item_p.h tm_item.h \
  tm_enum_p.h tm_enum.h \
  tm_util.h \
  typemaker2.h \
  tm_builder_c.h

typemaker2_SOURCES=\
  tm_type.c \
  tm_member.c \
  tm_header.c \
  tm_util.c \
  tm_typemanager.c \
  tm_builder.c \
  tm_builder_c.c \
  tm_item.c \
  tm_enum.c \
  build.c \
  main.c
  
#  tm_enum.c

typemaker2_LDADD=$(top_builddir)/src/$(gwenhywfar_internal_libname)


if IS_WINDOWS

typemaker2_LDFLAGS=-mconsole
typemaker2_LDADD+= $(PROG_LIBRARIES)

endif

EXTRA_DIST=$(typefiles)

#BUILT_SOURCES=



sources:
	for f in $(typemaker2_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

indent:  $(BUILT_SOURCES)
	astyle \
	  --style=gnu \
	  --indent=spaces=2 \
	  --min-conditional-indent=0 \
	  --indent-labels \
	  --pad=oper \
	  --one-line=keep-blocks \
	  --suffix=none \
	  -a -y $(BUILT_SOURCES)

types: $(typefiles)
	./typemaker2 -I types/c -D GWENHYWFAR_API test.xml


tm_item.h tm_item.c tm_item_p.h: tm_item.xml
	./typemaker2 -I types/c -I . tm_item.xml

tm_enum.h tm_enum.c tm_enum_p.h: tm_enum.xml
	./typemaker2 -I types/c -I $(srcdir) tm_enum.xml

tm_define.h tm_define.c tm_define_p.h: tm_define.xml
	./typemaker2 -I types/c -I $(srcdir) tm_define.xml



