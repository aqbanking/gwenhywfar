/***************************************************************************
 begin       : Mon Feb 22 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef HTMLOBJECT_GRIDENTRY_P_H
#define HTMLOBJECT_GRIDENTRY_P_H


#include "o_gridentry_l.h"


typedef struct OBJECT_GRIDENTRY OBJECT_GRIDENTRY;
struct OBJECT_GRIDENTRY {
  int row;
  int column;
  int isHeader;
};
static void GWENHYWFAR_CB HtmlObject_GridEntry_FreeData(void *bp, void *p);


#endif

