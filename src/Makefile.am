INCLUDES=@all_includes@ -Ios/@GWEN_OS@

SUBDIRS=base crypt io ipc os parser

EXTRA_DIST=\
 version.h.in

lib_LTLIBRARIES=libgwenhywfar.la
iheaderdir=${includedir}/gwenhywfar
iheader_HEADERS=\
  gwenhywfar.h \
  gwenhywfarapi.h \
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
   base/stringlist.lo \
   base/buffer.lo\
   base/error.lo\
   base/list.lo\
   base/logger.lo\
   io/bufferedio.lo\
   crypt/ossl/cryptssl.lo \
   crypt/ossl/cryptssl_rsa.lo \
   crypt/ossl/cryptssl_rmd160.lo\
   crypt/ossl/cryptssl_des.lo\
   crypt/md.lo\
   crypt/keymanager.lo\
   crypt/keyspec.lo\
   crypt/crypt.lo\
   ipc/transportlayertcp.lo\
   ipc/connlayer.lo\
   ipc/ipc.lo\
   ipc/msglayer.lo\
   ipc/transportlayer.lo\
   ipc/servicelayer.lo\
   ipc/transportlayersocket.lo\
   ipc/transportlayerunix.lo\
   os/windows/directory.lo\
   os/windows/inetaddr.lo\
   os/windows/inetsocket.lo\
   os/windows/init.lo \
   os/windows/libloader.lo\
   os/directory_all.lo\
   parser/db.lo\
   parser/csv.lo \
   parser/path.lo\
   parser/msgengine.lo\
   parser/text.lo\
   parser/xml.lo\
   gwenhywfar.lo


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

