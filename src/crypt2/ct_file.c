/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Wed Mar 16 2005
    copyright   : (C) 2005 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include "ct_file_p.h"
#include <gwenhywfar/debug.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>



GWEN_INHERIT(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_FILE)


GWEN_INHERIT_FUNCTIONS(GWEN_CT_FILE_CONTEXT)
GWEN_LIST_FUNCTIONS(GWEN_CT_FILE_CONTEXT, GWEN_CryptTokenFile_Context)
GWEN_LIST2_FUNCTIONS(GWEN_CT_FILE_CONTEXT, GWEN_CryptTokenFile_Context)



GWEN_CT_FILE_CONTEXT *GWEN_CryptTokenFile_Context_new() {
  GWEN_CT_FILE_CONTEXT *fc;

  GWEN_NEW_OBJECT(GWEN_CT_FILE_CONTEXT, fc);
  GWEN_LIST_INIT(GWEN_CT_FILE_CONTEXT, fc);
  GWEN_INHERIT_INIT(GWEN_CT_FILE_CONTEXT, fc);

  fc->localSignSeq=1; /* always start with "1" */

  return fc;
}



void GWEN_CryptTokenFile_Context_free(GWEN_CT_FILE_CONTEXT *fc) {
  if (fc) {
    GWEN_LIST_FINI(GWEN_CT_FILE_CONTEXT, fc);
    GWEN_INHERIT_FINI(GWEN_CT_FILE_CONTEXT, fc);
    GWEN_CryptKey_free(fc->localSignKey);
    GWEN_CryptKey_free(fc->localCryptKey);
    GWEN_CryptKey_free(fc->remoteSignKey);
    GWEN_CryptKey_free(fc->remoteCryptKey);
    GWEN_CryptToken_User_new(fc->user);

    GWEN_FREE_OBJECT(fc);
  }
}



unsigned int
GWEN_CryptTokenFile_Context_GetLocalSignSeq(const GWEN_CT_FILE_CONTEXT *fc){
  assert(fc);
  return fc->localSignSeq;
}



void GWEN_CryptTokenFile_Context_SetLocalSignSeq(GWEN_CT_FILE_CONTEXT *fc,
                                                 unsigned int i){
  assert(fc);
  fc->localSignSeq=i;
}




unsigned int
GWEN_CryptTokenFile_Context_GetRemoteSignSeq(const GWEN_CT_FILE_CONTEXT *fc){
  assert(fc);
  return fc->remoteSignSeq;
}



void GWEN_CryptTokenFile_Context_SetRemoteSignSeq(GWEN_CT_FILE_CONTEXT *fc,
                                                  unsigned int i) {
  assert(fc);
  fc->remoteSignSeq=i;
}



GWEN_CRYPTKEY*
GWEN_CryptTokenFile_Context_GetLocalSignKey(const GWEN_CT_FILE_CONTEXT *fc){
  assert(fc);
  return fc->localSignKey;
}



void GWEN_CryptTokenFile_Context_SetLocalSignKey(GWEN_CT_FILE_CONTEXT *fc,
                                                 GWEN_CRYPTKEY *key) {
  assert(fc);
  fc->localSignKey=key;
}



GWEN_CRYPTKEY*
GWEN_CryptTokenFile_Context_GetLocalCryptKey(const GWEN_CT_FILE_CONTEXT *fc){
  assert(fc);
  return fc->localCryptKey;
}



void GWEN_CryptTokenFile_Context_SetLocalCryptKey(GWEN_CT_FILE_CONTEXT *fc,
                                                  GWEN_CRYPTKEY *key){
  assert(fc);
  fc->localCryptKey=key;
}




GWEN_CRYPTKEY*
GWEN_CryptTokenFile_Context_GetRemoteSignKey(const GWEN_CT_FILE_CONTEXT *fc){
  assert(fc);
  return fc->remoteSignKey;
}



void GWEN_CryptTokenFile_Context_SetRemoteSignKey(GWEN_CT_FILE_CONTEXT *fc,
                                                  GWEN_CRYPTKEY *key){
  assert(fc);
  fc->remoteSignKey=key;
}



GWEN_CRYPTKEY*
GWEN_CryptTokenFile_Context_GetRemoteCryptKey(const GWEN_CT_FILE_CONTEXT *fc){
  assert(fc);
  return fc->remoteCryptKey;
}



void GWEN_CryptTokenFile_Context_SetRemoteCryptKey(GWEN_CT_FILE_CONTEXT *fc,
                                                   GWEN_CRYPTKEY *key){
  assert(fc);
  fc->remoteCryptKey=key;
}



GWEN_CRYPTTOKEN_USER*
GWEN_CryptTokenFile_Context_GetUser(const GWEN_CT_FILE_CONTEXT *fc){
  assert(fc);
  return fc->user;
}



void GWEN_CryptTokenFile_Context_SetUser(GWEN_CT_FILE_CONTEXT *fc,
                                         GWEN_CRYPTTOKEN_USER *u){
  assert(fc);
  fc->user=u;
}









GWEN_CRYPTTOKEN *GWEN_CryptTokenFile_new(GWEN_PLUGIN_MANAGER *pm,
                                         const char *typeName,
                                         const char *subTypeName,
                                         const char *name) {
  GWEN_CRYPTTOKEN *ct;
  GWEN_CRYPTTOKEN_FILE *lct;

  ct=GWEN_CryptToken_new(pm,
                         GWEN_CryptToken_Device_File,
                         typeName, subTypeName, name);
  GWEN_NEW_OBJECT(GWEN_CRYPTTOKEN_FILE, lct);
  GWEN_INHERIT_SETDATA(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_FILE, ct, lct,
                       GWEN_CryptTokenFile_FreeData);

  lct->fileContextList=GWEN_CryptTokenFile_Context_List_new();

  GWEN_CryptToken_SetOpenFn(ct, GWEN_CryptTokenFile_Open);
  GWEN_CryptToken_SetCloseFn(ct, GWEN_CryptTokenFile_Close);
  GWEN_CryptToken_SetCreateFn(ct, GWEN_CryptTokenFile_Create);
  GWEN_CryptToken_SetSignFn(ct, GWEN_CryptTokenFile_Sign);
  GWEN_CryptToken_SetVerifyFn(ct, GWEN_CryptTokenFile_Verify);
  GWEN_CryptToken_SetEncryptFn(ct, GWEN_CryptTokenFile_Encrypt);
  GWEN_CryptToken_SetDecryptFn(ct, GWEN_CryptTokenFile_Decrypt);
  GWEN_CryptToken_SetGetSignSeqFn(ct, GWEN_CryptTokenFile_GetSignSeq);
  GWEN_CryptToken_SetReadKeyFn(ct, GWEN_CryptTokenFile_ReadKey);
  GWEN_CryptToken_SetWriteKeyFn(ct, GWEN_CryptTokenFile_WriteKey);
  GWEN_CryptToken_SetReadKeySpecFn(ct, GWEN_CryptTokenFile_ReadKeySpec);
  GWEN_CryptToken_SetWriteKeySpecFn(ct, GWEN_CryptTokenFile_WriteKeySpec);
  GWEN_CryptToken_SetGenerateKeyFn(ct, GWEN_CryptTokenFile_GenerateKey);
  GWEN_CryptToken_SetFillUserListFn(ct, GWEN_CryptTokenFile_FillUserList);

  return ct;
}



