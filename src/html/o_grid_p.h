/***************************************************************************
 begin       : Mon Feb 22 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef HTMLOBJECT_GRID_P_H
#define HTMLOBJECT_GRID_P_H


#include "o_grid_l.h"


HTML_OBJECT *HtmlObject_Grid_new();


typedef struct OBJECT_GRID OBJECT_GRID;
struct OBJECT_GRID {
  int rows;
  int columns;
};
static void GWENHYWFAR_CB HtmlObject_Grid_FreeData(void *bp, void *p);


#endif

