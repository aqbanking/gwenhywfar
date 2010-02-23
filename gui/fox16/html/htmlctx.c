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

#include "htmlctx_p.h"
#include "o_box_l.h"
#include "g_box_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>



GWEN_INHERIT(GWEN_XML_CONTEXT, HTML_XMLCTX)




GWEN_XML_CONTEXT *HtmlCtx_new(uint32_t flags,
			      uint32_t guiid,
			      int timeout) {
  GWEN_XML_CONTEXT *ctx;
  HTML_XMLCTX *xctx;
  HTML_GROUP *g;
  HTML_OBJECT *o;

  /* create base object */
  ctx=GWEN_XmlCtx_new(flags, guiid, timeout);
  assert(ctx);

  /* create and assign extension */
  GWEN_NEW_OBJECT(HTML_XMLCTX, xctx);
  assert(xctx);
  GWEN_INHERIT_SETDATA(GWEN_XML_CONTEXT, HTML_XMLCTX, ctx, xctx,
		       HtmlCtx_FreeData);

  /* set virtual functions */
  GWEN_XmlCtx_SetStartTagFn(ctx, HtmlCtx_StartTag);
  GWEN_XmlCtx_SetEndTagFn(ctx, HtmlCtx_EndTag);
  GWEN_XmlCtx_SetAddDataFn(ctx, HtmlCtx_AddData);
  GWEN_XmlCtx_SetAddCommentFn(ctx, HtmlCtx_AddComment);
  GWEN_XmlCtx_SetAddAttrFn(ctx, HtmlCtx_AddAttr);

  xctx->objects=HtmlObject_Tree_new();
  xctx->fontList=HtmlFont_List_new();

  /* create initial group */
  g=HtmlGroup_Box_new("HTML_ROOT", NULL, ctx);
  assert(g);
  o=HtmlObject_Box_new(ctx);
  HtmlGroup_SetObject(g, o);
  HtmlObject_Tree_Add(xctx->objects, o);
  HtmlCtx_SetCurrentGroup(ctx, g);

  /* return base object */
  return ctx;
}



GWENHYWFAR_CB
void HtmlCtx_FreeData(void *bp, void *p) {
  HTML_XMLCTX *xctx;
  HTML_GROUP *g;

  xctx=(HTML_XMLCTX*)p;

  g=xctx->currentGroup;
  while (g) {
    HTML_GROUP *gParent;

    gParent=HtmlGroup_GetParent(g);
    HtmlGroup_free(g);
    g=gParent;
  }

  GWEN_DB_Group_free(xctx->dbCurrentAttribs);
  free(xctx->currentTagName);
  HtmlFont_List_free(xctx->fontList);
  HtmlObject_Tree_free(xctx->objects);

  GWEN_FREE_OBJECT(xctx);
}



HTML_GROUP *HtmlCtx_GetCurrentGroup(const GWEN_XML_CONTEXT *ctx) {
  HTML_XMLCTX *xctx;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, HTML_XMLCTX, ctx);
  assert(xctx);

  return xctx->currentGroup;
}



void HtmlCtx_SetCurrentGroup(GWEN_XML_CONTEXT *ctx, HTML_GROUP *g){
  HTML_XMLCTX *xctx;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, HTML_XMLCTX, ctx);
  assert(xctx);

  xctx->currentGroup=g;
}



const char *HtmlCtx_GetCurrentTagName(const GWEN_XML_CONTEXT *ctx) {
  HTML_XMLCTX *xctx;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, HTML_XMLCTX, ctx);
  assert(xctx);

  return xctx->currentTagName;
}



void HtmlCtx_SetCurrentTagName(GWEN_XML_CONTEXT *ctx, const char *s) {
  HTML_XMLCTX *xctx;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, HTML_XMLCTX, ctx);
  assert(xctx);

  free(xctx->currentTagName);
  if (s) xctx->currentTagName=strdup(s);
  else xctx->currentTagName=NULL;
}



GWEN_DB_NODE *HtmlCtx_GetCurrentAttributes(const GWEN_XML_CONTEXT *ctx) {
  HTML_XMLCTX *xctx;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, HTML_XMLCTX, ctx);
  assert(xctx);

  return xctx->dbCurrentAttribs;
}



HTML_OBJECT_TREE *HtmlCtx_GetObjects(const GWEN_XML_CONTEXT *ctx) {
  HTML_XMLCTX *xctx;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, HTML_XMLCTX, ctx);
  assert(xctx);

  return xctx->objects;
}



