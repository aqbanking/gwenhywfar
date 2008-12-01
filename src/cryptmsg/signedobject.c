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


#include "signedobject_p.h"
#include "i18n_l.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/tag16.h>





GWEN_SIGNED_OBJECT *GWEN_SignedObject_new() {
  GWEN_SIGNED_OBJECT *so;

  GWEN_NEW_OBJECT(GWEN_SIGNED_OBJECT, so);
  so->sigHeadList=GWEN_SigHead_List_new();
  so->sigTailList=GWEN_SigTail_List_new();

  return so;
}



void GWEN_SignedObject_free(GWEN_SIGNED_OBJECT *so) {
  if (so) {
    if (so->pData && so->lData && so->dataOwned)
      free(so->pData);
    GWEN_SigHead_List_free(so->sigHeadList);
    GWEN_SigTail_List_free(so->sigTailList);
    GWEN_FREE_OBJECT(so);
  }
}



GWEN_SIGNED_OBJECT *GWEN_SignedObject_fromBuffer(const uint8_t *p, uint32_t l, int doCopy) {
  if (p==NULL || l<1) {
    DBG_INFO(GWEN_LOGDOMAIN, "Bad tag");
    return NULL;
  }
  else {
    GWEN_SIGNED_OBJECT *so;
    const uint8_t *sp;
    uint32_t sl;

    so=GWEN_SignedObject_new();
    sp=p;
    sl=l;
    while(sl) {
      GWEN_TAG16 *subtag;
      uint32_t subtagLen;
      const uint8_t *subtagPtr;

      subtag=GWEN_Tag16_fromBuffer2(sp, sl, 0);
      if (subtag==NULL) {
	DBG_INFO(GWEN_LOGDOMAIN, "Bad sub-tag");
	GWEN_SignedObject_free(so);
	return NULL;
      }
      subtagLen=GWEN_Tag16_GetTagLength(subtag);
      subtagPtr=GWEN_Tag16_GetTagData(subtag);

      if (subtagLen && subtagPtr) {
	switch(GWEN_Tag16_GetTagType(subtag)) {

	case GWEN_SIGNEDOBJECT_TLV_SIGDATA:
	  if (doCopy) {
	    so->pData=(uint8_t*)malloc(subtagLen);
	    memmove(so->pData, subtagPtr, subtagLen);
	    so->lData=subtagLen;
	    so->dataOwned=1;
	  }
	  else {
	    so->pData=(uint8_t*) subtagPtr;
	    so->lData=subtagLen;
            so->dataOwned=0;
	  }
	  break;

	case GWEN_SIGNEDOBJECT_TLV_SIGHEAD: {
	  GWEN_SIGHEAD *sh;

	  sh=GWEN_SigHead_fromBuffer(subtagPtr, subtagLen);
	  if (sh)
	    GWEN_SigHead_List_Add(sh, so->sigHeadList);
	  else {
	    DBG_ERROR(GWEN_LOGDOMAIN, "Bad signature head");
	    GWEN_Tag16_free(subtag);
	    GWEN_SignedObject_free(so);
	    return NULL;
	  }
	  break;
	}

	case GWEN_SIGNEDOBJECT_TLV_SIGTAIL: {
	  GWEN_SIGTAIL *st;

	  st=GWEN_SigTail_fromBuffer(subtagPtr, subtagLen);
	  if (st)
	    GWEN_SigTail_List_Add(st, so->sigTailList);
	  else {
	    DBG_ERROR(GWEN_LOGDOMAIN, "Bad signature tail");
	    GWEN_Tag16_free(subtag);
	    GWEN_SignedObject_free(so);
	    return NULL;
	  }
	  break;
	}

	default:
          DBG_WARN(GWEN_LOGDOMAIN, "Unknown tag %02x", GWEN_Tag16_GetTagType(subtag));
	}
      }

      sp+=GWEN_Tag16_GetTagSize(subtag);
      sl-=GWEN_Tag16_GetTagSize(subtag);
      GWEN_Tag16_free(subtag);
    } /* while */

    return so;
  }
}



