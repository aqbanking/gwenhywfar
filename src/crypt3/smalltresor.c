/***************************************************************************
    begin       : Wed May 11 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define DISABLE_DEBUGLOG


#include "smalltresor.h"

#include <gwenhywfar/mdigest.h>
#include <gwenhywfar/cryptkey.h>
#include <gwenhywfar/cryptdefs.h>
#include <gwenhywfar/cryptkeysym.h>
#include <gwenhywfar/padd.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/debug.h>



#define BLOWFISH_KEYSIZE 32




static int _encodeData(const uint8_t *ptr,
		       uint32_t len,
		       uint8_t *pOutData,
		       uint32_t *pOutLen,
		       const uint8_t *pKey) {
  GWEN_CRYPT_KEY *k;
  int rv;

  k=GWEN_Crypt_KeyBlowFish_fromData(GWEN_Crypt_CryptMode_Cbc,
				    BLOWFISH_KEYSIZE,
				    pKey, BLOWFISH_KEYSIZE);
  if (!k) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not create key");
    return GWEN_ERROR_ENCRYPT;
  }

  rv=GWEN_Crypt_Key_Encipher(k,
			     ptr, len,
			     pOutData, pOutLen);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Error on GWEN_Crypt_Key_Encipher(len=%d, *outLen=%d): %d",
	      len, *pOutLen, rv);
    GWEN_Crypt_Key_free(k);
    return rv;
  }
  GWEN_Crypt_Key_free(k);

  return 0;
}



static int _encode(const uint8_t *p, uint32_t len, GWEN_BUFFER *buf, int iterations) {
  GWEN_BUFFER *tbuf1;
  GWEN_BUFFER *tbuf2;
  int i;
  int rv;
  uint8_t *pDest;
  uint32_t lDest;

  tbuf1=GWEN_Buffer_new(0, 256, 0, 1);
  tbuf2=GWEN_Buffer_new(0, 256, 0, 1);

  for (i=0; i<iterations; i++) {
    GWEN_BUFFER *tmpbufptr;
    GWEN_CRYPT_KEY *ck;

    ck=GWEN_Crypt_KeyBlowFish_Generate(GWEN_Crypt_CryptMode_Cbc,
                                       BLOWFISH_KEYSIZE,
                                       3);
    if (ck==NULL) {
      DBG_ERROR(GWEN_LOGDOMAIN, "here");
      GWEN_Buffer_free(tbuf2);
      GWEN_Buffer_free(tbuf1);
      return GWEN_ERROR_INTERNAL;
    }

    lDest=len;
    GWEN_Buffer_AllocRoom(tbuf1, lDest);
    pDest=(uint8_t*)GWEN_Buffer_GetPosPointer(tbuf1);

    rv=GWEN_Crypt_Key_Encipher(ck, p, len, pDest, &lDest);
    if (rv<0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error on GWEN_Crypt_Key_Encipher(len=%d): %d", len, rv);
      GWEN_Crypt_Key_free(ck);
      GWEN_Buffer_free(tbuf2);
      GWEN_Buffer_free(tbuf1);
      return rv;
    }

    GWEN_Buffer_IncrementPos(tbuf1, lDest);
    GWEN_Buffer_AdjustUsedBytes(tbuf1);

    /* append key */
    GWEN_Buffer_AppendBytes(tbuf1,
                            (const char*) GWEN_Crypt_KeyBlowFish_GetKeyDataPtr(ck),
                            BLOWFISH_KEYSIZE);
    GWEN_Crypt_Key_free(ck);

    /* swap buffers */
    tmpbufptr=tbuf2;
    tbuf2=tbuf1;
    tbuf1=tmpbufptr;
    /* reset buffer 1, point to buffer 2 for next iteration */
    GWEN_Buffer_Reset(tbuf1);
    p=(uint8_t*)GWEN_Buffer_GetStart(tbuf2);
    len=GWEN_Buffer_GetUsedBytes(tbuf2);
  }

  /* add data from last round to buffer */
  GWEN_Buffer_AppendBytes(buf, (const char*) p, len);

  GWEN_Buffer_free(tbuf2);
  GWEN_Buffer_free(tbuf1);

  return 0;
}



static int _addRandomBytes(GWEN_BUFFER *dst, int withLength) {
  uint8_t v1[2];
  uint16_t len;

  GWEN_Crypt_Random(3, v1, sizeof(v1));
  len=((v1[0]<<8)+v1[1]) & 0x3fff;

  if (withLength) {
    GWEN_Buffer_AppendByte(dst, (len>>8) & 0xff);
    GWEN_Buffer_AppendByte(dst, len & 0xff);
  }
  GWEN_Buffer_AllocRoom(dst, len);
  GWEN_Crypt_Random(3, (uint8_t*) GWEN_Buffer_GetPosPointer(dst), len);
  GWEN_Buffer_IncrementPos(dst, len);
  GWEN_Buffer_AdjustUsedBytes(dst);

  return 0;
}



