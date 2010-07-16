/***************************************************************************
 begin       : Mon Feb 22 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef HTMLXMLCTX_BE_H
#define HTMLXMLCTX_BE_H

#include "htmlfont_be.h"
#include "htmlobject_be.h"
#include "htmlimage_be.h"

#include <gwen-gui-fox16/api.h>

#include <gwenhywfar/xmlctx.h>
#include <gwenhywfar/stringlist.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef int (*HTMLCTX_GET_TEXT_WIDTH_FN)(GWEN_XML_CONTEXT *ctx,
					 HTML_FONT *fnt,
					 const char *s);

typedef int (*HTMLCTX_GET_TEXT_HEIGHT_FN)(GWEN_XML_CONTEXT *ctx,
					  HTML_FONT *fnt,
					  const char *s);


typedef uint32_t (*HTMLCTX_GET_COLOR_FROM_NAME_FN)(const GWEN_XML_CONTEXT *ctx, const char *s);


typedef HTML_FONT* (*HTMLCTX_GET_FONT_FN)(GWEN_XML_CONTEXT *ctx,
					  const char *fontName,
					  int fontSize,
					  uint32_t fontFlags);

typedef HTML_IMAGE* (*HTMLCTX_GET_IMAGE_FN)(GWEN_XML_CONTEXT *ctx, const char *imageName);



FOX16GUI_API 
GWEN_XML_CONTEXT *HtmlCtx_new(uint32_t flags);

FOX16GUI_API 
void HtmlCtx_SetText(GWEN_XML_CONTEXT *ctx, const char *s);

FOX16GUI_API 
int HtmlCtx_Layout(GWEN_XML_CONTEXT *ctx, int width, int height);

FOX16GUI_API 
HTML_OBJECT *HtmlCtx_GetRootObject(const GWEN_XML_CONTEXT *ctx);


FOX16GUI_API 
HTML_FONT *HtmlCtx_GetFont(GWEN_XML_CONTEXT *ctx,
			   const char *fontName,
			   int fontSize,
			   uint32_t fontFlags);

/**
 * The implementation must set width and height in the image returned.
 */
FOX16GUI_API 
HTML_IMAGE *HtmlCtx_GetImage(GWEN_XML_CONTEXT *ctx, const char *imageName);

FOX16GUI_API 
HTML_PROPS *HtmlCtx_GetStandardProps(const GWEN_XML_CONTEXT *ctx);

FOX16GUI_API 
void HtmlCtx_SetStandardProps(GWEN_XML_CONTEXT *ctx, HTML_PROPS *pr);


FOX16GUI_API 
int HtmlCtx_GetWidth(const GWEN_XML_CONTEXT *ctx);

FOX16GUI_API 
int HtmlCtx_GetHeight(const GWEN_XML_CONTEXT *ctx);


FOX16GUI_API 
GWEN_STRINGLIST *HtmlCtx_GetMediaPaths(const GWEN_XML_CONTEXT *ctx);

FOX16GUI_API 
void HtmlCtx_AddMediaPath(GWEN_XML_CONTEXT *ctx, const char *s);


FOX16GUI_API 
HTMLCTX_GET_TEXT_WIDTH_FN HtmlCtx_SetGetTextWidthFn(GWEN_XML_CONTEXT *ctx,
						    HTMLCTX_GET_TEXT_WIDTH_FN fn);

FOX16GUI_API 
HTMLCTX_GET_TEXT_HEIGHT_FN HtmlCtx_SetGetTextHeightFn(GWEN_XML_CONTEXT *ctx,
						      HTMLCTX_GET_TEXT_HEIGHT_FN fn);


FOX16GUI_API 
HTMLCTX_GET_COLOR_FROM_NAME_FN HtmlCtx_SetGetColorFromNameFn(GWEN_XML_CONTEXT *ctx,
							     HTMLCTX_GET_COLOR_FROM_NAME_FN fn);

FOX16GUI_API 
HTMLCTX_GET_FONT_FN HtmlCtx_SetGetFontFn(GWEN_XML_CONTEXT *ctx,
					 HTMLCTX_GET_FONT_FN fn);

FOX16GUI_API 
HTMLCTX_GET_IMAGE_FN HtmlCtx_SetGetImageFn(GWEN_XML_CONTEXT *ctx, HTMLCTX_GET_IMAGE_FN fn);

#ifdef __cplusplus
}
#endif

#endif

