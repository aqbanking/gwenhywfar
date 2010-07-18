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


#include "g_table_p.h"
#include "g_generic_l.h"
#include "htmlctx_l.h"
#include "o_grid_l.h"
#include "g_tablerow_l.h"

#include <gwenhywfar/debug.h>



HTML_GROUP *HtmlGroup_Table_new(const char *groupName,
			       HTML_GROUP *parent,
			       GWEN_XML_CONTEXT *ctx) {
  HTML_GROUP *g;

  /* create base group */
  g=HtmlGroup_Generic_new(groupName, parent, ctx);
  assert(g);

  /* set virtual functions */
  HtmlGroup_SetStartTagFn(g, HtmlGroup_Table_StartTag);
  HtmlGroup_SetEndSubGroupFn(g, HtmlGroup_Table_EndSubGroup);

  return g;
}



int HtmlGroup_Table_StartTag(HTML_GROUP *g, const char *tagName) {
  HTML_GROUP *gNew=NULL;
  GWEN_XML_CONTEXT *ctx;

  assert(g);

  ctx=HtmlGroup_GetXmlContext(g);

  if (strcasecmp(tagName, "tr")==0) {
    HTML_OBJECT *o;
    int rows;

    o=HtmlGroup_GetObject(g);
    assert(o);
    rows=HtmlObject_Grid_GetRows(o);

    /* Create new parser group with new properties but use the same object */
    gNew=HtmlGroup_TableRow_new(tagName, g, ctx);
    HtmlGroup_TableRow_SetRow(gNew, rows);
    HtmlGroup_SetProperties(gNew, HtmlGroup_GetProperties(g));
    HtmlObject_Grid_SetRows(o, ++rows);
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



int HtmlGroup_Table_EndSubGroup(HTML_GROUP *g, HTML_GROUP *sg) {
  GWEN_XML_CONTEXT *ctx;
  const char *s;

  assert(g);

  ctx=HtmlGroup_GetXmlContext(g);

  s=HtmlGroup_GetGroupName(sg);
  if (strcasecmp(s, "tr")==0) {
    HTML_OBJECT *o;
    int i;
    int j;

    o=HtmlGroup_GetObject(g);
    i=HtmlObject_Grid_GetColumns(o);
    j=HtmlGroup_TableRow_GetColumns(sg);
    if (j>i)
      HtmlObject_Grid_SetColumns(o, j);
  }

  return 0;
}