HTML_OBJECT_TREE *HtmlCtx_TakeObjects(const GWEN_XML_CONTEXT *ctx) {
  HTML_XMLCTX *xctx;
  HTML_OBJECT_TREE *t;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, HTML_XMLCTX, ctx);
  assert(xctx);

  t=xctx->objects;
  xctx->objects=NULL;

  return t;
}



HTML_OBJECT *HtmlCtx_GetRootObject(const GWEN_XML_CONTEXT *ctx) {
  HTML_XMLCTX *xctx;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, HTML_XMLCTX, ctx);
  assert(xctx);

  return HtmlObject_Tree_GetFirst(xctx->objects);
}



int HtmlCtx_SanitizeData(GWEN_XML_CONTEXT *ctx,
			       const char *data,
			       GWEN_BUFFER *buf) {
  const uint8_t *p;
  uint8_t *dst;
  uint8_t *src;
  unsigned int size;
  unsigned int i;
  int lastWasBlank;
  uint8_t *lastBlankPos;
  uint32_t bStart=0;

  if (GWEN_Text_UnescapeXmlToBuffer(data, buf)) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    return GWEN_ERROR_BAD_DATA;
  }

  dst=(uint8_t*)GWEN_Buffer_GetStart(buf);
  src=dst;

  /* skip leading blanks */
  while(*src && (*src<33 || *src>=127))
    src++;

  p=src;
  bStart=src-((uint8_t*)GWEN_Buffer_GetStart(buf));
  size=GWEN_Buffer_GetUsedBytes(buf)-bStart;
  lastWasBlank=0;
  lastBlankPos=0;

  for (i=0; i<size; i++) {
    uint8_t c;

    c=*p;
    if (c<32 || c>=127)
      c=32;

    /* remember next loop whether this char was a blank */
    if (c==32) {
      if (!lastWasBlank) {
	/* store only one blank */
	lastWasBlank=1;
	lastBlankPos=dst;
	*(dst++)=c;
      }
    }
    else {
      lastWasBlank=0;
      lastBlankPos=0;
      *(dst++)=c;
    }
    p++;
  }

  /* remove trailing blanks */
  if (lastBlankPos!=0)
    dst=lastBlankPos;

  size=dst-(uint8_t*)GWEN_Buffer_GetStart(buf);
  GWEN_Buffer_Crop(buf, 0, size);

  return 0;
}



HTML_PROPS *HtmlCtx_GetStandardProps(const GWEN_XML_CONTEXT *ctx) {
  HTML_XMLCTX *xctx;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, HTML_XMLCTX, ctx);
  assert(xctx);

  return xctx->standardProps;
}



void HtmlCtx_SetStandardProps(GWEN_XML_CONTEXT *ctx, HTML_PROPS *pr) {
  HTML_XMLCTX *xctx;
  HTML_OBJECT *o;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, HTML_XMLCTX, ctx);
  assert(xctx);

  HtmlProps_Attach(pr);
  HtmlProps_free(xctx->standardProps);
  xctx->standardProps=pr;

  if (xctx->currentGroup && HtmlGroup_GetParent(xctx->currentGroup)==NULL)
    HtmlGroup_SetProperties(xctx->currentGroup, pr);
  o=HtmlObject_Tree_GetFirst(xctx->objects);
  if (o && HtmlObject_GetProperties(o)==NULL)
    HtmlObject_SetProperties(o, pr);
}



HTML_FONT *HtmlCtx_FindFont(const GWEN_XML_CONTEXT *ctx,
			       const char *fontName,
			       int fontSize,
			       uint32_t fontFlags) {
  HTML_XMLCTX *xctx;
  HTML_FONT *fnt;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, HTML_XMLCTX, ctx);
  assert(xctx);

  assert(xctx->fontList);
  fnt=HtmlFont_List_First(xctx->fontList);
  while(fnt) {
    const char *s;

    s=HtmlFont_GetFontName(fnt);
    if (s && *s &&
	HtmlFont_GetFontSize(fnt)==fontSize &&
	HtmlFont_GetFontFlags(fnt)==fontFlags &&
	strcasecmp(s, fontName)==0)
      break;
    fnt=HtmlFont_List_Next(fnt);
  }

  return fnt;
}