void GWEN_CryptTokenFile_FreeData(void *bp, void *p) {
  GWEN_CRYPTTOKEN_FILE *lct;

  lct=(GWEN_CRYPTTOKEN_FILE*)p;
  GWEN_CryptTokenFile_Context_List_free(lct->fileContextList);
  GWEN_FREE_OBJECT(lct);
}



GWEN_CT_FILE_CONTEXT_LIST*
GWEN_CryptTokenFile_GetFileContextList(GWEN_CRYPTTOKEN *ct) {
  GWEN_CRYPTTOKEN_FILE *lct;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_FILE, ct);
  assert(lct);

  return lct->fileContextList;
}



void GWEN_CryptTokenFile_ClearFileContextList(GWEN_CRYPTTOKEN *ct) {
  GWEN_CRYPTTOKEN_FILE *lct;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_FILE, ct);
  assert(lct);

  GWEN_CryptTokenFile_Context_List_Clear(lct->fileContextList);
}



void GWEN_CryptTokenFile_AddFileContext(GWEN_CRYPTTOKEN *ct,
                                        GWEN_CT_FILE_CONTEXT *fc) {
  GWEN_CRYPTTOKEN_FILE *lct;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_FILE, ct);
  assert(lct);

  GWEN_CryptTokenFile_Context_List_Add(fc, lct->fileContextList);
}



void GWEN_CryptTokenFile_SetReadFn(GWEN_CRYPTTOKEN *ct,
                                   GWEN_CRYPTTOKEN_FILE_READ_FN fn) {
  GWEN_CRYPTTOKEN_FILE *lct;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_FILE, ct);
  assert(lct);

  lct->readFn=fn;
}



void GWEN_CryptTokenFile_SetWriteFn(GWEN_CRYPTTOKEN *ct,
                                    GWEN_CRYPTTOKEN_FILE_WRITE_FN fn) {
  GWEN_CRYPTTOKEN_FILE *lct;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_FILE, ct);
  assert(lct);

  lct->writeFn=fn;
}




int GWEN_CryptTokenFile__OpenFile(GWEN_CRYPTTOKEN *ct, int wr){
  int fd;
  GWEN_CRYPTTOKEN_FILE *lct;
  GWEN_FSLOCK_RESULT lres;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_FILE, ct);
  assert(lct);

  lct->lock=GWEN_FSLock_new(GWEN_CryptToken_GetTokenName(ct),
                            GWEN_FSLock_TypeFile);
  lres=GWEN_FSLock_Lock(lct->lock, 10000);
  if (lres!=GWEN_FSLock_ResultOk) {
    GWEN_FSLock_free(lct->lock);
    lct->lock=0;
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not lock file");
    if (lres==GWEN_FSLock_ResultUserAbort)
      return GWEN_ERROR_USER_ABORTED;
    else
      return GWEN_ERROR_CT_IO_ERROR;
  }

  if (wr) {
    /* write file */
    fd=open(GWEN_CryptToken_GetTokenName(ct),
            O_RDWR|O_CREAT
#ifdef OS_WIN32
            | O_BINARY
#endif
            ,
	    S_IRUSR|S_IWUSR | lct->keyfile_mode);
  }
  else {
    /* Remember the access permissions when opening the file */
    struct stat statbuffer;
    if (!stat(GWEN_CryptToken_GetTokenName(ct), &statbuffer)) {
      /* Save the access mode, but masked by the bit masks for
	 user/group/other permissions */
      lct->keyfile_mode = 
	statbuffer.st_mode & (S_IRWXU
#ifndef OS_WIN32
			      | S_IRWXG | S_IRWXO
#endif
			      );
    }

    /* and open the file */
    fd=open(GWEN_CryptToken_GetTokenName(ct),
            O_RDONLY
#ifdef OS_WIN32
            | O_BINARY
#endif
           );
  }

  if (fd==-1) {
    DBG_ERROR(GWEN_LOGDOMAIN,
	      "open(%s): %s",
	      GWEN_CryptToken_GetTokenName(ct),
	      strerror(errno));
    GWEN_FSLock_Unlock(lct->lock);
    GWEN_FSLock_free(lct->lock);
    lct->lock=0;
    return -1;
  }

  lct->fd=fd;

  return 0;
}



int GWEN_CryptTokenFile__Read(GWEN_CRYPTTOKEN *ct){
  GWEN_CRYPTTOKEN_FILE *lct;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_FILE, ct);
  assert(lct);

  assert(lct->readFn);
  if (lseek(lct->fd, 0, SEEK_SET)==-1) {
    DBG_ERROR(GWEN_LOGDOMAIN, "lseek(%s): %s",
              GWEN_CryptToken_GetTokenName(ct),
              strerror(errno));
    return GWEN_ERROR_CT_IO_ERROR;
  }
  return lct->readFn(ct, lct->fd);
}



int GWEN_CryptTokenFile__Write(GWEN_CRYPTTOKEN *ct, int cr){
  GWEN_CRYPTTOKEN_FILE *lct;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_FILE, ct);
  assert(lct);

  if (lct->writeFn==0) {
    DBG_WARN(GWEN_LOGDOMAIN,
             "No write function in crypt token type \"%s\"",
             GWEN_CryptToken_GetTokenType(ct));
    return GWEN_ERROR_CT_NOT_SUPPORTED;
  }

  if (lseek(lct->fd, 0, SEEK_SET)==-1) {
    DBG_ERROR(GWEN_LOGDOMAIN, "lseek(%s): %s",
              GWEN_CryptToken_GetTokenName(ct),
              strerror(errno));
    return GWEN_ERROR_CT_IO_ERROR;
  }
  return lct->writeFn(ct, lct->fd, cr);
}



