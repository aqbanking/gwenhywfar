
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gwenhywfar/iomanager.h>
#include <gwenhywfar/io_file.h>
#include <gwenhywfar/buffer.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>



#define TESTFILE_NAME1 "testfile"
#define TEST_PATTERN1 \
  "This is a test file. Do not edit it\n" \
  "It will soon be deleted, anyway\n" \
  "                      " \
  "00000000000000000000000000000000" \
  "11111111111111111111111111111111" \
  "22222222222222222222222222222222" \
  "33333333333333333333333333333333" \
  "44444444444444444444444444444444" \
  "55555555555555555555555555555555" \
  "66666666666666666666666666666666" \
  "77777777777777777777777777777777" \
  "88888888888888888888888888888888" \
  "99999999999999999999999999999999" \
  "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" \
  "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB" \
  "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC" \
  "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD" \
  "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE" \
  "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF" \
  "GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG" \
  "HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH" \
  "IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII" \
  "JJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJ" \
  "KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK" \
  "LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL" \
  "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM" \
  "NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN" \
  "OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO" \
  "PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP" \
  "QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ" \
  "RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR" \
  "SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS" \
  "TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT" \
  "UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU" \
  "VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV" \
  "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW" \
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" \
  "YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY" \
  "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ" \
  "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" \
  "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb" \
  "cccccccccccccccccccccccccccccccc" \
  "dddddddddddddddddddddddddddddddd" \
  "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee" \
  "ffffffffffffffffffffffffffffffff" \
  "gggggggggggggggggggggggggggggggg" \
  "hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh" \
  "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii" \
  "jjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjj" \
  "kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk" \
  "llllllllllllllllllllllllllllllll" \
  "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm" \
  "nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn" \
  "oooooooooooooooooooooooooooooooo" \
  "pppppppppppppppppppppppppppppppp" \
  "qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq" \
  "rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr" \
  "ssssssssssssssssssssssssssssssss" \
  "tttttttttttttttttttttttttttttttt" \
  "uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu" \
  "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv" \
  "wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww" \
  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" \
  "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy" \
  "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz"




static int createTestFile1() {
  FILE *f;
  int rv;

  f=fopen(TESTFILE_NAME1, "w+");
  if (f==NULL)
    return 2;

  rv=fprintf(f, "%s", TEST_PATTERN1);
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



static int check_io_file1() {
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
	    "ERROR in check_io_file1: Could not open testfile (%s)\n", strerror(errno));
    unlink(TESTFILE_NAME1);
    return 2;
  }

  /* create io layer for this file (readonly) */
  io=GWEN_Io_LayerFile_new(fd, -1);
  if (io==NULL) {
    fprintf(stderr,
	    "ERROR in check_io_file1: Could not create io layer.\n");
    unlink(TESTFILE_NAME1);
    return 2;
  }

  if (GWEN_Io_LayerFile_GetReadFileDescriptor(io)!=fd) {
    fprintf(stderr,
	    "ERROR in check_io_file1: File read descriptor does not match.\n");
    GWEN_Io_Layer_free(io);
    unlink(TESTFILE_NAME1);
    return 2;
  }

  if (GWEN_Io_LayerFile_GetWriteFileDescriptor(io)!=-1) {
    fprintf(stderr,
	    "ERROR in check_io_file1: File write descriptor does not match.\n");
    GWEN_Io_Layer_free(io);
    unlink(TESTFILE_NAME1);
    return 2;
  }

  rv=GWEN_Io_Manager_RegisterLayer(io);
  if (rv) {
    fprintf(stderr,
	    "ERROR in check_io_file1: Could not register io layer.\n");
    GWEN_Io_Layer_free(io);
    unlink(TESTFILE_NAME1);
    return 2;
  }

  fbuf=GWEN_Buffer_new(0, 1024, 0, 1);
  for (;;) {
    rv=GWEN_Io_Layer_ReadBytes(io, buffer, sizeof(buffer), 0, 0, 2000);
    if (rv<0) {
      if (rv!=GWEN_ERROR_EOF) {
	fprintf(stderr,
		"ERROR in check_io_file1: Could not read from testfile (%d)\n", rv);
        GWEN_Io_Layer_free(io);
	unlink(TESTFILE_NAME1);
	return 2;
      }
      else
        break;
    }
    else if (rv==0) {
      fprintf(stderr,
	      "ERROR in check_io_file1: 0 bytes read.\n");
      GWEN_Io_Layer_free(io);
      unlink(TESTFILE_NAME1);
      return 2;
    }
    GWEN_Buffer_AppendBytes(fbuf, (const char*)buffer, rv);
  }

  if (GWEN_Buffer_GetUsedBytes(fbuf)!=strlen(TEST_PATTERN1)) {
    fprintf(stderr,
	    "ERROR in check_io_file1: Number of bytes read does not match (%d!=%d).\n",
	    GWEN_Buffer_GetUsedBytes(fbuf), (int)strlen(TEST_PATTERN1));
    GWEN_Io_Layer_free(io);
    unlink(TESTFILE_NAME1);
    return 2;
  }

  if (memcmp(GWEN_Buffer_GetStart(fbuf), TEST_PATTERN1, GWEN_Buffer_GetUsedBytes(fbuf))) {
    fprintf(stderr, "ERROR in check_io_file1: Content does not match.\n");
    GWEN_Io_Layer_free(io);
    unlink(TESTFILE_NAME1);
    return 2;
  }

  rv=GWEN_Io_Layer_Disconnect(io, 0, 0, 2000);
  if (rv) {
    fprintf(stderr,
	    "ERROR in check_io_file1: Could not close testfile (%d)\n", rv);
    GWEN_Io_Layer_free(io);
    unlink(TESTFILE_NAME1);
    return 2;
  }

  GWEN_Io_Layer_free(io);
  unlink(TESTFILE_NAME1);
  return 0;
}