HTML_FONT *HtmlCtx_GetFont(GWEN_XML_CONTEXT *ctx,
			      const char *fontName,
			      int fontSize,
			      uint32_t fontFlags) {
  HTML_XMLCTX *xctx;
  HTML_FONT *fnt;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, HTML_XMLCTX, ctx);
  assert(xctx);

  fnt=HtmlCtx_FindFont(ctx, fontName, fontSize, fontFlags);
  if (fnt)
    return fnt;
  fnt=HtmlFont_new();
  HtmlFont_SetFontName(fnt, fontName);
  HtmlFont_SetFontSize(fnt, fontSize);
  HtmlFont_SetFontFlags(fnt, fontFlags);
  HtmlFont_List_Add(fnt, xctx->fontList);

  return fnt;
}









int HtmlCtx_StartTag(GWEN_XML_CONTEXT *ctx, const char *tagName) {
  HTML_XMLCTX *xctx;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, HTML_XMLCTX, ctx);
  assert(xctx);

  DBG_INFO(GWEN_LOGDOMAIN, "Starting tag [%s]", tagName);

  /* store for later, do nothing more here */
  HtmlCtx_SetCurrentTagName(ctx, tagName);
  GWEN_DB_Group_free(xctx->dbCurrentAttribs);
  xctx->dbCurrentAttribs=GWEN_DB_Group_new("attribs");

  return 0;
}



int HtmlCtx_EndTag(GWEN_XML_CONTEXT *ctx, int closing) {
  HTML_XMLCTX *xctx;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, HTML_XMLCTX, ctx);
  assert(xctx);

  if (closing) {
    /* just ignore empty tags which are closed immediately */
    DBG_INFO(GWEN_LOGDOMAIN, "Closing empty tag [%s]",
	     (xctx->currentTagName)?xctx->currentTagName:"<noname>");
    return 0;
  }

  if (xctx->currentTagName==NULL) {
    DBG_INFO(GWEN_LOGDOMAIN, "No tag name, malformed HTML data");
    return GWEN_ERROR_BAD_DATA;
  }

  DBG_INFO(GWEN_LOGDOMAIN, "Completed tag [%s]", xctx->currentTagName);

  if (xctx->currentGroup) {
    if (*(xctx->currentTagName)=='/') {
      int rv;

      /* it is a closing tag, call EndTagFn */
      DBG_INFO(GWEN_LOGDOMAIN,
	       "Calling %s->EndTag(%s)",
	       HtmlGroup_GetGroupName(xctx->currentGroup),
	       xctx->currentTagName);
      rv=HtmlGroup_EndTag(xctx->currentGroup, xctx->currentTagName+1);
      if (rv<0) {
	if (rv!=GWEN_ERROR_NOT_IMPLEMENTED) {
	  DBG_INFO(GWEN_LOGDOMAIN,
		   "Error in EndTag(%s) for [%s]",
		   HtmlGroup_GetGroupName(xctx->currentGroup),
		   xctx->currentTagName);
	  return rv;
	}
      }
      else if (rv==1) {
        HTML_GROUP *g;
	HTML_GROUP *gParent;

	/* pop current group from stack */
	g=xctx->currentGroup;
	gParent=HtmlGroup_GetParent(g);
	xctx->currentGroup=gParent;
	if (gParent) {
	  DBG_INFO(GWEN_LOGDOMAIN,
		   "Calling %s->EndSubGroup(%s)",
		   HtmlGroup_GetGroupName(gParent),
                   HtmlGroup_GetGroupName(g));
	  HtmlGroup_EndSubGroup(gParent, g);
	}
	HtmlGroup_free(g);
	GWEN_XmlCtx_DecDepth(ctx);
      }
    }
    else {
      int rv;

      /* it is an opening tag, call StartTagFn */
      DBG_INFO(GWEN_LOGDOMAIN,
	       "Calling %s->StartTag(%s)",
	       HtmlGroup_GetGroupName(xctx->currentGroup),
	       xctx->currentTagName);
      rv=HtmlGroup_StartTag(xctx->currentGroup, xctx->currentTagName);
      if (rv<0) {
	if (rv!=GWEN_ERROR_NOT_IMPLEMENTED) {
	  DBG_INFO(GWEN_LOGDOMAIN,
		   "Error in StartTag(%s) for [%s]",
		   HtmlGroup_GetGroupName(xctx->currentGroup),
		   xctx->currentTagName);
	  return rv;
	}
      }
    }
  }

  return 0;
}






