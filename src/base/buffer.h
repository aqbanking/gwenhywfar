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


#ifndef GWENHYFWAR_BUFFER_H
#define GWENHYFWAR_BUFFER_H

#include <gwenhyfwar/gwenhyfwarapi.h>



typedef struct GWEN_BUFFER GWEN_BUFFER;


/**
 * @param buffer if !=0, then the given buffer will be used. Otherwise
 * a new buffer will be allocated (with <i>size</i> bytes)
 * @param size real size of the buffer (if @<i>buffer</i> is null, then
 * this number of bytes will be allocated)
 * @param used number of bytes of the buffer actually used. This is
 * interesting when reading from a buffer
 * @param take if buffer!=0 then this function takes over the ownership
 * of the given buffer, if take is !=0
 */
GWEN_BUFFER *GWEN_Buffer_new(char *buffer,
                             unsigned int size,
                             unsigned int used,
                             int take);

void GWEN_Buffer_free(GWEN_BUFFER *bf);


/**
 * Returns the start of the buffer. You can use the function
 * @ref GWEN_Buffer_GetPos to navigate within the buffer.
 */
char *GWEN_Buffer_GetStart(GWEN_BUFFER *bf);


/**
 * Returns the size of the buffer (i.e. the number of bytes allocated).
 */
unsigned int GWEN_Buffer_GetSize(GWEN_BUFFER *bf);


/**
 * Returns the current position within the buffer. This pointer is adjusted
 * by the various read and write functions.
 */
unsigned int GWEN_Buffer_GetPos(GWEN_BUFFER *bf);

/**
 * @return 0 if ok, !=0 on error
 */
int GWEN_Buffer_SetPos(GWEN_BUFFER *bf, unsigned int i);

/**
 * @return 0 if ok, !=0 on error
 */
unsigned int GWEN_Buffer_GetUsedBytes(GWEN_BUFFER *bf);

/**
 * @return 0 if ok, !=0 on error
 */
int GWEN_Buffer_SetUsedBytes(GWEN_BUFFER *bf, unsigned int i);

/**
 * Copies the contents of the given buffer to this GWEN_BUFFER, if there is
 * enough room.
 * The position pointer is adjusted accordingly.
 * @return 0 if ok, !=0 on error
 */
int GWEN_Buffer_AppendBytes(GWEN_BUFFER *bf,
                            const char *buffer,
                            unsigned int size);

/**
 * Appends a single byte to this GWEN_BUFFER, if there is
 * enough room.
 * The position pointer is adjusted accordingly.
 * @return 0 if ok, !=0 on error
 */
int GWEN_Buffer_AppendByte(GWEN_BUFFER *bf, char c);


/**
 * Returns the byte from the current position.
 * The position pointer is adjusted accordingly.
 * @return -1 on error, read char otherwise (in low byte)
 */
int GWEN_Buffer_ReadByte(GWEN_BUFFER *bf);


int GWEN_Buffer_IncrementPos(GWEN_BUFFER *bf, unsigned int i);


int GWEN_Buffer_AppendBuffer(GWEN_BUFFER *bf,
                             GWEN_BUFFER *sf);

/**
 * Returns the number of unused bytes within the buffer.
 */
unsigned int GWEN_Buffer_RoomLeft(GWEN_BUFFER *bf);


/**
 * Returns a pointer to the current position within the buffer.
 */
char *GWEN_Buffer_GetPosPointer(GWEN_BUFFER *bf);

#endif






