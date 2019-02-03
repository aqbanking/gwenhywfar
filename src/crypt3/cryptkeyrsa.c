/***************************************************************************
    begin       : Wed Mar 16 2005
    copyright   : (C) 2005 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define DISABLE_DEBUGLOG


#include "cryptkeyrsa_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>




GWEN_INHERIT(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_RSA)




#if 0
static void dumpKeyData(gcry_ac_data_t data)
{
  int i;
  unsigned int l;

  l=gcry_ac_data_length(data);
  for (i=0; i<l; i++) {
    const char *dname;
    gcry_mpi_t mpi;
    gcry_error_t err;
    unsigned char *buf;
    size_t nbytes;

    gcry_ac_data_get_index(data, 0, i, &dname, &mpi);
    fprintf(stderr, "%3d: [%s]\n", i, dname);

    /* write mpi as bin into a buffer which will be allocates by this function */
    err=gcry_mpi_aprint(GCRYMPI_FMT_USG, &buf, &nbytes, mpi);
    if (err) {
      DBG_INFO(GWEN_LOGDOMAIN, "gcry_mpi_aprint(): %d", err);
    }
    else {
      GWEN_Text_DumpString((const char *)buf, nbytes, stderr, 6);
      gcry_free(buf);
    }
  }
}
#endif



static int GWEN_Crypt_KeyRsa__getNamedElement(gcry_sexp_t pkey, const char *name, gcry_mpi_t *pMpi)
{
  gcry_sexp_t list;
  gcry_mpi_t mpi;

  /* get public exponent */
  list=gcry_sexp_find_token(pkey, name, 0);
  if (!list) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Missing element \"%s\" in sexp", name);
    return GWEN_ERROR_GENERIC;
  }

  mpi=gcry_sexp_nth_mpi(list, 1, GCRYMPI_FMT_USG);
  if (!mpi) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Eror getting value for element \"%s\"", name);
    gcry_sexp_release(list);
    return GWEN_ERROR_GENERIC;
  }

  *pMpi=mpi;
  gcry_sexp_release(list);

  return 0;
}



