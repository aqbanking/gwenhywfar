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


#ifndef GWENHYWFAR_CRYPTSSL_RSA_P_H
#define GWENHYWFAR_CRYPTSSL_RSA_P_H


#define GWEN_CRYPT_RSA_DEFAULT_KEY_LENGTH  1024
#define GWEN_CRYPT_RSA_MAX_KEY_LENGTH      4096
#define GWEN_CRYPT_RSA_DEFAULT_EXPONENT    65537
#define GWEN_CRYPT_RSA_NAME "RSA"

#include <gwenhywfar/crypt.h>
#include <openssl/rsa.h>
#include <openssl/objects.h>


GWEN_ERRORCODE GWEN_CryptKeyRSA_Register();


GWEN_CRYPTKEY *GWEN_CryptKeyRSA_new();
GWEN_CRYPTKEY *GWEN_CryptKeyRSA_dup(const GWEN_CRYPTKEY *key);
void GWEN_CryptKeyRSA_FreeKeyData(GWEN_CRYPTKEY *key);
GWEN_ERRORCODE GWEN_CryptKeyRSA_Encrypt(const GWEN_CRYPTKEY *key,
                                        GWEN_BUFFER *src,
                                        GWEN_BUFFER *dst);
GWEN_ERRORCODE GWEN_CryptKeyRSA_Decrypt(const GWEN_CRYPTKEY *key,
                                        GWEN_BUFFER *src,
                                        GWEN_BUFFER *dst);
GWEN_ERRORCODE GWEN_CryptKeyRSA_Sign(const GWEN_CRYPTKEY *key,
                                     GWEN_BUFFER *src,
                                     GWEN_BUFFER *dst);
GWEN_ERRORCODE GWEN_CryptKeyRSA_Verify(const GWEN_CRYPTKEY *key,
                                       GWEN_BUFFER *src,
                                       GWEN_BUFFER *signature);
unsigned int GWEN_CryptKeyRSA_GetChunkSize(const GWEN_CRYPTKEY *key);
GWEN_ERRORCODE GWEN_CryptKeyRSA_FromDb(GWEN_CRYPTKEY *key,
                                       GWEN_DB_NODE *db);
GWEN_ERRORCODE GWEN_CryptKeyRSA_ToDb(const GWEN_CRYPTKEY *key,
                                     GWEN_DB_NODE *db,
                                     int pub);
GWEN_ERRORCODE GWEN_CryptKeyRSA_Generate(GWEN_CRYPTKEY *key,
                                         unsigned keylength);
GWEN_ERRORCODE GWEN_CryptKeyRSA_Open(GWEN_CRYPTKEY *key);
GWEN_ERRORCODE GWEN_CryptKeyRSA_Close(GWEN_CRYPTKEY *key);


GWEN_ERRORCODE GWEN_CryptKeyRSA_SignBigNum(const GWEN_CRYPTKEY *key,
                                           GWEN_BUFFER *src,
                                           BIGNUM *bnresult);

void GWEN_CryptKeyRSA_DumpPubKey(const GWEN_CRYPTKEY *key);





#endif