static int check_io_file2() {
  GWEN_IO_LAYER *io;
  int fd;
  int rv;
  GWEN_BUFFER *fbuf;

  /* open for reading */
  fd=open(TESTFILE_NAME1, O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
  if (fd==-1) {
    fprintf(stderr,
	    "ERROR in check_io_file1: Could not create testfile (%s)\n", strerror(errno));
    unlink(TESTFILE_NAME1);
    return 2;
  }

  /* create io layer for this file (wronly) */
  io=GWEN_Io_LayerFile_new(-1, fd);
  if (io==NULL) {
    fprintf(stderr,
	    "ERROR in check_io_file1: Could not create io layer.\n");
    unlink(TESTFILE_NAME1);
    return 2;
  }

  if (GWEN_Io_LayerFile_GetReadFileDescriptor(io)!=-1) {
    fprintf(stderr,
	    "ERROR in check_io_file1: File read descriptor does not match.\n");
    GWEN_Io_Layer_free(io);
    unlink(TESTFILE_NAME1);
    return 2;
  }

  if (GWEN_Io_LayerFile_GetWriteFileDescriptor(io)!=fd) {
    fprintf(stderr,
	    "ERROR in check_io_file1: File write descriptor does not match.\n");
    GWEN_Io_Layer_free(io);
    unlink(TESTFILE_NAME1);
    return 2;
  }

  rv=GWEN_Io_Manager_RegisterLayer(io);
  if (rv) {
    fprintf(stderr,
	    "ERROR in check_io_file1: Could not register io layer.\n");
    GWEN_Io_Layer_free(io);
    unlink(TESTFILE_NAME1);
    return 2;
  }

  rv=GWEN_Io_Layer_WriteBytes(io, (const uint8_t*)TEST_PATTERN1, strlen(TEST_PATTERN1), 0, 0, 2000);
  if (rv<0) {
    fprintf(stderr,
	    "ERROR in check_io_file1: Could not write to testfile (%d)\n", rv);
    GWEN_Io_Layer_free(io);
    unlink(TESTFILE_NAME1);
    return 2;
  }
  else if (rv!=strlen(TEST_PATTERN1)) {
    fprintf(stderr,
	    "ERROR in check_io_file1: Number of bytes written does not match (%d!=%d).\n",
	    rv, (int)strlen(TEST_PATTERN1));
    GWEN_Io_Layer_free(io);
    unlink(TESTFILE_NAME1);
    return 2;
  }

  /* close file */
  rv=GWEN_Io_Layer_Disconnect(io, 0, 0, 2000);
  if (rv) {
    fprintf(stderr,
	    "ERROR in check_io_file1: Could not close testfile (%d)\n", rv);
    GWEN_Io_Layer_free(io);
    unlink(TESTFILE_NAME1);
    return 2;
  }

  GWEN_Io_Layer_free(io);

  /* reread file */
  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=readTestFile1(fbuf);
  if (rv) {
    fprintf(stderr,
	    "ERROR in check_io_file1: Could not reread testfile (%d)\n", rv);
    unlink(TESTFILE_NAME1);
    return 2;
  }

  /* check size */
  if (GWEN_Buffer_GetUsedBytes(fbuf)!=strlen(TEST_PATTERN1)) {
    fprintf(stderr,
	    "ERROR in check_io_file2: Number of bytes read does not match (%d!=%d).\n",
	    GWEN_Buffer_GetUsedBytes(fbuf), (int)strlen(TEST_PATTERN1));
    unlink(TESTFILE_NAME1);
    return 2;
  }

  /* check content */
  if (memcmp(GWEN_Buffer_GetStart(fbuf), TEST_PATTERN1, GWEN_Buffer_GetUsedBytes(fbuf))) {
    fprintf(stderr, "ERROR in check_io_file1: Content does not match.\n");
    unlink(TESTFILE_NAME1);
    return 2;
  }

  unlink(TESTFILE_NAME1);
  return 0;
}



int check_io_file() {
  int errs=0;
  int chks=0;

  fprintf(stderr, "IO file checks... ");
  chks++;
  if (check_io_file1())
    errs++;
  chks++;
  if (check_io_file2())
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





