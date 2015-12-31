/***************************************************************************
    begin       : Thu Jul 02 2009
    copyright   : (C) 2009 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



#ifndef TYPEMAKER2_GROUP_P_H
#define TYPEMAKER2_GROUP_P_H

#include "tm_group.h"


struct TYPEMAKER2_GROUP {
  GWEN_TREE_ELEMENT(TYPEMAKER2_GROUP)

  char *title;
  char *description;

  int refCount;
};



#endif



