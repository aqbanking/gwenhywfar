/***************************************************************************
    begin       : Sun Jun 13 2004
    copyright   : (C) 2024 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define DISABLE_DEBUGLOG


#include "tag16_p.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/portable_endian.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>



GWEN_LIST_FUNCTIONS(GWEN_TAG16, GWEN_Tag16)



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static void _writeTagToBuffer(unsigned int tagType, const uint8_t *p, int size, GWEN_BUFFER *buf);



/* ------------------------------------------------------------------------------------------------
 * code
 * ------------------------------------------------------------------------------------------------
 */

GWEN_TAG16 *GWEN_Tag16_new(void)
{
  GWEN_TAG16 *tag;

  GWEN_NEW_OBJECT(GWEN_TAG16, tag);
  GWEN_LIST_INIT(GWEN_TAG16, tag);

  return tag;
}



GWEN_TAG16 *GWEN_Tag16_newNoCopy(unsigned int tagType, unsigned int tagLength, const uint8_t *tagData)
{
  GWEN_TAG16 *tag;

  tag=GWEN_Tag16_new();
  tag->tagType=tagType;
  tag->tagLength=tagLength;
  if (tagLength) {
    tag->tagData=tagData;
    tag->dataOwned=0;
  }

  tag->tagSize=tagLength+3;
  return tag;
}



GWEN_TAG16 *GWEN_Tag16_newCopy(unsigned int tagType, unsigned int tagLength, const uint8_t *tagData)
{
  GWEN_TAG16 *tag;

  tag=GWEN_Tag16_new();
  tag->tagType=tagType;
  tag->tagLength=tagLength;
  if (tagLength) {
    uint8_t *p;

    p=malloc(tagLength);
    memmove(p, tagData, tagLength);
    tag->tagData=(const uint8_t*)p;
    tag->dataOwned=1;
  }

  tag->tagSize=tagLength+3;
  return tag;
}






void GWEN_Tag16_free(GWEN_TAG16 *tag)
{
  if (tag) {
    if (tag->dataOwned)
      free((uint8_t*)(tag->tagData));
    GWEN_LIST_FINI(GWEN_TAG16, tag);
    GWEN_FREE_OBJECT(tag);
  }
}



unsigned int GWEN_Tag16_GetTagType(const GWEN_TAG16 *tag)
{
  return tag?(tag->tagType):0;
}



unsigned int GWEN_Tag16_GetTagLength(const GWEN_TAG16 *tag)
{
  return tag?(tag->tagLength):0;
}



unsigned int GWEN_Tag16_GetTagSize(const GWEN_TAG16 *tag)
{
  return tag?(tag->tagSize):0;
}



const void *GWEN_Tag16_GetTagData(const GWEN_TAG16 *tag)
{
  return tag?(tag->tagData):NULL;
}



GWEN_TAG16 *GWEN_Tag16_fromBuffer(GWEN_BUFFER *mbuf, GWEN_UNUSED int isBerTlv)
{

  GWEN_TAG16 *tag;

  tag=GWEN_Tag16_fromBuffer2((const uint8_t*) GWEN_Buffer_GetPosPointer(mbuf), GWEN_Buffer_GetUsedBytes(mbuf), 1);
  if (tag)
    GWEN_Buffer_IncrementPos(mbuf, tag->tagSize);
  return tag;
}



GWEN_TAG16 *GWEN_Tag16_fromBuffer2(const uint8_t *bufferPtr, uint32_t bufferLen, int doCopy)
{
  unsigned int tagType;
  unsigned int tagLength;

  if (bufferLen<3) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Buffer too small to contain a TAG16 object (%d < 3)", bufferLen);
    return NULL;
  }

  tagType=*(bufferPtr++);
  bufferLen--;
  tagLength=(uint16_t)(bufferPtr[0])+(bufferPtr[1]<<8);
  bufferPtr+=2;
  bufferLen-=2;
  if (bufferLen<tagLength) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Buffer too small to contain complete TAG16 object with data (%d < %d)", bufferLen, tagLength);
    return NULL;
  }

  return (doCopy?GWEN_Tag16_newCopy(tagType, tagLength, bufferPtr):GWEN_Tag16_newNoCopy(tagType, tagLength, bufferPtr));
}



