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



HTML_GROUP *HtmlGroup_UnorderedList_new(const char *groupName,
					HTML_GROUP *parent,
					GWEN_XML_CONTEXT *ctx) {
  HTML_GROUP *g;

  /* create base group */
  g=HtmlGroup_Generic_new(groupName, parent, ctx);
  assert(g);

  /* set virtual functions */
  HtmlGroup_SetStartTagFn(g, HtmlGroup_UnorderedList_StartTag);

  return g;
}



int HtmlGroup_UnorderedList_StartTag(HTML_GROUP *g, const char *tagName) {
  HTML_GROUP *gNew=NULL;
  GWEN_XML_CONTEXT *ctx;

  assert(g);

  ctx=HtmlGroup_GetXmlContext(g);

  if (strcasecmp(tagName, "li")==0) {
    HTML_OBJECT *o;
    HTML_OBJECT *oGrid;

    /* first column in the grid is "-" */
    oGrid=HtmlObject_GridEntry_new(ctx);
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
    HtmlObject_SetProperties(o, HtmlGroup_GetProperties(g));
    HtmlObject_Tree_AddChild(oGrid, o);

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



