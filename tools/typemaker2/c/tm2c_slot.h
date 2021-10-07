/***************************************************************************
    begin       : Thu Jul 02 2009
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef TM2C_SLOT_H
#define TM2C_SLOT_H


#include "tm_builder.h"



int TM2C_BuildSlotFunctions(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty);
int TM2C_AddVirtualFnsFromSlots(TYPEMAKER2_TYPE *ty);
void TM2C_DelVirtualFnsFromSlots(TYPEMAKER2_TYPE *ty);



#endif

