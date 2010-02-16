/***************************************************************************
 begin       : Tue Feb 16 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_GUI_PROGRESSDATA_P_H
#define GWEN_GUI_PROGRESSDATA_P_H


#include "progressdata_l.h"


struct GWEN_PROGRESS_DATA {
  GWEN_TREE_ELEMENT(GWEN_PROGRESS_DATA);
  GWEN_GUI *gui;
  uint32_t id;
  uint32_t flags;
  char *title;
  char *text;
  uint64_t total;
  uint64_t current;
  GWEN_BUFFER *logBuf;
  int aborted;
  int shown;
  time_t startTime;

  GWEN_DIALOG *dialog;
};




#endif



