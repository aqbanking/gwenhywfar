
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gwenhywfar/iomanager.h>
#include <gwenhywfar/io_socket.h>
#include <gwenhywfar/io_http.h>
#include <gwenhywfar/io_buffered.h>
#include <gwenhywfar/buffer.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>



static int check_io_http1() {
  GWEN_IO_LAYER *io;
  GWEN_IO_LAYER *baseLayer;
  int rv;
  GWEN_INETADDRESS *addr;
  GWEN_SOCKET *sk;
  char buffer[2048];
  GWEN_DB_NODE *db;
  int j;

  addr=GWEN_InetAddr_new(GWEN_AddressFamilyIP);
  rv=GWEN_InetAddr_SetName(addr, "devel.aqbanking.de");
  if (rv) {
    fprintf(stderr,
	    "ERROR in check_io_http1: Could not resolve hostname (%d)\n", rv);
    return 2;
  }
  rv=GWEN_InetAddr_SetPort(addr, 80);
  if (rv) {
    fprintf(stderr,
	    "ERROR in check_io_http1: Could not set port (%d)\n", rv);
    return 2;
  }

  sk=GWEN_Socket_new(GWEN_SocketTypeTCP);
  if (sk==NULL) {
    fprintf(stderr,
	    "ERROR in check_io_http1: Socket type not supported\n");
    return 2;
  }

  /* create socket layer */
  io=GWEN_Io_LayerSocket_new(sk);
  if (io==NULL) {
    fprintf(stderr,
	    "ERROR in check_io_http1: Could not create socket io layer.\n");
    return 2;
  }
  GWEN_Io_LayerSocket_SetPeerAddr(io, addr);

  baseLayer=io;

  io=GWEN_Io_LayerBuffered_new(baseLayer);
  if (io==NULL) {
    fprintf(stderr,
	    "ERROR in check_io_http1: Could not create HTTP io layer.\n");
    return 2;
  }

  baseLayer=io;

  io=GWEN_Io_LayerHttp_new(baseLayer);
  if (io==NULL) {
    fprintf(stderr,
	    "ERROR in check_io_http1: Could not create HTTP io layer.\n");
    return 2;
  }


  rv=GWEN_Io_Manager_RegisterLayer(io);
  if (rv) {
    fprintf(stderr,
	    "ERROR in check_io_http1: Could not register io layer.\n");
    GWEN_Io_Layer_free(io);
    return 2;
  }

  for (j=0; j<2; j++) {
    int firstRead=1;
    int bodySize=-1;
    int bytesRead=0;

    rv=GWEN_Io_Layer_ConnectRecursively(io, NULL, 0, 0, 10000);
    if (rv) {
      fprintf(stderr,
	      "ERROR in check_io_http1: Could not connect (%d)\n", rv);
      return 2;
    }

    db=GWEN_Io_LayerHttp_GetDbCommandOut(io);
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "command", "GET");
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "protocol", "HTTP/1.0");
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "url", "/index.html");

    db=GWEN_Io_LayerHttp_GetDbHeaderOut(io);
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "Host", "devel.aqbanking.de");
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "Content-length", 0);
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "Connection", "close");


    rv=GWEN_Io_Layer_WriteBytes(io, (uint8_t*)"", 0,
				GWEN_IO_REQUEST_FLAGS_PACKETBEGIN |
				GWEN_IO_REQUEST_FLAGS_PACKETEND |
				GWEN_IO_REQUEST_FLAGS_FLUSH,
				0, 10000);
    if (rv<0) {
      fprintf(stderr,
	      "ERROR in check_io_http1: Could not write (%d)\n", rv);
      return 2;
    }

    for (;;) {
      rv=GWEN_Io_Layer_ReadBytes(io, (uint8_t*)buffer, sizeof(buffer)-1,
				 firstRead?GWEN_IO_REQUEST_FLAGS_PACKETBEGIN:0,
				 0, 5000);
      if (rv==0)
	break;
      else if (rv<1) {
	if (rv==GWEN_ERROR_TIMEOUT || rv==GWEN_ERROR_EOF)
	  break;
	fprintf(stderr,
		"ERROR in check_io_http1: Could not read (%d)\n", rv);
	return 2;
      }
      else {
	buffer[rv]=0;
	bytesRead+=rv;
	if (firstRead) {
	  GWEN_DB_NODE *db;

	  db=GWEN_Io_LayerHttp_GetDbHeaderIn(io);
	  bodySize=GWEN_DB_GetIntValue(db, "Content-length", 0, -1);
	}
      }

      if (bodySize!=-1 && bytesRead>=bodySize) {
	break;
      }
      firstRead=0;
    }

    rv=GWEN_Io_Layer_DisconnectRecursively(io, NULL, 0, 0, 2000);
    if (rv) {
      fprintf(stderr,
	      "ERROR in check_io_http1: Could not disconnect (%d)\n", rv);
      return 2;
    }
  }

  return 0;
}



int check_io_http() {
  int errs=0;
  int chks=0;

  fprintf(stderr, "IO http checks... ");
  chks++;
  if (check_io_http1())
    errs++;

  if (chks) {
    if (chks==errs) {
      fprintf(stderr, "IO http checks: all failed.\n");
    }
    else if (errs) {
      fprintf(stderr, "IO http checks: some failed.\n");
    }
    else {
      fprintf(stderr, "passed.\n");
    }
  }

  return errs;
}





