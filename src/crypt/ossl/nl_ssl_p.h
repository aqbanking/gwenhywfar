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


#ifndef GWEN_NL_SSL_P_H
#define GWEN_NL_SSL_P_H


#include <gwenhywfar/netlayer.h>
#include <gwenhywfar/nl_ssl.h>
#include <gwenhywfar/gwentime.h>
#include <openssl/ssl.h>

#define GWEN_NL_SSL_MAXCOLL 100
#define GWEN_NL_SSL_DEFAULT_CERT_DAYS 365


typedef enum {
  GWEN_NetLayerSslMode_Idle=0,
  GWEN_NetLayerSslMode_PConnecting,
  GWEN_NetLayerSslMode_PConnected,
  GWEN_NetLayerSslMode_LConnecting,
  GWEN_NetLayerSslMode_LConnected,
  GWEN_NetLayerSslMode_LDisconnecting,
  GWEN_NetLayerSslMode_LDisconnected,
  GWEN_NetLayerSslMode_PDisconnecting,
  GWEN_NetLayerSslMode_PDisconnected,

} GWEN_NETLAYER_SSL_MODE;


typedef struct GWEN_NL_SSL GWEN_NL_SSL;
struct GWEN_NL_SSL {
  GWEN_NETLAYER_SSL_MODE mode;

  BIO *bio;
  SSL_CTX *ssl_ctx;
  SSL *ssl;

  int secure;
  char *CAdir;
  char *CAfile;
  char *newCAdir;
  char *ownCertFile;
  char *dhFolder;
  char *cipherList;

  GWEN_SSLCERTDESCR *peerCertificate;
  int isSecure;

  GWEN_NL_SSL_ASKADDCERT_FN askAddCertFn;
  void *askAddCertData;

};

void GWENHYWFAR_CB GWEN_NetLayerSsl_FreeData(void *bp, void *p);


int GWEN_NetLayerSsl_Connect(GWEN_NETLAYER *nl);
int GWEN_NetLayerSsl_Disconnect(GWEN_NETLAYER *nl);

int GWEN_NetLayerSsl_Listen(GWEN_NETLAYER *nl);

int GWEN_NetLayerSsl_Read(GWEN_NETLAYER *nl, char *buffer, int *bsize);
int GWEN_NetLayerSsl_Write(GWEN_NETLAYER *nl, const char *buffer,int *bsize);

int GWEN_NetLayerSsl_AddSockets(GWEN_NETLAYER *nl,
                                GWEN_SOCKETSET *readSet,
                                GWEN_SOCKETSET *writeSet,
                                GWEN_SOCKETSET *exSet);

GWEN_NETLAYER_RESULT GWEN_NetLayerSsl_Work(GWEN_NETLAYER *nl);


int GWEN_NetLayerSsl_Setup(GWEN_NETLAYER *nl);

const char *GWEN_NetLayerSsl_ErrorString(unsigned int e);
void GWEN_NetLayerSsl_InfoCallBack(SSL *s, int where, int ret);
int GWEN_NetLayerSsl_VerifyCallBack(int preverify_ok,
                                    X509_STORE_CTX *ctx);
DH *GWEN_NetLayerSsl_tmp_dh_callback(SSL *s, int is_export, int keylength);
void GWEN_NetLayerSsl__GenerateDhFile_Callback(int i, int j, void *p);

int GWEN_NetLayerSsl_GetPassword(char *buffer, int num, int rwflag,
                                 void *user_data);

char *GWEN_NetLayerSsl_GetCertEntry(X509_NAME *nm, int nid);
GWEN_TIME *GWEN_NetLayerSsl_ASN_GetTime(ASN1_TIME *d);

GWEN_SSLCERTDESCR *GWEN_NetLayerSsl_Cert2Descr(X509 *cert);

int GWEN_NetLayerSsl_SaveCert(GWEN_NETLAYER *nl,
                              X509 *cert,
                              const char *dir,
                              int overwrite);
int GWEN_NetLayerSsl_GetPublicCaFile(GWEN_BUFFER *pbuf);

void GWEN_NetLayerSsl_SetMode(GWEN_NETLAYER *nl, GWEN_NETLAYER_SSL_MODE m);




#endif /* GWEN_NL_SSL_P_H */



