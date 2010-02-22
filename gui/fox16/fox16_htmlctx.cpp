/***************************************************************************
 begin       : Mon Feb 22 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "fox16_htmlctx_p.hpp"
#include <assert.h>

#include <gwenhywfar/inherit.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui_be.h>
#include <gwenhywfar/i18n.h>

#include <gwenhywfar/text.h>
#include <gwenhywfar/debug.h>



GWEN_INHERIT(GWEN_XML_CONTEXT, FOX16_HtmlCtx)
GWEN_INHERIT(HTML_FONT, FXFont)



int FOX16_HtmlCtxLinker::GetTextWidth(GWEN_XML_CONTEXT *ctx,
				      HTML_FONT *fnt,
				      const char *s) {
  FOX16_HtmlCtx *xctx;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, FOX16_HtmlCtx, ctx);
  assert(xctx);

  return xctx->getTextWidth(fnt, s);
}



int FOX16_HtmlCtxLinker::GetTextHeight(GWEN_XML_CONTEXT *ctx,
				       HTML_FONT *fnt,
				       const char *s) {
  FOX16_HtmlCtx *xctx;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, FOX16_HtmlCtx, ctx);
  assert(xctx);

  return xctx->getTextHeight(fnt, s);
}



void FOX16_HtmlCtxLinker::freeData(void *bp, void *p) {
  FOX16_HtmlCtx *xctx;

  xctx=(FOX16_HtmlCtx*) p;
  if (xctx->_context)
    xctx->_context=NULL;
  delete xctx;
}



void FOX16_HtmlCtxLinker::freeFontData(void *bp, void *p) {
  FXFont *xfnt;

  xfnt=(FXFont*) p;
  delete xfnt;
}






FOX16_HtmlCtx::FOX16_HtmlCtx(uint32_t flags, uint32_t guiid, int timeout)
:_context(NULL)
,_font(NULL)
{
  HTML_PROPS *pr;
  HTML_FONT *fnt;

  _context=HtmlCtx_new(flags, guiid, timeout);
  GWEN_INHERIT_SETDATA(GWEN_XML_CONTEXT, FOX16_HtmlCtx, _context, this,
		       FOX16_HtmlCtxLinker::freeData);
  _font=FXApp::instance()->getNormalFont();

  pr=HtmlProps_new();
  fnt=HtmlCtx_GetFont(_context, _font->getName().text(), _font->getSize()/10, 0);
  HtmlProps_SetFont(pr, fnt);
  HtmlFont_free(fnt);
  HtmlCtx_SetStandardProps(_context, pr);
  HtmlProps_free(pr);

}



FOX16_HtmlCtx::~FOX16_HtmlCtx() {
  if (_context) {
    GWEN_INHERIT_UNLINK(GWEN_XML_CONTEXT, FOX16_HtmlCtx, _context);
    GWEN_XmlCtx_free(_context);
  }
}



FXFont *FOX16_HtmlCtx::_getFoxFont(HTML_FONT *fnt) {
  FXFont *xfnt;

  if (GWEN_INHERIT_ISOFTYPE(HTML_FONT, FXFont, fnt)) {
    xfnt=GWEN_INHERIT_GETDATA(HTML_FONT, FXFont, fnt);
    return xfnt;
  }
  else {
    FXuint size;
    FXuint weight;
    FXuint slant;
    FXuint encoding;
    FXString face;
    uint32_t flags;

    if (HtmlFont_GetFontName(fnt))
      face=HtmlFont_GetFontName(fnt);
    else
      face=_font->getName();
    size=HtmlFont_GetFontSize(fnt)*10;
    weight=FXFont::Normal;
    slant=_font->getSlant();
    encoding=_font->getEncoding();

    flags=HtmlFont_GetFontFlags(fnt);
    if (flags & HTML_FONT_FLAGS_STRONG)
      weight=FXFont::Bold;
    if (flags & HTML_FONT_FLAGS_ITALIC)
      weight=FXFont::Italic;

    xfnt=new FXFont(FXApp::instance(), face, size, weight, slant, encoding);
    if (xfnt==NULL) {
      DBG_ERROR(GWEN_LOGDOMAIN,
		"Could not create font [%s], size=%d, weight=%d, slant=%d, encoding=%d",
		face.text(), size, weight, slant, encoding);
      return NULL;
    }
    xfnt->create();
    GWEN_INHERIT_SETDATA(HTML_FONT, FXFont, fnt, xfnt,
			 FOX16_HtmlCtxLinker::freeFontData);
    return xfnt;
  }
}



int FOX16_HtmlCtx::getTextWidth(HTML_FONT *fnt, const char *s) {
  if (s==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "NULLPOINTER, returning size 0");
    return 0;
  }
  else {
    FXFont *xfnt;
    FXString str;

    str=FXString(s);
    xfnt=_getFoxFont(fnt);
    if (xfnt==NULL)
      return _font->getTextWidth(str);
    else
      return xfnt->getTextWidth(str);
  }
}



int FOX16_HtmlCtx::getTextHeight(HTML_FONT *fnt, const char *s) {
  if (s==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "NULLPOINTER, returning size 0");
    return 0;
  }
  else {
    FXFont *xfnt;
    FXString str;

    str=FXString(s);
    xfnt=_getFoxFont(fnt);
    if (xfnt==NULL)
      return _font->getTextHeight(str);
    else
      return xfnt->getTextHeight(str);
  }
}







