/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Fri Sep 12 2003
    copyright   : (C) 2003 by Martin Preuss
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


#ifndef GWENHYWFAR_BUFFER_P_H
#define GWENHYWFAR_BUFFER_P_H

#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/buffer.h>

#define GWEN_BUFFER_DEFAULT_HARDLIMIT 65535

/**
 * When reallocating the buffer a multiple of this value is used.
 * Needs to be aligned at 2^n
 */
#define GWEN_BUFFER_DYNAMIC_STEP 1024

#define GWEN_BUFFER_FLAGS_OWNED   0x0001


struct GWEN_BUFFER {
  char *realPtr;
  char *ptr;
  GWEN_TYPE_UINT32 pos;
  GWEN_TYPE_UINT32 bufferSize;
  GWEN_TYPE_UINT32 realBufferSize;
  GWEN_TYPE_UINT32 bytesUsed;
  GWEN_TYPE_UINT32 flags;
  GWEN_TYPE_UINT32 mode;
  GWEN_TYPE_UINT32 hardLimit;
  GWEN_TYPE_UINT32 step;
  GWEN_TYPE_UINT32 bookmarks[GWEN_BUFFER_MAX_BOOKMARKS];
};


void GWEN_Buffer_AdjustBookmarks(GWEN_BUFFER *bf,
                                 GWEN_TYPE_UINT32 pos,
                                 int offset);



#endif



