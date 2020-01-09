/***************************************************************************
 begin       : Thu Jan 09 2020
 copyright   : (C) 2020 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_TEST_GUI_P_H
#define GWEN_TEST_GUI_P_H


#include "testgui_l.h"

#include <gwenhywfar/stringlist.h>
#include <gwenhywfar/gui_be.h>



typedef struct GWEN_GUI_TEST GWEN_GUI_TEST;
struct GWEN_GUI_TEST {
  GWEN_STRINGLIST *logStrings;

  int logHookIsActive;
  char *logFile;
  int logLastX;

  GWEN_GUI_LOG_HOOK_FN previousLogHook;
};


#endif