void GWEN_Tag16_WriteStringTagToBuffer(unsigned int tagType, const char *s, GWEN_BUFFER *buf)
{
  _writeTagToBuffer(tagType, (const uint8_t*) s, s?(strlen(s)+1):0, buf);
}



void GWEN_Tag16_WriteUint8TagToBuffer(unsigned int tagType, uint8_t data, GWEN_BUFFER *buf)
{
  _writeTagToBuffer(tagType, (const uint8_t*) &data, 1, buf);
}



void GWEN_Tag16_WriteUint16TagToBuffer(unsigned int tagType, uint16_t data, GWEN_BUFFER *buf)
{
  uint16_t dataInLittleEndian;

  dataInLittleEndian=htole16(data);
  _writeTagToBuffer(tagType, (const uint8_t*) &dataInLittleEndian, sizeof(uint16_t), buf);
}



void GWEN_Tag16_WriteUint32TagToBuffer(unsigned int tagType, uint32_t data, GWEN_BUFFER *buf)
{
  uint32_t dataInLittleEndian;

  dataInLittleEndian=htole32(data);
  _writeTagToBuffer(tagType, (const uint8_t*) &dataInLittleEndian, sizeof(uint32_t), buf);
}



void GWEN_Tag16_WriteUint64TagToBuffer(unsigned int tagType, uint64_t data, GWEN_BUFFER *buf)
{
  uint64_t dataInLittleEndian;

  dataInLittleEndian=htole64(data);
  _writeTagToBuffer(tagType, (const uint8_t*) &dataInLittleEndian, sizeof(uint64_t), buf);
}



void GWEN_Tag16_WriteDoubleTagToBuffer(unsigned int tagType, double data, GWEN_BUFFER *buf)
{
  uint64_t dataInLittleEndian;
  union {
    double d;
    uint64_t u;
  } v;

  v.d=data;
  dataInLittleEndian=htole64(v.u);
  _writeTagToBuffer(tagType, (const uint8_t*) &dataInLittleEndian, sizeof(uint64_t), buf);
}



void GWEN_Tag16_DirectlyToBuffer(unsigned int tagType,
                                 const char *p,
                                 int size,
                                 GWEN_BUFFER *buf)
{
  _writeTagToBuffer(tagType, (const uint8_t*) p, (size==-1)?strlen(p):size, buf);
}



uint8_t GWEN_Tag16_GetTagDataAsUint8(const GWEN_TAG16 *tag, uint8_t defaultValue)
{
  if (tag && tag->tagLength>=sizeof(uint8_t))
    return *(uint8_t*)(tag->tagData);
  return defaultValue;
}



uint16_t GWEN_Tag16_GetTagDataAsUint16(const GWEN_TAG16 *tag, uint16_t defaultValue)
{
  if (tag && tag->tagLength>=sizeof(uint16_t))
    return le16toh(*(uint16_t*)(tag->tagData));
  return defaultValue;
}



uint32_t GWEN_Tag16_GetTagDataAsUint32(const GWEN_TAG16 *tag, uint32_t defaultValue)
{
  if (tag && tag->tagLength>=sizeof(uint32_t))
    return le32toh(*(uint32_t*)(tag->tagData));
  return defaultValue;
}



uint64_t GWEN_Tag16_GetTagDataAsUint64(const GWEN_TAG16 *tag, uint64_t defaultValue)
{
  if (tag && tag->tagLength>=sizeof(uint64_t))
    return le64toh(*(uint64_t*)(tag->tagData));
  return defaultValue;
}



double GWEN_Tag16_GetTagDataAsDouble(const GWEN_TAG16 *tag, double defaultValue)
{
  if (tag && tag->tagLength>=sizeof(uint64_t)) {
    union {
      double d;
      uint64_t u;
    } v;

    v.u=le64toh(*(uint64_t*)(tag->tagData));
    return v.d;
  }
  return defaultValue;
}



char *GWEN_Tag16_GetTagDataAsNewString(const GWEN_TAG16 *tag, const char *defaultValue)
{
  if (tag && tag->tagLength)
    return GWEN_Text_strndup((const char*)(tag->tagData), tag->tagLength);
  return defaultValue?strdup(defaultValue):NULL;
}



