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

/**
 * When reallocating the buffer a multiple of this value is used.
 * Needs to be aligned at 2^n
 */
#define GWEN_BUFFER_DYNAMIC_STEP 1024

#define GWEN_BUFFER_FLAGS_OWNED      0x0001
#define GWEN_BUFFER_FLAGS_OWN_SYNCIO 0x0002

#define GWEN_BUFFER_MODE_COPYMASK (\
  ~(GWEN_BUFFER_FLAGS_OWN_SYNCIO) \
  )


struct GWEN_BUFFER {
  char *realPtr;
  char *ptr;
  uint32_t pos;
  uint32_t bufferSize;
  uint32_t realBufferSize;
  uint32_t bytesUsed;
  uint32_t flags;
  uint32_t mode;
  uint32_t hardLimit;
  uint32_t step;
  uint32_t bookmarks[GWEN_BUFFER_MAX_BOOKMARKS];
  GWEN_SYNCIO *syncIo;
};



static void GWEN_Buffer_AdjustBookmarks(GWEN_BUFFER *bf,
					uint32_t pos,
					int offset);


static int GWEN_Buffer__FillBuffer(GWEN_BUFFER *bf);
static int GWEN_Buffer__FillBuffer_SyncIo(GWEN_BUFFER *bf);


#endif



