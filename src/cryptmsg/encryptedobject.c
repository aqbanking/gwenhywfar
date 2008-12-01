/***************************************************************************
    begin       : Mon Dec 01 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "encryptedobject_p.h"
#include "i18n_l.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/tag16.h>





GWEN_ENCRYPTED_OBJECT *GWEN_EncryptedObject_new() {
  GWEN_ENCRYPTED_OBJECT *eo;

  GWEN_NEW_OBJECT(GWEN_ENCRYPTED_OBJECT, eo);

  return eo;
}



void GWEN_EncryptedObject_free(GWEN_ENCRYPTED_OBJECT *eo) {
  if (eo) {
    if (eo->pData && eo->lData && eo->dataOwned)
      free(eo->pData);
    GWEN_CryptHead_free(eo->cryptHead);
    GWEN_FREE_OBJECT(eo);
  }
}



GWEN_ENCRYPTED_OBJECT *GWEN_EncryptedObject_fromBuffer(const uint8_t *p, uint32_t l, int doCopy) {
  if (p==NULL || l<1) {
    DBG_INFO(GWEN_LOGDOMAIN, "Bad tag");
    return NULL;
  }
  else {
    GWEN_ENCRYPTED_OBJECT *eo;
    const uint8_t *sp;
    uint32_t sl;

    eo=GWEN_EncryptedObject_new();
    sp=p;
    sl=l;
    while(sl) {
      GWEN_TAG16 *subtag;
      uint32_t subtagLen;
      const uint8_t *subtagPtr;

      subtag=GWEN_Tag16_fromBuffer2(sp, sl, 0);
      if (subtag==NULL) {
	DBG_INFO(GWEN_LOGDOMAIN, "Bad sub-tag");
	GWEN_EncryptedObject_free(eo);
	return NULL;
      }
      subtagLen=GWEN_Tag16_GetTagLength(subtag);
      subtagPtr=GWEN_Tag16_GetTagData(subtag);

      if (subtagLen && subtagPtr) {
	switch(GWEN_Tag16_GetTagType(subtag)) {

	case GWEN_ENCRYPTEDOBJECT_TLV_CRYPTHEAD: {
	  GWEN_CRYPTHEAD *ch;

	  ch=GWEN_CryptHead_fromBuffer(subtagPtr, subtagLen);
	  if (ch)
            eo->cryptHead=ch;
	  else {
	    DBG_ERROR(GWEN_LOGDOMAIN, "Bad signature head");
	    GWEN_Tag16_free(subtag);
	    GWEN_EncryptedObject_free(eo);
	    return NULL;
	  }
	  break;
	}

	case GWEN_ENCRYPTEDOBJECT_TLV_CRYPTDATA:
	  if (doCopy) {
	    eo->pData=(uint8_t*)malloc(subtagLen);
	    memmove(eo->pData, subtagPtr, subtagLen);
	    eo->lData=subtagLen;
	    eo->dataOwned=1;
	  }
	  else {
	    eo->pData=(uint8_t*) subtagPtr;
	    eo->lData=subtagLen;
            eo->dataOwned=0;
	  }
	  break;

	default:
          DBG_WARN(GWEN_LOGDOMAIN, "Unknown tag %02x", GWEN_Tag16_GetTagType(subtag));
	}
      }

      sp+=GWEN_Tag16_GetTagSize(subtag);
      sl-=GWEN_Tag16_GetTagSize(subtag);
      GWEN_Tag16_free(subtag);
    } /* while */

    return eo;
  }
}



int GWEN_EncryptedObject_toBuffer(const GWEN_ENCRYPTED_OBJECT *eo, GWEN_BUFFER *buf, uint8_t tagType) {
  uint32_t fpos;
  uint8_t *fp;
  uint32_t fl;

  assert(eo);
  GWEN_Buffer_AppendByte(buf, tagType);
  fpos=GWEN_Buffer_GetPos(buf);
  GWEN_Buffer_AppendByte(buf, 0);
  GWEN_Buffer_AppendByte(buf, 0);

  /* write crypt head */
  if (eo->cryptHead) {
    int rv;

    /* write signature head */
    rv=GWEN_CryptHead_toBuffer(eo->cryptHead, buf, GWEN_ENCRYPTEDOBJECT_TLV_CRYPTHEAD);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  /* write data */
  if (eo->pData && eo->lData)
    GWEN_Tag16_DirectlyToBuffer(GWEN_ENCRYPTEDOBJECT_TLV_CRYPTDATA,
				(const char*)eo->pData,
				eo->lData,
				buf);

  /* write size */
  fl=GWEN_Buffer_GetPos(buf)-fpos-2;
  fp=(uint8_t*)GWEN_Buffer_GetStart(buf)+fpos;
  *(fp++)=(fl>>8) & 0xff;
  *fp=fl & 0xff;

  return 0;
}



const uint8_t *GWEN_EncryptedObject_GetDataPtr(const GWEN_ENCRYPTED_OBJECT *eo) {
  assert(eo);
  return eo->pData;
}



uint32_t GWEN_EncryptedObject_GetDataLen(const GWEN_ENCRYPTED_OBJECT *eo) {
  assert(eo);
  return eo->lData;
}



void GWEN_EncryptedObject_SetData(GWEN_ENCRYPTED_OBJECT *eo, const uint8_t *p, uint32_t l) {
  assert(eo);
  if (eo->pData && eo->lData && eo->dataOwned)
    free(eo->pData);
  if (p && l) {
    eo->pData=(uint8_t*)malloc(l);
    memmove(eo->pData, p, l);
    eo->lData=l;
    eo->dataOwned=1;
  }
  else {
    eo->pData=NULL;
    eo->lData=0;
    eo->dataOwned=0;
  }
}



GWEN_CRYPTHEAD *GWEN_EncryptedObject_GetCryptHead(const GWEN_ENCRYPTED_OBJECT *eo) {
  assert(eo);
  return eo->cryptHead;
}



void GWEN_EncryptedObject_SetCryptHead(GWEN_ENCRYPTED_OBJECT *eo, GWEN_CRYPTHEAD *ch) {
  assert(eo);
  eo->cryptHead=ch;
}







