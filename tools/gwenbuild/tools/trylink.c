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

#include "gwenbuild/tools/trylink.h"

#include <gwenhywfar/syncio.h>
#include <gwenhywfar/process.h>
#include <gwenhywfar/debug.h>

#include <unistd.h>
#include <stdlib.h>




int GWB_Tools_TryLink(const char *testCode, const char *libName)
{
  GWEN_BUFFER *argBuffer;
  GWEN_BUFFER *responseBuffer;
  int rv;

  rv=GWEN_SyncIo_Helper_WriteFile("conftest.c", (const uint8_t*) testCode, strlen(testCode));
  if (rv<0) {
    DBG_ERROR(NULL, "Error writing conftest.c (%d)", rv);
    return rv;
  }

  responseBuffer=GWEN_Buffer_new(0, 256, 0, 1);
  argBuffer=GWEN_Buffer_new(0, 256, 0, 1);

  GWEN_Buffer_AppendString(argBuffer, "conftest.c -o contest -l");
  GWEN_Buffer_AppendString(argBuffer, libName);

  rv=GWEN_Process_RunCommandWaitAndGather("gcc", GWEN_Buffer_GetStart(argBuffer), responseBuffer);
  if (rv<0) {
    DBG_ERROR(NULL, "Error running gcc (%d)", rv);
    GWEN_Buffer_free(argBuffer);
    GWEN_Buffer_free(responseBuffer);
    return rv;
  }
  GWEN_Buffer_free(argBuffer);
  GWEN_Buffer_free(responseBuffer);

  unlink("conftest.c");
  unlink("conftest");

  return rv;
}



