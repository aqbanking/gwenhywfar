/***************************************************************************
    begin       : Wed Feb 03 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston,                 *
 *   MA  02111-1307  USA                                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define DISABLE_DEBUGLOG


#include "gwenthread_p.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>

#include <string.h>
#include <errno.h>



GWEN_INHERIT_FUNCTIONS(GWEN_THREAD)
GWEN_LIST_FUNCTIONS(GWEN_THREAD, GWEN_Thread)



static DWORD WINAPI _threadRun_cb(LPVOID lpParam);





GWEN_THREAD *GWEN_Thread_new()
{
  GWEN_THREAD *thr;

  GWEN_NEW_OBJECT(GWEN_THREAD, thr);
  assert(thr);

  GWEN_INHERIT_INIT(GWEN_THREAD, thr);
  GWEN_LIST_INIT(GWEN_THREAD, thr);

  return thr;
}



void GWEN_Thread_free(GWEN_THREAD *thr)
{
  if (thr) {
    GWEN_LIST_FINI(GWEN_THREAD, thr);
    GWEN_INHERIT_FINI(GWEN_THREAD, thr);
    GWEN_FREE_OBJECT(thr);
  }
}



int GWEN_Thread_Start(GWEN_THREAD *thr)
{
  if (thr->runFn==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No run function set in thread");
    return GWEN_ERROR_NOT_IMPLEMENTED;
  }

  thr->threadHandle=CreateThread(NULL,              /* default security attributes */
                                 0,                 /* use default stack size */
                                 _threadRun_cb,     /* thread function */
                                 thr,               /* argument to thread function */
                                 0,                 /* default creation flags */
                                 &(thr->threadId)); /* pointer to storage of thread id */

  if (thr->threadHandle==NULL) {
    DWORD lastError;

    lastError=GetLastError();
    DBG_ERROR(GWEN_LOGDOMAIN, "Error on CreateThread: %d (%s)", (int) lastError, h_strerror(lastError));
    return GWEN_ERROR_GENERIC;
  }

  return 0;
}



int GWEN_Thread_Join(GWEN_THREAD *thr)
{
  DWORD rv;

  rv=WaitForSingleObject(thr->threadHandle, INFINITE);
  if (rv!=WAIT_OBJECT_0) {
    switch(rv) {
    case WAIT_ABANDONED:
      DBG_ERROR(GWEN_LOGDOMAIN, "Error on WaitForSingleObject: Abandoned");
      break;
    case WAIT_TIMEOUT:
      DBG_ERROR(GWEN_LOGDOMAIN, "Error on WaitForSingleObject: Timeout");
      break;
    case WAIT_FAILED:
      {
        DWORD lastError;

        lastError=GetLastError();
        DBG_ERROR(GWEN_LOGDOMAIN, "Error on WaitForSingleObject: %d (%s)", (int) lastError, h_strerror(lastError));
        break;
      }
    default:
      DBG_ERROR(GWEN_LOGDOMAIN, "Unknown error on WaitForSingleObject: %d", (int) rv);
      break;
    }

    return GWEN_ERROR_GENERIC;
  }

  CloseHandle(thr->threadHandle);

  return 0;
}



GWEN_THREAD_RUN_FN GWEN_Thread_SetRunFn(GWEN_THREAD *thr, GWEN_THREAD_RUN_FN fn)
{
  GWEN_THREAD_RUN_FN oldFn;

  assert(thr);

  oldFn=thr->runFn;
  thr->runFn=fn;

  return oldFn;
}



DWORD _threadRun_cb(LPVOID lpParam);
{
  GWEN_THREAD *thr;

  thr=(GWEN_THREAD*) lpParam;
  assert(thr);

  if (thr->runFn)
    thr->runFn(thr);
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "No run function set in thread");
  }

  return NULL;
}










