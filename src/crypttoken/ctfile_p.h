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


#ifndef GWEN_CRYPT_TOKEN_FILE_P_H
#define GWEN_CRYPT_TOKEN_FILE_P_H


#include "ctfile_be.h"

#include <gwenhywfar/fslock.h>
#include <gwenhywfar/cryptkey.h>



typedef struct GWEN_CRYPT_TOKEN_FILE GWEN_CRYPT_TOKEN_FILE;
struct GWEN_CRYPT_TOKEN_FILE {
  GWEN_CRYPT_TOKEN_CONTEXT_LIST *contextList;

  time_t ctime;
  time_t mtime;

  mode_t keyfile_mode;

  int fd;
  GWEN_FSLOCK *lock;

  GWEN_CRYPT_TOKEN_FILE_READ_FN readFn;
  GWEN_CRYPT_TOKEN_FILE_WRITE_FN writeFn;
};

static GWENHYWFAR_CB void GWEN_Crypt_TokenFile_freeData(void *bp, void *p);


static int GWEN_Crypt_TokenFile__OpenFile(GWEN_CRYPT_TOKEN *ct, int wr, uint32_t gid);
static int GWEN_Crypt_TokenFile__CloseFile(GWEN_CRYPT_TOKEN *ct, uint32_t gid);
static int GWEN_Crypt_TokenFile__Read(GWEN_CRYPT_TOKEN *ct, uint32_t gid);
static int GWEN_Crypt_TokenFile__Write(GWEN_CRYPT_TOKEN *ct, int cr, uint32_t gid);
static int GWEN_Crypt_TokenFile__ReadFile(GWEN_CRYPT_TOKEN *ct, uint32_t gid);
static int GWEN_Crypt_TokenFile__WriteFile(GWEN_CRYPT_TOKEN *ct, int cr, uint32_t gid);
static int GWEN_Crypt_TokenFile__ReloadIfNeeded(GWEN_CRYPT_TOKEN *ct, uint32_t gid);

static GWEN_CRYPT_KEY *GWEN_Crypt_TokenFile__GetKey(GWEN_CRYPT_TOKEN *ct, uint32_t id, uint32_t gid);


static int GWENHYWFAR_CB
  GWEN_Crypt_TokenFile_Create(GWEN_CRYPT_TOKEN *ct, uint32_t gid);
static int GWENHYWFAR_CB
  GWEN_Crypt_TokenFile_Open(GWEN_CRYPT_TOKEN *ct, int admin, uint32_t gid);
static int GWENHYWFAR_CB
  GWEN_Crypt_TokenFile_Close(GWEN_CRYPT_TOKEN *ct, int abandon, uint32_t gid);


static int GWENHYWFAR_CB
  GWEN_Crypt_TokenFile__GetKeyIdList(GWEN_CRYPT_TOKEN *ct,
				     uint32_t *pIdList,
				     uint32_t *pCount,
				     uint32_t gid);

static const GWEN_CRYPT_TOKEN_KEYINFO* GWENHYWFAR_CB
  GWEN_Crypt_TokenFile__GetKeyInfo(GWEN_CRYPT_TOKEN *ct,
				   uint32_t id,
				   uint32_t flags,
				   uint32_t gid);

static int GWENHYWFAR_CB
  GWEN_Crypt_TokenFile__SetKeyInfo(GWEN_CRYPT_TOKEN *ct,
				   uint32_t id,
				   const GWEN_CRYPT_TOKEN_KEYINFO *ki,
				   uint32_t gid);


static int GWENHYWFAR_CB
  GWEN_Crypt_TokenFile__GetContextIdList(GWEN_CRYPT_TOKEN *ct,
					 uint32_t *pIdList,
					 uint32_t *pCount,
					 uint32_t gid);

static const GWEN_CRYPT_TOKEN_CONTEXT* GWENHYWFAR_CB
  GWEN_Crypt_TokenFile__GetContext(GWEN_CRYPT_TOKEN *ct,
				   uint32_t id,
				   uint32_t gid);

static int GWENHYWFAR_CB
  GWEN_Crypt_TokenFile__SetContext(GWEN_CRYPT_TOKEN *ct,
				   uint32_t id,
				   const GWEN_CRYPT_TOKEN_CONTEXT *nctx,
				   uint32_t gid);


static int GWENHYWFAR_CB
  GWEN_Crypt_TokenFile__Sign(GWEN_CRYPT_TOKEN *ct,
			     uint32_t keyId,
			     GWEN_CRYPT_PADDALGO *a,
			     const uint8_t *pInData,
			     uint32_t inLen,
			     uint8_t *pSignatureData,
			     uint32_t *pSignatureLen,
			     uint32_t *pSeqCounter,
			     uint32_t gid);

static int GWENHYWFAR_CB
  GWEN_Crypt_TokenFile__Verify(GWEN_CRYPT_TOKEN *ct,
			       uint32_t keyId,
			       GWEN_CRYPT_PADDALGO *a,
			       const uint8_t *pInData,
			       uint32_t inLen,
			       const uint8_t *pSignatureData,
			       uint32_t signatureLen,
			       uint32_t seqCounter,
			       uint32_t gid);

static int GWENHYWFAR_CB
  GWEN_Crypt_TokenFile__Encipher(GWEN_CRYPT_TOKEN *ct,
				 uint32_t keyId,
				 GWEN_CRYPT_PADDALGO *a,
				 const uint8_t *pInData,
				 uint32_t inLen,
				 uint8_t *pOutData,
				 uint32_t *pOutLen,
				 uint32_t gid);

static int GWENHYWFAR_CB
  GWEN_Crypt_TokenFile__Decipher(GWEN_CRYPT_TOKEN *ct,
				 uint32_t keyId,
				 GWEN_CRYPT_PADDALGO *a,
				 const uint8_t *pInData,
				 uint32_t inLen,
				 uint8_t *pOutData,
				 uint32_t *pOutLen,
				 uint32_t gid);

static int GWENHYWFAR_CB
  GWEN_Crypt_TokenFile__GenerateKey(GWEN_CRYPT_TOKEN *ct,
				    uint32_t keyId,
				    const GWEN_CRYPT_CRYPTALGO *a,
				    uint32_t gid);






#endif




