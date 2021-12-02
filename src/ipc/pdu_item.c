/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2021 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "pdu_item_p.h"

#include <gwenhywfar/memory.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>

#include <stdlib.h>


GWEN_TREE_FUNCTIONS(GWEN_PDU_ITEM, GWEN_PduItem)




GWEN_PDU_ITEM *GWEN_PduItem_new(void)
{
  GWEN_PDU_ITEM *item;

  GWEN_NEW_OBJECT(GWEN_PDU_ITEM, item);
  item->refCount=1;
  GWEN_TREE_INIT(GWEN_PDU_ITEM, item);

  return item;
}



void GWEN_PduItem_free(GWEN_PDU_ITEM *item)
{
  if (item) {
    assert(item->refCount);
    if (item->refCount==1) {
      GWEN_TREE_FINI(GWEN_PDU_ITEM, item);
      if (item->ownData) {
        if (item->dataPointer && item->length)
          free((void*) (item->dataPointer));
        item->dataPointer=NULL;
        item->length=0;
      }
      item->refCount=0;
      GWEN_FREE_OBJECT(item);

    }
    else {
      item->refCount--;
    }
  }
}



int8_t GWEN_PduItem_GetType(const GWEN_PDU_ITEM *item)
{
  assert(item);
  assert(item->refCount);

  return item->type;
}



void GWEN_PduItem_SetType(GWEN_PDU_ITEM *item, int8_t i)
{
  assert(item);
  assert(item->refCount);

  item->type=i;
}



uint32_t GWEN_PduItem_GetLength(const GWEN_PDU_ITEM *item)
{
  assert(item);
  assert(item->refCount);

  return item->length;
}



const uint8_t *GWEN_PduItem_GetDataPointer(const GWEN_PDU_ITEM *item)
{
  assert(item);
  assert(item->refCount);

  return item->dataPointer;
}




void GWEN_PduItem_SetData(GWEN_PDU_ITEM *item, const uint8_t *ptr, uint32_t len, int takeOver)
{
  assert(item);
  assert(item->refCount);

  if (item->ownData) {
    if (item->dataPointer && item->length)
      free((void*) (item->dataPointer));
  }
  item->dataPointer=ptr;
  item->length=len;
  item->ownData=(takeOver!=0)?1:0;
}



int GWEN_PduItem_ReadBuffer32(GWEN_PDU_ITEM *item, const uint8_t *bufferPointer, uint32_t bufferLength)
{
  assert(item);
  assert(item->refCount);

  if (bufferPointer && (bufferLength>5)) {
    uint32_t itemLength;

    item->type=*bufferPointer;
    itemLength=(((uint32_t)bufferPointer[2])<<24)+
      (((uint32_t)bufferPointer[3])<<16)+
      (((uint32_t)bufferPointer[4])<<8)+
      bufferPointer[5];
    if ((itemLength+6)>bufferLength) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Too few bytes for payload (%d > %d)", (int)(itemLength+6), (int) bufferLength);
      return GWEN_ERROR_PARTIAL;
    }
    GWEN_PduItem_SetData(item, bufferPointer+6, itemLength, 0); /* dont take over */
    return itemLength+6;
  }
  else
    return GWEN_ERROR_NO_DATA;
}



int GWEN_PduItem_ReadHeader32(const uint8_t *bufferPointer, uint32_t bufferLength)
{
  if (bufferPointer && (bufferLength>5)) {
    uint32_t itemLength;

    itemLength=(((uint32_t)bufferPointer[2])<<24)+
      (((uint32_t)bufferPointer[3])<<16)+
      (((uint32_t)bufferPointer[4])<<8)+
      bufferPointer[5];
    return itemLength+6;
  }
  else
    return GWEN_ERROR_NO_DATA;
}



int GWEN_PduItem_ReadBuffer16(GWEN_PDU_ITEM *item, const uint8_t *bufferPointer, uint32_t bufferLength)
{
  assert(item);
  assert(item->refCount);

  if (bufferPointer && (bufferLength>3)) {
    uint32_t itemLength;

    item->type=*bufferPointer;
    itemLength=(((uint32_t)bufferPointer[2])<<8)+bufferPointer[3];
    if ((itemLength+4)>bufferLength) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Too few bytes for payload (%d > %d)", (int)(itemLength+4), (int) bufferLength);
      return GWEN_ERROR_PARTIAL;
    }
    GWEN_PduItem_SetData(item, bufferPointer+4, itemLength, 0); /* dont take over */
    return itemLength+4;
  }
  else
    return GWEN_ERROR_NO_DATA;
}



