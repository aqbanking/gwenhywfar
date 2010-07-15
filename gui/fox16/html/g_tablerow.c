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


#include "g_tablerow_p.h"
#include "g_box_l.h"
#include "g_generic_l.h"
#include "htmlctx_l.h"
#include "o_grid_l.h"
#include "o_gridentry_l.h"

#include <gwenhywfar/debug.h>


GWEN_INHERIT(HTML_GROUP, GROUP_TABLEROW)




HTML_GROUP *HtmlGroup_TableRow_new(const char *groupName,
				   HTML_GROUP *parent,
				   GWEN_XML_CONTEXT *ctx) {
  HTML_GROUP *g;
  GROUP_TABLEROW *xg;

  /* create base group */
  g=HtmlGroup_Generic_new(groupName, parent, ctx);
  assert(g);
  GWEN_NEW_OBJECT(GROUP_TABLEROW, xg);
  GWEN_INHERIT_SETDATA(HTML_GROUP, GROUP_TABLEROW, g, xg, HtmlGroup_TableRow_FreeData);

  /* set virtual functions */
  HtmlGroup_SetStartTagFn(g, HtmlGroup_TableRow_StartTag);

  return g;
}



void GWENHYWFAR_CB HtmlGroup_TableRow_FreeData(void *bp, void *p) {
  GROUP_TABLEROW *xg;

  xg=(GROUP_TABLEROW*) p;

  GWEN_FREE_OBJECT(xg);
}



int HtmlGroup_TableRow_GetColumns(const HTML_GROUP *g) {
  GROUP_TABLEROW *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(HTML_GROUP, GROUP_TABLEROW, g);
  assert(xg);

  return xg->columns;
}



int HtmlGroup_TableRow_StartTag(HTML_GROUP *g, const char *tagName) {
  GROUP_TABLEROW *xg;
  HTML_GROUP *gNew=NULL;
  GWEN_XML_CONTEXT *ctx;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(HTML_GROUP, GROUP_TABLEROW, g);
  assert(xg);

  ctx=HtmlGroup_GetXmlContext(g);

  if (strcasecmp(tagName, "th")==0) {
    HTML_OBJECT *o;
    HTML_PROPS *pr;
    HTML_FONT *fnt;

    /* create new parser group */
    gNew=HtmlGroup_Box_new(tagName, g, ctx);
    pr=HtmlProps_dup(HtmlGroup_GetProperties(g));
    fnt=HtmlProps_GetFont(pr);
    fnt=HtmlCtx_GetFont(ctx,
			HtmlFont_GetFontName(fnt),
			HtmlFont_GetFontSize(fnt),
			HtmlFont_GetFontFlags(fnt) | HTML_FONT_FLAGS_STRONG);
    HtmlProps_SetFont(pr, fnt);
    HtmlGroup_SetProperties(gNew, pr);
    HtmlProps_free(pr);

    o=HtmlObject_GridEntry_new(ctx);
    HtmlObject_SetProperties(o, HtmlGroup_GetProperties(g));
    HtmlObject_GridEntry_SetColumn(o, xg->columns++);
    HtmlObject_GridEntry_SetRow(o, xg->row);
    HtmlObject_GridEntry_SetIsHeader(o, 1);
    HtmlObject_Tree_AddChild(HtmlGroup_GetObject(g), o);
    HtmlGroup_SetObject(gNew, o);
  }
  else if (strcasecmp(tagName, "td")==0) {
    HTML_OBJECT *o;

    /* create new parser group */
    gNew=HtmlGroup_Box_new(tagName, g, ctx);
    HtmlGroup_SetProperties(gNew, HtmlGroup_GetProperties(g));
    o=HtmlObject_GridEntry_new(ctx);
    HtmlObject_SetProperties(o, HtmlGroup_GetProperties(g));
    HtmlObject_GridEntry_SetColumn(o, xg->columns++);
    HtmlObject_GridEntry_SetRow(o, xg->row);
    HtmlObject_GridEntry_SetIsHeader(o, 0);
    HtmlObject_Tree_AddChild(HtmlGroup_GetObject(g), o);
    HtmlGroup_SetObject(gNew, o);
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN,
	      "Unexpected group [%s]", tagName);
    return GWEN_ERROR_BAD_DATA;
  }

  if (gNew) {
    HtmlCtx_SetCurrentGroup(ctx, gNew);
    GWEN_XmlCtx_IncDepth(ctx);
  }

  return 0;
}



void HtmlGroup_TableRow_SetRow(HTML_GROUP *g, int row) {
  GROUP_TABLEROW *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(HTML_GROUP, GROUP_TABLEROW, g);
  assert(xg);

  xg->row=row;
}






