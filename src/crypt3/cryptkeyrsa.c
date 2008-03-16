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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "cryptkeyrsa_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>




GWEN_INHERIT(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_RSA)




#if 0
static void dumpKeyData(gcry_ac_data_t data) {
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
      GWEN_Text_DumpString((const char*)buf, nbytes, stderr, 6);
      gcry_free(buf);
    }
  }
}
#endif




int GWEN_Crypt_KeyRsa_Sign(GWEN_CRYPT_KEY *k,
			   const uint8_t *pInData,
			   uint32_t inLen,
			   uint8_t *pSignatureData,
			   uint32_t *pSignatureLen) {
  GWEN_CRYPT_KEY_RSA *xk;
  gcry_error_t err;
  size_t nscanned;
  gcry_ac_data_t dsKey;
  gcry_mpi_t mpi_d;
  gcry_mpi_t mpi_n;
  gcry_mpi_t mpi_in;
  gcry_mpi_t mpi_sigout1;
  gcry_mpi_t mpi_sigout2=NULL;
  size_t nwritten;

  assert(k);
  xk=GWEN_INHERIT_GETDATA(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_RSA, k);
  assert(xk);

  dsKey=gcry_ac_key_data_get(xk->key);

  /* get modulus */
  err=gcry_ac_data_get_name(dsKey, 0, "n", &mpi_n);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_data_get_name(): %s",
	     gcry_strerror(err));
    return GWEN_ERROR_BAD_DATA;
  }

  /* get private exponent */
  err=gcry_ac_data_get_name(dsKey, 0, "d", &mpi_d);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_data_get_name(): %s",
	     gcry_strerror(err));
    return GWEN_ERROR_BAD_DATA;
  }

  /* convert input to MPI */
  mpi_in=gcry_mpi_new(GWEN_Crypt_Key_GetKeySize(k));
  err=gcry_mpi_scan(&mpi_in, GCRYMPI_FMT_USG, pInData, inLen, &nscanned);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_mpi_scan(): %s", gcry_strerror(err));
    gcry_mpi_release(mpi_in);
    return GWEN_ERROR_BAD_DATA;
  }

  /* create first signature */
  mpi_sigout1=gcry_mpi_new(GWEN_Crypt_Key_GetKeySize(k));
  gcry_mpi_powm(mpi_sigout1, mpi_in, mpi_d, mpi_n);

  if (!(xk->flags & GWEN_CRYPT_KEYRSA_FLAGS_DIRECTSIGN)) {
    /* create second signature */
    mpi_sigout2=gcry_mpi_new(GWEN_Crypt_Key_GetKeySize(k));
    gcry_mpi_sub(mpi_sigout2, mpi_n, mpi_sigout1);

    if (gcry_mpi_cmp(mpi_sigout2, mpi_sigout1)<0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Choosing 2nd variant");
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



int GWEN_Crypt_KeyRsa_Verify(GWEN_CRYPT_KEY *k,
			     const uint8_t *pInData,
			     uint32_t inLen,
			     const uint8_t *pSignatureData,
			     uint32_t signatureLen) {
  GWEN_CRYPT_KEY_RSA *xk;
  gcry_error_t err;
  size_t nscanned;
  gcry_ac_data_t dsKey;
  gcry_mpi_t mpi_e;
  gcry_mpi_t mpi_n;
  gcry_mpi_t mpi_in;
  gcry_mpi_t mpi_sigin1;
  gcry_mpi_t mpi_sigout;

  assert(k);
  xk=GWEN_INHERIT_GETDATA(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_RSA, k);
  assert(xk);

  dsKey=gcry_ac_key_data_get(xk->key);

  /* get modulus */
  err=gcry_ac_data_get_name(dsKey, 0, "n", &mpi_n);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_data_get_name(): %s",
	     gcry_strerror(err));
    return GWEN_ERROR_BAD_DATA;
  }

  /* get public exponent */
  err=gcry_ac_data_get_name(dsKey, 0, "e", &mpi_e);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_data_get_name(): %s",
	     gcry_strerror(err));
    return GWEN_ERROR_BAD_DATA;
  }

  /* convert input to MPI */
  mpi_in=gcry_mpi_new(GWEN_Crypt_Key_GetKeySize(k));
  err=gcry_mpi_scan(&mpi_in, GCRYMPI_FMT_USG, pInData, inLen, &nscanned);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_mpi_scan(): %s", gcry_strerror(err));
    gcry_mpi_release(mpi_in);
    return GWEN_ERROR_BAD_DATA;
  }

  /* convert signature to MPI */
  mpi_sigin1=gcry_mpi_new(GWEN_Crypt_Key_GetKeySize(k));
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
  gcry_mpi_powm(mpi_sigout, mpi_sigin1, mpi_e, mpi_n);
  /* compare */
  if (gcry_mpi_cmp(mpi_sigout, mpi_in)) {
    gcry_mpi_t mpi_sigin2;

    mpi_sigin2=gcry_mpi_new(GWEN_Crypt_Key_GetKeySize(k));

    DBG_ERROR(GWEN_LOGDOMAIN, "Trying 2nd variant");
    gcry_mpi_sub(mpi_sigin2, mpi_n, mpi_sigin1);
    gcry_mpi_powm(mpi_sigout, mpi_sigin2, mpi_e, mpi_n);
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



int GWEN_Crypt_KeyRsa_Encipher(GWEN_CRYPT_KEY *k,
			       const uint8_t *pInData,
			       uint32_t inLen,
			       uint8_t *pOutData,
			       uint32_t *pOutLen) {
  GWEN_CRYPT_KEY_RSA *xk;
  gcry_error_t err;
  size_t nscanned;
  gcry_ac_data_t dsKey;
  gcry_mpi_t mpi_e;
  gcry_mpi_t mpi_n;
  gcry_mpi_t mpi_in;
  gcry_mpi_t mpi_out;
  size_t nwritten;

  assert(k);
  xk=GWEN_INHERIT_GETDATA(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_RSA, k);
  assert(xk);

  dsKey=gcry_ac_key_data_get(xk->key);

  /* get modulus */
  err=gcry_ac_data_get_name(dsKey, 0, "n", &mpi_n);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_data_get_name(): %s",
	     gcry_strerror(err));
    return GWEN_ERROR_BAD_DATA;
  }

  /* get private exponent */
  err=gcry_ac_data_get_name(dsKey, 0, "e", &mpi_e);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_data_get_name(): %s",
	     gcry_strerror(err));
    return GWEN_ERROR_BAD_DATA;
  }

  /* convert input to MPI */
  mpi_in=gcry_mpi_new(GWEN_Crypt_Key_GetKeySize(k));
  err=gcry_mpi_scan(&mpi_in, GCRYMPI_FMT_USG, pInData, inLen, &nscanned);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_mpi_scan(): %s", gcry_strerror(err));
    gcry_mpi_release(mpi_in);
    return GWEN_ERROR_BAD_DATA;
  }

  /* encrypt */
  mpi_out=gcry_mpi_new(GWEN_Crypt_Key_GetKeySize(k));
  gcry_mpi_powm(mpi_out, mpi_in, mpi_e, mpi_n);

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



