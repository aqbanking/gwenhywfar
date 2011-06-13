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


#include "htmlfont_p.h"



GWEN_INHERIT_FUNCTIONS(HTML_FONT)
GWEN_LIST_FUNCTIONS(HTML_FONT, HtmlFont)


HTML_FONT *HtmlFont_new(void) {
  HTML_FONT *fnt;

  GWEN_NEW_OBJECT(HTML_FONT, fnt);
  fnt->refCount=1;
  GWEN_INHERIT_INIT(HTML_FONT, fnt);
  GWEN_LIST_INIT(HTML_FONT, fnt);

  return fnt;
}



void HtmlFont_free(HTML_FONT *fnt) {
  if (fnt) {
    assert(fnt->refCount);
    if (fnt->refCount>1)
      fnt->refCount--;
    else {
      GWEN_LIST_FINI(HTML_FONT, fnt);
      GWEN_INHERIT_FINI(HTML_FONT, fnt);
      free(fnt->fontName);

      fnt->refCount=0;
      GWEN_FREE_OBJECT(fnt);
    }
  }
}



void HtmlFont_Attach(HTML_FONT *fnt) {
  assert(fnt);
  assert(fnt->refCount);
  fnt->refCount++;
}



const char *HtmlFont_GetFontName(const HTML_FONT *fnt) {
  assert(fnt);
  assert(fnt->refCount);
  return fnt->fontName;
}



void HtmlFont_SetFontName(HTML_FONT *fnt, const char *s) {
  assert(fnt);
  assert(fnt->refCount);
  free(fnt->fontName);
  if (s) fnt->fontName=strdup(s);
  else fnt->fontName=NULL;
}



int HtmlFont_GetFontSize(const HTML_FONT *fnt) {
  assert(fnt);
  assert(fnt->refCount);
  return fnt->fontSize;
}



void HtmlFont_SetFontSize(HTML_FONT *fnt, int i) {
  assert(fnt);
  assert(fnt->refCount);
  fnt->fontSize=i;
}



uint32_t HtmlFont_GetFontFlags(const HTML_FONT *fnt) {
  assert(fnt);
  assert(fnt->refCount);
  return fnt->fontFlags;
}



void HtmlFont_SetFontFlags(HTML_FONT *fnt, uint32_t i) {
  assert(fnt);
  assert(fnt->refCount);
  fnt->fontFlags=i;
}



void HtmlFont_AddFontFlags(HTML_FONT *fnt, uint32_t i) {
  assert(fnt);
  assert(fnt->refCount);
  fnt->fontFlags|=i;
}



void HtmlFont_SubFontFlags(HTML_FONT *fnt, uint32_t i) {
  assert(fnt);
  assert(fnt->refCount);
  fnt->fontFlags&=~i;
}