int GWEN_Crypt_KeyRsa_GeneratePair(unsigned int nbytes, int use65537e,
                                   GWEN_CRYPT_KEY **pPubKey,
                                   GWEN_CRYPT_KEY **pSecretKey)
{
  gcry_sexp_t keyparm, key;
  int rc;
  char buffer[256];
  char numbuf[32];
  gcry_sexp_t pkey;
  int nbits;
  GWEN_CRYPT_KEY *pubKey=NULL;
  GWEN_CRYPT_KEY *secretKey=NULL;

  nbits=nbytes*8;
  assert(nbits>0);
  snprintf(numbuf, sizeof(numbuf)-1, "%d", nbits);
  if (use65537e) {
    snprintf(buffer, sizeof(buffer)-1,
             "(genkey\n"
             " (rsa\n"
             "  (nbits %zd:%d)\n"
             "  (rsa-use-e 5:65537)\n"
             " ))",
             strlen(numbuf),
             nbits);
  }
  else
    snprintf(buffer, sizeof(buffer)-1,
             "(genkey\n"
             " (rsa\n"
             "  (nbits %zd:%d)\n"
             "  (rsa-use-e 1:0)\n"
             " ))",
             strlen(numbuf),
             nbits);
  buffer[sizeof(buffer)-1]=0;

  /*DBG_ERROR(0, "Genkey string: [%s]", buffer);*/

  rc=gcry_sexp_new(&keyparm, buffer, 0, 1);
  if (rc) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Error creating S-expression: %s", gpg_strerror(rc));
    return GWEN_ERROR_GENERIC;
  }

  rc=gcry_pk_genkey(&key, keyparm);
  gcry_sexp_release(keyparm);
  if (rc) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Error generating RSA key: %s", gpg_strerror(rc));
    return GWEN_ERROR_GENERIC;
  }

  pkey=gcry_sexp_find_token(key, "public-key", 0);
  if (!pkey) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Public part missing in return value");
    gcry_sexp_release(key);
    return GWEN_ERROR_GENERIC;
  }
  else {
    int rv;
    gcry_mpi_t n=NULL;
    gcry_mpi_t e=NULL;
    GWEN_CRYPT_KEY *k;
    GWEN_CRYPT_KEY_RSA *xk;

    /* get public exponent */
    rv=GWEN_Crypt_KeyRsa__getNamedElement(pkey, "n", &n);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      gcry_sexp_release(key);
      return GWEN_ERROR_GENERIC;
    }

    rv=GWEN_Crypt_KeyRsa__getNamedElement(pkey, "e", &e);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      gcry_mpi_release(n);
      gcry_sexp_release(key);
      return GWEN_ERROR_GENERIC;
    }

    /* create public key */
    k=GWEN_Crypt_Key_new(GWEN_Crypt_CryptAlgoId_Rsa, nbytes);
    GWEN_NEW_OBJECT(GWEN_CRYPT_KEY_RSA, xk);
    GWEN_INHERIT_SETDATA(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_RSA, k, xk, GWEN_Crypt_KeyRsa_freeData);
    GWEN_Crypt_Key_SetSignFn(k, GWEN_Crypt_KeyRsa_Sign);
    GWEN_Crypt_Key_SetVerifyFn(k, GWEN_Crypt_KeyRsa_Verify);
    GWEN_Crypt_Key_SetEncipherFn(k, GWEN_Crypt_KeyRsa_Encipher);
    GWEN_Crypt_Key_SetDecipherFn(k, GWEN_Crypt_KeyRsa_Decipher);
    xk->modulus=gcry_mpi_copy(n);
    xk->pubExponent=gcry_mpi_copy(e);
    xk->pub=1;
    pubKey=k;

    gcry_mpi_release(e);
    gcry_mpi_release(n);
  }


  pkey=gcry_sexp_find_token(key, "private-key", 0);
  if (!pkey) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Private part missing in return value");
    gcry_sexp_release(key);
    return GWEN_ERROR_GENERIC;
  }
  else {
    int rv;
    gcry_mpi_t n=NULL;
    gcry_mpi_t e=NULL;
    gcry_mpi_t d=NULL;
    GWEN_CRYPT_KEY *k;
    GWEN_CRYPT_KEY_RSA *xk;

    /* get public exponent */
    rv=GWEN_Crypt_KeyRsa__getNamedElement(pkey, "n", &n);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      gcry_sexp_release(key);
      return GWEN_ERROR_GENERIC;
    }

    rv=GWEN_Crypt_KeyRsa__getNamedElement(pkey, "e", &e);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      gcry_mpi_release(n);
      gcry_sexp_release(key);
      return GWEN_ERROR_GENERIC;
    }

    rv=GWEN_Crypt_KeyRsa__getNamedElement(pkey, "d", &d);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      gcry_mpi_release(e);
      gcry_mpi_release(n);
      gcry_sexp_release(key);
      return GWEN_ERROR_GENERIC;
    }

    /* create private key */
    k=GWEN_Crypt_Key_new(GWEN_Crypt_CryptAlgoId_Rsa, nbytes);
    GWEN_NEW_OBJECT(GWEN_CRYPT_KEY_RSA, xk);
    GWEN_INHERIT_SETDATA(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_RSA, k, xk, GWEN_Crypt_KeyRsa_freeData);
    GWEN_Crypt_Key_SetSignFn(k, GWEN_Crypt_KeyRsa_Sign);
    GWEN_Crypt_Key_SetVerifyFn(k, GWEN_Crypt_KeyRsa_Verify);
    GWEN_Crypt_Key_SetEncipherFn(k, GWEN_Crypt_KeyRsa_Encipher);
    GWEN_Crypt_Key_SetDecipherFn(k, GWEN_Crypt_KeyRsa_Decipher);
    xk->modulus=gcry_mpi_copy(n);
    xk->pubExponent=gcry_mpi_copy(e);
    xk->privExponent=gcry_mpi_copy(d);
    xk->pub=0;
    secretKey=k;

    gcry_mpi_release(d);
    gcry_mpi_release(e);
    gcry_mpi_release(n);
  }

  gcry_sexp_release(key);

  *pPubKey=pubKey;
  *pSecretKey=secretKey;

  return 0;
}



