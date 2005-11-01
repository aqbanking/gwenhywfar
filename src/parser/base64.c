/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
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

#include "base64.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>
#include <string.h>


static const char GWEN_Base64_Alphabet[]=
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";


int GWEN_Base64_Encode(const unsigned char *src, unsigned int size,
                       GWEN_BUFFER *dst,
                       unsigned int maxLineLength) {
  unsigned int by3;
  unsigned int i;
  unsigned int l;
  GWEN_TYPE_UINT32 triplet;
  unsigned char c1, c2, c3, c4;

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
    triplet=(src[0]<<16)+(src[1]<<8)+src[2];
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
        GWEN_Buffer_AppendByte(dst, '\n');
        l=0;
      }
      l+=4;
    }
    GWEN_Buffer_AppendByte(dst, GWEN_Base64_Alphabet[c1]);
    GWEN_Buffer_AppendByte(dst, GWEN_Base64_Alphabet[c2]);
    GWEN_Buffer_AppendByte(dst, GWEN_Base64_Alphabet[c3]);
    GWEN_Buffer_AppendByte(dst, GWEN_Base64_Alphabet[c4]);
  }
  if (maxLineLength) {
    if (l+4>=maxLineLength) {
      GWEN_Buffer_AppendByte(dst, '\n');
      l=0;
    }
  }

  /* then encode remainder */
  switch(size % 3) {
  case 0:
    /* no remainder */
    break;

  case 1:
    /* 1 remainder, leads to two trailing "=" */
    triplet=(src[0]<<4);
    c2=triplet & 0x3f;
    triplet>>=6;
    c1=triplet & 0x3f;
    GWEN_Buffer_AppendByte(dst, GWEN_Base64_Alphabet[c1]);
    GWEN_Buffer_AppendByte(dst, GWEN_Base64_Alphabet[c2]);
    GWEN_Buffer_AppendString(dst, "==");
    break;

  case 2:
    /* 2 remainders, leads to one trailing "=" */
    triplet=(src[0]<<10)+(src[1]<<2);
    c3=triplet & 0x3f;
    triplet>>=6;
    c2=triplet & 0x3f;
    triplet>>=6;
    c1=triplet & 0x3f;
    GWEN_Buffer_AppendByte(dst, GWEN_Base64_Alphabet[c1]);
    GWEN_Buffer_AppendByte(dst, GWEN_Base64_Alphabet[c2]);
    GWEN_Buffer_AppendByte(dst, GWEN_Base64_Alphabet[c3]);
    GWEN_Buffer_AppendByte(dst, '=');
    break;
  }

  return 0;
}



int GWEN_Base64_Decode(const unsigned char *src, unsigned int size,
                       GWEN_BUFFER *dst) {
  unsigned int i;
  const char *p = "0";
  GWEN_TYPE_UINT32 v;
  int lastWasEq;
  int sizeGiven;

  /* first decode full triplets */
  sizeGiven=(size!=0);
  lastWasEq=0;
  for (;;) {
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
        while (*src && *src!='=')
          src++;
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
      if (*src=='=')
        lastWasEq=1;
      v<<=6;
      v+=(p-GWEN_Base64_Alphabet) & 0x3f;
      src++;
    } /* for */

    /* now we have a triplet */
    if (sizeGiven) {
      switch(size) {
      case 1:
        GWEN_Buffer_AppendByte(dst, (v>>16) & 0xff);
        size--;
        break;
      case 2:
        GWEN_Buffer_AppendByte(dst, (v>>16) & 0xff);
        GWEN_Buffer_AppendByte(dst, (v>>8) & 0xff);
        size-=2;
        break;
      default:
        GWEN_Buffer_AppendByte(dst, (v>>16) & 0xff);
        GWEN_Buffer_AppendByte(dst, (v>>8) & 0xff);
        GWEN_Buffer_AppendByte(dst, v & 0xff);
        size-=3;
        break;
      } /* switch */
    }
    else {
      GWEN_Buffer_AppendByte(dst, (v>>16) & 0xff);
      GWEN_Buffer_AppendByte(dst, (v>>8) & 0xff);
      GWEN_Buffer_AppendByte(dst, v & 0xff);
    }
  } /* for full quadruplets */

  return 0;
}





