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



GWEN_TLV *GWEN_TLV_create(unsigned int tagType,
                          unsigned int tagMode,
                          const void *p,
                          unsigned int dlen,
                          int isBerTlv) {
  GWEN_TLV *tlv;

  /* some checks first */
  if (tagType>255) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Tag type too high");
    abort();
  }
  if (isBerTlv) {
    if (dlen>65535) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Data too long");
      abort();
    }
  }
  else {
    if (dlen>255) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Data too long");
      abort();
    }
  }

  /* limits ok, create TLV */
  tlv=GWEN_TLV_new();
  tlv->tagType=tagType;
  tlv->tagMode=tagMode;
  tlv->isBerTlv=isBerTlv;

  tlv->tagLength=dlen;
  if (dlen) {
    tlv->tagData=malloc(dlen);
    assert(tlv->tagData);
    memmove(tlv->tagData, p, dlen);
  }

  return tlv;
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
  uint32_t startPos;

  if (!GWEN_Buffer_GetBytesLeft(mbuf)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Buffer empty");
    return 0;
  }

  startPos=GWEN_Buffer_GetPos(mbuf);

  tagMode=tagType=tagLength=0;

  p=GWEN_Buffer_GetPosPointer(mbuf);
  pos=0;
  size=GWEN_Buffer_GetBytesLeft(mbuf);

  /* get tag type */
  if (size<2) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Too few bytes for BER-TLV");
    return 0;
  }
  j=(unsigned char)(p[pos]);
  tagMode=(j & 0xe0);
  if (isBerTlv) {
    if ((j & 0x1f)==0x1f) {
      pos++;
      if (pos>=size) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Too few bytes");
        return 0;
      }
      j=(unsigned char)(p[pos]);
    }
    else
      j&=0x1f;
  }
  DBG_DEBUG(GWEN_LOGDOMAIN, "Tag type %02x%s", j,
            isBerTlv?" (BER-TLV)":"");
  tagType=j;

  /* get length */
  pos++;
  if (pos>=size) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Too few bytes");
    return 0;
  }
  j=(unsigned char)(p[pos]);
  if (isBerTlv) {
    if (j & 0x80) {
      if (j==0x81) {
        pos++;
        if (pos>=size) {
          DBG_ERROR(GWEN_LOGDOMAIN, "Too few bytes");
          return 0;
        }
        j=(unsigned char)(p[pos]);
      } /* 0x81 */
      else if (j==0x82) {
        if (pos+1>=size) {
          DBG_ERROR(GWEN_LOGDOMAIN, "Too few bytes");
          return 0;
        }
        pos++;
        j=((unsigned char)(p[pos]))<<8;
        pos++;
        j+=(unsigned char)(p[pos]);
      } /* 0x82 */
      else {
	DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected tag length modifier %02x at %d", j, pos);
        return 0;
      }
    } /* if tag length modifier */
  }
  else {
    if (j==255) {
      if (pos+2>=size) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Too few bytes");
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

  DBG_DEBUG(GWEN_LOGDOMAIN, "Tag: %02x (%d bytes)", tagType, tagLength);
  if (pos+j>size) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Too few bytes");
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



int GWEN_TLV_toBuffer(GWEN_TLV *tlv, GWEN_BUFFER *mbuf) {
  assert(tlv);
  return GWEN_TLV_DirectlyToBuffer(tlv->tagType,
                                   tlv->tagMode,
                                   tlv->tagData,
                                   tlv->tagLength,
                                   tlv->isBerTlv,
                                   mbuf);
}



int GWEN_TLV_DirectlyToBuffer(unsigned int tagType,
                              unsigned int tagMode,
                              const void *tagData,
                              int tagLength,
                              int isBerTlv,
                              GWEN_BUFFER *mbuf) {
  if (tagLength==-1)
    tagLength=strlen(tagData);

  if (isBerTlv) {
    unsigned char j;

    /* write tag type */
    j=tagMode;
    if (tagType>=0x1f) {
      j|=0x1f;
      GWEN_Buffer_AppendByte(mbuf, j);
      GWEN_Buffer_AppendByte(mbuf, (unsigned char)tagType);
    }
    else {
      j|=tagType;
      GWEN_Buffer_AppendByte(mbuf, j);
    }

    /* write tag length */
    if (tagLength>255) {
      /* two byte size */
      GWEN_Buffer_AppendByte(mbuf, 0x82);
      GWEN_Buffer_AppendByte(mbuf, ((tagLength>>8) && 0xff));
      GWEN_Buffer_AppendByte(mbuf, (tagLength && 0xff));
    }
    else if (tagLength>127) {
      /* one byte size */
      GWEN_Buffer_AppendByte(mbuf, 0x81);
      GWEN_Buffer_AppendByte(mbuf, (tagLength && 0xff));
    }
    else {
      GWEN_Buffer_AppendByte(mbuf, (tagLength && 0x7f));
    }

    /* write tag data */
    if (tagLength)
      GWEN_Buffer_AppendBytes(mbuf, tagData, tagLength);
  }
  else {
    /* write tag type */
    GWEN_Buffer_AppendByte(mbuf, (unsigned char)tagType);

    /* write tag length */
    GWEN_Buffer_AppendByte(mbuf, (tagLength && 0xff));

    /* write tag data */
    if (tagLength)
      GWEN_Buffer_AppendBytes(mbuf, tagData, tagLength);
  }

  return 0;
}










