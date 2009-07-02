/***************************************************************************
    begin       : Thu Jul 02 2009
    copyright   : (C) 2009 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef TYPEMAKER2_TYPEMANAGER_P_H
#define TYPEMAKER2_TYPEMANAGER_P_H

#include "tm_typemanager.h"


struct TYPEMAKER2_TYPEMANAGER {

  TYPEMAKER2_TYPE_LIST *typeList;

  GWEN_STRINGLIST *folders;

  char *lang;
  char *apiDeclaration;
};


TYPEMAKER2_TYPE *Typemaker2_TypeManager_LoadType(TYPEMAKER2_TYPEMANAGER *tym, const char *typeName);

int Typemaker2_TypeManager_SetTypePtrs(TYPEMAKER2_TYPEMANAGER *tym, TYPEMAKER2_TYPE *ty);
int Typemaker2_TypeManager_SetMemberTypePtrs(TYPEMAKER2_TYPEMANAGER *tym, TYPEMAKER2_TYPE *ty);




#endif




