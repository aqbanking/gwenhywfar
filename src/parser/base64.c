/***************************************************************************
    begin       : Thu May 06 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston,                 *
 *   MA  02111-1307  USA                                                   *
 *                                                                         *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define DISABLE_DEBUGLOG


#include "base64.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>
#include <string.h>


static const char GWEN_Base64_Alphabet[]=
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";


int GWEN_Base64_Encode(const unsigned char *src, unsigned int size,
                       GWEN_BUFFER *dst,
                       unsigned int maxLineLength)
{
  unsigned int by3;
  unsigned int i;
  unsigned int l;
  uint32_t triplet;
  unsigned char c1, c2, c3, c4;

  if (!dst || (size && !src)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid buffer for Base64 encode");
    return -1;
  }

  if (maxLineLength) {
    if (maxLineLength<4) {
      DBG_ERROR(GWEN_LOGDOMAIN, "I need at least 4 bytes per line");
      return -1;
    }
  }

  by3=size/3;
  /* first encode full triplets */
  l=0;
  for (i=0; i<by3; i++) {
    triplet=(((uint32_t)src[0])<<16)+(((uint32_t)src[1])<<8)+(uint32_t)src[2];
    src+=3;
    c4=triplet & 0x3f;
    triplet>>=6;
    c3=triplet & 0x3f;
    triplet>>=6;
    c2=triplet & 0x3f;
    triplet>>=6;
    c1=triplet & 0x3f;
    if (maxLineLength) {
      if (l+4>maxLineLength) {
        if (GWEN_Buffer_AppendByte(dst, '\n')<0)
          return -1;
        l=0;
      }
      l+=4;
    }
    if (GWEN_Buffer_AppendByte(dst, GWEN_Base64_Alphabet[c1])<0 ||
        GWEN_Buffer_AppendByte(dst, GWEN_Base64_Alphabet[c2])<0 ||
        GWEN_Buffer_AppendByte(dst, GWEN_Base64_Alphabet[c3])<0 ||
        GWEN_Buffer_AppendByte(dst, GWEN_Base64_Alphabet[c4])<0)
      return -1;
  }
  if (maxLineLength) {
    if (l+4>=maxLineLength) {
      if (GWEN_Buffer_AppendByte(dst, '\n')<0)
        return -1;
      l=0;
    }
  }

  /* then encode remainder */
  switch (size % 3) {
  case 0:
    /* no remainder */
    break;

  case 1:
    /* 1 remainder, leads to two trailing "=" */
    triplet=((uint32_t)src[0])<<4;
    c2=triplet & 0x3f;
    triplet>>=6;
    c1=triplet & 0x3f;
    if (GWEN_Buffer_AppendByte(dst, GWEN_Base64_Alphabet[c1])<0 ||
        GWEN_Buffer_AppendByte(dst, GWEN_Base64_Alphabet[c2])<0 ||
        GWEN_Buffer_AppendString(dst, "==")<0)
      return -1;
    break;

  case 2:
    /* 2 remainders, leads to one trailing "=" */
    triplet=(((uint32_t)src[0])<<10)+(((uint32_t)src[1])<<2);
    c3=triplet & 0x3f;
    triplet>>=6;
    c2=triplet & 0x3f;
    triplet>>=6;
    c1=triplet & 0x3f;
    if (GWEN_Buffer_AppendByte(dst, GWEN_Base64_Alphabet[c1])<0 ||
        GWEN_Buffer_AppendByte(dst, GWEN_Base64_Alphabet[c2])<0 ||
        GWEN_Buffer_AppendByte(dst, GWEN_Base64_Alphabet[c3])<0 ||
        GWEN_Buffer_AppendByte(dst, '=')<0)
      return -1;
    break;

  default:
    break;
  }

  return 0;
}



int GWEN_Base64_Decode(const unsigned char *src, unsigned int size,
                       GWEN_BUFFER *dst)
{
  unsigned int i;
  const char *p="0";
  uint32_t v;
  int lastWasEq;
  int sizeGiven;

  if (!src || !dst) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid buffer for Base64 decode");
    return -1;
  }

  /* first decode full triplets */
  sizeGiven=(size!=0);
  lastWasEq=0;
  for (;;) {
    int bytes;

    if ((sizeGiven && size==0) || lastWasEq || !*src)
      break;
    v=0;

    while (*src && ((p=strchr(GWEN_Base64_Alphabet, *src))==0))
      src++;
    if (!*src)
      break;
    for (i=0; i<4; i++) {
      /* get next valid character */
      if (lastWasEq) {
        while (*src && (unsigned char)*src<=32)
          src++;
        if (*src && *src!='=') {
          DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected character after '=' padding");
          return -1;
        }
        p=GWEN_Base64_Alphabet+64;
      }
      else {
        while (*src && ((p=strchr(GWEN_Base64_Alphabet, *src))==0))
          src++;
      }
      if (!*src) {
        if (i==0 && !sizeGiven) {
          return 0;
        }
        else {
          DBG_ERROR(GWEN_LOGDOMAIN, "Premature end reached (%d)", i);
          return -1;
        }
      }
      if (*src=='=') {
        if (i<2) {
          DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected '=' padding at index %d", i);
          return -1;
        }
        lastWasEq++;
      }
      v<<=6;
      v+=(uint32_t)(p-GWEN_Base64_Alphabet) & 0x3fU;
      src++;
    } /* for */

    /* now we have a triplet */
    bytes=(24-(lastWasEq*6))/8;
    if (sizeGiven) {
      if ((unsigned int)bytes>size)
        bytes=(int)size;
      size-=(unsigned int)bytes;
    }
    if (bytes>0) {
      if (GWEN_Buffer_AppendByte(dst, (v>>16) & 0xff)<0)
        return -1;
      if (bytes>1) {
        if (GWEN_Buffer_AppendByte(dst, (v>>8) & 0xff)<0)
          return -1;
        if (bytes>2) {
          if (GWEN_Buffer_AppendByte(dst, v & 0xff)<0)
            return -1;
        }
      }
    }
  } /* for full quadruplets */

  return 0;
}





