/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



#ifndef AH_MEDIUM_OHBCI_P_H
#define AH_MEDIUM_OHBCI_P_H


#define GWEN_CRYPT_TOKEN_OHBCI_NAME         "OHBCI"
#define GWEN_CRYPT_TOKEN_OHBCI_VMAJOR       1
#define GWEN_CRYPT_TOKEN_OHBCI_VMINOR       8

#define GWEN_CRYPT_TOKEN_OHBCI_PINMINLENGTH 4 /* temporary, should be 5 or more */

#define GWEN_CRYPT_TOKEN_OHBCI_MAX_PIN_TRY 10

#define GWEN_CRYPT_TOKEN_OHBCI_TRESOR_PWD_ITERATIONS   1469
#define GWEN_CRYPT_TOKEN_OHBCI_TRESOR_CRYPT_ITERATIONS  365


/* new in 1.6 */
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_MEDIUM1            (unsigned char)0xc1
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_MEDIUM2            (unsigned char)0xc2
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_MEDIUM3            (unsigned char)0xc3

#define GWEN_CRYPT_TOKEN_OHBCI_TAG_CRYPT_OLD          GWEN_CRYPT_TOKEN_OHBCI_TAG_MEDIUM1
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_CRYPT              GWEN_CRYPT_TOKEN_OHBCI_TAG_MEDIUM2
/* new in 1.6 */
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_CRYPT_BF           GWEN_CRYPT_TOKEN_OHBCI_TAG_MEDIUM3
/* new in 1.8 */
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_CRYPT_TRESOR       (unsigned char)0xc4

#define GWEN_CRYPT_TOKEN_OHBCI_TAG_VERSION_MAJOR      (unsigned char)0x02
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_VERSION_MINOR      (unsigned char)0x03
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_SEQ                (unsigned char)0x04

#define GWEN_CRYPT_TOKEN_OHBCI_TAG_USER_PUBSIGNKEY    (unsigned char)0xc5
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_USER_PRIVSIGNKEY   (unsigned char)0xc6
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_USER_PUBCRYPTKEY   (unsigned char)0xc7
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_USER_PRIVCRYPTKEY  (unsigned char)0xc8
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_USER_ID            (unsigned char)0x09

#define GWEN_CRYPT_TOKEN_OHBCI_TAG_INST_PUBSIGNKEY    (unsigned char)0xca
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_INST_PUBCRYPTKEY   (unsigned char)0xcb
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_INST_COUNTRY       (unsigned char)0x0c
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_INST_CODE          (unsigned char)0x0d
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_INST_SYSTEMID      (unsigned char)0x0e

/* temporary keys (reintroduced in version 1.8) */
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_TEMP_PUBSIGNKEY    (unsigned char)0xcf
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_TEMP_PRIVSIGNKEY   (unsigned char)0xd0
/* temporary keys (new in version 1.1, ignored in later versions) */
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_TEMP_PUBCRYPTKEY   (unsigned char)0xd1
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_TEMP_PRIVCRYPTKEY  (unsigned char)0xd2

/* new in version 1.4 */
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_SERVER_ADDR        (unsigned char)0xd3
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_SERVER_PORT        (unsigned char)0xd4
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_REMOTE_SEQ         (unsigned char)0xd5

/* new in version 1.6 */
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_HEADER             (unsigned char)0x16

/* new in version 1.7 */
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_USER_PRIVAUTHKEY   (unsigned char)0xd6
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_INST_PUBAUTHKEY    (unsigned char)0xd7


/* keydata */
/* ignored */
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_ISPUBLIC      (unsigned char) 0x01
/* ignored since 1.8 */
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_ISCRYPT       (unsigned char) 0x02
/* fixed in 1.5: fixes a bug in old OpenHBCI which included escape chars */
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_OWNER         (unsigned char) 0x03
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_VERSION       (unsigned char) 0x04
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_NUMBER        (unsigned char) 0x05
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_MODULUS       (unsigned char) 0x06
/* ignored */
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_EXP_OLD       (unsigned char) 0x07
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_N             (unsigned char) 0x08
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_P             (unsigned char) 0x09
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_Q             (unsigned char) 0x0a

/* ignored since 1.8 */
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_DMP1          (unsigned char) 0x0b
/* ignored since 1.8 */
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_DMQ1          (unsigned char) 0x0c
/* ignored since 1.8 */
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_IQMP          (unsigned char) 0x0d
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_D             (unsigned char) 0x0e
/* new in version 1.3 */
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_EXP           (unsigned char) 0x0f

/* new in version 1.7 */
#define GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_LEN           (unsigned char) 0x10