int GWEN_Crypt_KeyRsa_Decipher(GWEN_CRYPT_KEY *k,
			       const uint8_t *pInData,
			       uint32_t inLen,
			       uint8_t *pOutData,
			       uint32_t *pOutLen) {
  GWEN_CRYPT_KEY_RSA *xk;
  gcry_error_t err;
  size_t nscanned;
  gcry_ac_data_t dsKey;
  gcry_mpi_t mpi_d;
  gcry_mpi_t mpi_n;
  gcry_mpi_t mpi_in;
  gcry_mpi_t mpi_out;
  size_t nwritten;

  assert(k);
  xk=GWEN_INHERIT_GETDATA(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_RSA, k);
  assert(xk);

  dsKey=gcry_ac_key_data_get(xk->key);

  /* get modulus */
  err=gcry_ac_data_get_name(dsKey, 0, "n", &mpi_n);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_data_get_name(): %s",
	     gcry_strerror(err));
    return GWEN_ERROR_BAD_DATA;
  }

  /* get private exponent */
  err=gcry_ac_data_get_name(dsKey, 0, "d", &mpi_d);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_data_get_name(): %s",
	     gcry_strerror(err));
    return GWEN_ERROR_BAD_DATA;
  }

  /* convert input to MPI */
  mpi_in=gcry_mpi_new(GWEN_Crypt_Key_GetKeySize(k));
  err=gcry_mpi_scan(&mpi_in, GCRYMPI_FMT_USG, pInData, inLen, &nscanned);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_mpi_scan(): %s", gcry_strerror(err));
    gcry_mpi_release(mpi_in);
    return GWEN_ERROR_BAD_DATA;
  }

  /* decrypt */
  mpi_out=gcry_mpi_new(GWEN_Crypt_Key_GetKeySize(k));
  gcry_mpi_powm(mpi_out, mpi_in, mpi_d, mpi_n);

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



