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


#ifndef GWEN_IOLAYER_H
#define GWEN_IOLAYER_H

#include <gwenhywfar/list2.h>

#ifdef __cplusplus
extern "C" {
#endif


/** @addtogroup MOD_IOLAYER
 *
 * <p>
 * This module introduces IO layer objects which are used to perform input/output operations.
 * IO layers can be stacked in order to create IO layer chains. There are a few io layer types
 * which work as endpoints (i.e. they have no base layer underneath them but rather perform
 * direct input/output operations such as reading from a file, writing to a network socket etc).
 * </p>
 * <p>
 * Most io layer objects operate over base io layers, e.g. the HTTP io layer can work directly
 * over network socket io layers or over an SSL layer which in turn operates over a network socket
 * io layer.
 * </p>
 * <p>
 * All toplevel io layers (i.e. those which do not serve as base layers to others) must be registered
 * via @ref GWEN_Io_Manager_RegisterLayer() to allow Gwenhywfar to manage them.
 * </p>
 * <p>
 * This module is based on io requests, e.g. to read some bytes from from an io layer object an io request
 * is created and enqueued with the io layer object. IO requests may have a callback function which is
 * called when the request has been finished (either because the requested operations has been performed
 * or because the request was aborted).
 * </p>
 * <p>
 * This module also contains convenience functions which provide a simpler API for io operations thereby hiding
 * the internal io request usage.
 * </p>
 */
/*@{*/


typedef struct GWEN_IO_LAYER GWEN_IO_LAYER;

GWEN_LIST2_FUNCTION_LIB_DEFS(GWEN_IO_LAYER, GWEN_Io_Layer, GWENHYWFAR_API)


typedef enum {
  GWEN_Io_Layer_WorkResultError=-1,
  GWEN_Io_Layer_WorkResultOk=0,
  GWEN_Io_Layer_WorkResultBlocking
} GWEN_IO_LAYER_WORKRESULT;


typedef enum {
  GWEN_Io_Layer_StatusUnknown=-1,
  GWEN_Io_Layer_StatusUnconnected=0,
  GWEN_Io_Layer_StatusConnecting,
  GWEN_Io_Layer_StatusConnected,
  GWEN_Io_Layer_StatusDisconnecting,
  GWEN_Io_Layer_StatusDisconnected,
  GWEN_Io_Layer_StatusListening,
  GWEN_Io_Layer_StatusDisabled,
} GWEN_IO_LAYER_STATUS;


#include <gwenhywfar/iorequest.h>
#include <gwenhywfar/buffer.h>


/** @name Flags For IO Layers
 *
 * You can set these flags using @ref GWEN_Io_Layer_SetFlags() or @ref GWEN_Io_Layer_AddFlags().
 */
/*@{*/
/**
 * If this flag is set then an object will take over the buffer given to the constructor. i.e. the buffer
 * is only valid as long as the io layer exists.
 */
#define GWEN_IO_LAYER_FLAGS_TAKEOVER    0x01000000

/**
 * If this flag is set then this connection is a passive one (e.g. other hosts/processes
 * actively connect to this one)
 */
#define GWEN_IO_LAYER_FLAGS_PASSIVE     0x10000000

/**
 * If this flag is set then the io layer is packet based.
 */
#define GWEN_IO_LAYER_FLAGS_PKG_BASED   0x20000000
/**
 * If this flag is set then the given file descriptor will never be closed. However, upon receiption of
 * the request @ref GWEN_Io_Request_TypeDisconnect the file will be made unaccessibly by this module.
 */
#define GWEN_IO_LAYER_FLAGS_DONTCLOSE   0x40000000

/**
 * This flag is set by @ref GWEN_Io_Manager_RegisterLayer and cleared by
 * @ref GWEN_Io_Manager_UnregisterLayer
 */
#define GWEN_IO_LAYER_FLAGS_REGISTERED  0x80000000
/*@}*/



/** @name Object Management
 *
 */
/*@{*/
GWENHYWFAR_API void GWEN_Io_Layer_Attach(GWEN_IO_LAYER *io);
GWENHYWFAR_API void GWEN_Io_Layer_free(GWEN_IO_LAYER *io);
/*@}*/



/** @name Informational Functions
 *
 */
/*@{*/
GWENHYWFAR_API GWEN_IO_LAYER *GWEN_Io_Layer_GetBaseLayer(GWEN_IO_LAYER *io);

/**
 * Compare the type name of the given io layer and all its base layers until one with the given type name
 * is found.
 */
GWENHYWFAR_API GWEN_IO_LAYER *GWEN_Io_Layer_FindBaseLayerByType(GWEN_IO_LAYER *io, const char *tname);

GWENHYWFAR_API uint32_t GWEN_Io_Layer_GetFlags(const GWEN_IO_LAYER *io);
GWENHYWFAR_API void GWEN_Io_Layer_SetFlags(GWEN_IO_LAYER *io, uint32_t f);
GWENHYWFAR_API void GWEN_Io_Layer_AddFlags(GWEN_IO_LAYER *io, uint32_t f);
GWENHYWFAR_API void GWEN_Io_Layer_SubFlags(GWEN_IO_LAYER *io, uint32_t f);


GWENHYWFAR_API GWEN_IO_LAYER_STATUS GWEN_Io_Layer_GetStatus(const GWEN_IO_LAYER *io);

GWENHYWFAR_API GWEN_IO_LAYER *GWEN_Io_Layer_GetNextIncomingLayer(GWEN_IO_LAYER *io);
GWENHYWFAR_API void GWEN_Io_Layer_ClearIncomingLayers(GWEN_IO_LAYER *io);

/*@}*/



/** @name Request Handling
 *
 */
/*@{*/
/**
 * Adds a request to a given io layer.
 * <p>
 * Please note that some io layers handle requests directly within this function, so when it returns the
 * given request has already been finished (and the request's finish handler has already been called if any).
 * </p>
 * <p>
 * If the io layer is unable to accept this request (e.g. because there are already too many requests enqueued)
 * then it returns GWEN_ERROR_TRY_AGAIN thus indicating that you may try at a later time.
 * </p>
 * <p>
 * Please not that the request's finish handler (if any) is not called if GWEN_ERROR_TRY_AGAIN is returned
 * here to allow to try again later to add the request.
 * </p>
 */
GWENHYWFAR_API int GWEN_Io_Layer_AddRequest(GWEN_IO_LAYER *io, GWEN_IO_REQUEST *r);

/**
 * This function can be used to abort a request which has been added with @ref GWEN_Io_Layer_AddRequest().
 * The request's finish handler is called from within this function (if the request really is enqueued with this
 * io layer).
 */
GWENHYWFAR_API int GWEN_Io_Layer_DelRequest(GWEN_IO_LAYER *io, GWEN_IO_REQUEST *r);

/**
 * Checks whether this io layer has any waiting incoming requests.
 * @return 0 no waiting requests, !=0 otherwise
 */
GWENHYWFAR_API int GWEN_Io_Layer_HasWaitingRequests(GWEN_IO_LAYER *io);
/*@}*/



/** @name Convenience Functions
 *
 * Internally all IO layers work with IO requests. However, some users don't want to handle requests
 * by themselves. Those users can use the functions of this group which internally again setup and
 * dispatch IO requests.
 */
/*@{*/

/**
 * This function only connects the given io layer, it doesn't care for the possibly available base layers.
 */
GWENHYWFAR_API int GWEN_Io_Layer_Connect(GWEN_IO_LAYER *io,
					 uint32_t flags,
					 uint32_t guiid, int msecs);

/**
 * This function connects an io layer and all its base layers until the given layer has been reached (which is then
 * @b not connected).
 * The order of connecting all the layers in a chain is from the inner most base layer up to the outermost one.
 * If an io layer along the chain already is connected then this function will not try to connect it again.
 */
GWENHYWFAR_API int GWEN_Io_Layer_ConnectRecursively(GWEN_IO_LAYER *io,
						    GWEN_IO_LAYER *stopAtLayer,
						    uint32_t flags,
						    uint32_t guiid, int msecs);

/**
 * This function only disconnects the given io layer, it doesn't care for the possibly available base layers.
 */
GWENHYWFAR_API int GWEN_Io_Layer_Disconnect(GWEN_IO_LAYER *io,
					    uint32_t flags,
					    uint32_t guiid, int msecs);


GWENHYWFAR_API int GWEN_Io_Layer_Listen(GWEN_IO_LAYER *io);

GWENHYWFAR_API int GWEN_Io_Layer_ListenRecursively(GWEN_IO_LAYER *io, GWEN_IO_LAYER *stopAtLayer);


/**
 * This function disconnects all io layers along a chain of io layers until the given layer is reached (which is
 * then @b not disconnected).
 * The order of disconnecting all the layers is from the given io layer downto the inner most io layer (i.e. the one
 * which has no bas layer).
 * If the flag GWEN_IO_REQUEST_FLAGS_FORCE is cleared then this function first
 * tries to flush the given io layers and all layers below before disconnecting.
 */
GWENHYWFAR_API int GWEN_Io_Layer_DisconnectRecursively(GWEN_IO_LAYER *io,
						       GWEN_IO_LAYER *stopAtLayer,
						       uint32_t flags,
						       uint32_t guiid, int msecs);


GWENHYWFAR_API int GWEN_Io_Layer_ReadBytes(GWEN_IO_LAYER *io,
					   uint8_t *buffer,
					   uint32_t size,
					   uint32_t flags,
					   uint32_t guiid, int msecs);

GWENHYWFAR_API int GWEN_Io_Layer_ReadToBufferUntilEof(GWEN_IO_LAYER *io,
                                                      GWEN_BUFFER *buf,
						      uint32_t guiid, int msecs);

GWENHYWFAR_API int GWEN_Io_Layer_ReadPacket(GWEN_IO_LAYER *io,
					    uint8_t *buffer,
					    uint32_t size,
					    uint32_t flags,
					    uint32_t guiid, int msecs);

GWENHYWFAR_API int GWEN_Io_Layer_ReadPacketToBuffer(GWEN_IO_LAYER *io,
						    GWEN_BUFFER *buf,
						    uint32_t flags,
						    uint32_t guiid, int msecs);


GWENHYWFAR_API int GWEN_Io_Layer_WriteBytes(GWEN_IO_LAYER *io,
					    const uint8_t *buffer,
					    uint32_t size,
					    uint32_t flags,
					    uint32_t guiid, int msecs);

GWENHYWFAR_API int GWEN_Io_Layer_WriteString(GWEN_IO_LAYER *io,
					     const char *buffer,
					     uint32_t flags,
					     uint32_t guiid, int msecs);

GWENHYWFAR_API int GWEN_Io_Layer_WriteChar(GWEN_IO_LAYER *io,
					   char c,
					   uint32_t flags,
					   uint32_t guiid, int msecs);


/**
 * This functions lets all io layers write any data from any internal buffer.
 * It works its way down from the given io layer to its lowest base layer.
 */
GWENHYWFAR_API int GWEN_Io_Layer_FlushRecursively(GWEN_IO_LAYER *io, uint32_t guiid, int msecs);

/*@}*/


/*@}*/

#ifdef __cplusplus
}
#endif


#endif
