
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gwenhywfar/syncio_socket.h>
#include <gwenhywfar/syncio_http.h>
#include <gwenhywfar/syncio_tls.h>
#include <gwenhywfar/syncio_buffered.h>
#include <gwenhywfar/buffer.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>



static int check_syncio_https1() {
  GWEN_SYNCIO *sio;
  GWEN_SYNCIO *baseLayer;
  int rv;
  GWEN_DB_NODE *db;
  int j;

  /* create socket layer */
  sio=GWEN_SyncIo_Socket_new(GWEN_SocketTypeTCP, GWEN_AddressFamilyIP);
  if (sio==NULL) {
    fprintf(stderr,
	    "ERROR in check_syncio_http1: Could not create socket io layer.\n");
    return 2;
  }
  GWEN_SyncIo_Socket_SetAddress(sio, "devel.aqbanking.de");
  GWEN_SyncIo_Socket_SetPort(sio, 443);

  baseLayer=sio;

  sio=GWEN_SyncIo_Tls_new(baseLayer);
  if (sio==NULL) {
    fprintf(stderr,
	    "ERROR in check_syncio_http1: Could not create TLS io layer.\n");
    return 2;
  }

  baseLayer=sio;

  sio=GWEN_SyncIo_Buffered_new(baseLayer);
  if (sio==NULL) {
    fprintf(stderr,
	    "ERROR in check_syncio_http1: Could not create Buffered io layer.\n");
    return 2;
  }

  baseLayer=sio;

  sio=GWEN_SyncIo_Http_new(baseLayer);
  if (sio==NULL) {
    fprintf(stderr,
	    "ERROR in check_io_tls1: Could not create HTTP io layer.\n");
    return 2;
  }

  for (j=0; j<2; j++) {
    int firstRead=1;
    int bodySize=-1;
    int bytesRead=0;
    GWEN_BUFFER *tbuf;

    rv=GWEN_SyncIo_Connect(sio);
    if (rv<0) {
      fprintf(stderr,
	      "ERROR in check_syncio_http1: Could not connect (%d)\n", rv);
      return 2;
    }

    db=GWEN_SyncIo_Http_GetDbCommandOut(sio);
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "command", "GET");
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "protocol", "HTTP/1.0");
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "url", "/index.html");

    db=GWEN_SyncIo_Http_GetDbHeaderOut(sio);
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "Host", "devel.aqbanking.de");
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "Content-length", 0);
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "Connection", "close");

    rv=GWEN_SyncIo_Write(sio, (uint8_t*)"", 0);
    if (rv<0) {
      fprintf(stderr,
	      "ERROR in check_syncio_http1: Could not write (%d)\n", rv);
      return 2;
    }

    tbuf=GWEN_Buffer_new(0, 1024, 0, 1);
    for (;;) {
      uint8_t *p;
      uint32_t l;

      rv=GWEN_Buffer_AllocRoom(tbuf, 1024);
      if (rv<0) {
	fprintf(stderr,
		"ERROR in check_syncio_http1: Could not allocRoom (%d)\n", rv);
	return 2;
      }

      p=(uint8_t*) GWEN_Buffer_GetPosPointer(tbuf);
      l=GWEN_Buffer_GetMaxUnsegmentedWrite(tbuf);
      do {
	rv=GWEN_SyncIo_Read(sio, p, l-1);
      } while(rv==GWEN_ERROR_INTERRUPTED);
      if (rv==0)
	break;
      else if (rv<0) {
	if (rv==GWEN_ERROR_EOF) {
	  if (bodySize!=-1 && bytesRead<bodySize) {
	    fprintf(stderr,
		    "ERROR in check_syncio_http1: Received too few bytes (%d<%d)\n",
		    bytesRead, bodySize);
	    return 2;
	  }
	}
	fprintf(stderr,
		"ERROR in check_syncio_http1: Could not read (%d) [%d / %d]\n",
		rv, bytesRead, bodySize);
	return 2;
      }
      else {
	GWEN_Buffer_IncrementPos(tbuf, rv);
	GWEN_Buffer_AdjustUsedBytes(tbuf);
	if (firstRead) {
	  GWEN_DB_NODE *db;

	  db=GWEN_SyncIo_Http_GetDbHeaderIn(sio);
	  bodySize=GWEN_DB_GetIntValue(db, "Content-length", 0, -1);
	}
        bytesRead+=rv;
      }

      if (bodySize!=-1 && bytesRead>=bodySize) {
	break;
      }
      firstRead=0;
    }

#if 0
    fprintf(stderr, "Received:\n");
    GWEN_Buffer_Dump(tbuf, stderr, 2);
#endif
    GWEN_Buffer_free(tbuf);

    rv=GWEN_SyncIo_Disconnect(sio);
    if (rv<0) {
      fprintf(stderr,
	      "ERROR in check_syncio_http1: Could not disconnect (%d)\n", rv);
      return 2;
    }
  }

  return 0;
}



int check_syncio_https() {
  int errs=0;
  int chks=0;

  fprintf(stderr, "SyncIO https checks... ");
  chks++;
  if (check_syncio_https1())
    errs++;

  if (chks) {
    if (chks==errs) {
      fprintf(stderr, "SyncIO https checks: all failed.\n");
    }
    else if (errs) {
      fprintf(stderr, "SyncIO https checks: some failed.\n");
    }
    else {
      fprintf(stderr, "passed.\n");
    }
  }

  return errs;
}





