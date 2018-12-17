/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: crypttoken.h 1113 2007-01-10 09:14:16Z martin $
    begin       : Wed Mar 16 2005
    copyright   : (C) 2005 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "cryptdefs_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>

#include <gwenhywfar/mdigest.h>

#include <gcrypt.h>




GWEN_CRYPT_PINTYPE GWEN_Crypt_PinType_fromString(const char *s) {
  assert(s);
  if (strcasecmp(s, "none")==0)
    return GWEN_Crypt_PinType_None;
  else if (strcasecmp(s, "access")==0)
    return GWEN_Crypt_PinType_Access;
  else if (strcasecmp(s, "manage")==0)
    return GWEN_Crypt_PinType_Manage;
  return GWEN_Crypt_PinType_Unknown;
}



const char *GWEN_Crypt_PinType_toString(GWEN_CRYPT_PINTYPE pt) {
  switch(pt) {
  case GWEN_Crypt_PinType_None:
    return "none";
  case GWEN_Crypt_PinType_Access:
    return "access";
  case GWEN_Crypt_PinType_Manage:
    return "manage";
  default:
    return "unknown";
  }
}



GWEN_CRYPT_PINENCODING GWEN_Crypt_PinEncoding_fromString(const char *s) {
  assert(s);
  if (strcasecmp(s, "none")==0)
    return GWEN_Crypt_PinEncoding_None;
  else if (strcasecmp(s, "bin")==0)
    return GWEN_Crypt_PinEncoding_Bin;
  else if (strcasecmp(s, "bcd")==0)
    return GWEN_Crypt_PinEncoding_Bcd;
  else if (strcasecmp(s, "ascii")==0)
    return GWEN_Crypt_PinEncoding_Ascii;
  else if (strcasecmp(s, "fpin2")==0)
    return GWEN_Crypt_PinEncoding_FPin2;
  return GWEN_Crypt_PinEncoding_Unknown;
}



const char *GWEN_Crypt_PinEncoding_toString(GWEN_CRYPT_PINENCODING pe) {
  switch(pe) {
  case GWEN_Crypt_PinEncoding_None:
    return "none";
  case GWEN_Crypt_PinEncoding_Bin:
    return "bin";
  case GWEN_Crypt_PinEncoding_Bcd:
    return "bcd";
  case GWEN_Crypt_PinEncoding_Ascii:
    return "ascii";
  case GWEN_Crypt_PinEncoding_FPin2:
    return "fpin2";
  default:
    return "unknown";
  }
}



int GWEN_Crypt__TransformFromBCD(unsigned char *buffer,
                                 unsigned int bufLength,
                                 unsigned int *pinLength) {
  unsigned char *newBuf;
  unsigned char *p;
  unsigned int newSize;
  unsigned int i;
  unsigned int cnt=0;

  if (*pinLength==0)
    return 0;

  newSize=*pinLength*2;
  newBuf=(unsigned char*)malloc(newSize);
  p=newBuf;
  for (i=0; i<*pinLength; i++) {
    unsigned char c1;
    unsigned char c2;

    c1=buffer[i];
    /* 1st digit */
    c2=(c1 & 0xf0)>>4;
    if (c2==0x0f)
      break;
    *(p++)=c2+'0';
    cnt++;
    /* 2nd digit */
    c2=(c1 & 0x0f);
    if (c2==0x0f)
      break;
    *(p++)=c2+'0';
    cnt++;
  }

  if (cnt>bufLength) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Converted pin is too long (%d>%d)",
              cnt, bufLength);
    free(newBuf);
    return GWEN_ERROR_BUFFER_OVERFLOW;
  }

  memset(buffer, 0, bufLength);
  memmove(buffer, newBuf, cnt);
  *pinLength=cnt;
  free(newBuf);
  return 0;
}



int GWEN_Crypt__TransformFromFPIN2(unsigned char *buffer,
                                   unsigned int bufLength,
                                   unsigned int *pinLength) {
  unsigned char *newBuf;
  unsigned char *p;
  unsigned int newSize;
  unsigned int i;
  unsigned int cnt=0;
  unsigned int len;

  if (*pinLength<8) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Pin too small to be a FPIN2 (%d<8)", *pinLength);
    return GWEN_ERROR_INVALID;
  }
  len=(buffer[0] & 0x0f);
  newSize=len*2;
  newBuf=(unsigned char*)malloc(newSize);
  p=newBuf;
  for (i=1; i<8; i++) {
    unsigned char c1;
    unsigned char c2;

    if (cnt>=len)
      break;

    c1=buffer[i];
    /* 1st digit */
    c2=(c1 & 0xf0)>>4;
    if (c2==0x0f)
      break;
    *(p++)=c2+'0';
    cnt++;
    if (cnt>=len)
      break;

    /* 2nd digit */
    c2=(c1 & 0x0f);
    if (c2==0x0f)
      break;
    *(p++)=c2+'0';
    cnt++;
  }

  if (cnt>bufLength) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Converted pin is too long (%d>%d)",
              cnt, bufLength);
    free(newBuf);
    return GWEN_ERROR_BUFFER_OVERFLOW;
  }

  memset(buffer, 0, bufLength);
  memmove(buffer, newBuf, cnt);
  *pinLength=cnt;
  return 0;
}



