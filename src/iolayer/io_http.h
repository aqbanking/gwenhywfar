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


#ifndef GWEN_IOLAYER_HTTP_H
#define GWEN_IOLAYER_HTTP_H

#include <gwenhywfar/iolayer.h>
#include <gwenhywfar/db.h>

#ifdef __cplusplus
extern "C" {
#endif


/** @defgroup MOD_IOLAYER_HTTP
 * @ingroup MOD_IOLAYER
 *
 * This module implements the HTTP protocol over a base layer created with @ref GWEN_Io_LayerBuffered_new.
 * It supports the following request types:
 * <ul>
 *   <li>@ref GWEN_Io_Request_TypeRead (reading from the file)</li>
 *   <li>@ref GWEN_Io_Request_TypeWrite (writing to the file)</li>
 *   <li>@ref GWEN_Io_Request_TypeConnect (closing the file, preventing any further access)</li>
 *   <li>@ref GWEN_Io_Request_TypeDisconnect (closing the file, preventing any further access)</li>
 * </ul>
 * <p>
 * </p>
 * <p>
 * There is special handling of the flag @ref GWEN_IO_REQUEST_FLAG_PACKETBEGIN.
 * </p>
 * <p>
 * If a new HTTP request is to be read then this flag must be set in the first read request regarding that
 * HTTP message. All remaining read requests for this particular HTTP message must have this flag cleared.
 * This allows this layer to stay in sync with incoming data on one side and the user requests on the other
 * side.
 * </p>
 * <p>
 * If an incoming HTTP message has been fully read then this layer will set the flag
 * @ref GWEN_IO_REQUEST_FLAGS_PACKETEND on the user's read request.
 * </p>
 * <p>
 * If a write request has the flag @ref GWEN_IO_REQUEST_FLAG_PACKETBEGIN set then a new HTTP message is
 * started. Therefore this layer will write the command/status line followed by a header. If the write
 * request contained data this will also be sent.
 * </p>
 * <p>
 * If a write request does not have that flag set then it is assumed that an already started HTTP message
 * is to be continued. This layer wll monitor the amount of the data written for the HTTP body and will stop
 * sending data when the number of bytes written equals that given for the header @b Content-length.
 * </p>
 * <p>
 * If Content-length has the value -1 (indicating that it is unknown) then this kind of monitoring is
 * disabled (so this layer will send any data written to it).
 * </p>
 */
/*@{*/



/** @name Makro Defining the Type Name
 *
 */
/*@{*/
#define GWEN_IO_LAYER_HTTP_TYPE "http"
/*@}*/



/** @name Flags For This Module
 *
 * You can set these flags using @ref GWEN_Io_Layer_SetFlags() or @ref GWEN_Io_Layer_AddFlags().
 */
/*@{*/
/**
 * If this flag is set then this layer is used for GWEN's IPC code. This means that only requests are
 * sent (normally one side sends a request and the other replies with a response. In IPC mode however
 * all messages are requests in HTTP-sense).
 */
#define GWEN_IO_LAYER_HTTP_FLAGS_IPC     0x00000001
/*@}*/



/** @name Constructor
 *
 */
/*@{*/
/**
 * Creates a new IO layer using the given base layer.
 */
GWENHYWFAR_API GWEN_IO_LAYER *GWEN_Io_LayerHttp_new(GWEN_IO_LAYER *baseLayer);
/*@}*/



/** @name Get Information About Incoming HTTP Messages
 *
 */
/*@{*/

GWENHYWFAR_API GWEN_DB_NODE *GWEN_Io_LayerHttp_GetDbCommandIn(const GWEN_IO_LAYER *io);
GWENHYWFAR_API GWEN_DB_NODE *GWEN_Io_LayerHttp_GetDbStatusIn(const GWEN_IO_LAYER *io);
GWENHYWFAR_API GWEN_DB_NODE *GWEN_Io_LayerHttp_GetDbHeaderIn(const GWEN_IO_LAYER *io);
/*@}*/



/** @name Get Information About Outgoing HTTP Messages
 *
 */
/*@{*/

GWENHYWFAR_API GWEN_DB_NODE *GWEN_Io_LayerHttp_GetDbCommandOut(const GWEN_IO_LAYER *io);
GWENHYWFAR_API GWEN_DB_NODE *GWEN_Io_LayerHttp_GetDbStatusOut(const GWEN_IO_LAYER *io);
GWENHYWFAR_API GWEN_DB_NODE *GWEN_Io_LayerHttp_GetDbHeaderOut(const GWEN_IO_LAYER *io);
/*@}*/



#ifdef __cplusplus
}
#endif



#endif




