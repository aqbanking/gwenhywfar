/***************************************************************************
 begin       : Fri Jul 16 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef HTMLIMAGE_P_H
#define HTMLIMAGE_P_H

#include "htmlimage_be.h"


struct HTML_IMAGE {
  GWEN_INHERIT_ELEMENT(HTML_IMAGE)
  GWEN_LIST_ELEMENT(HTML_IMAGE)

  char *imageName;
  int width;
  int height;

  uint32_t refCount;
};



#endif

