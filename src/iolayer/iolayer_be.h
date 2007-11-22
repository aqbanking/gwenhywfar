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


#ifndef GWEN_IOLAYER_BE_H
#define GWEN_IOLAYER_BE_H


#include <gwenhywfar/iolayer.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/inetsocket.h>


GWEN_INHERIT_FUNCTION_LIB_DEFS(GWEN_IO_LAYER, GWENHYWFAR_API)



typedef GWEN_IO_LAYER_WORKRESULT (*GWEN_IO_LAYER_WORKONREQUESTS_FN)(GWEN_IO_LAYER *io);

typedef int (*GWEN_IO_LAYER_ADDREQUEST_FN)(GWEN_IO_LAYER *io, GWEN_IO_REQUEST *r);
typedef int (*GWEN_IO_LAYER_DELREQUEST_FN)(GWEN_IO_LAYER *io, GWEN_IO_REQUEST *r);
typedef int (*GWEN_IO_LAYER_HASWAITINGREQUESTS_FN)(GWEN_IO_LAYER *io);
typedef int (*GWEN_IO_LAYER_ADDWAITINGSOCKETS_FN)(GWEN_IO_LAYER *io,
						  GWEN_SOCKET_LIST2 *readSockets,
						  GWEN_SOCKET_LIST2 *writeSockets);
typedef int (*GWEN_IO_LAYER_LISTEN_FN)(GWEN_IO_LAYER *io);


GWENHYWFAR_API GWEN_IO_LAYER *GWEN_Io_Layer_new(const char *typeName, GWEN_IO_LAYER *baseLayer);


GWENHYWFAR_API void GWEN_Io_Layer_SetStatus(GWEN_IO_LAYER *io, GWEN_IO_LAYER_STATUS st);


GWENHYWFAR_API void GWEN_Io_Layer_AddIncomingLayer(GWEN_IO_LAYER *io, GWEN_IO_LAYER *newIo);

GWENHYWFAR_API GWEN_IO_LAYER_WORKRESULT GWEN_Io_Layer_WorkOnRequests(GWEN_IO_LAYER *io);


GWENHYWFAR_API GWEN_IO_LAYER_WORKONREQUESTS_FN
  GWEN_Io_Layer_SetWorkOnRequestsFn(GWEN_IO_LAYER *io, GWEN_IO_LAYER_WORKONREQUESTS_FN f);

GWENHYWFAR_API GWEN_IO_LAYER_ADDREQUEST_FN
  GWEN_Io_Layer_SetAddRequestFn(GWEN_IO_LAYER *io, GWEN_IO_LAYER_ADDREQUEST_FN f);

GWENHYWFAR_API GWEN_IO_LAYER_DELREQUEST_FN
  GWEN_Io_Layer_SetDelRequestFn(GWEN_IO_LAYER *io, GWEN_IO_LAYER_DELREQUEST_FN f);

GWENHYWFAR_API GWEN_IO_LAYER_HASWAITINGREQUESTS_FN
  GWEN_Io_Layer_SetHasWaitingRequestsFn(GWEN_IO_LAYER *io,
					GWEN_IO_LAYER_HASWAITINGREQUESTS_FN f);

GWENHYWFAR_API GWEN_IO_LAYER_ADDWAITINGSOCKETS_FN
  GWEN_Io_Layer_SetAddWaitingSocketsFn(GWEN_IO_LAYER *io,
				       GWEN_IO_LAYER_ADDWAITINGSOCKETS_FN f);

GWENHYWFAR_API GWEN_IO_LAYER_LISTEN_FN
  GWEN_Io_Layer_SetListenFn(GWEN_IO_LAYER *io, GWEN_IO_LAYER_LISTEN_FN f);


#endif
