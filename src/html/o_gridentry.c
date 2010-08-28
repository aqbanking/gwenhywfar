/***************************************************************************
 begin       : Mon Feb 22 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define DISABLE_DEBUGLOG


#include "o_gridentry_p.h"
#include "o_box_l.h"



GWEN_INHERIT(HTML_OBJECT, OBJECT_GRIDENTRY);




HTML_OBJECT *HtmlObject_GridEntry_new(GWEN_XML_CONTEXT *ctx) {
  HTML_OBJECT *o;
  OBJECT_GRIDENTRY *xo;

  /* derived from box object */
  o=HtmlObject_Box_new(ctx);
  /* extend box as grid entry */
  HtmlObject_SetObjectType(o, HtmlObjectType_GridEntry);
  GWEN_NEW_OBJECT(OBJECT_GRIDENTRY, xo);
  GWEN_INHERIT_SETDATA(HTML_OBJECT, OBJECT_GRIDENTRY, o, xo, HtmlObject_GridEntry_FreeData);

  return o;
}



void GWENHYWFAR_CB HtmlObject_GridEntry_FreeData(void *bp, void *p) {
  OBJECT_GRIDENTRY *xo;

  xo=(OBJECT_GRIDENTRY*) p;

  GWEN_FREE_OBJECT(xo);
}



int HtmlObject_GridEntry_GetRow(const HTML_OBJECT *o) {
  OBJECT_GRIDENTRY *xo;

  assert(o);
  xo=GWEN_INHERIT_GETDATA(HTML_OBJECT, OBJECT_GRIDENTRY, o);
  assert(xo);

  return xo->row;
}



void HtmlObject_GridEntry_SetRow(HTML_OBJECT *o, int i) {
  OBJECT_GRIDENTRY *xo;

  assert(o);
  xo=GWEN_INHERIT_GETDATA(HTML_OBJECT, OBJECT_GRIDENTRY, o);
  assert(xo);

  xo->row=i;
}



int HtmlObject_GridEntry_GetColumn(const HTML_OBJECT *o) {
  OBJECT_GRIDENTRY *xo;

  assert(o);
  xo=GWEN_INHERIT_GETDATA(HTML_OBJECT, OBJECT_GRIDENTRY, o);
  assert(xo);

  return xo->column;
}



void HtmlObject_GridEntry_SetColumn(HTML_OBJECT *o, int i) {
  OBJECT_GRIDENTRY *xo;

  assert(o);
  xo=GWEN_INHERIT_GETDATA(HTML_OBJECT, OBJECT_GRIDENTRY, o);
  assert(xo);

  xo->column=i;
}



int HtmlObject_GridEntry_GetIsHeader(const HTML_OBJECT *o) {
  OBJECT_GRIDENTRY *xo;

  assert(o);
  xo=GWEN_INHERIT_GETDATA(HTML_OBJECT, OBJECT_GRIDENTRY, o);
  assert(xo);

  return xo->isHeader;
}



void HtmlObject_GridEntry_SetIsHeader(HTML_OBJECT *o, int i) {
  OBJECT_GRIDENTRY *xo;

  assert(o);
  xo=GWEN_INHERIT_GETDATA(HTML_OBJECT, OBJECT_GRIDENTRY, o);
  assert(xo);

  xo->isHeader=i;
}