int HtmlCtx_AddData(GWEN_XML_CONTEXT *ctx, const char *data) {
  HTML_XMLCTX *xctx;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, HTML_XMLCTX, ctx);
  assert(xctx);

  if (xctx->currentGroup) {
    int rv;

    DBG_INFO(GWEN_LOGDOMAIN,
	     "Calling %s->AddData()",
	     HtmlGroup_GetGroupName(xctx->currentGroup));
    rv=HtmlGroup_AddData(xctx->currentGroup, data);
    if (rv<0) {
      if (rv!=GWEN_ERROR_NOT_IMPLEMENTED) {
	DBG_INFO(GWEN_LOGDOMAIN,
		 "Error in AddData(%s)",
		 HtmlGroup_GetGroupName(xctx->currentGroup));
	return rv;
      }
    }
  }

  return 0;
}



int HtmlCtx_AddComment(GWEN_XML_CONTEXT *ctx, const char *data) {
  HTML_XMLCTX *xctx;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, HTML_XMLCTX, ctx);
  assert(xctx);

  /* ignore comments */
  return 0;
}



int HtmlCtx_AddAttr(GWEN_XML_CONTEXT *ctx,
		    const char *attrName,
		    const char *attrData) {
  HTML_XMLCTX *xctx;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, HTML_XMLCTX, ctx);
  assert(xctx);

  if (xctx->currentGroup) {
    assert(xctx->dbCurrentAttribs);
    GWEN_DB_SetCharValue(xctx->dbCurrentAttribs,
			 GWEN_DB_FLAGS_DEFAULT,
			 attrName, attrData);
  }

  return 0;
}



int HtmlCtx_GetTextWidth(GWEN_XML_CONTEXT *ctx,
			 HTML_FONT *fnt,
			 const char *s) {
  HTML_XMLCTX *xctx;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, HTML_XMLCTX, ctx);
  assert(xctx);

  if (xctx->getTextWidthFn)
    return xctx->getTextWidthFn(ctx, fnt, s);
  else
    return -1;
}



int HtmlCtx_GetTextHeight(GWEN_XML_CONTEXT *ctx,
			  HTML_FONT *fnt,
			  const char *s) {
  HTML_XMLCTX *xctx;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, HTML_XMLCTX, ctx);
  assert(xctx);

  if (xctx->getTextHeightFn)
    return xctx->getTextHeightFn(ctx, fnt, s);
  else
    return -1;
}



uint32_t HtmlCtx_GetColorFromName(const GWEN_XML_CONTEXT *ctx,
				  const char *name) {
  HTML_XMLCTX *xctx;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, HTML_XMLCTX, ctx);
  assert(xctx);

  if (xctx->getColorFromNameFn)
    return xctx->getColorFromNameFn(ctx, name);
  else
    return 0xffffffff;
}



HTMLCTX_GET_TEXT_WIDTH_FN HtmlCtx_SetGetTextWidthFn(GWEN_XML_CONTEXT *ctx,
						    HTMLCTX_GET_TEXT_WIDTH_FN fn) {
  HTML_XMLCTX *xctx;
  HTMLCTX_GET_TEXT_WIDTH_FN of;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, HTML_XMLCTX, ctx);
  assert(xctx);

  of=xctx->getTextWidthFn;
  xctx->getTextWidthFn=fn;

  return of;
}



HTMLCTX_GET_TEXT_HEIGHT_FN HtmlCtx_SetGetTextHeightFn(GWEN_XML_CONTEXT *ctx,
						      HTMLCTX_GET_TEXT_HEIGHT_FN fn) {
  HTML_XMLCTX *xctx;
  HTMLCTX_GET_TEXT_HEIGHT_FN of;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, HTML_XMLCTX, ctx);
  assert(xctx);

  of=xctx->getTextHeightFn;
  xctx->getTextHeightFn=fn;

  return of;
}



HTMLCTX_GET_COLOR_FROM_NAME_FN HtmlCtx_SetGetColorFromNameFn(GWEN_XML_CONTEXT *ctx,
							     HTMLCTX_GET_COLOR_FROM_NAME_FN fn) {
  HTML_XMLCTX *xctx;
  HTMLCTX_GET_COLOR_FROM_NAME_FN of;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, HTML_XMLCTX, ctx);
  assert(xctx);

  of=xctx->getColorFromNameFn;
  xctx->getColorFromNameFn=fn;

  return of;
}



int HtmlCtx_Layout(GWEN_XML_CONTEXT *ctx, int width, int height) {
  HTML_XMLCTX *xctx;
  HTML_OBJECT *o;
  int rv;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, HTML_XMLCTX, ctx);
  assert(xctx);

  o=HtmlObject_Tree_GetFirst(xctx->objects);
  if (o==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No objects");
    return 0;
  }

  HtmlObject_SetWidth(o, width);
  HtmlObject_SetHeight(o, height);
  rv=HtmlObject_Layout(o);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}








