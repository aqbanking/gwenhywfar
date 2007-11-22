

#include <gwenhywfar/iomanager.h>
#include <gwenhywfar/io_file.h>
#include <gwenhywfar/io_buffered.h>
#include <gwenhywfar/buffer.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>


#define TESTFILE_NAME1 "testfile"

#define TEST_LINE1 "This is a test file. Do not edit it."
#define TEST_LINE2 "It will soon be deleted, anyway"
#define TEST_LINE3 "bye"



static int createTestFile1() {
  FILE *f;
  int rv;

  f=fopen(TESTFILE_NAME1, "w+");
  if (f==NULL)
    return 2;

  rv=fprintf(f, "%s\n%s\n%s", TEST_LINE1, TEST_LINE2, TEST_LINE3);
  if (rv<1) {
    fprintf(stderr,
	    "ERROR in createTestFile1: Could not write to file (%s)\n", strerror(errno));
    return 2;
  }
  if (fclose(f)) {
    fprintf(stderr,
	    "ERROR in createTestFile1: Could not close file (%s)\n", strerror(errno));
    return 2;
  }

  return 0;
}



static int readTestFile1(GWEN_BUFFER *fbuf) {
  FILE *f;
  int rv;
  char buffer[1024];

  f=fopen(TESTFILE_NAME1, "r");
  if (f==NULL) {
    fprintf(stderr, "ERROR in readTestFile1: Could not open file (%s)\n", strerror(errno));
    return 2;
  }

  while(!feof(f)) {
    rv=fread(buffer, 1, sizeof(buffer), f);
    if (rv<0) {
      fprintf(stderr,
	      "ERROR in readTestFile1: Could not read from file (%s)\n", strerror(errno));
      return 2;
    }
    else if (rv==0)
      break;
    GWEN_Buffer_AppendBytes(fbuf, buffer, rv);
  }

  if (fclose(f)) {
    fprintf(stderr,
	    "ERROR in openTestFile1: Could not close file (%s)\n", strerror(errno));
    return 2;
  }

  return 0;
}



