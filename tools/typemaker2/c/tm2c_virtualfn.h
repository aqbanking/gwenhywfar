/***************************************************************************
    begin       : Thu Jul 02 2009
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef TM2C_VIRTUALFN_H
#define TM2C_VIRTUALFN_H


#include "tm_builder.h"



int TM2C_BuildDefineVirtualFns(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, const char *loc);
int TM2C_BuildProtoVirtualFns(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty);
int TM2C_BuildCodeVirtualFns(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty);
int TM2C_BuildProtoSetterVirtualFns(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty);
int TM2C_BuildSetterVirtualFns(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty);
int TM2C_BuildProtoGetterVirtualFns(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty);
int TM2C_BuildGetterVirtualFns(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty);




#endif

