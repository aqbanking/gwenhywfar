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


#ifndef GWEN_IOREQUEST_P_H
#define GWEN_IOREQUEST_P_H



#include "iorequest_l.h"



struct GWEN_IO_REQUEST {
  GWEN_LIST_ELEMENT(GWEN_IO_REQUEST)

  GWEN_IO_REQUEST_TYPE type;
  uint32_t flags;
  uint8_t *bufferPtr;
  uint32_t bufferSize;
  uint32_t bufferPos;

  GWEN_IO_REQUEST_FINISH_FN finishFn;
  void *user_data;

  uint32_t guiid;

  GWEN_IO_LAYER *ioLayer;

  GWEN_IO_REQUEST_STATUS status;
  int resultCode;

  GWEN_IO_LAYER *incomingLayer;

  uint32_t refCount;
};



#endif
