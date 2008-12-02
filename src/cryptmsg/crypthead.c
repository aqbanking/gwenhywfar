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


#include "crypthead_p.h"
#include "i18n_l.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/tag16.h>


GWEN_LIST_FUNCTIONS(GWEN_CRYPTHEAD, GWEN_CryptHead)


GWEN_CRYPTHEAD *GWEN_CryptHead_new() {
  GWEN_CRYPTHEAD *ch;

  GWEN_NEW_OBJECT(GWEN_CRYPTHEAD, ch);

  return ch;
}



void GWEN_CryptHead_free(GWEN_CRYPTHEAD *ch) {
  if (ch) {
    free(ch->keyName);
    if (ch->pKey &&ch->lKey)
      free(ch->pKey);
    GWEN_FREE_OBJECT(ch);
  }
}



GWEN_CRYPTHEAD *GWEN_CryptHead_fromBuffer(const uint8_t *p, uint32_t l) {
  if (p==NULL || l<1) {
    DBG_INFO(GWEN_LOGDOMAIN, "Bad tag");
    return NULL;
  }
  else {
    GWEN_CRYPTHEAD *ch;
    const uint8_t *sp;
    uint32_t sl;

    ch=GWEN_CryptHead_new();
    sp=p;
    sl=l;
    while(sl) {
      GWEN_TAG16 *subtag;
      uint32_t subtagLen;
      const char *subtagPtr;
      int i;

      subtag=GWEN_Tag16_fromBuffer2(sp, sl, 0);
      if (subtag==NULL) {
	DBG_INFO(GWEN_LOGDOMAIN, "Bad sub-tag");
	GWEN_CryptHead_free(ch);
	return NULL;
      }
      subtagLen=GWEN_Tag16_GetTagLength(subtag);
      subtagPtr=(const char*)GWEN_Tag16_GetTagData(subtag);

      if (subtagLen && subtagPtr) {
	switch(GWEN_Tag16_GetTagType(subtag)) {

	case GWEN_CRYPTHEAD_TLV_KEYNAME:
	  ch->keyName=(char*)malloc(subtagLen+1);
	  memmove(ch->keyName, subtagPtr, subtagLen);
	  ch->keyName[subtagLen]=0;
	  break;

	case GWEN_CRYPTHEAD_TLV_KEYNUM:
	  if (sscanf(subtagPtr, "%d", &i)==1)
	    ch->keyNumber=i;
	  break;

	case GWEN_CRYPTHEAD_TLV_KEYVER:
	  if (sscanf(subtagPtr, "%d", &i)==1)
	    ch->keyVersion=i;
	  break;

	case GWEN_CRYPTHEAD_TLV_KEY:
	  ch->pKey=(uint8_t*)malloc(subtagLen);
	  assert(ch->pKey);
	  memmove(ch->pKey, subtagPtr, subtagLen);
	  ch->lKey=subtagLen;
	  break;

	case GWEN_CRYPTHEAD_TLV_CRYPTPROFILE:
	  if (sscanf(subtagPtr, "%d", &i)==1)
	    ch->cryptProfile=i;
	  break;

	default:
          DBG_WARN(GWEN_LOGDOMAIN, "Unknown tag %02x", GWEN_Tag16_GetTagType(subtag));
	}
      }

      sp+=GWEN_Tag16_GetTagSize(subtag);
      sl-=GWEN_Tag16_GetTagSize(subtag);
      GWEN_Tag16_free(subtag);
    } /* while */

    return ch;
  }

}



int GWEN_CryptHead_toBuffer(const GWEN_CRYPTHEAD *ch, GWEN_BUFFER *buf, uint8_t tagType) {
  char numbuf[32];
  uint32_t pos;
  uint8_t *p;
  uint32_t l;

  GWEN_Buffer_AppendByte(buf, tagType);
  pos=GWEN_Buffer_GetPos(buf);
  GWEN_Buffer_AppendByte(buf, 0);
  GWEN_Buffer_AppendByte(buf, 0);

  if (ch->keyName)
    GWEN_Tag16_DirectlyToBuffer(GWEN_CRYPTHEAD_TLV_KEYNAME, ch->keyName, -1, buf);

  snprintf(numbuf, sizeof(numbuf), "%d", ch->keyNumber);
  GWEN_Tag16_DirectlyToBuffer(GWEN_CRYPTHEAD_TLV_KEYNUM, numbuf, -1, buf);

  snprintf(numbuf, sizeof(numbuf), "%d", ch->keyVersion);
  GWEN_Tag16_DirectlyToBuffer(GWEN_CRYPTHEAD_TLV_KEYVER, numbuf, -1, buf);
  if (ch->pKey && ch->lKey)
    GWEN_Tag16_DirectlyToBuffer(GWEN_CRYPTHEAD_TLV_KEY,
				(const char*)ch->pKey,
				ch->lKey,
				buf);

  snprintf(numbuf, sizeof(numbuf), "%d", ch->cryptProfile);
  GWEN_Tag16_DirectlyToBuffer(GWEN_CRYPTHEAD_TLV_CRYPTPROFILE, numbuf, -1, buf);

  /* write size */
  l=GWEN_Buffer_GetPos(buf)-pos-2;
  p=(uint8_t*)GWEN_Buffer_GetStart(buf)+pos;
  *(p++)=l & 0xff;
  *p=(l>>8) & 0xff;

  return 0;
}



const char *GWEN_CryptHead_GetKeyName(const GWEN_CRYPTHEAD *ch) {
  assert(ch);
  return ch->keyName;
}



void GWEN_CryptHead_SetKeyName(GWEN_CRYPTHEAD *ch, const char *s) {
  assert(ch);
  free(ch->keyName);
  if (s) ch->keyName=strdup(s);
  else ch->keyName=NULL;
}



int GWEN_CryptHead_GetKeyNumber(const GWEN_CRYPTHEAD *ch) {
  assert(ch);
  return ch->keyNumber;
}



void GWEN_CryptHead_SetKeyNumber(GWEN_CRYPTHEAD *ch, int i) {
  assert(ch);
  ch->keyNumber=i;
}



int GWEN_CryptHead_GetKeyVersion(const GWEN_CRYPTHEAD *ch) {
  assert(ch);
  return ch->keyVersion;
}



void GWEN_CryptHead_SetKeyVersion(GWEN_CRYPTHEAD *ch, int i) {
  assert(ch);
  ch->keyVersion=i;
}



int GWEN_CryptHead_GetCryptProfile(const GWEN_CRYPTHEAD *ch) {
  assert(ch);
  return ch->cryptProfile;
}



void GWEN_CryptHead_SetCryptProfile(GWEN_CRYPTHEAD *ch, int i) {
  assert(ch);
  ch->cryptProfile=i;
}



const uint8_t *GWEN_CryptHead_GetKeyPtr(const GWEN_CRYPTHEAD *ch) {
  assert(ch);
  return ch->pKey;
}



uint32_t GWEN_CryptHead_GetKeyLen(const GWEN_CRYPTHEAD *ch) {
  assert(ch);
  return ch->lKey;
}



void GWEN_CryptHead_SetKey(GWEN_CRYPTHEAD *ch, const uint8_t *p, uint32_t l) {
  assert(ch);
  if (ch->pKey && ch->lKey)
    free(ch->pKey);
  if (p && l) {
    ch->pKey=(uint8_t*)malloc(l);
    assert(ch->pKey);
    memmove(ch->pKey, p, l);
    ch->lKey=l;
  }
  else {
    ch->pKey=NULL;
    ch->lKey=0;
  }
}




