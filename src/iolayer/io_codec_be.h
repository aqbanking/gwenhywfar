/***************************************************************************
    copyright   : (C) 2007 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_IOLAYER_CODEC_BE_H
#define GWEN_IOLAYER_CODEC_BE_H

#include <gwenhywfar/iolayer.h>
#include <gwenhywfar/ringbuffer.h>


typedef int (*GWEN_IO_LAYER_CODE_ENCODE_FN)(GWEN_IO_LAYER *io,
					    const uint8_t *pBuffer,
                                            uint32_t lBuffer);

typedef int (*GWEN_IO_LAYER_CODE_DECODE_FN)(GWEN_IO_LAYER *io,
					    uint8_t *pBuffer,
					    uint32_t lBuffer);


GWENHYWFAR_API GWEN_IO_LAYER *GWEN_Io_LayerCodec_new(const char *typeName, GWEN_IO_LAYER *baseLayer);

GWENHYWFAR_API int GWEN_Io_LayerCodec_CheckWriteOut(const GWEN_IO_LAYER *io);

GWENHYWFAR_API GWEN_RINGBUFFER *GWEN_Io_LayerCodec_GetReadBuffer(const GWEN_IO_LAYER *io);
GWENHYWFAR_API GWEN_RINGBUFFER *GWEN_Io_LayerCodec_GetWriteBuffer(const GWEN_IO_LAYER *io);

GWENHYWFAR_API int GWEN_Io_LayerCodec_EnsureReadOk(GWEN_IO_LAYER *io);


GWENHYWFAR_API GWEN_IO_LAYER_CODE_ENCODE_FN GWEN_Io_LayerCodec_SetEncodeFn(GWEN_IO_LAYER *io,
									   GWEN_IO_LAYER_CODE_ENCODE_FN f);

GWENHYWFAR_API GWEN_IO_LAYER_CODE_DECODE_FN GWEN_Io_LayerCodec_SetDecodeFn(GWEN_IO_LAYER *io,
									   GWEN_IO_LAYER_CODE_DECODE_FN f);


GWENHYWFAR_API void GWEN_Io_LayerCodec_AbortRequests(GWEN_IO_LAYER *io, int errorCode);
GWENHYWFAR_API void GWEN_Io_LayerCodec_Reset(GWEN_IO_LAYER *io);

GWENHYWFAR_API uint32_t GWEN_Io_LayerCodec_GetCurrentGuiId(const GWEN_IO_LAYER *io);
GWENHYWFAR_API void GWEN_Io_LayerCodec_SetCurrentGuiId(GWEN_IO_LAYER *io, uint32_t guiid);

#endif




