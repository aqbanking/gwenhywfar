/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sun Jan 25 2004
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


#ifndef GWEN_RINGBUFFER_H
#define GWEN_RINGBUFFER_H

#include <gwenhywfar/types.h>
#include <gwenhywfar/gwenhywfarapi.h>

#ifdef __cplusplus
extern "C" {
#endif


/** @defgroup MOD_RINGBUFFER Ringbuffer Management
 * @ingroup MOD_BASE
 *
 * @brief This file contains the definition of a GWEN_RINGBUFFER.
 *
 */
/*@{*/

typedef struct GWEN_RINGBUFFER GWEN_RINGBUFFER;


/** @name Constructor And Destructor
 *
 */
/*@{*/
/**
 * Creates a new ring buffer
 * @param size maximum size of the ring buffer
 */
GWEN_RINGBUFFER *GWEN_RingBuffer_new(unsigned int size);

/**
 * Destructor.
 */
void GWEN_RingBuffer_free(GWEN_RINGBUFFER *rb);


/** @name Reading And Writing
 *
 */
/*@{*/
/**
 * Writes the given bytes into the ring buffer.
 * @param rb ring buffer
 * @param buffer pointer to bytes to write
 * @param size pointer to a variable that contains the number of bytes
 * to write. Upon return this variable contains the number of bytes actually
 * copied.
 */
int Gwen_RingBuffer_WriteBytes(GWEN_RINGBUFFER *rb,
                               const char *buffer,
                               GWEN_TYPE_UINT32 *size);

/**
 * Writes a single byte to the ring buffer.
 */
int GWEN_RingBuffer_WriteByte(GWEN_RINGBUFFER *rb, char c);


/**
 * Read bytes from the ring buffer.
 * @param rb ring buffer
 * @param buffer pointer to the destination buffer
 * @param size pointer to a variable that contains the number of bytes
 * to read. Upon return this variable contains the number of bytes actually
 * copied.
 */
int Gwen_RingBuffer_ReadBytes(GWEN_RINGBUFFER *rb,
                              char *buffer,
                              GWEN_TYPE_UINT32 *size);

/**
 * Reads a single byte from the ring buffer.
 */
int GWEN_RingBuffer_ReadByte(GWEN_RINGBUFFER *rb);
/*@}*/


/** @name Informational Functions
 *
 */
/*@{*/
/**
 * Returns the number of bytes stored inside the ring buffer.
 */
GWEN_TYPE_UINT32 GWEN_RingBuffer_GetUsedBytes(const GWEN_RINGBUFFER *rb);

/**
 * Returns the number of bytes which still can be stored inside the ring
 * buffer.
 */
GWEN_TYPE_UINT32 GWEN_RingBuffer_GetBytesLeft(const GWEN_RINGBUFFER *rb);

/**
 * Returns the size of the ring buffer.
 */
GWEN_TYPE_UINT32 GWEN_RingBuffer_GetBufferSize(const GWEN_RINGBUFFER *rb);
/*@}*/



/** @name Statistical Functions
 *
 */
/*@{*/
/**
 * Returns the number of times the buffer was empty.
 */
GWEN_TYPE_UINT32 GWEN_RingBuffer_GetEmptyCounter(const GWEN_RINGBUFFER *rb);

void GWEN_RingBuffer_ResetEmptyCounter(GWEN_RINGBUFFER *rb);


/**
 * Returns the number of times the buffer was full.
 */
GWEN_TYPE_UINT32 GWEN_RingBuffer_GetFullCounter(const GWEN_RINGBUFFER *rb);

void GWEN_RingBuffer_ResetFullCounter(GWEN_RINGBUFFER *rb);


/**
 * Returns the number of bytes which have passed through this buffer (i.e.
 * bytes that have been written to <strong>and</strong> read from the buffer.
 */
GWEN_TYPE_UINT32 GWEN_RingBuffer_GetThroughput(GWEN_RINGBUFFER *rb);

/**
 * Resets the buffers throughput counter to zero.
 */
void GWEN_RingBuffer_ResetThroughput(GWEN_RINGBUFFER *rb);



/**
 * Returns the maximum number of bytes which has been stored in the buffer.
 */
GWEN_TYPE_UINT32 GWEN_RingBuffer_GetMaxUsedBytes(const GWEN_RINGBUFFER *rb);

/**
 * Resets the counter for the maximum number of bytes stored in the
 * buffer.
 */
void GWEN_RingBuffer_ResetMaxUsedBytes(GWEN_RINGBUFFER *rb);
/*@}*/ /* name */



/*@}*/ /* group */

#ifdef __cplusplus
}
#endif


#endif /* GWEN_RINGBUFFER_H */