int GWEN_CryptTokenFile__CloseFile(GWEN_CRYPTTOKEN *ct){
  GWEN_CRYPTTOKEN_FILE *lct;
  GWEN_FSLOCK_RESULT lres;
  struct stat st;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_FILE, ct);
  assert(lct);

  if (lct->fd==-1) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Keyfile \"%s\"not open",
              GWEN_CryptToken_GetTokenName(ct));
    return -1;
  }

  if (close(lct->fd)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "close(%s): %s",
              GWEN_CryptToken_GetTokenName(ct), strerror(errno));
    lct->fd=-1;
    GWEN_FSLock_Unlock(lct->lock);
    GWEN_FSLock_free(lct->lock);
    lct->lock=0;
    return -1;
  }
  lct->fd=-1;

  lres=GWEN_FSLock_Unlock(lct->lock);
  if (lres!=GWEN_FSLock_ResultOk) {
    DBG_WARN(GWEN_LOGDOMAIN, "Error removing lock from \"%s\": %d",
             GWEN_CryptToken_GetTokenName(ct), lres);
  }
  GWEN_FSLock_free(lct->lock);
  lct->lock=0;

  /* get times */
  if (stat(GWEN_CryptToken_GetTokenName(ct), &st)) {
    DBG_ERROR(GWEN_LOGDOMAIN,
	      "stat(%s): %s",
	      GWEN_CryptToken_GetTokenName(ct),
	      strerror(errno));
    return GWEN_ERROR_GENERIC;
  }

#ifndef OS_WIN32
  if (st.st_mode & 0007) {
    DBG_WARN(GWEN_LOGDOMAIN,
             "WARNING: Your keyfile \"%s\" is world accessible!\n"
             "Nobody but you should have access to the file. You \n"
	     "should probably change this with \"chmod 600 %s\"",
             GWEN_CryptToken_GetTokenName(ct),
             GWEN_CryptToken_GetTokenName(ct));
  }
#endif
  lct->mtime=st.st_mtime;
  lct->ctime=st.st_ctime;

  return 0;
}



int GWEN_CryptTokenFile__ReadFile(GWEN_CRYPTTOKEN *ct){
  GWEN_CRYPTTOKEN_FILE *lct;
  int rv;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_FILE, ct);
  assert(lct);

  /* open file */
  rv=GWEN_CryptTokenFile__OpenFile(ct, 0);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN,
             "Could not open keyfile for reading (%d)", rv);
    return rv;
  }

  /* read file */
  rv=GWEN_CryptTokenFile__Read(ct);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error reading keyfile");
    GWEN_CryptTokenFile__CloseFile(ct);
    return rv;
  }

  /* close file */
  rv=GWEN_CryptTokenFile__CloseFile(ct);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "Could not close keyfile");
    return rv;
  }

  return 0;
}



int GWEN_CryptTokenFile__WriteFile(GWEN_CRYPTTOKEN *ct, int cr){
  GWEN_CRYPTTOKEN_FILE *lct;
  int rv;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_FILE, ct);
  assert(lct);

  /* open file */
  rv=GWEN_CryptTokenFile__OpenFile(ct, 1);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN,
             "Could not open keyfile for writing (%d)", rv);
    return rv;
  }

  /* write file */
  rv=GWEN_CryptTokenFile__Write(ct, cr);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error writing keyfile");
    GWEN_CryptTokenFile__CloseFile(ct);
    return rv;
  }

  /* close file */
  rv=GWEN_CryptTokenFile__CloseFile(ct);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "Could not close keyfile");
    return rv;
  }

  return 0;
}



int GWEN_CryptTokenFile__ReloadIfNeeded(GWEN_CRYPTTOKEN *ct){
  GWEN_CRYPTTOKEN_FILE *lct;
  struct stat st;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_FILE, ct);
  assert(lct);

  if (stat(GWEN_CryptToken_GetTokenName(ct), &st)) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "stat(%s): %s",
              GWEN_CryptToken_GetTokenName(ct),
              strerror(errno));
    return -1;
  }
  if (lct->mtime!=st.st_mtime ||
      lct->ctime!=st.st_ctime) {
    int rv;

    /* file has changed, reload it */
    DBG_NOTICE(GWEN_LOGDOMAIN,
               "Keyfile changed externally, reloading it");
    /* read file */
    rv=GWEN_CryptTokenFile__ReadFile(ct);
    if (rv) {
      DBG_WARN(GWEN_LOGDOMAIN, "Error reloading keyfile");
      return rv;
    }
  }
  else {
    DBG_NOTICE(GWEN_LOGDOMAIN, "Keyfile unchanged, not reloading");
  }
  return 0;
}




int GWEN_CryptTokenFile_Create(GWEN_CRYPTTOKEN *ct){
  GWEN_CRYPTTOKEN_FILE *lct;
  struct stat st;
  int fd;
  int rv;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_FILE, ct);
  assert(lct);

  if (!GWEN_CryptToken_GetTokenName(ct)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No medium name given");
    return GWEN_ERROR_INVALID;
  }

  if (stat(GWEN_CryptToken_GetTokenName(ct), &st)) {
    if (errno!=ENOENT) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "stat(%s): %s",
                GWEN_CryptToken_GetTokenName(ct),
                strerror(errno));
      return GWEN_ERROR_CT_IO_ERROR;
    }
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Keyfile \"%s\" already exists, will not create it",
              GWEN_CryptToken_GetTokenName(ct));
    return GWEN_ERROR_INVALID;
  }


  /* create file */
  fd=open(GWEN_CryptToken_GetTokenName(ct),
          O_RDWR | O_CREAT | O_EXCL
#ifdef OS_WIN32
          | O_BINARY
#endif
          ,
          S_IRUSR|S_IWUSR);


  if (fd==-1) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "open(%s): %s",
              GWEN_CryptToken_GetTokenName(ct),
              strerror(errno));
    return GWEN_ERROR_CT_IO_ERROR;
  }

  close(fd);

  rv=GWEN_CryptTokenFile__WriteFile(ct, 1);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    return rv;
  }

  return 0;
}