int GWEN_PduItem_ReadHeader16(const uint8_t *bufferPointer, uint32_t bufferLength)
{
  if (bufferPointer && (bufferLength>3)) {
    uint32_t itemLength;

    itemLength=(((uint32_t)bufferPointer[2])<<8)+bufferPointer[3];
    return itemLength+4;
  }
  else
    return GWEN_ERROR_NO_DATA;
}



int GWEN_PduItem_WriteBuffer32(const GWEN_PDU_ITEM *item, GWEN_BUFFER *destBuffer)
{
  uint8_t itemLength[4];

  assert(item);
  assert(item->refCount);

  GWEN_Buffer_AppendByte(destBuffer, item->type);
  GWEN_Buffer_AppendByte(destBuffer, 0);

  itemLength[0]=((item->length)>>24) & 0xff;
  itemLength[1]=((item->length)>>16) & 0xff;
  itemLength[2]=((item->length)>>8) & 0xff;
  itemLength[3]=(item->length) & 0xff;
  GWEN_Buffer_AppendBytes(destBuffer, (const char*) &itemLength, 4);

  if (item->dataPointer && item->length)
    GWEN_Buffer_AppendBytes(destBuffer, (const char*) (item->dataPointer), item->length);

  return item->length+6;
}



int GWEN_PduItem_WriteBuffer16(const GWEN_PDU_ITEM *item, GWEN_BUFFER *destBuffer)
{
  return GWEN_PduItem_DirectlyWritePdu16(item->type, item->dataPointer, item->length, destBuffer);
}



int GWEN_PduItem_DirectlyWritePdu16(int8_t t, const uint8_t *dataPointer, uint32_t length, GWEN_BUFFER *destBuffer)
{
  uint8_t itemLength[2];

  GWEN_Buffer_AppendByte(destBuffer, t);
  GWEN_Buffer_AppendByte(destBuffer, 0);

  itemLength[0]=(length>>8) & 0xff;
  itemLength[1]=length & 0xff;
  GWEN_Buffer_AppendBytes(destBuffer, (const char*) &itemLength, 2);

  if (dataPointer && length)
    GWEN_Buffer_AppendBytes(destBuffer, (const char*) dataPointer, length);

  return length+4;
}



int GWEN_PduItem_DirectlyWritePdu16AsUint32(int8_t t, uint32_t i, GWEN_BUFFER *destBuffer)
{
  uint8_t value[4];

  value[0]=(i>>24) & 0xff;
  value[1]=(i>>16) & 0xff;
  value[2]=(i>>8) & 0xff;
  value[3]=i & 0xff;
  return GWEN_PduItem_DirectlyWritePdu16(t, value, 4, destBuffer);
}



void GWEN_PduItem_AdjustItemSizeInBuffer32(GWEN_BUFFER *destBuffer, uint32_t startPos)
{
  uint8_t *ptr;
  uint32_t itemLength;

  ptr=((uint8_t *)GWEN_Buffer_GetStart(destBuffer))+startPos;
  itemLength=(GWEN_Buffer_GetPos(destBuffer)-startPos)-6;

  ptr[2]=((itemLength)>>24) & 0xff;
  ptr[3]=((itemLength)>>16) & 0xff;
  ptr[4]=((itemLength)>>8) & 0xff;
  ptr[5]=(itemLength) & 0xff;
}



void GWEN_PduItem_AdjustItemSizeInBuffer16(GWEN_BUFFER *destBuffer, uint32_t startPos)
{
  uint8_t *ptr;
  uint32_t itemLength;

  ptr=((uint8_t *)GWEN_Buffer_GetStart(destBuffer))+startPos;
  itemLength=(GWEN_Buffer_GetPos(destBuffer)-startPos)-4;

  ptr[2]=((itemLength)>>8) & 0xff;
  ptr[3]=(itemLength) & 0xff;
}



void GWEN_PduItem_AppendItemHeader32(GWEN_BUFFER *destBuffer, uint8_t t, uint32_t length)
{
  uint8_t itemLength[4];

  GWEN_Buffer_AppendByte(destBuffer, t);
  GWEN_Buffer_AppendByte(destBuffer, 0);

  itemLength[0]=(length>>24) & 0xff;
  itemLength[1]=(length>>16) & 0xff;
  itemLength[2]=(length>>8) & 0xff;
  itemLength[3]=(length) & 0xff;
  GWEN_Buffer_AppendBytes(destBuffer, (const char*) &itemLength, 4);
}



