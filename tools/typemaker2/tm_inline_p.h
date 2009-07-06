/***************************************************************************
    begin       : Thu Jul 02 2009
    copyright   : (C) 2009 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



#ifndef TYPEMAKER2_INLINE_P_H
#define TYPEMAKER2_INLINE_P_H

#include "tm_inline.h"


struct TYPEMAKER2_INLINE {
  GWEN_LIST_ELEMENT(TYPEMAKER2_INLINE)

  char *content;
  int acc;
  int location;

  uint32_t typeFlagsMask;
  uint32_t typeFlagsValue;

  int refCount;
};



#endif



