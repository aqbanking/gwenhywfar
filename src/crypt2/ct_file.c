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



int GWEN_CryptTokenFile__Write(GWEN_CRYPTTOKEN *ct){
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
  return lct->writeFn(ct, lct->fd);
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



int GWEN_CryptTokenFile__WriteFile(GWEN_CRYPTTOKEN *ct){
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

  /* read file */
  rv=GWEN_CryptTokenFile__Write(ct);
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

  if (fstat(lct->fd, &st)) {
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
      DBG_INFO(GWEN_LOGDOMAIN, "Error reloading keyfile");
      return rv;
    }
  }
  else {
    DBG_DEBUG(GWEN_LOGDOMAIN, "Keyfile unchanged, not reloading");
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

  rv=GWEN_CryptTokenFile__WriteFile(ct);
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

  rv=GWEN_CryptTokenFile__WriteFile(ct);

  /* free/reset all data */
  GWEN_CryptTokenFile_ClearFileContextList(ct);
  lct->mtime=0;
  lct->ctime=0;

  return rv;
}













