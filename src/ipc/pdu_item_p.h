/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2021 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifndef GWEN_PDU_ITEM_P_H
#define GWEN_PDU_ITEM_P_H


#include <gwenhywfar/pdu_item.h>


struct GWEN_PDU_ITEM {
  GWEN_TREE_ELEMENT(GWEN_PDU_ITEM)

  int8_t type;
  uint32_t length;
  const uint8_t *dataPointer;

  int ownData;
  int refCount;
};



#endif
