/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "gwenbuild/tools/trycompile.h"

#include <gwenhywfar/syncio.h>
#include <gwenhywfar/process.h>
#include <gwenhywfar/debug.h>

#include <unistd.h>
#include <stdlib.h>




int GWB_Tools_TryCompile(const char *testCode)
{
  GWEN_BUFFER *responseBuffer;
  int rv;

  rv=GWEN_SyncIo_Helper_WriteFile("conftest.c", (const uint8_t*) testCode, strlen(testCode));
  if (rv<0) {
    DBG_ERROR(NULL, "Error writing conftest.c (%d)", rv);
    return rv;
  }

  responseBuffer=GWEN_Buffer_new(0, 256, 0, 1);

  rv=GWEN_Process_RunCommandWaitAndGather("gcc", "-c conftest.c -o contest.o", responseBuffer);
  if (rv<0) {
    DBG_ERROR(NULL, "Error running gcc (%d)", rv);
    GWEN_Buffer_free(responseBuffer);
    unlink("conftest.c");
    return rv;
  }
  GWEN_Buffer_free(responseBuffer);

  unlink("conftest.c");
  unlink("conftest.o");

  return rv;
}



