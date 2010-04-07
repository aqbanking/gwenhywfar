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


#include "g_unorderedlist_p.h"
#include "g_box_l.h"
#include "g_generic_l.h"
#include "htmlctx_l.h"
#include "o_box_l.h"
#include "o_grid_l.h"
#include "o_gridentry_l.h"
#include "o_word_l.h"

#include <gwenhywfar/debug.h>


GWEN_INHERIT(HTML_GROUP, GROUP_UNORDEREDLIST)



HTML_GROUP *HtmlGroup_UnorderedList_new(const char *groupName,
					HTML_GROUP *parent,
					GWEN_XML_CONTEXT *ctx) {
  HTML_GROUP *g;
  GROUP_UNORDEREDLIST *xg;

  /* create base group */
  g=HtmlGroup_Generic_new(groupName, parent, ctx);
  assert(g);

  /* inherit */
  GWEN_NEW_OBJECT(GROUP_UNORDEREDLIST, xg);
  GWEN_INHERIT_SETDATA(HTML_GROUP, GROUP_UNORDEREDLIST, g, xg,
		       HtmlGroup_UnorderedList_FreeData);

  /* set virtual functions */
  HtmlGroup_SetStartTagFn(g, HtmlGroup_UnorderedList_StartTag);

  return g;
}



void HtmlGroup_UnorderedList_FreeData(void *bp, void *p) {
  GROUP_UNORDEREDLIST *xg;

  xg=(GROUP_UNORDEREDLIST*) p;
  GWEN_FREE_OBJECT(xg);
}



int HtmlGroup_UnorderedList_StartTag(HTML_GROUP *g, const char *tagName) {
  GROUP_UNORDEREDLIST *xg;
  HTML_GROUP *gNew=NULL;
  GWEN_XML_CONTEXT *ctx;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(HTML_GROUP, GROUP_UNORDEREDLIST, g);
  assert(xg);

  ctx=HtmlGroup_GetXmlContext(g);

  if (strcasecmp(tagName, "li")==0) {
    HTML_OBJECT *o;
    HTML_OBJECT *oGrid;

    /* first column in the grid is "-" */
    oGrid=HtmlObject_GridEntry_new(ctx);
    HtmlObject_GridEntry_SetColumn(oGrid, 0);
    HtmlObject_GridEntry_SetRow(oGrid, xg->row);
    HtmlObject_SetProperties(oGrid, HtmlGroup_GetProperties(g));
    HtmlObject_Tree_AddChild(HtmlGroup_GetObject(g), oGrid);

    o=HtmlObject_Word_new(ctx, "-");
    HtmlObject_SetProperties(o, HtmlGroup_GetProperties(g));
    HtmlObject_Tree_AddChild(oGrid, o);

    /* Create new parser group with new properties but use the same object */
    gNew=HtmlGroup_Box_new(tagName, g, ctx);
    HtmlGroup_SetProperties(gNew, HtmlGroup_GetProperties(g));
    o=HtmlGroup_GetObject(g);
    assert(o);
    if (HtmlObject_GetObjectType(o)==HtmlObjectType_Grid) {
      int i;

      i=HtmlObject_Grid_GetRows(o);
      HtmlObject_Grid_SetRows(o, ++i);
    }

    /* second column is the content of li */
    o=HtmlObject_GridEntry_new(ctx);
    HtmlObject_GridEntry_SetColumn(oGrid, 1);
    HtmlObject_GridEntry_SetRow(oGrid, xg->row);
    HtmlObject_SetProperties(o, HtmlGroup_GetProperties(g));
    HtmlObject_Tree_AddChild(oGrid, o);

    HtmlGroup_SetObject(gNew, o);
    xg->row++;
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