int check_io_buffered1() {
  GWEN_IO_LAYER *baseIo;
  GWEN_IO_LAYER *io;
  int fd;
  uint8_t buffer[1024];
  int rv;
  GWEN_BUFFER *fbuf;

  if (createTestFile1())
    return 2;

  /* open for reading */
  fd=open(TESTFILE_NAME1, O_RDONLY);
  if (fd==-1) {
    fprintf(stderr,
	    "ERROR in check_io_buffered1: Could not open testfile (%s)\n", strerror(errno));
    unlink(TESTFILE_NAME1);
    return 2;
  }

  /* create io layer for this file (readonly) */
  io=GWEN_Io_LayerFile_new(fd, -1);
  if (io==NULL) {
    fprintf(stderr,
	    "ERROR in check_io_buffered1: Could not create io layer.\n");
    unlink(TESTFILE_NAME1);
    return 2;
  }
  baseIo=io;

  io=GWEN_Io_LayerBuffered_new(baseIo);
  if (io==NULL) {
    fprintf(stderr,
	    "ERROR in check_io_buffered1: Could not create 2nd io layer.\n");
    unlink(TESTFILE_NAME1);
    return 2;
  }

  rv=GWEN_Io_Manager_RegisterLayer(io);
  if (rv) {
    fprintf(stderr,
	    "ERROR in check_io_buffered1: Could not register io layer (%d).\n", rv);
    GWEN_Io_Layer_free(io);
    unlink(TESTFILE_NAME1);
    return 2;
  }

  fbuf=GWEN_Buffer_new(0, 1024, 0, 1);

  /* read first line */
  for (;;) {
    GWEN_IO_REQUEST *r;
    GWEN_IO_REQUEST_STATUS st;

    r=GWEN_Io_Request_new(GWEN_Io_Request_TypeRead,
			  buffer, sizeof(buffer), NULL, NULL, 0);
    //GWEN_Io_Request_AddFlags(r, GWEN_IO_REQUEST_BUFFERED_FLAGS_RAW);
    rv=GWEN_Io_Layer_AddRequest(io, r);
    if (rv) {
      GWEN_Io_Request_free(r);
      if (rv==GWEN_ERROR_EOF)
	break;
      else {
	fprintf(stderr,
		"ERROR in check_io_buffered1: Could not add request to io layer.\n");
	GWEN_Io_Layer_free(io);
	unlink(TESTFILE_NAME1);
	return 2;
      }
    }

    rv=GWEN_Io_Manager_WaitForRequest(r, 2000);
    if (rv) {
      fprintf(stderr,
	      "ERROR in check_io_buffered1: Error waiting for request (%d).\n", rv);
      GWEN_Io_Layer_free(io);
      unlink(TESTFILE_NAME1);
      return 2;
    }

    /* if not finished, report an error */
    st=GWEN_Io_Request_GetStatus(r);
    if (st!=GWEN_Io_Request_StatusFinished) {
      fprintf(stderr,
	      "ERROR in check_io_buffered1: Bad status for request (%d).\n", rv);
      GWEN_Io_Layer_free(io);
      unlink(TESTFILE_NAME1);
      return 2;
    }
  
    /* check result code */
    rv=GWEN_Io_Request_GetResultCode(r);
    if (rv && rv!=GWEN_ERROR_EOF) {
      fprintf(stderr,
	      "ERROR in check_io_buffered1: Result of request is an error (%d).\n", rv);
      GWEN_Io_Layer_free(io);
      unlink(TESTFILE_NAME1);
      return 2;
    }

    GWEN_Buffer_AppendBytes(fbuf, (const char*)buffer, GWEN_Io_Request_GetBufferPos(r));

    if (GWEN_Io_Request_GetFlags(r) & GWEN_IO_REQUEST_FLAGS_PACKETEND)
      break;

    GWEN_Io_Request_free(r);
  }


  if (GWEN_Buffer_GetUsedBytes(fbuf)!=strlen(TEST_LINE1)) {
    fprintf(stderr,
	    "ERROR in check_io_buffered1: Number of bytes read does not match (%d!=%d).\n",
	    GWEN_Buffer_GetUsedBytes(fbuf), (int)strlen(TEST_LINE1));
    GWEN_Io_Layer_free(io);
    unlink(TESTFILE_NAME1);
    return 2;
  }

  if (memcmp(GWEN_Buffer_GetStart(fbuf), TEST_LINE1, GWEN_Buffer_GetUsedBytes(fbuf))) {
    fprintf(stderr, "ERROR in check_io_buffered1: Content does not match.\n");
    GWEN_Io_Layer_free(io);
    unlink(TESTFILE_NAME1);
    return 2;
  }

  rv=GWEN_Io_Layer_Disconnect(io, 0, 0, 2000);
  if (rv) {
    fprintf(stderr,
	    "ERROR in check_io_buffered1: Could not close testfile (%d)\n", rv);
    GWEN_Io_Layer_free(io);
    unlink(TESTFILE_NAME1);
    return 2;
  }

  GWEN_Io_Layer_free(io);
  unlink(TESTFILE_NAME1);
  return 0;
}