int GWEN_Crypt_KeyRsa__ReadMpi(GWEN_DB_NODE *db,
			       const char *dbName,
			       gcry_ac_data_t ds,
			       const char *dsName) {
  gcry_error_t err;
  const void *p;
  unsigned int len;
  gcry_mpi_t mpi=NULL;
  size_t nscanned;

  /* read n */
  p=GWEN_DB_GetBinValue(db, dbName, 0, NULL, 0, &len);
  if (p==NULL || len<1) {
    DBG_INFO(GWEN_LOGDOMAIN, "Missing %s", dbName);
    return GWEN_ERROR_NO_DATA;
  }

  err=gcry_mpi_scan(&mpi, GCRYMPI_FMT_USG, p, len, &nscanned);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_mpi_scan(): %s", gcry_strerror(err));
    if (mpi)
      gcry_mpi_release(mpi);
    return GWEN_ERROR_GENERIC;
  }
  if (nscanned<1) {
    DBG_INFO(GWEN_LOGDOMAIN, "Empty %s", dbName);
    if (mpi)
      gcry_mpi_release(mpi);
    return GWEN_ERROR_BAD_DATA;
  }
  err=gcry_ac_data_set(ds, GCRY_AC_FLAG_COPY, (char*)dsName, mpi);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_ac_data_set(): %s", gcry_strerror(err));
    gcry_mpi_release(mpi);
    return GWEN_ERROR_GENERIC;
  }
  gcry_mpi_release(mpi);

  return 0;
}




int GWEN_Crypt_KeyRsa__WriteMpi(GWEN_DB_NODE *db,
				const char *dbName,
				gcry_ac_data_t ds,
				const char *dsName) {
  gcry_mpi_t mpi;
  gcry_error_t err;
  unsigned char *buf;
  size_t nbytes;

  /* read n (don't copy) */
  err=gcry_ac_data_get_name(ds, 0, dsName, &mpi);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_ac_data_get_name(%s): %s",
	     dsName, gcry_strerror(err));
    if (err==GPG_ERR_INV_ARG)
      return GWEN_ERROR_NO_DATA;
    else
      return GWEN_ERROR_GENERIC;
  }

  /* write mpi as bin into a buffer which will be allocates by this function */
  err=gcry_mpi_aprint(GCRYMPI_FMT_USG, &buf, &nbytes, mpi);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_mpi_aprint(%s): %s", dsName, gcry_strerror(err));
    return GWEN_ERROR_GENERIC;
  }
  GWEN_DB_SetBinValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      dbName,
		      buf, nbytes);
  gcry_free(buf);

  return 0;
}



int GWEN_Crypt_KeyRsa__DataFromDb(GWEN_DB_NODE *db, gcry_ac_data_t *pData,
				  int pub, unsigned int nbits) {
  gcry_ac_data_t ds;
  gcry_error_t err;
  int rv;

  /* allocate key data */
  err=gcry_ac_data_new(&ds);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_ac_data_new(): %d", err);
    return GWEN_ERROR_GENERIC;
  }

  /* read n */
  rv=GWEN_Crypt_KeyRsa__ReadMpi(db, "n", ds, "n");
  if (rv) {
    gcry_ac_data_destroy(ds);
    return rv;
  }

  /* read e */
  rv=GWEN_Crypt_KeyRsa__ReadMpi(db, "e", ds, "e");
  if (rv) {
    gcry_ac_data_destroy(ds);
    return rv;
  }

  if (!pub) {
    /* read d */
    rv=GWEN_Crypt_KeyRsa__ReadMpi(db, "d", ds, "d");
    if (rv) {
      gcry_ac_data_destroy(ds);
      return rv;
    }

    /* read p */
    rv=GWEN_Crypt_KeyRsa__ReadMpi(db, "p", ds, "p");
    if (rv) {
      gcry_ac_data_destroy(ds);
      return rv;
    }

    /* read q */
    rv=GWEN_Crypt_KeyRsa__ReadMpi(db, "q", ds, "q");
    if (rv) {
      gcry_ac_data_destroy(ds);
      return rv;
    }
  }

  *pData=ds;
  return 0;
}



