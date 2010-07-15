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
#include "o_gridentry_l.h"

#include <gwenhywfar/debug.h>



GWEN_INHERIT(HTML_OBJECT, OBJECT_GRID);


#define MAX_COLUMN     32
#define COLUMN_SPACING 4
#define ROW_SPACING    4



static int HtmlObject_Grid_Layout(HTML_OBJECT *o) {
  OBJECT_GRID *xo;
  HTML_OBJECT *c;
  int w;
  int h;
  int x;
  int y;
  int rv;
  int i;
  int j;
  int cw[MAX_COLUMN];
  int maxLineHeight;
  int maxLineWidth;
  int currentRow;

  assert(o);
  xo=GWEN_INHERIT_GETDATA(HTML_OBJECT, OBJECT_GRID, o);
  assert(xo);

  w=HtmlObject_GetWidth(o);
  h=HtmlObject_GetHeight(o);

  /* subtract spacing from available width */
  if (w!=-1)
    w-=(xo->columns+1)*COLUMN_SPACING;

  /* determine the maximum width of each column */
  for (i=0; i<xo->columns; i++)
    cw[i]=0;
  c=HtmlObject_Tree_GetFirstChild(o);
  while(c) {
    int k;

    i=HtmlObject_GridEntry_GetColumn(c);
    HtmlObject_SetHeight(c, -1);
    HtmlObject_SetWidth(c, -1);
    rv=HtmlObject_Layout(c);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    k=HtmlObject_GetWidth(c);
    if (k>cw[i])
      cw[i]=k;
    c=HtmlObject_Tree_GetNext(c);
  }

  if (w!=-1) {
    /* do the columns all fit into the width? */
    x=0;
    for (i=0; i<xo->columns; i++)
      x+=cw[i];

    if (x>w) {
      int fullw[MAX_COLUMN];
      int meanColumnWidth;
      int k;

      /* doesn't fit, so we need to adjust the columns */
      meanColumnWidth=w/xo->columns;


      /* reset full width of every column */
      for (i=0; i<xo->columns; i++)
	fullw[i]=0;
      /* calculate full width of every column */
      c=HtmlObject_Tree_GetFirstChild(o);
      while(c) {
	int k;
  
	i=HtmlObject_GridEntry_GetColumn(c);
	k=HtmlObject_GetWidth(c);
	if (k>fullw[i])
	  fullw[i]=k;
	c=HtmlObject_Tree_GetNext(c);
      }

      for (i=0; i<xo->columns; i++)
	cw[i]=0;

      /* set fixed widths to those columns which are smaller than fullWidth/columns */
      for (i=0; i<xo->columns; i++) {
	int p;

	p=fullw[i];
	if (p<=meanColumnWidth) {
          k+=p;
	  cw[i]=p;
	}
      }

      /* now get the remaining width */
      j=0;
      k=w-k;
      for (i=0; i<xo->columns; i++) {
        if (cw[i]==0)
	  j+=fullw[i];
      }

      /*DBG_ERROR(0, "Full width: %d, still needed space: %d, available space: %d", w, j, k);*/

      if (j>0) {
	/* calculate percentual width of each remaining column */
	for (i=0; i<xo->columns; i++) {
	  if (cw[i]==0) {
	    int p;

	    p=fullw[i]*100/j;
	    cw[i]=p*k/100;
	  }
	}
      }

      /* re-layout columns */
      c=HtmlObject_Tree_GetFirstChild(o);
      while(c) {
	i=HtmlObject_GridEntry_GetColumn(c);
	HtmlObject_SetHeight(c, -1);
	HtmlObject_SetWidth(c, cw[i]);
	rv=HtmlObject_Layout(c);
	if (rv<0) {
	  DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	  return rv;
	}
	c=HtmlObject_Tree_GetNext(c);
      }
    }
  }

  /* now layout elements according to column sizes */
  x=COLUMN_SPACING/2;
  y=ROW_SPACING/2;
  maxLineHeight=0;
  maxLineWidth=0;
  currentRow=0;
  c=HtmlObject_Tree_GetFirstChild(o);
  while(c) {
    int r;
    int ch;

    i=HtmlObject_GridEntry_GetColumn(c);
    r=HtmlObject_GridEntry_GetRow(c);
    if (r!=currentRow) {
      /* next row */
      y+=maxLineHeight+ROW_SPACING;
      x=COLUMN_SPACING/2;
      currentRow=r;
    }

    /* place object */
    HtmlObject_SetX(c, x);
    HtmlObject_SetY(c, y);

    /* calculate maximum height */
    ch=HtmlObject_GetHeight(c);
    if (ch>maxLineHeight)
      maxLineHeight=ch;

    /* advance */
    x+=cw[i]+COLUMN_SPACING;
    if (x>maxLineWidth)
      maxLineWidth=x;

    c=HtmlObject_Tree_GetNext(c);
  }
  y+=maxLineHeight+(ROW_SPACING/2);

  HtmlObject_SetWidth(o, maxLineWidth);
  HtmlObject_SetHeight(o, y);

  return 0;
}



HTML_OBJECT *HtmlObject_Grid_new(GWEN_XML_CONTEXT *ctx) {
  HTML_OBJECT *o;
  OBJECT_GRID *xo;

  o=HtmlObject_new(ctx, HtmlObjectType_Grid);
  GWEN_NEW_OBJECT(OBJECT_GRID, xo);
  GWEN_INHERIT_SETDATA(HTML_OBJECT, OBJECT_GRID, o, xo, HtmlObject_Grid_FreeData);

  HtmlObject_AddFlags(o,
		      HTML_OBJECT_FLAGS_START_ON_NEWLINE |
		      HTML_OBJECT_FLAGS_END_WITH_NEWLINE);
  HtmlObject_SetLayoutFn(o, HtmlObject_Grid_Layout);

  return o;
}



void GWENHYWFAR_CB HtmlObject_Grid_FreeData(void *bp, void *p) {
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






