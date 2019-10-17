/***************************************************************************
    begin       : Sun Jun 13 2004
    copyright   : (C) 2004 by Martin Preuss
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

#include <stdlib.h>
#include <assert.h>
#include <string.h>


GWEN_LIST_FUNCTIONS(GWEN_TAG16, GWEN_Tag16)


GWEN_TAG16 *GWEN_Tag16_new(void)
{
  GWEN_TAG16 *tlv;

  GWEN_NEW_OBJECT(GWEN_TAG16, tlv);
  GWEN_LIST_INIT(GWEN_TAG16, tlv);

  return tlv;
}



void GWEN_Tag16_free(GWEN_TAG16 *tlv)
{
  if (tlv) {
    if (tlv->dataOwned)
      free(tlv->tagData);
    GWEN_LIST_FINI(GWEN_TAG16, tlv);
    GWEN_FREE_OBJECT(tlv);
  }
}



unsigned int GWEN_Tag16_GetTagType(const GWEN_TAG16 *tlv)
{
  assert(tlv);
  return tlv->tagType;
}



unsigned int GWEN_Tag16_GetTagLength(const GWEN_TAG16 *tlv)
{
  assert(tlv);
  return tlv->tagLength;
}



unsigned int GWEN_Tag16_GetTagSize(const GWEN_TAG16 *tlv)
{
  assert(tlv);
  return tlv->tagSize;
}



const void *GWEN_Tag16_GetTagData(const GWEN_TAG16 *tlv)
{
  assert(tlv);
  return tlv->tagData;
}



GWEN_TAG16 *GWEN_Tag16_fromBuffer(GWEN_BUFFER *mbuf, GWEN_UNUSED int isBerTlv)
{
  const char *p;
  unsigned int tagType;
  unsigned int tagLength;
  const char *tagData;
  unsigned int size;
  unsigned int pos;
  unsigned int j;
  GWEN_TAG16 *tlv;
  uint32_t startPos;

  if (!GWEN_Buffer_GetBytesLeft(mbuf)) {
    DBG_ERROR(0, "Buffer empty");
    return 0;
  }

  startPos=GWEN_Buffer_GetPos(mbuf);

  tagType=tagLength=0;

  p=GWEN_Buffer_GetPosPointer(mbuf);
  pos=0;
  size=GWEN_Buffer_GetBytesLeft(mbuf);

  /* get tag type */
  if (size<2) {
    DBG_ERROR(0, "Too few bytes for BER-TLV");
    return 0;
  }
  j=(unsigned char)(p[pos]);
  tagType=j;

  /* get length */
  pos++;
  if (pos+1>=size) {
    DBG_ERROR(0, "Too few bytes");
    return 0;
  }
  j=((unsigned char)(p[pos+1]))<<8;
  j|=(unsigned char)(p[pos]);
  pos+=2;
  tagLength=j;
  tagData=p+pos;
  GWEN_Buffer_IncrementPos(mbuf, pos);

  tlv=GWEN_Tag16_new();
  assert(tlv);
  tlv->tagType=tagType;
  tlv->tagLength=tagLength;
  if (tagLength) {
    tlv->tagData=(void *)malloc(tagLength);
    memmove(tlv->tagData, tagData, tagLength);
    tlv->dataOwned=1;
  }

  GWEN_Buffer_IncrementPos(mbuf, tagLength);
  tlv->tagSize=GWEN_Buffer_GetPos(mbuf)-startPos;
  return tlv;
}



GWEN_TAG16 *GWEN_Tag16_fromBuffer2(const uint8_t *p, uint32_t l, int doCopy)
{
  unsigned int tagType;
  unsigned int tagLength;
  const uint8_t *tagData;
  unsigned int size;
  unsigned int pos;
  unsigned int j;
  GWEN_TAG16 *tlv;

  if (l<1) {
    DBG_ERROR(0, "Buffer empty");
    return NULL;
  }

  tagType=tagLength=0;

  pos=0;
  size=l;

  /* get tag type */
  if (size<2) {
    DBG_ERROR(0, "Too few bytes for TLV");
    return 0;
  }
  j=(unsigned char)(p[pos]);
  tagType=j;

  /* get length */
  pos++;
  if (pos+1>=size) {
    DBG_ERROR(0, "Too few bytes");
    return 0;
  }
  j=((unsigned char)(p[pos+1]))<<8;
  j|=(unsigned char)(p[pos]);
  pos+=2;
  tagLength=j;
  tagData=p+pos;

  tlv=GWEN_Tag16_new();
  assert(tlv);
  tlv->tagType=tagType;
  tlv->tagLength=tagLength;
  if (tagLength) {
    if (doCopy) {
      tlv->tagData=(void *)malloc(tagLength);
      memmove(tlv->tagData, tagData, tagLength);
      tlv->dataOwned=1;
    }
    else {
      tlv->tagData=(uint8_t *)tagData;
      tlv->dataOwned=0;
    }
  }

  tlv->tagSize=tagLength+3;
  return tlv;
}



void GWEN_Tag16_DirectlyToBuffer(unsigned int tagType,
                                 const char *p,
                                 int size,
                                 GWEN_BUFFER *buf)
{
  assert(buf);
  if (size==-1) {
    assert(p);
    size=strlen(p);
  }

  GWEN_Buffer_AppendByte(buf, tagType & 0xff);
  GWEN_Buffer_AppendByte(buf, size & 0xff);
  GWEN_Buffer_AppendByte(buf, (size>>8)&0xff);
  if (size) {
    assert(p);
    GWEN_Buffer_AppendBytes(buf, p, size);
  }

}








