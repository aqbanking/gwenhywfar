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


#ifndef GWEN_IOLAYER_BUFFERED_H
#define GWEN_IOLAYER_BUFFERED_H

#include <gwenhywfar/iolayer.h>
#include <gwenhywfar/buffer.h>

#ifdef __cplusplus
extern "C" {
#endif


/** @defgroup MOD_IOLAYER_BUFFERED
 * @ingroup MOD_IOLAYER
 *
 * This module provides access to already open files.
 * It supports the following request types:
 * <ul>
 *   <li>@ref GWEN_Io_Request_TypeRead (reading from the file)</li>
 *   <li>@ref GWEN_Io_Request_TypeWrite (writing to the file)</li>
 *   <li>@ref GWEN_Io_Request_TypeDisconnect (closing the file, preventing any further access)</li>
 * </ul>
 */
/*@{*/


/** @name Makro Defining the Type Name
 *
 */
/*@{*/
#define GWEN_IO_LAYER_BUFFERED_TYPE "buffered"
/*@}*/



/** @name Flags For This Module
 *
 * You can set these flags using @ref GWEN_Io_Layer_SetFlags() or @ref GWEN_Io_Layer_AddFlags().
 */
/*@{*/
/**
 * If this flag is set then lines are expected to end with CR/LF as opposed to Linux mode where a simple LF
 * suffices to mark the end of a line.
 */
#define GWEN_IO_LAYER_BUFFERED_FLAGS_DOSMODE  0x00000001

/*@}*/


/** @name Request Flags For This Module
 *
 * You can set these flags on IO requests using @ref GWEN_Io_Request_SetFlags() or @ref GWEN_Io_Request_AddFlags().
 */
/*@{*/
/**
 * If this flag is set then raw data is to be transfered so the data will not be modified by this io layer (e.g.
 * no CR/LF will be appended or removed).
 */
#define GWEN_IO_REQUEST_BUFFERED_FLAGS_RAW    0x00000001

/**
 * If this flag is set then the internal read pointer will not be advanced. This can be used to peek for available
 * data without actually removing it from the internal buffer. Subsequent read requests with this flag set will
 * return the same data.
 */
#define GWEN_IO_REQUEST_BUFFERED_FLAGS_PEEK           0x00000002

#define GWEN_IO_REQUEST_BUFFERED_FLAGS_UNTILEMPTYLINE 0x00000004

/*@}*/


/** @name Constructor
 *
 */
/*@{*/
/**
 * Creates a new IO layer using the given base layer.
 */
GWENHYWFAR_API GWEN_IO_LAYER *GWEN_Io_LayerBuffered_new(GWEN_IO_LAYER *baseLayer);
/*@}*/



/** @name Informational Functions
 *
 */
/*@{*/
GWENHYWFAR_API uint32_t GWEN_Io_LayerBuffered_GetReadLineCount(const GWEN_IO_LAYER *io);
GWENHYWFAR_API uint32_t GWEN_Io_LayerBuffered_GetReadLinePos(const GWEN_IO_LAYER *io);
GWENHYWFAR_API void GWEN_Io_LayerBuffered_ResetLinePosAndCounter(GWEN_IO_LAYER *io);

/*@}*/



/** @name Convenience Functions
 *
 */
/*@{*/

/**
 * This function reads bytes from the base layer until a Line Feed character (#10) is found. It then returns
 * these bytes (excluding the #10) in the given buffer. Carriage Return characters (#13) are ignored in any
 * case.
 */
GWENHYWFAR_API int GWEN_Io_LayerBuffered_ReadLineToBuffer(GWEN_IO_LAYER *io, GWEN_BUFFER *fbuf,
							  uint32_t guiid, int msecs);

/**
 * This function writes a line of text to the base layer and adds a CR/LF sequence when in DOS mode or a LF character
 * in non-DOS mode.
 */
GWENHYWFAR_API int GWEN_Io_LayerBuffered_WriteLine(GWEN_IO_LAYER *io,
						   const char *buffer, int len,
						   int flush, uint32_t guiid, int msecs);


/*@}*/



/*@}*/

#ifdef __cplusplus
}
#endif

#endif




