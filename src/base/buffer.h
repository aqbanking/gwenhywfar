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


#ifndef GWENHYWFAR_BUFFER_H
#define GWENHYWFAR_BUFFER_H

#include <gwenhywfar/gwenhywfarapi.h>

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup MOD_BUFFER Buffer Management
 * @ingroup MOD_BASE
 *
 * @brief This file contains the definition of a GWEN_DB database.
 *
 */
/*@{*/

#define GWEN_BUFFER_MAX_BOOKMARKS 4

#define GWEN_BUFFER_MODE_DYNAMIC 0x0001

#define GWEN_BUFFER_MODE_DEFAULT \
  GWEN_BUFFER_MODE_DYNAMIC


typedef struct GWEN_BUFFER GWEN_BUFFER;


/**
 * @param buffer if !=0, then the given buffer will be used. Otherwise
 * a new buffer will be allocated (with <i>size</i> bytes)
 * @param size real size of the buffer (if <i>buffer</i> is null, then
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


GWEN_BUFFER *GWEN_Buffer_dup(GWEN_BUFFER *bf);


/**
 * Returns the current mode of the buffer (such as GWEN_BUFFER_MODE_DYNAMIC).
 */
unsigned int GWEN_Buffer_GetMode(GWEN_BUFFER *bf);

/**
 * Changes the current mode of the buffer (such as GWEN_BUFFER_MODE_DYNAMIC).
 */
void GWEN_Buffer_SetMode(GWEN_BUFFER *bf, unsigned int mode);

/**
 * Returns the hard limit. This is the maximum size of a GWEN_BUFFER in
 * dynamic mode.
 */
unsigned int GWEN_Buffer_GetHardLimit(GWEN_BUFFER *bf);

/**
 * Changes the hard limit. This is the maximum size of a GWEN_BUFFER in
 * dynamic mode.
 */
void GWEN_Buffer_SetHardLimit(GWEN_BUFFER *bf, unsigned int l);


/**
 * In dynamic mode, whenever there is new data to allocate then this value
 * specifies how much data to allocate in addition.
 * The allocated data in total for this buffer will be aligned to this value.
 */
unsigned int GWEN_Buffer_GetStep(GWEN_BUFFER *bf);

/**
 * In dynamic mode, whenever there is new data to allocate then this value
 * specifies how much data to allocate in addition.
 * The allocated data in total for this buffer will be aligned to this value.
 * 1024 is a reasonable value. This value NEEDS to be aligned 2^n (i.e.
 * only ONE bit must be set !)
 */
void GWEN_Buffer_SetStep(GWEN_BUFFER *bf, unsigned int step);


/**
 * Reserves the given amount of bytes at the beginning of the buffer.
 * Please note that this most likely results in a shift of the current
 * position inside the buffer, so after this call all pointers obtained
 * from this module (e.g. via @ref GWEN_Buffer_GetStart) are invalid !
 * You can use this function to save some memory copy actions when
 * inserting bytes at the beginning of the buffer.
 */
int GWEN_Buffer_ReserveBytes(GWEN_BUFFER *bf, unsigned int res);


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
 */
unsigned int GWEN_Buffer_GetUsedBytes(GWEN_BUFFER *bf);


/**
 * Returns the given bookmark
 */
unsigned int GWEN_Buffer_GetBookmark(GWEN_BUFFER *bf, unsigned int idx);


/**
 * Set a bookmark. These bookmarks are not used by the GWEN_BUFFER functions,
 * but may be usefull for an application.
 */
void GWEN_Buffer_SetBookmark(GWEN_BUFFER *bf, unsigned int idx,
                             unsigned int v);


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
 * Inserts multiple bytes at the current position.
 * If the current position is 0 and there is reserved space at the beginning
 * of the buffer then that space will be used.
 * Otherwise the data at the current position will be moved out of the way
 * and the new bytes inserted.
 * The position pointer will not be altered, but all pointers obtained from
 * this module (e.g. via @ref GWEN_Buffer_GetStart) become invalid !
 */
int GWEN_Buffer_InsertBytes(GWEN_BUFFER *bf,
                            const char *buffer,
                            unsigned int size);

/**
 * Inserts a byte at the current position.
 * If the current position is 0 and there is reserved space at the beginning
 * of the buffer then that space will be used.
 * Otherwise the data at the current position will be moved out of the way
 * and the new byte inserted.
 * The position pointer will not be altered, but all pointers obtained from
 * this module (e.g. via @ref GWEN_Buffer_GetStart) become invalid !
 */
int GWEN_Buffer_InsertByte(GWEN_BUFFER *bf, char c);

/**
 * Returns the byte from the current position.
 * The position pointer is adjusted accordingly.
 * @return -1 on error, read char otherwise (in low byte)
 */
int GWEN_Buffer_ReadByte(GWEN_BUFFER *bf);


/**
 * Returns the bytes from the current position.
 * The position pointer is adjusted accordingly.
 * @return -1 on error, 0 if ok
 */
int GWEN_Buffer_ReadBytes(GWEN_BUFFER *bf,
                          char *buffer,
                          unsigned int *size);


/**
 * Returns the byte from the current position without changing the
 * position pointer. So multiple calls to this function will result
 * in returning the same character.
 * @return -1 on error, read char otherwise (in low byte)
 */
int GWEN_Buffer_PeekByte(GWEN_BUFFER *bf);


int GWEN_Buffer_IncrementPos(GWEN_BUFFER *bf, unsigned int i);
int GWEN_Buffer_DecrementPos(GWEN_BUFFER *bf, unsigned int i);
int GWEN_Buffer_AdjustUsedBytes(GWEN_BUFFER *bf);


int GWEN_Buffer_InsertBuffer(GWEN_BUFFER *bf,
                             GWEN_BUFFER *sf);

int GWEN_Buffer_AppendBuffer(GWEN_BUFFER *bf,
                             GWEN_BUFFER *sf);

/**
 * Returns the number of unused bytes within the buffer.
 */
unsigned int GWEN_Buffer_RoomLeft(GWEN_BUFFER *bf);


/**
 * Returns the number of bytes from pos to the end of the used area.
 */
unsigned int GWEN_Buffer_BytesLeft(GWEN_BUFFER *bf);


/**
 * Returns a pointer to the current position within the buffer.
 */
char *GWEN_Buffer_GetPosPointer(GWEN_BUFFER *bf);


/**
 * Resets the position pointer and the byte counter.
 */
void GWEN_Buffer_Reset(GWEN_BUFFER *bf);

/**
 * Resets the pos pointer
 */
void GWEN_Buffer_Rewind(GWEN_BUFFER *bf);


/**
 * Make sure that the buffer has enough room for the given bytes.
 */
int GWEN_Buffer_AllocRoom(GWEN_BUFFER *bf, unsigned int size);


/* crops the buffer to the specified bytes */
int GWEN_Buffer_Crop(GWEN_BUFFER *bf,
                     unsigned int pos,
                     unsigned int l);


void GWEN_Buffer_Dump(GWEN_BUFFER *bf, FILE *f, unsigned insert);


/*@}*/

#ifdef __cplusplus
}
#endif

#endif






