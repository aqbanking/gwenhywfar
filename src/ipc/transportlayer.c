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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "transportlayer_p.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>



/*_________________________________________________________________________
 *AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                   General Transport layer functions
 *YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */


/* --------------------------------------------------------------- FUNCTION */
const char *GWEN_IPCTransportLayer_GetAddress(GWEN_IPCTRANSPORTLAYER *tl) {
  assert(tl);
  return tl->address;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_IPCTransportLayer_SetAddress(GWEN_IPCTRANSPORTLAYER *tl,
				       const char *buffer) {
  assert(tl);
  assert(buffer);
  free(tl->address);
  tl->address=strdup(buffer);
}



/* --------------------------------------------------------------- FUNCTION */
int GWEN_IPCTransportLayer_GetPort(GWEN_IPCTRANSPORTLAYER *tl){
  assert(tl);
  return tl->port;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_IPCTransportLayer_SetPort(GWEN_IPCTRANSPORTLAYER *tl, int port){
  assert(tl);
  tl->port=port;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_TransportLayerType
GWEN_IPCTransportLayer_GetType(GWEN_IPCTRANSPORTLAYER *tl){
  assert(tl);
  return tl->type;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_IPCTRANSPORTLAYER *GWEN_IPCTransportLayer_new(){
  GWEN_IPCTRANSPORTLAYER *tl;

  GWEN_NEW_OBJECT(GWEN_IPCTRANSPORTLAYER, tl);

  return tl;
}



/* --------------------------------------------------------------- FUNCTION */
void GWEN_IPCTransportLayer_free(GWEN_IPCTRANSPORTLAYER * tl){
  if (tl) {
    if (tl->free)
      tl->free(tl);
    free(tl->address);
    free(tl);
  }
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE
GWEN_IPCTransportLayer_StartConnect(GWEN_IPCTRANSPORTLAYER *tl){
  assert(tl);
  assert(tl->startConnect);
  return tl->startConnect(tl);
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE
GWEN_IPCTransportLayer_FinishConnect(GWEN_IPCTRANSPORTLAYER *tl){
  assert(tl);
  assert(tl->finishConnect);
  return tl->finishConnect(tl);
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE
GWEN_IPCTransportLayer_Listen(GWEN_IPCTRANSPORTLAYER *tl){
  assert(tl);
  assert(tl->listen);
  return tl->listen(tl);
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE
GWEN_IPCTransportLayer_Accept(GWEN_IPCTRANSPORTLAYER *tl,
			      GWEN_IPCTRANSPORTLAYER **t){
  assert(tl);
  assert(tl->accept);
  return tl->accept(tl, t);
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE
GWEN_IPCTransportLayer_Disconnect(GWEN_IPCTRANSPORTLAYER *tl){
  assert(tl);
  assert(tl->disconnect);
  return tl->disconnect(tl);
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_SOCKET*
GWEN_IPCTransportLayer_GetReadSocket(GWEN_IPCTRANSPORTLAYER *tl){
  assert(tl);
  if (tl->getReadSocket)
    return tl->getReadSocket(tl);
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_SOCKET*
GWEN_IPCTransportLayer_GetWriteSocket(GWEN_IPCTRANSPORTLAYER *tl){
  assert(tl);
  if (tl->getWriteSocket)
    return tl->getWriteSocket(tl);
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE
GWEN_IPCTransportLayer_Read(GWEN_IPCTRANSPORTLAYER *tl,
			    char *buffer,
                            unsigned int *bsize){
  assert(tl);
  assert(tl->read);
  return tl->read(tl, buffer, bsize);
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE
GWEN_IPCTransportLayer_Write(GWEN_IPCTRANSPORTLAYER *tl,
			     const char *buffer,
                             unsigned int *bsize){
  assert(tl);
  assert(tl->write);
  return tl->write(tl, buffer, bsize);
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE
GWEN_IPCTransportLayer_Flush(GWEN_IPCTRANSPORTLAYER *tl){
  assert(tl);
  assert(tl->flush);
  return tl->flush(tl);
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE
GWEN_IPCTransportLayer_CanRead(GWEN_IPCTRANSPORTLAYER *tl){
  assert(tl);
  assert(tl->canRead);
  return tl->canRead(tl);
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE
GWEN_IPCTransportLayer_CanWrite(GWEN_IPCTRANSPORTLAYER *tl){
  assert(tl);
  assert(tl->canWrite);
  return tl->canWrite(tl);
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE
GWEN_IPCTransportLayer_GetPeerAddress(GWEN_IPCTRANSPORTLAYER *tl,
				      char *buffer,
				      unsigned int bsize){
  assert(tl);
  assert(tl->getPeerAddress);
  return tl->getPeerAddress(tl, buffer, bsize);
}



/* --------------------------------------------------------------- FUNCTION */
int GWEN_IPCTransportLayer_GetPeerPort(GWEN_IPCTRANSPORTLAYER *tl){
  assert(tl);
  assert(tl->getPeerPort);
  return tl->getPeerPort(tl);
}







