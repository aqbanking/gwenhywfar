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
#include "fox16_gui.hpp"
#include "htmlctx_be.h"
#include "o_image_l.h"

#include <assert.h>

#include <gwenhywfar/inherit.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui_be.h>
#include <gwenhywfar/i18n.h>

#include <gwenhywfar/text.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/syncio_memory.h>
#include <gwenhywfar/directory.h>



GWEN_INHERIT(GWEN_XML_CONTEXT, FOX16_HtmlCtx)
GWEN_INHERIT(HTML_FONT, FXFont)
GWEN_INHERIT(HTML_IMAGE, FXImage)



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



uint32_t FOX16_HtmlCtxLinker::GetColorFromName(const GWEN_XML_CONTEXT *ctx,
					       const char *name) {
  FOX16_HtmlCtx *xctx;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, FOX16_HtmlCtx, ctx);
  assert(xctx);

  return xctx->getColorFromName(name);
}



HTML_FONT *FOX16_HtmlCtxLinker::GetFont(GWEN_XML_CONTEXT *ctx,
					const char *fontName,
					int fontSize,
					uint32_t fontFlags) {
  FOX16_HtmlCtx *xctx;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, FOX16_HtmlCtx, ctx);
  assert(xctx);

  return xctx->getFont(fontName, fontSize, fontFlags);
}



HTML_IMAGE *FOX16_HtmlCtxLinker::GetImage(GWEN_XML_CONTEXT *ctx,
					  const char *imageName) {
  FOX16_HtmlCtx *xctx;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, FOX16_HtmlCtx, ctx);
  assert(xctx);

  return xctx->getImage(imageName);

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




void FOX16_HtmlCtxLinker::freeImageData(void *bp, void *p) {
  FXImage *ximg;

  ximg=(FXImage*) p;
  delete ximg;
}





FOX16_HtmlCtx::FOX16_HtmlCtx(uint32_t flags)
:_context(NULL)
,_font(NULL)
,_fgColor(0)
,_bgColor(0)
,m_iconSource(NULL)
{
  HTML_PROPS *pr;
  HTML_FONT *fnt;

  _context=HtmlCtx_new(flags);
  GWEN_INHERIT_SETDATA(GWEN_XML_CONTEXT, FOX16_HtmlCtx, _context, this,
		       FOX16_HtmlCtxLinker::freeData);
  _font=FXApp::instance()->getNormalFont();
  HtmlCtx_SetGetTextWidthFn(_context, FOX16_HtmlCtxLinker::GetTextWidth);
  HtmlCtx_SetGetTextHeightFn(_context, FOX16_HtmlCtxLinker::GetTextHeight);
  HtmlCtx_SetGetColorFromNameFn(_context, FOX16_HtmlCtxLinker::GetColorFromName);
  HtmlCtx_SetGetFontFn(_context, FOX16_HtmlCtxLinker::GetFont);
  HtmlCtx_SetGetImageFn(_context, FOX16_HtmlCtxLinker::GetImage);

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
    size=HtmlFont_GetFontSize(fnt);
    weight=FXFont::Normal;
    slant=_font->getSlant();
    encoding=_font->getEncoding();

    flags=HtmlFont_GetFontFlags(fnt);
    if (flags & HTML_FONT_FLAGS_STRONG)
      weight=FXFont::Bold;
    if (flags & HTML_FONT_FLAGS_ITALIC)
      slant=FXFont::Italic;

    DBG_DEBUG(GWEN_LOGDOMAIN,
	      "Creating font [%s], size=%d, weight=%d, slant=%d, encoding=%d",
	      face.text(), size, weight, slant, encoding);

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



uint32_t FOX16_HtmlCtx::getColorFromName(const char *name) {
  return fxcolorfromname(name);
}



int FOX16_HtmlCtx::layout(int width, int height) {
  return HtmlCtx_Layout(_context, width, height);
}



void FOX16_HtmlCtx::setText(const char *s) {
  int rv;

  rv=GWEN_XMLContext_ReadFromString(_context, s);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
  }
}



void FOX16_HtmlCtx::addMediaPath(const char *s) {
  HtmlCtx_AddMediaPath(_context, s);
}



