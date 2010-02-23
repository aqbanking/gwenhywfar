/***************************************************************************
 begin       : Mon Feb 22 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef FOX16_HTMLCTX_HPP
#define FOX16_HTMLCTX_HPP


#include "cppgui.hpp"
#include "htmlctx_be.h"

#include <fx.h>


class FOX16GUI_API FOX16_HtmlCtx {
    friend class FOX16_HtmlCtxLinker;

public:
  FOX16_HtmlCtx(uint32_t flags, uint32_t guiid, int timeout);
  ~FOX16_HtmlCtx();

  void setText(const char *s);

  int getWidth();
  int getHeight();

  int layout(int width, int height);
  void dump();

  void paint(FXDC *dc, int xOffset, int yOffset);

  HTML_PROPS *getStandardProps();
  void setStandardProps(HTML_PROPS *pr);

protected:
  GWEN_XML_CONTEXT *_context;
  FXFont *_font;

  FXFont *_getFoxFont(HTML_FONT *fnt);
  void _paint(FXDC *dc, HTML_OBJECT *o, int xOffset, int yOffset);

  int getTextWidth(HTML_FONT *fnt,
		   const char *s);

  int getTextHeight(HTML_FONT *fnt,
		    const char *s);

  uint32_t getColorFromName(const char *name);

};




#endif


