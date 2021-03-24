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

#include <gwenhywfar/process.h>

#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>

#include <unistd.h>
#include <time.h>



static int _readAndAddToBuffer(GWEN_SYNCIO *sio, GWEN_BUFFER *buf);
static int _waitAndRead(GWEN_PROCESS *pr, GWEN_BUFFER *stdOutBuffer);




int GWEN_Process_RunCommandWaitAndGather(const char *prg, const char *args, GWEN_BUFFER *stdOutBuffer)
{
  GWEN_PROCESS *pr;
  GWEN_PROCESS_STATE state;
  int rv;

  pr=GWEN_Process_new();
  GWEN_Process_AddFlags(pr, GWEN_PROCESS_FLAGS_REDIR_STDOUT);

  state=GWEN_Process_Start(pr, prg, args);
  if (state!=GWEN_ProcessStateRunning) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error starting process (%d)", state);
    GWEN_Process_free(pr);
    return GWEN_ERROR_GENERIC;
  }

  rv=_waitAndRead(pr, stdOutBuffer);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Process_free(pr);
    return rv;
  }

  rv=GWEN_Process_GetResult(pr);
  GWEN_Process_free(pr);
  return rv;
}



int _waitAndRead(GWEN_PROCESS *pr, GWEN_BUFFER *stdOutBuffer)
{
  GWEN_SYNCIO *sioStdOut;
  int rv;
  int eofMet=0;

  sioStdOut=GWEN_Process_GetStdout(pr);

  for(;;) {
    GWEN_PROCESS_STATE state;

    if (!eofMet) {
      rv=_readAndAddToBuffer(sioStdOut, stdOutBuffer);
      if (rv<0) {
	if (rv==GWEN_ERROR_EOF) {
	  DBG_INFO(GWEN_LOGDOMAIN, "EOF met");
	  eofMet=1;
	}
	else {
          DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
          GWEN_Process_CheckState(pr); /* avoid zombies */
	  return rv;
	}
      }
    }

    state=GWEN_Process_CheckState(pr);
    if (state!=GWEN_ProcessStateRunning)
      break;
    if (eofMet)
      sleep(2);
  }

  if (!eofMet)
    _readAndAddToBuffer(sioStdOut, stdOutBuffer);
  return 0;
}



int _readAndAddToBuffer(GWEN_SYNCIO *sio, GWEN_BUFFER *buf)
{
  uint8_t localBuffer[1024];
  int rv;

  rv=GWEN_SyncIo_Read(sio, localBuffer, sizeof(localBuffer));
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  else if (rv==0) {
    DBG_INFO(GWEN_LOGDOMAIN, "EOF met");
    return GWEN_ERROR_EOF;
  }
  GWEN_Buffer_AppendBytes(buf, (const char*) localBuffer, rv);
  return rv;
}




