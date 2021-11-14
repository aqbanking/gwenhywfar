/***************************************************************************
    begin       : Thu Jul 02 2009
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef TM2C_LIST1_H
#define TM2C_LIST1_H


#include "tm_builder.h"



int TM2C_BuildList1GetByMember(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm);
int TM2C_BuildList1Dup(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty);
int TM2C_BuildList1SortByMember(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm);



#endif