int GWEN_CryptTokenFile_Open(GWEN_CRYPTTOKEN *ct, int managed){
  GWEN_CRYPTTOKEN_FILE *lct;
  int rv;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_FILE, ct);
  assert(lct);

  rv=GWEN_CryptTokenFile__ReadFile(ct);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    return rv;
  }

  return 0;
}



int GWEN_CryptTokenFile_Close(GWEN_CRYPTTOKEN *ct){
  GWEN_CRYPTTOKEN_FILE *lct;
  int rv;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_FILE, ct);
  assert(lct);

  rv=GWEN_CryptTokenFile__WriteFile(ct, 0);

  /* free/reset all data */
  GWEN_CryptTokenFile_ClearFileContextList(ct);
  lct->mtime=0;
  lct->ctime=0;

  return rv;
}




GWEN_CT_FILE_CONTEXT*
GWEN_CryptTokenFile__GetFileContextByKeyId(GWEN_CRYPTTOKEN *ct,
                                           GWEN_TYPE_UINT32 kid,
                                           const GWEN_CRYPTTOKEN_CONTEXT **pctx,
                                           const GWEN_CRYPTTOKEN_KEYINFO **pki) {
  GWEN_CRYPTTOKEN_FILE *lct;
  GWEN_CT_FILE_CONTEXT *fc;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_FILE, ct);
  assert(lct);

  fc=GWEN_CryptTokenFile_Context_List_First(lct->fileContextList);
  if (fc==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No context");
  }
  while(fc) {
    GWEN_CRYPTTOKEN_USER *u;

    u=GWEN_CryptTokenFile_Context_GetUser(fc);
    if (u) {
      GWEN_TYPE_UINT32 cid;

      cid=GWEN_CryptToken_User_GetContextId(u);
      if (cid) {
        const GWEN_CRYPTTOKEN_CONTEXT *ctx;

        ctx=GWEN_CryptToken_GetContextById(ct, cid);
        if (ctx) {
          const GWEN_CRYPTTOKEN_KEYINFO *ki;

          ki=GWEN_CryptToken_Context_GetSignKeyInfo(ctx);
          if (ki && GWEN_CryptToken_KeyInfo_GetKeyId(ki)==(int)kid) {
            DBG_DEBUG(GWEN_LOGDOMAIN, "Context found");
            if (pctx)
              *pctx=ctx;
            if (pki)
              *pki=ki;
            return fc;
          }

          ki=GWEN_CryptToken_Context_GetVerifyKeyInfo(ctx);
          if (ki && GWEN_CryptToken_KeyInfo_GetKeyId(ki)==(int)kid) {
            DBG_DEBUG(GWEN_LOGDOMAIN, "Context found");
            if (pctx)
              *pctx=ctx;
            if (pki)
              *pki=ki;
            return fc;
          }

          ki=GWEN_CryptToken_Context_GetEncryptKeyInfo(ctx);
          if (ki && GWEN_CryptToken_KeyInfo_GetKeyId(ki)==(int)kid) {
            DBG_DEBUG(GWEN_LOGDOMAIN, "Context found");
            if (pctx)
              *pctx=ctx;
            if (pki)
              *pki=ki;
            return fc;
          }

          ki=GWEN_CryptToken_Context_GetDecryptKeyInfo(ctx);
          if (ki && GWEN_CryptToken_KeyInfo_GetKeyId(ki)==(int)kid) {
            DBG_DEBUG(GWEN_LOGDOMAIN, "Context found");
            if (pctx)
              *pctx=ctx;
            if (pki)
              *pki=ki;
            return fc;
          }
        }
      }
      else {
        DBG_ERROR(GWEN_LOGDOMAIN, "No context id in user data");
      }
    }
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "No user in context");
    }
    fc=GWEN_CryptTokenFile_Context_List_Next(fc);
  }

  return 0;
}



int GWEN_CryptTokenFile_GetSignSeq(GWEN_CRYPTTOKEN *ct,
                                   GWEN_TYPE_UINT32 kid,
                                   GWEN_TYPE_UINT32 *signSeq) {
  GWEN_CRYPTTOKEN_FILE *lct;
  GWEN_CT_FILE_CONTEXT *fctx;
  int rv;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_FILE, ct);
  assert(lct);

  rv=GWEN_CryptTokenFile__ReloadIfNeeded(ct);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error reloading (%d)", rv);
    return rv;
  }

  fctx=GWEN_CryptTokenFile__GetFileContextByKeyId(ct, kid, 0, 0);
  if (!fctx) {
    DBG_ERROR(GWEN_LOGDOMAIN, "File context for key not found");
    return GWEN_ERROR_GENERIC;
  }
  *signSeq=GWEN_CryptTokenFile_Context_GetLocalSignSeq(fctx);

  return 0;
}



int GWEN_CryptTokenFile_ReadKey(GWEN_CRYPTTOKEN *ct,
                                GWEN_TYPE_UINT32 kid,
                                GWEN_CRYPTKEY **key) {
  GWEN_CRYPTTOKEN_FILE *lct;
  GWEN_CRYPTKEY *k;
  GWEN_CT_FILE_CONTEXT *fctx;
  int rv;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_FILE, ct);
  assert(lct);

  assert(key);

  rv=GWEN_CryptTokenFile__ReloadIfNeeded(ct);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error reloading (%d)", rv);
    return rv;
  }

  fctx=GWEN_CryptTokenFile__GetFileContextByKeyId(ct, kid, 0, 0);
  if (!fctx) {
    DBG_ERROR(GWEN_LOGDOMAIN, "File context for key %d not found", kid);
    return GWEN_ERROR_GENERIC;
  }

  switch(kid & 0xff) {
  case 1: k=GWEN_CryptTokenFile_Context_GetLocalSignKey(fctx); break;
  case 2: k=GWEN_CryptTokenFile_Context_GetLocalCryptKey(fctx); break;
  case 3: k=GWEN_CryptTokenFile_Context_GetRemoteSignKey(fctx); break;
  case 4: k=GWEN_CryptTokenFile_Context_GetRemoteCryptKey(fctx); break;
  default:
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Invalid key id %d", kid);
    return GWEN_ERROR_INVALID;
  }

  if (k) {
    GWEN_DB_NODE *db;
    GWEN_ERRORCODE err;

    /* make sure we always export the public part only */
    db=GWEN_DB_Group_new("key");
    err=GWEN_CryptKey_toDb(k, db, 1);
    if (!GWEN_Error_IsOk(err)) {
      DBG_INFO_ERR(GWEN_LOGDOMAIN, err);
      GWEN_DB_Group_free(db);
      return GWEN_ERROR_GENERIC;
    }
  
    *key=GWEN_CryptKey_fromDb(db);
    if (!*key) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Could not create key from previous export");
      GWEN_DB_Group_free(db);
      return GWEN_ERROR_GENERIC;
    }

    GWEN_DB_Group_free(db);
  }
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "No data for key id %d", kid);
    return GWEN_ERROR_NO_DATA;
  }
  return 0;
}



