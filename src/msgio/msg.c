/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2023 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/*#define DISABLE_DEBUGLOG*/


#include "msgio/msg_p.h"

#include <gwenhywfar/error.h>
#include <gwenhywfar/misc.h>



GWEN_LIST_FUNCTIONS(GWEN_MSG, GWEN_Msg)



GWEN_MSG *GWEN_Msg_new(uint32_t bufferSize)
{
  GWEN_MSG *msg;

  GWEN_NEW_OBJECT(GWEN_MSG, msg);
  GWEN_LIST_INIT(GWEN_MSG, msg);
  if (bufferSize) {
    msg->buffer=(uint8_t*) malloc(bufferSize);
    msg->maxSize=bufferSize;
  }
  return msg;
}



void GWEN_Msg_free(GWEN_MSG *msg)
{
  if (msg) {
    GWEN_LIST_FINI(GWEN_MSG, msg);
    free(msg->buffer);
    GWEN_FREE_OBJECT(msg);
  }
}



GWEN_MSG *GWEN_Msg_dup(const GWEN_MSG *srcMsg)
{
  GWEN_MSG *msg;

  msg=GWEN_Msg_new(srcMsg->maxSize);
  if (srcMsg->maxSize)
    memmove(msg->buffer, srcMsg->buffer, msg->maxSize);
  msg->bytesInBuffer=srcMsg->bytesInBuffer;
  msg->currentPos=srcMsg->currentPos;

  return msg;
}



uint8_t *GWEN_Msg_GetBuffer(GWEN_MSG *msg)
{
  if (msg)
    return msg->buffer;
  return NULL;
}



const uint8_t *GWEN_Msg_GetConstBuffer(const GWEN_MSG *msg)
{
  if (msg)
    return msg->buffer;
  return NULL;
}



uint8_t GWEN_Msg_GetBytesInBuffer(const GWEN_MSG *msg)
{
  if (msg)
    return msg->bytesInBuffer;
  else
    return 0;
}



uint8_t GWEN_Msg_GetCurrentPos(const GWEN_MSG *msg)
{
  if (msg)
    return msg->currentPos;
  else
    return 0;
}



int GWEN_Msg_AddByte(GWEN_MSG *msg, uint8_t b)
{
  if (msg) {
    if ((msg->bytesInBuffer<msg->maxSize) &&
        (msg->currentPos<msg->maxSize)) {
      msg->buffer[(msg->currentPos)++]=b;
      msg->bytesInBuffer++;
      return 0;
    }
  }
  return GWEN_ERROR_MEMORY_FULL;
}



int GWEN_Msg_ReadNextByte(GWEN_MSG *msg)
{
  if (msg) {
    if ((msg->currentPos<msg->maxSize) &&
        (msg->currentPos<msg->bytesInBuffer)) {
      return ((int)(msg->buffer[(msg->currentPos)++])) & 0xff;
    }
  }
  return GWEN_ERROR_EOF;
}



int GWEN_Msg_IncCurrentPos(GWEN_MSG *msg, uint8_t i)
{
  if (msg) {
    if (((msg->currentPos+i)<msg->maxSize) &&
        ((msg->currentPos+i)<msg->bytesInBuffer)) {
      msg->currentPos+=i;
      return 0;
    }
  }
  return GWEN_ERROR_EOF;
}



int GWEN_Msg_RewindCurrentPos(GWEN_MSG *msg)
{
  if (msg) {
    msg->currentPos=0;
    return 0;
  }
  return GWEN_ERROR_EOF;
}



int GWEN_Msg_GetRemainingBytes(const GWEN_MSG *msg)
{
  if (msg)
    return msg->bytesInBuffer-msg->currentPos;
  return 0;
}





