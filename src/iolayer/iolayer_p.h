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


#ifndef GWEN_IOLAYER_P_H
#define GWEN_IOLAYER_P_H

#include "iolayer_l.h"


struct GWEN_IO_LAYER {
  GWEN_INHERIT_ELEMENT(GWEN_IO_LAYER)
  GWEN_LIST_ELEMENT(GWEN_IO_LAYER)

  char *typeName;
  GWEN_IO_LAYER *baseLayer;
  uint32_t flags;

  GWEN_IO_LAYER_STATUS status;

  GWEN_IO_LAYER_WORKONREQUESTS_FN workOnRequestsFn;
  GWEN_IO_LAYER_ADDREQUEST_FN addRequestFn;
  GWEN_IO_LAYER_DELREQUEST_FN delRequestFn;
  GWEN_IO_LAYER_HASWAITINGREQUESTS_FN hasWaitingRequestsFn;
  GWEN_IO_LAYER_ADDWAITINGSOCKETS_FN addWaitingSocketsFn;

  GWEN_IO_LAYER_LISTEN_FN listenFn;

  GWEN_IO_LAYER_LIST *incomingLayers;

  uint32_t refCount;
};


static int GWEN_Io_Layer__WaitForRequest(GWEN_IO_LAYER *io, GWEN_IO_REQUEST *r,
					 int msecs, int deleteRequest);

static int GWEN_Io_Layer__DisconnectRecursively(GWEN_IO_LAYER *io,
						GWEN_IO_LAYER *stopAtLayer,
						uint32_t flags,
						uint32_t guiid, int msecs);


#endif
