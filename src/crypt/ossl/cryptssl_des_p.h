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


#ifndef GWENHYWFAR_CRYPTSSL_DES_P_H
#define GWENHYWFAR_CRYPTSSL_DES_P_H


#define GWEN_CRYPT_DES_DEFAULT_KEY_LENGTH  1024
#define GWEN_CRYPT_DES_MAX_KEY_LENGTH      4096
#define GWEN_CRYPT_DES_NAME "DES"

#include <gwenhywfar/crypt.h>
#include <openssl/des.h>
#include <openssl/objects.h>


GWEN_ERRORCODE GWEN_CryptKeyDES_Register();


GWEN_CRYPTKEY *GWEN_CryptKeyDES_new();
GWEN_CRYPTKEY *GWEN_CryptKeyDES_dup(const GWEN_CRYPTKEY *key);
void GWEN_CryptKeyDES_FreeKeyData(GWEN_CRYPTKEY *key);
GWEN_ERRORCODE GWEN_CryptKeyDES_Encrypt(const GWEN_CRYPTKEY *key,
                                        GWEN_BUFFER *src,
                                        GWEN_BUFFER *dst);
GWEN_ERRORCODE GWEN_CryptKeyDES_Decrypt(const GWEN_CRYPTKEY *key,
                                        GWEN_BUFFER *src,
                                        GWEN_BUFFER *dst);
GWEN_ERRORCODE GWEN_CryptKeyDES_Sign(const GWEN_CRYPTKEY *key,
                                     GWEN_BUFFER *src,
                                     GWEN_BUFFER *dst);
GWEN_ERRORCODE GWEN_CryptKeyDES_Verify(const GWEN_CRYPTKEY *key,
                                       GWEN_BUFFER *src,
                                       GWEN_BUFFER *signature);
unsigned int GWEN_CryptKeyDES_GetChunkSize(const GWEN_CRYPTKEY *key);
GWEN_ERRORCODE GWEN_CryptKeyDES_FromDb(GWEN_CRYPTKEY *key,
                                       GWEN_DB_NODE *db);
GWEN_ERRORCODE GWEN_CryptKeyDES_ToDb(const GWEN_CRYPTKEY *key,
                                     GWEN_DB_NODE *db,
                                     int pub);
GWEN_ERRORCODE GWEN_CryptKeyDES_Generate(GWEN_CRYPTKEY *key,
                                         unsigned keylength);
GWEN_ERRORCODE GWEN_CryptKeyDES_Open(GWEN_CRYPTKEY *key);
GWEN_ERRORCODE GWEN_CryptKeyDES_Close(GWEN_CRYPTKEY *key);


GWEN_ERRORCODE GWEN_CryptKeyDES_Crypt(const GWEN_CRYPTKEY *key,
                                      GWEN_BUFFER *src,
                                      GWEN_BUFFER *dst,
                                      int cryptMode);






#endif