int GWEN_Crypt__TransformFromBin(unsigned char *buffer,
                                 unsigned int bufLength,
                                 unsigned int *pinLength) {
  unsigned int i;
  unsigned char *newBuf;
  unsigned char *p;
  unsigned int newSize;

  if (*pinLength==0)
    return 0;

  newSize=*pinLength;
  newBuf=(unsigned char*)malloc(newSize);
  p=newBuf;

  for (i=0; i<*pinLength; i++) {
    unsigned char c;

    c=buffer[i];
    if (c>9) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Invalid element in pin (a digit > 9)");
      free(newBuf);
      return GWEN_ERROR_INVALID;
    }
    *p=c+'0';
  }
  memset(buffer, 0, bufLength);
  memmove(buffer, newBuf, *pinLength);
  free(newBuf);

  return 0;
}



int GWEN_Crypt__TransformToBCD(unsigned char *buffer,
                               unsigned int bufLength,
                               unsigned int *pinLength) {
  unsigned char *newBuf;
  unsigned char *p;
  unsigned int newSize;
  unsigned int i;
  unsigned int cnt=0;

  newSize=*pinLength/2+1;
  newBuf=(unsigned char*)malloc(newSize);
  memset(newBuf, 0xff, newSize);
  p=newBuf;
  i=0;
  while (i<*pinLength) {
    unsigned char c1;
    unsigned char c2;

    /* 1st digit */
    c1=buffer[i];
    if (c1<'0' || c1>'9') {
      DBG_ERROR(GWEN_LOGDOMAIN, "Invalid element in pin (a non-number character)");
      free(newBuf);
      return GWEN_ERROR_INVALID;
    }
    c1-='0';
    c1=c1<<4;
    *p=c1+0x0f; /* don't incement yet */
    cnt++; /* only increment once !! */
    i++;
    if (i>=*pinLength)
      break;

    /* 2nd digit */
    c2=buffer[i];
    if (c2<'0' || c2>'9') {
      DBG_ERROR(GWEN_LOGDOMAIN, "Invalid element in pin (a non-number character)");
      free(newBuf);
      return GWEN_ERROR_INVALID;
    }
    c2-='0';
    c1|=(c2 & 0x0f);
    *(p++)=c1;
    i++;
  }

  if (cnt>bufLength) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Converted pin is too long (%d>%d)",
              cnt, bufLength);
    free(newBuf);
    return GWEN_ERROR_BUFFER_OVERFLOW;
  }

  memset(buffer, 0, bufLength);
  for (i=0; i<cnt; i++)
    buffer[i]=newBuf[i];
  *pinLength=cnt;
  free(newBuf);
  return 0;
}



int GWEN_Crypt__TransformToFPIN2(unsigned char *buffer,
                                 unsigned int bufLength,
                                 unsigned int *pinLength) {
  unsigned char *newBuf;
  unsigned char *p;
  unsigned int newSize;
  unsigned int i;

  if (*pinLength>14) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Pin too long for FPIN2 (%d>14)",
              *pinLength);
    return GWEN_ERROR_INVALID;
  }
  if (8>bufLength) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Converted pin is too long (8>%d)",
              bufLength);
    return GWEN_ERROR_BUFFER_OVERFLOW;
  }

  newSize=8;
  newBuf=(unsigned char*)malloc(newSize);
  memset(newBuf, 0xff, newSize);
  p=newBuf;
  *(p++)=0x20+*pinLength;
  i=0;
  while (i<*pinLength) {
    unsigned char c1;
    unsigned char c2;

    /* 1st digit */
    c1=buffer[i];
    if (c1<'0' || c1>'9') {
      DBG_ERROR(GWEN_LOGDOMAIN, "Invalid element in pin (a non-number character)");
      free(newBuf);
      return GWEN_ERROR_INVALID;
    }
    c1-='0';
    c1=c1<<4;
    *p=c1+0x0f; /* don't incement yet */
    i++;
    if (i>=*pinLength)
      break;

    /* 2nd digit */
    c2=buffer[i];
    if (c2<'0' || c2>'9') {
      DBG_ERROR(GWEN_LOGDOMAIN, "Invalid element in pin (a non-number character)");
      free(newBuf);
      return GWEN_ERROR_INVALID;
    }
    c2-='0';
    c1|=(c2 & 0x0f);
    *(p++)=c1;
    i++;
  }

  memset(buffer, 0, bufLength);
  for (i=0; i<8; i++)
    buffer[i]=newBuf[i];
  *pinLength=8;
  free(newBuf);
  return 0;

}



