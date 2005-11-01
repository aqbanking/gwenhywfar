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


#ifndef GWEN_NL_SSL_H
#define GWEN_NL_SSL_H


#define GWEN_NL_SSL_NAME "SSL"

#include <gwenhywfar/netlayer.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/ssl_cipher.h>
#include <gwenhywfar/ssl_cert_descr.h>


typedef enum {
  /** error asking the user */
  GWEN_NetLayerSsl_AskAddCertResult_Error=0,
  /** user does not trust the given certificate */
  GWEN_NetLayerSsl_AskAddCertResult_No,
  /** user trusts the given certificate for this session only */
  GWEN_NetLayerSsl_AskAddCertResult_Tmp,
  /** user trusts the given certificate permanently */
  GWEN_NetLayerSsl_AskAddCertResult_Perm,
  /** store the cert in an incoming directory, trust it for this session  */
  GWEN_NetLayerSsl_AskAddCertResult_Incoming
} GWEN_NL_SSL_ASKADDCERT_RESULT;


typedef GWEN_NL_SSL_ASKADDCERT_RESULT
(*GWEN_NL_SSL_ASKADDCERT_FN)(GWEN_NETLAYER *nl,
                             const GWEN_SSLCERTDESCR *cert,
                             void *user_data);

GWENHYWFAR_API
void GWEN_NetLayerSsl_SetAskAddCertFn(GWEN_NETLAYER *nl,
                                      GWEN_NL_SSL_ASKADDCERT_FN fn,
                                      void *user_data);


GWEN_NL_SSL_ASKADDCERT_RESULT
  GWEN_NetLayerSsl_AskAddCert(GWEN_NETLAYER *nl,
                              const GWEN_SSLCERTDESCR *cd);


GWEN_NETLAYER *GWEN_NetLayerSsl_new(GWEN_NETLAYER *baseLayer,
                                    const char *capath,
                                    const char *newcapath,
                                    const char *ownCertFile,
                                    const char *dhFolder,
                                    int secure);



void GWEN_NetLayerSsl_SetCiphers(GWEN_NETLAYER *nl, const char *ciphers);
GWEN_SSLCIPHER_LIST *GWEN_NetLayerSsl_GetCipherList();

int GWEN_NetLayerSsl_GenerateDhFile(const char *fname, int bits);

int GWEN_NetLayerSsl_GenerateCertAndKeyFile(const char *fname,
                                            int bits,
                                            int serial,
                                            int days,
                                            const GWEN_SSLCERTDESCR *cd);



#endif /* GWEN_NL_SSL_H */