void GWEN_PduItem_AppendItemHeader16(GWEN_BUFFER *destBuffer, uint8_t t, uint32_t length)
{
  uint8_t itemLength[2];

  GWEN_Buffer_AppendByte(destBuffer, t);
  GWEN_Buffer_AppendByte(destBuffer, 0);

  itemLength[0]=(length>>8) & 0xff;
  itemLength[1]=(length) & 0xff;
  GWEN_Buffer_AppendBytes(destBuffer, (const char*) &itemLength, 2);
}



char *GWEN_PduItem_GetDataAsString(const GWEN_PDU_ITEM *item)
{
  assert(item);
  assert(item->refCount);

  if (item->dataPointer && item->length) {
    char *value;

    value=strndup((const char*) (item->dataPointer), item->length);
    assert(value);
    return value;
  }

  return NULL;
}



uint32_t GWEN_PduItem_GetDataAsUint32(const GWEN_PDU_ITEM *item)
{
  assert(item);
  assert(item->refCount);

  if (item->dataPointer && item->length>=4) {
    const uint8_t *p;

    p=item->dataPointer;
    return (((uint32_t)p[0])<<24)+(((uint32_t)p[1])<<16)+(((uint32_t)p[2])<<8)+p[3];
  }

  return 0;
}



void GWEN_PduItem_SetDataAsUint32(GWEN_PDU_ITEM *item, uint32_t i)
{
  assert(item);
  assert(item->refCount);

  if (item->dataPointer && item->length>=4) {
    uint8_t *pValue;

    pValue=(uint8_t*) malloc(4);
    assert(pValue);
    pValue[0]=(i>>24) & 0xff;
    pValue[1]=(i>>16) & 0xff;
    pValue[2]=(i>>8) & 0xff;
    pValue[3]=i & 0xff;
    GWEN_PduItem_SetData(item, pValue, 4, 1); /* take over */
  }
}



void GWEN_PduItem_SetDataAsString(GWEN_PDU_ITEM *item, const char *s)
{
  assert(item);
  assert(item->refCount);

  if (s) {
    int stringLen;
    char *value;

    stringLen=strlen(s);
    value=strdup(s);
    assert(value);
    GWEN_PduItem_SetData(item, (const uint8_t*) value, stringLen, 1); /* dont count trailing zero, take over data */
  }
  else {
    GWEN_PduItem_SetData(item, NULL, 0, 0); /* reset data */
  }
}



char *GWEN_PduItem_GetDataAsStringAt(const GWEN_PDU_ITEM *item, int pos, int maxLen)
{
  assert(item);
  assert(item->refCount);

  if (item->dataPointer && ((pos+maxLen)<=item->length)) {
    const char *ptr;
    const char *startPtr;
    int i;
    char *value;

    /* skip leading spaces */
    ptr=(const char*)(item->dataPointer+pos);
    i=maxLen;
    while(i>0 && *ptr==32) {
      i--;
      ptr++;
    }
    if (i==0)
      return NULL;
    startPtr=ptr;
    ptr+=(i-1);

    /* cut trailing spaces */
    while(i>0 && *ptr==32) {
      i--;
      ptr--;
    }
    if (i==0)
      return NULL;

    value=strndup((const char*) startPtr, i);
    assert(value);
    return value;
  }

  return NULL;
}



uint32_t GWEN_PduItem_GetDataAsUint32At(const GWEN_PDU_ITEM *item, int pos)
{
  assert(item);
  assert(item->refCount);

  if (item->dataPointer && (pos+4)>=item->length) {
    const uint8_t *p;

    p=item->dataPointer+pos;
    return (((uint32_t)p[0])<<24)+(((uint32_t)p[1])<<16)+(((uint32_t)p[2])<<8)+p[3];
  }

  return 0;
}



void GWEN_PduItem_Dump(const GWEN_PDU_ITEM *item, int indent)
{
  int i;

  for (i=0; i<indent; i++)
    fprintf(stdout, " ");
  fprintf(stdout, "PduItem\n");

  for (i=0; i<indent+2; i++)
    fprintf(stdout, " ");
  fprintf(stdout, "Type  : 0x%02x\n", item->type);

  for (i=0; i<indent+2; i++)
    fprintf(stdout, " ");
  fprintf(stdout, "Length: 0x%08x\n", item->length);

  if (item->length && item->dataPointer) {
    for (i=0; i<indent+2; i++)
      fprintf(stdout, " ");
    fprintf(stdout, "Data  :\n");
    GWEN_Text_DumpString((const char*)(item->dataPointer), item->length, indent+4);
  }
}



#include "pdu_item-t.c"



