/***************************************************************************
 begin       : Mon Feb 22 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef FOX16_HTMLCTX_HPP
#define FOX16_HTMLCTX_HPP


#include <gwen-gui-fox16/cppgui.hpp>
#include <gwen-gui-fox16/html/htmlobject_be.h>
#include <gwen-gui-fox16/html/htmlfont_be.h>
#include <gwen-gui-fox16/html/htmlctx_be.h>

#include <fx.h>


class FOX16GUI_API FOX16_HtmlCtx {
    friend class FOX16_HtmlCtxLinker;

public:
  FOX16_HtmlCtx(uint32_t flags);
  ~FOX16_HtmlCtx();

  void setText(const char *s);

  int getWidth();
  int getHeight();

  HTML_FONT *getFont(const char *fontName,
		     int fontSize,
		     uint32_t fontFlags);

  int layout(int width, int height);
  void dump();

  void paint(FXDC *dc, int xOffset, int yOffset);

  void setBackgroundColor(FXColor c);
  void setForegroundColor(FXColor c);

protected:
  GWEN_XML_CONTEXT *_context;
  FXFont *_font;
  FXColor _fgColor;
  FXColor _bgColor;

  FXFont *_getFoxFont(HTML_FONT *fnt);
  void _paint(FXDC *dc, HTML_OBJECT *o, int xOffset, int yOffset);

  int getTextWidth(HTML_FONT *fnt,
		   const char *s);

  int getTextHeight(HTML_FONT *fnt,
		    const char *s);

  uint32_t getColorFromName(const char *name);

};




#endif


