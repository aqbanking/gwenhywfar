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

#define DISABLE_DEBUGLOG

#include "g_generic_p.h"
#include "htmlctx_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>




HTML_GROUP *HtmlGroup_Generic_new(const char *groupName,
                                  HTML_GROUP *parent,
                                  GWEN_XML_CONTEXT *ctx)
{
  HTML_GROUP *g;

  /* create base group */
  g=HtmlGroup_new(groupName, parent, ctx);
  assert(g);

  /* set virtual functions */
  HtmlGroup_SetEndTagFn(g, HtmlGroup_Generic_EndTag);
  HtmlGroup_SetAddDataFn(g, HtmlGroup_Generic_AddData);
  HtmlGroup_SetEndSubGroupFn(g, HtmlGroup_Generic_EndSubGroup);

  return g;
}



int HtmlGroup_Generic_EndTag(HTML_GROUP *g, const char *tagName)
{
  assert(g);

  if (strcasecmp(HtmlGroup_GetGroupName(g), tagName)!=0) {
    DBG_INFO(GWEN_LOGDOMAIN,
             "Tag [%s] does not close [%s], ignoring",
             tagName, HtmlGroup_GetGroupName(g));
    /*return GWEN_ERROR_BAD_DATA;*/
    return 0;
  }

  /* always end this tag */
  return 1;
}



int HtmlGroup_Generic_AddData(HTML_GROUP *g, const char *data)
{
  assert(g);

  /* just ignore the data */
  return 0;
}



int HtmlGroup_Generic_EndSubGroup(HTML_GROUP *g, HTML_GROUP *sg)
{
  assert(g);

  /* just ignore the end of sub group */
  return 0;
}











