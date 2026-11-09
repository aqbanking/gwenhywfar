/***************************************************************************
    begin       : Sun Jun 13 2004
    copyright   : (C) 2004-2011 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define DISABLE_DEBUGLOG


#include "tlv_p.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/text.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define BER_TLV_TAG_FIRST_BYTE_BYTE_FOLLOWS 0b00011111
#define BER_TLV_TAG_SECOND_BYTE_BYTE_FOLLOWS 0b10000000
#define BER_TLV_TAG_IS_CONSTRUCTED 0b00100000

GWEN_LIST_FUNCTIONS(GWEN_TLV, GWEN_TLV)


GWEN_TLV *GWEN_TLV_new(void)
{
  GWEN_TLV *tlv;

  GWEN_NEW_OBJECT(GWEN_TLV, tlv);
  GWEN_LIST_INIT(GWEN_TLV, tlv);

  return tlv;
}



void GWEN_TLV_free(GWEN_TLV *tlv)
{
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
                          int isBerTlv)
{
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



int GWEN_TLV_IsBerTlv(const GWEN_TLV *tlv)
{
  assert(tlv);
  return tlv->isBerTlv;
}



unsigned int GWEN_TLV_GetTagType(const GWEN_TLV *tlv)
{
  assert(tlv);
  return tlv->tagType;
}



unsigned int GWEN_TLV_GetTagLength(const GWEN_TLV *tlv)
{
  assert(tlv);
  return tlv->tagLength;
}



unsigned int GWEN_TLV_GetTagSize(const GWEN_TLV *tlv)
{
  assert(tlv);
  return tlv->tagSize;
}



const void *GWEN_TLV_GetTagData(const GWEN_TLV *tlv)
{
  assert(tlv);
  return tlv->tagData;
}



GWEN_TLV *GWEN_TLV_fromBuffer(GWEN_BUFFER *mbuf, int isBerTlv)
{
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
    tlv->tagData=(void *)malloc(tagLength);
    memmove(tlv->tagData, tagData, tagLength);
  }

  GWEN_Buffer_IncrementPos(mbuf, tagLength);
  tlv->tagSize=GWEN_Buffer_GetPos(mbuf)-startPos;
  return tlv;
}



int GWEN_TLV_IsContructed(const GWEN_TLV *tlv)
{
  assert(tlv);
  return (tlv->tagMode & 0x20);
}



unsigned int GWEN_TLV_GetClass(const GWEN_TLV *tlv)
{
  assert(tlv);
  return (tlv->tagMode & 0xc0);
}



int GWEN_TLV_toBuffer(GWEN_TLV *tlv, GWEN_BUFFER *mbuf)
{
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
                              GWEN_BUFFER *mbuf)
{
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
      GWEN_Buffer_AppendByte(mbuf, ((tagLength>>8) & 0xff));
      GWEN_Buffer_AppendByte(mbuf, (tagLength & 0xff));
    }
    else if (tagLength>127) {
      /* one byte size */
      GWEN_Buffer_AppendByte(mbuf, 0x81);
      GWEN_Buffer_AppendByte(mbuf, (tagLength & 0xff));
    }
    else {
      GWEN_Buffer_AppendByte(mbuf, (tagLength & 0x7f));
    }

    /* write tag data */
    if (tagLength)
      GWEN_Buffer_AppendBytes(mbuf, tagData, tagLength);
  }
  else {
    /* write tag type */
    GWEN_Buffer_AppendByte(mbuf, (unsigned char)tagType);

    /* write tag length */
    GWEN_Buffer_AppendByte(mbuf, (tagLength & 0xff));

    /* write tag data */
    if (tagLength)
      GWEN_Buffer_AppendBytes(mbuf, tagData, tagLength);
  }

  return 0;
}



