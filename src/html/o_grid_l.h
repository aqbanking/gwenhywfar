/***************************************************************************
 begin       : Mon Feb 22 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef HTMLOBJECT_GRID_L_H
#define HTMLOBJECT_GRID_L_H

#include "htmlobject_be.h"


HTML_OBJECT *HtmlObject_Grid_new(GWEN_XML_CONTEXT *ctx);

int HtmlObject_Grid_GetRows(const HTML_OBJECT *o);
void HtmlObject_Grid_SetRows(HTML_OBJECT *o, int i);

int HtmlObject_Grid_GetColumns(const HTML_OBJECT *o);
void HtmlObject_Grid_SetColumns(HTML_OBJECT *o, int i);



#endif

