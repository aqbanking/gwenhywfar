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


#include "o_word_l.h"
#include "htmlctx_l.h"

#include <gwenhywfar/debug.h>



static int HtmlObject_Word_Layout(HTML_OBJECT *o) {
  const char *s;
  int w=0;
  int h=0;

  s=HtmlObject_GetText(o);
  if (s) {
    HTML_PROPS *pr;
    HTML_FONT *fnt;

    pr=HtmlObject_GetProperties(o);
    assert(pr);
    fnt=HtmlProps_GetFont(pr);
    w=HtmlCtx_GetTextWidth(HtmlObject_GetXmlCtx(o), fnt, s)+(HtmlFont_GetFontSize(fnt)/2);
    h=HtmlCtx_GetTextHeight(HtmlObject_GetXmlCtx(o), fnt, s);
  }

  HtmlObject_SetWidth(o, w);
  HtmlObject_SetHeight(o, h);
  return 0;
}



HTML_OBJECT *HtmlObject_Word_new(GWEN_XML_CONTEXT *ctx, const char *s) {
  HTML_OBJECT *o;

  o=HtmlObject_new(ctx, HtmlObjectType_Word);
  HtmlObject_SetText(o, s);
  HtmlObject_SetLayoutFn(o, HtmlObject_Word_Layout);

  return o;
}


