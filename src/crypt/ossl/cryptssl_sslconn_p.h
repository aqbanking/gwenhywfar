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


#ifndef GWEN_CRYPTSSL_SSL_CONNECTION_P_H
#define GWEN_CRYPTSSL_SSL_CONNECTION_P_H


#include <openssl/ssl.h>
#include <gwenhywfar/inetsocket_l.h>
#include <gwenhywfar/sslconnection.h>


struct GWEN_SSL_CONNECTION {
  GWEN_SOCKET *socket;
  char *CAfile;
  char *CAdir;
  SSL_CTX *ssl_ctx;
  SSL *ssl;
  int isSecure;
};

GWEN_ERRORCODE GWEN_SSLConn__ReadOrWrite(GWEN_SSL_CONNECTION *conn,
                                         const char *outbuffer,
                                         char *inbuffer,
                                         int *bsize,
                                         int timeout,
                                         int reading);

const char *GWEN_SSLConn_ErrorString(unsigned int e);



#endif