int GWEN_TLV_ReadHeader(GWEN_TLV *tlv, const uint8_t *p, uint32_t size, int isBerTlv)
{
  uint64_t tagMode;
  uint64_t tagType;
  uint64_t tagLength;
  unsigned int pos;
  uint64_t j;

  tagMode=tagType=tagLength=0;

  pos=0;

  /* get tag type */
  if (size<2) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Too few bytes for TLV");
    return GWEN_ERROR_BAD_DATA;
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
    return GWEN_ERROR_BAD_DATA;
  }
  j=(unsigned char)(p[pos]);
  if (isBerTlv) {
    if (j & 0x80) {
      if (j==0x81) {
        pos++;
        if (pos>=size) {
          DBG_ERROR(GWEN_LOGDOMAIN, "Too few bytes");
          return GWEN_ERROR_BAD_DATA;
        }
        j=(unsigned char)(p[pos]);
      } /* 0x81 */
      else if (j==0x82) {
        if (pos+1>=size) {
          DBG_ERROR(GWEN_LOGDOMAIN, "Too few bytes");
          return GWEN_ERROR_BAD_DATA;
        }
        pos++;
        j=((unsigned char)(p[pos]))<<8;
        pos++;
        j+=(unsigned char)(p[pos]);
      } /* 0x82 */
      else if (j==0x83) {
        if (pos+2>=size) {
          DBG_ERROR(GWEN_LOGDOMAIN, "Too few bytes");
          return GWEN_ERROR_BAD_DATA;
        }
        pos++;
        j=((unsigned char)(p[pos]))<<16;
        pos++;
        j+=((unsigned char)(p[pos]))<<8;
        pos++;
        j+=(unsigned char)(p[pos]);
      } /* 0x83 */
      else if (j==0x84) {
        if (pos+3>=size) {
          DBG_ERROR(GWEN_LOGDOMAIN, "Too few bytes");
          return GWEN_ERROR_BAD_DATA;
        }
        pos++;
        j=((unsigned char)(p[pos]))<<24;
        pos++;
        j+=((unsigned char)(p[pos]))<<16;
        pos++;
        j+=((unsigned char)(p[pos]))<<8;
        pos++;
        j+=(unsigned char)(p[pos]);
      } /* 0x84 */
      else if (j==0x85) {
        if (pos+4>=size) {
          DBG_ERROR(GWEN_LOGDOMAIN, "Too few bytes");
          return GWEN_ERROR_BAD_DATA;
        }
        pos++;
        j=((uint64_t)((unsigned char)(p[pos])))<<32;
        pos++;
        j+=((uint64_t)((unsigned char)(p[pos])))<<24;
        pos++;
        j+=((uint64_t)((unsigned char)(p[pos])))<<16;
        pos++;
        j+=((uint64_t)((unsigned char)(p[pos])))<<8;
        pos++;
        j+=(unsigned char)(p[pos]);
      } /* 0x85 */
      else {
        DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected tag length modifier %02x at %d", (int) j, pos);
        return GWEN_ERROR_BAD_DATA;
      }
    } /* if tag length modifier */
  }
  else {
    if (j==255) {
      if (pos+2>=size) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Too few bytes");
        return GWEN_ERROR_BAD_DATA;
      }
      pos++;
      j=((unsigned char)(p[pos]))<<8;
      pos++;
      j+=(unsigned char)(p[pos]);
    }
  }
  pos++;
  tagLength=j;

  DBG_DEBUG(GWEN_LOGDOMAIN, "Tag: %02x (%d bytes)", tagType, tagLength);

  tlv->isBerTlv=isBerTlv;
  tlv->tagMode=tagMode;
  tlv->tagType=tagType;
  tlv->tagLength=tagLength;

  tlv->tagSize=pos+tagLength;
  return (int) pos;
}