GWENHYWFAR_CB
void GWEN_Crypt_KeyRsa_freeData(void *bp, void *p) {
  GWEN_CRYPT_KEY_RSA *xk;

  xk=(GWEN_CRYPT_KEY_RSA*) p;
  if (xk->keyValid)
    gcry_ac_key_destroy(xk->key);
  if (xk->algoValid)
    gcry_ac_close(xk->algoHandle);
  GWEN_FREE_OBJECT(xk);
}



GWEN_CRYPT_KEY *GWEN_Crypt_KeyRsa_fromDb(GWEN_DB_NODE *db) {
  gcry_error_t err;
  gcry_ac_data_t data;
  int rv;
  int isPublic;
  GWEN_CRYPT_KEY *k;
  GWEN_CRYPT_KEY_RSA *xk;
  unsigned int nbits;
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
  nbits=GWEN_Crypt_Key_GetKeySize(k)*8;

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

  /* prepare data */
  rv=GWEN_Crypt_KeyRsa__DataFromDb(dbR, &data, isPublic, nbits);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Crypt_Key_free(k);
    return NULL;
  }

  err=gcry_ac_open(&xk->algoHandle, GCRY_AC_RSA, 0); /* TODO: lookup flags to use */
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_ac_open(): %d", err);
    gcry_ac_data_destroy(data);
    GWEN_Crypt_Key_free(k);
    return NULL;
  }
  xk->algoValid=1;

  err=gcry_ac_key_init(&xk->key, xk->algoHandle,
		       isPublic?GCRY_AC_KEY_PUBLIC:GCRY_AC_KEY_SECRET,
		       data);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_ac_key_init(): %d", err);
    gcry_ac_data_destroy(data);
    GWEN_Crypt_Key_free(k);
    return NULL;
  }
  xk->keyValid=1;

#if 0
  DBG_ERROR(0, "fromDb:");
  dumpKeyData(data);
#endif

  gcry_ac_data_destroy(data);
  return k;
}



int GWEN_Crypt_KeyRsa_toDb(const GWEN_CRYPT_KEY *k, GWEN_DB_NODE *db, int pub) {
  GWEN_CRYPT_KEY_RSA *xk;
  GWEN_DB_NODE *dbR;
  int rv;
  gcry_ac_data_t ds;

  assert(k);
  xk=GWEN_INHERIT_GETDATA(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_RSA, k);
  assert(xk);

  if (xk->algoValid==0 || xk->keyValid==0) {
    /* should not happen */
    DBG_ERROR(GWEN_LOGDOMAIN, "Key is not open");
    return GWEN_ERROR_NOT_OPEN;
  }

  if (xk->pub && !pub) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Can't write public key as secret key");
    return GWEN_ERROR_INVALID;
  }

  ds=gcry_ac_key_data_get(xk->key);

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
  rv=GWEN_Crypt_KeyRsa__WriteMpi(dbR, "n", ds, "n");
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* store e */
  rv=GWEN_Crypt_KeyRsa__WriteMpi(dbR, "e", ds, "e");
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  if (!pub) {
    /* store d */
    rv=GWEN_Crypt_KeyRsa__WriteMpi(dbR, "d", ds, "d");
    if (rv) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    /* store p */
    rv=GWEN_Crypt_KeyRsa__WriteMpi(dbR, "p", ds, "p");
    if (rv) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    /* store q */
    rv=GWEN_Crypt_KeyRsa__WriteMpi(dbR, "q", ds, "q");
    if (rv) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  return 0;
}




int GWEN_Crypt_KeyRsa__sKeyElementToData(gcry_ac_data_t data, gcry_sexp_t sx, const char *name) {
  int rc;
  gcry_sexp_t list;
  gcry_mpi_t mpi=NULL;

  list=gcry_sexp_find_token(sx, name, 0);
  if (!list || !(mpi=gcry_sexp_nth_mpi(list, 1, 0)) ) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Entry \"%s\" not found", name);
    return GWEN_ERROR_GENERIC;
  }

  rc=gcry_ac_data_set(data, GCRY_AC_FLAG_COPY, (char*)name, mpi);
  if (rc) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_ac_data_set(): %s", gcry_strerror(rc));
    gcry_mpi_release(mpi);
    return GWEN_ERROR_GENERIC;
  }
  gcry_mpi_release(mpi);
  gcry_sexp_release(list);

  return 0;
}