int GWEN_CryptTokenFile_WriteKey(GWEN_CRYPTTOKEN *ct,
                                 GWEN_TYPE_UINT32 kid,
                                 const GWEN_CRYPTKEY *key) {
  GWEN_CRYPTTOKEN_FILE *lct;
  GWEN_CRYPTKEY *k;
  GWEN_CT_FILE_CONTEXT *fctx;
  int rv;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_FILE, ct);
  assert(lct);

  rv=GWEN_CryptTokenFile__ReloadIfNeeded(ct);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error reloading (%d)", rv);
    return rv;
  }

  fctx=GWEN_CryptTokenFile__GetFileContextByKeyId(ct, kid, 0, 0);
  if (!fctx) {
    DBG_ERROR(GWEN_LOGDOMAIN, "File context for key not found");
    return GWEN_ERROR_GENERIC;
  }

  if (key) k=GWEN_CryptKey_dup(key);
  else k=0;

  switch(kid & 0xff) {
  case 1: GWEN_CryptTokenFile_Context_SetLocalSignKey(fctx, k); break;
  case 2: GWEN_CryptTokenFile_Context_SetLocalCryptKey(fctx, k); break;
  case 3: GWEN_CryptTokenFile_Context_SetRemoteSignKey(fctx, k); break;
  case 4: GWEN_CryptTokenFile_Context_SetRemoteCryptKey(fctx, k); break;
  default:
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Invalid key id %d", kid);
    GWEN_CryptKey_free(k);
    return GWEN_ERROR_INVALID;
  }

  return 0;
}



int GWEN_CryptTokenFile_ReadKeySpec(GWEN_CRYPTTOKEN *ct,
                                    GWEN_TYPE_UINT32 kid,
                                    GWEN_KEYSPEC **ks) {
  GWEN_CRYPTTOKEN_FILE *lct;
  GWEN_CRYPTKEY *k;
  GWEN_CT_FILE_CONTEXT *fctx;
  int rv;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_FILE, ct);
  assert(lct);

  assert(ks);

  rv=GWEN_CryptTokenFile__ReloadIfNeeded(ct);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error reloading (%d)", rv);
    return rv;
  }

  fctx=GWEN_CryptTokenFile__GetFileContextByKeyId(ct, kid, 0, 0);
  if (!fctx) {
    DBG_ERROR(GWEN_LOGDOMAIN, "File context for key not found");
    return GWEN_ERROR_GENERIC;
  }

  switch(kid & 0xff) {
  case 1: k=GWEN_CryptTokenFile_Context_GetLocalSignKey(fctx); break;
  case 2: k=GWEN_CryptTokenFile_Context_GetLocalCryptKey(fctx); break;
  case 3: k=GWEN_CryptTokenFile_Context_GetRemoteSignKey(fctx); break;
  case 4: k=GWEN_CryptTokenFile_Context_GetRemoteCryptKey(fctx); break;
  default:
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Invalid key id %d", kid);
    return GWEN_ERROR_INVALID;
  }

  if (k) {
    const GWEN_KEYSPEC *cks;

    cks=GWEN_CryptKey_GetKeySpec(k);
    assert(cks);
    *ks=GWEN_KeySpec_dup(cks);
  }
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "No data for key id %d", kid);
    return GWEN_ERROR_NO_DATA;
  }

  return 0;
}



int GWEN_CryptTokenFile_WriteKeySpec(GWEN_CRYPTTOKEN *ct,
                                     GWEN_TYPE_UINT32 kid,
                                     const GWEN_KEYSPEC *ks) {
  GWEN_CRYPTTOKEN_FILE *lct;
  GWEN_CRYPTKEY *k;
  GWEN_CT_FILE_CONTEXT *fctx;
  int rv;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_FILE, ct);
  assert(lct);

  assert(ks);

  rv=GWEN_CryptTokenFile__ReloadIfNeeded(ct);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error reloading (%d)", rv);
    return rv;
  }

  fctx=GWEN_CryptTokenFile__GetFileContextByKeyId(ct, kid, 0, 0);
  if (!fctx) {
    DBG_ERROR(GWEN_LOGDOMAIN, "File context for key not found");
    return GWEN_ERROR_GENERIC;
  }

  switch(kid & 0xff) {
  case 1: k=GWEN_CryptTokenFile_Context_GetLocalSignKey(fctx); break;
  case 2: k=GWEN_CryptTokenFile_Context_GetLocalCryptKey(fctx); break;
  case 3: k=GWEN_CryptTokenFile_Context_GetRemoteSignKey(fctx); break;
  case 4: k=GWEN_CryptTokenFile_Context_GetRemoteCryptKey(fctx); break;
  default:
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Invalid key id %d", kid);
    return GWEN_ERROR_INVALID;
  }

  if (k) {
    GWEN_CryptKey_SetKeySpec(k, ks);
  }
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "No data for key id %d", kid);
    return GWEN_ERROR_NO_DATA;
  }

  return 0;
}



