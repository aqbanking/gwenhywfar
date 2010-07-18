/***************************************************************************
 begin       : Sat Feb 20 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef HTMLPROPS_P_H
#define HTMLPROPS_P_H


#include "htmlprops_be.h"


struct HTML_PROPS {
  HTML_FONT *font;
  uint32_t foregroundColor;
  uint32_t backgroundColor;

  uint32_t refCount;
};



#endif