int GWEN_Crypt_KeyRsa__sKeyToDataPubKey(gcry_ac_data_t data, gcry_sexp_t sx) {
  int rv;

  rv=GWEN_Crypt_KeyRsa__sKeyElementToData(data, sx, "n");
  if (rv)
    return rv;
  rv=GWEN_Crypt_KeyRsa__sKeyElementToData(data, sx, "e");
  if (rv)
    return rv;
  return 0;
}



int GWEN_Crypt_KeyRsa__sKeyToDataPrivKey(gcry_ac_data_t data, gcry_sexp_t sx) {
  int rv;

  rv=GWEN_Crypt_KeyRsa__sKeyElementToData(data, sx, "n");
  if (rv)
    return rv;
  rv=GWEN_Crypt_KeyRsa__sKeyElementToData(data, sx, "e");
  if (rv)
    return rv;
  rv=GWEN_Crypt_KeyRsa__sKeyElementToData(data, sx, "d");
  if (rv)
    return rv;
  rv=GWEN_Crypt_KeyRsa__sKeyElementToData(data, sx, "p");
  if (rv)
    return rv;
  rv=GWEN_Crypt_KeyRsa__sKeyElementToData(data, sx, "q");
  if (rv)
    return rv;
  return 0;
}



int GWEN_Crypt_KeyRsa_GeneratePair(unsigned int nbytes, int use65537e,
				   GWEN_CRYPT_KEY **pPubKey,
				   GWEN_CRYPT_KEY **pSecretKey) {
  gcry_sexp_t keyparm, key;
  int rc;
  char buffer[256];
  char numbuf[32];
  gcry_sexp_t skey, pkey;

  snprintf(numbuf, sizeof(numbuf)-1, "%d", nbytes*8);
  if (use65537e) {
    snprintf(buffer, sizeof(buffer)-1,
	     "(genkey\n"
	     " (rsa\n"
	     "  (nbits %zd:%d)\n"
	     "  (rsa-use-e 5:65537)\n"
	     " ))",
	     strlen(numbuf),
	     nbytes*8);
  }
  else
    snprintf(buffer, sizeof(buffer)-1,
	     "(genkey\n"
	     " (rsa\n"
	     "  (nbits %zd:%d)\n"
	     "  (rsa-use-e 1:0)\n"
	     " ))",
	     strlen(numbuf),
	     nbytes*8);

  rc=gcry_sexp_new(&keyparm, buffer, 0, 1);
  if (rc) {
    DBG_ERROR(GWEN_LOGDOMAIN,
	      "Error creating S-expression: %s", gpg_strerror (rc));
    return GWEN_ERROR_GENERIC;
  }

  rc=gcry_pk_genkey(&key, keyparm);
  gcry_sexp_release(keyparm);
  if (rc) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Error generating RSA key: %s", gpg_strerror (rc));
    return GWEN_ERROR_GENERIC;
  }

  pkey=gcry_sexp_find_token(key, "public-key", 0);
  if (!pkey) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Public part missing in return value");
    gcry_sexp_release(key);
    return GWEN_ERROR_GENERIC;
  }
  else {
    gcry_ac_data_t data;
    GWEN_CRYPT_KEY *k;
    GWEN_CRYPT_KEY_RSA *xk;

    /* allocate key data */
    rc=gcry_ac_data_new(&data);
    if (rc) {
      DBG_INFO(GWEN_LOGDOMAIN, "gcry_ac_data_new(): %s", gcry_strerror(rc));
      gcry_sexp_release(key);
      return GWEN_ERROR_GENERIC;
    }

    rc=GWEN_Crypt_KeyRsa__sKeyToDataPubKey(data, pkey);
    if (rc) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rc);
      gcry_ac_data_destroy(data);
      gcry_sexp_release(key);
      return rc;
    }
    gcry_sexp_release(pkey);

    /* create public key */
    k=GWEN_Crypt_Key_new(GWEN_Crypt_CryptAlgoId_Rsa, nbytes);
    assert(k);
    GWEN_NEW_OBJECT(GWEN_CRYPT_KEY_RSA, xk);
    GWEN_INHERIT_SETDATA(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_RSA, k, xk,
			 GWEN_Crypt_KeyRsa_freeData);
    GWEN_Crypt_Key_SetSignFn(k, GWEN_Crypt_KeyRsa_Sign);
    GWEN_Crypt_Key_SetVerifyFn(k, GWEN_Crypt_KeyRsa_Verify);
    GWEN_Crypt_Key_SetEncipherFn(k, GWEN_Crypt_KeyRsa_Encipher);
    GWEN_Crypt_Key_SetDecipherFn(k, GWEN_Crypt_KeyRsa_Decipher);
  
    /* open algo for key */
    rc=gcry_ac_open(&xk->algoHandle, GCRY_AC_RSA, 0);
    if (rc) {
      DBG_INFO(GWEN_LOGDOMAIN, "gcry_ac_open(): %s", gcry_strerror(rc));
      GWEN_Crypt_Key_free(k);
      gcry_ac_data_destroy(data);
      gcry_sexp_release(key);
      return GWEN_ERROR_GENERIC;
    }
    xk->algoValid=1;
    /* copy key */
    rc=gcry_ac_key_init(&xk->key, xk->algoHandle, GCRY_AC_KEY_PUBLIC, data);
    xk->pub=1;
    if (rc) {
      DBG_INFO(GWEN_LOGDOMAIN, "gcry_ac_key_init(): %s", gcry_strerror(rc));
      GWEN_Crypt_Key_free(k);
      gcry_ac_data_destroy(data);
      gcry_sexp_release(key);
      return GWEN_ERROR_GENERIC;
    }
    xk->keyValid=1;
    *pPubKey=k;
    gcry_ac_data_destroy(data);
  }

  skey=gcry_sexp_find_token(key, "private-key", 0);
  if (!skey) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Private part missing in return value");
    return GWEN_ERROR_GENERIC;
  }
  else {
    gcry_ac_data_t data;
    GWEN_CRYPT_KEY *k;
    GWEN_CRYPT_KEY_RSA *xk;

    /* allocate key data */
    rc=gcry_ac_data_new(&data);
    if (rc) {
      DBG_INFO(GWEN_LOGDOMAIN, "gcry_ac_data_new(): %s", gcry_strerror(rc));
      gcry_sexp_release(key);
      return GWEN_ERROR_GENERIC;
    }

    rc=GWEN_Crypt_KeyRsa__sKeyToDataPrivKey(data, skey);
    if (rc) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rc);
      gcry_ac_data_destroy(data);
      gcry_sexp_release(key);
      return rc;
    }
    gcry_sexp_release(skey);

    /* create public key */
    k=GWEN_Crypt_Key_new(GWEN_Crypt_CryptAlgoId_Rsa, nbytes);
    assert(k);
    GWEN_NEW_OBJECT(GWEN_CRYPT_KEY_RSA, xk);
    GWEN_INHERIT_SETDATA(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_RSA, k, xk,
			 GWEN_Crypt_KeyRsa_freeData);
    GWEN_Crypt_Key_SetSignFn(k, GWEN_Crypt_KeyRsa_Sign);
    GWEN_Crypt_Key_SetVerifyFn(k, GWEN_Crypt_KeyRsa_Verify);
    GWEN_Crypt_Key_SetEncipherFn(k, GWEN_Crypt_KeyRsa_Encipher);
    GWEN_Crypt_Key_SetDecipherFn(k, GWEN_Crypt_KeyRsa_Decipher);
  
    /* open algo for key */
    rc=gcry_ac_open(&xk->algoHandle, GCRY_AC_RSA, 0);
    if (rc) {
      DBG_INFO(GWEN_LOGDOMAIN, "gcry_ac_open(): %s", gcry_strerror(rc));
      GWEN_Crypt_Key_free(k);
      gcry_ac_data_destroy(data);
      gcry_sexp_release(key);
      return GWEN_ERROR_GENERIC;
    }
    xk->algoValid=1;
    /* copy key */
    rc=gcry_ac_key_init(&xk->key, xk->algoHandle, GCRY_AC_KEY_SECRET, data);
    xk->pub=0;
    if (rc) {
      DBG_INFO(GWEN_LOGDOMAIN, "gcry_ac_open(): %s", gcry_strerror(rc));
      GWEN_Crypt_Key_free(k);
      gcry_ac_data_destroy(data);
      gcry_sexp_release(key);
      return GWEN_ERROR_GENERIC;
    }
    xk->keyValid=1;
    *pSecretKey=k;
    gcry_ac_data_destroy(data);
  }

  gcry_sexp_release(key);
  return 0;
}



