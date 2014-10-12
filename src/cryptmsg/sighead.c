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


#include "sighead_p.h"
#include "i18n_l.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/tag16.h>


GWEN_LIST_FUNCTIONS(GWEN_SIGHEAD, GWEN_SigHead)



GWEN_SIGHEAD *GWEN_SigHead_new(void) {
  GWEN_SIGHEAD *sh;

  GWEN_NEW_OBJECT(GWEN_SIGHEAD, sh);
  GWEN_LIST_INIT(GWEN_SIGHEAD, sh);

  return sh;
}



void GWEN_SigHead_free(GWEN_SIGHEAD *sh) {
  if (sh) {
    GWEN_LIST_FINI(GWEN_SIGHEAD, sh);
    free(sh->keyName);
    GWEN_Time_free(sh->dateTime);

    GWEN_FREE_OBJECT(sh);
  }
}



GWEN_SIGHEAD *GWEN_SigHead_fromBuffer(const uint8_t *p, uint32_t l) {
  if (p==NULL || l<1) {
    DBG_INFO(GWEN_LOGDOMAIN, "Bad tag");
    return NULL;
  }
  else {
    GWEN_SIGHEAD *sh;
    const uint8_t *sp;
    uint32_t sl;

    sh=GWEN_SigHead_new();
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
        GWEN_SigHead_free(sh);
        return NULL;
      }
      subtagLen=GWEN_Tag16_GetTagLength(subtag);
      subtagPtr=(const char*)GWEN_Tag16_GetTagData(subtag);

      if (subtagLen && subtagPtr) {
        switch(GWEN_Tag16_GetTagType(subtag)) {
        case GWEN_SIGHEAD_TLV_KEYNAME:
          sh->keyName=(char*)malloc(subtagLen+1);
          memmove(sh->keyName, subtagPtr, subtagLen);
          sh->keyName[subtagLen]=0;
          break;

        case GWEN_SIGHEAD_TLV_KEYNUM:
          if (sscanf(subtagPtr, "%d", &i)==1)
            sh->keyNumber=i;
          break;

        case GWEN_SIGHEAD_TLV_KEYVER:
          if (sscanf(subtagPtr, "%d", &i)==1)
            sh->keyVersion=i;
          break;

        case GWEN_SIGHEAD_TLV_DATETIME:
          if (subtagLen<128) {
            char dt[128];

            dt[0]=0;
            strncpy(dt, (const char*) subtagPtr, sizeof(dt)-1);
            dt[sizeof(dt)-1]=0;
            sh->dateTime=GWEN_Time_fromUtcString(dt, "YYYYMMDD-hh:mm:ss");
            if (sh->dateTime==NULL) {
              DBG_ERROR(GWEN_LOGDOMAIN, "Bad format of dateTime [%s]", dt);
              GWEN_Tag16_free(subtag);
              GWEN_SigHead_free(sh);
              return NULL;
            }
          }
          else {
            DBG_ERROR(GWEN_LOGDOMAIN, "Data for dateTime too long (%d bytes)", subtagLen);
            GWEN_Tag16_free(subtag);
            GWEN_SigHead_free(sh);
            return NULL;
          }
          break;

        case GWEN_SIGHEAD_TLV_SIGPROFILE:
          if (sscanf(subtagPtr, "%d", &i)==1)
            sh->signatureProfile=i;
          break;

        case GWEN_SIGHEAD_TLV_SIGNUM:
          if (sscanf(subtagPtr, "%d", &i)==1)
            sh->signatureNumber=i;
          break;

        default:
          DBG_WARN(GWEN_LOGDOMAIN, "Unknown tag %02x", GWEN_Tag16_GetTagType(subtag));
        }
      }

      sp+=GWEN_Tag16_GetTagSize(subtag);
      sl-=GWEN_Tag16_GetTagSize(subtag);
      GWEN_Tag16_free(subtag);
    } /* while */

    return sh;
  }
}



