/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sun Sep 14 2003
    copyright   : (C) 2003 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston,                 *
 *   MA  02111-1307  USA                                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef GWENHYFWAR_TRANSPORTLAYER_P_H
#define GWENHYFWAR_TRANSPORTLAYER_P_H

#include <gwenhywfar/transportlayer.h>


GWENHYFWAR_API typedef
  GWEN_ERRORCODE (*GWEN_IPCTRANS_STARTCONNECTPTR)(GWEN_IPCTRANSPORTLAYER *tl);

GWENHYFWAR_API typedef
  GWEN_ERRORCODE (*GWEN_IPCTRANS_FINISHCONNECTPTR)(GWEN_IPCTRANSPORTLAYER *tl);

GWENHYFWAR_API typedef
  GWEN_ERRORCODE (*GWEN_IPCTRANS_LISTENPTR)(GWEN_IPCTRANSPORTLAYER *tl);

GWENHYFWAR_API typedef
  GWEN_ERRORCODE (*GWEN_IPCTRANS_ACCEPTPTR)(GWEN_IPCTRANSPORTLAYER *tl,
                                            GWEN_IPCTRANSPORTLAYER **t);
GWENHYFWAR_API typedef
  GWEN_ERRORCODE (*GWEN_IPCTRANS_DISCONNECTPTR)(GWEN_IPCTRANSPORTLAYER *tl);

GWENHYFWAR_API typedef
  GWEN_SOCKET* (*GWEN_IPCTRANS_GETREADSOCKETPTR)(GWEN_IPCTRANSPORTLAYER *tl);

GWENHYFWAR_API typedef
  GWEN_SOCKET* (*GWEN_IPCTRANS_GETWRITESOCKETPTR)(GWEN_IPCTRANSPORTLAYER *tl);

GWENHYFWAR_API typedef
  GWEN_ERRORCODE (*GWEN_IPCTRANS_READPTR)(GWEN_IPCTRANSPORTLAYER *tl,
                                          char *buffer,
                                          unsigned int *bsize);

GWENHYFWAR_API typedef
  GWEN_ERRORCODE (*GWEN_IPCTRANS_WRITEPTR)(GWEN_IPCTRANSPORTLAYER *tl,
                                           const char *buffer,
                                           unsigned int *bsize);

GWENHYFWAR_API typedef
  GWEN_ERRORCODE (*GWEN_IPCTRANS_FLUSHPTR)(GWEN_IPCTRANSPORTLAYER *tl);

GWENHYFWAR_API typedef
  GWEN_ERRORCODE (*GWEN_IPCTRANS_CANREADPTR)(GWEN_IPCTRANSPORTLAYER *tl);

GWENHYFWAR_API typedef
  GWEN_ERRORCODE (*GWEN_IPCTRANS_CANWRITEPTR)(GWEN_IPCTRANSPORTLAYER *tl);

GWENHYFWAR_API typedef
  GWEN_ERRORCODE (*GWEN_IPCTRANS_GETADDRPTR)(GWEN_IPCTRANSPORTLAYER *tl,
                                             char *buffer,
                                             unsigned int bsize);
GWENHYFWAR_API typedef
  int (*GWEN_IPCTRANS_GETPORTPTR)(GWEN_IPCTRANSPORTLAYER *tl);

GWENHYFWAR_API typedef
  void (*GWEN_IPCTRANS_FREEPTR)(GWEN_IPCTRANSPORTLAYER *tl);





GWEN_IPCTRANSPORTLAYER *GWEN_IPCTransportLayer_new();
void GWEN_IPCTransportLayer_free(GWEN_IPCTRANSPORTLAYER *tl);


struct GWEN_IPCTRANSPORTLAYER {
  GWEN_TransportLayerType type;
  GWEN_IPCTRANS_STARTCONNECTPTR startConnect;
  GWEN_IPCTRANS_FINISHCONNECTPTR finishConnect;
  GWEN_IPCTRANS_LISTENPTR listen;
  GWEN_IPCTRANS_ACCEPTPTR accept;
  GWEN_IPCTRANS_DISCONNECTPTR disconnect;
  GWEN_IPCTRANS_READPTR read;
  GWEN_IPCTRANS_WRITEPTR write;
  GWEN_IPCTRANS_FLUSHPTR flush;
  GWEN_IPCTRANS_CANREADPTR canRead;
  GWEN_IPCTRANS_CANWRITEPTR canWrite;
  GWEN_IPCTRANS_GETREADSOCKETPTR getReadSocket;
  GWEN_IPCTRANS_GETWRITESOCKETPTR getWriteSocket;
  GWEN_IPCTRANS_GETADDRPTR getPeerAddress;
  GWEN_IPCTRANS_GETPORTPTR getPeerPort;
  GWEN_IPCTRANS_FREEPTR free;
  char *address;
  int port;
  void *privateData;
};




#endif /*GWENHYFWAR_TRANSPORTLAYER_P_H */