int GWEN_Crypt_KeyRsa__GetNamedElement(const GWEN_CRYPT_KEY *k,
                                       const char *name,
				       uint8_t *buffer,
				       uint32_t *pBufLen) {
  gcry_ac_data_t ds;
  GWEN_CRYPT_KEY_RSA *xk;
  gcry_mpi_t mpi;
  gcry_error_t err;
  unsigned char *buf;
  size_t nbytes;

  assert(k);
  xk=GWEN_INHERIT_GETDATA(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_RSA, k);
  assert(xk);

  if (xk->algoValid==0 || xk->keyValid==0) {
    DBG_INFO(GWEN_LOGDOMAIN, "Invalid key data");
    return GWEN_ERROR_GENERIC;
  }

  ds=gcry_ac_key_data_get(xk->key);

  /* read n (don't copy) */
  err=gcry_ac_data_get_name(ds, 0, name, &mpi);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_ac_data_get_name(): %d", err);
    if (err==GPG_ERR_INV_ARG)
      return GWEN_ERROR_NO_DATA;
    else
      return GWEN_ERROR_GENERIC;
  }

  /* write mpi as bin into a buffer which will be allocated by this function */
  err=gcry_mpi_aprint(GCRYMPI_FMT_USG, &buf, &nbytes, mpi);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_mpi_aprint(): %d", err);
    return GWEN_ERROR_GENERIC;
  }
  if (nbytes>*pBufLen) {
    DBG_INFO(GWEN_LOGDOMAIN, "Buffer too small");
    gcry_free(buf);
    return GWEN_ERROR_BUFFER_OVERFLOW;
  }

  memmove(buffer, buf, nbytes);
  *pBufLen=nbytes;
  gcry_free(buf);

  return 0;
}



