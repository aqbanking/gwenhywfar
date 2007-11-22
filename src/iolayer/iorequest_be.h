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


#ifndef GWEN_IOREQUEST_BE_H
#define GWEN_IOREQUEST_BE_H


#include <gwenhywfar/iorequest.h>


GWEN_LIST_FUNCTION_LIB_DEFS(GWEN_IO_REQUEST, GWEN_Io_Request, GWENHYWFAR_API)


GWENHYWFAR_API void GWEN_Io_Request_SetBufferPos(GWEN_IO_REQUEST *r, uint32_t i);

GWENHYWFAR_API void GWEN_Io_Request_SetIncomingLayer(GWEN_IO_REQUEST *r, GWEN_IO_LAYER *iol);

GWENHYWFAR_API void GWEN_Io_Request_Finished(GWEN_IO_REQUEST *r, GWEN_IO_REQUEST_STATUS st, int result);



#endif