#include "ohbci_l.h"
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <gwenhywfar/fslock.h>
#include <gwenhywfar/cryptkey.h>
#include <gwenhywfar/ct_be.h>
#include <gwenhywfar/tag16.h>


GWENHYWFAR_EXPORT
GWEN_PLUGIN *ct_ohbci_factory(GWEN_PLUGIN_MANAGER *pm,
			      const char *modName,
			      const char *fileName);

GWEN_PLUGIN *GWEN_Crypt_TokenOHBCI_Plugin_new(GWEN_PLUGIN_MANAGER *pm,
					      const char *modName,
					      const char *fileName);
GWEN_CRYPT_TOKEN* GWENHYWFAR_CB 
  GWEN_Crypt_TokenOHBCI_Plugin_CreateToken(GWEN_PLUGIN *pl, const char *name);

int GWENHYWFAR_CB
  GWEN_Crypt_TokenOHBCI_Plugin_CheckToken(GWEN_PLUGIN *pl,
					  GWEN_BUFFER *name);


typedef struct GWEN_CRYPT_TOKEN_OHBCI GWEN_CRYPT_TOKEN_OHBCI;
struct GWEN_CRYPT_TOKEN_OHBCI {
  GWEN_CRYPT_TOKEN_OPEN_FN openFn;
  GWEN_CRYPT_TOKEN_CREATE_FN createFn;
  GWEN_CRYPT_TOKEN_CLOSE_FN closeFn;

  unsigned int mediumTag;
  unsigned int cryptoTag;
  unsigned int vminor;

  char password[64];
  int passWordIsSet;

  int justCreated;
};


void GWENHYWFAR_CB GWEN_Crypt_TokenOHBCI_FreeData(void *bp, void *p);


int GWEN_Crypt_TokenOHBCI__DecryptFile(GWEN_CRYPT_TOKEN *ct,
				       GWEN_BUFFER *fbuf,
				       int trynum,
				       uint32_t gid);
int GWEN_Crypt_TokenOHBCI__DecryptFile16(GWEN_CRYPT_TOKEN *ct,
					 GWEN_BUFFER *fbuf,
					 int trynum,
					 uint32_t gid);

int GWEN_Crypt_TokenOHBCI__DecryptTresor(GWEN_CRYPT_TOKEN *ct,
					 GWEN_BUFFER *fbuf,
					 int trynum,
					 uint32_t gid);

void GWEN_Crypt_TokenOHBCI__DecodeKey(GWEN_CRYPT_TOKEN *ct,
				      GWEN_TAG16 *keyTlv,
				      GWEN_DB_NODE *dbKeys,
				      const char *keyName);
int GWEN_Crypt_TokenOHBCI__Decode(GWEN_CRYPT_TOKEN *ct, GWEN_BUFFER *dbuf);



int GWEN_Crypt_TokenOHBCI__EncodeKey(const GWEN_CRYPT_KEY *key,
                                     GWEN_CRYPT_TOKEN_CONTEXT *fct,
				     unsigned int tagType,
				     int wantPublic,
				     int isCrypt,
				     GWEN_BUFFER *dbuf);
int GWEN_Crypt_TokenOHBCI_Encode(GWEN_CRYPT_TOKEN *ct, GWEN_BUFFER *dbuf);


int GWEN_Crypt_TokenOHBCI__EnsurePassword(GWEN_CRYPT_TOKEN *ct,
					  int trynum,
                                          int twice,
					  uint32_t gid);



int GWENHYWFAR_CB 
  GWEN_Crypt_TokenOHBCI_Create(GWEN_CRYPT_TOKEN *ct, uint32_t gid);
int GWENHYWFAR_CB 
  GWEN_Crypt_TokenOHBCI_Open(GWEN_CRYPT_TOKEN *ct, int manage, uint32_t gid);
int GWENHYWFAR_CB 
  GWEN_Crypt_TokenOHBCI_Close(GWEN_CRYPT_TOKEN *ct, int abandon, uint32_t gid);

int GWENHYWFAR_CB 
  GWEN_Crypt_TokenOHBCI_Write(GWEN_CRYPT_TOKEN *ct, int fd, int cre, uint32_t gid);
int GWENHYWFAR_CB 
  GWEN_Crypt_TokenOHBCI_Read(GWEN_CRYPT_TOKEN *ct, int fd, uint32_t gid);


int GWENHYWFAR_CB 
  GWEN_Crypt_TokenOHBCI_ChangePin(GWEN_CRYPT_TOKEN *ct,
				  int admin,
				  uint32_t gid);




#endif /* GWEN_CRYPT_TOKEN_OHBCI_P_H */



