/***************************************************************************
 begin       : Thu Feb 04 2021
 copyright   : (C) 2021 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "testthread_p.h"

#include <gwenhywfar/nogui.h>
#include <gwenhywfar/debug.h>

#include <unistd.h>



GWEN_INHERIT(GWEN_THREAD, TEST_THREAD)



static void GWENHYWFAR_CB _freeData(GWEN_UNUSED void *bp, void *p);
static GWENHYWFAR_CB void _threadRun_cb(GWEN_UNUSED GWEN_THREAD *thr);



GWEN_THREAD *TestThread_new(int threadId, int loops)
{
  GWEN_THREAD *thr;
  TEST_THREAD *xthr;

  thr=GWEN_Thread_new();
  GWEN_NEW_OBJECT(TEST_THREAD, xthr);

  GWEN_INHERIT_SETDATA(GWEN_THREAD, TEST_THREAD, thr, xthr, _freeData);

  xthr->threadId=threadId;
  xthr->loops=loops;

  GWEN_Thread_SetRunFn(thr, _threadRun_cb);

  return thr;
}



void _freeData(GWEN_UNUSED void *bp, void *p)
{
  TEST_THREAD *xthr;

  xthr=(TEST_THREAD*) p;
  GWEN_FREE_OBJECT(xthr);
}



void _threadRun_cb(GWEN_UNUSED GWEN_THREAD *thr)
{
  GWEN_GUI *gui;
  TEST_THREAD *xthr;
  int i;

  xthr=GWEN_INHERIT_GETDATA(GWEN_THREAD, TEST_THREAD, thr);
  assert(xthr);

  sleep(2);

  gui=GWEN_NoGui_new();
  GWEN_Gui_SetGui(gui);

  for (i=0; i<xthr->loops; i++) {
    DBG_ERROR(GWEN_LOGDOMAIN, "GUI: Thread %2d: Loop %d", xthr->threadId, i);
    fprintf(stdout, "STD: Thread %2d: Loop %d\n", xthr->threadId, i);
  }

  GWEN_Gui_SetGui(NULL);
}



