/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id: progress_p.h 409 2005-08-05 18:13:06Z aquamaniac $
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_GUI_CONSOLE_PROGRESS_H
#define GWEN_GUI_CONSOLE_PROGRESS_H

#define GWEN_GUI_CPROGRESS_CHAR_ABORT 27

#include "cprogress_l.h"

#include <time.h>


struct GWEN_GUI_CPROGRESS {
  GWEN_LIST_ELEMENT(GWEN_GUI_CPROGRESS)
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
};




#endif



