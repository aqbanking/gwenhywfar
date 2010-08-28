/***************************************************************************
    begin       : Sun Nov 30 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define DISABLE_DEBUGLOG


#include "sigtail_p.h"
#include "i18n_l.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/tag16.h>


GWEN_LIST_FUNCTIONS(GWEN_SIGTAIL, GWEN_SigTail)



GWEN_SIGTAIL *GWEN_SigTail_new() {
  GWEN_SIGTAIL *st;

  GWEN_NEW_OBJECT(GWEN_SIGTAIL, st);
  GWEN_LIST_INIT(GWEN_SIGTAIL, st);

  return st;
}



void GWEN_SigTail_free(GWEN_SIGTAIL *st) {
  if (st) {
    GWEN_LIST_FINI(GWEN_SIGTAIL, st);
    if (st->pSignature && st->lSignature)
      free(st->pSignature);

    GWEN_FREE_OBJECT(st);
  }
}



GWEN_SIGTAIL *GWEN_SigTail_fromBuffer(const uint8_t *p, uint32_t l) {
  if (p==NULL || l<1) {
    DBG_INFO(GWEN_LOGDOMAIN, "Bad tag");
    return NULL;
  }
  else {
    GWEN_SIGTAIL *st;
    const uint8_t *sp;
    uint32_t sl;

    st=GWEN_SigTail_new();
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
	GWEN_SigTail_free(st);
	return NULL;
      }
      subtagLen=GWEN_Tag16_GetTagLength(subtag);
      subtagPtr=(const char*)GWEN_Tag16_GetTagData(subtag);

      if (subtagLen && subtagPtr) {
	switch(GWEN_Tag16_GetTagType(subtag)) {
	case GWEN_SIGTAIL_TLV_SIGNATURE:
	  st->pSignature=(uint8_t*)malloc(subtagLen);
	  memmove(st->pSignature, subtagPtr, subtagLen);
          st->lSignature=subtagLen;
	  break;

	case GWEN_SIGTAIL_TLV_SIGNUM:
	  if (sscanf(subtagPtr, "%d", &i)==1)
	    st->signatureNumber=i;
	  break;

	default:
          DBG_WARN(GWEN_LOGDOMAIN, "Unknown tag %02x", GWEN_Tag16_GetTagType(subtag));
	}
      }

      sp+=GWEN_Tag16_GetTagSize(subtag);
      sl-=GWEN_Tag16_GetTagSize(subtag);
      GWEN_Tag16_free(subtag);
    } /* while */

    return st;
  }
}



int GWEN_SigTail_toBuffer(const GWEN_SIGTAIL *st, GWEN_BUFFER *buf, uint8_t tagType) {
  char numbuf[32];
  uint32_t pos;
  uint8_t *p;
  uint32_t l;

  GWEN_Buffer_AppendByte(buf, tagType);
  pos=GWEN_Buffer_GetPos(buf);
  GWEN_Buffer_AppendByte(buf, 0);
  GWEN_Buffer_AppendByte(buf, 0);

  if (st->pSignature && st->lSignature)
    GWEN_Tag16_DirectlyToBuffer(GWEN_SIGTAIL_TLV_SIGNATURE,
				(const char*)st->pSignature,
				st->lSignature,
				buf);

  snprintf(numbuf, sizeof(numbuf), "%d", st->signatureNumber);
  GWEN_Tag16_DirectlyToBuffer(GWEN_SIGTAIL_TLV_SIGNUM, numbuf, -1, buf);

  /* write size */
  l=GWEN_Buffer_GetPos(buf)-pos-2;
  p=(uint8_t*)GWEN_Buffer_GetStart(buf)+pos;
  *(p++)=l & 0xff;
  *p=(l>>8) & 0xff;

  return 0;
}



const uint8_t *GWEN_SigTail_GetSignaturePtr(const GWEN_SIGTAIL *st) {
  assert(st);
  return st->pSignature;
}



uint32_t GWEN_SigTail_GetSignatureLen(const GWEN_SIGTAIL *st) {
  assert(st);
  return st->lSignature;
}



void GWEN_SigTail_SetSignature(GWEN_SIGTAIL *st, const uint8_t *p, uint32_t l) {
  assert(st);
  if (st->pSignature && st->lSignature)
    free(st->pSignature);
  if (p && l) {
    st->pSignature=(uint8_t*)malloc(l);
    memmove(st->pSignature, p, l);
    st->lSignature=l;
  }
  else {
    st->pSignature=NULL;
    st->lSignature=0;
  }
}



int GWEN_SigTail_GetSignatureNumber(const GWEN_SIGTAIL *st) {
  assert(st);
  return st->signatureNumber;
}



void GWEN_SigTail_SetSignatureNumber(GWEN_SIGTAIL *st, int i) {
  assert(st);
  st->signatureNumber=i;
}