int GWEN_SignedObject_toBuffer(const GWEN_SIGNED_OBJECT *so, GWEN_BUFFER *buf, uint8_t tagType) {
  GWEN_SIGHEAD *sh;
  GWEN_SIGTAIL *st;
  uint32_t fpos;
  uint8_t *fp;
  uint32_t fl;

  assert(so);
  GWEN_Buffer_AppendByte(buf, tagType);
  fpos=GWEN_Buffer_GetPos(buf);
  GWEN_Buffer_AppendByte(buf, 0);
  GWEN_Buffer_AppendByte(buf, 0);

  /* write signature heads */
  sh=GWEN_SigHead_List_First(so->sigHeadList);
  while(sh) {
    int rv;

    /* write signature head */
    rv=GWEN_SigHead_toBuffer(sh, buf, GWEN_SIGNEDOBJECT_TLV_SIGHEAD);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    sh=GWEN_SigHead_List_Next(sh);
  }

  /* write data */
  if (so->pData && so->lData)
    GWEN_Tag16_DirectlyToBuffer(GWEN_SIGNEDOBJECT_TLV_SIGDATA,
				(const char*)so->pData,
				so->lData,
				buf);

  /* write signature tails */
  st=GWEN_SigTail_List_First(so->sigTailList);
  while(st) {
    int rv;

    /* write signature tail */
    rv=GWEN_SigTail_toBuffer(st, buf, GWEN_SIGNEDOBJECT_TLV_SIGTAIL);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    st=GWEN_SigTail_List_Next(st);
  }

  /* write size */
  fl=GWEN_Buffer_GetPos(buf)-fpos-2;
  fp=(uint8_t*)GWEN_Buffer_GetStart(buf)+fpos;
  *(fp++)=(fl>>8) & 0xff;
  *fp=fl & 0xff;

  return 0;
}



const uint8_t *GWEN_SignedObject_GetDataPtr(const GWEN_SIGNED_OBJECT *so) {
  assert(so);
  return so->pData;
}



uint32_t GWEN_SignedObject_GetDataLen(const GWEN_SIGNED_OBJECT *so) {
  assert(so);
  return so->lData;
}



void GWEN_SignedObject_SetData(GWEN_SIGNED_OBJECT *so, const uint8_t *p, uint32_t l) {
  assert(so);
  if (so->pData && so->lData && so->dataOwned)
    free(so->pData);
  if (p && l) {
    so->pData=(uint8_t*)malloc(l);
    memmove(so->pData, p, l);
    so->lData=l;
    so->dataOwned=1;
  }
  else {
    so->pData=NULL;
    so->lData=0;
    so->dataOwned=0;
  }
}



GWEN_SIGHEAD_LIST *GWEN_SignedObject_GetSigHeadList(const GWEN_SIGNED_OBJECT *so) {
  assert(so);
  return so->sigHeadList;
}



void GWEN_SignedObject_AddSigHead(GWEN_SIGNED_OBJECT *so, GWEN_SIGHEAD *sh) {
  assert(so);
  GWEN_SigHead_List_Add(sh, so->sigHeadList);
}



GWEN_SIGTAIL_LIST *GWEN_SignedObject_GetSigTailList(const GWEN_SIGNED_OBJECT *so) {
  assert(so);
  return so->sigTailList;
}



void GWEN_SignedObject_AddSigTail(GWEN_SIGNED_OBJECT *so, GWEN_SIGTAIL *st) {
  assert(so);
  GWEN_SigTail_List_Add(st, so->sigTailList);
}



GWEN_SIGHEAD *GWEN_SignedObject_GetSigHeadByNumber(const GWEN_SIGNED_OBJECT *so, int i) {
  GWEN_SIGHEAD *sh;

  assert(so);
  sh=GWEN_SigHead_List_First(so->sigHeadList);
  while(sh) {
    if (GWEN_SigHead_GetSignatureNumber(sh)==i)
      break;
    sh=GWEN_SigHead_List_Next(sh);
  }

  return sh;
}



GWEN_SIGTAIL *GWEN_SignedObject_GetSigTailByNumber(const GWEN_SIGNED_OBJECT *so, int i) {
  GWEN_SIGTAIL *st;

  assert(so);
  st=GWEN_SigTail_List_First(so->sigTailList);
  while(st) {
    if (GWEN_SigTail_GetSignatureNumber(st)==i)
      break;
    st=GWEN_SigTail_List_Next(st);
  }

  return st;
}





