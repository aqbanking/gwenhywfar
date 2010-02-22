/***************************************************************************
 begin       : Sat Feb 20 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef HTMLXMLCTX_L_H
#define HTMLXMLCTX_L_H


#include "htmlctx_be.h"
#include "htmlgroup_l.h"


HTML_OBJECT_TREE *HtmlCtx_GetObjects(const GWEN_XML_CONTEXT *ctx);
HTML_OBJECT_TREE *HtmlCtx_TakeObjects(const GWEN_XML_CONTEXT *ctx);

HTML_GROUP *HtmlCtx_GetCurrentGroup(const GWEN_XML_CONTEXT *ctx);
void HtmlCtx_SetCurrentGroup(GWEN_XML_CONTEXT *ctx, HTML_GROUP *g);

const char *HtmlCtx_GetCurrentTagName(const GWEN_XML_CONTEXT *ctx);
void HtmlCtx_SetCurrentTagName(GWEN_XML_CONTEXT *ctx, const char *s);

GWEN_DB_NODE *HtmlCtx_GetCurrentAttributes(const GWEN_XML_CONTEXT *ctx);

HTML_FONT *HtmlCtx_FindFont(const GWEN_XML_CONTEXT *ctx,
			    const char *fontName,
			    int fontSize,
			    uint32_t fontFlags);


int HtmlCtx_SanitizeData(GWEN_XML_CONTEXT *ctx,
			 const char *data,
			 GWEN_BUFFER *buf);


uint32_t HtmlCtx_GetColorFromName(const GWEN_XML_CONTEXT *ctx,
				  const char *name);


int HtmlCtx_GetTextWidth(GWEN_XML_CONTEXT *ctx,
			 HTML_FONT *fnt,
			 const char *s);

int HtmlCtx_GetTextHeight(GWEN_XML_CONTEXT *ctx,
			  HTML_FONT *fnt,
			  const char *s);


#endif

