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


#include "mdigestgc_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>



GWEN_INHERIT(GWEN_MDIGEST, GWEN_MDIGEST_GC)




int GWEN_MDigest_Gc_Begin(GWEN_MDIGEST *md) {
  GWEN_MDIGEST_GC *xmd;
  int err;

  assert(md);
  xmd=GWEN_INHERIT_GETDATA(GWEN_MDIGEST, GWEN_MDIGEST_GC, md);
  assert(xmd);

  err=gcry_md_open(&xmd->handle, xmd->algo, xmd->flags);
  if (err) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_md_open(): %d", err);
    return GWEN_ERROR_GENERIC;
  }

  xmd->isOpen=1;

  return 0;
}



int GWEN_MDigest_Gc_End(GWEN_MDIGEST *md) {
  GWEN_MDIGEST_GC *xmd;
  uint8_t *p;
  unsigned int len;

  assert(md);
  xmd=GWEN_INHERIT_GETDATA(GWEN_MDIGEST, GWEN_MDIGEST_GC, md);
  assert(xmd);

  gcry_md_final(xmd->handle);

  len=gcry_md_get_algo_dlen(xmd->algo);
  if (len<1) {
    DBG_INFO(GWEN_LOGDOMAIN, "gcry_md_get_algo_dlen(): %d", len);
    gcry_md_close(xmd->handle);
    xmd->isOpen=0;
    return GWEN_ERROR_GENERIC;
  }

  p=(uint8_t*)malloc(len);
  assert(p);
  memmove(p, gcry_md_read(xmd->handle, xmd->algo), len);

  GWEN_MDigest_SetDigestBuffer(md, p, len);

  gcry_md_close(xmd->handle);
  xmd->isOpen=0;
  return 0;
}



int GWEN_MDigest_Gc_Update(GWEN_MDIGEST *md, const uint8_t *buf, unsigned int l) {
  GWEN_MDIGEST_GC *xmd;

  assert(md);
  xmd=GWEN_INHERIT_GETDATA(GWEN_MDIGEST, GWEN_MDIGEST_GC, md);
  assert(xmd);

  if (xmd->isOpen==0) {
    DBG_INFO(GWEN_LOGDOMAIN, "MDigest not open");
    return GWEN_ERROR_NOT_OPEN;
  }
  gcry_md_write(xmd->handle, buf, l);

  return 0;
}



GWENHYWFAR_CB
void GWEN_Digest_Gc_freeData(GWEN_UNUSED void *bp, void *p) {
  GWEN_MDIGEST_GC *xmd;

  xmd=(GWEN_MDIGEST_GC*) p;
  if (xmd->isOpen) {
    gcry_md_close(xmd->handle);
    xmd->isOpen=0;
  }
  GWEN_FREE_OBJECT(xmd);
}



GWEN_MDIGEST *GWEN_MDigest_Gc_new(GWEN_CRYPT_HASHALGOID a) {
  GWEN_MDIGEST *md;
  GWEN_MDIGEST_GC *xmd;

  GWEN_NEW_OBJECT(GWEN_MDIGEST_GC, xmd)
  md=GWEN_MDigest_new(a);
  assert(md);
  GWEN_INHERIT_SETDATA(GWEN_MDIGEST, GWEN_MDIGEST_GC, md, xmd, GWEN_Digest_Gc_freeData)

  GWEN_MDigest_SetBeginFn(md, GWEN_MDigest_Gc_Begin);
  GWEN_MDigest_SetEndFn(md, GWEN_MDigest_Gc_End);
  GWEN_MDigest_SetUpdateFn(md, GWEN_MDigest_Gc_Update);

  return md;
}






GWEN_MDIGEST *GWEN_MDigest_Md5_new() {
  GWEN_MDIGEST *md;
  GWEN_MDIGEST_GC *xmd;

  md=GWEN_MDigest_Gc_new(GWEN_Crypt_HashAlgoId_Md5);
  assert(md);
  xmd=GWEN_INHERIT_GETDATA(GWEN_MDIGEST, GWEN_MDIGEST_GC, md);
  assert(xmd);

  xmd->algo=GCRY_MD_MD5;
  GWEN_MDigest_SetDigestLen(md, gcry_md_get_algo_dlen(xmd->algo));

  return md;
}



GWEN_MDIGEST *GWEN_MDigest_Rmd160_new() {
  GWEN_MDIGEST *md;
  GWEN_MDIGEST_GC *xmd;

  md=GWEN_MDigest_Gc_new(GWEN_Crypt_HashAlgoId_Rmd160);
  assert(md);
  xmd=GWEN_INHERIT_GETDATA(GWEN_MDIGEST, GWEN_MDIGEST_GC, md);
  assert(xmd);

  xmd->algo=GCRY_MD_RMD160;
  GWEN_MDigest_SetDigestLen(md, gcry_md_get_algo_dlen(xmd->algo));

  return md;
}



GWEN_MDIGEST *GWEN_MDigest_Sha1_new() {
  GWEN_MDIGEST *md;
  GWEN_MDIGEST_GC *xmd;

  md=GWEN_MDigest_Gc_new(GWEN_Crypt_HashAlgoId_Sha1);
  assert(md);
  xmd=GWEN_INHERIT_GETDATA(GWEN_MDIGEST, GWEN_MDIGEST_GC, md);
  assert(xmd);

  xmd->algo=GCRY_MD_SHA1;
  GWEN_MDigest_SetDigestLen(md, gcry_md_get_algo_dlen(xmd->algo));

  return md;
}



GWEN_MDIGEST *GWEN_MDigest_Sha256_new() {
  GWEN_MDIGEST *md;
  GWEN_MDIGEST_GC *xmd;

  md=GWEN_MDigest_Gc_new(GWEN_Crypt_HashAlgoId_Sha256);
  assert(md);
  xmd=GWEN_INHERIT_GETDATA(GWEN_MDIGEST, GWEN_MDIGEST_GC, md);
  assert(xmd);

  xmd->algo=GCRY_MD_SHA256;
  xmd->flags=GCRY_MD_FLAG_SECURE;
  GWEN_MDigest_SetDigestLen(md, gcry_md_get_algo_dlen(xmd->algo));

  return md;
}






