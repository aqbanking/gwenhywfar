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


#include "htmlprops_p.h"

#include <gwenhywfar/misc.h>

#include <assert.h>




HTML_PROPS *HtmlProps_new() {
  HTML_PROPS *pr;

  GWEN_NEW_OBJECT(HTML_PROPS, pr);
  pr->refCount=1;
  pr->foregroundColor=HTML_PROPS_NOCOLOR;
  pr->backgroundColor=HTML_PROPS_NOCOLOR;

  return pr;
}



void HtmlProps_free(HTML_PROPS *pr) {
  if (pr) {
    assert(pr->refCount);
    if (pr->refCount>1) {
      pr->refCount--;
    }
    else {
      HtmlFont_free(pr->font);
      pr->refCount=0;
      GWEN_FREE_OBJECT(pr);
    }
  }
}



HTML_PROPS *HtmlProps_dup(const HTML_PROPS *pro) {
  HTML_PROPS *pr;

  pr=HtmlProps_new();
  pr->font=pro->font;
  if (pr->font)
    HtmlFont_Attach(pr->font);
  pr->foregroundColor=pro->foregroundColor;
  pr->backgroundColor=pro->backgroundColor;

  return pr;
}



void HtmlProps_Attach(HTML_PROPS *pr) {
  assert(pr);
  assert(pr->refCount);
  pr->refCount++;
}



HTML_FONT *HtmlProps_GetFont(const HTML_PROPS *pr) {
  assert(pr);
  assert(pr->refCount);
  return pr->font;
}



void HtmlProps_SetFont(HTML_PROPS *pr, HTML_FONT *fnt) {
  assert(pr);
  assert(pr->refCount);
  HtmlFont_Attach(fnt);
  HtmlFont_free(pr->font);
  pr->font=fnt;
}



uint32_t HtmlProps_GetForegroundColor(const HTML_PROPS *pr) {
  assert(pr);
  assert(pr->refCount);
  return pr->foregroundColor;
}



void HtmlProps_SetForegroundColor(HTML_PROPS *pr, uint32_t c) {
  assert(pr);
  assert(pr->refCount);
  pr->foregroundColor=c;
}



uint32_t HtmlProps_GetBackgroundColor(const HTML_PROPS *pr) {
  assert(pr);
  assert(pr->refCount);
  return pr->backgroundColor;
}



void HtmlProps_SetBackgroundColor(HTML_PROPS *pr, uint32_t c) {
  assert(pr);
  assert(pr->refCount);
  pr->backgroundColor=c;
}







