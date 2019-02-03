
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gwenhywfar/syncio_socket.h>
#include <gwenhywfar/syncio_tls.h>
#include <gwenhywfar/buffer.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>


#define TEST_PATTERN1 \
  "GET / HTTP/1.1\r\n" \
  "host: sourceforge.net\r\n" \
  "Connection: close\r\n" \
  "\r\n"




static int check_syncio_tls1()
{
  GWEN_SYNCIO *sio;
  GWEN_SYNCIO *baseLayer;
  int rv;
  char buffer[2048];


  /* create socket layer */
  sio=GWEN_SyncIo_Socket_new(GWEN_SocketTypeTCP, GWEN_AddressFamilyIP);
  if (sio==NULL) {
    fprintf(stderr,
            "ERROR in check_io_tls1: Could not create socket io layer.\n");
    return 2;
  }
  GWEN_SyncIo_Socket_SetAddress(sio, "sourceforge.net");
  GWEN_SyncIo_Socket_SetPort(sio, 443);

  baseLayer=sio;

  sio=GWEN_SyncIo_Tls_new(baseLayer);
  if (sio==NULL) {
    fprintf(stderr,
            "ERROR in check_io_tls1: Could not create TLS io layer.\n");
    return 2;
  }
  //GWEN_SyncIo_Tls_SetLocalTrustFile(sio, "test.crt");

  rv=GWEN_SyncIo_Connect(sio);
  if (rv) {
    fprintf(stderr,
            "ERROR in check_io_tls1: Could not connect (%d)\n", rv);
    return 2;
  }

  rv=GWEN_SyncIo_WriteForced(sio, (const uint8_t *)TEST_PATTERN1, strlen(TEST_PATTERN1));
  if (rv<1) {
    fprintf(stderr,
            "ERROR in check_io_tls1: Could not write (%d)\n", rv);
    return 2;
  }

  for (;;) {
    rv=GWEN_SyncIo_Read(sio, (uint8_t *)buffer, sizeof(buffer)-1);
    if (rv==0)
      break;
    else if (rv<1) {
      if (rv==GWEN_ERROR_TIMEOUT || rv==GWEN_ERROR_EOF || rv==GWEN_ERROR_SSL_PREMATURE_CLOSE)
        break;
      fprintf(stderr,
              "ERROR in check_io_tls1: Could not read (%d)\n", rv);
      return 2;
    }
    else {
      buffer[rv]=0;
      fprintf(stderr, "Data received: %s\n", buffer);
    }
  }

  rv=GWEN_SyncIo_Disconnect(sio);
  if (rv) {
    fprintf(stderr,
            "WARNING in check_io_tls1: Could not disconnect (%d)\n", rv);
  }

  return 0;
}



int check_syncio_tls()
{
  int errs=0;
  int chks=0;

  fprintf(stderr, "IO tls checks... ");
  chks++;
  if (check_syncio_tls1())
    errs++;

  if (chks) {
    if (chks==errs) {
      fprintf(stderr, "IO tls checks: all failed.\n");
    }
    else if (errs) {
      fprintf(stderr, "IO tls checks: some failed.\n");
    }
    else {
      fprintf(stderr, "passed.\n");
    }
  }

  return errs;
}





