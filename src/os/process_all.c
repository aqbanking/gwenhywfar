/***************************************************************************
    begin       : Wed Mar 24 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "Gwenhywfar".                          *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define DISABLE_DEBUGLOG

#include "process_l.h"

#include <gwenhywfar/process.h>

#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>

#include <unistd.h>
#include <time.h>




int GWEN_Process_RunCommandWaitAndGather(const char *prg, const char *args,
					 GWEN_BUFFER *stdOutBuffer,
					 GWEN_BUFFER *stdErrBuffer)
{
#if GWENHYWFAR_SYS_IS_WINDOWS
  /* not supported for now */
  return GWEN_ERROR_NOT_SUPPORTED;
#else
  GWEN_PROCESS *pr;
  GWEN_PROCESS_STATE state;
  int rv;

  pr=GWEN_Process_new();
  if (stdOutBuffer)
    GWEN_Process_AddFlags(pr, GWEN_PROCESS_FLAGS_REDIR_STDOUT);
  if (stdErrBuffer)
    GWEN_Process_AddFlags(pr, GWEN_PROCESS_FLAGS_REDIR_STDERR);

  state=GWEN_Process_Start(pr, prg, args);
  if (state!=GWEN_ProcessStateRunning) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error starting process (%d)", state);
    GWEN_Process_free(pr);
    return GWEN_ERROR_GENERIC;
  }

  rv=GWEN_Process_WaitAndRead(pr, stdOutBuffer, stdErrBuffer);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Process_free(pr);
    return rv;
  }

  rv=GWEN_Process_GetResult(pr);
  GWEN_Process_free(pr);
  return rv;
#endif
}






