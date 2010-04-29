/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: crypttoken.h 1113 2007-01-10 09:14:16Z martin $
    begin       : Wed Mar 16 2005
    copyright   : (C) 2005 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_IOLAYER_MEMORY_H
#define GWEN_IOLAYER_MEMORY_H

#include <gwenhywfar/iolayer.h>
#include <gwenhywfar/buffer.h>


#ifdef __cplusplus
extern "C" {
#endif



/** @defgroup MOD_IOLAYER_MEMORY Memory-based IO Layer
 * @ingroup MOD_IOLAYER
 *
 * This module uses a GWEN_BUFFER object to read/write data..
 * It supports the following request types:
 * <ul>
 *   <li>@ref GWEN_Io_Request_TypeRead (reading from the file)</li>
 *   <li>@ref GWEN_Io_Request_TypeWrite (writing to the file)</li>
 *   <li>@ref GWEN_Io_Request_TypeDisconnect (closing the file, preventing any further access)</li>
 * </ul>
 * <p>
 * An io layer of this module is an endpoint since it has no base layer.
 * </p>
 * <p>
 * This io layer always handles request directly upon @ref GWEN_Io_Layer_AddRequest() so when that function
 * returns the given request is already finished.
 * </p>
 */
/*@{*/


/** @name Makro Defining the Type Name
 *
 */
/*@{*/
#define GWEN_IO_LAYER_MEMORY_TYPE "memory"
/*@}*/



/** @name Constructors
 *
 */
/*@{*/
/**
 * Creates a new IO layer using the given buffer.
 * @param buffer GWEN_BUFFER to be used
 */
GWENHYWFAR_API DEPRECATED GWEN_IO_LAYER *GWEN_Io_LayerMemory_new(GWEN_BUFFER *buffer);

/**
 * Creates a new IO layer using the given data. This function internally creates a GWEN_BUFFER and calls
 * @ref GWEN_Io_LayerMemory_new() with it. After that it sets the flag @ref GWEN_IO_LAYER_MEMORY_FLAGS_TAKEOVER to
 * make the new io layer take over the just created buffer.
 * Such an io layer can immediately by used to read data from.
 *
 * @param p pointer to the string from which the internal buffer is to filled.
 * @param size size of the string pointed to by p (use -1 to make this function calculate the size using strlen)
 */
GWENHYWFAR_API GWEN_IO_LAYER *GWEN_Io_LayerMemory_fromString(const uint8_t *p, int size);

/*@}*/



/** @name Informational Functions
 *
 */
/*@{*/

/**
 * Return a pointer to the internally used GWEN_BUFFER. Please note that if the flag
 * @ref GWEN_IO_LAYER_MEMORY_FLAGS_TAKEOVER is set when this io layer is free'd the GWEN_BUFFER will also be
 * free'd!
 */
GWENHYWFAR_API GWEN_BUFFER *GWEN_Io_LayerMemory_GetBuffer(const GWEN_IO_LAYER *io);

/*@}*/


/*@}*/

#ifdef __cplusplus
}
#endif

#endif




