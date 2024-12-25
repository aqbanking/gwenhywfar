/***************************************************************************
    begin       : Tuet Apr 26 2016
    copyright   : (C) 2016 by Martin Preuss
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

#ifndef GWENHYWFAR_ENDIANFNS_H
#define GWENHYWFAR_ENDIANFNS_H

#include <gwenhywfar/gwenhywfarapi.h>


#if GWENHYWFAR_SYS_IS_WINDOWS
/* assume little endian for now (is there any big endian Windows system??) */
#  define GWEN_ENDIAN_LE16TOH(x) (x)
#  define GWEN_ENDIAN_HTOLE16(x) (x)

#  define GWEN_ENDIAN_LE32TOH(x) (x)
#  define GWEN_ENDIAN_HTOLE32(x) (x)

#  define GWEN_ENDIAN_LE64TOH(x) (x)
#  define GWEN_ENDIAN_HTOLE64(x) (x)

#elif defined(__APPLE__)
/* inspired by https://stackoverflow.com/questions/20813028/endian-h-not-found-on-mac-osx */
#include <libkern/OSByteOrder.h>

#define GWEN_ENDIAN_HTOLE16(x) OSSwapHostToLittleInt16(x)
#define GWEN_ENDIAN_LE16TOH(x) OSSwapLittleToHostInt16(x)

#define GWEN_ENDIAN_HTOLE32(x) OSSwapHostToLittleInt32(x)
#define GWEN_ENDIAN_LE32TOH(x) OSSwapLittleToHostInt32(x)

#define GWEN_ENDIAN_HTOLE64(x) OSSwapHostToLittleInt64(x)
#define GWEN_ENDIAN_LE64TOH(x) OSSwapLittleToHostInt64(x)

#define GWEN_ENDIAN_HTOBE16(x) OSSwapHostToBigInt16(x)
#define GWEN_ENDIAN_BE16TOH(x) OSSwapBigToHostInt16(x)

#define GWEN_ENDIAN_HTOBE32(x) OSSwapHostToBigInt32(x)
#define GWEN_ENDIAN_BE32TOH(x) OSSwapBigToHostInt32(x)

#define GWEN_ENDIAN_HTOBE64(x) OSSwapHostToBigInt64(x)
#define GWEN_ENDIAN_BE64TOH(x) OSSwapBigToHostInt64(x)

#else
/* for Linux and others use definitions from endian.h */
#  include <endian.h>

#  define GWEN_ENDIAN_LE16TOH(x) le16toh(x)
#  define GWEN_ENDIAN_HTOLE16(x) htole16(x)

#  define GWEN_ENDIAN_LE32TOH(x) le32toh(x)
#  define GWEN_ENDIAN_HTOLE32(x) htole32(x)

#  define GWEN_ENDIAN_LE64TOH(x) le64toh(x)
#  define GWEN_ENDIAN_HTOLE64(x) htole64(x)
#endif




#endif
