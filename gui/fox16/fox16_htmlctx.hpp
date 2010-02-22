/***************************************************************************
 begin       : Mon Feb 22 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef FOX16_HTMLCTX_HPP
#define FOX16_HTMLCTX_HPP


#include "htmlctx_be.h"

#include <fx.h>


class FOX16_HtmlCtx {
    friend class FOX16_HtmlCtxLinker;

public:
  FOX16_HtmlCtx(uint32_t flags, uint32_t guiid, int timeout);
  ~FOX16_HtmlCtx();

  int getTextWidth(HTML_FONT *fnt,
		   const char *s);

  int getTextHeight(HTML_FONT *fnt,
		    const char *s);

protected:
  GWEN_XML_CONTEXT *_context;
  FXFont *_font;

  FXFont *_getFoxFont(HTML_FONT *fnt);
};




#endif


