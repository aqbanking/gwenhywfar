/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Tue Oct 02 2002
    copyright   : (C) 2002 by Martin Preuss
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



#include "inetsocket_p.h"
#include "inetaddr_p.h"
#include <gwenhywfar/misc.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>


/* forward declaration */
const char *GWEN_Socket_ErrorString(int c);

static int gwen_socket_is_initialized=0;
static GWEN_ERRORTYPEREGISTRATIONFORM *gwen_socket_errorform=0;




GWEN_ERRORCODE GWEN_Socket_ModuleInit(){
  if (!gwen_socket_is_initialized) {
    GWEN_ERRORCODE err;

    gwen_socket_errorform=GWEN_ErrorType_new();
    GWEN_ErrorType_SetName(gwen_socket_errorform,
			   GWEN_SOCKET_ERROR_TYPE);
    GWEN_ErrorType_SetMsgPtr(gwen_socket_errorform,
                             GWEN_Socket_ErrorString);
    err=GWEN_Error_RegisterType(gwen_socket_errorform);
    if (!GWEN_Error_IsOk(err))
      return err;
    gwen_socket_is_initialized=1;
  }
  return 0;
}



GWEN_ERRORCODE GWEN_Socket_ModuleFini(){
  if (gwen_socket_is_initialized) {
    GWEN_ERRORCODE err;

    err=GWEN_Error_UnregisterType(gwen_socket_errorform);
    GWEN_ErrorType_free(gwen_socket_errorform);
    if (!GWEN_Error_IsOk(err))
      return err;
    gwen_socket_is_initialized=0;
  }
  return 0;
}



GWEN_ERRORCODE GWEN_SocketSet_Clear(GWEN_SOCKETSET *ssp){
  assert(ssp);
}



GWEN_SOCKETSET *GWEN_SocketSet_new() {
  GWEN_SOCKETSET *ssp;

  GWEN_NEW_OBJECT(GWEN_SOCKETSET, ssp);
  return ssp;
}



void GWEN_SocketSet_free(GWEN_SOCKETSET *ssp) {
  if (ssp) {
    free(ssp);
  }
}



GWEN_ERRORCODE GWEN_SocketSet_AddSocket(GWEN_SOCKETSET *ssp,
                                        const GWEN_SOCKET *sp){
  assert(ssp);
  assert(sp);
}



GWEN_ERRORCODE GWEN_SocketSet_RemoveSocket(GWEN_SOCKETSET *ssp,
                                           const GWEN_SOCKET *sp){
  assert(ssp);
  assert(sp);
}



int GWEN_SocketSet_HasSocket(GWEN_SOCKETSET *ssp,
                             const GWEN_SOCKET *sp){
  assert(ssp);
  assert(sp);
}



GWEN_SOCKET *GWEN_Socket_new(){
  GWEN_SOCKET *sp;

  GWEN_NEW_OBJECT(GWEN_SOCKET, sp);
  return sp;
}



void GWEN_Socket_free(GWEN_SOCKET *sp){
  if (sp)
    free(sp);
}



GWEN_ERRORCODE GWEN_Socket_Open(GWEN_SOCKET *sp, GWEN_SOCKETTYPE socketType){
}



GWEN_ERRORCODE GWEN_Socket_Connect(GWEN_SOCKET *sp,
                                   const GWEN_INETADDRESS *addr){
}



GWEN_ERRORCODE GWEN_Socket_Close(GWEN_SOCKET *sp){
}



GWEN_ERRORCODE GWEN_Socket_Bind(GWEN_SOCKET *sp,
                                const GWEN_INETADDRESS *addr){
}



GWEN_ERRORCODE GWEN_Socket_Listen(GWEN_SOCKET *sp, int backlog){
}



GWEN_ERRORCODE GWEN_Socket_Accept(GWEN_SOCKET *sp,
                                  GWEN_INETADDRESS **newaddr,
                                  GWEN_SOCKET **newsock){
}



GWEN_ERRORCODE GWEN_Socket_GetPeerAddr(GWEN_SOCKET *sp,
                                       GWEN_INETADDRESS **newaddr){
}



GWEN_ERRORCODE GWEN_Socket_Select(GWEN_SOCKETSET *rs,
                                  GWEN_SOCKETSET *ws,
                                  GWEN_SOCKETSET *xs,
                                  int timeout){
}



GWEN_ERRORCODE GWEN_Socket_Read(GWEN_SOCKET *sp,
                                char *buffer,
                                int *bsize){
}



GWEN_ERRORCODE GWEN_Socket_Write(GWEN_SOCKET *sp,
                                 const char *buffer,
                                 int *bsize){
}



GWEN_ERRORCODE GWEN_Socket_ReadFrom(GWEN_SOCKET *sp,
                                    GWEN_INETADDRESS **newaddr,
                                    char *buffer,
                                    int *bsize){
}



GWEN_ERRORCODE GWEN_Socket_WriteTo(GWEN_SOCKET *sp,
                                   const GWEN_INETADDRESS *addr,
                                   const char *buffer,
                                   int *bsize){
}



GWEN_ERRORCODE GWEN_Socket_SetBlocking(GWEN_SOCKET *sp,
                                       int fl) {
}



GWEN_ERRORCODE GWEN_Socket_SetBroadcast(GWEN_SOCKET *sp,
                                        int fl) {
}



GWEN_ERRORCODE GWEN_Socket_SetReuseAddress(GWEN_SOCKET *sp, int fl){
}



GWEN_ERRORCODE GWEN_Socket_GetSocketError(GWEN_SOCKET *sp) {
}



GWEN_ERRORCODE GWEN_Socket_WaitForRead(GWEN_SOCKET *sp,
                                       int timeout) {
}



GWEN_ERRORCODE GWEN_Socket_WaitForWrite(GWEN_SOCKET *sp,
                                        int timeout) {
}



GWEN_SOCKETTYPE GWEN_Socket_GetSocketType(GWEN_SOCKET *sp){
  assert(sp);
  return sp->type;
}



const char *GWEN_Socket_ErrorString(int c){
  const char *s;

  switch(c) {
  case 0:
    s="Success";
    break;
  case GWEN_SOCKET_ERROR_BAD_SOCKETTYPE:
    s="Bad socket type";
    break;
  case GWEN_SOCKET_ERROR_NOT_OPEN:
    s="Socket not open";
    break;
  case GWEN_SOCKET_ERROR_TIMEOUT:
    s="Socket timeout";
    break;
  case GWEN_SOCKET_ERROR_IN_PROGRESS:
    s="Operation in progress";
    break;
  case GWEN_SOCKET_ERROR_INTERRUPTED:
    s="Operation interrupted by system signal.";
    break;
  default:
    s=(const char*)0;
  } /* switch */
  return s;
}