#ifndef NO_DEPRECATED_SYMBOLS
int GWEN_Crypt_KeyRsa_GeneratePair2(unsigned int nbits, int use65537e,
                                    GWEN_CRYPT_KEY **pPubKey,
                                    GWEN_CRYPT_KEY **pSecretKey)
{
  if (nbits%8) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "nbits is required to be a multiple of 8 (%d)", nbits);
    return GWEN_ERROR_INVALID;
  }
  return GWEN_Crypt_KeyRsa_GeneratePair(nbits/8, use65537e, pPubKey, pSecretKey);
}
#endif // ifndef NO_DEPRECATED_SYMBOLS




GWENHYWFAR_CB int GWEN_Crypt_KeyRsa_Sign(GWEN_CRYPT_KEY *k,
                                         const uint8_t *pInData,
                                         uint32_t inLen,
                                         uint8_t *pSignatureData,
                                         uint32_t *pSignatureLen)
{
  GWEN_CRYPT_KEY_RSA *xk;
  gcry_error_t err;
  size_t nscanned;
  gcry_mpi_t mpi_in=NULL;
  gcry_mpi_t mpi_sigout1;
  gcry_mpi_t mpi_sigout2=NULL;
  size_t nwritten;

  assert(k);
  xk=GWEN_INHERIT_GETDATA(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_RSA, k);
  assert(xk);

  if (xk->modulus==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No modulus in key data");
    return GWEN_ERROR_BAD_DATA;
  }

  if (xk->privExponent==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No secret exponent in key data");
    return GWEN_ERROR_BAD_DATA;
  }

  /* convert input to MPI */
  err=gcry_mpi_scan(&mpi_in, GCRYMPI_FMT_USG, pInData, inLen, &nscanned);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_mpi_scan(): %s", gcry_strerror(err));
    gcry_mpi_release(mpi_in);
    return GWEN_ERROR_BAD_DATA;
  }

  /* create first signature */
  mpi_sigout1=gcry_mpi_new(GWEN_Crypt_Key_GetKeySize(k));
  gcry_mpi_powm(mpi_sigout1, mpi_in, xk->privExponent, xk->modulus);

  if (!(xk->flags & GWEN_CRYPT_KEYRSA_FLAGS_DIRECTSIGN)) {
    /* create second signature */
    mpi_sigout2=gcry_mpi_new(GWEN_Crypt_Key_GetKeySize(k));
    gcry_mpi_sub(mpi_sigout2, xk->modulus, mpi_sigout1);

    if (gcry_mpi_cmp(mpi_sigout2, mpi_sigout1)<0) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "Choosing 2nd variant");
      gcry_mpi_set(mpi_sigout1, mpi_sigout2);
    }
  }

  /* release unneeded objects */
  gcry_mpi_release(mpi_sigout2);
  gcry_mpi_release(mpi_in);

  /* convert signature MPI */
  err=gcry_mpi_print(GCRYMPI_FMT_USG,
                     pSignatureData, *pSignatureLen,
                     &nwritten, mpi_sigout1);
  gcry_mpi_release(mpi_sigout1);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_mpi_print(): %s", gcry_strerror(err));
    return GWEN_ERROR_BAD_DATA;
  }
  *pSignatureLen=nwritten;

  return 0;
}



