/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2023 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/


#ifndef GWEN_MSG_MSG_P_H
#define GWEN_MSG_MSG_P_H

#include "msgio/msg.h"


struct GWEN_MSG {
  GWEN_LIST_ELEMENT(GWEN_MSG)

  uint32_t bytesInBuffer;
  uint32_t currentPos;
  uint32_t maxSize;
  uint8_t *buffer;

  uint32_t flags;

  uint32_t parsedPayloadSize;
  uint32_t parsedPayloadOffset;

  int groupId;

  GWEN_DB_NODE *dbParsedInfo;

  int refCount;
};



#endif


