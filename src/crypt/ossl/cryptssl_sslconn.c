/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Sat Jan 17 2004
 copyright   : (C) 2004 by Martin Preuss
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

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>

#include "cryptssl_sslconn_p.h"



int GWEN_SSLConn_IsAvailable(){
  return -1;
}


GWEN_SSL_CONNECTION *GWEN_SSLConn_new(int server,
                                      const GWEN_INETADDRESS *addr,
                                      const char *cafile,
                                      const char *capath){
  GWEN_SSL_CONNECTION *conn;

  GWEN_NEW_OBJECT(GWEN_SSL_CONNECTION, conn);
  if (cafile)
    conn->CAfile=strdup(cafile);
  if (capath)
    conn->CAdir=strdup(capath);
  conn->isServer=(server!=0);
  return conn;
}



void GWEN_SSLConn_free(GWEN_SSL_CONNECTION *conn){
  if (conn) {
    free(conn->CAfile);
    free(conn->CAdir);
    GWEN_Socket_free(conn->socket);
    free(conn);
  }
}



GWEN_ERRORCODE GWEN_SSLConn_Connect(GWEN_SSL_CONNECTION *conn,
                                    const GWEN_INETADDRESS *addr,
                                    int timeout){
}



GWEN_ERRORCODE GWEN_SSLConn_Disconnect(GWEN_SSL_CONNECTION *conn){
}



GWEN_ERRORCODE GWEN_SSLConn_Accept(GWEN_SSL_CONNECTION *conn,
                                   GWEN_INETADDRESS **addr,
                                   int timeout){
}



GWEN_ERRORCODE GWEN_SSLConn_Read(GWEN_SSL_CONNECTION *conn,
                                 char *buffer,
                                 int *bsize,
                                 int timeout){
}



GWEN_ERRORCODE GWEN_SSLConn_Write(GWEN_SSL_CONNECTION *conn,
                                  const char *buffer,
                                  int *bsize,
                                  int timeout){
}







