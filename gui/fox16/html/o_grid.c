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

#include "o_grid_p.h"


GWEN_INHERIT(HTML_OBJECT, OBJECT_GRID);




HTML_OBJECT *HtmlObject_Grid_new(GWEN_XML_CONTEXT *ctx) {
  HTML_OBJECT *o;
  OBJECT_GRID *xo;

  o=HtmlObject_new(ctx, HtmlObjectType_Grid);
  GWEN_NEW_OBJECT(OBJECT_GRID, xo);
  GWEN_INHERIT_SETDATA(HTML_OBJECT, OBJECT_GRID, o, xo, HtmlObject_Grid_FreeData);

  HtmlObject_AddFlags(o,
		      HTML_OBJECT_FLAGS_START_ON_NEWLINE |
		      HTML_OBJECT_FLAGS_END_WITH_NEWLINE);

  return o;
}



void HtmlObject_Grid_FreeData(void *bp, void *p) {
  OBJECT_GRID *xo;

  xo=(OBJECT_GRID*) p;

  GWEN_FREE_OBJECT(xo);
}



int HtmlObject_Grid_GetRows(const HTML_OBJECT *o) {
  OBJECT_GRID *xo;

  assert(o);
  xo=GWEN_INHERIT_GETDATA(HTML_OBJECT, OBJECT_GRID, o);
  assert(xo);

  return xo->rows;
}



void HtmlObject_Grid_SetRows(HTML_OBJECT *o, int i) {
  OBJECT_GRID *xo;

  assert(o);
  xo=GWEN_INHERIT_GETDATA(HTML_OBJECT, OBJECT_GRID, o);
  assert(xo);

  xo->rows=i;
}



int HtmlObject_Grid_GetColumns(const HTML_OBJECT *o) {
  OBJECT_GRID *xo;

  assert(o);
  xo=GWEN_INHERIT_GETDATA(HTML_OBJECT, OBJECT_GRID, o);
  assert(xo);

  return xo->columns;
}



void HtmlObject_Grid_SetColumns(HTML_OBJECT *o, int i) {
  OBJECT_GRID *xo;

  assert(o);
  xo=GWEN_INHERIT_GETDATA(HTML_OBJECT, OBJECT_GRID, o);
  assert(xo);

  xo->columns=i;
}






