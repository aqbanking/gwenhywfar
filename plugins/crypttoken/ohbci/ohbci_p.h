/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



#ifndef AH_MEDIUM_OHBCI_P_H
#define AH_MEDIUM_OHBCI_P_H


#define GWEN_CRYPTTOKEN_OHBCI_NAME         "OHBCI"
#define GWEN_CRYPTTOKEN_OHBCI_VMAJOR       1
#define GWEN_CRYPTTOKEN_OHBCI_VMINOR       5

#define GWEN_CRYPTTOKEN_OHBCI_KEYLEN       768
#define GWEN_CRYPTTOKEN_OHBCI_PINMINLENGTH 4 /* temporary, should be 5 or more */

#define GWEN_CRYPTTOKEN_OHBCI_MAX_PIN_TRY 10

#define GWEN_CRYPTTOKEN_OHBCI_TAG_CRYPT_OLD          (unsigned char)0xc1
#define GWEN_CRYPTTOKEN_OHBCI_TAG_CRYPT              (unsigned char)0xc2


#define GWEN_CRYPTTOKEN_OHBCI_TAG_VERSION_MAJOR      (unsigned char)0x02
#define GWEN_CRYPTTOKEN_OHBCI_TAG_VERSION_MINOR      (unsigned char)0x03
#define GWEN_CRYPTTOKEN_OHBCI_TAG_SEQ                (unsigned char)0x04

#define GWEN_CRYPTTOKEN_OHBCI_TAG_USER_PUBSIGNKEY    (unsigned char)0xc5
#define GWEN_CRYPTTOKEN_OHBCI_TAG_USER_PRIVSIGNKEY   (unsigned char)0xc6
#define GWEN_CRYPTTOKEN_OHBCI_TAG_USER_PUBCRYPTKEY   (unsigned char)0xc7
#define GWEN_CRYPTTOKEN_OHBCI_TAG_USER_PRIVCRYPTKEY  (unsigned char)0xc8
#define GWEN_CRYPTTOKEN_OHBCI_TAG_USER_ID            (unsigned char)0x09

#define GWEN_CRYPTTOKEN_OHBCI_TAG_INST_PUBSIGNKEY    (unsigned char)0xca
#define GWEN_CRYPTTOKEN_OHBCI_TAG_INST_PUBCRYPTKEY   (unsigned char)0xcb
#define GWEN_CRYPTTOKEN_OHBCI_TAG_INST_COUNTRY       (unsigned char)0x0c
#define GWEN_CRYPTTOKEN_OHBCI_TAG_INST_CODE          (unsigned char)0x0d
#define GWEN_CRYPTTOKEN_OHBCI_TAG_INST_SYSTEMID      (unsigned char)0x0e

/* temporary keys (new in version 1.1) */
#define GWEN_CRYPTTOKEN_OHBCI_TAG_TEMP_PUBSIGNKEY    (unsigned char)0xcf
#define GWEN_CRYPTTOKEN_OHBCI_TAG_TEMP_PRIVSIGNKEY   (unsigned char)0xd0
#define GWEN_CRYPTTOKEN_OHBCI_TAG_TEMP_PUBCRYPTKEY   (unsigned char)0xd1
#define GWEN_CRYPTTOKEN_OHBCI_TAG_TEMP_PRIVCRYPTKEY  (unsigned char)0xd2

/* new in version 1.4 */
#define GWEN_CRYPTTOKEN_OHBCI_TAG_SERVER_ADDR        (unsigned char)0xd3
#define GWEN_CRYPTTOKEN_OHBCI_TAG_SERVER_PORT        (unsigned char)0xd4
#define GWEN_CRYPTTOKEN_OHBCI_TAG_REMOTE_SEQ         (unsigned char)0xd5