static int _decodeData(const uint8_t *ptr,
		       uint32_t len,
		       uint8_t *pOutData,
		       uint32_t *pOutLen,
		       const uint8_t *pKey) {
  GWEN_CRYPT_KEY *k;
  int rv;

  k=GWEN_Crypt_KeyBlowFish_fromData(GWEN_Crypt_CryptMode_Cbc,
				    BLOWFISH_KEYSIZE,
				    pKey, BLOWFISH_KEYSIZE);
  if (!k) {
    return GWEN_ERROR_DECRYPT;
  }

  rv=GWEN_Crypt_Key_Decipher(k, ptr, len, pOutData, pOutLen);
  GWEN_Crypt_Key_free(k);
  if (rv)
    return rv;
  return 0;
}



static int _decode(const uint8_t *p, uint32_t len, GWEN_BUFFER *dst, int iterations) {
  GWEN_BUFFER *tbuf1;
  GWEN_BUFFER *tbuf2;
  int i;
  int rv;
  uint8_t *pDest;
  uint32_t lDest;
  uint8_t key[BLOWFISH_KEYSIZE];

  tbuf1=GWEN_Buffer_new(0, 256, 0, 1);
  tbuf2=GWEN_Buffer_new(0, 256, 0, 1);

  for (i=0; i<iterations; i++) {
    GWEN_BUFFER *tmpbufptr;

    /* last 16 bytes are the key for the next data */
    memmove(key, p+(len-sizeof(key)), sizeof(key));
    len-=sizeof(key);
    lDest=len;
    GWEN_Buffer_AllocRoom(tbuf1, lDest);
    pDest=(uint8_t*)GWEN_Buffer_GetPosPointer(tbuf1);
    /* only unpadd for last loop */
    rv=_decodeData(p, len, pDest, &lDest, key);
    if (rv) {
      GWEN_Buffer_free(tbuf2);
      GWEN_Buffer_free(tbuf1);
      return rv;
    }
    GWEN_Buffer_IncrementPos(tbuf1, lDest);
    GWEN_Buffer_AdjustUsedBytes(tbuf1);
    /* swap buffers */
    tmpbufptr=tbuf2;
    tbuf2=tbuf1;
    tbuf1=tmpbufptr;
    /* reset buffer 1, point to buffer 2 for next iteration */
    GWEN_Buffer_Reset(tbuf1);
    p=(const uint8_t*)GWEN_Buffer_GetStart(tbuf2);
    len=GWEN_Buffer_GetUsedBytes(tbuf2);
  }

  /* return buffer */
  GWEN_Buffer_AppendBytes(dst,
			  GWEN_Buffer_GetStart(tbuf2),
			  GWEN_Buffer_GetUsedBytes(tbuf2));
  GWEN_Buffer_free(tbuf2);
  GWEN_Buffer_free(tbuf1);

  return 0;
}






int GWEN_SmallTresor_Encrypt(const uint8_t *src,
			     uint32_t slen,
			     const char *password,
			     GWEN_BUFFER *dst,
			     int passwordIterations,
			     int cryptIterations) {
  GWEN_BUFFER *tbuf;
  GWEN_BUFFER *xbuf;
  uint32_t x;
  const uint8_t *p;
  uint8_t *pDest;
  uint32_t lDest;
  uint32_t len;
  int rv;
  GWEN_MDIGEST *md;
  uint8_t salt[128];
  uint8_t key[BLOWFISH_KEYSIZE];

  /* first derive the key from the given password */
  GWEN_Crypt_Random(3, salt, sizeof(salt));
  md=GWEN_MDigest_Sha256_new();
  rv=GWEN_MDigest_PBKDF2(md, password, salt, sizeof(salt), key, BLOWFISH_KEYSIZE, passwordIterations);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    return rv;
  }
  GWEN_MDigest_free(md);

  tbuf=GWEN_Buffer_new(0, slen+(cryptIterations*BLOWFISH_KEYSIZE), 0, 1);

  /* add random bytes at the beginning */
  rv=_addRandomBytes(tbuf, 1);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }

  /* add length of data */
  GWEN_Buffer_AppendByte(tbuf, (slen>>8) & 0xff);
  GWEN_Buffer_AppendByte(tbuf, slen & 0xff);

  /* add data itself */
  GWEN_Buffer_AppendBytes(tbuf, (const char*) src, slen);

  /* add random bytes at the end (without length marker) */
  rv=_addRandomBytes(tbuf, 0);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }

  /* padd using iso 9796_2 */
  len=GWEN_Buffer_GetUsedBytes(tbuf);
  x=(len+7+12) & ~0x7;
  rv=GWEN_Padd_PaddWithIso9796_2(tbuf, x);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }

  /* actually encode the data into xbuf */
  xbuf=GWEN_Buffer_new(0, GWEN_Buffer_GetUsedBytes(tbuf)+(cryptIterations*BLOWFISH_KEYSIZE), 0, 1);
  rv=_encode((const uint8_t*) GWEN_Buffer_GetStart(tbuf),
	     GWEN_Buffer_GetUsedBytes(tbuf),
	     xbuf,
	     cryptIterations);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(xbuf);
    GWEN_Buffer_free(tbuf);
    return rv;
  }
  GWEN_Buffer_free(tbuf);

  /* append salt (including length) to dst buffer */
  len=sizeof(salt);
  GWEN_Buffer_AppendByte(dst, (len>>8) & 0xff);
  GWEN_Buffer_AppendByte(dst, len & 0xff);
  GWEN_Buffer_AppendBytes(dst, (const char*) salt, len);

  /* final round */
  p=(const uint8_t*) GWEN_Buffer_GetStart(xbuf);
  len=GWEN_Buffer_GetUsedBytes(xbuf);

  GWEN_Buffer_AllocRoom(dst, len);
  pDest=(uint8_t*)GWEN_Buffer_GetPosPointer(dst);
  lDest=len;
  rv=_encodeData(p, len, pDest, &lDest, key);
  if (rv<0) {
    GWEN_Buffer_free(xbuf);
    return rv;
  }
  GWEN_Buffer_IncrementPos(dst, lDest);
  GWEN_Buffer_AdjustUsedBytes(dst);

  GWEN_Buffer_free(xbuf);

  return 0;
}



