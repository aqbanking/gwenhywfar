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

#include "g_box_p.h"
#include "g_generic_l.h"
#include "g_table_l.h"
#include "g_unorderedlist_l.h"
#include "htmlctx_l.h"
#include "o_box_l.h"
#include "o_word_l.h"
#include "o_grid_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>

#include <ctype.h>




HTML_GROUP *HtmlGroup_Box_new(const char *groupName,
			      HTML_GROUP *parent,
			      GWEN_XML_CONTEXT *ctx) {
  HTML_GROUP *g;

  /* create base group */
  g=HtmlGroup_Generic_new(groupName, parent, ctx);
  assert(g);

  /* set virtual functions */
  HtmlGroup_SetStartTagFn(g, HtmlGroup_Box_StartTag);
  HtmlGroup_SetAddDataFn(g, HtmlGroup_Box_AddData);

  return g;
}



int HtmlGroup_Box_StartTag(HTML_GROUP *g, const char *tagName) {
  HTML_GROUP *gNew=NULL;
  GWEN_XML_CONTEXT *ctx;

  assert(g);

  ctx=HtmlGroup_GetXmlContext(g);

  if (strcasecmp(tagName, "b")==0) {
    /* Create new parser group with new properties but use the same object */
    HTML_PROPS *pr;
    HTML_FONT *fnt;

    gNew=HtmlGroup_Box_new(tagName, g, ctx);
    pr=HtmlProps_dup(HtmlGroup_GetProperties(g));
    fnt=HtmlProps_GetFont(pr);
    fnt=HtmlCtx_GetFont(ctx,
			HtmlFont_GetFontName(fnt),
			HtmlFont_GetFontSize(fnt),
			HtmlFont_GetFontFlags(fnt) | HTML_FONT_FLAGS_STRONG);
    HtmlProps_SetFont(pr, fnt);
    HtmlGroup_SetProperties(gNew, pr);
    HtmlProps_free(pr);
    HtmlGroup_SetObject(gNew, HtmlGroup_GetObject(g));
  }
  else if (strcasecmp(tagName, "i")==0) {
    /* Create new parser group with new properties but use the same object */
    HTML_PROPS *pr;
    HTML_FONT *fnt;

    gNew=HtmlGroup_Box_new(tagName, g, ctx);
    pr=HtmlProps_dup(HtmlGroup_GetProperties(g));
    fnt=HtmlProps_GetFont(pr);
    fnt=HtmlCtx_GetFont(ctx,
			HtmlFont_GetFontName(fnt),
			HtmlFont_GetFontSize(fnt),
			HtmlFont_GetFontFlags(fnt) | HTML_FONT_FLAGS_ITALIC);
    HtmlProps_SetFont(pr, fnt);
    HtmlGroup_SetProperties(gNew, pr);
    HtmlProps_free(pr);
    HtmlGroup_SetObject(gNew, HtmlGroup_GetObject(g));
  }
  else if (strcasecmp(tagName, "u")==0) {
    /* Create new parser group with new properties but use the same object */
    HTML_PROPS *pr;
    HTML_FONT *fnt;

    gNew=HtmlGroup_Box_new(tagName, g, ctx);
    pr=HtmlProps_dup(HtmlGroup_GetProperties(g));
    fnt=HtmlProps_GetFont(pr);
    fnt=HtmlCtx_GetFont(ctx,
			HtmlFont_GetFontName(fnt),
			HtmlFont_GetFontSize(fnt),
			HtmlFont_GetFontFlags(fnt) | HTML_FONT_FLAGS_UNDERLINE);
    HtmlProps_SetFont(pr, fnt);
    HtmlGroup_SetProperties(gNew, pr);
    HtmlProps_free(pr);
    HtmlGroup_SetObject(gNew, HtmlGroup_GetObject(g));
  }
  else if (strcasecmp(tagName, "p")==0) {
    HTML_OBJECT *o;

    gNew=HtmlGroup_Box_new(tagName, g, ctx);
    o=HtmlObject_Box_new(ctx);
    HtmlObject_AddFlags(o,
			HTML_OBJECT_FLAGS_START_ON_NEWLINE |
			HTML_OBJECT_FLAGS_END_WITH_NEWLINE);
    HtmlObject_Tree_AddChild(HtmlGroup_GetObject(g), o);
    HtmlObject_SetProperties(o, HtmlGroup_GetProperties(g));
    HtmlGroup_SetObject(gNew, o);
  }
  else if (strcasecmp(tagName, "br")==0) {
    HTML_OBJECT *o;

    /* just create and add a control object */
    o=HtmlObject_new(ctx, HtmlObjectType_Control);
    HtmlObject_AddFlags(o, HTML_OBJECT_FLAGS_END_WITH_NEWLINE);
    HtmlObject_Tree_AddChild(HtmlGroup_GetObject(g), o);
    HtmlObject_SetProperties(o, HtmlGroup_GetProperties(g));
  }
  else if (strcasecmp(tagName, "table")==0) {
    HTML_OBJECT *o;

    gNew=HtmlGroup_Table_new(tagName, g, ctx);
    HtmlGroup_SetProperties(gNew, HtmlGroup_GetProperties(g));
    o=HtmlObject_Grid_new(ctx);
    HtmlObject_SetProperties(o, HtmlGroup_GetProperties(g));
    HtmlGroup_SetObject(gNew, o);
    HtmlObject_Tree_AddChild(HtmlGroup_GetObject(g), o);
  }
  else if (strcasecmp(tagName, "ul")==0) {
    HTML_OBJECT *o;

    gNew=HtmlGroup_UnorderedList_new(tagName, g, ctx);
    HtmlGroup_SetProperties(gNew, HtmlGroup_GetProperties(g));
    o=HtmlObject_Grid_new(ctx);
    HtmlObject_Grid_SetColumns(o, 2);
    HtmlObject_SetProperties(o, HtmlGroup_GetProperties(g));
    HtmlGroup_SetObject(gNew, o);
    HtmlObject_Tree_AddChild(HtmlGroup_GetObject(g), o);
  }
  else if (strcasecmp(tagName, "font")==0) {
    /* Create new parser group with new properties but use the same object */
    HTML_PROPS *pr;
    GWEN_DB_NODE *dbAttribs;

    gNew=HtmlGroup_Box_new(tagName, g, ctx);
    pr=HtmlProps_dup(HtmlGroup_GetProperties(g));

    dbAttribs=HtmlCtx_GetCurrentAttributes(ctx);
    if (dbAttribs) {
      HTML_FONT *fnt;
      const char *s;
      const char *fontName;
      int fontSize;
      uint32_t fontFlags;

      fnt=HtmlProps_GetFont(pr);
      fontName=GWEN_DB_GetCharValue(dbAttribs, "face", 0, NULL);
      if (fontName==NULL)
	fontName=HtmlFont_GetFontName(fnt);
      fontFlags=HtmlFont_GetFontFlags(fnt);
      fontSize=HtmlFont_GetFontSize(fnt);
      s=GWEN_DB_GetCharValue(dbAttribs, "size", 0, NULL);
      if (s && *s) {
	if (*s=='+') {
	  int i;

	  sscanf(s, "%d", &i);
	  fontSize+=i*4;
	}
	else if (*s=='-') {
	  int i;

	  sscanf(s, "%d", &i);
	  fontSize+=i*4;
	}
	else
	  sscanf(s, "%d", &fontSize);
      }

      s=GWEN_DB_GetCharValue(dbAttribs, "color", 0, NULL);
      if (s && *s) {
	uint32_t color;

	color=HtmlCtx_GetColorFromName(ctx, s);
	HtmlProps_SetForegroundColor(pr, color);
      }

      fnt=HtmlCtx_GetFont(ctx, fontName, fontSize, fontFlags);
      HtmlProps_SetFont(pr, fnt);
    }

    HtmlGroup_SetProperties(gNew, pr);
    HtmlProps_free(pr);
    HtmlGroup_SetObject(gNew, HtmlGroup_GetObject(g));
  }
  else if (strcasecmp(tagName, "h1")==0) {
    /* Create new parser group with new properties but use the same object */
    HTML_PROPS *pr;
    HTML_FONT *fnt;
    HTML_OBJECT *o;

    gNew=HtmlGroup_Box_new(tagName, g, ctx);
    pr=HtmlProps_dup(HtmlGroup_GetProperties(g));
    fnt=HtmlProps_GetFont(pr);
    fnt=HtmlCtx_GetFont(ctx,
			HtmlFont_GetFontName(fnt),
			HtmlFont_GetFontSize(fnt)+8,
			HtmlFont_GetFontFlags(fnt) | HTML_FONT_FLAGS_STRONG);
    HtmlProps_SetFont(pr, fnt);
    HtmlGroup_SetProperties(gNew, pr);

    o=HtmlObject_Box_new(ctx);
    HtmlObject_Tree_AddChild(HtmlGroup_GetObject(g), o);
    HtmlObject_AddFlags(o,
			HTML_OBJECT_FLAGS_START_ON_NEWLINE |
			HTML_OBJECT_FLAGS_END_WITH_NEWLINE);
    HtmlObject_SetProperties(o, pr);
    HtmlGroup_SetObject(gNew, o);
    HtmlProps_free(pr);
  }
  else if (strcasecmp(tagName, "h2")==0) {
    /* Create new parser group with new properties but use the same object */
    HTML_PROPS *pr;
    HTML_FONT *fnt;
    HTML_OBJECT *o;

    gNew=HtmlGroup_Box_new(tagName, g, ctx);
    pr=HtmlProps_dup(HtmlGroup_GetProperties(g));
    fnt=HtmlProps_GetFont(pr);
    fnt=HtmlCtx_GetFont(ctx,
			HtmlFont_GetFontName(fnt),
			HtmlFont_GetFontSize(fnt)+4,
			HtmlFont_GetFontFlags(fnt) | HTML_FONT_FLAGS_STRONG);
    HtmlProps_SetFont(pr, fnt);
    HtmlGroup_SetProperties(gNew, pr);

    o=HtmlObject_Box_new(ctx);
    HtmlObject_Tree_AddChild(HtmlGroup_GetObject(g), o);
    HtmlObject_AddFlags(o,
			HTML_OBJECT_FLAGS_START_ON_NEWLINE |
			HTML_OBJECT_FLAGS_END_WITH_NEWLINE);
    HtmlObject_SetProperties(o, pr);
    HtmlGroup_SetObject(gNew, o);
    HtmlProps_free(pr);
  }
  else if (strcasecmp(tagName, "h3")==0) {
    /* Create new parser group with new properties but use the same object */
    HTML_PROPS *pr;
    HTML_FONT *fnt;
    HTML_OBJECT *o;

    gNew=HtmlGroup_Box_new(tagName, g, ctx);
    pr=HtmlProps_dup(HtmlGroup_GetProperties(g));
    fnt=HtmlProps_GetFont(pr);
    fnt=HtmlCtx_GetFont(ctx,
			HtmlFont_GetFontName(fnt),
			HtmlFont_GetFontSize(fnt),
			HtmlFont_GetFontFlags(fnt) | HTML_FONT_FLAGS_STRONG);
    HtmlProps_SetFont(pr, fnt);
    HtmlGroup_SetProperties(gNew, pr);

    o=HtmlObject_Box_new(ctx);
    HtmlObject_Tree_AddChild(HtmlGroup_GetObject(g), o);
    HtmlObject_AddFlags(o,
			HTML_OBJECT_FLAGS_START_ON_NEWLINE |
			HTML_OBJECT_FLAGS_END_WITH_NEWLINE);
    HtmlObject_SetProperties(o, pr);
    HtmlGroup_SetObject(gNew, o);
    HtmlProps_free(pr);
  }
  else {
    DBG_WARN(GWEN_LOGDOMAIN,
	     "Unknown group [%s], handling as normal box", tagName);
    gNew=HtmlGroup_Box_new(tagName, g, ctx);
    HtmlGroup_SetProperties(gNew, HtmlGroup_GetProperties(g));
    HtmlGroup_SetObject(gNew, HtmlGroup_GetObject(g));
  }

  if (gNew) {
    HtmlCtx_SetCurrentGroup(ctx, gNew);
    GWEN_XmlCtx_IncDepth(ctx);
  }

  return 0;
}



int HtmlGroup_Box_AddData(HTML_GROUP *g, const char *data) {
  GWEN_XML_CONTEXT *ctx;
  GWEN_BUFFER *buf;
  int rv;
  uint8_t *s;
  HTML_OBJECT *o;

  assert(g);

  ctx=HtmlGroup_GetXmlContext(g);

  buf=GWEN_Buffer_new(0, strlen(data), 0, 1);
  rv=HtmlCtx_SanitizeData(ctx, data, buf);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(buf);
    return rv;
  }
  s=(uint8_t*)GWEN_Buffer_GetStart(buf);

  while(*s) {
    uint8_t *t;
    uint8_t c;

    /* find begin of word */
    while(*s && isblank(*s))
      s++;

    /* find end of word */
    t=s;
    while(*t && !isblank(*t))
      t++;
    c=*t;
    *t=0;
    o=HtmlObject_Word_new(ctx, (const char*) s);
    HtmlObject_SetProperties(o, HtmlGroup_GetProperties(g));
    HtmlObject_Tree_AddChild(HtmlGroup_GetObject(g), o);
    *t=c;
    s=t;
  }

  return 0;
}