int GWEN_CryptTokenFile_GenerateKey(GWEN_CRYPTTOKEN *ct,
                                    const GWEN_CRYPTTOKEN_KEYINFO *ki,
                                    GWEN_CRYPTKEY **key) {
  GWEN_CRYPTTOKEN_FILE *lct;
  GWEN_CRYPTKEY *k;
  GWEN_CT_FILE_CONTEXT *fctx;
  GWEN_TYPE_UINT32 kid;
  GWEN_ERRORCODE err;
  int rv;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_FILE, ct);
  assert(lct);

  assert(ki);

  rv=GWEN_CryptTokenFile__ReloadIfNeeded(ct);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error reloading (%d)", rv);
    return rv;
  }

  kid=GWEN_CryptToken_KeyInfo_GetKeyId(ki);
  if (kid<1 || kid>4) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid key id (%d)", kid);
    return GWEN_ERROR_INVALID;
  }

  fctx=GWEN_CryptTokenFile__GetFileContextByKeyId(ct, kid, 0, 0);
  if (!fctx) {
    DBG_ERROR(GWEN_LOGDOMAIN, "File context for key not found");
    return GWEN_ERROR_GENERIC;
  }

  k=GWEN_CryptKey_Factory("rsa");
  if (!k) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Key type \"rsa\" not available");
    return GWEN_ERROR_GENERIC;
  }

  err=GWEN_CryptKey_Generate(k, GWEN_CryptToken_KeyInfo_GetKeySize(ki));
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
    GWEN_CryptKey_free(k);
    return GWEN_ERROR_CT_IO_ERROR;
  }

  switch(kid & 0xff) {
  case 1: GWEN_CryptTokenFile_Context_SetLocalSignKey(fctx, k); break;
  case 2: GWEN_CryptTokenFile_Context_SetLocalCryptKey(fctx, k); break;
  case 3: GWEN_CryptTokenFile_Context_SetRemoteSignKey(fctx, k); break;
  case 4: GWEN_CryptTokenFile_Context_SetRemoteCryptKey(fctx, k); break;
  default:
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Invalid key id %d", kid);
    GWEN_CryptKey_free(k);
    return GWEN_ERROR_INVALID;
  }

  if (key)
    *key=GWEN_CryptKey_dup(k);

  return 0;
}



int GWEN_CryptTokenFile_FillUserList(GWEN_CRYPTTOKEN *ct,
                                     GWEN_CRYPTTOKEN_USER_LIST *ul) {
  GWEN_CRYPTTOKEN_FILE *lct;
  GWEN_CT_FILE_CONTEXT *fc;
  int rv;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_FILE, ct);
  assert(lct);

  rv=GWEN_CryptTokenFile__ReloadIfNeeded(ct);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error reloading (%d)", rv);
    return rv;
  }

  fc=GWEN_CryptTokenFile_Context_List_First(lct->fileContextList);
  while(fc) {
    GWEN_CRYPTTOKEN_USER *u;

    u=GWEN_CryptTokenFile_Context_GetUser(fc);
    if (!u) {
      DBG_ERROR(GWEN_LOGDOMAIN, "File context has no user");
      return GWEN_ERROR_GENERIC;
    }
    else {
      GWEN_CRYPTTOKEN_USER *nu;

      nu=GWEN_CryptToken_User_dup(u);
      GWEN_CryptToken_User_List_Add(nu, ul);
    }
    fc=GWEN_CryptTokenFile_Context_List_Next(fc);
  }

  return 0;
}



int GWEN_CryptTokenFile_Sign(GWEN_CRYPTTOKEN *ct,
                             const GWEN_CRYPTTOKEN_CONTEXT *ctx,
                             const char *ptr,
                             unsigned int len,
                             GWEN_BUFFER *dst) {
  GWEN_CRYPTTOKEN_FILE *lct;
  GWEN_CT_FILE_CONTEXT *fctx;
  const GWEN_CRYPTTOKEN_KEYINFO *ki;
  const GWEN_CRYPTTOKEN_SIGNINFO *si;
  int rv;
  GWEN_BUFFER *hbuf;
  GWEN_ERRORCODE err;
  GWEN_TYPE_UINT32 kid;
  GWEN_CRYPTKEY *key;
  unsigned int ui;
  int chunkSize;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_FILE, ct);
  assert(lct);

  /* reload if necessary */
  rv=GWEN_CryptTokenFile__ReloadIfNeeded(ct);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error reloading (%d)", rv);
    return rv;
  }

  /* get sign info */
  si=GWEN_CryptToken_Context_GetSignInfo(ctx);
  assert(si);

  /* get keyinfo and perform some checks */
  ki=GWEN_CryptToken_Context_GetSignKeyInfo(ctx);
  assert(ki);
  kid=GWEN_CryptToken_KeyInfo_GetKeyId(ki);
  if ((kid & 0xf)!=1) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid key id");
    return GWEN_ERROR_INVALID;
  }
  if (!(GWEN_CryptToken_KeyInfo_GetKeyFlags(ki) &
	GWEN_CRYPTTOKEN_KEYINFO_FLAGS_CAN_SIGN)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Key can not be used for signing");
    return GWEN_ERROR_INVALID;
  }
  if (GWEN_CryptToken_KeyInfo_GetCryptAlgo(ki)!=
      GWEN_CryptToken_CryptAlgo_RSA) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid crypt algo");
    return GWEN_ERROR_INVALID;
  }

  /* get user context */
  fctx=GWEN_CryptTokenFile__GetFileContextByKeyId(ct, kid, 0, 0);
  if (!fctx) {
    DBG_ERROR(GWEN_LOGDOMAIN, "File context for key not found");
    return GWEN_ERROR_GENERIC;
  }

  /* check for existence of the key */
  key=GWEN_CryptTokenFile_Context_GetLocalSignKey(fctx);
  if (key==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No key");
    return GWEN_ERROR_CT_NO_KEY;
  }

  chunkSize=GWEN_CryptKey_GetChunkSize(key);
  if (chunkSize!=GWEN_CryptToken_KeyInfo_GetChunkSize(ki)) {
    DBG_WARN(GWEN_LOGDOMAIN,
             "ChunkSize of key != ChunkSize in CryptToken (%d!=%d)",
             chunkSize, GWEN_CryptToken_KeyInfo_GetChunkSize(ki));
  }

  /* hash data */
  hbuf=GWEN_Buffer_new(0, chunkSize, 0, 1);
  rv=GWEN_CryptToken_Hash(GWEN_CryptToken_SignInfo_GetHashAlgo(si),
                          ptr, len,
			  hbuf);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    GWEN_Buffer_free(hbuf);
    return rv;
  }

  /* padd hash */
  GWEN_Buffer_Rewind(hbuf);
  rv=GWEN_CryptToken_Padd(GWEN_CryptToken_SignInfo_GetPaddAlgo(si),
                          chunkSize,
                          hbuf);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    GWEN_Buffer_free(hbuf);
    return rv;
  }
  if ((int)GWEN_Buffer_GetUsedBytes(hbuf)!=chunkSize) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Bad padding (result!=%d bytes, it is %d)",
              chunkSize,
              GWEN_Buffer_GetUsedBytes(hbuf));
    GWEN_Buffer_free(hbuf);
    return GWEN_ERROR_INVALID;
  }

  /* sign padded hash */
  GWEN_Buffer_Rewind(hbuf);
  err=GWEN_CryptKey_Sign(key, hbuf, dst);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
    GWEN_Buffer_free(hbuf);
    return GWEN_Error_GetSimpleCode(err);
  }
  GWEN_Buffer_free(hbuf);

  /* TODO: Lock file */

  /* increment signature sequence counter */
  ui=GWEN_CryptTokenFile_Context_GetLocalSignSeq(fctx);
  ui++;
  GWEN_CryptTokenFile_Context_SetLocalSignSeq(fctx, ui);

  /* write file */
  rv=GWEN_CryptTokenFile__WriteFile(ct, 0);
  if (rv) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Error writing file (%d)", rv);
    return GWEN_ERROR_CT_IO_ERROR;
  }

  /* TODO: Unlock file */

  /* done */
  return 0;
}



