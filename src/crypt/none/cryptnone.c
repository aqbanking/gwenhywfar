/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Thu Nov 06 2003
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


#include "cryptnone_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/nettransportssl.h>


GWEN_ERRORCODE GWEN_CryptImpl_Init(){
  return 0;
}



GWEN_ERRORCODE GWEN_CryptImpl_Fini(){
  return 0;
}


int GWEN_Crypt_IsAvailable() {
  return 0;
}



GWEN_NETTRANSPORT *GWEN_NetTransportSSL_new(GWEN_SOCKET *sk,
                                            const char *capath,
                                            const char *ownCertFile,
					    const char *dhfile,
                                            int secure,
                                            int relinquish){
  DBG_ERROR(0, "Crypto functions not supported");
  return 0;
}



void
GWEN_NetTransportSSL_SetGetPasswordFn(GWEN_NETTRANSPORTSSL_GETPASSWD_FN fn){
}



GWEN_NETTRANSPORTSSL_GETPASSWD_FN
GWEN_NetTransportSSL_GetGetPasswordFn(){
  DBG_ERROR(0, "Crypto functions not supported");
  return 0;
}



void
GWEN_NetTransportSSL_SetAskAddCertFn(GWEN_NETTRANSPORTSSL_ASKADDCERT_FN fn){
  DBG_ERROR(0, "Crypto functions not supported");
}



GWEN_NETTRANSPORTSSL_ASKADDCERT_FN
GWEN_NetTransportSSL_GetAskAddCertFn(){
  DBG_ERROR(0, "Crypto functions not supported");
  return 0;
}



void GWEN_NetTransportSSL_SetCipherList(GWEN_NETTRANSPORT *tr,
                                        const char *ciphers){
  DBG_ERROR(0, "Crypto functions not supported");
}



GWEN_DB_NODE *GWEN_NetTransportSSL_GetCipherList(){
  DBG_ERROR(0, "Crypto functions not supported");
  return 0;
}



int GWEN_NetTransportSSL_GenerateCertAndKeyFile(const char *fname,
                                                int bits,
                                                int serial,
                                                int days,
                                                GWEN_DB_NODE *db) {
  DBG_ERROR(0, "Crypto functions not supported");
  return -1;
}



GWEN_DB_NODE*
GWEN_NetTransportSSL_GetPeerCertificate(const GWEN_NETTRANSPORT *tr){
  DBG_ERROR(0, "Crypto functions not supported");
  return 0;
}



int GWEN_NetTransportSSL_IsSecure(const GWEN_NETTRANSPORT *tr){
  DBG_ERROR(0, "Crypto functions not supported");
  return 0;
}










