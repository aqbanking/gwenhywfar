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


#ifndef GWEN_IOLAYER_L_H
#define GWEN_IOLAYER_L_H


#include "iolayer_be.h"
#include <gwenhywfar/list1.h>


GWEN_LIST_FUNCTION_DEFS(GWEN_IO_LAYER, GWEN_Io_Layer)



int GWEN_Io_Layer_AddWaitingSockets(GWEN_IO_LAYER *io,
				    GWEN_SOCKET_LIST2 *readSockets,
				    GWEN_SOCKET_LIST2 *writeSockets);


#endif