int GWEN_SmallTresor_Decrypt(const uint8_t *p,
			     uint32_t len,
			     const char *password,
			     GWEN_BUFFER *dst,
			     int passwordIterations,
			     int cryptIterations) {
  GWEN_BUFFER *tbuf1;
  GWEN_BUFFER *tbuf2;
  int rv;
  uint8_t *pDest;
  uint32_t lDest;
  GWEN_MDIGEST *md;
  uint8_t key[BLOWFISH_KEYSIZE];

  if (len<2) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid data length");
    return GWEN_ERROR_INVALID;
  }

  /* first derive the key from the given password */
  lDest=(p[0]<<8)+p[1];
  if (lDest>len-2) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid salt length");
    return GWEN_ERROR_BAD_DATA;
  }

  md=GWEN_MDigest_Sha256_new();
  rv=GWEN_MDigest_PBKDF2(md, password, p+2, lDest, key, BLOWFISH_KEYSIZE, passwordIterations);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    return rv;
  }
  GWEN_MDigest_free(md);

  /* remove salt from input */
  p+=2+lDest;
  len-=2+lDest;

  /* check size */
  if (len<(BLOWFISH_KEYSIZE*cryptIterations)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Data too small");
    return GWEN_ERROR_INVALID;
  }

  /* now decrypt first round */
  tbuf1=GWEN_Buffer_new(0, len, 0, 1);
  GWEN_Buffer_AllocRoom(tbuf1, len);

  pDest=(uint8_t*)GWEN_Buffer_GetPosPointer(tbuf1);
  lDest=len;
  rv=_decodeData(p, len, pDest, &lDest, key);
  if (rv<0) {
    GWEN_Buffer_free(tbuf1);
    return rv;
  }
  GWEN_Buffer_IncrementPos(tbuf1, lDest);
  GWEN_Buffer_AdjustUsedBytes(tbuf1);

  /* decode the next rounds */
  p=(const uint8_t*) GWEN_Buffer_GetStart(tbuf1);
  len=GWEN_Buffer_GetUsedBytes(tbuf1);
  tbuf2=GWEN_Buffer_new(0, len, 0, 1);
  rv=_decode(p, len, tbuf2, cryptIterations);
  if (rv<0) {
    GWEN_Buffer_free(tbuf2);
    GWEN_Buffer_free(tbuf1);
    return rv;
  }
  GWEN_Buffer_free(tbuf1);

  /* unpadd */
  rv=GWEN_Padd_UnpaddWithIso9796_2(tbuf2);
  if (rv<0) {
    GWEN_Buffer_free(tbuf2);
    return rv;
  }

  /* extract data */
  p=(const uint8_t*) GWEN_Buffer_GetStart(tbuf2);
  len=GWEN_Buffer_GetUsedBytes(tbuf2);

  /* skip random bytes at the beginning */
  lDest=(p[0]<<8)+p[1];
  if (lDest>len-2) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid random area length");
    GWEN_Buffer_free(tbuf2);
    return GWEN_ERROR_BAD_DATA;
  }
  p+=2+lDest;
  len-=2+lDest;

  /* get size of data */
  lDest=(p[0]<<8)+p[1];
  if (lDest>len-2) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid data length");
    GWEN_Buffer_free(tbuf2);
    return GWEN_ERROR_BAD_DATA;
  }
  p+=2;
  len-=2;
  GWEN_Buffer_AppendBytes(dst, (const char*) p, lDest);

  GWEN_Buffer_free(tbuf2);

  return 0;
}







