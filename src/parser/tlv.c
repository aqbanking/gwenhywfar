/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sun Jun 13 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "tlv_p.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/text.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>


GWEN_LIST_FUNCTIONS(GWEN_TLV, GWEN_TLV)


GWEN_TLV *GWEN_TLV_new() {
  GWEN_TLV *tlv;

  GWEN_NEW_OBJECT(GWEN_TLV, tlv);
  GWEN_LIST_INIT(GWEN_TLV, tlv);

  return tlv;
}



void GWEN_TLV_free(GWEN_TLV *tlv) {
  if (tlv) {
    free(tlv->tagData);
    GWEN_LIST_FINI(GWEN_TLV, tlv);
    GWEN_FREE_OBJECT(tlv);
  }
}



int GWEN_TLV_IsBerTlv(const GWEN_TLV *tlv){
  assert(tlv);
  return tlv->isBerTlv;
}



unsigned int GWEN_TLV_GetTagType(const GWEN_TLV *tlv){
  assert(tlv);
  return tlv->tagType;
}



unsigned int GWEN_TLV_GetTagLength(const GWEN_TLV *tlv){
  assert(tlv);
  return tlv->tagLength;
}



unsigned int GWEN_TLV_GetTagSize(const GWEN_TLV *tlv){
  assert(tlv);
  return tlv->tagSize;
}



const void *GWEN_TLV_GetTagData(const GWEN_TLV *tlv){
  assert(tlv);
  return tlv->tagData;
}



GWEN_TLV *GWEN_TLV_fromBuffer(GWEN_BUFFER *mbuf, int isBerTlv) {
  const char *p;
  unsigned int tagMode;
  unsigned int tagType;
  unsigned int tagLength;
  const char *tagData;
  unsigned int size;
  unsigned int pos;
  unsigned int j;
  GWEN_TLV *tlv;
  GWEN_TYPE_UINT32 startPos;

  if (!GWEN_Buffer_GetBytesLeft(mbuf)) {
    DBG_ERROR(0, "Buffer empty");
    return 0;
  }

  startPos=GWEN_Buffer_GetPos(mbuf);

  tagMode=tagType=tagLength=0;

  p=GWEN_Buffer_GetPosPointer(mbuf);
  pos=0;
  size=GWEN_Buffer_GetBytesLeft(mbuf);

  /* get tag type */
  if (size<2) {
    DBG_ERROR(0, "Too few bytes for BER-TLV");
    return 0;
  }
  j=(unsigned char)(p[pos]);
  tagMode=(j & 0xe0);
  if (isBerTlv) {
    if ((j & 0x1f)==0x1f) {
      pos++;
      if (pos>=size) {
        DBG_ERROR(0, "Too few bytes");
        return 0;
      }
      j=(unsigned char)(p[pos]);
    }
    else
      j&=0x1f;
  }
  DBG_DEBUG(0, "Tag type %02x%s", j,
            isBerTlv?" (BER-TLV)":"");
  tagType=j;

  /* get length */
  pos++;
  if (pos>=size) {
    DBG_ERROR(0, "Too few bytes");
    return 0;
  }
  j=(unsigned char)(p[pos]);
  if (isBerTlv) {
    if (j & 0x80) {
      if (j==0x81) {
        pos++;
        if (pos>=size) {
          DBG_ERROR(0, "Too few bytes");
          return 0;
        }
        j=(unsigned char)(p[pos]);
      } /* 0x81 */
      else if (j==0x82) {
        if (pos+1>=size) {
          DBG_ERROR(0, "Too few bytes");
          return 0;
        }
        pos++;
        j=((unsigned char)(p[pos]))<<8;
        pos++;
        j+=(unsigned char)(p[pos]);
      } /* 0x82 */
      else {
        DBG_ERROR(0, "Unexpected tag length modifier %02x", j);
        return 0;
      }
    } /* if tag length modifier */
  }
  else {
    if (j==255) {
      if (pos+2>=size) {
        DBG_ERROR(0, "Too few bytes");
        return 0;
      }
      pos++;
      j=((unsigned char)(p[pos]))<<8;
      pos++;
      j+=(unsigned char)(p[pos]);
    }
  }
  pos++;
  tagLength=j;
  tagData=p+pos;
  GWEN_Buffer_IncrementPos(mbuf, pos);

  DBG_DEBUG(0, "Tag: %02x (%d bytes)", tagType, tagLength);
  if (pos+j>size) {
    DBG_ERROR(0, "Too few bytes");
    return 0;
  }

  tlv=GWEN_TLV_new();
  assert(tlv);
  tlv->isBerTlv=isBerTlv;
  tlv->tagMode=tagMode;
  tlv->tagType=tagType;
  tlv->tagLength=tagLength;
  if (tagLength) {
    tlv->tagData=(void*)malloc(tagLength);
    memmove(tlv->tagData, tagData, tagLength);
  }

  GWEN_Buffer_IncrementPos(mbuf, tagLength);
  tlv->tagSize=GWEN_Buffer_GetPos(mbuf)-startPos;
  return tlv;
}



int GWEN_TLV_IsContructed(const GWEN_TLV *tlv){
  assert(tlv);
  return (tlv->tagMode & 0x20);
}



unsigned int GWEN_TLV_GetClass(const GWEN_TLV *tlv){
  assert(tlv);
  return (tlv->tagMode & 0xc0);
}













