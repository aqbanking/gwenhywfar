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




int GWB_Tools_TryCompile(GWB_CONTEXT *context, const char *testCode)
{
  GWEN_BUFFER *argBuffer;
  GWEN_BUFFER *stdOutBuffer;
  GWEN_BUFFER *stdErrBuffer;
  const char *toolName;
  const char *s;
  int rv;

  toolName=GWEN_DB_GetCharValue(GWB_Context_GetVars(context), "GWBUILD_TOOL_CC", 0, "gcc");
  if (!(toolName && *toolName)) {
    DBG_ERROR(NULL, "No tool name for \"CC\"");
    return GWEN_ERROR_INTERNAL;
  }

  rv=GWEN_SyncIo_Helper_WriteFile("conftest.c", (const uint8_t*) testCode, strlen(testCode));
  if (rv<0) {
    DBG_ERROR(NULL, "Error writing conftest.c (%d)", rv);
    return rv;
  }

  stdOutBuffer=GWEN_Buffer_new(0, 256, 0, 1);
  stdErrBuffer=GWEN_Buffer_new(0, 256, 0, 1);

  argBuffer=GWEN_Buffer_new(0, 256, 0, 1);
  s=GWEN_DB_GetCharValue(GWB_Context_GetVars(context), "cflags", 0, NULL);
  if (s) {
    if (GWEN_Buffer_GetUsedBytes(argBuffer))
      GWEN_Buffer_AppendString(argBuffer, " ");
    GWEN_Buffer_AppendString(argBuffer, s);
  }
  s=GWEN_DB_GetCharValue(GWB_Context_GetVars(context), "local/cflags", 0, NULL);
  if (s) {
    if (GWEN_Buffer_GetUsedBytes(argBuffer))
      GWEN_Buffer_AppendString(argBuffer, " ");
    GWEN_Buffer_AppendString(argBuffer, s);
  }
  if (GWEN_Buffer_GetUsedBytes(argBuffer))
    GWEN_Buffer_AppendString(argBuffer, " ");
  GWEN_Buffer_AppendString(argBuffer, "-c -fPIC -o conftest.o conftest.c");


  rv=GWEN_Process_RunCommandWaitAndGather(toolName, GWEN_Buffer_GetStart(argBuffer), stdOutBuffer, stdErrBuffer);
  if (rv<0) {
    DBG_ERROR(NULL, "Error running gcc (%d)", rv);
    GWEN_Buffer_free(argBuffer);
    GWEN_Buffer_free(stdErrBuffer);
    GWEN_Buffer_free(stdOutBuffer);
    unlink("conftest.c");
    return rv;
  }
  GWEN_Buffer_free(argBuffer);
  GWEN_Buffer_free(stdErrBuffer);
  GWEN_Buffer_free(stdOutBuffer);

  unlink("conftest.c");
  unlink("conftest.o");

  return rv;
}



int GWB_Tools_CheckCompilerArgs(GWB_CONTEXT *context, const char *compilerArgs)
{
  GWEN_BUFFER *stdOutBuffer;
  GWEN_BUFFER *stdErrBuffer;
  const char *toolName;
  int rv;
  const char *testCode="int main(int argc, char **argv) { return 0;};\n";
  GWEN_BUFFER *argBuffer;


  toolName=GWEN_DB_GetCharValue(GWB_Context_GetVars(context), "GWBUILD_TOOL_CC", 0, "gcc");
  if (!(toolName && *toolName)) {
    DBG_ERROR(NULL, "No tool name for \"CC\"");
    return GWEN_ERROR_INTERNAL;
  }

  rv=GWEN_SyncIo_Helper_WriteFile("conftest.c", (const uint8_t*) testCode, strlen(testCode));
  if (rv<0) {
    DBG_ERROR(NULL, "Error writing conftest.c (%d)", rv);
    return rv;
  }

  stdOutBuffer=GWEN_Buffer_new(0, 256, 0, 1);
  stdErrBuffer=GWEN_Buffer_new(0, 256, 0, 1);

  argBuffer=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(argBuffer, compilerArgs);
  GWEN_Buffer_AppendString(argBuffer, " -c -fPIC conftest.c -o conftest.o");

  rv=GWEN_Process_RunCommandWaitAndGather(toolName, GWEN_Buffer_GetStart(argBuffer), stdOutBuffer, stdErrBuffer);
  if (rv<0) {
    DBG_ERROR(NULL, "Error running gcc (%d)", rv);
    GWEN_Buffer_free(argBuffer);
    GWEN_Buffer_free(stdErrBuffer);
    GWEN_Buffer_free(stdOutBuffer);
    unlink("conftest.c");
    return rv;
  }
  GWEN_Buffer_free(argBuffer);
  GWEN_Buffer_free(stdErrBuffer);
  GWEN_Buffer_free(stdOutBuffer);

  unlink("conftest.c");
  unlink("conftest.o");

  return rv;
}



