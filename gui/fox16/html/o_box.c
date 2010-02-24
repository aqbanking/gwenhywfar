/***************************************************************************
 begin       : Sat Feb 20 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "o_box_l.h"

#include <gwenhywfar/debug.h>



#define LINE_EXTRA_OFFSET 4



static int HtmlObject_Box_Layout(HTML_OBJECT *o) {
  HTML_OBJECT *c;
  int w;
  int h;
  int x=0;
  int y=0;
  int maxX=0;
  int lineHeight=0;
  int rv;

  w=HtmlObject_GetWidth(o);
  h=HtmlObject_GetHeight(o);

  c=HtmlObject_Tree_GetFirstChild(o);
  while(c) {
    int th;

    if ((HtmlObject_GetFlags(c) & HTML_OBJECT_FLAGS_START_ON_NEWLINE) &&
	x>0) {
      /* next line */
      if (x>maxX)
	maxX=x;
      x=0;
      y+=lineHeight+LINE_EXTRA_OFFSET;
      lineHeight=0;
    }

    HtmlObject_SetHeight(c, -1);
    if (w==-1) {
      HtmlObject_SetWidth(c, -1);
      rv=HtmlObject_Layout(c);
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }
    }
    else {
      int tw;

      tw=w-x;
      HtmlObject_SetWidth(c, tw);
      rv=HtmlObject_Layout(c);
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }

      if (HtmlObject_GetWidth(c)>tw && x>0) {
	/* next line */
	if (x>maxX)
	  maxX=x;
	x=0;
	y+=lineHeight+LINE_EXTRA_OFFSET;
        lineHeight=0;
      }
    }

    HtmlObject_SetX(c, x);
    HtmlObject_SetY(c, y);

    th=HtmlObject_GetHeight(c);
    if (th>lineHeight)
      lineHeight=th;

    x+=HtmlObject_GetWidth(c);

    if ((HtmlObject_GetFlags(c) & HTML_OBJECT_FLAGS_END_WITH_NEWLINE) &&
	x>0) {
      /* next line */
      if (x>maxX)
	maxX=x;
      x=0;
      y+=lineHeight+LINE_EXTRA_OFFSET;
      lineHeight=0;
    }

    c=HtmlObject_Tree_GetNext(c);
  }

  if (x>0) {
    /* next line */
    if (x>maxX)
      maxX=x;
    x=0;
    y+=lineHeight+LINE_EXTRA_OFFSET;
    lineHeight=0;
  }

  /* finish layout */
  HtmlObject_SetWidth(o, maxX+1);
  HtmlObject_SetHeight(o, y+1);
  return 0;
}



HTML_OBJECT *HtmlObject_Box_new(GWEN_XML_CONTEXT *ctx) {
  HTML_OBJECT *o;

  o=HtmlObject_new(ctx, HtmlObjectType_Box);
  HtmlObject_SetLayoutFn(o, HtmlObject_Box_Layout);

  return o;
}





