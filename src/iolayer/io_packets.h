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


#ifndef GWEN_PACKETS_H
#define GWEN_PACKETS_H

#include <gwenhywfar/iolayer.h>
#include <gwenhywfar/iorequest.h>



#define GWEN_IO_LAYER_PACKETS_TYPE "packets"


#define GWEN_IO_LAYER_PACKETS_DEFAULT_READSIZE  2048
#define GWEN_IO_LAYER_PACKETS_DEFAULT_READQUEUE  16
#define GWEN_IO_LAYER_PACKETS_DEFAULT_WRITEQUEUE 16

#ifdef __cplusplus
extern "C" {
#endif


GWENHYWFAR_API GWEN_IO_LAYER *GWEN_Io_LayerPackets_new(GWEN_IO_LAYER *baseLayer);


GWENHYWFAR_API int GWEN_Io_LayerPackets_GetReadRequest(GWEN_IO_LAYER *io,
						       GWEN_IO_REQUEST **pRequest,
						       uint32_t guiid, int msecs);

GWENHYWFAR_API int GWEN_Io_LayerPackets_HasReadRequests(const GWEN_IO_LAYER *io);

GWENHYWFAR_API int GWEN_Io_LayerPackets_AddWriteRequest(GWEN_IO_LAYER *io, GWEN_IO_REQUEST *r);


GWENHYWFAR_API int GWEN_Io_LayerPackets_GetReadSize(const GWEN_IO_LAYER *io);
GWENHYWFAR_API void GWEN_Io_LayerPackets_SetReadSize(GWEN_IO_LAYER *io, int i);

GWENHYWFAR_API int GWEN_Io_LayerPackets_GetMaxReadQueue(const GWEN_IO_LAYER *io);
GWENHYWFAR_API void GWEN_Io_LayerPackets_SetMaxReadQueue(GWEN_IO_LAYER *io, int i);

GWENHYWFAR_API int GWEN_Io_LayerPackets_GetMaxWriteQueue(const GWEN_IO_LAYER *io);
GWENHYWFAR_API void GWEN_Io_LayerPackets_SetMaxWriteQueue(GWEN_IO_LAYER *io, int i);


#ifdef __cplusplus
}
#endif



#endif


