/* A program that "make installcheck" builds against the installed
 * Gwenhywfar, together with the code that the installed typemaker and
 * typemaker2 generate from consumer_type1.xml and consumer_type2.xml.
 * It fails if the installed library and headers do not match, or if the
 * generated enum functions do not convert the enum values.
 */

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/version.h>

#include "consumer_type1.h"
#include "consumer_type2.h"

#include <stdio.h>
#include <string.h>



static int checkTypes(void)
{
  CONSUMER_TYPE1 *t1;
  CONSUMER_TYPE2 *t2;
  int errors=0;

  t1=Consumer_Type1_new();
  Consumer_Type1_SetKind(t1, Consumer_Type1_Kind_fromString("second"));
  if (Consumer_Type1_GetKind(t1)!=Consumer_Type1_KindSecond ||
      strcmp(Consumer_Type1_Kind_toString(Consumer_Type1_KindFirst), "first")!=0) {
    fprintf(stderr, "typemaker enum functions failed\n");
    errors++;
  }
  Consumer_Type1_free(t1);

  t2=Consumer_Type2_new();
  Consumer_Type2_SetKind(t2, Consumer_Type2_Kind_fromString("second"));
  if (Consumer_Type2_GetKind(t2)!=Consumer_Type2_Kind_Second ||
      strcmp(Consumer_Type2_Kind_toString(Consumer_Type2_Kind_First), "first")!=0) {
    fprintf(stderr, "typemaker2 enum functions failed\n");
    errors++;
  }
  Consumer_Type2_free(t2);

  return errors;
}



int main(void)
{
  int major, minor, patchlevel, build;
  int rv;

  rv=GWEN_Init();
  if (rv) {
    fprintf(stderr, "GWEN_Init() failed: %d\n", rv);
    return 1;
  }

  GWEN_Version(&major, &minor, &patchlevel, &build);
  printf("library %d.%d.%d.%d, headers %d.%d.%d.%d\n",
         major, minor, patchlevel, build,
         GWENHYWFAR_VERSION_MAJOR, GWENHYWFAR_VERSION_MINOR,
         GWENHYWFAR_VERSION_PATCHLEVEL, GWENHYWFAR_VERSION_BUILD);
  if (major!=GWENHYWFAR_VERSION_MAJOR ||
      minor!=GWENHYWFAR_VERSION_MINOR ||
      patchlevel!=GWENHYWFAR_VERSION_PATCHLEVEL ||
      build!=GWENHYWFAR_VERSION_BUILD) {
    fprintf(stderr, "library and header versions differ\n");
    GWEN_Fini();
    return 1;
  }

  if (checkTypes()) {
    GWEN_Fini();
    return 1;
  }

  rv=GWEN_Fini();
  if (rv) {
    fprintf(stderr, "GWEN_Fini() failed: %d\n", rv);
    return 1;
  }
  return 0;
}
