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


#ifndef GWENHYFWAR_CRYPT_H
#define GWENHYFWAR_CRYPT_H


#include <gwenhyfwar/error.h>
#include <gwenhyfwar/buffer.h>
#include <gwenhyfwar/db.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GWEN_CRYPT_ERROR_TYPE "Crypt"
#define GWEN_CRYPT_ERROR_ALREADY_REGISTERED 1
#define GWEN_CRYPT_ERROR_NOT_REGISTERED     2
#define GWEN_CRYPT_ERROR_BAD_SIZE           3
#define GWEN_CRYPT_ERROR_BUFFER_FULL        4
#define GWEN_CRYPT_ERROR_ENCRYPT            5
#define GWEN_CRYPT_ERROR_DECRYPT            6
#define GWEN_CRYPT_ERROR_SIGN               7
#define GWEN_CRYPT_ERROR_VERIFY             8


  GWEN_ERRORCODE GWEN_Crypt_ModuleInit();
  GWEN_ERRORCODE GWEN_Crypt_ModuleFini();


  /** @name Functions expected in implementation
   *
   */
  /*@{*/
  /**
   * This function is expected to register all it's crypto service
   * providers. It is called when the GWEN_Crypt module is initialized.
   */
  GWEN_ERRORCODE GWEN_CryptImpl_Init();

  /**
   * This function is called when the GWEN_Crypt module is deinitialized.
   * It can be used to release allocated data. It MUST NOT unregister any
   * crypt key type, this is done automatically.
   */
  GWEN_ERRORCODE GWEN_CryptImpl_Fini();
  /*@}*/


  typedef struct GWEN_CRYPTKEY GWEN_CRYPTKEY;

  /** @name Function prototypes
   *
   */
  /*@{*/
  typedef GWEN_CRYPTKEY* (*GWEN_CRYPTKEY_DUP_FN)(GWEN_CRYPTKEY *key);
  typedef GWEN_ERRORCODE (*GWEN_CRYPTKEY_ENCRYPT_FN)(GWEN_CRYPTKEY *key,
                                                     GWEN_BUFFER *src,
                                                     GWEN_BUFFER *dst);

  typedef GWEN_ERRORCODE (*GWEN_CRYPTKEY_DECRYPT_FN)(GWEN_CRYPTKEY *key,
                                                     GWEN_BUFFER *src,
                                                     GWEN_BUFFER *dst);

  typedef GWEN_ERRORCODE (*GWEN_CRYPTKEY_SIGN_FN)(GWEN_CRYPTKEY *key,
                                                  GWEN_BUFFER *src,
                                                  GWEN_BUFFER *dst);

  typedef GWEN_ERRORCODE (*GWEN_CRYPTKEY_VERIFY_FN)(GWEN_CRYPTKEY *key,
                                                    GWEN_BUFFER *src,
                                                    GWEN_BUFFER *dst);

  typedef unsigned int (*GWEN_CRYPTKEY_GETCHUNKSIZE_FN)(GWEN_CRYPTKEY *key);

  typedef GWEN_ERRORCODE (*GWEN_CRYPTKEY_FROMDB_FN)(GWEN_CRYPTKEY *key,
                                                    GWEN_DB_NODE *db);

  typedef GWEN_ERRORCODE (*GWEN_CRYPTKEY_TODB_FN)(GWEN_CRYPTKEY *key,
                                               GWEN_DB_NODE *db,
                                               int pub);

  typedef void (*GWEN_CRYPTKEY_FREEKEYDATA_FN)(GWEN_CRYPTKEY *key);

  typedef GWEN_ERRORCODE (*GWEN_CRYPTKEY_GENERATEKEY_FN)(GWEN_CRYPTKEY *key,
                                                         unsigned keylength);

  typedef GWEN_ERRORCODE (*GWEN_CRYPTKEY_OPEN_FN)(GWEN_CRYPTKEY *key);

  typedef GWEN_ERRORCODE (*GWEN_CRYPTKEY_CLOSE_FN)(GWEN_CRYPTKEY *key);
  /*@}*/



  /**
   * @internal
   */
  GWEN_CRYPTKEY *GWEN_CryptKey_new();


  GWEN_CRYPTKEY *GWEN_CryptKey_Factory(const char *t);

  void GWEN_CryptKey_free(GWEN_CRYPTKEY *key);
  GWEN_CRYPTKEY *GWEN_CryptKey_dup(GWEN_CRYPTKEY *key);

  GWEN_ERRORCODE GWEN_CryptKey_Encrypt(GWEN_CRYPTKEY *key,
                                       GWEN_BUFFER *src,
                                       GWEN_BUFFER *dst);

  GWEN_ERRORCODE GWEN_CryptKey_Decrypt(GWEN_CRYPTKEY *key,
                                       GWEN_BUFFER *src,
                                       GWEN_BUFFER *dst);

  GWEN_ERRORCODE GWEN_CryptKey_Sign(GWEN_CRYPTKEY *key,
                                    GWEN_BUFFER *src,
                                    GWEN_BUFFER *dst);

  GWEN_ERRORCODE GWEN_CryptKey_Verify(GWEN_CRYPTKEY *key,
                                      GWEN_BUFFER *src,
                                      GWEN_BUFFER *dst);

  unsigned int GWEN_CryptKey_GetChunkSize(GWEN_CRYPTKEY *key);

  GWEN_ERRORCODE GWEN_CryptKey_FromDb(GWEN_CRYPTKEY *key,
                                      GWEN_DB_NODE *db);

  GWEN_ERRORCODE GWEN_CryptKey_ToDb(GWEN_CRYPTKEY *key,
                                    GWEN_DB_NODE *db,
                                    int pub);

  GWEN_ERRORCODE GWEN_CryptKey_Generate(GWEN_CRYPTKEY *key,
                                        unsigned int keylength);

  GWEN_ERRORCODE GWEN_CryptKey_Open(GWEN_CRYPTKEY *key);

  GWEN_ERRORCODE GWEN_CryptKey_Close(GWEN_CRYPTKEY *key);

  const char *GWEN_CryptKey_GetKeyType(GWEN_CRYPTKEY *key);
  void GWEN_CryptKey_SetKeyType(GWEN_CRYPTKEY *key,
                                const char *s);

  const char *GWEN_CryptKey_GetKeyName(GWEN_CRYPTKEY *key);
  void GWEN_CryptKey_SetKeyName(GWEN_CRYPTKEY *key,
                                const char *s);

  const char *GWEN_CryptKey_GetOwner(GWEN_CRYPTKEY *key);
  void GWEN_CryptKey_SetOwner(GWEN_CRYPTKEY *key,
                              const char *s);
  unsigned int GWEN_CryptKey_GetNumber(GWEN_CRYPTKEY *key);
  void GWEN_CryptKey_SetNumber(GWEN_CRYPTKEY *key,
                               unsigned int i);
  unsigned int GWEN_CryptKey_GetVersion(GWEN_CRYPTKEY *key);
  void GWEN_CryptKey_SetVersion(GWEN_CRYPTKEY *key,
                                unsigned int i);
  void *GWEN_CryptKey_GetKeyData(GWEN_CRYPTKEY *key);
  void GWEN_CryptKey_SetKeyData(GWEN_CRYPTKEY *key,
                                void *kd);
  int GWEN_CryptKey_GetOpenCount(GWEN_CRYPTKEY *key);
  void GWEN_CryptKey_IncrementOpenCount(GWEN_CRYPTKEY *key);
  void GWEN_CryptKey_DecrementOpenCount(GWEN_CRYPTKEY *key);


  /** @name Function Setter
   *
   */
  /*@{*/
  void GWEN_CryptKey_SetEncryptFn(GWEN_CRYPTKEY *key,
                                  GWEN_CRYPTKEY_ENCRYPT_FN encryptFn);
  void GWEN_CryptKey_SetDecryptFn(GWEN_CRYPTKEY *key,
                                  GWEN_CRYPTKEY_DECRYPT_FN decryptFn);
  void GWEN_CryptKey_SetSignFn(GWEN_CRYPTKEY *key,
                               GWEN_CRYPTKEY_SIGN_FN signFn);
  void GWEN_CryptKey_SetVerifyFn(GWEN_CRYPTKEY *key,
                                 GWEN_CRYPTKEY_VERIFY_FN verifyFn);
  void GWEN_CryptKey_SetGetChunkSizeFn(GWEN_CRYPTKEY *key,
                                       GWEN_CRYPTKEY_GETCHUNKSIZE_FN getChunkSizeFn);
  void GWEN_CryptKey_SetFromDbFn(GWEN_CRYPTKEY *key,
                                 GWEN_CRYPTKEY_FROMDB_FN fromDbFn);
  void GWEN_CryptKey_SetToDbFn(GWEN_CRYPTKEY *key,
                               GWEN_CRYPTKEY_TODB_FN toDbFn);
  void GWEN_CryptKey_SetGenerateKeyFn(GWEN_CRYPTKEY *key,
                        GWEN_CRYPTKEY_GENERATEKEY_FN generateKeyFn);
  void GWEN_CryptKey_SetFreeKeyDataFn(GWEN_CRYPTKEY *key,
                                      GWEN_CRYPTKEY_FREEKEYDATA_FN freeKeyDataFn);

  void GWEN_CryptKey_SetOpenFn(GWEN_CRYPTKEY *key,
                               GWEN_CRYPTKEY_OPEN_FN openKeyFn);
  void GWEN_CryptKey_SetCloseFn(GWEN_CRYPTKEY *key,
                                GWEN_CRYPTKEY_CLOSE_FN closeKeyFn);
  void GWEN_CryptKey_SetDupFn(GWEN_CRYPTKEY *key,
                              GWEN_CRYPTKEY_DUP_FN dupFn);
  /*@}*/





  /* _______________________________________________________________________
   * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
   *                           Crypt provider (plugin)
   * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
   */

  typedef struct GWEN_CRYPTKEY_PROVIDER GWEN_CRYPTKEY_PROVIDER;

  typedef GWEN_CRYPTKEY*
    (*GWEN_CRYPTPROVIDER_NEWKEY_FN)(GWEN_CRYPTKEY_PROVIDER *pr);

  GWEN_CRYPTKEY_PROVIDER *GWEN_CryptProvider_new();
  void GWEN_CryptProvider_free(GWEN_CRYPTKEY_PROVIDER *pr);

  void GWEN_CryptProvider_SetNewKeyFn(GWEN_CRYPTKEY_PROVIDER *pr,
                                      GWEN_CRYPTPROVIDER_NEWKEY_FN newKeyFn);
  void GWEN_CryptProvider_SetName(GWEN_CRYPTKEY_PROVIDER *pr,
                                  const char *name);

  /**
   * On success this function takes over ownership of the given object.
   */
  GWEN_ERRORCODE GWEN_Crypt_RegisterProvider(GWEN_CRYPTKEY_PROVIDER *pr);
  GWEN_ERRORCODE GWEN_Crypt_UnregisterProvider(GWEN_CRYPTKEY_PROVIDER *pr);

  void GWEN_Crypt_UnregisterAllProviders();


#ifdef __cplusplus
}
#endif



#endif
