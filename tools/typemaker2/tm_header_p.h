/***************************************************************************
    begin       : Thu Jul 02 2009
    copyright   : (C) 2009 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



#ifndef TYPEMAKER2_HEADER_P_H
#define TYPEMAKER2_HEADER_P_H

#include "tm_header.h"


struct TYPEMAKER2_HEADER {
  GWEN_LIST_ELEMENT(TYPEMAKER2_HEADER)

  char *fileName;
  int type;
  int location;

  int refCount;
};



#endif