int check_io_buffered2() {
  GWEN_IO_LAYER *baseIo;
  GWEN_IO_LAYER *io;
  int fd;
  int rv;
  GWEN_BUFFER *fbuf;
  GWEN_BUFFER *tbuf;

  if (createTestFile1())
    return 2;

  /* open for reading */
  fd=open(TESTFILE_NAME1, O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
  if (fd==-1) {
    fprintf(stderr,
	    "ERROR in check_io_buffered2: Could not open testfile (%s)\n", strerror(errno));
    unlink(TESTFILE_NAME1);
    return 2;
  }

  /* create io layer for this file (readonly) */
  io=GWEN_Io_LayerFile_new(-1, fd);
  if (io==NULL) {
    fprintf(stderr,
	    "ERROR in check_io_buffered2: Could not create io layer.\n");
    unlink(TESTFILE_NAME1);
    return 2;
  }
  baseIo=io;

  io=GWEN_Io_LayerBuffered_new(baseIo);
  if (io==NULL) {
    fprintf(stderr,
	    "ERROR in check_io_buffered2: Could not create 2nd io layer.\n");
    unlink(TESTFILE_NAME1);
    return 2;
  }

  rv=GWEN_Io_Manager_RegisterLayer(io);
  if (rv) {
    fprintf(stderr,
	    "ERROR in check_io_buffered2: Could not register io layer (%d).\n", rv);
    GWEN_Io_Layer_free(io);
    unlink(TESTFILE_NAME1);
    return 2;
  }

  /* try to write a line */
  rv=GWEN_Io_Layer_WriteBytes(io, (const uint8_t*)TEST_LINE1, strlen(TEST_LINE1),
			      GWEN_IO_REQUEST_FLAGS_PACKETEND |
                              GWEN_IO_REQUEST_FLAGS_WRITEALL |
			      GWEN_IO_REQUEST_FLAGS_FLUSH,
			      0, 2000);
  if (rv<0) {
    fprintf(stderr,
	    "ERROR in check_io_buffered2: Could not write to io layer (%d).\n", rv);
    GWEN_Io_Layer_free(io);
    unlink(TESTFILE_NAME1);
    return 2;
  }

  if (rv!=strlen(TEST_LINE1)) {
    fprintf(stderr,
	    "ERROR in check_io_buffered2: Number of bytes written does not match (%d!=%d).\n",
	    rv, (int)strlen(TEST_LINE1));
    GWEN_Io_Layer_free(io);
    unlink(TESTFILE_NAME1);
    return 2;
  }

  /* close file */
  rv=GWEN_Io_Layer_Disconnect(io, 0, 0, 2000);
  if (rv) {
    fprintf(stderr,
	    "ERROR in check_io_buffered2: Could not close testfile (%d)\n", rv);
    GWEN_Io_Layer_free(io);
    unlink(TESTFILE_NAME1);
    return 2;
  }
  GWEN_Io_Layer_free(io);

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(tbuf, TEST_LINE1);
  GWEN_Buffer_AppendByte(tbuf, 10);

  /* reread file */
  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=readTestFile1(fbuf);
  if (rv) {
    fprintf(stderr,
	    "ERROR in check_io_buffered2: Could not reread testfile (%d)\n", rv);
    GWEN_Io_Layer_free(io);
    unlink(TESTFILE_NAME1);
    return 2;
  }

  /* check size */
  if (GWEN_Buffer_GetUsedBytes(fbuf)!=GWEN_Buffer_GetUsedBytes(tbuf)) {
    fprintf(stderr,
	    "ERROR in check_io_buffered2: Number of bytes read does not match (%d!=%d).\n",
	    GWEN_Buffer_GetUsedBytes(fbuf), GWEN_Buffer_GetUsedBytes(tbuf));
    unlink(TESTFILE_NAME1);
    return 2;
  }

  /* check content */
  if (memcmp(GWEN_Buffer_GetStart(fbuf),
	     GWEN_Buffer_GetStart(tbuf), GWEN_Buffer_GetUsedBytes(fbuf))) {
    fprintf(stderr, "ERROR in check_io_buffered2: Content does not match.\n");
    unlink(TESTFILE_NAME1);
    return 2;
  }

  unlink(TESTFILE_NAME1);
  return 0;
}



int check_io_buffered3() {
  GWEN_IO_LAYER *baseIo;
  GWEN_IO_LAYER *io;
  int fd;
  int rv;
  GWEN_BUFFER *fbuf;
  GWEN_BUFFER *tbuf;

  if (createTestFile1())
    return 2;

  /* open for reading */
  fd=open(TESTFILE_NAME1, O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
  if (fd==-1) {
    fprintf(stderr,
	    "ERROR in check_io_buffered3: Could not open testfile (%s)\n", strerror(errno));
    unlink(TESTFILE_NAME1);
    return 2;
  }

  /* create io layer for this file (readonly) */
  io=GWEN_Io_LayerFile_new(-1, fd);
  if (io==NULL) {
    fprintf(stderr,
	    "ERROR in check_io_buffered3: Could not create io layer.\n");
    unlink(TESTFILE_NAME1);
    return 2;
  }
  baseIo=io;

  io=GWEN_Io_LayerBuffered_new(baseIo);
  if (io==NULL) {
    fprintf(stderr,
	    "ERROR in check_io_buffered3: Could not create 2nd io layer.\n");
    unlink(TESTFILE_NAME1);
    return 2;
  }

  GWEN_Io_Layer_AddFlags(io, GWEN_IO_LAYER_BUFFERED_FLAGS_DOSMODE);

  rv=GWEN_Io_Manager_RegisterLayer(io);
  if (rv) {
    fprintf(stderr,
	    "ERROR in check_io_buffered3: Could not register io layer (%d).\n", rv);
    GWEN_Io_Layer_free(io);
    unlink(TESTFILE_NAME1);
    return 2;
  }

  /* try to write a line */
  rv=GWEN_Io_Layer_WriteBytes(io, (const uint8_t*)TEST_LINE1, strlen(TEST_LINE1),
			      GWEN_IO_REQUEST_FLAGS_PACKETEND |
			      GWEN_IO_REQUEST_FLAGS_WRITEALL |
			      GWEN_IO_REQUEST_FLAGS_FLUSH,
			      0, 2000);
  if (rv<0) {
    fprintf(stderr,
	    "ERROR in check_io_buffered3: Could not write to io layer (%d).\n", rv);
    GWEN_Io_Layer_free(io);
    unlink(TESTFILE_NAME1);
    return 2;
  }

  if (rv!=strlen(TEST_LINE1)) {
    fprintf(stderr,
	    "ERROR in check_io_buffered3: Number of bytes written does not match (%d!=%d).\n",
	    rv, (int)strlen(TEST_LINE1));
    GWEN_Io_Layer_free(io);
    unlink(TESTFILE_NAME1);
    return 2;
  }

  /* close file */
  rv=GWEN_Io_Layer_Disconnect(io, 0, 0, 2000);
  if (rv) {
    fprintf(stderr,
	    "ERROR in check_io_buffered3: Could not close testfile (%d)\n", rv);
    GWEN_Io_Layer_free(io);
    unlink(TESTFILE_NAME1);
    return 2;
  }
  GWEN_Io_Layer_free(io);

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(tbuf, TEST_LINE1);
  GWEN_Buffer_AppendByte(tbuf, 13);
  GWEN_Buffer_AppendByte(tbuf, 10);

  /* reread file */
  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=readTestFile1(fbuf);
  if (rv) {
    fprintf(stderr,
	    "ERROR in check_io_buffered3: Could not reread testfile (%d)\n", rv);
    GWEN_Io_Layer_free(io);
    unlink(TESTFILE_NAME1);
    return 2;
  }

  /* check size */
  if (GWEN_Buffer_GetUsedBytes(fbuf)!=GWEN_Buffer_GetUsedBytes(tbuf)) {
    fprintf(stderr,
	    "ERROR in check_io_buffered3: Number of bytes read does not match (%d!=%d).\n",
	    GWEN_Buffer_GetUsedBytes(fbuf), GWEN_Buffer_GetUsedBytes(tbuf));
    unlink(TESTFILE_NAME1);
    return 2;
  }

  /* check content */
  if (memcmp(GWEN_Buffer_GetStart(fbuf),
	     GWEN_Buffer_GetStart(tbuf), GWEN_Buffer_GetUsedBytes(fbuf))) {
    fprintf(stderr, "ERROR in check_io_buffered3: Content does not match.\n");
    unlink(TESTFILE_NAME1);
    return 2;
  }

  unlink(TESTFILE_NAME1);
  return 0;
}



int check_io_buffered4() {
  GWEN_IO_LAYER *baseIo;
  GWEN_IO_LAYER *io;
  int fd;
  int rv;
  GWEN_BUFFER *fbuf;

  if (createTestFile1())
    return 2;

  /* open for reading */
  fd=open(TESTFILE_NAME1, O_RDONLY);
  if (fd==-1) {
    fprintf(stderr,
	    "ERROR in check_io_buffered4: Could not open testfile (%s)\n", strerror(errno));
    unlink(TESTFILE_NAME1);
    return 2;
  }

  /* create io layer for this file (readonly) */
  io=GWEN_Io_LayerFile_new(fd, -1);
  if (io==NULL) {
    fprintf(stderr,
	    "ERROR in check_io_buffered4: Could not create io layer.\n");
    unlink(TESTFILE_NAME1);
    return 2;
  }
  baseIo=io;

  io=GWEN_Io_LayerBuffered_new(baseIo);
  if (io==NULL) {
    fprintf(stderr,
	    "ERROR in check_io_buffered4: Could not create 2nd io layer.\n");
    unlink(TESTFILE_NAME1);
    return 2;
  }

  rv=GWEN_Io_Manager_RegisterLayer(io);
  if (rv) {
    fprintf(stderr,
	    "ERROR in check_io_buffered4: Could not register io layer (%d).\n", rv);
    GWEN_Io_Layer_free(io);
    unlink(TESTFILE_NAME1);
    return 2;
  }

  fbuf=GWEN_Buffer_new(0, 1024, 0, 1);
  rv=GWEN_Io_LayerBuffered_ReadLineToBuffer(io, fbuf, 0, 2000);
  if (rv) {
    fprintf(stderr,
	    "ERROR in check_io_buffered4: Could not read line.\n");
    GWEN_Io_Layer_free(io);
    unlink(TESTFILE_NAME1);
    return 2;
  }

  if (GWEN_Buffer_GetUsedBytes(fbuf)!=strlen(TEST_LINE1)) {
    fprintf(stderr,
	    "ERROR in check_io_buffered4: Number of bytes read does not match (%d!=%d).\n",
	    GWEN_Buffer_GetUsedBytes(fbuf), (int)strlen(TEST_LINE1));
    GWEN_Io_Layer_free(io);
    unlink(TESTFILE_NAME1);
    return 2;
  }

  if (memcmp(GWEN_Buffer_GetStart(fbuf), TEST_LINE1, GWEN_Buffer_GetUsedBytes(fbuf))) {
    fprintf(stderr, "ERROR in check_io_buffered4: Content does not match.\n");
    GWEN_Io_Layer_free(io);
    unlink(TESTFILE_NAME1);
    return 2;
  }

  if (GWEN_Io_LayerBuffered_GetReadLineCount(io)!=1) {
    fprintf(stderr, "ERROR in check_io_buffered4: Number of lines does not match.\n");
    GWEN_Io_Layer_free(io);
    unlink(TESTFILE_NAME1);
    return 2;
  }

  rv=GWEN_Io_Layer_Disconnect(io, 0, 0, 2000);
  if (rv) {
    fprintf(stderr,
	    "ERROR in check_io_buffered4: Could not close testfile (%d)\n", rv);
    GWEN_Io_Layer_free(io);
    unlink(TESTFILE_NAME1);
    return 2;
  }

  GWEN_Io_Layer_free(io);
  unlink(TESTFILE_NAME1);
  return 0;
}



int check_io_buffered5() {
  GWEN_IO_LAYER *baseIo;
  GWEN_IO_LAYER *io;
  int fd;
  int rv;
  GWEN_BUFFER *fbuf;
  GWEN_BUFFER *tbuf;

  if (createTestFile1())
    return 2;

  /* open for reading */
  fd=open(TESTFILE_NAME1, O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
  if (fd==-1) {
    fprintf(stderr,
	    "ERROR in check_io_buffered5: Could not open testfile (%s)\n", strerror(errno));
    unlink(TESTFILE_NAME1);
    return 2;
  }

  /* create io layer for this file (readonly) */
  io=GWEN_Io_LayerFile_new(-1, fd);
  if (io==NULL) {
    fprintf(stderr,
	    "ERROR in check_io_buffered5: Could not create io layer.\n");
    unlink(TESTFILE_NAME1);
    return 2;
  }
  baseIo=io;

  io=GWEN_Io_LayerBuffered_new(baseIo);
  if (io==NULL) {
    fprintf(stderr,
	    "ERROR in check_io_buffered5: Could not create 2nd io layer.\n");
    unlink(TESTFILE_NAME1);
    return 2;
  }

  rv=GWEN_Io_Manager_RegisterLayer(io);
  if (rv) {
    fprintf(stderr,
	    "ERROR in check_io_buffered5: Could not register io layer (%d).\n", rv);
    GWEN_Io_Layer_free(io);
    unlink(TESTFILE_NAME1);
    return 2;
  }

  /* try to write a line */
  rv=GWEN_Io_LayerBuffered_WriteLine(io, TEST_LINE1, -1, 1, 0, 2000);
  if (rv<0) {
    fprintf(stderr,
	    "ERROR in check_io_buffered5: Could not write to io layer (%d).\n", rv);
    GWEN_Io_Layer_free(io);
    unlink(TESTFILE_NAME1);
    return 2;
  }

  if (rv!=strlen(TEST_LINE1)) {
    fprintf(stderr,
	    "ERROR in check_io_buffered5: Number of bytes written does not match (%d!=%d).\n",
	    rv, (int)strlen(TEST_LINE1));
    GWEN_Io_Layer_free(io);
    unlink(TESTFILE_NAME1);
    return 2;
  }

  /* close file */
  rv=GWEN_Io_Layer_Disconnect(io, 0, 0, 2000);
  if (rv) {
    fprintf(stderr,
	    "ERROR in check_io_buffered5: Could not close testfile (%d)\n", rv);
    GWEN_Io_Layer_free(io);
    unlink(TESTFILE_NAME1);
    return 2;
  }
  GWEN_Io_Layer_free(io);

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(tbuf, TEST_LINE1);
  GWEN_Buffer_AppendByte(tbuf, 10);

  /* reread file */
  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=readTestFile1(fbuf);
  if (rv) {
    fprintf(stderr,
	    "ERROR in check_io_buffered5: Could not reread testfile (%d)\n", rv);
    GWEN_Io_Layer_free(io);
    unlink(TESTFILE_NAME1);
    return 2;
  }

  /* check size */
  if (GWEN_Buffer_GetUsedBytes(fbuf)!=GWEN_Buffer_GetUsedBytes(tbuf)) {
    fprintf(stderr,
	    "ERROR in check_io_buffered5: Number of bytes read does not match (%d!=%d).\n",
	    GWEN_Buffer_GetUsedBytes(fbuf), GWEN_Buffer_GetUsedBytes(tbuf));
    unlink(TESTFILE_NAME1);
    return 2;
  }

  /* check content */
  if (memcmp(GWEN_Buffer_GetStart(fbuf),
	     GWEN_Buffer_GetStart(tbuf), GWEN_Buffer_GetUsedBytes(fbuf))) {
    fprintf(stderr, "ERROR in check_io_buffered5: Content does not match.\n");
    unlink(TESTFILE_NAME1);
    return 2;
  }

  unlink(TESTFILE_NAME1);
  return 0;
}



int check_io_buffered() {
  int errs=0;
  int chks=0;

  fprintf(stderr, "IO buffered checks... ");
  chks++;
  if (check_io_buffered1())
    errs++;
  chks++;
  if (check_io_buffered2())
    errs++;
  chks++;
  if (check_io_buffered3())
    errs++;
  chks++;
  if (check_io_buffered4())
    errs++;
  chks++;
  if (check_io_buffered5())
    errs++;

  if (chks) {
    if (chks==errs) {
      fprintf(stderr, "IO file checks: all failed.\n");
    }
    else if (errs) {
      fprintf(stderr, "IO file checks: some failed.\n");
    }
    else {
      fprintf(stderr, "passed.\n");
    }
  }

  return errs;
}





