/***************************************************************************
 begin       : Sat Feb 20 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef HTMLOBJECT_P_H
#define HTMLOBJECT_P_H


#include "htmlobject_be.h"


struct HTML_OBJECT {
  GWEN_TREE_ELEMENT(HTML_OBJECT)
  GWEN_INHERIT_ELEMENT(HTML_OBJECT)

  GWEN_XML_CONTEXT *xmlCtx;
  HTML_OBJECT_TYPE objectType;

  HTML_PROPS *properties;
  int x;
  int y;
  int width;
  int height;

  uint32_t flags;

  int configuredWidth;
  int configuredHeight;

  char *text;

  HTML_OBJECT_LAYOUT_FN layoutFn;

  uint32_t refCount;
};



#endif

