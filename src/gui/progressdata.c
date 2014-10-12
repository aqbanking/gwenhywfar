/***************************************************************************
 begin       : Tue Feb 16 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define DISABLE_DEBUGLOG


#include "progressdata_p.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>




GWEN_TREE_FUNCTIONS(GWEN_PROGRESS_DATA, GWEN_ProgressData)




GWEN_PROGRESS_DATA *GWEN_ProgressData_new(GWEN_GUI *gui,
    uint32_t id,
    uint32_t progressFlags,
    const char *title,
    const char *text,
    uint64_t total) {
  GWEN_PROGRESS_DATA *pd;

  GWEN_NEW_OBJECT(GWEN_PROGRESS_DATA, pd);
  GWEN_TREE_INIT(GWEN_PROGRESS_DATA, pd);

  pd->gui=gui;
  pd->id=id;
  pd->flags=progressFlags;
  if (title)
    pd->title=strdup(title);
  if (text)
    pd->text=strdup(text);
  pd->total=total;

  pd->logBuf=GWEN_Buffer_new(0, 1024, 0, 1);
  pd->startTime=time(0);

  return pd;
}



void GWEN_ProgressData_free(GWEN_PROGRESS_DATA *pd) {
  if (pd) {
    GWEN_TREE_FINI(GWEN_PROGRESS_DATA, pd);
    free(pd->title);
    free(pd->text);
    GWEN_Buffer_free(pd->logBuf);
    GWEN_FREE_OBJECT(pd);
  }
}



GWEN_GUI *GWEN_ProgressData_GetGui(const GWEN_PROGRESS_DATA *pd) {
  assert(pd);
  return pd->gui;
}



uint32_t GWEN_ProgressData_GetId(const GWEN_PROGRESS_DATA *pd) {
  assert(pd);
  return pd->id;
}



uint32_t GWEN_ProgressData_GetPreviousId(const GWEN_PROGRESS_DATA *pd) {
  assert(pd);
  return pd->previousId;
}



void GWEN_ProgressData_SetPreviousId(GWEN_PROGRESS_DATA *pd, uint32_t i) {
  assert(pd);
  pd->previousId=i;
}



uint32_t GWEN_ProgressData_GetFlags(const GWEN_PROGRESS_DATA *pd) {
  assert(pd);
  return pd->flags;
}



void GWEN_ProgressData_AddFlags(GWEN_PROGRESS_DATA *pd, uint32_t fl) {
  assert(pd);
  pd->flags|=fl;
}



void GWEN_ProgressData_SubFlags(GWEN_PROGRESS_DATA *pd, uint32_t fl) {
  assert(pd);
  pd->flags&=~fl;
}



const char *GWEN_ProgressData_GetTitle(const GWEN_PROGRESS_DATA *pd) {
  assert(pd);
  return pd->title;
}



const char *GWEN_ProgressData_GetText(const GWEN_PROGRESS_DATA *pd) {
  assert(pd);
  return pd->text;
}



uint64_t GWEN_ProgressData_GetTotal(const GWEN_PROGRESS_DATA *pd) {
  assert(pd);
  return pd->total;
}



uint64_t GWEN_ProgressData_GetCurrent(const GWEN_PROGRESS_DATA *pd) {
  assert(pd);
  return pd->current;
}



void GWEN_ProgressData_SetCurrent(GWEN_PROGRESS_DATA *pd, uint64_t i) {
  assert(pd);
  pd->current=i;
}



void GWEN_ProgressData_SetTotal(GWEN_PROGRESS_DATA *pd, uint64_t i) {
  assert(pd);
  pd->total=i;
}



GWEN_BUFFER *GWEN_ProgressData_GetLogBuf(const GWEN_PROGRESS_DATA *pd) {
  assert(pd);
  return pd->logBuf;
}



const char *GWEN_ProgressData_GetLogText(const GWEN_PROGRESS_DATA *pd) {
  assert(pd);
  return GWEN_Buffer_GetStart(pd->logBuf);
}



void GWEN_ProgressData_ClearLogText(GWEN_PROGRESS_DATA *pd) {
  assert(pd);
  GWEN_Buffer_Reset(pd->logBuf);
}



void GWEN_ProgressData_AddLogText(GWEN_PROGRESS_DATA *pd,
                                  GWEN_LOGGER_LEVEL level,
                                  const char *s) {
  assert(pd);
  GWEN_Buffer_AppendString(pd->logBuf, s);
}



int GWEN_ProgressData_GetAborted(const GWEN_PROGRESS_DATA *pd) {
  assert(pd);
  return pd->aborted;
}



void GWEN_ProgressData_SetAborted(GWEN_PROGRESS_DATA *pd, int i) {
  assert(pd);
  pd->aborted=i;
}



int GWEN_ProgressData_GetShown(const GWEN_PROGRESS_DATA *pd) {
  assert(pd);
  return pd->shown;
}



void GWEN_ProgressData_SetShown(GWEN_PROGRESS_DATA *pd, int i) {
  assert(pd);
  pd->shown=i;
}



time_t GWEN_ProgressData_GetStartTime(const GWEN_PROGRESS_DATA *pd) {
  assert(pd);
  return pd->startTime;
}



void GWEN_ProgressData_SetStartTime(GWEN_PROGRESS_DATA *pd, time_t t) {
  assert(pd);
  pd->startTime=t;
}



time_t GWEN_ProgressData_GetCheckTime(const GWEN_PROGRESS_DATA *pd) {
  assert(pd);
  return pd->checkTime;
}



void GWEN_ProgressData_SetCheckTime(GWEN_PROGRESS_DATA *pd, time_t t) {
  assert(pd);
  pd->checkTime=t;
}



GWEN_DIALOG *GWEN_ProgressData_GetDialog(const GWEN_PROGRESS_DATA *pd) {
  assert(pd);
  return pd->dialog;
}



void GWEN_ProgressData_SetDialog(GWEN_PROGRESS_DATA *pd, GWEN_DIALOG *dlg) {
  assert(pd);
  pd->dialog=dlg;
}



GWEN_PROGRESS_DATA *GWEN_ProgressData_Tree_FindProgressById(GWEN_PROGRESS_DATA_TREE *pt, uint32_t id) {
  GWEN_PROGRESS_DATA *pd;

  pd=GWEN_ProgressData_Tree_GetFirst(pt);
  while(pd) {
    if (GWEN_ProgressData_GetId(pd)==id)
      break;
    pd=GWEN_ProgressData_Tree_GetBelow(pd);
  }

  return pd;
}