int GWEN_TLV_WriteHeader(unsigned int tagType,
                         unsigned int tagMode,
                         uint64_t tagLength,
                         int isBerTlv,
                         GWEN_BUFFER *mbuf)
{
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
    if (tagLength>0xffffffffL) {
      /* five byte size */
      GWEN_Buffer_AppendByte(mbuf, 0x85);
      GWEN_Buffer_AppendByte(mbuf, ((tagLength>>32) & 0xff));
      GWEN_Buffer_AppendByte(mbuf, ((tagLength>>24) & 0xff));
      GWEN_Buffer_AppendByte(mbuf, ((tagLength>>16) & 0xff));
      GWEN_Buffer_AppendByte(mbuf, ((tagLength>>8) & 0xff));
      GWEN_Buffer_AppendByte(mbuf, (tagLength & 0xff));
    }
    else if (tagLength>0xffffffL) {
      /* four byte size */
      GWEN_Buffer_AppendByte(mbuf, 0x84);
      GWEN_Buffer_AppendByte(mbuf, ((tagLength>>24) & 0xff));
      GWEN_Buffer_AppendByte(mbuf, ((tagLength>>16) & 0xff));
      GWEN_Buffer_AppendByte(mbuf, ((tagLength>>8) & 0xff));
      GWEN_Buffer_AppendByte(mbuf, (tagLength & 0xff));
    }
    else if (tagLength>0xffff) {
      /* three byte size */
      GWEN_Buffer_AppendByte(mbuf, 0x83);
      GWEN_Buffer_AppendByte(mbuf, ((tagLength>>16) & 0xff));
      GWEN_Buffer_AppendByte(mbuf, ((tagLength>>8) & 0xff));
      GWEN_Buffer_AppendByte(mbuf, (tagLength & 0xff));
    }
    else if (tagLength>0xff) {
      /* two byte size */
      GWEN_Buffer_AppendByte(mbuf, 0x82);
      GWEN_Buffer_AppendByte(mbuf, ((tagLength>>8) & 0xff));
      GWEN_Buffer_AppendByte(mbuf, (tagLength & 0xff));
    }
    else if (tagLength>127) {
      /* one byte size */
      GWEN_Buffer_AppendByte(mbuf, 0x81);
      GWEN_Buffer_AppendByte(mbuf, (tagLength & 0xff));
    }
    else {
      GWEN_Buffer_AppendByte(mbuf, (tagLength & 0x7f));
    }
  }
  else {
    /* write tag type */
    GWEN_Buffer_AppendByte(mbuf, (unsigned char)tagType);

    /* write tag length */
    GWEN_Buffer_AppendByte(mbuf, (tagLength & 0xff));
  }

  return 0;
}


static void _writeByteAsHex(uint8_t b, uint8_t *buf)
{
  uint8_t c;

  c=(b>>4)&0xf;
  if (c>9)
    c+=7;
  c+='0';
  buf[0]=c;
  c=b&0xf;
  if (c>9)
    c+=7;
  c+='0';
  buf[1]=c;
  buf[2]=0;
}


