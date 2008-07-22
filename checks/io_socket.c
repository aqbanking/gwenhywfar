
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gwenhywfar/iomanager.h>
#include <gwenhywfar/io_socket.h>
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
  "host: devel.aqbanking.de\r\n" \
  "\r\n"




static int check_io_socket1() {
  GWEN_IO_LAYER *io;
  int rv;
  GWEN_INETADDRESS *addr;
  GWEN_SOCKET *sk;
  char buffer[2048];

  addr=GWEN_InetAddr_new(GWEN_AddressFamilyIP);
  rv=GWEN_InetAddr_SetName(addr, "devel.aqbanking.de");
  if (rv) {
    fprintf(stderr,
	    "ERROR in check_io_socket1: Could not resolve hostname (%d)\n", rv);
    return 2;
  }
  rv=GWEN_InetAddr_SetPort(addr, 80);
  if (rv) {
    fprintf(stderr,
	    "ERROR in check_io_socket1: Could not set port (%d)\n", rv);
    return 2;
  }

  sk=GWEN_Socket_new(GWEN_SocketTypeTCP);
  if (sk==NULL) {
    fprintf(stderr,
	    "ERROR in check_io_socket1: Socket type not supported\n");
    return 2;
  }

  io=GWEN_Io_LayerSocket_new(sk);
  GWEN_Io_LayerSocket_SetPeerAddr(io, addr);

  rv=GWEN_Io_Manager_RegisterLayer(io);
  if (rv) {
    fprintf(stderr,
	    "ERROR in check_io_socket1: Could not register io layer.\n");
    GWEN_Io_Layer_free(io);
    return 2;
  }


  rv=GWEN_Io_Layer_Connect(io, 0, 0, 10000);
  if (rv) {
    fprintf(stderr,
	    "ERROR in check_io_socket1: Could not connect (%d)\n", rv);
    return 2;
  }

  rv=GWEN_Io_Layer_WriteBytes(io, (const uint8_t*)TEST_PATTERN1, strlen(TEST_PATTERN1),
			      GWEN_IO_REQUEST_FLAGS_WRITEALL |
			      GWEN_IO_REQUEST_FLAGS_FLUSH,
			      0, 2000);
  if (rv<1) {
    fprintf(stderr,
	    "ERROR in check_io_socket1: Could not write (%d)\n", rv);
    return 2;
  }

  rv=GWEN_Io_Layer_ReadBytes(io, (uint8_t*)buffer, sizeof(buffer)-1, 0, 0, 10000);
  if (rv<1) {
    fprintf(stderr,
	    "ERROR in check_io_socket1: Could not read (%d)\n", rv);
    return 2;
  }

  rv=GWEN_Io_Layer_Disconnect(io, 0, 0, 2000);
  if (rv) {
    fprintf(stderr,
	    "ERROR in check_io_socket1: Could not disconnect (%d)\n", rv);
    return 2;
  }

  return 0;
}



int check_io_socket() {
  int errs=0;
  int chks=0;

  fprintf(stderr, "IO Socket checks... ");
  chks++;
  if (check_io_socket1())
    errs++;

  if (chks) {
    if (chks==errs) {
      fprintf(stderr, "IO socket checks: all failed.\n");
    }
    else if (errs) {
      fprintf(stderr, "IO socket checks: some failed.\n");
    }
    else {
      fprintf(stderr, "passed.\n");
    }
  }

  return errs;
}