static GWENHYWFAR_CB int GWEN_Crypt_KeyRsa_Verify(GWEN_CRYPT_KEY *k,
                                                  const uint8_t *pInData,
                                                  uint32_t inLen,
                                                  const uint8_t *pSignatureData,
                                                  uint32_t signatureLen)
{
  GWEN_CRYPT_KEY_RSA *xk;
  gcry_error_t err;
  size_t nscanned;
  gcry_mpi_t mpi_in=NULL;
  gcry_mpi_t mpi_sigin1=NULL;
  gcry_mpi_t mpi_sigout;

  assert(k);
  xk=GWEN_INHERIT_GETDATA(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_RSA, k);
  assert(xk);

  if (xk->modulus==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No modulus in key data");
    return GWEN_ERROR_BAD_DATA;
  }

  if (xk->pubExponent==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No public exponent in key data");
    return GWEN_ERROR_BAD_DATA;
  }


  /* convert input to MPI */
  err=gcry_mpi_scan(&mpi_in, GCRYMPI_FMT_USG, pInData, inLen, &nscanned);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_mpi_scan(): %s", gcry_strerror(err));
    gcry_mpi_release(mpi_in);
    return GWEN_ERROR_BAD_DATA;
  }

  /* convert signature to MPI */
  err=gcry_mpi_scan(&mpi_sigin1, GCRYMPI_FMT_USG,
                    pSignatureData, signatureLen,
                    &nscanned);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_mpi_scan(): %s", gcry_strerror(err));
    gcry_mpi_release(mpi_sigin1);
    gcry_mpi_release(mpi_in);
    return GWEN_ERROR_BAD_DATA;
  }

  /* create signature */
  mpi_sigout=gcry_mpi_new(GWEN_Crypt_Key_GetKeySize(k));
  gcry_mpi_powm(mpi_sigout, mpi_sigin1, xk->pubExponent, xk->modulus);
  /* compare */
  if (gcry_mpi_cmp(mpi_sigout, mpi_in)) {
    gcry_mpi_t mpi_sigin2;

    mpi_sigin2=gcry_mpi_new(GWEN_Crypt_Key_GetKeySize(k));

    DBG_DEBUG(GWEN_LOGDOMAIN, "Trying 2nd variant");
    gcry_mpi_sub(mpi_sigin2, xk->modulus, mpi_sigin1);
    gcry_mpi_powm(mpi_sigout, mpi_sigin2, xk->pubExponent, xk->modulus);
    if (gcry_mpi_cmp(mpi_sigout, mpi_in)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Bad signature");
      gcry_mpi_release(mpi_sigin2);
      gcry_mpi_release(mpi_sigout);
      gcry_mpi_release(mpi_sigin1);
      gcry_mpi_release(mpi_in);
      return GWEN_ERROR_VERIFY;
    }
    gcry_mpi_release(mpi_sigin2);
  }

  gcry_mpi_release(mpi_sigout);
  gcry_mpi_release(mpi_sigin1);
  gcry_mpi_release(mpi_in);

  return 0;
}



static GWENHYWFAR_CB int GWEN_Crypt_KeyRsa_Encipher(GWEN_CRYPT_KEY *k,
                                                    const uint8_t *pInData,
                                                    uint32_t inLen,
                                                    uint8_t *pOutData,
                                                    uint32_t *pOutLen)
{
  GWEN_CRYPT_KEY_RSA *xk;
  gcry_error_t err;
  size_t nscanned;
  gcry_mpi_t mpi_in=NULL;
  gcry_mpi_t mpi_out;
  size_t nwritten;

  assert(k);
  xk=GWEN_INHERIT_GETDATA(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_RSA, k);
  assert(xk);

  if (xk->modulus==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No modulus in key data");
    return GWEN_ERROR_BAD_DATA;
  }

  if (xk->pubExponent==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No public exponent in key data");
    return GWEN_ERROR_BAD_DATA;
  }


  /* convert input to MPI */
  err=gcry_mpi_scan(&mpi_in, GCRYMPI_FMT_USG, pInData, inLen, &nscanned);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_mpi_scan(): %s", gcry_strerror(err));
    gcry_mpi_release(mpi_in);
    return GWEN_ERROR_BAD_DATA;
  }

  /* encrypt */
  mpi_out=gcry_mpi_new(GWEN_Crypt_Key_GetKeySize(k));
  gcry_mpi_powm(mpi_out, mpi_in, xk->pubExponent, xk->modulus);

  /* release unneeded objects */
  gcry_mpi_release(mpi_in);

  /* convert result MPI */
  err=gcry_mpi_print(GCRYMPI_FMT_USG,
                     pOutData, *pOutLen,
                     &nwritten, mpi_out);
  gcry_mpi_release(mpi_out);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_mpi_print(): %s", gcry_strerror(err));
    return GWEN_ERROR_BAD_DATA;
  }
  *pOutLen=nwritten;

  return 0;
}



