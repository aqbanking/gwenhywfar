/***************************************************************************
    begin       : Thu Jul 02 2009
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef TM2C_ENUMS_H
#define TM2C_ENUMS_H


#include "tm_builder.h"



int TM2C_BuildDefineEnums(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty);
int TM2C_SetEnumStringFns(TYPEMAKER2_TYPE *ty);
int TM2C_BuildEnumFromString(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty);
int TM2C_BuildEnumToString(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty);






#endif