int GWEN_Crypt__TransformToBin(unsigned char *buffer,
                               unsigned int bufLength,
                               unsigned int *pinLength) {
  unsigned int i;
  unsigned char *newBuf;
  unsigned char *p;
  unsigned int newSize;

  if (*pinLength==0)
    return 0;

  newSize=*pinLength;
  newBuf=(unsigned char*)malloc(newSize);
  p=newBuf;

  for (i=0; i<*pinLength; i++) {
    unsigned char c;

    c=buffer[i];
    if (c<'0' || c>'9') {
      DBG_ERROR(GWEN_LOGDOMAIN, "Invalid element in pin (a non-number character)");
      free(newBuf);
      return GWEN_ERROR_INVALID;
    }
    *(p++)=c-'0';
  }
  memset(buffer, 0, bufLength);
  memmove(buffer, newBuf, *pinLength);
  free(newBuf);

  return 0;
}



int GWEN_Crypt_TransformPin(GWEN_CRYPT_PINENCODING peSrc,
                            GWEN_CRYPT_PINENCODING peDst,
                            unsigned char *buffer,
                            unsigned int bufLength,
                            unsigned int *pinLength) {
  int rv;

  if (peSrc==peDst)
    return 0;

  switch(peSrc) {
  case GWEN_Crypt_PinEncoding_Bin:
    rv=GWEN_Crypt__TransformFromBin(buffer, bufLength, pinLength);
    break;
  case GWEN_Crypt_PinEncoding_Bcd:
    rv=GWEN_Crypt__TransformFromBCD(buffer, bufLength, pinLength);
    break;
  case GWEN_Crypt_PinEncoding_Ascii:
    rv=0;
    break;
  case GWEN_Crypt_PinEncoding_FPin2:
    rv=GWEN_Crypt__TransformFromFPIN2(buffer, bufLength, pinLength);
    break;
  default:
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Unhandled source encoding \"%s\"",
              GWEN_Crypt_PinEncoding_toString(peSrc));
    return GWEN_ERROR_INVALID;
  }
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  switch(peDst) {
  case GWEN_Crypt_PinEncoding_Bin:
    rv=GWEN_Crypt__TransformToBin(buffer, bufLength, pinLength);
    break;
  case GWEN_Crypt_PinEncoding_Bcd:
    rv=GWEN_Crypt__TransformToBCD(buffer, bufLength, pinLength);
    break;
  case GWEN_Crypt_PinEncoding_Ascii:
    rv=0;
    break;
  case GWEN_Crypt_PinEncoding_FPin2:
    rv=GWEN_Crypt__TransformToFPIN2(buffer, bufLength, pinLength);
    break;
  default:
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Unhandled destination encoding \"%s\"",
              GWEN_Crypt_PinEncoding_toString(peDst));
    return GWEN_ERROR_INVALID;
  }
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}




static int GWEN_Crypt__KeyDataFromText(const char *text,
                                       unsigned char *buffer,
                                       unsigned int bufLength) {
  GWEN_MDIGEST *md;
  int rv;

  assert(text);
  assert(buffer);
  assert(bufLength);

  switch(bufLength) {
  case 16:
    md=GWEN_MDigest_Md5_new();
    break;
  case 20:
    md=GWEN_MDigest_Rmd160_new();
    break;
  default:
    DBG_ERROR(GWEN_LOGDOMAIN, "Bad size (%d)", bufLength);
    return GWEN_ERROR_BAD_SIZE;
  }

  rv=GWEN_MDigest_Begin(md);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    return rv;
  }

  rv=GWEN_MDigest_Update(md,
                         (const uint8_t*)text,
                         strlen(text));
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    return rv;
  }

  rv=GWEN_MDigest_End(md);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    return rv;
  }

  /* get hash, copy it to given buffer */
  memmove(buffer, GWEN_MDigest_GetDigestPtr(md), bufLength);

  /* cleanup, return */
  GWEN_MDigest_free(md);
  return 0;
}



int GWEN_Crypt_KeyDataFromText(const char *text,
                               unsigned char *buffer,
                               unsigned int bufLength) {
  if (bufLength==24) {
    int rv;

    rv=GWEN_Crypt__KeyDataFromText(text, buffer, 16);
    if (rv)
      return rv;
    memmove(buffer+16, buffer, 8);
    return rv;
  }
  else
    return GWEN_Crypt__KeyDataFromText(text, buffer, bufLength);
}



void GWEN_Crypt_Random(int quality, uint8_t *buffer, uint32_t len) {
  enum gcry_random_level q;

  switch(quality) {
  case 0:
    q=GCRY_WEAK_RANDOM;
    break;
  case 1:
    q=GCRY_STRONG_RANDOM;
    break;
  case 2:
  default:
    q=GCRY_VERY_STRONG_RANDOM;
    break;
  }

  gcry_randomize(buffer, len, q);
}