static GWENHYWFAR_CB int GWEN_Crypt_KeyRsa_Decipher(GWEN_CRYPT_KEY *k,
                                                    const uint8_t *pInData,
                                                    uint32_t inLen,
                                                    uint8_t *pOutData,
                                                    uint32_t *pOutLen)
{
  GWEN_CRYPT_KEY_RSA *xk;
  gcry_error_t err;
  size_t nscanned;
  gcry_mpi_t mpi_in=NULL;
  gcry_mpi_t mpi_out;
  size_t nwritten;

  assert(k);
  xk=GWEN_INHERIT_GETDATA(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_RSA, k);
  assert(xk);

  if (xk->modulus==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No modulus in key data");
    return GWEN_ERROR_BAD_DATA;
  }

  if (xk->privExponent==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No secret exponent in key data");
    return GWEN_ERROR_BAD_DATA;
  }


  /* convert input to MPI */
  err=gcry_mpi_scan(&mpi_in, GCRYMPI_FMT_USG, pInData, inLen, &nscanned);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_mpi_scan(): %s", gcry_strerror(err));
    gcry_mpi_release(mpi_in);
    return GWEN_ERROR_BAD_DATA;
  }

  /* decrypt */
  mpi_out=gcry_mpi_new(GWEN_Crypt_Key_GetKeySize(k));
  gcry_mpi_powm(mpi_out, mpi_in, xk->privExponent, xk->modulus);

  /* release unneeded objects */
  gcry_mpi_release(mpi_in);

  /* convert result MPI */
  err=gcry_mpi_print(GCRYMPI_FMT_USG,
                     pOutData, *pOutLen,
                     &nwritten, mpi_out);
  gcry_mpi_release(mpi_out);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_mpi_print(): %s", gcry_strerror(err));
    return GWEN_ERROR_BAD_DATA;
  }
  *pOutLen=nwritten;

  return 0;
}



static int GWEN_Crypt_KeyRsa__ReadMpi(GWEN_DB_NODE *db, const char *dbName, gcry_mpi_t *pMpi)
{
  gcry_error_t err;
  const void *p;
  unsigned int len;
  gcry_mpi_t mpi=NULL;
  size_t nscanned=0;

  /* read n */
  p=GWEN_DB_GetBinValue(db, dbName, 0, NULL, 0, &len);
  if (p==NULL || len<1) {
    DBG_INFO(GWEN_LOGDOMAIN, "Missing %s", dbName);
    return GWEN_ERROR_NO_DATA;
  }

  err=gcry_mpi_scan(&mpi, GCRYMPI_FMT_USG, p, len, &nscanned);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_mpi_scan(): %s", gcry_strerror(err));
    gcry_mpi_release(mpi);
    return GWEN_ERROR_GENERIC;
  }
  if (nscanned<1) {
    if (mpi==NULL) {
      DBG_INFO(GWEN_LOGDOMAIN, "Empty %s (%d)", dbName, (int)nscanned);
      return GWEN_ERROR_BAD_DATA;
    }
  }
  *pMpi=mpi;

  return 0;
}




static int GWEN_Crypt_KeyRsa__WriteMpi(GWEN_DB_NODE *db, const char *dbName, const gcry_mpi_t mpi)
{
  gcry_error_t err;
  unsigned char *buf;
  size_t nbytes;

  /* write mpi as bin into a buffer which will be allocates by this function */
  err=gcry_mpi_aprint(GCRYMPI_FMT_USG, &buf, &nbytes, mpi);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_mpi_aprint(%s): %s", dbName, gcry_strerror(err));
    return GWEN_ERROR_GENERIC;
  }
  GWEN_DB_SetBinValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      dbName,
                      buf, nbytes);
  gcry_free(buf);

  return 0;
}



static int GWEN_Crypt_KeyRsa__MpiToBuffer(const gcry_mpi_t mpi, unsigned char *buf, size_t nbytes)
{
  gcry_error_t err;
  size_t nwritten=0;

  /* write mpi as bin into the given buffer */
  err=gcry_mpi_print(GCRYMPI_FMT_USG, buf, nbytes, &nwritten, mpi);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_mpi_print(): %s", gcry_strerror(err));
    return GWEN_ERROR_GENERIC;
  }

  return nwritten;
}



