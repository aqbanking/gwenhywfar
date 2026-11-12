/***************************************************************************
 begin       : Mon Feb 22 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef FOX16_HTMLCTX_P_HPP
#define FOX16_HTMLCTX_P_HPP


#include "fox16_htmlctx.hpp"


class FOX16_HtmlCtxLinker {
  friend class FOX16_HtmlCtx;

  static int GetTextWidth(GWEN_XML_CONTEXT *ctx,
                          HTML_FONT *fnt,
                          const char *s);

  static int GetTextHeight(GWEN_XML_CONTEXT *ctx,
                           HTML_FONT *fnt,
                           const char *s);

  static uint32_t GetColorFromName(const GWEN_XML_CONTEXT *ctx,
                                   const char *name);

  static HTML_FONT *GetFont(GWEN_XML_CONTEXT *ctx,
                            const char *fontName,
                            int fontSize,
                            uint32_t fontFlags);

  static HTML_IMAGE *GetImage(GWEN_XML_CONTEXT *ctx,
                              const char *imageName);

  static void GWENHYWFAR_CB freeData(void *bp, void *p);
  static void GWENHYWFAR_CB freeFontData(void *bp, void *p);
  static void GWENHYWFAR_CB freeImageData(void *bp, void *p);

};




#endif


