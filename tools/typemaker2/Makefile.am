SUBDIRS=c types

AM_CPPFLAGS=@all_includes@ -I$(gwenhywfar_symlinkdir) \
  -I$(srcdir)/c \
  -I$(builddir)/c
  


typefiles=\
  tm_item.xml \
  tm_enum.xml \
  tm_define.xml \
  tm_code.xml \
  tm_signal.xml \
  tm_slot.xml


bin_PROGRAMS=typemaker2
noinst_HEADERS=\
  tm_type_p.h tm_type.h \
  tm_member_p.h tm_member.h \
  tm_header_p.h tm_header.h \
  tm_typemanager_p.h tm_typemanager.h \
  tm_virtualfn_p.h tm_virtualfn.h \
  tm_builder_p.h tm_builder.h \
  tm_item_p.h tm_item.h \
  tm_enum_p.h tm_enum.h \
  tm_define_p.h tm_define.h \
  tm_code_p.h tm_code.h \
  tm_inline_p.h tm_inline.h \
  tm_signal_p.h tm_signal.h \
  tm_slot_p.h tm_slot.h \
  tm_group_p.h tm_group.h \
  tm_util.h \
  typemaker2.h

typemaker2_SOURCES=\
  tm_type.c \
  tm_member.c \
  tm_header.c \
  tm_util.c \
  tm_typemanager.c \
  tm_builder.c \
  tm_item.c \
  tm_enum.c \
  tm_define.c \
  tm_code.c \
  tm_inline.c \
  tm_virtualfn.c \
  tm_signal.c \
  tm_slot.c \
  tm_group.c \
  build.c \
  builddefs.c \
  main.c
  
#  tm_enum.c

typemaker2_LDADD=c/libtm2_c.la $(top_builddir)/src/$(gwenhywfar_internal_libname)


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
	$(ASTYLE) \
	  --style=gnu \
	  --indent=spaces=2 \
	  --min-conditional-indent=0 \
	  --indent-labels \
	  --pad=oper \
	  --one-line=keep-blocks \
	  --suffix=none \
	  -a -y $(BUILT_SOURCES)

types: $(typefiles)
	./typemaker2 -I types/c -I $(srcdir) $(typefiles)



