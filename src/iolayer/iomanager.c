/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: crypttoken.h 1113 2007-01-10 09:14:16Z martin $
    begin       : Wed Mar 16 2005
    copyright   : (C) 2005 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "iomanager_l.h"
#include "iolayer_l.h"

#include "i18n_l.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/gwentime.h>

#include <assert.h>

#include <gnutls/gnutls.h>



static GWEN_IO_LAYER_LIST *ioLayerList=NULL;



int GWEN_Io_Manager_ModuleInit() {
  ioLayerList=GWEN_Io_Layer_List_new();

  gnutls_global_init();

  return 0;
}



int GWEN_Io_Manager_ModuleFini() {
  if (ioLayerList) {
    GWEN_Io_Layer_List_free(ioLayerList);
    ioLayerList=NULL;
  }

  gnutls_global_deinit();

  return 0;
}



int GWEN_Io_Manager_RegisterLayer(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER *p;

  /* check whether the given layer exists in our list */
  p=GWEN_Io_Layer_List_First(ioLayerList);
  while(p) {
    if (p==io) {
      DBG_INFO(GWEN_LOGDOMAIN, "IO Layer %p already registered", (void*)io);
      return GWEN_ERROR_INVALID;
    }
    p=GWEN_Io_Layer_List_Next(p);
  }

  GWEN_Io_Layer_List_Add(io, ioLayerList);
  //GWEN_Io_Layer_Attach(io);
  GWEN_Io_Layer_AddFlags(io, GWEN_IO_LAYER_FLAGS_REGISTERED);
  return 0;
}



int GWEN_Io_Manager_UnregisterLayer(GWEN_IO_LAYER *io) {
  GWEN_IO_LAYER *p;

  /* check whether the given layer exists in our list */
  p=GWEN_Io_Layer_List_First(ioLayerList);
  while(p) {
    if (p==io)
      break;
    p=GWEN_Io_Layer_List_Next(p);
  }

  if (p!=io) {
    DBG_INFO(GWEN_LOGDOMAIN, "IO Layer %p not registered", (void*)io);
    return GWEN_ERROR_INVALID;
  }

  GWEN_Io_Layer_SubFlags(io, GWEN_IO_LAYER_FLAGS_REGISTERED);
  GWEN_Io_Layer_List_Del(io);
  //GWEN_Io_Layer_free(io);

  return 0;
}



GWEN_IO_LAYER_WORKRESULT GWEN_Io_Manager_Work(void) {
  int anythingDone=0;
  int i;

  for (i=0;i<10;i++) {
    GWEN_IO_LAYER *p;
    int somethingDone=0;

    p=GWEN_Io_Layer_List_First(ioLayerList);
    while(p) {
      GWEN_IO_LAYER_WORKRESULT res;

      res=GWEN_Io_Layer_WorkOnRequests(p);
      if (res==GWEN_Io_Layer_WorkResultOk)
	somethingDone=1;

      p=GWEN_Io_Layer_List_Next(p);
    }
    if (somethingDone)
      anythingDone++;
    else
      break;
  }

  return anythingDone?GWEN_Io_Layer_WorkResultOk:GWEN_Io_Layer_WorkResultBlocking;
}



int GWEN_Io_Manager_Wait(int msecs, uint32_t guiid) {
  GWEN_IO_LAYER *p;
  GWEN_SOCKET_LIST2 *readSockets;
  GWEN_SOCKET_LIST2 *writeSockets;
  int rv;

  /* populate socket lists */
  readSockets=GWEN_Socket_List2_new();
  writeSockets=GWEN_Socket_List2_new();

  p=GWEN_Io_Layer_List_First(ioLayerList);
  while(p) {
    GWEN_Io_Layer_AddWaitingSockets(p, readSockets, writeSockets);
    p=GWEN_Io_Layer_List_Next(p);
  }

  rv=GWEN_Gui_WaitForSockets(readSockets, writeSockets, guiid, msecs);
  GWEN_Socket_List2_free(writeSockets);
  GWEN_Socket_List2_free(readSockets);

  if (rv) {
    if (rv!=GWEN_ERROR_EOF) {
      DBG_INFO(GWEN_LOGDOMAIN, "WAITFORSOCKETS: %d", rv);
    }
  }

  return rv;
}