static GWENHYWFAR_CB
void GWEN_Crypt_KeyRsa_freeData(GWEN_UNUSED void *bp, void *p)
{
  GWEN_CRYPT_KEY_RSA *xk;

  xk=(GWEN_CRYPT_KEY_RSA *) p;
  if (xk->modulus)
    gcry_mpi_release(xk->modulus);
  if (xk->pubExponent)
    gcry_mpi_release(xk->pubExponent);
  if (xk->privExponent)
    gcry_mpi_release(xk->privExponent);

  GWEN_FREE_OBJECT(xk);
}



GWEN_CRYPT_KEY *GWEN_Crypt_KeyRsa_fromDb(GWEN_DB_NODE *db)
{
  int rv;
  int isPublic;
  GWEN_CRYPT_KEY *k;
  GWEN_CRYPT_KEY_RSA *xk;
  GWEN_DB_NODE *dbR;

  dbR=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "rsa");
  if (dbR==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "DB does not contain an RSA key (no RSA group)");
    return NULL;
  }
  k=GWEN_Crypt_Key_fromDb(db);
  if (k==NULL) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    return NULL;
  }
  if (GWEN_Crypt_Key_GetCryptAlgoId(k)!=GWEN_Crypt_CryptAlgoId_Rsa) {
    DBG_ERROR(GWEN_LOGDOMAIN, "DB does not contain an RSA key");
    GWEN_Crypt_Key_free(k);
    return NULL;
  }

  /* extend key */
  GWEN_NEW_OBJECT(GWEN_CRYPT_KEY_RSA, xk);
  GWEN_INHERIT_SETDATA(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_RSA, k, xk,
                       GWEN_Crypt_KeyRsa_freeData);
  GWEN_Crypt_Key_SetSignFn(k, GWEN_Crypt_KeyRsa_Sign);
  GWEN_Crypt_Key_SetVerifyFn(k, GWEN_Crypt_KeyRsa_Verify);
  GWEN_Crypt_Key_SetEncipherFn(k, GWEN_Crypt_KeyRsa_Encipher);
  GWEN_Crypt_Key_SetDecipherFn(k, GWEN_Crypt_KeyRsa_Decipher);

  isPublic=GWEN_DB_GetIntValue(dbR, "isPublic", 0, 1);
  xk->pub=isPublic;

  xk->flags=GWEN_DB_GetIntValue(dbR, "flags", 0, 0);

  /* read data */
  rv=GWEN_Crypt_KeyRsa__ReadMpi(dbR, "n", &(xk->modulus));
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Crypt_Key_free(k);
    return NULL;
  }

  rv=GWEN_Crypt_KeyRsa__ReadMpi(dbR, "e", &(xk->pubExponent));
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Crypt_Key_free(k);
    return NULL;
  }

  if (!isPublic) {
    rv=GWEN_Crypt_KeyRsa__ReadMpi(dbR, "d", &(xk->privExponent));
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Crypt_Key_free(k);
      return NULL;
    }
  }


  return k;
}



int GWEN_Crypt_KeyRsa_toDb(const GWEN_CRYPT_KEY *k, GWEN_DB_NODE *db, int pub)
{
  GWEN_CRYPT_KEY_RSA *xk;
  GWEN_DB_NODE *dbR;
  int rv;

  assert(k);
  xk=GWEN_INHERIT_GETDATA(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_RSA, k);
  assert(xk);

  if (xk->pub && !pub) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Can't write public key as secret key");
    return GWEN_ERROR_INVALID;
  }

#if 0
  DBG_ERROR(0, "toDb (%s):", pub?"public":"private");
  dumpKeyData(ds);
#endif

  /* let key module write basic key info */
  rv=GWEN_Crypt_Key_toDb(k, db);
  if (rv)
    return rv;

  /* write RSA stuff into our own group */
  dbR=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "rsa");
  assert(dbR);

  GWEN_DB_SetIntValue(dbR, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "isPublic", pub);
  GWEN_DB_SetIntValue(dbR, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "flags", xk->flags);

  /* store n */
  rv=GWEN_Crypt_KeyRsa__WriteMpi(dbR, "n", xk->modulus);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* store e */
  rv=GWEN_Crypt_KeyRsa__WriteMpi(dbR, "e", xk->pubExponent);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }


  if (!pub) {
    /* store d */
    rv=GWEN_Crypt_KeyRsa__WriteMpi(dbR, "d", xk->privExponent);
    if (rv) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  return 0;
}




