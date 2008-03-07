/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: crypttoken.h 1113 2007-01-10 09:14:16Z martin $
    begin       : Wed Mar 16 2005
    copyright   : (C) 2005 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_CRYPT_KEY_SYM_P_H
#define GWEN_CRYPT_KEY_SYM_P_H

#include "cryptkeysym.h"
#include "cryptkey_be.h"

#include <gcrypt.h>


typedef struct GWEN_CRYPT_KEY_SYM GWEN_CRYPT_KEY_SYM;
struct GWEN_CRYPT_KEY_SYM {
  int algoValid;
  gcry_cipher_hd_t algoHandle;
  GWEN_CRYPT_CRYPTMODE mode;
  int algo;
  uint8_t *keyData;
  uint32_t keyLen;
};

static GWENHYWFAR_CB void GWEN_Crypt_KeySym_freeData(void *bp, void *p);

static int GWEN_Crypt_KeySym_Encipher(GWEN_CRYPT_KEY *k,
				      const uint8_t *pInData,
				      uint32_t inLen,
				      uint8_t *pOutData,
				      uint32_t *pOutLen);
static int GWEN_Crypt_KeySym_Decipher(GWEN_CRYPT_KEY *k,
				      const uint8_t *pInData,
				      uint32_t inLen,
				      uint8_t *pOutData,
				      uint32_t *pOutLen);

static enum gcry_cipher_modes GWEN_Crypt_KeySym__MyMode2GMode(GWEN_CRYPT_CRYPTMODE mode);

static GWEN_CRYPT_KEY *GWEN_Crypt_KeySym_Generate(GWEN_CRYPT_CRYPTALGOID cryptAlgoId, int keySize,
						  GWEN_CRYPT_CRYPTMODE mode,
						  int algo,
						  unsigned int flags,
						  int quality);

static GWEN_CRYPT_KEY *GWEN_Crypt_KeySym_fromDb(GWEN_CRYPT_CRYPTALGOID cryptAlgoId,
						GWEN_CRYPT_CRYPTMODE mode,
						int algo,
						unsigned int flags,
						const char *gname,
						GWEN_DB_NODE *db);

static GWEN_CRYPT_KEY *GWEN_Crypt_KeySym_fromData(GWEN_CRYPT_CRYPTALGOID cryptAlgoId, int keySize,
						  GWEN_CRYPT_CRYPTMODE mode,
						  int algo,
						  unsigned int flags,
						  const uint8_t *kd, uint32_t kl);

static int GWEN_Crypt_KeySym_toDb(const GWEN_CRYPT_KEY *k, GWEN_DB_NODE *db, const char *gname);

static int GWEN_Crypt_KeySym_SetKeyData(GWEN_CRYPT_KEY *k, const uint8_t *kd, uint32_t kl);



#endif

