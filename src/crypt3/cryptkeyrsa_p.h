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


#ifndef GWEN_CRYPT_KEY_RSA_P_H
#define GWEN_CRYPT_KEY_RSA_P_H

#include "cryptkeyrsa.h"
#include "cryptkey_be.h"

#include <gcrypt.h>


typedef struct GWEN_CRYPT_KEY_RSA GWEN_CRYPT_KEY_RSA;
struct GWEN_CRYPT_KEY_RSA {
  int pub;
  int algoValid;
  gcry_ac_handle_t algoHandle;
  int keyValid;
  gcry_ac_key_t key;
};

static GWENHYWFAR_CB void GWEN_Crypt_KeyRsa_freeData(void *bp, void *p);

static int GWEN_Crypt_KeyRsa__ReadMpi(GWEN_DB_NODE *db,
				      const char *dbName,
				      gcry_ac_data_t ds,
				      const char *dsName);
static int GWEN_Crypt_KeyRsa__WriteMpi(GWEN_DB_NODE *db,
				       const char *dbName,
				       gcry_ac_data_t ds,
				       const char *dsName);
static int GWEN_Crypt_KeyRsa__DataFromDb(GWEN_DB_NODE *db, gcry_ac_data_t *pData,
					 int pub, unsigned int nbits);

static int GWEN_Crypt_KeyRsa__GetNamedElement(const GWEN_CRYPT_KEY *k,
					      const char *name,
					      uint8_t *buffer,
					      uint32_t *pBufLen);


static int GWEN_Crypt_KeyRsa__sKeyElementToData(gcry_ac_data_t data,
						gcry_sexp_t sx,
						const char *name);

static int GWEN_Crypt_KeyRsa__sKeyToDataPubKey(gcry_ac_data_t data,
					       gcry_sexp_t sx);


static int GWEN_Crypt_KeyRsa__sKeyToDataPrivKey(gcry_ac_data_t data,
						gcry_sexp_t sx);


#endif