int GWEN_TLV_Buffer_To_DB(GWEN_DB_NODE *dbRecord, GWEN_BUFFER *mbuf, int len)
{
  int tlv_len=0;
  unsigned int tag_len=0;
  uint32_t data_len;
  uint8_t byte;
  int isConstructed;
  int anotherByte;
  uint8_t tag[129];
  GWEN_DB_NODE *dbTLV;


  while (tlv_len < len) {
    int rv;

    /* read TAG */
    tag_len=0;
    rv=GWEN_Buffer_ReadByte(mbuf);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "Truncated BER-TLV tag");
      return GWEN_ERROR_BAD_DATA;
    }
    byte=(uint8_t) rv;
    isConstructed = byte & BER_TLV_TAG_IS_CONSTRUCTED;
    tlv_len++;
    _writeByteAsHex(byte, &tag[tag_len]);
    tag_len+=2;
    anotherByte=((byte & BER_TLV_TAG_FIRST_BYTE_BYTE_FOLLOWS)==BER_TLV_TAG_FIRST_BYTE_BYTE_FOLLOWS);
    while (anotherByte) {
      rv=GWEN_Buffer_ReadByte(mbuf);
      if (rv<0) {
	DBG_ERROR(GWEN_LOGDOMAIN, "Truncated BER-TLV tag");
	return GWEN_ERROR_BAD_DATA;
      }
      byte=(uint8_t) rv;
      tlv_len++;
      if ((tag_len+3)>sizeof(tag)) {
	DBG_ERROR(GWEN_LOGDOMAIN, "BER-TLV tag too long");
	return GWEN_ERROR_BAD_DATA;
      }
      _writeByteAsHex(byte, &tag[tag_len]);
      tag_len+=2;
      anotherByte=byte>127;
    }
    dbTLV=GWEN_DB_Group_new((const char*) tag);

    /* read LENGTH */
    byte=GWEN_Buffer_ReadByte(mbuf);
    tlv_len++;
    if (byte>=0x81) {
      uint8_t numLengthBytes=byte-128;

      assert(byte!=0xFF);
      data_len=0;
      while (numLengthBytes--) {
        byte=GWEN_Buffer_ReadByte(mbuf);
        tlv_len++;
        data_len<<=8;
        data_len+=(uint32_t)byte;
      }
    }
    else {
      data_len=(uint8_t) byte;
    }
    GWEN_DB_SetIntValue(dbTLV, 0, "length", data_len);

    /* write VALUE */
    if (isConstructed) {
      rv=GWEN_TLV_Buffer_To_DB(dbTLV, mbuf, data_len);
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	GWEN_DB_Group_free(dbTLV);
      }
      tlv_len+=rv;
    }
    else {
      char *buffer;

      buffer=(char *)GWEN_Memory_malloc((data_len*2)+1);
      assert(buffer);
      GWEN_Text_ToHex(GWEN_Buffer_GetPosPointer(mbuf), data_len, buffer, data_len*2+1);
      GWEN_DB_SetCharValue(dbTLV, 0, "data", buffer);
      GWEN_DB_SetBinValue(dbTLV, 0, "dataBin", GWEN_Buffer_GetPosPointer(mbuf), data_len);
      GWEN_Memory_dealloc(buffer);
      GWEN_Buffer_IncrementPos(mbuf, data_len);
      tlv_len+=data_len;
    }
    GWEN_DB_AddGroup(dbRecord, dbTLV);
  }
  assert(len==tlv_len);
  return tlv_len;
}




uint32_t GWEN_TLV_ParseLength(GWEN_BUFFER *mbuf, uint32_t *tag_len_len)
{
  uint32_t data_len=0;
  uint32_t tlv_len=0;
  int rv;
  uint8_t byte;
  uint8_t anotherByte;

  /* get first byte */
  rv=GWEN_Buffer_ReadByte(mbuf);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    abort();
    return 0;
  }
  byte=(uint8_t) rv;
  tlv_len++;
  anotherByte=((byte & BER_TLV_TAG_FIRST_BYTE_BYTE_FOLLOWS)==BER_TLV_TAG_FIRST_BYTE_BYTE_FOLLOWS);
  while (anotherByte) {
    rv=GWEN_Buffer_ReadByte(mbuf);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      abort();
      return 0;
    }
    byte=(uint8_t) rv;
    tlv_len++;
    anotherByte= byte > 127;
  }
  byte = GWEN_Buffer_ReadByte(mbuf);
  tlv_len++;
  if (byte & 0x80) {
    uint8_t numLengthBytes= byte-128;
    assert(byte!=0xFF);
    data_len=0;
    while (numLengthBytes--) {
      rv=GWEN_Buffer_ReadByte(mbuf);
      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        abort();
        return 0;
      }
      byte=(uint8_t) rv;
      tlv_len++;
      data_len<<=8;
      data_len+=(uint32_t)byte;
    }
  }
  else {
    data_len= (uint8_t) byte;
  }
  *tag_len_len=tlv_len;

  return data_len;
}



