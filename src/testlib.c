

#include <gwenhywfar/buffer.h>
#include <gwenhywfar/base64.h>



int check1() {
  const char *testString="01234567890123456789";
  int rv;
  GWEN_BUFFER *buf1;
  GWEN_BUFFER *buf2;
  const char *p1, *p2;
  int i;
  int len;

  buf1=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_Base64_Encode((const unsigned char*)testString,
                        strlen(testString),
                        buf1, 0);
  if (rv) {
    fprintf(stderr, "FAILED: Could not encode.\n");
    return 2;
  }

  buf2=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_Base64_Decode((const unsigned char*)GWEN_Buffer_GetStart(buf1), 0,
                        buf2);
  if (rv) {
    fprintf(stderr, "FAILED: Could not decode.\n");
    return 2;
  }

  p1=testString;
  len=strlen(testString);
  p2=GWEN_Buffer_GetStart(buf2);
  if (GWEN_Buffer_GetUsedBytes(buf2)!=len) {
    fprintf(stderr, "Data differs in size\n");
    return 3;
  }
  rv=0;
  for (i=0; i<len; i++) {
    if (p1[i]!=p2[i]) {
      fprintf(stderr, "Buffer1:\n%s\n", testString);
      fprintf(stderr, "Buffer2:\n");
      GWEN_Buffer_Dump(buf2, stderr, 2);

      fprintf(stderr, "Differ at %d (%04x)\n", i, i);
      rv=-1;
    }
  }

  if (rv) {
    fprintf(stderr, "Data differs in content\n");
    return 3;
  }

  fprintf(stderr, "Check 1: PASSED.\n");

  return 0;
}



int main(int argc, char **argv) {
  int rv;

  rv=check1();
  if (rv)
    return rv;

  return 0;
}