int GWEN_Io_Manager_WaitForRequest(GWEN_IO_REQUEST *r, int msecs) {
  GWEN_TIME *t0;
  uint32_t progressId;
  uint64_t timeout;
  const char *s;
  GWEN_IO_LAYER_WORKRESULT res;

  if (GWEN_Io_Request_GetStatus(r)==GWEN_Io_Request_StatusFinished) {
    /* request already finished */
    return 0;
  }

  if (msecs==GWEN_TIMEOUT_NONE) {
    /* no timeout, return immediately */
    DBG_INFO(GWEN_LOGDOMAIN, "No timeout, returning immediately");
    return GWEN_ERROR_TIMEOUT;
  }

  /* first shot: in many cases one work call is enough */
  res=GWEN_Io_Manager_Work();
  if (GWEN_Io_Request_GetStatus(r)==GWEN_Io_Request_StatusFinished) {
    return 0;
  }

  /* ok, we have to go the long way */
  t0=GWEN_CurrentTime();
  assert(t0);

  if (msecs==GWEN_TIMEOUT_NONE ||
      msecs==GWEN_TIMEOUT_FOREVER)
    timeout=0;
  else
    timeout=msecs;

  switch(GWEN_Io_Request_GetType(r)) {
  case GWEN_Io_Request_TypeConnect:
    s=I18N("Connecting...");
    break;
  case GWEN_Io_Request_TypeDisconnect:
    s=I18N("Disconnecting...");
    break;
  case GWEN_Io_Request_TypeRead:
    s=I18N("Reading...");
    break;
  case GWEN_Io_Request_TypeWrite:
    s=I18N("Writing...");
    break;
  default:
    s=I18N("Waiting for request to finish...");
  }

  progressId=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_DELAY |
				    GWEN_GUI_PROGRESS_ALLOW_EMBED |
				    GWEN_GUI_PROGRESS_SHOW_PROGRESS |
				    GWEN_GUI_PROGRESS_SHOW_ABORT,
				    s,
				    NULL,
				    timeout,
				    GWEN_Io_Request_GetGuiId(r));

  for (;;) {
    int rv;
    int i;
    int shortTimeout=msecs;

    /* first: work as long as we can /and check status on the way) */
    for (i=0; i<10000; i++) {
      if (GWEN_Io_Request_GetStatus(r)==GWEN_Io_Request_StatusFinished) {
	/* request finished */
	GWEN_Gui_ProgressEnd(progressId);
	GWEN_Time_free(t0);
	return 0;
      }
      res=GWEN_Io_Manager_Work();
      if (res==GWEN_Io_Layer_WorkResultBlocking)
	break;
      if (res==GWEN_Io_Layer_WorkResultError) {
        DBG_INFO(GWEN_LOGDOMAIN, "IO error (res=%d)", res);
	GWEN_Gui_ProgressLog(progressId,
			     GWEN_LoggerLevel_Error,
			     I18N("IO error"));
	GWEN_Gui_ProgressEnd(progressId);
	GWEN_Time_free(t0);
	return GWEN_ERROR_IO;
      }
    }

    if (msecs!=GWEN_TIMEOUT_FOREVER) {
      GWEN_TIME *t1;
      int d;

      /* check for timeout */
      t1=GWEN_CurrentTime();
      assert(t1);
      d=(int)GWEN_Time_Diff(t1, t0);
      GWEN_Time_free(t1);
      if (d>=msecs) {
	DBG_INFO(GWEN_LOGDOMAIN, "Timeout (%d>%d)", d, msecs);
	GWEN_Gui_ProgressEnd(progressId);
	GWEN_Time_free(t0);
	return GWEN_ERROR_TIMEOUT;
      }
      /* calculate remaining time */
      shortTimeout=msecs-d;
    }

    /* if still not blocking (i.e. there still are things to do) we might have an error in one
     * of the io layers (e.g. an ioi layer always reports non-blocking even though it had nothing
     * left to do)
     */
    if (res!=GWEN_Io_Layer_WorkResultBlocking) {
      /* sleep for a feew ms if the result is still non-blocking */
      DBG_WARN(GWEN_LOGDOMAIN, "Still working after %d loops, sleeping", i);
      GWEN_Socket_Select(NULL, NULL, NULL, GWEN_GUI_CPU_TIMEOUT);
    }
    else {
      /* wait for changes on sockets */
      rv=GWEN_Io_Manager_Wait(shortTimeout, progressId);
      if (rv!=0 && rv!=GWEN_ERROR_TIMEOUT) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	GWEN_Gui_ProgressLog(progressId,
			     GWEN_LoggerLevel_Error,
			     I18N("IO error"));
	GWEN_Gui_ProgressEnd(progressId);
	GWEN_Time_free(t0);
	return rv;
      }
    }
  } /* for */
}