int GWEN_Crypt_KeyRsa_GetModulus(const GWEN_CRYPT_KEY *k, uint8_t *buffer, uint32_t *pBufLen) {
  return GWEN_Crypt_KeyRsa__GetNamedElement(k, "n", buffer, pBufLen);
}



int GWEN_Crypt_KeyRsa_GetExponent(const GWEN_CRYPT_KEY *k, uint8_t *buffer, uint32_t *pBufLen) {
  return GWEN_Crypt_KeyRsa__GetNamedElement(k, "e", buffer, pBufLen);
}



GWEN_CRYPT_KEY *GWEN_Crypt_KeyRsa_fromModExp(unsigned int nbytes,
					     const uint8_t *pModulus,
					     uint32_t lModulus,
					     const uint8_t *pExponent,
					     uint32_t lExponent) {
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
    GWEN_DB_Dump(dbKey, stderr, 2);
    GWEN_DB_Group_free(dbKey);
    return NULL;
  }

  GWEN_DB_Group_free(dbKey);
  return key;
}



GWEN_CRYPT_KEY *GWEN_Crypt_KeyRsa_dup(const GWEN_CRYPT_KEY *k) {
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



uint32_t GWEN_Crypt_KeyRsa_GetFlags(const GWEN_CRYPT_KEY *k) {
  GWEN_CRYPT_KEY_RSA *xk;

  assert(k);
  xk=GWEN_INHERIT_GETDATA(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_RSA, k);
  assert(xk);

  return xk->flags;
}



void GWEN_Crypt_KeyRsa_SetFlags(GWEN_CRYPT_KEY *k, uint32_t fl) {
  GWEN_CRYPT_KEY_RSA *xk;

  assert(k);
  xk=GWEN_INHERIT_GETDATA(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_RSA, k);
  assert(xk);

  xk->flags=fl;
}



void GWEN_Crypt_KeyRsa_AddFlags(GWEN_CRYPT_KEY *k, uint32_t fl) {
  GWEN_CRYPT_KEY_RSA *xk;

  assert(k);
  xk=GWEN_INHERIT_GETDATA(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_RSA, k);
  assert(xk);

  xk->flags|=fl;
}



void GWEN_Crypt_KeyRsa_SubFlags(GWEN_CRYPT_KEY *k, uint32_t fl) {
  GWEN_CRYPT_KEY_RSA *xk;

  assert(k);
  xk=GWEN_INHERIT_GETDATA(GWEN_CRYPT_KEY, GWEN_CRYPT_KEY_RSA, k);
  assert(xk);

  xk->flags&=~fl;
}








