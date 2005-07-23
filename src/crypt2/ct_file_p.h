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


#ifndef GWEN_CRYPTTOKEN_FILE_P_H
#define GWEN_CRYPTTOKEN_FILE_P_H

#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <gwenhywfar/fslock.h>
#include <gwenhywfar/ct_file.h>
#include "crypttoken_user_l.h"


struct GWEN_CT_FILE_CONTEXT {
  GWEN_INHERIT_ELEMENT(GWEN_CT_FILE_CONTEXT)
  GWEN_LIST_ELEMENT(GWEN_CT_FILE_CONTEXT)

  unsigned int localSignSeq;
  unsigned int remoteSignSeq;

  GWEN_CRYPTKEY *localSignKey;
  GWEN_CRYPTKEY *localCryptKey;

  GWEN_CRYPTKEY *remoteSignKey;
  GWEN_CRYPTKEY *remoteCryptKey;

  GWEN_CRYPTTOKEN_USER *user;
};


struct GWEN_CRYPTTOKEN_FILE {
  /* runtime data */
  GWEN_CT_FILE_CONTEXT_LIST *fileContextList;

  time_t ctime;
  time_t mtime;

  mode_t keyfile_mode;

  int fd;
  GWEN_FSLOCK *lock;

  GWEN_CRYPTTOKEN_FILE_READ_FN readFn;
  GWEN_CRYPTTOKEN_FILE_WRITE_FN writeFn;

};

void GWEN_CryptTokenFile_FreeData(void *bp, void *p);

int GWEN_CryptTokenFile__OpenFile(GWEN_CRYPTTOKEN *ct, int wr);
int GWEN_CryptTokenFile__CloseFile(GWEN_CRYPTTOKEN *ct);

int GWEN_CryptTokenFile__Read(GWEN_CRYPTTOKEN *ct);
int GWEN_CryptTokenFile__Write(GWEN_CRYPTTOKEN *ct);

int GWEN_CryptTokenFile__ReadFile(GWEN_CRYPTTOKEN *ct);
int GWEN_CryptTokenFile__WriteFile(GWEN_CRYPTTOKEN *ct);

int GWEN_CryptTokenFile__ReloadIfNeeded(GWEN_CRYPTTOKEN *ct);


int GWEN_CryptTokenFile_Create(GWEN_CRYPTTOKEN *ct);
int GWEN_CryptTokenFile_Open(GWEN_CRYPTTOKEN *ct, int managed);
int GWEN_CryptTokenFile_Close(GWEN_CRYPTTOKEN *ct);






#endif /* GWEN_CRYPTTOKEN_FILE_P_H */

