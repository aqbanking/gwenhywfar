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


#ifndef GWEN_IOLAYER_FILE_H
#define GWEN_IOLAYER_FILE_H

#include <gwenhywfar/iolayer.h>

#ifdef __cplusplus
extern "C" {
#endif



/** @defgroup MOD_IOLAYER_FILE File-based IO Layer
 * @ingroup MOD_IOLAYER
 *
 * This module provides access to already open files.
 * It supports the following request types:
 * <ul>
 *   <li>@ref GWEN_Io_Request_TypeRead (reading from the file)</li>
 *   <li>@ref GWEN_Io_Request_TypeWrite (writing to the file)</li>
 *   <li>@ref GWEN_Io_Request_TypeDisconnect (closing the file, preventing any further access)</li>
 * </ul>
 * An io layer of this module is an endpoint since it has no base layer.
 */
/*@{*/


/** @name Makro Defining the Type Name
 *
 */
/*@{*/
#define GWEN_IO_LAYER_FILE_TYPE "file"
/*@}*/



/** @name Constructor
 *
 */
/*@{*/
/**
 * Creates a new IO layer using the given file descriptors.
 * Given file descriptors will be modified using fcntl(2) to set the O_NONBLOCKING flag. Upon receiption of a
 * disconnect request the file status flags of the descriptors are reset to the values from before this function was
 * called.
 * @param fdRead descriptor for reading (or -1 if reading is not wanted)
 * @param fdWrite descriptor for writing (or -1 if writing is not wanted)
 */
GWENHYWFAR_API GWEN_IO_LAYER *GWEN_Io_LayerFile_new(int fdRead, int fdWrite);
/*@}*/



/** @name Informational Functions
 *
 */
/*@{*/
/**
 * Return the file descriptor used by this io layer for reading. After receiption of a disconnect request the file
 * descriptor is set to -1 unless the flag @ref GWEN_IO_LAYER_FILE_FLAGS_DONTCLOSE is set.
 */
GWENHYWFAR_API int GWEN_Io_LayerFile_GetReadFileDescriptor(const GWEN_IO_LAYER *io);

/**
 * Return the file descriptor used by this io layer for writing. After receiption of a disconnect request the file
 * descriptor is set to -1 unless the flag @ref GWEN_IO_LAYER_FILE_FLAGS_DONTCLOSE is set.
 */
GWENHYWFAR_API int GWEN_Io_LayerFile_GetWriteFileDescriptor(const GWEN_IO_LAYER *io);



/*@}*/


/*@}*/

#ifdef __cplusplus
}
#endif

#endif




