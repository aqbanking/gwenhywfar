/***************************************************************************
 begin       : Sat Feb 20 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef HTMLXMLCTX_P_H
#define HTMLXMLCTX_P_H


#include "htmlctx_l.h"



typedef struct HTML_XMLCTX HTML_XMLCTX;
struct HTML_XMLCTX {
  HTML_GROUP *currentGroup;
  HTML_OBJECT_TREE *objects;
  char *currentTagName;
  GWEN_DB_NODE *dbCurrentAttribs;

  HTML_PROPS *standardProps;
  HTML_FONT_LIST *fontList;

  HTMLCTX_GET_TEXT_WIDTH_FN getTextWidthFn;
  HTMLCTX_GET_TEXT_HEIGHT_FN getTextHeightFn;
  HTMLCTX_GET_COLOR_FROM_NAME_FN getColorFromNameFn;
};


static void GWENHYWFAR_CB HtmlCtx_FreeData(void *bp, void *p);


static int HtmlCtx_StartTag(GWEN_XML_CONTEXT *ctx, const char *tagName);
static int HtmlCtx_EndTag(GWEN_XML_CONTEXT *ctx, int closing);
static int HtmlCtx_AddData(GWEN_XML_CONTEXT *ctx, const char *data);
static int HtmlCtx_AddComment(GWEN_XML_CONTEXT *ctx, const char *data);
static int HtmlCtx_AddAttr(GWEN_XML_CONTEXT *ctx,
			   const char *attrName,
			   const char *attrData);


#endif

