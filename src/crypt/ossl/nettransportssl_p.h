/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Wed May 05 2004
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


#ifndef GWEN_NETTRANSPORTSSL_P_H
#define GWEN_NETTRANSPORTSSL_P_H


#include <gwenhywfar/nettransportssl.h>
#include <openssl/ssl.h>

#define GWEN_NETTRANSPORTSSL_MAXCOLL 100


typedef struct GWEN_NETTRANSPORTSSL {
  GWEN_SOCKET *socket;
  int ownSocket;

  char *CAdir;
  char *ownCertFile;
  char *dhfile;
  SSL_CTX *ssl_ctx;
  SSL *ssl;
  int secure;
  int isSecure;
  int active;
  char *cipherList;
} GWEN_NETTRANSPORTSSL;


GWEN_NETTRANSPORTSSL *GWEN_NetTransportSSLData_new();

void GWEN_NetTransportSSL_FreeData(void *bp, void *p);


GWEN_NETTRANSPORT_RESULT
  GWEN_NetTransportSSL_StartConnect(GWEN_NETTRANSPORT *tr);

GWEN_NETTRANSPORT_RESULT
  GWEN_NetTransportSSL_StartAccept(GWEN_NETTRANSPORT *tr);

GWEN_NETTRANSPORT_RESULT
  GWEN_NetTransportSSL_StartDisconnect(GWEN_NETTRANSPORT *tr);



GWEN_NETTRANSPORT_RESULT
  GWEN_NetTransportSSL_Read(GWEN_NETTRANSPORT *tr,
                               char *buffer,
                               int *bsize);

GWEN_NETTRANSPORT_RESULT
  GWEN_NetTransportSSL_Write(GWEN_NETTRANSPORT *tr,
                                const char *buffer,
                                int *bsize);



int GWEN_NetTransportSSL_AddSockets(GWEN_NETTRANSPORT *tr,
                                       GWEN_SOCKETSET *sset,
                                       int forReading);

GWEN_NETTRANSPORT_WORKRESULT
  GWEN_NetTransportSSL_Work(GWEN_NETTRANSPORT *tr);


const char *GWEN_NetTransportSSL_ErrorString(unsigned int e);

int GWEN_NetTransportSSL__Check_Cert(GWEN_NETTRANSPORTSSL *skd,
                                     const char *name);
int GWEN_NetTransportSSL__SetupSSL(GWEN_NETTRANSPORT *tr, int fd);

int GWEN_NetTransportSSL_GetPassword(GWEN_NETTRANSPORT *tr,
                                     char *buffer, int num,
                                     int rwflag);

static int GWEN_NetTransportSSL_PasswordCB(char *buffer, int num,
                                           int rwflag, void *userdata);

GWEN_NETTRANSPORTSSL_ASKADDCERT_RESULT
  GWEN_NetTransportSSL__AskAddCert(GWEN_NETTRANSPORT *tr,
                                   GWEN_DB_NODE *cert);

int GWEN_NetTransportSSL__SaveCert(GWEN_NETTRANSPORT *tr,
                                   X509 *cert);

void GWEN_NetTransportSSL__CertEntries2Db(X509_NAME *nm,
                                          GWEN_DB_NODE *db,
                                          int nid,
                                          const char *name);
GWEN_DB_NODE *GWEN_NetTransportSSL__Cert2Db(X509 *cert);




#endif /* GWEN_NETTRANSPORTSSL_P_H */