int GWEN_Crypt_KeyRsa_GetModulus(const GWEN_CRYPT_KEY *k, uint8_t *buffer, uint32_t *pBufLen)
{
  GWEN_CRYPT_KEY_RSA *xk;
  int rv;

  assert(k);
  xk=GWEN_INHERIT_GETDATA(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_RSA, k);
  assert(xk);

  rv=GWEN_Crypt_KeyRsa__MpiToBuffer(xk->modulus, buffer, *pBufLen);
  if (rv<1) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    if (rv<0)
      return rv;
    return GWEN_ERROR_GENERIC;
  }

  *pBufLen=rv;
  return 0;
}



int GWEN_Crypt_KeyRsa_GetExponent(const GWEN_CRYPT_KEY *k, uint8_t *buffer, uint32_t *pBufLen)
{
  GWEN_CRYPT_KEY_RSA *xk;
  int rv;

  assert(k);
  xk=GWEN_INHERIT_GETDATA(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_RSA, k);
  assert(xk);

  rv=GWEN_Crypt_KeyRsa__MpiToBuffer(xk->pubExponent, buffer, *pBufLen);
  if (rv<1) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    if (rv<0)
      return rv;
    return GWEN_ERROR_GENERIC;
  }

  *pBufLen=rv;
  return 0;
}



int GWEN_Crypt_KeyRsa_GetSecretExponent(const GWEN_CRYPT_KEY *k, uint8_t *buffer, uint32_t *pBufLen)
{
  GWEN_CRYPT_KEY_RSA *xk;
  int rv;

  assert(k);
  xk=GWEN_INHERIT_GETDATA(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_RSA, k);
  assert(xk);

  rv=GWEN_Crypt_KeyRsa__MpiToBuffer(xk->privExponent, buffer, *pBufLen);
  if (rv<1) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    if (rv<0)
      return rv;
    return GWEN_ERROR_GENERIC;
  }

  *pBufLen=rv;
  return 0;
}



GWEN_CRYPT_KEY *GWEN_Crypt_KeyRsa_fromModExp(unsigned int nbytes,
                                             const uint8_t *pModulus,
                                             uint32_t lModulus,
                                             const uint8_t *pExponent,
                                             uint32_t lExponent)
{
  GWEN_DB_NODE *dbKey;
  GWEN_DB_NODE *dbR;
  GWEN_CRYPT_KEY *key;

  assert(nbytes);
  assert(pModulus);
  assert(lModulus);
  assert(pExponent);
  assert(lExponent);

  dbKey=GWEN_DB_Group_new("key");
  dbR=GWEN_DB_GetGroup(dbKey, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "rsa");

  /* basic key stuff */
  GWEN_DB_SetCharValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "cryptAlgoId",
                       GWEN_Crypt_CryptAlgoId_toString(GWEN_Crypt_CryptAlgoId_Rsa));
  GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "keySize", nbytes);

  /* RSA stuff */
  GWEN_DB_SetIntValue(dbR, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "isPublic", 1);
  GWEN_DB_SetBinValue(dbR, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "n",
                      pModulus, lModulus);
  GWEN_DB_SetBinValue(dbR, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "e",
                      pExponent, lExponent);

  /* create key from DB */
  key=GWEN_Crypt_KeyRsa_fromDb(dbKey);
  if (key==NULL) {
    DBG_INFO(GWEN_LOGDOMAIN,
             "Internal error: Bad RSA key group");
    GWEN_DB_Dump(dbKey, 2);
    GWEN_DB_Group_free(dbKey);
    return NULL;
  }

  GWEN_DB_Group_free(dbKey);
  return key;
}



