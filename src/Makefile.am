INCLUDES=@all_includes@ -Ios/@GWEN_OS@

SUBDIRS=base crypt io ipc os parser

EXTRA_DIST=\
 types.h.in \
 version.h.in \
 Makefile.palm.in

lib_LTLIBRARIES=libgwenhywfar.la
iheaderdir=${includedir}/gwenhywfar
iheader_HEADERS=\
  gwenhywfar.h \
  gwenhywfarapi.h \
  types.h \
  version.h

libgwenhywfar_la_SOURCES=\
  gwenhywfar.c

libgwenhywfar_la_LIBADD=\
  base/libbase.la \
  crypt/libcrypt.la \
  io/libio.la \
  ipc/libipc.la \
  os/libos.la \
  parser/libparser.la @all_libraries@
libgwenhywfar_la_LDFLAGS= \
  -version-info \
   @GWENHYWFAR_SO_CURRENT@:@GWENHYWFAR_SO_REVISION@:@GWENHYWFAR_SO_AGE@


all-local: @GWEN_MAKE_DLL_TARGET@

install-exec-local: @GWEN_INSTALL_DLL_TARGET@


# dll stuff
DLLTOOL=dlltool
DLLWRAP=dllwrap
WINDRES=windres

DLL_NAME=gwenhywfar32.dll
DLL_EXP_LIB=gwenhywfar32.lib
DLL_EXP_DEF=gwenhywfar32.def
DLL_OBJS= \
   base/stringlist.o \
   base/buffer.o\
   base/error.o\
   base/list.o\
   base/logger.o\
   io/bufferedio.o\
   crypt/ossl/cryptssl.o \
   crypt/ossl/cryptssl_rsa.o \
   crypt/ossl/cryptssl_rmd160.o\
   crypt/ossl/cryptssl_des.o\
   crypt/md.o\
   crypt/keymanager.o\
   crypt/keyspec.o\
   crypt/crypt.o\
   ipc/transportlayertcp.o\
   ipc/connlayer.o\
   ipc/ipc.o\
   ipc/msglayer.o\
   ipc/transportlayer.o\
   ipc/servicelayer.o\
   ipc/transportlayersocket.o\
   ipc/transportlayerunix.o\
   os/windows/directory.o\
   os/windows/inetaddr.o\
   os/windows/inetsocket.o\
   os/windows/init.o \
   os/windows/libloader.o\
   os/directory_all.o\
   os/windows/process.o\
   parser/db.o\
   parser/csv.o \
   parser/path.o\
   parser/msgengine.o\
   parser/text.o\
   parser/xml.o\
   gwenhywfar.o


DLL_LDLIBS=-lstdc++ -L/c/windows -llibeay32 -llibssl32 -lwsock32
DLL_WRAP_FLAGS=--export-all --output-def $(DLL_EXP_DEF) \
	--implib $(DLL_EXP_LIB) \
	--driver-name $(CXX)


$(DLL_NAME) $(DLL_EXP_LIB) $(DLL_EXP_DEF): $(DLL_OBJS) libgwenhywfar.la ressource.o exports.o
	rm -f $(DLL_NAME) $(DLL_EXP_LIB) $(DLL_EXP_DEF) >/dev/null
	$(DLLWRAP) $(DLL_WRAP_FLAGS) -o $(DLL_NAME) \
	    $(DLL_OBJS) ressource.o exports.o $(DLL_LDFLAGS) $(DLL_LDLIBS)

dll: $(DLL_NAME)

dll-install: dll
	$(INSTALL) -D $(DLL_NAME) $(DESTDIR)/c/windows/$(DLL_NAME)

ressource.o: ressource.rc
	$(WINDRES) -i $< -o $@

exports.o: libgwenhywfar.la $(DLL_OBJS)
	$(DLLTOOL) -e exports.o $(DLL_OBJS)

