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

#include "g_ignore_p.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>



GWEN_INHERIT(HTML_GROUP, HTML_GROUP_IGNORE)




HTML_GROUP *HtmlGroup_Ignore_new(const char *groupName,
				       HTML_GROUP *parent,
				       GWEN_XML_CONTEXT *ctx) {
  HTML_GROUP *g;
  HTML_GROUP_IGNORE *xg;

  /* create base group */
  g=HtmlGroup_new(groupName, parent, ctx);
  assert(g);

  /* create extension, assign to base group */
  GWEN_NEW_OBJECT(HTML_GROUP_IGNORE, xg);
  assert(xg);
  GWEN_INHERIT_SETDATA(HTML_GROUP, HTML_GROUP_IGNORE, g, xg,
                       HtmlGroup_Ignore_FreeData);
  xg->openTags=GWEN_StringList_new();

  /* set virtual functions */
  HtmlGroup_SetStartTagFn(g, HtmlGroup_Ignore_StartTag);
  HtmlGroup_SetEndTagFn(g, HtmlGroup_Ignore_EndTag);
  HtmlGroup_SetAddDataFn(g, HtmlGroup_Ignore_AddData);

  return g;
}



GWENHYWFAR_CB
void HtmlGroup_Ignore_FreeData(void *bp, void *p) {
  HTML_GROUP_IGNORE *xg;

  xg=(HTML_GROUP_IGNORE*)p;
  GWEN_StringList_free(xg->openTags);
  GWEN_FREE_OBJECT(xg);
}



int HtmlGroup_Ignore_StartTag(HTML_GROUP *g, const char *tagName) {
  HTML_GROUP_IGNORE *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(HTML_GROUP, HTML_GROUP_IGNORE, g);
  assert(xg);

  /* just insert the new tag into list */
  GWEN_StringList_InsertString(xg->openTags, tagName, 0, 0);

  return 0;
}



int HtmlGroup_Ignore_EndTag(HTML_GROUP *g, const char *tagName) {
  HTML_GROUP_IGNORE *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(HTML_GROUP, HTML_GROUP_IGNORE, g);
  assert(xg);

  if (strcasecmp(HtmlGroup_GetGroupName(g), tagName)==0)
    /* ending this tag */
    return 1;

  /* should be a subtag */
  for (;;) {
    const char *s;

    s=GWEN_StringList_FirstString(xg->openTags);
    if (!s)
      /* empty and it is not our name, so let tha caller try parents */
      break;

    /* remove 1st open tag in any case */
    GWEN_StringList_RemoveString(xg->openTags, s);
    if (strcasecmp(s, tagName)==0)
      /* it was the one we wanted, stop here */
      break;
    /* otherwise this loop continues to remove all subtags until the
     * matching one is found */
  }

  return 0;
}



int HtmlGroup_Ignore_AddData(HTML_GROUP *g, const char *data) {
  /* just ignore the data */
  return 0;
}



