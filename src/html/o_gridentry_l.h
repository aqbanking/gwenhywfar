/***************************************************************************
 begin       : Mon Feb 22 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef HTMLOBJECT_GRIDENTRY_L_H
#define HTMLOBJECT_GRIDENTRY_L_H


#include "htmlobject_be.h"


HTML_OBJECT *HtmlObject_GridEntry_new(GWEN_XML_CONTEXT *ctx);

int HtmlObject_GridEntry_GetRow(const HTML_OBJECT *o);
void HtmlObject_GridEntry_SetRow(HTML_OBJECT *o, int i);

int HtmlObject_GridEntry_GetColumn(const HTML_OBJECT *o);
void HtmlObject_GridEntry_SetColumn(HTML_OBJECT *o, int i);

int HtmlObject_GridEntry_GetIsHeader(const HTML_OBJECT *o);
void HtmlObject_GridEntry_SetIsHeader(HTML_OBJECT *o, int i);



#endif