GWEN_CRYPT_KEY *GWEN_Crypt_KeyRsa_fromModPrivExp(unsigned int nbytes,
                                                 const uint8_t *pModulus,
                                                 uint32_t lModulus,
                                                 const uint8_t *pExponent,
                                                 uint32_t lExponent,
                                                 const uint8_t *pPrivExponent,
                                                 uint32_t lPrivExponent)
{
  GWEN_DB_NODE *dbKey;
  GWEN_DB_NODE *dbR;
  GWEN_CRYPT_KEY *key;

  assert(nbytes);
  assert(pModulus);
  assert(lModulus);
  assert(pExponent);
  assert(lExponent);
  assert(pPrivExponent);
  assert(lPrivExponent);

  dbKey=GWEN_DB_Group_new("key");
  dbR=GWEN_DB_GetGroup(dbKey, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "rsa");

  /* basic key stuff */
  GWEN_DB_SetCharValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "cryptAlgoId",
                       GWEN_Crypt_CryptAlgoId_toString(GWEN_Crypt_CryptAlgoId_Rsa));
  GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "keySize", nbytes);

  /* RSA stuff */
  GWEN_DB_SetIntValue(dbR, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "isPublic", 0);
  GWEN_DB_SetBinValue(dbR, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "n",
                      pModulus, lModulus);
  GWEN_DB_SetBinValue(dbR, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "e",
                      pExponent, lExponent);
  GWEN_DB_SetBinValue(dbR, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "d",
                      pPrivExponent, lPrivExponent);

  /* create key from DB */
  key=GWEN_Crypt_KeyRsa_fromDb(dbKey);
  if (key==NULL) {
    DBG_INFO(GWEN_LOGDOMAIN,
             "Internal error: Bad RSA key group");
    GWEN_DB_Dump(dbKey, 2);
    GWEN_DB_Group_free(dbKey);
    return NULL;
  }

  GWEN_DB_Group_free(dbKey);
  return key;
}



GWEN_CRYPT_KEY *GWEN_Crypt_KeyRsa_dup(const GWEN_CRYPT_KEY *k)
{
  GWEN_CRYPT_KEY_RSA *xk;
  GWEN_DB_NODE *dbKey;
  GWEN_CRYPT_KEY *nk;
  int rv;

  assert(k);
  xk=GWEN_INHERIT_GETDATA(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_RSA, k);
  assert(xk);

  dbKey=GWEN_DB_Group_new("dbKey");
  rv=GWEN_Crypt_KeyRsa_toDb(k, dbKey, xk->pub);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(dbKey);
    return NULL;
  }

  nk=GWEN_Crypt_KeyRsa_fromDb(dbKey);
  GWEN_DB_Group_free(dbKey);
  if (nk==NULL) {
    DBG_INFO(GWEN_LOGDOMAIN, "Could not create key");
  }

  GWEN_Crypt_KeyRsa_SetFlags(nk, xk->flags);

  return nk;
}



uint32_t GWEN_Crypt_KeyRsa_GetFlags(const GWEN_CRYPT_KEY *k)
{
  GWEN_CRYPT_KEY_RSA *xk;

  assert(k);
  xk=GWEN_INHERIT_GETDATA(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_RSA, k);
  assert(xk);

  return xk->flags;
}



void GWEN_Crypt_KeyRsa_SetFlags(GWEN_CRYPT_KEY *k, uint32_t fl)
{
  GWEN_CRYPT_KEY_RSA *xk;

  assert(k);
  xk=GWEN_INHERIT_GETDATA(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_RSA, k);
  assert(xk);

  xk->flags=fl;
}



void GWEN_Crypt_KeyRsa_AddFlags(GWEN_CRYPT_KEY *k, uint32_t fl)
{
  GWEN_CRYPT_KEY_RSA *xk;

  assert(k);
  xk=GWEN_INHERIT_GETDATA(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_RSA, k);
  assert(xk);

  xk->flags|=fl;
}



void GWEN_Crypt_KeyRsa_SubFlags(GWEN_CRYPT_KEY *k, uint32_t fl)
{
  GWEN_CRYPT_KEY_RSA *xk;

  assert(k);
  xk=GWEN_INHERIT_GETDATA(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_RSA, k);
  assert(xk);

  xk->flags&=~fl;
}