int GWEN_CryptTokenFile_Verify(GWEN_CRYPTTOKEN *ct,
                               const GWEN_CRYPTTOKEN_CONTEXT *ctx,
                               const char *ptr,
                               unsigned int len,
                               const char *sigptr,
                               unsigned int siglen) {
  GWEN_CRYPTTOKEN_FILE *lct;
  GWEN_CT_FILE_CONTEXT *fctx;
  const GWEN_CRYPTTOKEN_KEYINFO *ki;
  const GWEN_CRYPTTOKEN_SIGNINFO *si;
  int rv;
  GWEN_BUFFER *hbuf;
  GWEN_BUFFER *sigBuf;
  GWEN_ERRORCODE err;
  GWEN_TYPE_UINT32 kid;
  GWEN_CRYPTKEY *key;
  int chunkSize;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_FILE, ct);
  assert(lct);

  /* reload if necessary */
  rv=GWEN_CryptTokenFile__ReloadIfNeeded(ct);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error reloading (%d)", rv);
    return rv;
  }

  /* get sign info */
  si=GWEN_CryptToken_Context_GetSignInfo(ctx);
  assert(si);

  /* get keyinfo and perform some checks */
  ki=GWEN_CryptToken_Context_GetVerifyKeyInfo(ctx);
  assert(ki);
  kid=GWEN_CryptToken_KeyInfo_GetKeyId(ki);
  if ((kid & 0xf)!=3) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid key id");
    return GWEN_ERROR_INVALID;
  }
  if (!(GWEN_CryptToken_KeyInfo_GetKeyFlags(ki) &
        GWEN_CRYPTTOKEN_KEYINFO_FLAGS_CAN_VERIFY)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Key can not be used for verification");
    return GWEN_ERROR_INVALID;
  }
  if (GWEN_CryptToken_KeyInfo_GetCryptAlgo(ki)!=
      GWEN_CryptToken_CryptAlgo_RSA) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid crypt algo");
    return GWEN_ERROR_INVALID;
  }

  /* get user context */
  fctx=GWEN_CryptTokenFile__GetFileContextByKeyId(ct, kid, 0, 0);
  if (!fctx) {
    DBG_ERROR(GWEN_LOGDOMAIN, "File context for key not found");
    return GWEN_ERROR_GENERIC;
  }

  /* check for existence of the key */
  key=GWEN_CryptTokenFile_Context_GetRemoteSignKey(fctx);
  if (key==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No key");
    return GWEN_ERROR_CT_NO_KEY;
  }

  chunkSize=GWEN_CryptKey_GetChunkSize(key);
  if (chunkSize!=GWEN_CryptToken_KeyInfo_GetChunkSize(ki)) {
    DBG_WARN(GWEN_LOGDOMAIN,
             "ChunkSize of key != ChunkSize in CryptToken (%d!=%d)",
             chunkSize, GWEN_CryptToken_KeyInfo_GetChunkSize(ki));
  }

  /* hash data */
  hbuf=GWEN_Buffer_new(0, chunkSize, 0, 1);
  rv=GWEN_CryptToken_Hash(GWEN_CryptToken_SignInfo_GetHashAlgo(si),
                          ptr, len,
                          hbuf);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    GWEN_Buffer_free(hbuf);
    return rv;
  }

  /* padd hash */
  GWEN_Buffer_Rewind(hbuf);
  rv=GWEN_CryptToken_Padd(GWEN_CryptToken_SignInfo_GetPaddAlgo(si),
                          chunkSize,
                          hbuf);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    GWEN_Buffer_free(hbuf);
    return rv;
  }
  if ((int)GWEN_Buffer_GetUsedBytes(hbuf)!=chunkSize) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Bad padding (result!=%d bytes, it is %d)",
              chunkSize,
              GWEN_Buffer_GetUsedBytes(hbuf));
    GWEN_Buffer_free(hbuf);
    return GWEN_ERROR_INVALID;
  }

  /* verify padded hash */
  GWEN_Buffer_Rewind(hbuf);
  sigBuf=GWEN_Buffer_new(0, siglen, 0, 1);
  GWEN_Buffer_AppendBytes(sigBuf, sigptr, siglen);
  err=GWEN_CryptKey_Verify(key, hbuf, sigBuf);
  GWEN_Buffer_free(sigBuf);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
    GWEN_Buffer_free(hbuf);
    return GWEN_Error_GetSimpleCode(err);
  }
  GWEN_Buffer_free(hbuf);

  /* done */
  return 0;
}



