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


#ifndef GWEN_IOLAYER_SOCKET_H
#define GWEN_IOLAYER_SOCKET_H

#include <gwenhywfar/iolayer.h>
#include <gwenhywfar/inetsocket.h>
#include <gwenhywfar/inetaddr.h>

#ifdef __cplusplus
extern "C" {
#endif



/** @defgroup MOD_IOLAYER_SOCKET Socket-based IO Layer
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
#define GWEN_IO_LAYER_SOCKET_TYPE "socket"
/*@}*/


/** @name Constructor
 *
 */
/*@{*/
/**
 * Creates a new IO layer using the given socket.
 * @param sk socket to use for reading and writing)
 */
GWENHYWFAR_API GWEN_IO_LAYER *GWEN_Io_LayerSocket_new(GWEN_SOCKET *sk);
/*@}*/



/** @name Informational Functions
 *
 */
/*@{*/
/**
 * Return the file descriptor used by this io layer for reading. After receiption of a disconnect request the file
 * descriptor is set to -1 unless the flag @ref GWEN_IO_LAYER_SOCKET_FLAGS_DONTCLOSE is set.
 */
GWENHYWFAR_API GWEN_SOCKET *GWEN_Io_LayerSocket_GetSocket(const GWEN_IO_LAYER *io);

GWENHYWFAR_API GWEN_INETADDRESS *GWEN_Io_LayerSocket_GetLocalAddr(const GWEN_IO_LAYER *io);
GWENHYWFAR_API void GWEN_Io_LayerSocket_SetLocalAddr(GWEN_IO_LAYER *io, const GWEN_INETADDRESS *addr);

GWENHYWFAR_API GWEN_INETADDRESS *GWEN_Io_LayerSocket_GetPeerAddr(const GWEN_IO_LAYER *io);
GWENHYWFAR_API void GWEN_Io_LayerSocket_SetPeerAddr(GWEN_IO_LAYER *io, const GWEN_INETADDRESS *addr);

/*@}*/


/*@}*/

#ifdef __cplusplus
}
#endif

#endif




