INCLUDES = $(all_includes) -I$(srcdir)/os/$(GWEN_OS)

SUBDIRS=base crypt io os parser net

EXTRA_DIST=\
 types.h.in \
 version.h.in

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
  net/libnet.la \
  os/libos.la \
  parser/libparser.la @all_libraries@
libgwenhywfar_la_LDFLAGS= \
  @STRIPALL@ -version-info \
   @GWENHYWFAR_SO_CURRENT@:@GWENHYWFAR_SO_REVISION@:@GWENHYWFAR_SO_AGE@

# Before compiling the sources, first built these files
BUILT_SOURCES = types.h version.h

all-local: @GWEN_MAKE_DLL_TARGET@

install-exec-local: @GWEN_INSTALL_DLL_TARGET@


# dll stuff
DLLTOOL=dlltool
DLLWRAP=dllwrap
WINDRES=windres

DLLNAME=gwenhywfar32.dll
DLLEXP_LIB=gwenhywfar32.lib
DLLEXP_DEF=gwenhywfar32.def

# Decide for object files for crypto module
GWEN_CRYPT_OSSL_OBJS = crypt/ossl/cryptssl.o \
   crypt/ossl/cryptssl_des.o \
   crypt/ossl/cryptssl_md5.o\
   crypt/ossl/cryptssl_rmd160.o\
   crypt/ossl/cryptssl_rsa.o 
GWEN_CRYPT_NONE_OBJS = crypt/none/cryptnone.o
if GWEN_SSL_CRYPTO
GWEN_CRYPT_OBJS = $(GWEN_CRYPT_OSSL_OBJS)
else
GWEN_CRYPT_OBJS = $(GWEN_CRYPT_NONE_OBJS)
endif

DLLOBJS= \
   base/buffer.o\
   base/debug.o\
   base/error.o\
   base/inherit.o \
   base/list.o \
   base/logger.o \
   base/memory.o \
   base/ringbuffer.o \
   base/stringlist.o \
   base/waitcallback.o \
   io/bufferedio.o\
   $(GWEN_CRYPT_OBJS) \
   crypt/md.o\
   crypt/keymanager.o\
   crypt/keyspec.o\
   crypt/crypt.o\
   crypt/padd.o \
   net/net.o \
   net/netconnection.o \
   net/netmsg.o \
   net/nettransport.o \
   net/nettransportsock.o\
   os/windows/directory.o\
   os/windows/inetaddr.o\
   os/windows/inetsocket.o\
   os/windows/init.o \
   os/windows/libloader.o\
   os/directory_all.o\
   os/inetsocket_all.o\
   os/windows/process.o\
   parser/db.o\
   parser/csv.o \
   parser/path.o\
   parser/msgengine.o\
   parser/text.o\
   parser/xml.o\
   gwenhywfar.o


DLLLDLIBS = -lstdc++ $(ssl_libraries) $(ssl_lib) -lwsock32
DLLLDFLAGS = $(STRIPALL) $(LDFLAGS)
DLLWRAP_FLAGS=--export-all --output-def $(DLLEXP_DEF) \
	--implib $(DLLEXP_LIB) \
	--driver-name $(CXX)


$(DLLNAME) $(DLLEXP_LIB) $(DLLEXP_DEF): $(DLLOBJS) libgwenhywfar.la ressource.o exports.o
	rm -f $(DLLNAME) $(DLLEXP_LIB) $(DLLEXP_DEF) >/dev/null
	$(DLLWRAP) $(DLLWRAP_FLAGS) -o $(DLLNAME) \
	    $(DLLOBJS) ressource.o exports.o $(DLLLDFLAGS) $(DLLLDLIBS)

if IS_WINDOWS
dll: $(DLLNAME)

dll-install: dll
	$(INSTALL) -D $(DLLNAME) $(DESTDIR)$(WIN_PATH_SYSTEM_MINGW)/$(DLLNAME)

ressource.o: ressource.rc
	$(WINDRES) -i $< -o $@

exports.o: libgwenhywfar.la $(DLLOBJS)
	$(DLLTOOL) -e exports.o $(DLLOBJS)

endif

CLEANFILES = $(DLLNAME) $(DLLEXP_LIB) $(DLLEXP_DEF)
