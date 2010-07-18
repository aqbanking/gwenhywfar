/***************************************************************************
 begin       : Sat Feb 20 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef HTMLGROUP_L_H
#define HTMLGROUP_L_H

#include <gwenhywfar/inherit.h>
#include <gwenhywfar/xmlctx.h>



typedef struct HTML_GROUP HTML_GROUP;
GWEN_INHERIT_FUNCTION_DEFS(HTML_GROUP)


typedef int (*HTML_GROUP_STARTTAG_FN)(HTML_GROUP *g,
				      const char *tagName);
typedef int (*HTML_GROUP_ENDTAG_FN)(HTML_GROUP *g,
				    const char *tagName);
typedef int (*HTML_GROUP_ADDDATA_FN)(HTML_GROUP *g,
				     const char *data);

typedef int (*HTML_GROUP_ENDSUBGROUP_FN)(HTML_GROUP *g,
					 HTML_GROUP *sg);


#include "htmlobject_be.h"
#include "htmlprops_be.h"



HTML_GROUP *HtmlGroup_new(const char *groupName,
			  HTML_GROUP *parent,
			  GWEN_XML_CONTEXT *ctx);
void HtmlGroup_free(HTML_GROUP *g);


HTML_GROUP *HtmlGroup_GetParent(const HTML_GROUP *g);
GWEN_XML_CONTEXT *HtmlGroup_GetXmlContext(const HTML_GROUP *g);
const char *HtmlGroup_GetGroupName(const HTML_GROUP *g);

HTML_OBJECT *HtmlGroup_GetObject(const HTML_GROUP *g);
void HtmlGroup_SetObject(HTML_GROUP *g, HTML_OBJECT *o);


HTML_PROPS *HtmlGroup_GetProperties(const HTML_GROUP *g);
void HtmlGroup_SetProperties(HTML_GROUP *g, HTML_PROPS *pr);


HTML_GROUP_STARTTAG_FN
  HtmlGroup_SetStartTagFn(HTML_GROUP *g,
			  HTML_GROUP_STARTTAG_FN f);

HTML_GROUP_ENDTAG_FN
  HtmlGroup_SetEndTagFn(HTML_GROUP *g,
			HTML_GROUP_ENDTAG_FN f);

HTML_GROUP_ADDDATA_FN
  HtmlGroup_SetAddDataFn(HTML_GROUP *g,
			 HTML_GROUP_ADDDATA_FN f);

HTML_GROUP_ENDSUBGROUP_FN
  HtmlGroup_SetEndSubGroupFn(HTML_GROUP *g,
			     HTML_GROUP_ENDSUBGROUP_FN f);

int HtmlGroup_StartTag(HTML_GROUP *g, const char *tagName);

/**
 * @return 1 if this tag ends the current group, 0 otherwise (<0 on error)
 */
int HtmlGroup_EndTag(HTML_GROUP *g, const char *tagName);
int HtmlGroup_AddData(HTML_GROUP *g,
			 const char *data);

int HtmlGroup_EndSubGroup(HTML_GROUP *g, HTML_GROUP *sg);

#endif