static void dumpObject(HTML_OBJECT *o, FILE *f, int indent) {
  HTML_OBJECT *c;
  int i;
  const char *s;
  HTML_PROPS *pr;
  HTML_FONT *fnt;

  s=HtmlObject_GetText(o);
  for (i=0; i<indent; i++) fprintf(f, " ");
  fprintf(stderr, "Object type: %d [%s] flags: %08x, x=%d, y=%d, w=%d, h=%d\n",
	  HtmlObject_GetObjectType(o),
	  s?s:"(empty)",
	  HtmlObject_GetFlags(o),
          HtmlObject_GetX(o),
          HtmlObject_GetY(o),
          HtmlObject_GetWidth(o),
          HtmlObject_GetHeight(o));

  pr=HtmlObject_GetProperties(o);
  fnt=HtmlProps_GetFont(pr);

  for (i=0; i<indent+2; i++) fprintf(f, " ");
  fprintf(stderr, "fgcol=%06x, bgcol=%06x, fontsize=%d, fontflags=%08x, fontname=[%s]\n",
	  HtmlProps_GetForegroundColor(pr),
	  HtmlProps_GetBackgroundColor(pr),
	  HtmlFont_GetFontSize(fnt),
	  HtmlFont_GetFontFlags(fnt),
	  HtmlFont_GetFontName(fnt));

  c=HtmlObject_Tree_GetFirstChild(o);
  while(c) {
    dumpObject(c, f, indent+2);
    c=HtmlObject_Tree_GetNext(c);
  }
}



void FOX16_HtmlCtx::dump() {
  HTML_OBJECT *o;

  o=HtmlCtx_GetRootObject(_context);
  if (o)
    dumpObject(o, stderr, 2);
}



void FOX16_HtmlCtx::_paint(FXDC *dc, HTML_OBJECT *o, int xOffset, int yOffset) {
  HTML_OBJECT *c;

  xOffset+=HtmlObject_GetX(o);
  yOffset+=HtmlObject_GetY(o);

  switch(HtmlObject_GetObjectType(o)) {
  case HtmlObjectType_Word: {
    HTML_PROPS *pr;
    HTML_FONT *fnt;
    FXFont *xfnt;
    int ascent=0;
    uint32_t col;

    pr=HtmlObject_GetProperties(o);

    /* select font */
    fnt=HtmlProps_GetFont(pr);
    xfnt=_getFoxFont(fnt);
    if (xfnt) {
      dc->setFont(xfnt);
      ascent=xfnt->getFontAscent();
      //DBG_ERROR(0, "Font size: %d (%d)", xfnt->getSize(), HtmlFont_GetFontSize(fnt));
    }

    /* select foreground color */
    col=HtmlProps_GetForegroundColor(pr);
    if (col==HTML_PROPS_NOCOLOR)
      dc->setForeground(_fgColor);
    else
      dc->setForeground(col);

    /* select background color */
    col=HtmlProps_GetBackgroundColor(pr);
    if (col==HTML_PROPS_NOCOLOR)
      dc->setBackground(_bgColor);
    else
      dc->setBackground(col);

    dc->drawText(xOffset, yOffset+ascent, HtmlObject_GetText(o));
    break;
  }

  case HtmlObjectType_Image: {
    HTML_IMAGE *img;

    img=HtmlObject_Image_GetImage(o);
    if (img) {
      FXImage *ximg;

      ximg=GWEN_INHERIT_GETDATA(HTML_IMAGE, FXImage, img);
      if (ximg) {
	HTML_PROPS *pr;
	uint32_t col;

	pr=HtmlObject_GetProperties(o);

	/* select background color */
	col=HtmlProps_GetBackgroundColor(pr);
	if (col==HTML_PROPS_NOCOLOR) {
	  dc->setBackground(_bgColor);
	  dc->setForeground(_bgColor);
	}
	else {
	  dc->setBackground(col);
	  dc->setForeground(col);
	}

	dc->fillRectangle(xOffset, yOffset, ximg->getWidth(), ximg->getHeight());

	dc->drawImage(ximg, xOffset, yOffset);
      }
    }
    break;
  }
  default:
    break;
  }

  c=HtmlObject_Tree_GetFirstChild(o);
  while(c) {
    _paint(dc, c, xOffset, yOffset);
    c=HtmlObject_Tree_GetNext(c);
  }
}



