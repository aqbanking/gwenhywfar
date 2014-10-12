/***************************************************************************
    begin       : Mon May 07 2012
    copyright   : (C) 2012 by Martin Preuss
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
  gcry_mpi_t modulus;
  gcry_mpi_t pubExponent;
  gcry_mpi_t privExponent;

  uint32_t flags;
};

static GWENHYWFAR_CB void GWEN_Crypt_KeyRsa_freeData(void *bp, void *p);


static int GWEN_Crypt_KeyRsa__getNamedElement(gcry_sexp_t pkey, const char *name, gcry_mpi_t *pMpi);
static int GWEN_Crypt_KeyRsa__ReadMpi(GWEN_DB_NODE *db, const char *dbName, gcry_mpi_t *pMpi);
static int GWEN_Crypt_KeyRsa__WriteMpi(GWEN_DB_NODE *db, const char *dbName, const gcry_mpi_t mpi);
static int GWEN_Crypt_KeyRsa__MpiToBuffer(const gcry_mpi_t mpi, unsigned char *buf, size_t nbytes);

static GWENHYWFAR_CB int GWEN_Crypt_KeyRsa_Sign(GWEN_CRYPT_KEY *k,
    const uint8_t *pInData,
    uint32_t inLen,
    uint8_t *pSignatureData,
    uint32_t *pSignatureLen);

static GWENHYWFAR_CB int GWEN_Crypt_KeyRsa_Verify(GWEN_CRYPT_KEY *k,
    const uint8_t *pInData,
    uint32_t inLen,
    const uint8_t *pSignatureData,
    uint32_t signatureLen);

static GWENHYWFAR_CB int GWEN_Crypt_KeyRsa_Encipher(GWEN_CRYPT_KEY *k,
    const uint8_t *pInData,
    uint32_t inLen,
    uint8_t *pOutData,
    uint32_t *pOutLen);

static GWENHYWFAR_CB int GWEN_Crypt_KeyRsa_Decipher(GWEN_CRYPT_KEY *k,
    const uint8_t *pInData,
    uint32_t inLen,
    uint8_t *pOutData,
    uint32_t *pOutLen);



#endif

