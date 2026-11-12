/* A program that "make installcheck" builds against the installed
 * Gwenhywfar. It fails if the installed library and headers do not match.
 */

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/version.h>

#include <stdio.h>



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

  rv=GWEN_Fini();
  if (rv) {
    fprintf(stderr, "GWEN_Fini() failed: %d\n", rv);
    return 1;
  }
  return 0;
}