int GWEN_CryptTokenFile_Encrypt(GWEN_CRYPTTOKEN *ct,
                                const GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                const char *ptr,
                                unsigned int len,
                                GWEN_BUFFER *dst) {
  GWEN_CRYPTTOKEN_FILE *lct;
  GWEN_CT_FILE_CONTEXT *fctx;
  const GWEN_CRYPTTOKEN_KEYINFO *ki;
  const GWEN_CRYPTTOKEN_CRYPTINFO *ci;
  int rv;
  GWEN_BUFFER *hbuf;
  GWEN_ERRORCODE err;
  GWEN_TYPE_UINT32 kid;
  GWEN_CRYPTKEY *key;
  int chunkSize;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_FILE, ct);
  assert(lct);

  /* reload if necessary */
  rv=GWEN_CryptTokenFile__ReloadIfNeeded(ct);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error reloading (%d)", rv);
    return rv;
  }

  /* get crypt info */
  ci=GWEN_CryptToken_Context_GetCryptInfo(ctx);
  assert(ci);

  /* get keyinfo and perform some checks */
  ki=GWEN_CryptToken_Context_GetEncryptKeyInfo(ctx);
  assert(ki);
  kid=GWEN_CryptToken_KeyInfo_GetKeyId(ki);
  if ((kid & 0xff)!=4) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid key id");
    return GWEN_ERROR_INVALID;
  }
  if (!(GWEN_CryptToken_KeyInfo_GetKeyFlags(ki) &
        GWEN_CRYPTTOKEN_KEYINFO_FLAGS_CAN_ENCRYPT)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Key can not be used for encrypting");
    return GWEN_ERROR_INVALID;
  }
  if (GWEN_CryptToken_KeyInfo_GetCryptAlgo(ki)!=
      GWEN_CryptToken_CryptAlgo_RSA) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid crypt algo");
    return GWEN_ERROR_INVALID;
  }

  /* get user context */
  fctx=GWEN_CryptTokenFile__GetFileContextByKeyId(ct, kid, 0, 0);
  if (!fctx) {
    DBG_ERROR(GWEN_LOGDOMAIN, "File context for key not found");
    return GWEN_ERROR_GENERIC;
  }

  /* check for existence of the key */
  key=GWEN_CryptTokenFile_Context_GetRemoteCryptKey(fctx);
  if (key==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No key");
    return GWEN_ERROR_CT_NO_KEY;
  }

  chunkSize=GWEN_CryptKey_GetChunkSize(key);
  DBG_DEBUG(GWEN_LOGDOMAIN, "ChunkSize: %d", chunkSize);
  if (chunkSize!=GWEN_CryptToken_KeyInfo_GetChunkSize(ki)) {
    DBG_WARN(GWEN_LOGDOMAIN,
             "ChunkSize of key != ChunkSize in CryptToken (%d!=%d)",
             chunkSize, GWEN_CryptToken_KeyInfo_GetChunkSize(ki));
  }

  /* copy data */
  hbuf=GWEN_Buffer_new(0, chunkSize, 0, 1);
  GWEN_Buffer_AppendBytes(hbuf, ptr, len);

  /* padd source data */
  GWEN_Buffer_Rewind(hbuf);
  rv=GWEN_CryptToken_Padd(GWEN_CryptToken_CryptInfo_GetPaddAlgo(ci),
                          chunkSize,
                          hbuf);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    GWEN_Buffer_free(hbuf);
    return rv;
  }
  if ((int)GWEN_Buffer_GetUsedBytes(hbuf)!=chunkSize) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Bad padding (result!=%d bytes, it is %d)",
              chunkSize,
              GWEN_Buffer_GetUsedBytes(hbuf));
    GWEN_Buffer_free(hbuf);
    return GWEN_ERROR_INVALID;
  }

  /* encrypt padded data */
  GWEN_Buffer_Rewind(hbuf);
  DBG_DEBUG(GWEN_LOGDOMAIN, "Encrypting %d bytes",
            GWEN_Buffer_GetUsedBytes(hbuf));
  err=GWEN_CryptKey_Encrypt(key, hbuf, dst);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
    GWEN_Buffer_free(hbuf);
    return GWEN_ERROR_CT_IO_ERROR;
  }
  GWEN_Buffer_free(hbuf);

  /* done */
  return 0;
}



int GWEN_CryptTokenFile_Decrypt(GWEN_CRYPTTOKEN *ct,
                                const GWEN_CRYPTTOKEN_CONTEXT *ctx,
                                const char *ptr,
                                unsigned int len,
                                GWEN_BUFFER *dst) {
  GWEN_CRYPTTOKEN_FILE *lct;
  GWEN_CT_FILE_CONTEXT *fctx;
  const GWEN_CRYPTTOKEN_KEYINFO *ki;
  const GWEN_CRYPTTOKEN_CRYPTINFO *ci;
  int rv;
  GWEN_ERRORCODE err;
  GWEN_TYPE_UINT32 kid;
  GWEN_CRYPTKEY *key;
  GWEN_BUFFER *srcBuf;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_FILE, ct);
  assert(lct);

  /* reload if necessary */
  rv=GWEN_CryptTokenFile__ReloadIfNeeded(ct);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error reloading (%d)", rv);
    return rv;
  }

  /* get crypt info */
  ci=GWEN_CryptToken_Context_GetCryptInfo(ctx);
  assert(ci);

  /* get keyinfo and perform some checks */
  ki=GWEN_CryptToken_Context_GetDecryptKeyInfo(ctx);
  assert(ki);
  kid=GWEN_CryptToken_KeyInfo_GetKeyId(ki);
  if ((kid & 0xff)!=2) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid key id");
    return GWEN_ERROR_INVALID;
  }
  if (!(GWEN_CryptToken_KeyInfo_GetKeyFlags(ki) &
        GWEN_CRYPTTOKEN_KEYINFO_FLAGS_CAN_DECRYPT)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Key can not be used for decrypting");
    return GWEN_ERROR_INVALID;
  }
  if (GWEN_CryptToken_KeyInfo_GetCryptAlgo(ki)!=
      GWEN_CryptToken_CryptAlgo_RSA) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid crypt algo");
    return GWEN_ERROR_INVALID;
  }

  /* get user context */
  fctx=GWEN_CryptTokenFile__GetFileContextByKeyId(ct, kid, 0, 0);
  if (!fctx) {
    DBG_ERROR(GWEN_LOGDOMAIN, "File context for key not found");
    return GWEN_ERROR_GENERIC;
  }

  /* check for existence of the key */
  key=GWEN_CryptTokenFile_Context_GetLocalCryptKey(fctx);
  if (key==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No key");
    return GWEN_ERROR_CT_NO_KEY;
  }

  /* decrypt data */
  srcBuf=GWEN_Buffer_new(0, len, 0, 1);
  GWEN_Buffer_AppendBytes(srcBuf, ptr, len);
  err=GWEN_CryptKey_Decrypt(key, srcBuf, dst);
  GWEN_Buffer_free(srcBuf);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
    return GWEN_ERROR_CT_IO_ERROR;
  }

  /* unpadd decrypted data */
  GWEN_Buffer_Rewind(dst);
  rv=GWEN_CryptToken_Unpadd(GWEN_CryptToken_CryptInfo_GetPaddAlgo(ci),
                            dst);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    return rv;
  }

  /* done */
  return 0;
}