void FOX16_HtmlCtx::_paintAt(FXDC *dc, HTML_OBJECT *o,
			     int xOffset, int yOffset,
			     int xText, int yText,
			     int w, int h) {
  HTML_OBJECT *c;
  int x;
  int y;
  int printX;
  int printY;
  int objectW;
  int objectH;

  x=xText+HtmlObject_GetX(o);
  y=yText+HtmlObject_GetY(o);
  objectW=HtmlObject_GetWidth(o);
  objectH=HtmlObject_GetHeight(o);

  printX=x-xOffset;
  printY=y-yOffset;

  if (printX<w && printX+objectW>=0 &&
      printY<h && printY+objectH>=0) {
    switch(HtmlObject_GetObjectType(o)) {
    case HtmlObjectType_Word: {
      HTML_PROPS *pr;
      HTML_FONT *fnt;
      FXFont *xfnt;
      int ascent=0;
      uint32_t col;
  
      pr=HtmlObject_GetProperties(o);
  
      /* select font */
      fnt=HtmlProps_GetFont(pr);
      xfnt=_getFoxFont(fnt);
      if (xfnt) {
	dc->setFont(xfnt);
	ascent=xfnt->getFontAscent();
	//DBG_ERROR(0, "Font size: %d (%d)", xfnt->getSize(), HtmlFont_GetFontSize(fnt));
      }
  
      /* select foreground color */
      col=HtmlProps_GetForegroundColor(pr);
      if (col==HTML_PROPS_NOCOLOR)
	dc->setForeground(_fgColor);
      else
	dc->setForeground(col);
  
      /* select background color */
      col=HtmlProps_GetBackgroundColor(pr);
      if (col==HTML_PROPS_NOCOLOR)
	dc->setBackground(_bgColor);
      else
	dc->setBackground(col);
  
      dc->drawText(printX, printY+ascent, HtmlObject_GetText(o));
      break;
    }

    case HtmlObjectType_Image: {
      HTML_IMAGE *img;
  
      img=HtmlObject_Image_GetImage(o);
      if (img) {
	FXImage *ximg;
  
	ximg=GWEN_INHERIT_GETDATA(HTML_IMAGE, FXImage, img);
	if (ximg) {
	  HTML_PROPS *pr;
	  uint32_t col;

	  pr=HtmlObject_GetProperties(o);

	  /* select background color */
	  col=HtmlProps_GetBackgroundColor(pr);
	  if (col==HTML_PROPS_NOCOLOR) {
	    dc->setBackground(_bgColor);
	    dc->setForeground(_bgColor);
	  }
	  else {
	    dc->setBackground(col);
	    dc->setForeground(col);
	  }
	  dc->fillRectangle(xOffset, yOffset, ximg->getWidth(), ximg->getHeight());

	  dc->drawImage(ximg, printX, printY);
	}
      }
      break;
    }
    default:
      break;
    }


    c=HtmlObject_Tree_GetFirstChild(o);
    while(c) {
      _paintAt(dc, c, xOffset, yOffset, x, y, w, h);
      c=HtmlObject_Tree_GetNext(c);
    }
  }
}



void FOX16_HtmlCtx::paint(FXDC *dc, int xOffset, int yOffset) {
  HTML_OBJECT *o;

  o=HtmlCtx_GetRootObject(_context);
  if (o)
    _paint(dc, o, xOffset, yOffset);
}



void FOX16_HtmlCtx::paintAt(FXDC *dc,
			    int xOffset, int yOffset,
			    int xText, int yText,
			    int w, int h) {
  HTML_OBJECT *o;

  o=HtmlCtx_GetRootObject(_context);
  if (o)
    _paintAt(dc, o, xOffset, yOffset, xText, yText, w, h);
}



int FOX16_HtmlCtx::getWidth() {
  HTML_OBJECT *o;

  o=HtmlCtx_GetRootObject(_context);
  if (o)
    return HtmlObject_GetWidth(o);
  else
    return -1;
}



int FOX16_HtmlCtx::getHeight() {
  HTML_OBJECT *o;

  o=HtmlCtx_GetRootObject(_context);
  if (o)
    return HtmlObject_GetHeight(o);
  else
    return -1;
}



HTML_FONT *FOX16_HtmlCtx::getFont(const char *fontName,
				  int fontSize,
				  uint32_t fontFlags) {
  FOX16_Gui *gui;

  gui=FOX16_Gui::getFgGui();
  assert(gui);

  return gui->getFont(fontName, fontSize, fontFlags);
}



HTML_IMAGE *FOX16_HtmlCtx::getImage(const char *fileName) {
  GWEN_STRINGLIST *sl;

  sl=HtmlCtx_GetMediaPaths(_context);
  if (sl) {
    GWEN_BUFFER *tbuf;
    int rv;
    FXImage *ximg;
    HTML_IMAGE *img;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    rv=GWEN_Directory_FindFileInPaths(sl, fileName, tbuf);
    if (rv<0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(tbuf);
      return NULL;
    }

    if (m_iconSource==NULL)
      m_iconSource=new FXIconSource(FXApp::instance());

    DBG_ERROR(0, "Loading [%s]", GWEN_Buffer_GetStart(tbuf));
    ximg=m_iconSource->loadIconFile(GWEN_Buffer_GetStart(tbuf));
    if (ximg==NULL) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Could not load icon [%s]", GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_free(tbuf);
      return NULL;
    }

    ximg->create();
    img=HtmlImage_new();
    HtmlImage_SetImageName(img, GWEN_Buffer_GetStart(tbuf));
    HtmlImage_SetWidth(img, ximg->getWidth());
    HtmlImage_SetHeight(img, ximg->getHeight());

    GWEN_INHERIT_SETDATA(HTML_IMAGE, FXImage, img, ximg,
			 FOX16_HtmlCtxLinker::freeImageData);
    GWEN_Buffer_free(tbuf);
    return img;
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "No media paths in dialog");
    return NULL;
  }
}




void FOX16_HtmlCtx::setBackgroundColor(FXColor c) {
  _bgColor=c;
}



void FOX16_HtmlCtx::setForegroundColor(FXColor c) {
  _fgColor=c;
}








