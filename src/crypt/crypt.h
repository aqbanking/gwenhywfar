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


#ifndef GWENHYWFAR_CRYPT_H
#define GWENHYWFAR_CRYPT_H


#include <gwenhywfar/error.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/keyspec.h>

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
#define GWEN_CRYPT_ERROR_UNSUPPORTED        9
#define GWEN_CRYPT_ERROR_SSL                10
#define GWEN_CRYPT_ERROR_GENERIC            11


int GWEN_Crypt_IsAvailable();


/** @name Functions expected in implementation
 *
 */
/*@{*/
/**
 * This function is expected to register all it's crypto service
 * providers. It is called when the GWEN_Crypt module is initialized.
 */
GWENHYWFAR_API
  GWEN_ERRORCODE GWEN_CryptImpl_Init();

/**
 * This function is called when the GWEN_Crypt module is deinitialized.
 * It can be used to release allocated data. It MUST NOT unregister any
 * crypt key type, this is done automatically.
 */
GWENHYWFAR_API
  GWEN_ERRORCODE GWEN_CryptImpl_Fini();
/*@}*/


typedef struct GWEN_CRYPTKEY GWEN_CRYPTKEY;

/** @name Function prototypes
 *
 */
/*@{*/
typedef GWEN_CRYPTKEY* (*GWEN_CRYPTKEY_DUP_FN)(const GWEN_CRYPTKEY *key);

typedef GWEN_ERRORCODE (*GWEN_CRYPTKEY_ENCRYPT_FN)(const GWEN_CRYPTKEY *key,
						     GWEN_BUFFER *src,
						     GWEN_BUFFER *dst);

typedef GWEN_ERRORCODE (*GWEN_CRYPTKEY_DECRYPT_FN)(const GWEN_CRYPTKEY *key,
						     GWEN_BUFFER *src,
						     GWEN_BUFFER *dst);

typedef GWEN_ERRORCODE (*GWEN_CRYPTKEY_SIGN_FN)(const GWEN_CRYPTKEY *key,
						  GWEN_BUFFER *src,
						  GWEN_BUFFER *dst);

typedef GWEN_ERRORCODE (*GWEN_CRYPTKEY_VERIFY_FN)(const GWEN_CRYPTKEY *key,
						    GWEN_BUFFER *src,
						    GWEN_BUFFER *dst);

typedef unsigned int
  (*GWEN_CRYPTKEY_GETCHUNKSIZE_FN)(const GWEN_CRYPTKEY *key);

typedef GWEN_ERRORCODE (*GWEN_CRYPTKEY_FROMDB_FN)(GWEN_CRYPTKEY *key,
						    GWEN_DB_NODE *db);