int GWEN_SigHead_toBuffer(const GWEN_SIGHEAD *sh, GWEN_BUFFER *buf, uint8_t tagType) {
  char numbuf[32];
  uint32_t pos;
  uint8_t *p;
  uint32_t l;

  GWEN_Buffer_AppendByte(buf, tagType);
  pos=GWEN_Buffer_GetPos(buf);
  GWEN_Buffer_AppendByte(buf, 0);
  GWEN_Buffer_AppendByte(buf, 0);

  if (sh->keyName)
    GWEN_Tag16_DirectlyToBuffer(GWEN_SIGHEAD_TLV_KEYNAME, sh->keyName, -1, buf);

  snprintf(numbuf, sizeof(numbuf), "%d", sh->keyNumber);
  GWEN_Tag16_DirectlyToBuffer(GWEN_SIGHEAD_TLV_KEYNUM, numbuf, -1, buf);

  snprintf(numbuf, sizeof(numbuf), "%d", sh->keyVersion);
  GWEN_Tag16_DirectlyToBuffer(GWEN_SIGHEAD_TLV_KEYVER, numbuf, -1, buf);
  if (sh->dateTime) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 32, 0, 1);
    GWEN_Time_toUtcString(sh->dateTime, "YYYYMMDD-hh:mm:ss", tbuf);
    GWEN_Tag16_DirectlyToBuffer(GWEN_SIGHEAD_TLV_DATETIME,
                                GWEN_Buffer_GetStart(tbuf),
                                -1,
                                buf);
    GWEN_Buffer_free(tbuf);
  }

  snprintf(numbuf, sizeof(numbuf), "%d", sh->signatureProfile);
  GWEN_Tag16_DirectlyToBuffer(GWEN_SIGHEAD_TLV_SIGPROFILE, numbuf, -1, buf);

  snprintf(numbuf, sizeof(numbuf), "%d", sh->signatureNumber);
  GWEN_Tag16_DirectlyToBuffer(GWEN_SIGHEAD_TLV_SIGNUM, numbuf, -1, buf);

  /* write size */
  l=GWEN_Buffer_GetPos(buf)-pos-2;
  p=(uint8_t*)GWEN_Buffer_GetStart(buf)+pos;
  *(p++)=l & 0xff;
  *p=(l>>8) & 0xff;

  return 0;
}



const char *GWEN_SigHead_GetKeyName(const GWEN_SIGHEAD *sh) {
  assert(sh);
  return sh->keyName;
}



void GWEN_SigHead_SetKeyName(GWEN_SIGHEAD *sh, const char *s) {
  assert(sh);
  free(sh->keyName);
  if (s) sh->keyName=strdup(s);
  else sh->keyName=NULL;
}



int GWEN_SigHead_GetKeyNumber(const GWEN_SIGHEAD *sh) {
  assert(sh);
  return sh->keyNumber;
}



void GWEN_SigHead_SetKeyNumber(GWEN_SIGHEAD *sh, int i) {
  assert(sh);
  sh->keyNumber=i;
}



int GWEN_SigHead_GetKeyVersion(const GWEN_SIGHEAD *sh) {
  assert(sh);
  return sh->keyVersion;
}



void GWEN_SigHead_SetKeyVersion(GWEN_SIGHEAD *sh, int i) {
  assert(sh);
  sh->keyVersion=i;
}



const GWEN_TIME *GWEN_SigHead_GetDateTime(const GWEN_SIGHEAD *sh) {
  assert(sh);
  return sh->dateTime;
}



void GWEN_SigHead_SetDateTime(GWEN_SIGHEAD *sh, const GWEN_TIME *ti) {
  assert(sh);
  GWEN_Time_free(sh->dateTime);
  if (ti) sh->dateTime=GWEN_Time_dup(ti);
  else sh->dateTime=NULL;
}



int GWEN_SigHead_GetSignatureProfile(const GWEN_SIGHEAD *sh) {
  assert(sh);
  return sh->signatureProfile;
}



void GWEN_SigHead_SetSignatureProfile(GWEN_SIGHEAD *sh, int i) {
  assert(sh);
  sh->signatureProfile=i;
}



int GWEN_SigHead_GetSignatureNumber(const GWEN_SIGHEAD *sh) {
  assert(sh);
  return sh->signatureNumber;
}



void GWEN_SigHead_SetSignatureNumber(GWEN_SIGHEAD *sh, int i) {
  assert(sh);
  sh->signatureNumber=i;
}









