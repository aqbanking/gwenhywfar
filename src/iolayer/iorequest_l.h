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


#ifndef GWEN_IOREQUEST_L_H
#define GWEN_IOREQUEST_L_H

#include "iorequest_be.h"


void GWEN_Io_Request_SetStatus(GWEN_IO_REQUEST *r, GWEN_IO_REQUEST_STATUS st);
void GWEN_Io_Request_SetResultCode(GWEN_IO_REQUEST *r, int result);
void GWEN_Io_Request_SetIoLayer(GWEN_IO_REQUEST *r, GWEN_IO_LAYER *io);


#endif

