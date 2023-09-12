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
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>



GWEN_LIST_FUNCTIONS(GWEN_MSG, GWEN_Msg)



GWEN_MSG *GWEN_Msg_new(uint32_t bufferSize)
{
  GWEN_MSG *msg;

  GWEN_NEW_OBJECT(GWEN_MSG, msg);
  msg->refCount=1;
  GWEN_INHERIT_INIT(GWEN_MSG, msg);
  GWEN_LIST_INIT(GWEN_MSG, msg);
  if (bufferSize) {
    msg->buffer=(uint8_t*) malloc(bufferSize);
    msg->maxSize=bufferSize;
  }
  return msg;
}



GWEN_MSG *GWEN_Msg_fromBytes(const uint8_t *ptr, uint32_t len)
{
  if (ptr && len) {
    GWEN_MSG *msg;

    msg=GWEN_Msg_new(len);
    memmove(msg->buffer, ptr, len);
    msg->bytesInBuffer=len;
    return msg;
  }

  return NULL;
}



void GWEN_Msg_Attach(GWEN_MSG *msg)
{
  if (msg && msg->refCount>0)
    msg->refCount++;
}



void GWEN_Msg_free(GWEN_MSG *msg)
{
  if (msg && msg->refCount>0) {
    if (msg->refCount==1) {
      GWEN_LIST_FINI(GWEN_MSG, msg);
      GWEN_INHERIT_FINI(GWEN_MSG, msg);
      free(msg->buffer);
      GWEN_DB_Group_free(msg->dbParsedInfo);
      GWEN_FREE_OBJECT(msg);
    }
    else
      msg->refCount--;
  }
}



GWEN_MSG *GWEN_Msg_dup(const GWEN_MSG *srcMsg)
{
  if (srcMsg && srcMsg->refCount>0) {
    GWEN_MSG *msg;

    msg=GWEN_Msg_new(srcMsg->maxSize);
    if (srcMsg->maxSize)
      memmove(msg->buffer, srcMsg->buffer, msg->maxSize);
    msg->bytesInBuffer=srcMsg->bytesInBuffer;
    msg->currentPos=srcMsg->currentPos;
    msg->groupId=srcMsg->groupId;
    msg->parsedPayloadSize=srcMsg->parsedPayloadSize;
    msg->parsedPayloadOffset=srcMsg->parsedPayloadOffset;
    msg->flags=srcMsg->flags;
    if (srcMsg->dbParsedInfo)
      msg->dbParsedInfo=GWEN_DB_Group_dup(srcMsg->dbParsedInfo);

    return msg;
  }
  return NULL;
}



int GWEN_Msg_GetGroupId(const GWEN_MSG *msg)
{
  return msg->groupId;
}



