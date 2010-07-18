/***************************************************************************
 begin       : Sat Feb 20 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef HTMLFONT_P_H
#define HTMLFONT_P_H

#include "htmlfont_be.h"


struct HTML_FONT {
  GWEN_INHERIT_ELEMENT(HTML_FONT)
  GWEN_LIST_ELEMENT(HTML_FONT)

  char *fontName;
  int fontSize;
  uint32_t fontFlags;

  uint32_t refCount;
};



#endif

