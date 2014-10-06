/***************************************************************************
 begin       : Tue Feb 16 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_GUI_PROGRESSDATA_L_H
#define GWEN_GUI_PROGRESSDATA_L_H


#include <gwenhywfar/tree.h>
#include <gwenhywfar/gui_be.h>
#include <gwenhywfar/buffer.h>

#include <time.h>



typedef struct GWEN_PROGRESS_DATA GWEN_PROGRESS_DATA;
GWEN_TREE_FUNCTION_DEFS(GWEN_PROGRESS_DATA, GWEN_ProgressData)


GWEN_PROGRESS_DATA *GWEN_ProgressData_new(GWEN_GUI *gui,
    uint32_t id,
    uint32_t progressFlags,
    const char *title,
    const char *text,
    uint64_t total);

void GWEN_ProgressData_free(GWEN_PROGRESS_DATA *pd);


GWEN_GUI *GWEN_ProgressData_GetGui(const GWEN_PROGRESS_DATA *pd);

uint32_t GWEN_ProgressData_GetId(const GWEN_PROGRESS_DATA *pd);

uint32_t GWEN_ProgressData_GetPreviousId(const GWEN_PROGRESS_DATA *pd);
void GWEN_ProgressData_SetPreviousId(GWEN_PROGRESS_DATA *pd, uint32_t i);

uint32_t GWEN_ProgressData_GetFlags(const GWEN_PROGRESS_DATA *pd);
void GWEN_ProgressData_AddFlags(GWEN_PROGRESS_DATA *pd, uint32_t fl);
void GWEN_ProgressData_SubFlags(GWEN_PROGRESS_DATA *pd, uint32_t fl);

const char *GWEN_ProgressData_GetTitle(const GWEN_PROGRESS_DATA *pd);

const char *GWEN_ProgressData_GetText(const GWEN_PROGRESS_DATA *pd);

uint64_t GWEN_ProgressData_GetTotal(const GWEN_PROGRESS_DATA *pd);
void GWEN_ProgressData_SetTotal(GWEN_PROGRESS_DATA *pd, uint64_t i);

uint64_t GWEN_ProgressData_GetCurrent(const GWEN_PROGRESS_DATA *pd);
void GWEN_ProgressData_SetCurrent(GWEN_PROGRESS_DATA *pd, uint64_t i);



const char *GWEN_ProgressData_GetLogText(const GWEN_PROGRESS_DATA *pd);
void GWEN_ProgressData_ClearLogText(GWEN_PROGRESS_DATA *pd);
void GWEN_ProgressData_AddLogText(GWEN_PROGRESS_DATA *pd,
                                  GWEN_LOGGER_LEVEL level,
                                  const char *s);

int GWEN_ProgressData_GetAborted(const GWEN_PROGRESS_DATA *pd);
void GWEN_ProgressData_SetAborted(GWEN_PROGRESS_DATA *pd, int i);

int GWEN_ProgressData_GetShown(const GWEN_PROGRESS_DATA *pd);
void GWEN_ProgressData_SetShown(GWEN_PROGRESS_DATA *pd, int i);

time_t GWEN_ProgressData_GetStartTime(const GWEN_PROGRESS_DATA *pd);
void GWEN_ProgressData_SetStartTime(GWEN_PROGRESS_DATA *pd, time_t t);

time_t GWEN_ProgressData_GetCheckTime(const GWEN_PROGRESS_DATA *pd);
void GWEN_ProgressData_SetCheckTime(GWEN_PROGRESS_DATA *pd, time_t t);


GWEN_PROGRESS_DATA *GWEN_ProgressData_Tree_FindProgressById(GWEN_PROGRESS_DATA_TREE *pt, uint32_t id);


GWEN_DIALOG *GWEN_ProgressData_GetDialog(const GWEN_PROGRESS_DATA *pd);
void GWEN_ProgressData_SetDialog(GWEN_PROGRESS_DATA *pd, GWEN_DIALOG *dlg);



#endif