typedef GWEN_ERRORCODE (*GWEN_CRYPTKEY_TODB_FN)(const GWEN_CRYPTKEY *key,
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
GWENHYWFAR_API
  GWEN_CRYPTKEY *GWEN_CryptKey_new();


GWENHYWFAR_API
  GWEN_CRYPTKEY *GWEN_CryptKey_Factory(const char *t);

GWENHYWFAR_API
  void GWEN_CryptKey_free(GWEN_CRYPTKEY *key);

GWENHYWFAR_API
  GWEN_CRYPTKEY *GWEN_CryptKey_dup(const GWEN_CRYPTKEY *key);

GWENHYWFAR_API
  int GWEN_CryptKey_IsPublic(const GWEN_CRYPTKEY *key);
GWENHYWFAR_API
  void GWEN_CryptKey_SetPublic(GWEN_CRYPTKEY *key, int i);

GWENHYWFAR_API
  GWEN_ERRORCODE GWEN_CryptKey_Encrypt(const GWEN_CRYPTKEY *key,
				       GWEN_BUFFER *src,
				       GWEN_BUFFER *dst);

GWENHYWFAR_API
  GWEN_ERRORCODE GWEN_CryptKey_Decrypt(const GWEN_CRYPTKEY *key,
				       GWEN_BUFFER *src,
				       GWEN_BUFFER *dst);

GWENHYWFAR_API
  GWEN_ERRORCODE GWEN_CryptKey_Sign(const GWEN_CRYPTKEY *key,
				    GWEN_BUFFER *src,
				    GWEN_BUFFER *dst);

GWENHYWFAR_API
  GWEN_ERRORCODE GWEN_CryptKey_Verify(const GWEN_CRYPTKEY *key,
				      GWEN_BUFFER *src,
				      GWEN_BUFFER *dst);

GWENHYWFAR_API
  unsigned int GWEN_CryptKey_GetChunkSize(const GWEN_CRYPTKEY *key);

GWENHYWFAR_API
  GWEN_CRYPTKEY *GWEN_CryptKey_FromDb(GWEN_DB_NODE *db);

GWENHYWFAR_API
  GWEN_ERRORCODE GWEN_CryptKey_ToDb(const GWEN_CRYPTKEY *key,
				    GWEN_DB_NODE *db,
				    int pub);

/**
 * This is a conveniance function.
 * Sets the key data. This can be used by the application for keys which
 * use the variable "keydata" when calling @ref GWEN_CryptKey_FromDb (such
 * as most symmetric keys).
 */
GWENHYWFAR_API
  GWEN_ERRORCODE GWEN_CryptKey_SetData(GWEN_CRYPTKEY *key,
				       const char *buffer,
				       unsigned int bsize);

/**
 * Creates a password from a given string. Depending on the size needed
 * a hash-algorithm is applied to the given password.
 */
int GWEN_CryptKey_FromPassword(const char *password,
			       unsigned char *buffer,
			       unsigned int bsize);

/**
 * This is a conveniance function.
 * Gets the key data. This can be used by the application for keys which
 * use the variable "keydata" when calling @ref GWEN_CryptKey_ToDb (such
 * as most symmetric keys).
 */
GWEN_ERRORCODE GWEN_CryptKey_GetData(GWEN_CRYPTKEY *key,
				     char *buffer,
				     unsigned int *bsize);


GWENHYWFAR_API
  GWEN_ERRORCODE GWEN_CryptKey_Generate(GWEN_CRYPTKEY *key,
					unsigned int keylength);

GWENHYWFAR_API
  GWEN_ERRORCODE GWEN_CryptKey_Open(GWEN_CRYPTKEY *key);

GWENHYWFAR_API
  GWEN_ERRORCODE GWEN_CryptKey_Close(GWEN_CRYPTKEY *key);

/**
 * Returns a pointer to the key's description.
 * You MUST NOT free the returned pointer.
 */
GWENHYWFAR_API
  const GWEN_KEYSPEC *GWEN_CryptKey_GetKeySpec(const GWEN_CRYPTKEY *key);

GWENHYWFAR_API
  const char *GWEN_CryptKey_GetKeyType(const GWEN_CRYPTKEY *key);
GWENHYWFAR_API
  void GWEN_CryptKey_SetKeyType(GWEN_CRYPTKEY *key,
				const char *s);

GWENHYWFAR_API
  const char *GWEN_CryptKey_GetKeyName(const GWEN_CRYPTKEY *key);
GWENHYWFAR_API
  void GWEN_CryptKey_SetKeyName(GWEN_CRYPTKEY *key,
				const char *s);

GWENHYWFAR_API
  const char *GWEN_CryptKey_GetOwner(const GWEN_CRYPTKEY *key);
GWENHYWFAR_API
  void GWEN_CryptKey_SetOwner(GWEN_CRYPTKEY *key,
			      const char *s);
GWENHYWFAR_API
  unsigned int GWEN_CryptKey_GetNumber(const GWEN_CRYPTKEY *key);
GWENHYWFAR_API
  void GWEN_CryptKey_SetNumber(GWEN_CRYPTKEY *key,
			       unsigned int i);
GWENHYWFAR_API
  unsigned int GWEN_CryptKey_GetVersion(const GWEN_CRYPTKEY *key);
GWENHYWFAR_API
  void GWEN_CryptKey_SetVersion(GWEN_CRYPTKEY *key,
				unsigned int i);

#if (defined(GWEN_EXTEND_CRYPTKEY) || defined(DOXYGEN))
/**
 * This function is only available if you define <i>GWEN_EXTEND_CRYPTKEY</i>
 * to protect it against careless calls by the application.
 * You should only define <i>GWEN_EXTEND_CRYPTKEY</i> if you actually are
 * extending this "class".
 * This makes this function kinda like <i>protected</i> in C++.
 */
GWENHYWFAR_API
  void *GWEN_CryptKey_GetKeyData(const GWEN_CRYPTKEY *key);

/**
 * This function is only available if you define <i>GWEN_EXTEND_CRYPTKEY</i>
 * to protect it against careless calls by the application.
 * You should only define <i>GWEN_EXTEND_CRYPTKEY</i> if you actually are
 * extending this "class".
 * This makes this function kinda like <i>protected</i> in C++.
 */
GWENHYWFAR_API
  void GWEN_CryptKey_SetKeyData(GWEN_CRYPTKEY *key,
				void *kd);
#endif

GWENHYWFAR_API
  int GWEN_CryptKey_GetOpenCount(const GWEN_CRYPTKEY *key);

#if (defined(GWEN_EXTEND_CRYPTKEY) || defined(DOXYGEN))
GWENHYWFAR_API
  void GWEN_CryptKey_IncrementOpenCount(GWEN_CRYPTKEY *key);
GWENHYWFAR_API
  void GWEN_CryptKey_DecrementOpenCount(GWEN_CRYPTKEY *key);
#endif

/** @name Function Setter
 *
 */
/*@{*/
#if (defined(GWEN_EXTEND_CRYPTKEY) || defined(DOXYGEN))
GWENHYWFAR_API
  void GWEN_CryptKey_SetEncryptFn(GWEN_CRYPTKEY *key,
				  GWEN_CRYPTKEY_ENCRYPT_FN encryptFn);
GWENHYWFAR_API
  void GWEN_CryptKey_SetDecryptFn(GWEN_CRYPTKEY *key,
				  GWEN_CRYPTKEY_DECRYPT_FN decryptFn);
GWENHYWFAR_API
  void GWEN_CryptKey_SetSignFn(GWEN_CRYPTKEY *key,
			       GWEN_CRYPTKEY_SIGN_FN signFn);
GWENHYWFAR_API
  void GWEN_CryptKey_SetVerifyFn(GWEN_CRYPTKEY *key,
				 GWEN_CRYPTKEY_VERIFY_FN verifyFn);
GWENHYWFAR_API
  void GWEN_CryptKey_SetGetChunkSizeFn(GWEN_CRYPTKEY *key,
				       GWEN_CRYPTKEY_GETCHUNKSIZE_FN getChunkSizeFn);
GWENHYWFAR_API
  void GWEN_CryptKey_SetFromDbFn(GWEN_CRYPTKEY *key,
				 GWEN_CRYPTKEY_FROMDB_FN fromDbFn);
GWENHYWFAR_API
  void GWEN_CryptKey_SetToDbFn(GWEN_CRYPTKEY *key,
			       GWEN_CRYPTKEY_TODB_FN toDbFn);
GWENHYWFAR_API
  void GWEN_CryptKey_SetGenerateKeyFn(GWEN_CRYPTKEY *key,
				      GWEN_CRYPTKEY_GENERATEKEY_FN generateKeyFn);
GWENHYWFAR_API
  void GWEN_CryptKey_SetFreeKeyDataFn(GWEN_CRYPTKEY *key,
				      GWEN_CRYPTKEY_FREEKEYDATA_FN freeKeyDataFn);

GWENHYWFAR_API
  void GWEN_CryptKey_SetOpenFn(GWEN_CRYPTKEY *key,
			       GWEN_CRYPTKEY_OPEN_FN openKeyFn);
GWENHYWFAR_API
  void GWEN_CryptKey_SetCloseFn(GWEN_CRYPTKEY *key,
				GWEN_CRYPTKEY_CLOSE_FN closeKeyFn);
GWENHYWFAR_API
  void GWEN_CryptKey_SetDupFn(GWEN_CRYPTKEY *key,
			      GWEN_CRYPTKEY_DUP_FN dupFn);
#endif
/*@}*/





  /* _______________________________________________________________________
   * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
   *                           Crypt provider (plugin)
   * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
   */

typedef struct GWEN_CRYPTKEY_PROVIDER GWEN_CRYPTKEY_PROVIDER;

typedef GWEN_CRYPTKEY*
    (*GWEN_CRYPTPROVIDER_NEWKEY_FN)(GWEN_CRYPTKEY_PROVIDER *pr);

GWENHYWFAR_API
  GWEN_CRYPTKEY_PROVIDER *GWEN_CryptProvider_new();
GWENHYWFAR_API
  void GWEN_CryptProvider_free(GWEN_CRYPTKEY_PROVIDER *pr);

GWENHYWFAR_API
  void GWEN_CryptProvider_SetNewKeyFn(GWEN_CRYPTKEY_PROVIDER *pr,
                                      GWEN_CRYPTPROVIDER_NEWKEY_FN newKeyFn);
GWENHYWFAR_API
  void GWEN_CryptProvider_SetName(GWEN_CRYPTKEY_PROVIDER *pr,
                                  const char *name);

  /**
   * On success this function takes over ownership of the given object.
   */
GWENHYWFAR_API
  GWEN_ERRORCODE GWEN_Crypt_RegisterProvider(GWEN_CRYPTKEY_PROVIDER *pr);
GWENHYWFAR_API
  GWEN_ERRORCODE GWEN_Crypt_UnregisterProvider(GWEN_CRYPTKEY_PROVIDER *pr);

GWENHYWFAR_API
  void GWEN_Crypt_UnregisterAllProviders();


#ifdef __cplusplus
}
#endif



#endif