const GWEN_TAG16 *GWEN_Tag16_List_FindFirstByTagType(const GWEN_TAG16_LIST *tagList, unsigned int tagType)
{
  const GWEN_TAG16 *tag;

  tag=GWEN_Tag16_List_First(tagList);
  while(tag) {
    if (tag->tagType==tagType)
      return tag;
    tag=GWEN_Tag16_List_Next(tag);
  }

  return NULL;
}



const GWEN_TAG16 *GWEN_Tag16_List_FindNextByTagType(const GWEN_TAG16 *tag, unsigned int tagType)
{
  if (tag) {
    tag=GWEN_Tag16_List_Next(tag);
    while(tag) {
      if (tag->tagType==tagType)
	return tag;
      tag=GWEN_Tag16_List_Next(tag);
    }
  }

  return NULL;
}



GWEN_TAG16_LIST *GWEN_Tag16_List_fromBuffer(const uint8_t *p, uint32_t l, int doCopy)
{
  GWEN_TAG16_LIST *tagList;

  tagList=GWEN_Tag16_List_new();
  while(l) {
    GWEN_TAG16 *tag;

    tag=GWEN_Tag16_fromBuffer2(p, l, doCopy);
    if (tag==NULL) {
      DBG_INFO(GWEN_LOGDOMAIN, "here");
      GWEN_Tag16_List_free(tagList);
      return NULL;
    }
    GWEN_Tag16_List_Add(tag, tagList);
    if (l<tag->tagSize) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Invalid remaining data (%d < %d)", l, tag->tagSize);
      GWEN_Tag16_List_free(tagList);
      return NULL;
    }
    p+=tag->tagSize;
    l-=tag->tagSize;
  }

  if (GWEN_Tag16_List_GetCount(tagList)<1) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No entries in tag list");
    GWEN_Tag16_List_free(tagList);
    return NULL;
  }
  return tagList;
}



void GWEN_Tag16_WriteTagToBuffer(unsigned int tagType, const uint8_t *s, int size, GWEN_BUFFER *buf)
{
  _writeTagToBuffer(tagType, s, size, buf);
}



void _writeTagToBuffer(unsigned int tagType, const uint8_t *p, int size, GWEN_BUFFER *buf)
{
  if (GWEN_Buffer_AllocRoom(buf, size+3)==0) {
    uint8_t *posPtr;

    posPtr=(uint8_t*) GWEN_Buffer_GetPosPointer(buf);
    *(posPtr++)=tagType & 0xff;
    *(posPtr++)=size & 0xff;
    *(posPtr++)=(size>>8) & 0xff;
    if (size)
      memmove(posPtr, p, size);
    GWEN_Buffer_IncrementPos(buf, size+3);
    GWEN_Buffer_AdjustUsedBytes(buf);
  }
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
  }
}



int GWEN_Tag16_StartTagInBuffer(unsigned int tagType, GWEN_BUFFER *buf)
{
  if (buf) {
    int pos;
    int rv;

    pos=GWEN_Buffer_GetPos(buf);
    rv=GWEN_Buffer_AllocRoom(buf, 3);
    if (rv==0) {
      uint8_t *posPtr;

      posPtr=(uint8_t*) GWEN_Buffer_GetPosPointer(buf);
      *(posPtr++)=tagType & 0xff;
      *(posPtr++)=0;
      *(posPtr++)=0;
      GWEN_Buffer_IncrementPos(buf, 3);
      GWEN_Buffer_AdjustUsedBytes(buf);
    }
    else {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    return pos;
  }
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "NULLPOINTER");
    return GWEN_ERROR_INVALID;
  }
}



int GWEN_Tag16_EndTagInBuffer(int startPos, GWEN_BUFFER *buf)
{
  int currentPos;
  int payloadSize;

  currentPos=GWEN_Buffer_GetPos(buf);
  payloadSize=currentPos-startPos-3;
  if (payloadSize<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Bad size(%d) or startpos(%d)", payloadSize, startPos);
    return GWEN_ERROR_GENERIC;
  }
  else {
    uint8_t *posPtr;

    posPtr=(uint8_t*) GWEN_Buffer_GetStart(buf)+startPos+1;
    *(posPtr++)=payloadSize & 0xff;
    *(posPtr++)=(payloadSize>>8) & 0xff;
    return 0;
  }
}






#include "tag16-t.c"


