/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sun Dec 04 2004
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

#include "debug_p.h"

#include <stdarg.h>


GWEN_TYPE_UINT32 GWEN_Debug_PrintDec(char *buffer,
                                     GWEN_TYPE_UINT32 size,
                                     GWEN_TYPE_UINT32 num,
                                     int leadingZero,
                                     GWEN_TYPE_UINT32 length) {
  GWEN_TYPE_UINT32 i;
  GWEN_TYPE_UINT32 j;
  GWEN_TYPE_UINT32 k;
  char numbuf[16];
  int numOr;

  /* first convert number */
  numOr=0;
  i=0;
  j=1000000000;

  while(j) {
    k=num/j;
    numOr|=k;
    if (numOr || leadingZero || j==1) {
      numbuf[i]=k+'0';
      i++;
    }
    num-=(k*j);
    j/=10;
  } /* while j */

  j=0;
  if (length) {
    if (i>length)
      i=length;

    /* fill left up to length-(sizeof number) */
    k=length-i;
    while(k) {
      if (j<size) {
        if (leadingZero)
          buffer[j]='0';
        else
          buffer[j]=' ';
      }
      j++;
      k--;
    } /* while k */
  } /* if length */

  /* copy number */
  for (k=0; k<i; k++) {
    if (j<size)
      buffer[j]=numbuf[k];
    j++;
  } /* while i */

  /* append trailing 0 */
  if (j<size)
    buffer[j]=0;
  j++;
  /* return length (or possible length) */
  return j;
}



GWEN_TYPE_UINT32 GWEN_Debug_PrintHex(char *buffer,
                                 GWEN_TYPE_UINT32 size,
                                 GWEN_TYPE_UINT32 num,
                                 int leadingZero,
                                 int up,
                                 GWEN_TYPE_UINT32 length) {
  GWEN_TYPE_UINT32 i;
  GWEN_TYPE_UINT32 j;
  GWEN_TYPE_UINT32 k;
  char numbuf[16];
  int numOr;

  /* first convert number */
  numOr=0;
  i=0;
  j=8;

  while(j) {
    k=(num>>((j-1)*4))&0xf;
    numOr|=k;
    if (numOr || leadingZero || j==1) {
      if (k>9) {
        if (up)
          numbuf[i]=k+'0'+7;
        else
          numbuf[i]=k+'0'+7+32;
      }
      else
        numbuf[i]=k+'0';
      i++;
    }
    j--;
  } /* while j */

  j=0;
  if (length) {
    if (i>length)
      i=length;

    /* fill left up to length-(sizeof number) */
    k=length-i;
    while(k) {
      if (j<size) {
        if (leadingZero)
          buffer[j]='0';
        else
          buffer[j]=' ';
      }
      j++;
      k--;
    } /* while k */
  } /* if length */

  /* copy number */
  for (k=0; k<i; k++) {
    if (j<size)
      buffer[j]=numbuf[k];
    j++;
  } /* while i */

  /* append trailing 0 */
  if (j<size)
    buffer[j]=0;
  j++;
  /* return length (or possible length) */
  return j;
}





GWEN_TYPE_UINT32 GWEN_Debug_Snprintf(char *buffer,
                                 GWEN_TYPE_UINT32 size,
                                 const char *fmt, ...) {
  va_list arguments;
  GWEN_TYPE_UINT32 i;

  i=0;
  va_start(arguments, fmt);
  while(*fmt) {
    if (*fmt=='%') {
      fmt++;
      if (*fmt=='%') {
        /* write character '%' */
        if (i<size)
          buffer[i]='%';
        i++;
      }
      else {
        GWEN_TYPE_UINT32 length;
        int leadingZero;

        leadingZero=0;
        length=0;

        /* read length */
        if ((*fmt)>='0' && (*fmt)<='9') {
          /* read number */
          if (*fmt=='0') {
            leadingZero=1;
          }
          while ((*fmt)>='0' && (*fmt)<='9') {
            length*=10;
            length+=*fmt-'0';
            fmt++;
          } /* while */
        }

        /* read type */
        switch(*fmt) {
        /* decimal integer */
        case 'c':
        case 'd': {
          int p;

          p=va_arg(arguments, int);
          if (p<0) {
            if (i<size)
              buffer[i]='-';
            i++;
            p=-p;
          }
          i+=GWEN_Debug_PrintDec(buffer+i,
                                 size-i,
                                 p,
                                 leadingZero,
                                 length)-1;
          break;
        }

        /* hexadecimal integer */
        case 'x': {
          unsigned int p;

          p=va_arg(arguments, unsigned int);
          i+=GWEN_Debug_PrintHex(buffer+i,
                                 size-i,
                                 p,
                                 leadingZero,
                                 0,
                                 length)-1;
          break;
        }

        /* hexadecimal integer */
        case 'X': {
          unsigned int p;

          p=va_arg(arguments, unsigned int);
          i+=GWEN_Debug_PrintHex(buffer+i,
                                 size-i,
                                 p,
                                 leadingZero,
                                 1,
                                 length)-1;
          break;
        }

        case 's': {
          const char *p;

          p=va_arg(arguments, const char*);
          if (!p)
            p="(null)";
          while(*p) {
            if (i<size)
              buffer[i]=*p;
            i++;
            p++;
          } /* while */
          break;
        }

        default:
          break;
        } /* switch */
      }
    }
    else {
      if (i<size)
        buffer[i]=*fmt;
      i++;
    }
    fmt++;
  } /* while */

  /* add trailing 0 */
  if (i<size)
    buffer[i]=0;
  i++;
  va_end(arguments);
  return i;
}




void GWEN_Debug_Error_Log(GWEN_LOGGER *lg, const char *fmt, ...) {
  va_list arguments;

  va_start(arguments, fmt);

}



void GWEN_Debug_Error_Err(GWEN_LOGGER *lg, GWEN_ERRORCODE err) {
}


