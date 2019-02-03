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


#include "htmlgroup_p.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>



GWEN_INHERIT_FUNCTIONS(HTML_GROUP)





HTML_GROUP *HtmlGroup_new(const char *groupName,
                          HTML_GROUP *parent,
                          GWEN_XML_CONTEXT *ctx)
{
  HTML_GROUP *g;

  GWEN_NEW_OBJECT(HTML_GROUP, g);
  GWEN_INHERIT_INIT(HTML_GROUP, g);
  g->parent=parent;
  g->xmlContext=ctx;
  if (groupName)
    g->groupName=strdup(groupName);
  if (g->xmlContext==NULL && g->parent)
    g->xmlContext=parent->xmlContext;

  return g;
}



void HtmlGroup_free(HTML_GROUP *g)
{
  if (g) {
    GWEN_INHERIT_FINI(HTML_GROUP, g);

    HtmlProps_free(g->properties);
    free(g->groupName);

    GWEN_FREE_OBJECT(g);
  }
}



HTML_GROUP *HtmlGroup_GetParent(const HTML_GROUP *g)
{
  assert(g);
  return g->parent;
}



GWEN_XML_CONTEXT *HtmlGroup_GetXmlContext(const HTML_GROUP *g)
{
  assert(g);
  return g->xmlContext;
}



const char *HtmlGroup_GetGroupName(const HTML_GROUP *g)
{
  assert(g);
  return g->groupName;
}



HTML_OBJECT *HtmlGroup_GetObject(const HTML_GROUP *g)
{
  assert(g);
  return g->object;
}



void HtmlGroup_SetObject(HTML_GROUP *g, HTML_OBJECT *o)
{
  assert(g);
  g->object=o;
}



HTML_PROPS *HtmlGroup_GetProperties(const HTML_GROUP *g)
{
  assert(g);
  return g->properties;
}



void HtmlGroup_SetProperties(HTML_GROUP *g, HTML_PROPS *pr)
{
  assert(g);

  HtmlProps_Attach(pr);
  HtmlProps_free(g->properties);
  g->properties=pr;
}








HTML_GROUP_STARTTAG_FN HtmlGroup_SetStartTagFn(HTML_GROUP *g,
                                               HTML_GROUP_STARTTAG_FN f)
{
  HTML_GROUP_STARTTAG_FN oldFn;

  assert(g);
  oldFn=g->startTagFn;
  g->startTagFn=f;
  return oldFn;
}



HTML_GROUP_ENDTAG_FN HtmlGroup_SetEndTagFn(HTML_GROUP *g,
                                           HTML_GROUP_ENDTAG_FN f)
{
  HTML_GROUP_ENDTAG_FN oldFn;

  assert(g);
  oldFn=g->endTagFn;
  g->endTagFn=f;
  return oldFn;
}



HTML_GROUP_ADDDATA_FN HtmlGroup_SetAddDataFn(HTML_GROUP *g,
                                             HTML_GROUP_ADDDATA_FN f)
{
  HTML_GROUP_ADDDATA_FN oldFn;

  assert(g);
  oldFn=g->addDataFn;
  g->addDataFn=f;
  return oldFn;
}



HTML_GROUP_ENDSUBGROUP_FN HtmlGroup_SetEndSubGroupFn(HTML_GROUP *g,
                                                     HTML_GROUP_ENDSUBGROUP_FN f)
{
  HTML_GROUP_ENDSUBGROUP_FN oldFn;

  assert(g);
  oldFn=g->endSubGroupFn;
  g->endSubGroupFn=f;
  return oldFn;
}





int HtmlGroup_StartTag(HTML_GROUP *g,
                       const char *tagName)
{
  assert(g);
  if (g->startTagFn)
    return g->startTagFn(g, tagName);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int HtmlGroup_EndTag(HTML_GROUP *g,
                     const char *tagName)
{
  assert(g);
  if (g->endTagFn)
    return g->endTagFn(g, tagName);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int HtmlGroup_AddData(HTML_GROUP *g,
                      const char *data)
{
  assert(g);
  if (g->addDataFn)
    return g->addDataFn(g, data);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int HtmlGroup_EndSubGroup(HTML_GROUP *g, HTML_GROUP *sg)
{
  assert(g);
  if (g->endSubGroupFn)
    return g->endSubGroupFn(g, sg);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}