/* keydata */
#define GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_ISPUBLIC      (unsigned char) 0x01
#define GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_ISCRYPT       (unsigned char) 0x02
/* fixed in 1.5: fixes a bug in old OpenHBCI which included escape chars */
#define GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_OWNER         (unsigned char) 0x03
#define GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_VERSION       (unsigned char) 0x04
#define GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_NUMBER        (unsigned char) 0x05
#define GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_MODULUS       (unsigned char) 0x06
#define GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_EXP_OLD       (unsigned char) 0x07
#define GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_N             (unsigned char) 0x08
#define GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_P             (unsigned char) 0x09
#define GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_Q             (unsigned char) 0x0a
#define GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_DMP1          (unsigned char) 0x0b
#define GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_DMQ1          (unsigned char) 0x0c
#define GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_IQMP          (unsigned char) 0x0d
#define GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_D             (unsigned char) 0x0e
/* new in version 1.3 */
#define GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_EXP           (unsigned char) 0x0f




#include "ohbci_l.h"
#include "tag16_l.h"
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <gwenhywfar/fslock.h>
#include <gwenhywfar/crypttoken.h>



GWEN_PLUGIN *GWEN_CryptTokenOHBCI_Plugin_new(GWEN_PLUGIN_MANAGER *pm,
                                             const char *modName,
                                             const char *fileName);
GWEN_CRYPTTOKEN *GWEN_CryptTokenOHBCI_Plugin_CreateToken(GWEN_PLUGIN *pl,
                                                         const char *subTypeName,
                                                         const char *name);
int GWEN_CryptTokenOHBCI_Plugin_CheckToken(GWEN_PLUGIN *pl,
                                           GWEN_BUFFER *subTypeName,
                                           GWEN_BUFFER *name);


typedef struct GWEN_CRYPTTOKEN_OHBCI GWEN_CRYPTTOKEN_OHBCI;
struct GWEN_CRYPTTOKEN_OHBCI {
  GWEN_CRYPTTOKEN_OPEN_FN openFn;
  GWEN_CRYPTTOKEN_CREATE_FN createFn;
  GWEN_CRYPTTOKEN_CLOSE_FN closeFn;

  unsigned int cryptoTag;

  char password[16];
  int passWordIsSet;

  int justCreated;
};


void GWEN_CryptTokenOHBCI_FreeData(void *bp, void *p);


int GWEN_CryptTokenOHBCI__DecryptFile(GWEN_CRYPTTOKEN *ct,
                                      GWEN_BUFFER *fbuf,
                                      int trynum);
void GWEN_CryptTokenOHBCI__DecodeKey(GWEN_CRYPTTOKEN *ct,
                                     GWEN_TAG16 *keyTlv,
                                     GWEN_DB_NODE *dbKeys,
                                     const char *keyName);
int GWEN_CryptTokenOHBCI__Decode(GWEN_CRYPTTOKEN *ct, GWEN_BUFFER *dbuf);


int GWEN_CryptTokenOHBCI__EncodeKey(const GWEN_CRYPTKEY *key,
                                    unsigned int tagType,
                                    int wantPublic,
                                    int isCrypt,
                                    GWEN_BUFFER *dbuf);
int GWEN_CryptTokenOHBCI_Encode(GWEN_CRYPTTOKEN *ct, GWEN_BUFFER *dbuf);




int GWEN_CryptTokenOHBCI_Create(GWEN_CRYPTTOKEN *ct);
int GWEN_CryptTokenOHBCI_Open(GWEN_CRYPTTOKEN *ct, int manage);
int GWEN_CryptTokenOHBCI_Close(GWEN_CRYPTTOKEN *ct);

int GWEN_CryptTokenOHBCI_Write(GWEN_CRYPTTOKEN *ct, int fd);
int GWEN_CryptTokenOHBCI_Read(GWEN_CRYPTTOKEN *ct, int fd);


int GWEN_CryptTokenOHBCI_ChangePin(GWEN_CRYPTTOKEN *ct);




#endif /* GWEN_CRYPTTOKEN_OHBCI_P_H */



