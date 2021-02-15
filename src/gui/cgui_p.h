/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004-2017 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_GUI_CGUI_P_H
#define GWEN_GUI_CGUI_P_H



#define GWEN_GUI_CGUI_CHAR_ABORT  27
#define GWEN_GUI_CGUI_CHAR_DELETE 127
#define GWEN_GUI_CGUI_CHAR_ENTER  10

#include "cgui_l.h"
#include "cprogress_l.h"

#include <gwenhywfar/db.h>
#include <gwenhywfar/stringlist.h>




typedef struct GWEN_GUI_CGUI GWEN_GUI_CGUI;
struct GWEN_GUI_CGUI {
  GWEN_GUI_CPROGRESS_LIST *progressList;

  uint32_t nextBoxId;
  uint32_t nextProgressId;
};




void GWENHYWFAR_CB GWEN_Gui_CGui_FreeData(void *bp, void *p);



char GWEN_Gui_CGui__readCharFromStdin(int waitFor);
int GWEN_Gui_CGui__input(GWEN_GUI *gui,
                         uint32_t flags,
                         char *buffer,
                         int minLen,
                         int maxLen,
                         uint32_t guiid);

GWEN_GUI_CPROGRESS *GWEN_Gui_CGui__findProgress(GWEN_GUI *gui, uint32_t id);


int GWENHYWFAR_CB GWEN_Gui_CGui_MessageBox(GWEN_GUI *gui,
                             uint32_t flags,
                             const char *title,
                             const char *text,
                             const char *b1,
                             const char *b2,
                             const char *b3,
                             uint32_t guiid);

int GWENHYWFAR_CB GWEN_Gui_CGui_InputBox(GWEN_GUI *gui,
                           uint32_t flags,
                           const char *title,
                           const char *text,
                           char *buffer,
                           int minLen,
                           int maxLen,
                           uint32_t guiid);

uint32_t GWENHYWFAR_CB GWEN_Gui_CGui_ShowBox(GWEN_GUI *gui,
                               uint32_t flags,
                               const char *title,
                               const char *text,
                               uint32_t guiid);

void GWENHYWFAR_CB GWEN_Gui_CGui_HideBox(GWEN_GUI *gui, uint32_t id);

uint32_t GWENHYWFAR_CB GWEN_Gui_CGui_ProgressStart(GWEN_GUI *gui,
                                     uint32_t progressFlags,
                                     const char *title,
                                     const char *text,
                                     uint64_t total,
                                     uint32_t guiid);

int GWENHYWFAR_CB GWEN_Gui_CGui_ProgressAdvance(GWEN_GUI *gui,
                                  uint32_t id,
                                  uint64_t progress);

int GWENHYWFAR_CB GWEN_Gui_CGui_ProgressSetTotal(GWEN_GUI *gui, uint32_t id, uint64_t total);

int GWENHYWFAR_CB GWEN_Gui_CGui_ProgressLog(GWEN_GUI *gui,
                              uint32_t id,
                              GWEN_LOGGER_LEVEL level,
                              const char *text);

int GWENHYWFAR_CB GWEN_Gui_CGui_ProgressEnd(GWEN_GUI *gui, uint32_t id);

#endif


