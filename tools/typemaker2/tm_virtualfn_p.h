/***************************************************************************
    begin       : Mon Jun 28 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



#ifndef TYPEMAKER2_VIRTUALFN_P_H
#define TYPEMAKER2_VIRTUALFN_P_H

#include "tm_virtualfn.h"


struct TYPEMAKER2_VIRTUALFN_PARAM {
  GWEN_LIST_ELEMENT(TYPEMAKER2_VIRTUALFN_PARAM)

  char *name;
  char *type;
  char *descr;
};




struct TYPEMAKER2_VIRTUALFN {
  GWEN_LIST_ELEMENT(TYPEMAKER2_VIRTUALFN)

  char *name;
  char *location;

  char *descr;

  int access;
  uint32_t flags;

  char *returnType;
  char *returnTypeDescr;
  char *defaultReturnValue;
  char *preset;

  TYPEMAKER2_VIRTUALFN_PARAM_LIST *paramList;

  int refCount;

};



#endif