void GWEN_Msg_SetGroupId(GWEN_MSG *msg, int groupId)
{
  msg->groupId=groupId;
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



uint32_t GWEN_Msg_GetBytesInBuffer(const GWEN_MSG *msg)
{
  if (msg)
    return msg->bytesInBuffer;
  else
    return 0;
}



void GWEN_Msg_SetBytesInBuffer(GWEN_MSG *msg, uint32_t i)
{
  if (msg && i<=msg->maxSize)
    msg->bytesInBuffer=i;
}



uint32_t GWEN_Msg_GetMaxSize(const GWEN_MSG *msg)
{
  return msg?(msg->maxSize):0;
}



uint32_t GWEN_Msg_GetCurrentPos(const GWEN_MSG *msg)
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



int GWEN_Msg_AddBytes(GWEN_MSG *msg, const uint8_t *bufferPtr, uint32_t bufferLen)
{
  if (msg) {
    if (((msg->bytesInBuffer+bufferLen)<=msg->maxSize) &&
        ((msg->currentPos)+bufferLen<=msg->maxSize)) {
      memmove(msg->buffer+msg->currentPos, bufferPtr, bufferLen);
      msg->currentPos+=bufferLen;
      msg->bytesInBuffer+=bufferLen;
      return 0;
    }
    else {
      DBG_INFO(GWEN_LOGDOMAIN,
               "Buffer too small (adding %d bytes at %d, space left=%d bytes)",
               bufferLen, msg->currentPos, msg->maxSize-msg->currentPos);
      return GWEN_ERROR_BUFFER_OVERFLOW;
    }
  }
  return GWEN_ERROR_GENERIC;
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



int GWEN_Msg_IncCurrentPos(GWEN_MSG *msg, uint32_t i)
{
  if (msg) {
    if (((msg->currentPos+i)<=msg->maxSize) &&
        ((msg->currentPos+i)<=msg->bytesInBuffer)) {
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



uint32_t GWEN_Msg_GetParsedPayloadSize(const GWEN_MSG *msg)
{
  if (msg)
    return msg->parsedPayloadSize;
  return 0;
}



void GWEN_Msg_SetParsedPayloadSize(GWEN_MSG *msg, uint32_t v)
{
  if (msg)
    msg->parsedPayloadSize=v;
}



uint32_t GWEN_Msg_GetParsedPayloadOffset(const GWEN_MSG *msg)
{
  if (msg)
    return msg->parsedPayloadOffset;
  return 0;
}



void GWEN_Msg_SetParsedPayloadOffset(GWEN_MSG *msg, uint32_t v)
{
  if (msg)
    msg->parsedPayloadOffset=v;
}



uint32_t GWEN_Msg_GetFlags(const GWEN_MSG *msg)
{
  if (msg)
    return msg->flags;
  return 0;
}



void GWEN_Msg_SetFlags(GWEN_MSG *msg, uint32_t f)
{
  if (msg)
    msg->flags=f;
}



void GWEN_Msg_AddFlags(GWEN_MSG *msg, uint32_t f)
{
  if (msg)
    msg->flags|=f;
}



void GWEN_Msg_DelFlags(GWEN_MSG *msg, uint32_t f)
{
  if (msg)
    msg->flags&=~f;
}



GWEN_DB_NODE *GWEN_Msg_GetDbParsedInfo(const GWEN_MSG *msg)
{
  if (msg)
    return msg->dbParsedInfo;
  return NULL;
}



void GWEN_Msg_SetDbParsedInfo(GWEN_MSG *msg, GWEN_DB_NODE *db)
{
  if (msg) {
    if (msg->dbParsedInfo)
      GWEN_DB_Group_free(msg->dbParsedInfo);
    msg->dbParsedInfo=db;
  }
}



uint64_t GWEN_Msg_GetUint64At(const GWEN_MSG *msg, int offs, uint64_t defaultValue)
{
  if (msg) {
    if (msg->bytesInBuffer>=offs+8) {
      const uint8_t *ptr;
      uint64_t v;

      ptr=msg->buffer+offs;
      v=(uint64_t)(*(ptr++));
      v|=(uint64_t)(*(ptr++))<<8;
      v|=(uint64_t)(*(ptr++))<<16;
      v|=(uint64_t)(*(ptr++))<<24;
      v|=(uint64_t)(*(ptr++))<<32;
      v|=(uint64_t)(*(ptr++))<<40;
      v|=(uint64_t)(*(ptr++))<<48;
      v|=(uint64_t)(*(ptr++))<<56;
      return v;
    }
  }
  return defaultValue;
}



uint32_t GWEN_Msg_GetUint32At(const GWEN_MSG *msg, int offs, uint32_t defaultValue)
{
  if (msg) {
    if (msg->bytesInBuffer>=offs+4) {
      const uint8_t *ptr;

      ptr=msg->buffer+offs;
      return (uint32_t)(ptr[0])+(ptr[1]<<8)+(ptr[2]<<16)+(ptr[3]<<24);
    }
  }
  return defaultValue;
}



uint16_t GWEN_Msg_GetUint16At(const GWEN_MSG *msg, int offs, uint16_t defaultValue)
{
  if (msg) {
    if (msg->bytesInBuffer>=offs+2) {
      const uint8_t *ptr;

      ptr=msg->buffer+offs;
      return (uint16_t)(ptr[0])+(ptr[1]<<8);
    }
  }
  return defaultValue;
}



uint8_t GWEN_Msg_GetUint8At(const GWEN_MSG *msg, int offs, uint8_t defaultValue)
{
  if (msg) {
    if (msg->bytesInBuffer>=offs+1) {
      const uint8_t *ptr;

      ptr=msg->buffer+offs;
      return ptr[0];
    }
  }
  return defaultValue;
}



void GWEN_Msg_Dump(const GWEN_MSG *msg, GWEN_BUFFER *buf)
{
  GWEN_Buffer_AppendArgs(buf, "Msg: bytesInBuffer=%d, maxSize=%d, currentPos=%d",
                         msg->bytesInBuffer, msg->maxSize, msg->currentPos);
  if (msg->buffer && msg->bytesInBuffer) {
    GWEN_Buffer_AppendByte(buf, '\n');
    GWEN_Text_DumpString2Buffer((const char*)(msg->buffer), msg->bytesInBuffer, buf, 2);
  }
}




