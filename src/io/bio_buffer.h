/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Fri Feb 07 2003
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


#ifndef GWENHYWFAR_BIO_BUFFER_H
#define GWENHYWFAR_BIO_BUFFER_H

#include <gwenhywfar/bufferedio.h>
#include <gwenhywfar/buffer.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a buffered IO context using a GWEN_BUFFER.
 * This function takes over ownership for that buffer !
 * This context can be free'd using @ref GWEN_BufferedIO_free.
 */
GWENHYWFAR_API
  GWEN_BUFFEREDIO *GWEN_BufferedIO_Buffer_new(GWEN_BUFFER *buffer);


/**
 * Create a buffered IO context using a GWEN_BUFFER.
 * This function does NOT take over ownership for that buffer !
 * This context can be free'd using @ref GWEN_BufferedIO_free.
 */
GWENHYWFAR_API
  GWEN_BUFFEREDIO *GWEN_BufferedIO_Buffer2_new(GWEN_BUFFER *buffer,
                                               int take);

/**
 * Create a buffered IO context using a GWEN_BUFFER.
 * This context can be free'd using @ref GWEN_BufferedIO_free.
 */
GWENHYWFAR_API
  GWEN_BUFFEREDIO *GWEN_BufferedIO_Buffer_fromString(const char *s, int len);

#ifdef __cplusplus
}
#endif


#endif /* GWENHYWFAR_BIO_BUFFER_H */





