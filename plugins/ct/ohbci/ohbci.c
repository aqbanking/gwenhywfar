/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/*#define DEBUG_OHBCI_MODULE*/



#include "ohbci_p.h"
#include "i18n_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/padd.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/ctfile_be.h>
#include <gwenhywfar/ctplugin_be.h>
#include <gwenhywfar/ctf_context_be.h>
#include <gwenhywfar/text.h> /* DEBUG */
#include <gwenhywfar/cryptkeysym.h>
#include <gwenhywfar/cryptkeyrsa.h>
#include <gwenhywfar/smalltresor.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#ifdef OS_WIN32
# define ftruncate chsize
#endif


GWEN_INHERIT(GWEN_CRYPT_TOKEN, GWEN_CRYPT_TOKEN_OHBCI)


GWEN_PLUGIN *ct_ohbci_factory(GWEN_PLUGIN_MANAGER *pm,
			      const char *modName,
			      const char *fileName) {
  GWEN_PLUGIN *pl;

  pl=GWEN_Crypt_TokenOHBCI_Plugin_new(pm, modName, fileName);
  assert(pl);

  return pl;
}



GWEN_PLUGIN *GWEN_Crypt_TokenOHBCI_Plugin_new(GWEN_PLUGIN_MANAGER *pm,
					     const char *modName,
					     const char *fileName) {
  GWEN_PLUGIN *pl;

  pl=GWEN_Crypt_Token_Plugin_new(pm,
				 GWEN_Crypt_Token_Device_File,
				 modName,
				 fileName);

  /* set virtual functions */
  GWEN_Crypt_Token_Plugin_SetCreateTokenFn(pl,
					   GWEN_Crypt_TokenOHBCI_Plugin_CreateToken);
  GWEN_Crypt_Token_Plugin_SetCheckTokenFn(pl,
					  GWEN_Crypt_TokenOHBCI_Plugin_CheckToken);

  return pl;
}



GWEN_CRYPT_TOKEN* GWENHYWFAR_CB
GWEN_Crypt_TokenOHBCI_Plugin_CreateToken(GWEN_PLUGIN *pl,
					 const char *name) {
  GWEN_PLUGIN_MANAGER *pm;
  GWEN_CRYPT_TOKEN *ct;

  assert(pl);

  pm=GWEN_Plugin_GetManager(pl);
  assert(pm);

  ct=GWEN_Crypt_TokenOHBCI_new(pm, name);
  assert(ct);

  return ct;
}



int GWENHYWFAR_CB 
GWEN_Crypt_TokenOHBCI_Plugin_CheckToken(GWEN_PLUGIN *pl,
					GWEN_BUFFER *name) {
  FILE *f;
  const char *p;
  char buffer[16];
  int rv;

  if (GWEN_Buffer_GetUsedBytes(name)==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Empty name");
    return GWEN_ERROR_BAD_NAME;
  }

  p=GWEN_Buffer_GetStart(name);
  if (access(p, F_OK)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "File [%s] does not exist", p);
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Info, "File does not exist");
    return GWEN_ERROR_BAD_NAME;
  }

  if (access(p, R_OK | W_OK)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "File exists but I have no writes on it");
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Info,
			 "File exists but I have no writes on it");
    return GWEN_ERROR_IO;
  }

  f=fopen(p, "rb");
  if (!f) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "File exists, I have all rights but still can't open it");
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Notice,
			 "File exists, I have all rights but "
			 "still can't open it");
    return GWEN_ERROR_IO;
  }

  rv=fread(buffer, sizeof(buffer), 1, f);
  fclose(f);
  if (rv!=1) {
    DBG_INFO(GWEN_LOGDOMAIN, "This seems not to be an OpenHBCI keyfile");
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Info,
			 "This seems not to be an OpenHBCI keyfile");
    return GWEN_ERROR_NOT_SUPPORTED;
  }

  if (rv!=1) {
    DBG_INFO(GWEN_LOGDOMAIN, "This seems not to be an OpenHBCI keyfile (bad size)");
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Notice,
			 "This seems not to be an OpenHBCI keyfile "
			 "(bad size)");
    return GWEN_ERROR_NOT_SUPPORTED;
  }

  if ((unsigned char)(buffer[0])==GWEN_CRYPT_TOKEN_OHBCI_TAG_MEDIUM1) {
    DBG_INFO(GWEN_LOGDOMAIN,
             "Old OpenHBCI file detected");
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Notice,
			 "Old OpenHBCI file detected");
    return 0;
  }
  else if ((unsigned char)(buffer[0])==GWEN_CRYPT_TOKEN_OHBCI_TAG_MEDIUM2) {
    DBG_INFO(GWEN_LOGDOMAIN,
             "OpenHBCI file (<1.6) detected");
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Notice,
			 "OpenHBCI file (<1.6) detected");
    return 0;
  }
  else if ((unsigned char)(buffer[0])==GWEN_CRYPT_TOKEN_OHBCI_TAG_MEDIUM3) {
    if ((unsigned char)(buffer[3])==GWEN_CRYPT_TOKEN_OHBCI_TAG_HEADER &&
        strncmp(buffer+6,
                GWEN_CRYPT_TOKEN_OHBCI_NAME,
                strlen(GWEN_CRYPT_TOKEN_OHBCI_NAME))==0) {
      DBG_INFO(GWEN_LOGDOMAIN,
               "New OpenHBCI file (>=1.6) detected");
      GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Notice,
			   "New OpenHBCI file (>=1.6) detected");
      return 0;
    }
  }

  DBG_INFO(GWEN_LOGDOMAIN,
           "This seems not to be an OpenHBCI keyfile");
  GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Notice,
		       "This seems not to be an OpenHBCI keyfile");
  return GWEN_ERROR_NOT_SUPPORTED;
}





GWEN_CRYPT_TOKEN *GWEN_Crypt_TokenOHBCI_new(GWEN_PLUGIN_MANAGER *pm,
					    const char *name){
  GWEN_CRYPT_TOKEN *ct;
  GWEN_CRYPT_TOKEN_OHBCI *lct;

  ct=GWEN_Crypt_TokenFile_new("ohbci", name);

  GWEN_NEW_OBJECT(GWEN_CRYPT_TOKEN_OHBCI, lct);
  GWEN_INHERIT_SETDATA(GWEN_CRYPT_TOKEN, GWEN_CRYPT_TOKEN_OHBCI,
                       ct, lct,
                       GWEN_Crypt_TokenOHBCI_FreeData);
  lct->mediumTag=GWEN_CRYPT_TOKEN_OHBCI_TAG_MEDIUM3;
  lct->vminor=GWEN_CRYPT_TOKEN_OHBCI_VMINOR;
  lct->cryptoTag=GWEN_CRYPT_TOKEN_OHBCI_TAG_CRYPT_BF;

  /* set virtual functions */
  lct->openFn=GWEN_Crypt_Token_SetOpenFn(ct, GWEN_Crypt_TokenOHBCI_Open);
  lct->closeFn=GWEN_Crypt_Token_SetCloseFn(ct, GWEN_Crypt_TokenOHBCI_Close);
  lct->createFn=GWEN_Crypt_Token_SetCreateFn(ct, GWEN_Crypt_TokenOHBCI_Create);
  GWEN_Crypt_Token_SetChangePinFn(ct, GWEN_Crypt_TokenOHBCI_ChangePin);

  GWEN_Crypt_TokenFile_SetReadFn(ct, GWEN_Crypt_TokenOHBCI_Read);
  GWEN_Crypt_TokenFile_SetWriteFn(ct, GWEN_Crypt_TokenOHBCI_Write);

  return ct;
}



void GWENHYWFAR_CB GWEN_Crypt_TokenOHBCI_FreeData(void *bp, void *p) {
  GWEN_CRYPT_TOKEN_OHBCI *lct;

  lct=(GWEN_CRYPT_TOKEN_OHBCI*) p;
  memset(lct->password, 0, sizeof(lct->password));
  GWEN_FREE_OBJECT(lct);
}




int GWEN_Crypt_TokenOHBCI__EnsurePassword(GWEN_CRYPT_TOKEN *ct,
					  int trynum,
					  int confirm,
					  uint32_t gid){
  GWEN_CRYPT_TOKEN_OHBCI *lct;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, GWEN_CRYPT_TOKEN_OHBCI, ct);
  assert(lct);

  if (lct->passWordIsSet==0) {
    char password[64];
    int rv;
    unsigned int pinLength=0;
    uint32_t flags;

    /* create key from password */
    memset(lct->password, 0, sizeof(lct->password));

    flags=0;
    if (trynum)
      flags|=GWEN_GUI_INPUT_FLAGS_RETRY;
    if (confirm)
      flags|=GWEN_GUI_INPUT_FLAGS_CONFIRM;
    rv=GWEN_Crypt_Token_GetPin(ct,
			       GWEN_Crypt_PinType_Access,
			       GWEN_Crypt_PinEncoding_Ascii,
                               flags,
			       (unsigned char*)password,
			       GWEN_CRYPT_TOKEN_OHBCI_PINMINLENGTH,
			       sizeof(password)-1,
			       &pinLength,
			       gid);
    if (rv) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error asking for PIN, aborting (%d)", rv);
      return rv;
    }

    if (strlen(password)<GWEN_CRYPT_TOKEN_OHBCI_PINMINLENGTH) {
      DBG_ERROR(GWEN_LOGDOMAIN,
		"Your program returned a shorter PIN than instructed!");
      return GWEN_ERROR_GENERIC;
    }

    DBG_NOTICE(GWEN_LOGDOMAIN, "Checking...");
    if (lct->cryptoTag==GWEN_CRYPT_TOKEN_OHBCI_TAG_CRYPT) {
      DBG_NOTICE(GWEN_LOGDOMAIN, "New OpenHBCI file detected");

      rv=GWEN_Crypt_KeyDataFromText(password,
				    (unsigned char*)lct->password,
				    24);
      if (rv) {
	DBG_ERROR(GWEN_LOGDOMAIN,
		  "Could not create key data from password (%d)", rv);
	return GWEN_ERROR_GENERIC;
      }
    }
    else if (lct->cryptoTag==GWEN_CRYPT_TOKEN_OHBCI_TAG_CRYPT_OLD) {
      DBG_NOTICE(GWEN_LOGDOMAIN, "Old OpenHBCI file detected");

      rv=GWEN_Gui_KeyDataFromText_OpenSSL(password,
					  (unsigned char*)lct->password,
					  16);
      if (rv) {
	if (rv==GWEN_ERROR_NOT_IMPLEMENTED) {
	  DBG_ERROR(GWEN_LOGDOMAIN,
		    "OpenSSL-style password creation not supported with Libgcrypt!");
	}
	else {
	  DBG_ERROR(GWEN_LOGDOMAIN, "Could not create key data from password (%d)", rv);
	}
	return GWEN_ERROR_GENERIC;
      }
    }
    else if (lct->cryptoTag==GWEN_CRYPT_TOKEN_OHBCI_TAG_CRYPT_BF) {
      DBG_NOTICE(GWEN_LOGDOMAIN, "New OpenHBCI (1.6+) file detected");
      /* same as above but for different key type */
      rv=GWEN_Crypt_KeyDataFromText(password,
				    (unsigned char*)lct->password,
				    16);
      if (rv) {
	DBG_ERROR(GWEN_LOGDOMAIN,
		  "Could not create key data from password (%d)", rv);
	return GWEN_ERROR_GENERIC;
      }
    }
    else if (lct->cryptoTag==GWEN_CRYPT_TOKEN_OHBCI_TAG_CRYPT_TRESOR) {
      DBG_NOTICE(GWEN_LOGDOMAIN, "New OpenHBCI (1.8+) file detected");
      strncpy(lct->password, password, sizeof(lct->password)-1);
      lct->password[sizeof(lct->password)-1]=0;
    }
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected crypto tag %d",
		lct->cryptoTag);
      abort();
    }

    lct->passWordIsSet=1;
  }

  return 0;
}



int GWEN_Crypt_TokenOHBCI__DecryptFile(GWEN_CRYPT_TOKEN *ct,
				       GWEN_BUFFER *fbuf,
				       int trynum,
				       uint32_t gid){
  GWEN_CRYPT_TOKEN_OHBCI *lct;
  GWEN_CRYPT_KEY *key;
  int err;
  char password[64];
  GWEN_BUFFER *rawbuf;
  uint32_t size;
  int rv;
  unsigned int pinLength=0;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, GWEN_CRYPT_TOKEN_OHBCI, ct);
  assert(lct);

  rv=GWEN_Crypt_TokenOHBCI__EnsurePassword(ct, trynum, 0, gid);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  if (lct->cryptoTag==GWEN_CRYPT_TOKEN_OHBCI_TAG_CRYPT) {
    key=GWEN_Crypt_KeyDes3K_fromData(GWEN_Crypt_CryptMode_Cbc, 24,
				     (const uint8_t*)lct->password, 24);
  }
  else if (lct->cryptoTag==GWEN_CRYPT_TOKEN_OHBCI_TAG_CRYPT_OLD) {
    key=GWEN_Crypt_KeyDes3K_fromData(GWEN_Crypt_CryptMode_Cbc, 16,
				     (const uint8_t*)lct->password, 16);
  }
  else if (lct->cryptoTag==GWEN_CRYPT_TOKEN_OHBCI_TAG_CRYPT_BF) {
    key=GWEN_Crypt_KeyBlowFish_fromData(GWEN_Crypt_CryptMode_Cbc, 16,
					(const uint8_t*)lct->password, 16);
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Unknown crypt tag, should not occur");
    abort();
  }

  /* decrypt file */
  DBG_INFO(GWEN_LOGDOMAIN, "Decrypting file");
  size=GWEN_Buffer_GetUsedBytes(fbuf);
  rawbuf=GWEN_Buffer_new(0, size, 0, 1);
  GWEN_Buffer_Rewind(fbuf);
  err=GWEN_Crypt_Key_Decipher(key,
			      (const uint8_t*)GWEN_Buffer_GetStart(fbuf),
			      size,
			      (uint8_t*)GWEN_Buffer_GetStart(rawbuf),
			      &size);
  if (err) {
    DBG_INFO_ERR(GWEN_LOGDOMAIN, err);
    GWEN_Buffer_free(rawbuf);
    GWEN_Crypt_Key_free(key);
    if (pinLength)
      GWEN_Crypt_Token_SetPinStatus(ct,
				    GWEN_Crypt_PinType_Access,
				    GWEN_Crypt_PinEncoding_Ascii,
				    (trynum
				     ?GWEN_GUI_INPUT_FLAGS_RETRY:0),
				    (unsigned char*)password,
				    pinLength,
				    0,
				    gid);
    memset(password, 0, sizeof(password));
    lct->passWordIsSet=0;
    return GWEN_ERROR_BAD_PIN;
  }

  /* advance buffer pointers since we wrote directly to the buffer */
  GWEN_Buffer_IncrementPos(rawbuf, size);
  GWEN_Buffer_AdjustUsedBytes(rawbuf);

  /* unpadd raw data */
  DBG_INFO(GWEN_LOGDOMAIN, "Unpadding file");
  if (GWEN_Padd_UnpaddWithAnsiX9_23(rawbuf)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not unpadd keyfile, i.e. wrong PIN");
    GWEN_Buffer_free(rawbuf);
    GWEN_Crypt_Key_free(key);
    /* TODO: Set Pin status */
    lct->passWordIsSet=0;
    if (pinLength)
      GWEN_Crypt_Token_SetPinStatus(ct,
				    GWEN_Crypt_PinType_Access,
				    GWEN_Crypt_PinEncoding_Ascii,
				    (trynum?GWEN_GUI_INPUT_FLAGS_RETRY:0),
				    (unsigned char*)password,
				    pinLength,
				    0,
				    gid);
    memset(password, 0, sizeof(password));
    return GWEN_ERROR_BAD_PIN;
  }
  GWEN_Crypt_Key_free(key);

  /* parse raw data */
  DBG_INFO(GWEN_LOGDOMAIN, "Parsing file");
  GWEN_Buffer_Rewind(rawbuf);

  rv=GWEN_Crypt_TokenOHBCI__Decode(ct, rawbuf);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(rawbuf);
    /* Set Pin status */
    if (pinLength)
      GWEN_Crypt_Token_SetPinStatus(ct,
				    GWEN_Crypt_PinType_Access,
				    GWEN_Crypt_PinEncoding_Ascii,
				    (trynum
				     ?GWEN_GUI_INPUT_FLAGS_RETRY:0),
				    (unsigned char*)password,
				    pinLength,
				    0,
				    gid);
    memset(password, 0, sizeof(password));
    lct->passWordIsSet=0;
    return GWEN_ERROR_BAD_PIN;
  }
  GWEN_Buffer_free(rawbuf);
  /* Set Pin status */
  if (pinLength)
    GWEN_Crypt_Token_SetPinStatus(ct,
				  GWEN_Crypt_PinType_Access,
				  GWEN_Crypt_PinEncoding_Ascii,
				  (trynum?GWEN_GUI_INPUT_FLAGS_RETRY:0),
				  (unsigned char*)password,
				  pinLength,
				  1,
				  gid);
  memset(password, 0, sizeof(password));
  return 0;
}



int GWEN_Crypt_TokenOHBCI__DecryptTresor(GWEN_CRYPT_TOKEN *ct,
					 GWEN_BUFFER *fbuf,
					 int trynum,
					 uint32_t gid){
  GWEN_CRYPT_TOKEN_OHBCI *lct;
  GWEN_BUFFER *rawbuf;
  uint32_t size;
  int rv;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, GWEN_CRYPT_TOKEN_OHBCI, ct);
  assert(lct);

  rv=GWEN_Crypt_TokenOHBCI__EnsurePassword(ct, trynum, 0, gid);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  size=GWEN_Buffer_GetUsedBytes(fbuf);
  rawbuf=GWEN_Buffer_new(0, size, 0, 1);
  GWEN_Buffer_Rewind(fbuf);
  rv=GWEN_SmallTresor_Decrypt((const uint8_t*) GWEN_Buffer_GetStart(fbuf), size,
			      lct->password,
			      rawbuf,
			      GWEN_CRYPT_TOKEN_OHBCI_TRESOR_PWD_ITERATIONS,
			      GWEN_CRYPT_TOKEN_OHBCI_TRESOR_CRYPT_ITERATIONS);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(rawbuf);
    if (lct->password[0])
      GWEN_Crypt_Token_SetPinStatus(ct,
				    GWEN_Crypt_PinType_Access,
				    GWEN_Crypt_PinEncoding_Ascii,
				    (trynum?GWEN_GUI_INPUT_FLAGS_RETRY:0),
				    (unsigned char*)lct->password,
                                    strlen(lct->password),
				    0,
				    gid);
    memset(lct->password, 0, sizeof(lct->password));
    lct->passWordIsSet=0;
    return GWEN_ERROR_BAD_PIN;
  }

  /* parse raw data */
  DBG_INFO(GWEN_LOGDOMAIN, "Parsing file");
  GWEN_Buffer_Rewind(rawbuf);

  rv=GWEN_Crypt_TokenOHBCI__Decode(ct, rawbuf);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(rawbuf);
    /* Set Pin status */
    if (lct->password[0])
      GWEN_Crypt_Token_SetPinStatus(ct,
				    GWEN_Crypt_PinType_Access,
				    GWEN_Crypt_PinEncoding_Ascii,
				    (trynum?GWEN_GUI_INPUT_FLAGS_RETRY:0),
				    (unsigned char*)lct->password,
				    strlen(lct->password),
				    0,
				    gid);
    memset(lct->password, 0, sizeof(lct->password));
    lct->passWordIsSet=0;
    return GWEN_ERROR_BAD_PIN;
  }
  GWEN_Buffer_free(rawbuf);
  /* Set Pin status */
  if (lct->password[0])
    GWEN_Crypt_Token_SetPinStatus(ct,
				  GWEN_Crypt_PinType_Access,
				  GWEN_Crypt_PinEncoding_Ascii,
				  (trynum?GWEN_GUI_INPUT_FLAGS_RETRY:0),
				  (unsigned char*)lct->password,
				  strlen(lct->password),
				  1,
				  gid);
  return 0;
}



int GWENHYWFAR_CB 
GWEN_Crypt_TokenOHBCI_Read(GWEN_CRYPT_TOKEN *ct, int fd, uint32_t gid){
  GWEN_CRYPT_TOKEN_OHBCI *lct;
  GWEN_BUFFER *rbuf;
  GWEN_BUFFER *fbuf;
  unsigned char c;
  GWEN_TAG16 *tlv;
  int i;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, GWEN_CRYPT_TOKEN_OHBCI, ct);
  assert(lct);

  rbuf=GWEN_Buffer_new(0, 1024, 0, 1);
  /* read file into rbuf */
  while(1) {
    char buffer[256];
    int rv;

    rv=read(fd, buffer, sizeof(buffer));
    if (rv==-1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "read: %s", strerror(errno));
      return -1;
    }
    if (rv==0)
      break;
    GWEN_Buffer_AppendBytes(rbuf, buffer, rv);
  }

  if (GWEN_Buffer_GetUsedBytes(rbuf)<3) {
    DBG_ERROR(GWEN_LOGDOMAIN, "This seems not to be an OpenHBCI key file");
    GWEN_Buffer_free(rbuf);
    return -1;
  }

  /* check whether this is a known OpenHBCI(2) keyfile */
  GWEN_Buffer_Rewind(rbuf);
  c=*GWEN_Buffer_GetStart(rbuf);
  if (c!=GWEN_CRYPT_TOKEN_OHBCI_TAG_MEDIUM1 &&
      c!=GWEN_CRYPT_TOKEN_OHBCI_TAG_MEDIUM2 &&
      c!=GWEN_CRYPT_TOKEN_OHBCI_TAG_MEDIUM3) {
    DBG_ERROR(GWEN_LOGDOMAIN, "This seems not to be an OpenHBCI key file");
    GWEN_Buffer_free(rbuf);
    return -1;
  }
  lct->mediumTag=c;
  lct->cryptoTag=0;

  tlv=GWEN_Tag16_fromBuffer(rbuf, 0);
  if (!tlv) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Bad file data");
    GWEN_Buffer_free(rbuf);
    return -1;
  }

  fbuf=GWEN_Buffer_new(0, GWEN_Tag16_GetTagLength(tlv), 0, 1);
  GWEN_Buffer_AppendBytes(fbuf,
                          GWEN_Tag16_GetTagData(tlv),
                          GWEN_Tag16_GetTagLength(tlv));
  GWEN_Buffer_Rewind(fbuf);
  GWEN_Buffer_free(rbuf);
  GWEN_Tag16_free(tlv);
  /* now fbuf contains the data from the crypt TLV */


  for (i=0;;i++) {
    int rv;

    if (i>GWEN_CRYPT_TOKEN_OHBCI_MAX_PIN_TRY) {
      DBG_ERROR(GWEN_LOGDOMAIN,
		"No valid PIN within %d tries, giving up", i);
      GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Critical,
			   I18N("No valid PIN (tried too often), "
				"aborting."));
      GWEN_Buffer_free(fbuf);
      return GWEN_ERROR_ABORTED;
    }

    switch(lct->mediumTag) {
    case GWEN_CRYPT_TOKEN_OHBCI_TAG_MEDIUM1:
      lct->cryptoTag=GWEN_CRYPT_TOKEN_OHBCI_TAG_CRYPT_OLD;
      rv=GWEN_Crypt_TokenOHBCI__DecryptFile(ct, fbuf, i, gid);
      break;
    case GWEN_CRYPT_TOKEN_OHBCI_TAG_MEDIUM2:
      lct->cryptoTag=GWEN_CRYPT_TOKEN_OHBCI_TAG_CRYPT;
      rv=GWEN_Crypt_TokenOHBCI__DecryptFile(ct, fbuf, i, gid);
      break;
    case GWEN_CRYPT_TOKEN_OHBCI_TAG_MEDIUM3:
      lct->cryptoTag=0;
      rv=GWEN_Crypt_TokenOHBCI__DecryptFile16(ct, fbuf, i, gid);
      break;
    default:
      rv = GWEN_ERROR_NOT_SUPPORTED;
      /* otherwise 'rv' might be uninitialized */
    }
    if (rv==0)
      break;
    else {
      switch(rv) {
      case GWEN_ERROR_USER_ABORTED:
        DBG_INFO(GWEN_LOGDOMAIN, "Aborted by user");
        GWEN_Buffer_free(fbuf);
        return rv;
      case GWEN_ERROR_BAD_PIN:
        DBG_ERROR(GWEN_LOGDOMAIN, "Bad pin.");
	GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Critical,
			     I18N("Bad PIN, will try again"));
	break;

      default:
	DBG_INFO(GWEN_LOGDOMAIN, "Other error, giving up");
	GWEN_Buffer_free(fbuf);
	return rv;
      }
    }

  } /* for */
  GWEN_Buffer_free(fbuf);

  lct->justCreated=0;

  return 0;
}



void GWEN_Crypt_TokenOHBCI__DecodeKey(GWEN_CRYPT_TOKEN *ct,
				      GWEN_TAG16 *keyTlv,
				      GWEN_DB_NODE *dbKeys,
				      const char *keyName) {
  GWEN_BUFFER *dbuf;
  const char *p;
  int size;
  GWEN_DB_NODE *node;
  uint32_t flags;
  const char defaultExpo[3]={0x01, 0x00, 0x01};

  p=GWEN_Tag16_GetTagData(keyTlv);
  size=GWEN_Tag16_GetTagLength(keyTlv);
  if (size<2) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Tag too small to contain any subtag");
    return;
  }
  /* create static buffer */
  dbuf=GWEN_Buffer_new((char*)p, size, size, 0);
  GWEN_Buffer_SubMode(dbuf, GWEN_BUFFER_MODE_DYNAMIC);

  node=GWEN_DB_GetGroup(dbKeys, GWEN_DB_FLAGS_DEFAULT, keyName);
  assert(node);

  /* preset */
  GWEN_DB_SetCharValue(node,
                       GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "cryptAlgoId",
		       "rsa");
  GWEN_DB_SetIntValue(node,
		      GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "keySize",
                      96);
  GWEN_DB_SetBinValue(node,
                      GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "rsa/e",
		      defaultExpo,
		      sizeof(defaultExpo));
  flags=0;
  GWEN_DB_SetIntValue(node,
                      GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "flags",
                      flags);

  while(GWEN_Buffer_GetBytesLeft(dbuf)) {
    const char *pp;
    char *p;
    GWEN_TAG16 *tlv;
    unsigned int l;

    tlv=GWEN_Tag16_fromBuffer(dbuf, 0);
    if (!tlv) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Bad file (no TLV)");
      return;
    }
    p=0;
    pp=(const char*)GWEN_Tag16_GetTagData(tlv);
    l=GWEN_Tag16_GetTagLength(tlv);
    if (pp && l) {
      p=(char*)malloc(l+1);
      assert(p);
      memmove(p, pp, l);
      p[l]=0;
    }

    switch(GWEN_Tag16_GetTagType(tlv)) {
    case GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_ISPUBLIC:
      /* ignore this tag, since it is buggy in OpenHBCI(2) */
      break;

    case GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_ISCRYPT:
      /* no longer supported since 1.8*/
      break;

    case GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_OWNER:
      GWEN_DB_SetCharValue(node,
			   GWEN_DB_FLAGS_OVERWRITE_VARS,
			   "keyOwner",
			   p);
      break;

    case GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_VERSION:
      assert(p);
      GWEN_DB_SetIntValue(node,
                          GWEN_DB_FLAGS_OVERWRITE_VARS,
			  "keyVersion",
                          atoi(p));
      break;

    case GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_NUMBER:
      assert(p);
      GWEN_DB_SetIntValue(node,
                          GWEN_DB_FLAGS_OVERWRITE_VARS,
			  "keyNumber",
                          atoi(p));
      break;

    case GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_MODULUS:
      if (p && l)
        GWEN_DB_SetBinValue(node,
                            GWEN_DB_FLAGS_OVERWRITE_VARS,
			    "rsa/n",
                            p, l);
      break;

    case GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_EXP_OLD:
      DBG_INFO(GWEN_LOGDOMAIN,
               "Ignoring old exponent (%d), keeping default", l);
      break;

    case GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_EXP:
      if (p && l)
        GWEN_DB_SetBinValue(node,
                            GWEN_DB_FLAGS_OVERWRITE_VARS,
			    "rsa/e",
                            p, l);
      break;

    case GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_N:
      if (p && l)
        GWEN_DB_SetBinValue(node,
                            GWEN_DB_FLAGS_OVERWRITE_VARS,
                            "rsa/n",
                            p, l);
      break;

    case GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_P:
      if (p && l)
	GWEN_DB_SetBinValue(node,
			    GWEN_DB_FLAGS_OVERWRITE_VARS,
			    "rsa/p",
			    p, l);
      break;

    case GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_Q:
      if (p && l)
	GWEN_DB_SetBinValue(node,
			    GWEN_DB_FLAGS_OVERWRITE_VARS,
			    "rsa/q",
			    p, l);
      break;

    case GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_D:
      if (p && l)
        GWEN_DB_SetBinValue(node,
                            GWEN_DB_FLAGS_OVERWRITE_VARS,
                            "rsa/d",
                            p, l);
      break;

    case GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_LEN:
      if (p && l) {
	int i;

        /* fix for some versions which stored keysize*8 */
	i=atoi(p);
	if (i>512)
	  i/=8;
	GWEN_DB_SetIntValue(node,
			    GWEN_DB_FLAGS_OVERWRITE_VARS,
			    "keySize",
			    i);
      }
      break;

    default:
      DBG_WARN(GWEN_LOGDOMAIN, "Unknown tag %02x", GWEN_Tag16_GetTagType(tlv));
      break;
    } /* switch */

    GWEN_Tag16_free(tlv);
    free(p);
  } /* while */
  GWEN_Buffer_free(dbuf);
}



int GWEN_Crypt_TokenOHBCI__Decode(GWEN_CRYPT_TOKEN *ct, GWEN_BUFFER *dbuf) {
  GWEN_CRYPT_TOKEN_OHBCI *lct;
  GWEN_TAG16 *tlv;
  int rv;
  GWEN_DB_NODE *dbKeys;
  GWEN_DB_NODE *dbKey;
  GWEN_CRYPT_TOKEN_CONTEXT *fct;
  const char *peerId=0;
  uint32_t localSignSeq=0;
  uint32_t remoteSignSeq=0;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, GWEN_CRYPT_TOKEN_OHBCI, ct);
  assert(lct);

  tlv=GWEN_Tag16_fromBuffer(dbuf, 0);
  GWEN_Buffer_Rewind(dbuf);
  if (!tlv) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "File doesn't contain a TLV: Either bad pin or bad file");
    return -1;
  }

  if (GWEN_Tag16_GetTagType(tlv)!=GWEN_CRYPT_TOKEN_OHBCI_TAG_HEADER &&
      GWEN_Tag16_GetTagType(tlv)!=GWEN_CRYPT_TOKEN_OHBCI_TAG_VERSION_MAJOR) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "File doesn't start with version info or header.");
    GWEN_Tag16_free(tlv);
    return -1;
  }
  GWEN_Tag16_free(tlv);

  fct=GWEN_CTF_Context_new();

  /* now parse it */
  dbKeys=GWEN_DB_Group_new("keys");
  while(GWEN_Buffer_GetBytesLeft(dbuf)) {
    int i;
    const char *pp;
    char *p;
    unsigned int l;

    tlv=GWEN_Tag16_fromBuffer(dbuf, 0);
    if (!tlv) {
      DBG_ERROR(GWEN_LOGDOMAIN,
		"File doesn't contain a TLV: Either bad pin or bad file");
      GWEN_Crypt_Token_Context_free(fct);
      return GWEN_ERROR_BAD_PIN;
    }
    p=0;
    pp=(const char*)GWEN_Tag16_GetTagData(tlv);
    l=GWEN_Tag16_GetTagLength(tlv);
    if (pp && l) {
      p=(char*)malloc(l+1);
      assert(p);
      memmove(p, pp, l);
      p[l]=0;
    }

    switch(GWEN_Tag16_GetTagType(tlv)) {
    case GWEN_CRYPT_TOKEN_OHBCI_TAG_VERSION_MAJOR:
      assert(p);
      i=atoi(p);
      DBG_INFO(GWEN_LOGDOMAIN, "OHBCI: Major version: %d", i);
      if (i!=GWEN_CRYPT_TOKEN_OHBCI_VMAJOR) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Unsupported keyfile version (%d)", i);
	GWEN_Gui_ProgressLog(0,
                             GWEN_LoggerLevel_Warning,
			     "Basically this file type is supported.\n"
			     "However, the major versions do not match,\n"
			     "so this particular version is not supported");
	free(p);
        GWEN_Tag16_free(tlv);
	GWEN_Crypt_Token_Context_free(fct);
        return -1;
      }
      break;

    case GWEN_CRYPT_TOKEN_OHBCI_TAG_VERSION_MINOR:
      assert(p);
      i=atoi(p);
      DBG_INFO(GWEN_LOGDOMAIN, "OHBCI: Minor version: %d", i);
      if (i>GWEN_CRYPT_TOKEN_OHBCI_VMINOR) {
	DBG_ERROR(GWEN_LOGDOMAIN,
		  "Keyfile version is higher than mine (%d).\n",
		  i);
	GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Warning,
			     "This key file file has been created with a "
			     "newer library version.\n");
	free(p);
        GWEN_Tag16_free(tlv);
        GWEN_Crypt_Token_Context_free(fct);
        return GWEN_ERROR_NOT_SUPPORTED;
      }
      else if (i<GWEN_CRYPT_TOKEN_OHBCI_VMINOR) {
        DBG_INFO(GWEN_LOGDOMAIN, "Will update this file upon unmount");
      }
      lct->vminor=i;
      break;

    case GWEN_CRYPT_TOKEN_OHBCI_TAG_SEQ:
      assert(p);
      localSignSeq=atoi(p);
      if (localSignSeq==0)
	localSignSeq=1;
      break;

    case GWEN_CRYPT_TOKEN_OHBCI_TAG_USER_ID:
      GWEN_Crypt_Token_Context_SetUserId(fct, p);
      break;

    case GWEN_CRYPT_TOKEN_OHBCI_TAG_INST_COUNTRY:
      /* ignore */
      break;

    case GWEN_CRYPT_TOKEN_OHBCI_TAG_INST_CODE:
      GWEN_Crypt_Token_Context_SetServiceId(fct, p);
      break;

    case GWEN_CRYPT_TOKEN_OHBCI_TAG_INST_SYSTEMID:
      GWEN_Crypt_Token_Context_SetSystemId(fct, p);
      break;

    case GWEN_CRYPT_TOKEN_OHBCI_TAG_SERVER_ADDR:
      /* new in 1.4 */
      GWEN_Crypt_Token_Context_SetAddress(fct, p);
      break;

    case GWEN_CRYPT_TOKEN_OHBCI_TAG_SERVER_PORT:
      /* new in 1.4 */
      GWEN_Crypt_Token_Context_SetPort(fct, atoi(p));
      break;

    case GWEN_CRYPT_TOKEN_OHBCI_TAG_REMOTE_SEQ:
      /* new in 1.4 */
      remoteSignSeq=atoi(p);
      break;

    case GWEN_CRYPT_TOKEN_OHBCI_TAG_USER_PUBSIGNKEY:
    case GWEN_CRYPT_TOKEN_OHBCI_TAG_USER_PRIVSIGNKEY:
      GWEN_Crypt_TokenOHBCI__DecodeKey(ct, tlv, dbKeys, "localSignKey");
      break;

    case GWEN_CRYPT_TOKEN_OHBCI_TAG_USER_PUBCRYPTKEY:
    case GWEN_CRYPT_TOKEN_OHBCI_TAG_USER_PRIVCRYPTKEY:
      GWEN_Crypt_TokenOHBCI__DecodeKey(ct, tlv, dbKeys, "localCryptKey");
      break;

    case GWEN_CRYPT_TOKEN_OHBCI_TAG_INST_PUBSIGNKEY:
      GWEN_Crypt_TokenOHBCI__DecodeKey(ct, tlv, dbKeys, "remoteSignKey");
      break;

    case GWEN_CRYPT_TOKEN_OHBCI_TAG_INST_PUBCRYPTKEY:
      GWEN_Crypt_TokenOHBCI__DecodeKey(ct, tlv, dbKeys, "remoteCryptKey");
      break;

      /* handled again since 1.8 */
    case GWEN_CRYPT_TOKEN_OHBCI_TAG_TEMP_PUBSIGNKEY:
    case GWEN_CRYPT_TOKEN_OHBCI_TAG_TEMP_PRIVSIGNKEY:
      GWEN_Crypt_TokenOHBCI__DecodeKey(ct, tlv, dbKeys, "tempLocalSignKey");
      break;

    case GWEN_CRYPT_TOKEN_OHBCI_TAG_TEMP_PUBCRYPTKEY:
    case GWEN_CRYPT_TOKEN_OHBCI_TAG_TEMP_PRIVCRYPTKEY:
      DBG_INFO(GWEN_LOGDOMAIN,
	       "Ignoring temporary crypt keys");
      break;

    case GWEN_CRYPT_TOKEN_OHBCI_TAG_USER_PRIVAUTHKEY:
      GWEN_Crypt_TokenOHBCI__DecodeKey(ct, tlv, dbKeys, "localAuthKey");
      break;

    case GWEN_CRYPT_TOKEN_OHBCI_TAG_INST_PUBAUTHKEY:
      GWEN_Crypt_TokenOHBCI__DecodeKey(ct, tlv, dbKeys, "remoteAuthKey");
      break;

    case GWEN_CRYPT_TOKEN_OHBCI_TAG_HEADER:
      /* ignore header here */
      break;
    default:
      DBG_WARN(GWEN_LOGDOMAIN, "Unknown tag %02x",
	       GWEN_Tag16_GetTagType(tlv));
      break;
    } /* switch */

    GWEN_Tag16_free(tlv);
    free(p);
  } /* while */


  /* now check for keys */
  rv=0;

  /* local sign key */
  dbKey=GWEN_DB_GetGroup(dbKeys, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                         "localSignKey");
  if (dbKey) {
    GWEN_CRYPT_KEY *key;

    GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
			"rsa/isPublic", 0);
    key=GWEN_Crypt_KeyRsa_fromDb(dbKey);
    if (!key) {
      rv=-1;
      DBG_ERROR(GWEN_LOGDOMAIN, "Bad key format");
    }
    else {
      GWEN_CRYPT_TOKEN_KEYINFO *ki;
      uint8_t kbuf[256];
      uint32_t klen;

      /* create key info */
      ki=GWEN_Crypt_Token_KeyInfo_new(0x0001, /* local sign key */
				      GWEN_Crypt_Key_GetCryptAlgoId(key),
				      GWEN_Crypt_Key_GetKeySize(key));
      assert(ki);

      GWEN_Crypt_Token_KeyInfo_SetKeyDescr(ki, I18N("Local Sign Key"));

      /* get modulus */
      klen=sizeof(kbuf);
      rv=GWEN_Crypt_KeyRsa_GetModulus(key, kbuf, &klen);
      if (rv) {
	DBG_INFO(GWEN_LOGDOMAIN, "No modulus for key");
	GWEN_Crypt_Token_KeyInfo_free(ki);
	GWEN_Crypt_Key_free(key);
	return rv;
      }
      GWEN_Crypt_Token_KeyInfo_SetModulus(ki, kbuf, klen);

      /* get exponent */
      klen=sizeof(kbuf);
      rv=GWEN_Crypt_KeyRsa_GetExponent(key, kbuf, &klen);
      if (rv) {
	DBG_INFO(GWEN_LOGDOMAIN, "No exponent for key");
	GWEN_Crypt_Token_KeyInfo_free(ki);
	GWEN_Crypt_Key_free(key);
	return rv;
      }
      GWEN_Crypt_Token_KeyInfo_SetExponent(ki, kbuf, klen);

      GWEN_Crypt_Token_KeyInfo_SetSignCounter(ki, localSignSeq);
      GWEN_Crypt_Token_KeyInfo_SetKeyNumber(ki, GWEN_Crypt_Key_GetKeyNumber(key));
      GWEN_Crypt_Token_KeyInfo_SetKeyVersion(ki, GWEN_Crypt_Key_GetKeyVersion(key));

      GWEN_Crypt_Token_KeyInfo_AddFlags(ki,
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS |
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT |
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER |
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASSIGNCOUNTER |
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASACTIONFLAGS |
                                        GWEN_CRYPT_TOKEN_KEYFLAGS_CANVERIFY |
					GWEN_CRYPT_TOKEN_KEYFLAGS_CANSIGN);
      GWEN_CTF_Context_SetLocalSignKeyInfo(fct, ki);

      /* set key */
      if (GWEN_Crypt_Token_GetModes(ct) & GWEN_CRYPT_TOKEN_MODE_DIRECT_SIGN){
        DBG_INFO(GWEN_LOGDOMAIN, "Adding mode \"direct sign\" to key");
	GWEN_Crypt_KeyRsa_AddFlags(key, GWEN_CRYPT_KEYRSA_FLAGS_DIRECTSIGN);
      }
      GWEN_CTF_Context_SetLocalSignKey(fct, key);
    }
  }
  else {
    GWEN_CRYPT_TOKEN_KEYINFO *ki;

    /* create key info */
    ki=GWEN_Crypt_Token_KeyInfo_new(0x0001, /* local sign key */
                                    GWEN_Crypt_CryptAlgoId_Rsa, 96);
    assert(ki);

    GWEN_Crypt_Token_KeyInfo_SetKeyDescr(ki, I18N("Local Sign Key"));
    GWEN_Crypt_Token_KeyInfo_AddFlags(ki,
				      GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER |
				      GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
				      GWEN_CRYPT_TOKEN_KEYFLAGS_HASACTIONFLAGS |
				      GWEN_CRYPT_TOKEN_KEYFLAGS_CANVERIFY |
				      GWEN_CRYPT_TOKEN_KEYFLAGS_CANSIGN);
    GWEN_CTF_Context_SetLocalSignKeyInfo(fct, ki);
  }

  /* local crypt key */
  dbKey=GWEN_DB_GetGroup(dbKeys, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                         "localCryptKey");
  if (dbKey) {
    GWEN_CRYPT_KEY *key;

    GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
			"rsa/isPublic", 0);
    key=GWEN_Crypt_KeyRsa_fromDb(dbKey);
    if (!key) {
      rv=-1;
      DBG_ERROR(GWEN_LOGDOMAIN, "Bad key format");
    }
    else {
      GWEN_CRYPT_TOKEN_KEYINFO *ki;
      uint8_t kbuf[256];
      uint32_t klen;

      /* create key info */
      ki=GWEN_Crypt_Token_KeyInfo_new(0x0002, /* local crypt key */
				      GWEN_Crypt_Key_GetCryptAlgoId(key),
				      GWEN_Crypt_Key_GetKeySize(key));
      assert(ki);
      GWEN_Crypt_Token_KeyInfo_SetKeyDescr(ki, I18N("Local Crypt Key"));

      /* get modulus */
      klen=sizeof(kbuf);
      rv=GWEN_Crypt_KeyRsa_GetModulus(key, kbuf, &klen);
      if (rv) {
	DBG_INFO(GWEN_LOGDOMAIN, "No modulus for key");
	GWEN_Crypt_Token_KeyInfo_free(ki);
	GWEN_Crypt_Key_free(key);
	return rv;
      }
      GWEN_Crypt_Token_KeyInfo_SetModulus(ki, kbuf, klen);

      /* get exponent */
      klen=sizeof(kbuf);
      rv=GWEN_Crypt_KeyRsa_GetExponent(key, kbuf, &klen);
      if (rv) {
	DBG_INFO(GWEN_LOGDOMAIN, "No exponent for key");
	GWEN_Crypt_Token_KeyInfo_free(ki);
	GWEN_Crypt_Key_free(key);
	return rv;
      }
      GWEN_Crypt_Token_KeyInfo_SetExponent(ki, kbuf, klen);
      GWEN_Crypt_Token_KeyInfo_SetKeyNumber(ki, GWEN_Crypt_Key_GetKeyNumber(key));
      GWEN_Crypt_Token_KeyInfo_SetKeyVersion(ki, GWEN_Crypt_Key_GetKeyVersion(key));

      GWEN_Crypt_Token_KeyInfo_AddFlags(ki,
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS |
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT |
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER |
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASACTIONFLAGS |
                                        GWEN_CRYPT_TOKEN_KEYFLAGS_CANENCIPHER |
					GWEN_CRYPT_TOKEN_KEYFLAGS_CANDECIPHER);
      GWEN_CTF_Context_SetLocalCryptKeyInfo(fct, ki);
      /* set key */
      GWEN_CTF_Context_SetLocalCryptKey(fct, key);
    }
  }
  else {
    GWEN_CRYPT_TOKEN_KEYINFO *ki;

    /* create key info */
    ki=GWEN_Crypt_Token_KeyInfo_new(0x0002, /* local crypt key */
				    GWEN_Crypt_CryptAlgoId_Rsa, 96);
    assert(ki);
    GWEN_Crypt_Token_KeyInfo_SetKeyDescr(ki, I18N("Local Crypt Key"));
    GWEN_Crypt_Token_KeyInfo_AddFlags(ki,
				      GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER |
				      GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
				      GWEN_CRYPT_TOKEN_KEYFLAGS_HASACTIONFLAGS |
				      GWEN_CRYPT_TOKEN_KEYFLAGS_CANENCIPHER |
				      GWEN_CRYPT_TOKEN_KEYFLAGS_CANDECIPHER);
    GWEN_CTF_Context_SetLocalCryptKeyInfo(fct, ki);
  }

  /* remote sign key */
  dbKey=GWEN_DB_GetGroup(dbKeys, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
			 "remoteSignKey");
  if (dbKey) {
    GWEN_CRYPT_KEY *key;

    GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
			"rsa/isPublic", 1);
    key=GWEN_Crypt_KeyRsa_fromDb(dbKey);
    if (!key) {
      rv=-1;
      DBG_ERROR(GWEN_LOGDOMAIN, "Bad key format");
    }
    else {
      GWEN_CRYPT_TOKEN_KEYINFO *ki;
      uint8_t kbuf[256];
      uint32_t klen;

      /* create key info */
      ki=GWEN_Crypt_Token_KeyInfo_new(0x0003, /* remote sign key */
				      GWEN_Crypt_Key_GetCryptAlgoId(key),
				      GWEN_Crypt_Key_GetKeySize(key));
      assert(ki);
      GWEN_Crypt_Token_KeyInfo_SetKeyDescr(ki, I18N("Remote Sign Key"));

      /* get modulus */
      klen=sizeof(kbuf);
      rv=GWEN_Crypt_KeyRsa_GetModulus(key, kbuf, &klen);
      if (rv) {
	DBG_INFO(GWEN_LOGDOMAIN, "No modulus for key");
	GWEN_Crypt_Token_KeyInfo_free(ki);
	GWEN_Crypt_Key_free(key);
	return rv;
      }
      GWEN_Crypt_Token_KeyInfo_SetModulus(ki, kbuf, klen);

      /* get exponent */
      klen=sizeof(kbuf);
      rv=GWEN_Crypt_KeyRsa_GetExponent(key, kbuf, &klen);
      if (rv) {
	DBG_INFO(GWEN_LOGDOMAIN, "No exponent for key");
	GWEN_Crypt_Token_KeyInfo_free(ki);
	GWEN_Crypt_Key_free(key);
	return rv;
      }
      GWEN_Crypt_Token_KeyInfo_SetExponent(ki, kbuf, klen);

      GWEN_Crypt_Token_KeyInfo_SetSignCounter(ki, remoteSignSeq);
      GWEN_Crypt_Token_KeyInfo_SetKeyNumber(ki, GWEN_Crypt_Key_GetKeyNumber(key));
      GWEN_Crypt_Token_KeyInfo_SetKeyVersion(ki, GWEN_Crypt_Key_GetKeyVersion(key));

      GWEN_Crypt_Token_KeyInfo_AddFlags(ki,
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS |
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT |
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER |
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASSIGNCOUNTER |
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASACTIONFLAGS |
					GWEN_CRYPT_TOKEN_KEYFLAGS_CANVERIFY);
      GWEN_CTF_Context_SetRemoteSignKeyInfo(fct, ki);

      /* set key */
      if (GWEN_Crypt_Token_GetModes(ct) & GWEN_CRYPT_TOKEN_MODE_DIRECT_SIGN)
	GWEN_Crypt_KeyRsa_AddFlags(key, GWEN_CRYPT_KEYRSA_FLAGS_DIRECTSIGN);
      GWEN_CTF_Context_SetRemoteSignKey(fct, key);
    }
  }
  else {
    GWEN_CRYPT_TOKEN_KEYINFO *ki;

    /* create key info */
    ki=GWEN_Crypt_Token_KeyInfo_new(0x0003, /* remote sign key */
				    GWEN_Crypt_CryptAlgoId_Rsa, 96);
    assert(ki);
    GWEN_Crypt_Token_KeyInfo_SetKeyDescr(ki, I18N("Remote Sign Key"));
    GWEN_Crypt_Token_KeyInfo_AddFlags(ki,
				      GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER |
				      GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
				      GWEN_CRYPT_TOKEN_KEYFLAGS_HASACTIONFLAGS |
				      GWEN_CRYPT_TOKEN_KEYFLAGS_CANVERIFY);
    GWEN_CTF_Context_SetRemoteSignKeyInfo(fct, ki);
  }

  /* remote crypt key */
  dbKey=GWEN_DB_GetGroup(dbKeys, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                         "remoteCryptKey");
  if (dbKey) {
    GWEN_CRYPT_KEY *key;

    peerId=GWEN_DB_GetCharValue(dbKey, "keyOwner", 0, NULL);
    if (peerId) {
      DBG_INFO(0, "Got Peer Id [%s]", peerId);
    }
    GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
			"rsa/isPublic", 1);
    key=GWEN_Crypt_KeyRsa_fromDb(dbKey);
    if (!key) {
      rv=-1;
      DBG_ERROR(GWEN_LOGDOMAIN, "Bad key format");
    }
    else {
      GWEN_CRYPT_TOKEN_KEYINFO *ki;
      uint8_t kbuf[256];
      uint32_t klen;

      /* create key info */
      ki=GWEN_Crypt_Token_KeyInfo_new(0x0004, /* remote crypt key */
				      GWEN_Crypt_Key_GetCryptAlgoId(key),
				      GWEN_Crypt_Key_GetKeySize(key));
      assert(ki);
      GWEN_Crypt_Token_KeyInfo_SetKeyDescr(ki, I18N("Remote Crypt Key"));

      /* get modulus */
      klen=sizeof(kbuf);
      rv=GWEN_Crypt_KeyRsa_GetModulus(key, kbuf, &klen);
      if (rv) {
	DBG_INFO(GWEN_LOGDOMAIN, "No modulus for key");
	GWEN_Crypt_Token_KeyInfo_free(ki);
	GWEN_Crypt_Key_free(key);
	return rv;
      }
      GWEN_Crypt_Token_KeyInfo_SetModulus(ki, kbuf, klen);

      /* get exponent */
      klen=sizeof(kbuf);
      rv=GWEN_Crypt_KeyRsa_GetExponent(key, kbuf, &klen);
      if (rv) {
	DBG_INFO(GWEN_LOGDOMAIN, "No exponent for key");
	GWEN_Crypt_Token_KeyInfo_free(ki);
	GWEN_Crypt_Key_free(key);
	return rv;
      }
      GWEN_Crypt_Token_KeyInfo_SetExponent(ki, kbuf, klen);
      GWEN_Crypt_Token_KeyInfo_SetKeyNumber(ki, GWEN_Crypt_Key_GetKeyNumber(key));
      GWEN_Crypt_Token_KeyInfo_SetKeyVersion(ki, GWEN_Crypt_Key_GetKeyVersion(key));
      GWEN_Crypt_Token_KeyInfo_AddFlags(ki,
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS |
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT |
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER |
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASACTIONFLAGS |
					GWEN_CRYPT_TOKEN_KEYFLAGS_CANENCIPHER);
      GWEN_CTF_Context_SetRemoteCryptKeyInfo(fct, ki);

      /* set key */
      GWEN_CTF_Context_SetRemoteCryptKey(fct, key);
    }
  }
  else {
    GWEN_CRYPT_TOKEN_KEYINFO *ki;

    /* create key info */
    ki=GWEN_Crypt_Token_KeyInfo_new(0x0004, /* remote crypt key */
				    GWEN_Crypt_CryptAlgoId_Rsa, 96);
    assert(ki);
    GWEN_Crypt_Token_KeyInfo_SetKeyDescr(ki, I18N("Remote Crypt Key"));
    GWEN_Crypt_Token_KeyInfo_AddFlags(ki,
				      GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER |
				      GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
				      GWEN_CRYPT_TOKEN_KEYFLAGS_HASACTIONFLAGS |
				      GWEN_CRYPT_TOKEN_KEYFLAGS_CANENCIPHER);
    GWEN_CTF_Context_SetRemoteCryptKeyInfo(fct, ki);
  }

  /* local auth key */
  dbKey=GWEN_DB_GetGroup(dbKeys, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                         "localAuthKey");
  if (dbKey) {
    GWEN_CRYPT_KEY *key;

    GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
			"rsa/isPublic", 0);
    key=GWEN_Crypt_KeyRsa_fromDb(dbKey);
    if (!key) {
      rv=-1;
      DBG_ERROR(GWEN_LOGDOMAIN, "Bad key format");
    }
    else {
      GWEN_CRYPT_TOKEN_KEYINFO *ki;
      uint8_t kbuf[256];
      uint32_t klen;

      /* create key info */
      ki=GWEN_Crypt_Token_KeyInfo_new(0x0005, /* local auth key */
				      GWEN_Crypt_Key_GetCryptAlgoId(key),
				      GWEN_Crypt_Key_GetKeySize(key));
      assert(ki);
      GWEN_Crypt_Token_KeyInfo_SetKeyDescr(ki, I18N("Local Auth Key"));

      /* get modulus */
      klen=sizeof(kbuf);
      rv=GWEN_Crypt_KeyRsa_GetModulus(key, kbuf, &klen);
      if (rv) {
	DBG_INFO(GWEN_LOGDOMAIN, "No modulus for key");
	GWEN_Crypt_Token_KeyInfo_free(ki);
	GWEN_Crypt_Key_free(key);
	return rv;
      }
      GWEN_Crypt_Token_KeyInfo_SetModulus(ki, kbuf, klen);

      /* get exponent */
      klen=sizeof(kbuf);
      rv=GWEN_Crypt_KeyRsa_GetExponent(key, kbuf, &klen);
      if (rv) {
	DBG_INFO(GWEN_LOGDOMAIN, "No exponent for key");
	GWEN_Crypt_Token_KeyInfo_free(ki);
	GWEN_Crypt_Key_free(key);
	return rv;
      }
      GWEN_Crypt_Token_KeyInfo_SetExponent(ki, kbuf, klen);
      GWEN_Crypt_Token_KeyInfo_SetKeyNumber(ki, GWEN_Crypt_Key_GetKeyNumber(key));
      GWEN_Crypt_Token_KeyInfo_SetKeyVersion(ki, GWEN_Crypt_Key_GetKeyVersion(key));
      GWEN_Crypt_Token_KeyInfo_AddFlags(ki,
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS |
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT |
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER |
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASACTIONFLAGS |
					GWEN_CRYPT_TOKEN_KEYFLAGS_CANSIGN |
					GWEN_CRYPT_TOKEN_KEYFLAGS_CANVERIFY);
      GWEN_CTF_Context_SetLocalAuthKeyInfo(fct, ki);
      /* set key */
      if (GWEN_Crypt_Token_GetModes(ct) & GWEN_CRYPT_TOKEN_MODE_DIRECT_SIGN){
        DBG_INFO(GWEN_LOGDOMAIN, "Adding mode \"direct sign\" to key");
	GWEN_Crypt_KeyRsa_AddFlags(key, GWEN_CRYPT_KEYRSA_FLAGS_DIRECTSIGN);
      }
      GWEN_CTF_Context_SetLocalAuthKey(fct, key);
    }
  }
  else {
    GWEN_CRYPT_TOKEN_KEYINFO *ki;

    /* create key info */
    ki=GWEN_Crypt_Token_KeyInfo_new(0x0005, /* local auth key */
				    GWEN_Crypt_CryptAlgoId_Rsa, 96);
    assert(ki);
    GWEN_Crypt_Token_KeyInfo_SetKeyDescr(ki, I18N("Local Auth Key"));
    GWEN_Crypt_Token_KeyInfo_AddFlags(ki,
				      GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER |
				      GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
				      GWEN_CRYPT_TOKEN_KEYFLAGS_HASACTIONFLAGS |
				      GWEN_CRYPT_TOKEN_KEYFLAGS_CANSIGN |
				      GWEN_CRYPT_TOKEN_KEYFLAGS_CANVERIFY);
    GWEN_CTF_Context_SetLocalAuthKeyInfo(fct, ki);
  }

  /* remote auth key */
  dbKey=GWEN_DB_GetGroup(dbKeys, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                         "remoteAuthKey");
  if (dbKey) {
    GWEN_CRYPT_KEY *key;

    GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
			"rsa/isPublic", 1);
    key=GWEN_Crypt_KeyRsa_fromDb(dbKey);
    if (!key) {
      rv=-1;
      DBG_ERROR(GWEN_LOGDOMAIN, "Bad key format");
    }
    else {
      GWEN_CRYPT_TOKEN_KEYINFO *ki;
      uint8_t kbuf[256];
      uint32_t klen;

      /* create key info */
      ki=GWEN_Crypt_Token_KeyInfo_new(0x0006, /* remote auth key */
				      GWEN_Crypt_Key_GetCryptAlgoId(key),
				      GWEN_Crypt_Key_GetKeySize(key));
      assert(ki);
      GWEN_Crypt_Token_KeyInfo_SetKeyDescr(ki, I18N("Remote Auth Key"));

      /* get modulus */
      klen=sizeof(kbuf);
      rv=GWEN_Crypt_KeyRsa_GetModulus(key, kbuf, &klen);
      if (rv) {
	DBG_INFO(GWEN_LOGDOMAIN, "No modulus for key");
	GWEN_Crypt_Token_KeyInfo_free(ki);
	GWEN_Crypt_Key_free(key);
	return rv;
      }
      GWEN_Crypt_Token_KeyInfo_SetModulus(ki, kbuf, klen);

      /* get exponent */
      klen=sizeof(kbuf);
      rv=GWEN_Crypt_KeyRsa_GetExponent(key, kbuf, &klen);
      if (rv) {
	DBG_INFO(GWEN_LOGDOMAIN, "No exponent for key");
	GWEN_Crypt_Token_KeyInfo_free(ki);
	GWEN_Crypt_Key_free(key);
	return rv;
      }
      GWEN_Crypt_Token_KeyInfo_SetExponent(ki, kbuf, klen);
      GWEN_Crypt_Token_KeyInfo_SetKeyNumber(ki, GWEN_Crypt_Key_GetKeyNumber(key));
      GWEN_Crypt_Token_KeyInfo_SetKeyVersion(ki, GWEN_Crypt_Key_GetKeyVersion(key));
      GWEN_Crypt_Token_KeyInfo_AddFlags(ki,
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS |
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT |
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER |
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASACTIONFLAGS |
					GWEN_CRYPT_TOKEN_KEYFLAGS_CANVERIFY);
      GWEN_CTF_Context_SetRemoteAuthKeyInfo(fct, ki);
      /* set key */
      if (GWEN_Crypt_Token_GetModes(ct) & GWEN_CRYPT_TOKEN_MODE_DIRECT_SIGN){
        DBG_INFO(GWEN_LOGDOMAIN, "Adding mode \"direct sign\" to key");
	GWEN_Crypt_KeyRsa_AddFlags(key, GWEN_CRYPT_KEYRSA_FLAGS_DIRECTSIGN);
      }
      GWEN_CTF_Context_SetRemoteAuthKey(fct, key);
    }
  }
  else {
    GWEN_CRYPT_TOKEN_KEYINFO *ki;

    /* create key info */
    ki=GWEN_Crypt_Token_KeyInfo_new(0x0006, /* remote auth key */
				    GWEN_Crypt_CryptAlgoId_Rsa, 96);
    assert(ki);
    GWEN_Crypt_Token_KeyInfo_SetKeyDescr(ki, I18N("Remote Auth Key"));
    GWEN_Crypt_Token_KeyInfo_AddFlags(ki,
				      GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER |
				      GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
				      GWEN_CRYPT_TOKEN_KEYFLAGS_HASACTIONFLAGS |
				      GWEN_CRYPT_TOKEN_KEYFLAGS_CANVERIFY);
    GWEN_CTF_Context_SetRemoteAuthKeyInfo(fct, ki);
  }


  /* temp local sign key */
  dbKey=GWEN_DB_GetGroup(dbKeys, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                         "tempLocalSignKey");
  if (dbKey) {
    GWEN_CRYPT_KEY *key;

    GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
			"rsa/isPublic", 0);
    key=GWEN_Crypt_KeyRsa_fromDb(dbKey);
    if (!key) {
      rv=-1;
      DBG_ERROR(GWEN_LOGDOMAIN, "Bad key format");
    }
    else {
      GWEN_CRYPT_TOKEN_KEYINFO *ki;
      uint8_t kbuf[256];
      uint32_t klen;

      /* create key info */
      ki=GWEN_Crypt_Token_KeyInfo_new(0x0007, /* temp local sign key */
				      GWEN_Crypt_Key_GetCryptAlgoId(key),
				      GWEN_Crypt_Key_GetKeySize(key));
      assert(ki);

      GWEN_Crypt_Token_KeyInfo_SetKeyDescr(ki, I18N("Temporary Local Sign Key"));

      /* get modulus */
      klen=sizeof(kbuf);
      rv=GWEN_Crypt_KeyRsa_GetModulus(key, kbuf, &klen);
      if (rv) {
	DBG_INFO(GWEN_LOGDOMAIN, "No modulus for key");
	GWEN_Crypt_Token_KeyInfo_free(ki);
	GWEN_Crypt_Key_free(key);
	return rv;
      }
      GWEN_Crypt_Token_KeyInfo_SetModulus(ki, kbuf, klen);

      /* get exponent */
      klen=sizeof(kbuf);
      rv=GWEN_Crypt_KeyRsa_GetExponent(key, kbuf, &klen);
      if (rv) {
	DBG_INFO(GWEN_LOGDOMAIN, "No exponent for key");
	GWEN_Crypt_Token_KeyInfo_free(ki);
	GWEN_Crypt_Key_free(key);
	return rv;
      }
      GWEN_Crypt_Token_KeyInfo_SetExponent(ki, kbuf, klen);

      GWEN_Crypt_Token_KeyInfo_SetKeyNumber(ki, GWEN_Crypt_Key_GetKeyNumber(key));
      GWEN_Crypt_Token_KeyInfo_SetKeyVersion(ki, GWEN_Crypt_Key_GetKeyVersion(key));

      GWEN_Crypt_Token_KeyInfo_AddFlags(ki,
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS |
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT |
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER |
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASSIGNCOUNTER |
					GWEN_CRYPT_TOKEN_KEYFLAGS_HASACTIONFLAGS |
                                        GWEN_CRYPT_TOKEN_KEYFLAGS_CANVERIFY |
					GWEN_CRYPT_TOKEN_KEYFLAGS_CANSIGN);
      GWEN_CTF_Context_SetTempLocalSignKeyInfo(fct, ki);

      /* set key */
      if (GWEN_Crypt_Token_GetModes(ct) & GWEN_CRYPT_TOKEN_MODE_DIRECT_SIGN){
        DBG_INFO(GWEN_LOGDOMAIN, "Adding mode \"direct sign\" to key");
	GWEN_Crypt_KeyRsa_AddFlags(key, GWEN_CRYPT_KEYRSA_FLAGS_DIRECTSIGN);
      }
      GWEN_CTF_Context_SetTempLocalSignKey(fct, key);
    }
  }
  else {
    GWEN_CRYPT_TOKEN_KEYINFO *ki;

    /* create key info */
    ki=GWEN_Crypt_Token_KeyInfo_new(0x0007, /* temp local sign key */
                                    GWEN_Crypt_CryptAlgoId_Rsa, 96);
    assert(ki);

    GWEN_Crypt_Token_KeyInfo_SetKeyDescr(ki, I18N("Temporary Local Sign Key"));
    GWEN_Crypt_Token_KeyInfo_AddFlags(ki,
				      GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER |
				      GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
				      GWEN_CRYPT_TOKEN_KEYFLAGS_HASACTIONFLAGS |
				      GWEN_CRYPT_TOKEN_KEYFLAGS_CANVERIFY |
				      GWEN_CRYPT_TOKEN_KEYFLAGS_CANSIGN);
    GWEN_CTF_Context_SetTempLocalSignKeyInfo(fct, ki);
  }


  /* finalize user and file context */
  GWEN_Crypt_Token_Context_SetId(fct, 1);            /* only one user */
  GWEN_Crypt_Token_Context_SetPeerId(fct, peerId);   /* only one user */

  GWEN_Crypt_Token_Context_SetSignKeyId(fct, 0x01);
  GWEN_Crypt_Token_Context_SetDecipherKeyId(fct, 0x02);
  GWEN_Crypt_Token_Context_SetVerifyKeyId(fct, 0x03);
  GWEN_Crypt_Token_Context_SetEncipherKeyId(fct, 0x04);
  GWEN_Crypt_Token_Context_SetAuthSignKeyId(fct, 0x05);
  GWEN_Crypt_Token_Context_SetAuthVerifyKeyId(fct, 0x06);
  GWEN_Crypt_Token_Context_SetTempSignKeyId(fct, 0x07);

  /* clear context list, add new context */
  GWEN_Crypt_TokenFile_AddContext(ct, fct);

  GWEN_DB_Group_free(dbKeys);
  return rv;
}



int GWEN_Crypt_TokenOHBCI__DecryptFile16(GWEN_CRYPT_TOKEN *ct,
					 GWEN_BUFFER *dbuf,
					 int tryNum,
					 uint32_t gid) {
  GWEN_CRYPT_TOKEN_OHBCI *lct;
  GWEN_TAG16 *tlv;
  int rv;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, GWEN_CRYPT_TOKEN_OHBCI, ct);
  assert(lct);

  tlv=GWEN_Tag16_fromBuffer(dbuf, 0);
  GWEN_Buffer_Rewind(dbuf);
  if (!tlv) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "File doesn't contain a TLV: Either bad pin or bad file");
    return GWEN_ERROR_BAD_PIN;
  }

  if (GWEN_Tag16_GetTagType(tlv)!=GWEN_CRYPT_TOKEN_OHBCI_TAG_HEADER) {
    DBG_ERROR(GWEN_LOGDOMAIN, "File doesn't start with header tag.");
    GWEN_Tag16_free(tlv);
    return GWEN_ERROR_GENERIC;
  }
  GWEN_Tag16_free(tlv);

  /* now parse it */
  while(GWEN_Buffer_GetBytesLeft(dbuf)) {
    int i;
    const char *pp;
    char *p;
    unsigned int l;

    tlv=GWEN_Tag16_fromBuffer(dbuf, 0);
    if (!tlv) {
      DBG_ERROR(GWEN_LOGDOMAIN,
		"File doesn't contain a TLV: Either bad pin or bad file");
      return GWEN_ERROR_BAD_PIN;
    }
    p=0;
    pp=(const char*)GWEN_Tag16_GetTagData(tlv);
    l=GWEN_Tag16_GetTagLength(tlv);
    if (pp && l) {
      p=(char*)malloc(l+1);
      assert(p);
      memmove(p, pp, l);
      p[l]=0;
    }

    switch(GWEN_Tag16_GetTagType(tlv)) {
    case GWEN_CRYPT_TOKEN_OHBCI_TAG_HEADER:
      if (strcasecmp(p, GWEN_CRYPT_TOKEN_OHBCI_NAME)!=0) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Bad header (%s)", p);
        free(p);
        GWEN_Tag16_free(tlv);
        return -1;
      }
      break;

    case GWEN_CRYPT_TOKEN_OHBCI_TAG_VERSION_MAJOR:
      i=atoi(p);
      if (i!=GWEN_CRYPT_TOKEN_OHBCI_VMAJOR) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Unsupported keyfile version (%d)", i);
	GWEN_Gui_ProgressLog(0,
                             GWEN_LoggerLevel_Warning,
			     "Basically this file type is supported.\n"
			     "However, the major versions do not match,\n"
			     "so this particular version is not supported");
	free(p);
        GWEN_Tag16_free(tlv);
        return -1;
      }
      break;

    case GWEN_CRYPT_TOKEN_OHBCI_TAG_VERSION_MINOR:
      i=atoi(p);
      if (i>GWEN_CRYPT_TOKEN_OHBCI_VMINOR) {
        DBG_WARN(GWEN_LOGDOMAIN,
                 "Keyfile version is higher than mine (%d).\n",
                 i);
        GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Warning,
			     "This key file file has been created with a "
			     "newer library version.\n");
        free(p);
        GWEN_Tag16_free(tlv);
        return -1;
      }
      else if (i<GWEN_CRYPT_TOKEN_OHBCI_VMINOR) {
        DBG_INFO(GWEN_LOGDOMAIN,
                 "Will update this file upon unmount (%d)", i);
      }
      lct->vminor=i;
      break;

    case GWEN_CRYPT_TOKEN_OHBCI_TAG_CRYPT_OLD:
    case GWEN_CRYPT_TOKEN_OHBCI_TAG_CRYPT:
    case GWEN_CRYPT_TOKEN_OHBCI_TAG_CRYPT_BF:
      {
        GWEN_BUFFER *fbuf;

        lct->cryptoTag=GWEN_Tag16_GetTagType(tlv);
        fbuf=GWEN_Buffer_new(0, GWEN_Tag16_GetTagLength(tlv), 0, 1);
        GWEN_Buffer_AppendBytes(fbuf,
                                GWEN_Tag16_GetTagData(tlv),
                                GWEN_Tag16_GetTagLength(tlv));
        GWEN_Buffer_Rewind(fbuf);
	rv=GWEN_Crypt_TokenOHBCI__DecryptFile(ct, fbuf, tryNum, gid);
        GWEN_Buffer_free(fbuf);
        if (rv) {
          free(p);
          GWEN_Tag16_free(tlv);
          return rv;
        }
        break;
      }

    case GWEN_CRYPT_TOKEN_OHBCI_TAG_CRYPT_TRESOR:
      {
        GWEN_BUFFER *fbuf;

        lct->cryptoTag=GWEN_Tag16_GetTagType(tlv);
        fbuf=GWEN_Buffer_new(0, GWEN_Tag16_GetTagLength(tlv), 0, 1);
        GWEN_Buffer_AppendBytes(fbuf,
                                GWEN_Tag16_GetTagData(tlv),
                                GWEN_Tag16_GetTagLength(tlv));
        GWEN_Buffer_Rewind(fbuf);
	rv=GWEN_Crypt_TokenOHBCI__DecryptTresor(ct, fbuf, tryNum, gid);
        GWEN_Buffer_free(fbuf);
        if (rv) {
          free(p);
          GWEN_Tag16_free(tlv);
          return rv;
        }
        break;
      }

    default:
      DBG_WARN(GWEN_LOGDOMAIN, "Unknown tag %02x",
	       GWEN_Tag16_GetTagType(tlv));
      break;
    } /* switch */

    GWEN_Tag16_free(tlv);
    free(p);
  } /* while */

  return 0;
}



int GWEN_Crypt_TokenOHBCI__EncodeKey(const GWEN_CRYPT_KEY *key,
				     GWEN_CRYPT_TOKEN_CONTEXT *fct,
				     unsigned int tagType,
				     int wantPublic,
				     int isCrypt,
				     GWEN_BUFFER *dbuf) {
  GWEN_DB_NODE *dbKey;
  int err;
  const void *p;
  unsigned int bs;
  char *pp;
  uint32_t pos;
  char numbuf[16];

  if (!key) {
    DBG_INFO(GWEN_LOGDOMAIN, "No key");
    return 0;
  }
  dbKey=GWEN_DB_Group_new("key");
  err=GWEN_Crypt_KeyRsa_toDb(key, dbKey, wantPublic);
  if (err) {
    DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
    GWEN_DB_Group_free(dbKey);
    return err;
  }

  GWEN_Buffer_AppendByte(dbuf, tagType & 0xff);
  /* remember pos to insert size later */
  pos=GWEN_Buffer_GetPos(dbuf);
  GWEN_Buffer_AppendBytes(dbuf, "00", 2);

  /* always write "NO" for "isPublic", since OpenHBCI always writes "NO"
   * due to a bug */
  GWEN_Tag16_DirectlyToBuffer(GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_ISPUBLIC,
                             "NO",
                             -1,
                             dbuf);
  GWEN_Tag16_DirectlyToBuffer(GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_ISCRYPT,
                             isCrypt?"YES":"NO",
                             -1,
                             dbuf);

  snprintf(numbuf, sizeof(numbuf), "%d", GWEN_Crypt_Key_GetKeyNumber(key));
  GWEN_Tag16_DirectlyToBuffer(GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_NUMBER,
                             numbuf,
                             -1,
                             dbuf);
  snprintf(numbuf, sizeof(numbuf), "%d", GWEN_Crypt_Key_GetKeyVersion(key));
  GWEN_Tag16_DirectlyToBuffer(GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_VERSION,
                             numbuf,
                             -1,
			      dbuf);
  snprintf(numbuf, sizeof(numbuf), "%d", GWEN_Crypt_Key_GetKeySize(key));
  GWEN_Tag16_DirectlyToBuffer(GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_LEN,
                             numbuf,
                             -1,
                             dbuf);

  /* write key owner again */
  if (tagType==GWEN_CRYPT_TOKEN_OHBCI_TAG_INST_PUBCRYPTKEY) {
    const char *s;

    s=GWEN_Crypt_Token_Context_GetPeerId(fct);
    if (s)
      GWEN_Tag16_DirectlyToBuffer(GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_OWNER,
				  s, -1, dbuf);
  }

  p=GWEN_DB_GetBinValue(dbKey, "rsa/e", 0, 0, 0, &bs);
  if (p && bs)
    GWEN_Tag16_DirectlyToBuffer(GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_EXP, p, bs, dbuf);

  p=GWEN_DB_GetBinValue(dbKey, "rsa/n", 0, 0, 0, &bs);
  if (p && bs) {
    GWEN_Tag16_DirectlyToBuffer(GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_MODULUS, p, bs, dbuf);
    GWEN_Tag16_DirectlyToBuffer(GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_N, p, bs, dbuf);
  }
  else {
    DBG_WARN(GWEN_LOGDOMAIN, "No modulus !");
  }

  p=GWEN_DB_GetBinValue(dbKey, "rsa/p", 0, 0, 0, &bs);
  if (p && bs)
    GWEN_Tag16_DirectlyToBuffer(GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_P, p, bs, dbuf);

  p=GWEN_DB_GetBinValue(dbKey, "rsa/q", 0, 0, 0, &bs);
  if (p && bs)
    GWEN_Tag16_DirectlyToBuffer(GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_Q, p, bs, dbuf);

  p=GWEN_DB_GetBinValue(dbKey, "rsa/d", 0, 0, 0, &bs);
  if (p && bs)
    GWEN_Tag16_DirectlyToBuffer(GWEN_CRYPT_TOKEN_OHBCI_TAG_KEY_D, p, bs, dbuf);

  GWEN_DB_Group_free(dbKey);
  bs=(GWEN_Buffer_GetPos(dbuf)-pos)-2;
  pp=GWEN_Buffer_GetStart(dbuf)+pos;
  pp[0]=bs & 0xff;
  pp[1]=(bs>>8) & 0xff;

  return 0;
}



int GWEN_Crypt_TokenOHBCI_Encode(GWEN_CRYPT_TOKEN *ct, GWEN_BUFFER *dbuf) {
  GWEN_CRYPT_TOKEN_OHBCI *lct;
  char numbuf[16];
  const char *p;
  GWEN_CRYPT_TOKEN_CONTEXT *fct;
  GWEN_CRYPT_KEY *key;
  GWEN_CRYPT_TOKEN_KEYINFO *ki;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, GWEN_CRYPT_TOKEN_OHBCI, ct);
  assert(lct);

  /* get the only context */
  fct=GWEN_Crypt_TokenFile_GetContext(ct, 0);
  if (!fct) {
    DBG_ERROR(GWEN_LOGDOMAIN,
	      "Crypt token does not contain a file context");
    return GWEN_ERROR_INVALID;
  }

  /* write header again */
  GWEN_Tag16_DirectlyToBuffer(GWEN_CRYPT_TOKEN_OHBCI_TAG_HEADER,
			      GWEN_CRYPT_TOKEN_OHBCI_NAME,
			      -1, dbuf);

  if (lct->mediumTag!=GWEN_CRYPT_TOKEN_OHBCI_TAG_MEDIUM3) {
    /* do not include version info here for ohbci files after 1.5 */
    snprintf(numbuf, sizeof(numbuf), "%d", GWEN_CRYPT_TOKEN_OHBCI_VMAJOR);
    GWEN_Tag16_DirectlyToBuffer(GWEN_CRYPT_TOKEN_OHBCI_TAG_VERSION_MAJOR,
                                numbuf, -1, dbuf);

    snprintf(numbuf, sizeof(numbuf), "%d", GWEN_CRYPT_TOKEN_OHBCI_VMINOR);
    GWEN_Tag16_DirectlyToBuffer(GWEN_CRYPT_TOKEN_OHBCI_TAG_VERSION_MINOR,
                                numbuf, -1, dbuf);
  }

  ki=GWEN_CTF_Context_GetLocalSignKeyInfo(fct);
  if (ki) {
    snprintf(numbuf, sizeof(numbuf), "%d", GWEN_Crypt_Token_KeyInfo_GetSignCounter(ki));
    GWEN_Tag16_DirectlyToBuffer(GWEN_CRYPT_TOKEN_OHBCI_TAG_SEQ, numbuf, -1, dbuf);
  }
  key=GWEN_CTF_Context_GetLocalSignKey(fct);
  if (GWEN_Crypt_TokenOHBCI__EncodeKey(key, fct,
				       GWEN_CRYPT_TOKEN_OHBCI_TAG_USER_PUBSIGNKEY,
				       1, 0, dbuf)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not encode key");
    return -1;
  }

  if (GWEN_Crypt_TokenOHBCI__EncodeKey(key, fct,
				       GWEN_CRYPT_TOKEN_OHBCI_TAG_USER_PRIVSIGNKEY,
				       0, 0, dbuf)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not encode key");
    return -1;
  }

  key=GWEN_CTF_Context_GetLocalCryptKey(fct);
  if (GWEN_Crypt_TokenOHBCI__EncodeKey(key, fct,
				       GWEN_CRYPT_TOKEN_OHBCI_TAG_USER_PUBCRYPTKEY,
				       1, 1, dbuf)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not encode key");
    return -1;
  }

  if (GWEN_Crypt_TokenOHBCI__EncodeKey(key, fct,
				       GWEN_CRYPT_TOKEN_OHBCI_TAG_USER_PRIVCRYPTKEY,
				       0, 1, dbuf)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not encode key");
    return -1;
  }

  p=GWEN_Crypt_Token_Context_GetUserId(fct);
  if (p && *p)
    GWEN_Tag16_DirectlyToBuffer(GWEN_CRYPT_TOKEN_OHBCI_TAG_USER_ID,
				p, -1, dbuf);

  key=GWEN_CTF_Context_GetRemoteSignKey(fct);
  if (GWEN_Crypt_TokenOHBCI__EncodeKey(key, fct,
				       GWEN_CRYPT_TOKEN_OHBCI_TAG_INST_PUBSIGNKEY,
				       1, 0, dbuf)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not encode key");
    return -1;
  }

  key=GWEN_CTF_Context_GetRemoteCryptKey(fct);
  if (GWEN_Crypt_TokenOHBCI__EncodeKey(key, fct,
				       GWEN_CRYPT_TOKEN_OHBCI_TAG_INST_PUBCRYPTKEY,
				       1, 1, dbuf)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not encode key");
    return -1;
  }

  snprintf(numbuf, sizeof(numbuf), "%d", 280);
  GWEN_Tag16_DirectlyToBuffer(GWEN_CRYPT_TOKEN_OHBCI_TAG_INST_COUNTRY,
			      numbuf, -1, dbuf);

  p=GWEN_Crypt_Token_Context_GetServiceId(fct);
  if (p && *p)
    GWEN_Tag16_DirectlyToBuffer(GWEN_CRYPT_TOKEN_OHBCI_TAG_INST_CODE,
				p, -1, dbuf);

  p=GWEN_Crypt_Token_Context_GetSystemId(fct);
  if (p && *p)
    GWEN_Tag16_DirectlyToBuffer(GWEN_CRYPT_TOKEN_OHBCI_TAG_INST_SYSTEMID,
				p, -1, dbuf);

  /* new in 1.4 */
  p=GWEN_Crypt_Token_Context_GetAddress(fct);
  if (p && *p) {
    GWEN_Tag16_DirectlyToBuffer(GWEN_CRYPT_TOKEN_OHBCI_TAG_SERVER_ADDR,
				p, -1, dbuf);
    snprintf(numbuf, sizeof(numbuf), "%d",
	     GWEN_Crypt_Token_Context_GetPort(fct));
    GWEN_Tag16_DirectlyToBuffer(GWEN_CRYPT_TOKEN_OHBCI_TAG_SERVER_PORT,
				numbuf, -1, dbuf);
  }

  /* new in 1.4 */
  ki=GWEN_CTF_Context_GetRemoteSignKeyInfo(fct);
  if (ki) {
    snprintf(numbuf, sizeof(numbuf), "%d", GWEN_Crypt_Token_KeyInfo_GetSignCounter(ki));
    GWEN_Tag16_DirectlyToBuffer(GWEN_CRYPT_TOKEN_OHBCI_TAG_REMOTE_SEQ,
				numbuf, -1, dbuf);
  }

  /* new in 1.7 */
  key=GWEN_CTF_Context_GetLocalAuthKey(fct);
  if (GWEN_Crypt_TokenOHBCI__EncodeKey(key, fct,
				       GWEN_CRYPT_TOKEN_OHBCI_TAG_USER_PRIVAUTHKEY,
				       0, 0, dbuf)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not encode key");
    return -1;
  }

  key=GWEN_CTF_Context_GetRemoteAuthKey(fct);
  if (GWEN_Crypt_TokenOHBCI__EncodeKey(key, fct,
				       GWEN_CRYPT_TOKEN_OHBCI_TAG_INST_PUBAUTHKEY,
				       1, 0, dbuf)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not encode key");
    return -1;
  }

  /* new in 1.8 */
  ki=GWEN_CTF_Context_GetTempLocalSignKeyInfo(fct);
  key=GWEN_CTF_Context_GetTempLocalSignKey(fct);
  if (GWEN_Crypt_TokenOHBCI__EncodeKey(key, fct,
                                       GWEN_CRYPT_TOKEN_OHBCI_TAG_TEMP_PUBSIGNKEY,
				       1, 0, dbuf)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not encode key");
    return -1;
  }

  if (GWEN_Crypt_TokenOHBCI__EncodeKey(key, fct,
				       GWEN_CRYPT_TOKEN_OHBCI_TAG_TEMP_PRIVSIGNKEY,
				       0, 0, dbuf)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not encode key");
    return -1;
  }

  return 0;
}



int GWENHYWFAR_CB 
GWEN_Crypt_TokenOHBCI_Write(GWEN_CRYPT_TOKEN *ct, int fd, int cre, uint32_t gid){
  GWEN_CRYPT_TOKEN_OHBCI *lct;
  int rv;
  GWEN_BUFFER *fbuf;
  GWEN_BUFFER *rawbuf;
  char *p;
  unsigned int bs;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, GWEN_CRYPT_TOKEN_OHBCI, ct);
  assert(lct);

  if (!GWEN_Crypt_Token_GetTokenName(ct)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No medium name given");
    return -1;
  }

  /* maybe update key file */
  if ((GWEN_Crypt_Token_GetModes(ct) & GWEN_CRYPT_TOKEN_MODE_ALLOW_UPDATE) &&
      ((lct->mediumTag!=GWEN_CRYPT_TOKEN_OHBCI_TAG_MEDIUM3) ||
       (lct->cryptoTag!=GWEN_CRYPT_TOKEN_OHBCI_TAG_CRYPT_TRESOR))) {
    DBG_WARN(GWEN_LOGDOMAIN,
	     "Updating ancient key file to new one");
    GWEN_Gui_ProgressLog(gid, GWEN_LoggerLevel_Warning,
			 I18N("Updating ancient key file to new one"));
    lct->passWordIsSet=0;
    lct->mediumTag=GWEN_CRYPT_TOKEN_OHBCI_TAG_MEDIUM3;
    lct->cryptoTag=GWEN_CRYPT_TOKEN_OHBCI_TAG_CRYPT_TRESOR;
  }

  /* create raw data */
  rawbuf=GWEN_Buffer_new(0, 1024, 0, 1);
  rv=GWEN_Crypt_TokenOHBCI_Encode(ct, rawbuf);
  if (rv) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not encode key file (%d)", rv);
    return -1;
  }

#ifdef DEBUG_OHBCI_MODULE
  if (1) {
    FILE *f;

    f=fopen("encoded.medium", "w+b");
    if (f) {
      if (1!=fwrite(GWEN_Buffer_GetStart(rawbuf),
                    GWEN_Buffer_GetUsedBytes(rawbuf),
                    1, f)) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Could not save encoded file.");
      }
      if (fclose(f)) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Could not close encoded file.");
      }
    }
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "Could not open encoded file.");
    }
  }
#endif

  /* create key from password */
  rv=GWEN_Crypt_TokenOHBCI__EnsurePassword(ct, 0, 1 /* always let user confirm new password on write */, gid);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(rawbuf);
    return rv;
  }

  if (lct->cryptoTag==GWEN_CRYPT_TOKEN_OHBCI_TAG_CRYPT_TRESOR) {
    /* encrypt file */
    bs=GWEN_Buffer_GetUsedBytes(rawbuf);
    GWEN_Buffer_Rewind(rawbuf);
    fbuf=GWEN_Buffer_new(0, bs+128, 0, 1);
    GWEN_Buffer_ReserveBytes(fbuf, 4);
    rv=GWEN_SmallTresor_Encrypt((const uint8_t*)GWEN_Buffer_GetStart(rawbuf), bs,
				lct->password,
				fbuf,
				GWEN_CRYPT_TOKEN_OHBCI_TRESOR_PWD_ITERATIONS,
				GWEN_CRYPT_TOKEN_OHBCI_TRESOR_CRYPT_ITERATIONS);
    if (rv<0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Could not encode keyfile");
      GWEN_Buffer_free(fbuf);
      GWEN_Buffer_free(rawbuf);
      return GWEN_ERROR_GENERIC;
    }

    GWEN_Buffer_free(rawbuf);
  }
  else {
    GWEN_CRYPT_KEY *key;

    if (lct->cryptoTag==GWEN_CRYPT_TOKEN_OHBCI_TAG_CRYPT) {
      key=GWEN_Crypt_KeyDes3K_fromData(GWEN_Crypt_CryptMode_Cbc, 24,
				       (const uint8_t*)lct->password, 24);
    }
    else if (lct->cryptoTag==GWEN_CRYPT_TOKEN_OHBCI_TAG_CRYPT_OLD) {
      key=GWEN_Crypt_KeyDes3K_fromData(GWEN_Crypt_CryptMode_Cbc, 16,
				       (const uint8_t*)lct->password, 16);
    }
    else if (lct->cryptoTag==GWEN_CRYPT_TOKEN_OHBCI_TAG_CRYPT_BF) {
      key=GWEN_Crypt_KeyBlowFish_fromData(GWEN_Crypt_CryptMode_Cbc, 16,
					  (const uint8_t*)lct->password, 16);
    }
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unknown crypt tag, should not occur");
      abort();
    }

    if (!key) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Could not create key");
      GWEN_Buffer_free(rawbuf);
      return GWEN_ERROR_GENERIC;
    }

    /* padd raw data */
    if (GWEN_Padd_PaddWithAnsiX9_23(rawbuf)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Could not padd keyfile");
      GWEN_Crypt_Key_free(key);
      GWEN_Buffer_free(rawbuf);
      return GWEN_ERROR_GENERIC;
    }

    /* encrypt file */
    bs=GWEN_Buffer_GetUsedBytes(rawbuf);
    GWEN_Buffer_Rewind(rawbuf);
    fbuf=GWEN_Buffer_new(0, bs+128, 0, 1);
    GWEN_Buffer_ReserveBytes(fbuf, 4);
    rv=GWEN_Crypt_Key_Encipher(key,
			       (const uint8_t*)GWEN_Buffer_GetStart(rawbuf),
			       bs,
			       (uint8_t*)GWEN_Buffer_GetStart(fbuf),
			       &bs);
    GWEN_Buffer_free(rawbuf);
    GWEN_Crypt_Key_free(key);
    if (rv) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(fbuf);
      return rv;
    }
    GWEN_Buffer_IncrementPos(fbuf, bs);
    GWEN_Buffer_AdjustUsedBytes(fbuf);
  }

  /* insert crypto tag and make it own the content of fbuf */
  GWEN_Buffer_Rewind(fbuf);
  bs=GWEN_Buffer_GetUsedBytes(fbuf);
  GWEN_Buffer_InsertBytes(fbuf, "000", 3);
  p=GWEN_Buffer_GetStart(fbuf);
  p[0]=(unsigned char)(lct->cryptoTag);
  p[1]=(unsigned char)(bs & 0xff);
  p[2]=(unsigned char)((bs>>8) & 0xff);

  /* for new-typ media create envelope */
  if (lct->mediumTag==GWEN_CRYPT_TOKEN_OHBCI_TAG_MEDIUM3) {
    char numbuf[16];
    GWEN_BUFFER *dbuf;

    /* this is a new medium type, create envelope */
    dbuf=GWEN_Buffer_new(0, 2048, 0, 1);
    /* prepare container tag */
    GWEN_Buffer_AppendBytes(dbuf, "000", 3);

    GWEN_Tag16_DirectlyToBuffer(GWEN_CRYPT_TOKEN_OHBCI_TAG_HEADER,
                                GWEN_CRYPT_TOKEN_OHBCI_NAME, -1, dbuf);
    snprintf(numbuf, sizeof(numbuf), "%d", GWEN_CRYPT_TOKEN_OHBCI_VMAJOR);
    GWEN_Tag16_DirectlyToBuffer(GWEN_CRYPT_TOKEN_OHBCI_TAG_VERSION_MAJOR,
                                numbuf, -1, dbuf);

    snprintf(numbuf, sizeof(numbuf), "%d", GWEN_CRYPT_TOKEN_OHBCI_VMINOR);
    GWEN_Tag16_DirectlyToBuffer(GWEN_CRYPT_TOKEN_OHBCI_TAG_VERSION_MINOR,
                                numbuf, -1, dbuf);
    /* write complete medium into new tag */
    GWEN_Buffer_AppendBytes(dbuf,
                            GWEN_Buffer_GetStart(fbuf),
                            GWEN_Buffer_GetUsedBytes(fbuf));
    p=GWEN_Buffer_GetStart(dbuf);
    bs=GWEN_Buffer_GetUsedBytes(dbuf)-3; /* subtract medium tag bytes */
    p[0]=(unsigned char)(lct->mediumTag);
    p[1]=(unsigned char)(bs & 0xff);
    p[2]=(unsigned char)((bs>>8) & 0xff);
    /* swap buffers */
    GWEN_Buffer_free(fbuf);
    fbuf=dbuf;
    GWEN_Buffer_Rewind(fbuf);
  }

  if (ftruncate(fd, 0)==-1) {
    DBG_ERROR(GWEN_LOGDOMAIN,
	      "ftruncate(%s): %s",
	      GWEN_Crypt_Token_GetTokenName(ct),
	      strerror(errno));
    GWEN_Buffer_free(fbuf);
    return GWEN_ERROR_GENERIC;
  }

  p=GWEN_Buffer_GetStart(fbuf);
  bs=GWEN_Buffer_GetUsedBytes(fbuf);
  while(bs) {
    ssize_t rv;

    rv=write(fd, p, bs);
    if (rv==-1) {
      if (errno!=EINTR) {
	DBG_ERROR(GWEN_LOGDOMAIN,
		  "write(%s): %s",
		  GWEN_Crypt_Token_GetTokenName(ct),
		  strerror(errno));
	GWEN_Buffer_free(fbuf);
	return GWEN_ERROR_GENERIC;
      }
    }
    else if (rv==0)
      break;
    else {
      p+=rv;
      bs-=rv;
    }
  } /* while */

  GWEN_Buffer_free(fbuf);

  lct->justCreated=0;

  return 0;
}



int GWENHYWFAR_CB 
GWEN_Crypt_TokenOHBCI_ChangePin(GWEN_CRYPT_TOKEN *ct,
				int admin,
				uint32_t gid) {
  GWEN_CRYPT_TOKEN_OHBCI *lct;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, GWEN_CRYPT_TOKEN_OHBCI, ct);
  assert(lct);

  /* just reset the password so it will be asked for upon write */
  lct->passWordIsSet=0;
  memset(lct->password, 0, sizeof(lct->password));

  return 0;
}



int GWENHYWFAR_CB 
GWEN_Crypt_TokenOHBCI_Create(GWEN_CRYPT_TOKEN *ct, uint32_t gid){
  GWEN_CRYPT_TOKEN_OHBCI *lct;
  GWEN_CRYPT_TOKEN_CONTEXT *fct;
  GWEN_CRYPT_TOKEN_KEYINFO *ki;
  int rv;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, GWEN_CRYPT_TOKEN_OHBCI, ct);
  assert(lct);

  lct->justCreated=1;

  fct=GWEN_CTF_Context_new();

  /* create user and file context */
  GWEN_Crypt_Token_Context_SetId(fct, 1);            /* only one user */
  GWEN_Crypt_Token_Context_SetSignKeyId(fct, 0x01);
  GWEN_Crypt_Token_Context_SetDecipherKeyId(fct, 0x02);
  GWEN_Crypt_Token_Context_SetVerifyKeyId(fct, 0x03);
  GWEN_Crypt_Token_Context_SetEncipherKeyId(fct, 0x04);
  GWEN_Crypt_Token_Context_SetAuthSignKeyId(fct, 0x05);
  GWEN_Crypt_Token_Context_SetAuthVerifyKeyId(fct, 0x06);

  /* create key info */
  ki=GWEN_Crypt_Token_KeyInfo_new(0x0001, /* local sign key */
				  GWEN_Crypt_CryptAlgoId_Rsa,
				  96);
  assert(ki);
  GWEN_Crypt_Token_KeyInfo_SetKeyDescr(ki, I18N("Local Sign Key"));

  GWEN_Crypt_Token_KeyInfo_AddFlags(ki,
				    GWEN_CRYPT_TOKEN_KEYFLAGS_HASSIGNCOUNTER |
				    GWEN_CRYPT_TOKEN_KEYFLAGS_HASACTIONFLAGS |
				    GWEN_CRYPT_TOKEN_KEYFLAGS_CANVERIFY |
				    GWEN_CRYPT_TOKEN_KEYFLAGS_CANSIGN |
				    GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
				    GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER);
  GWEN_CTF_Context_SetLocalSignKeyInfo(fct, ki);

  /* create key info */
  ki=GWEN_Crypt_Token_KeyInfo_new(0x0002, /* local crypt key */
				  GWEN_Crypt_CryptAlgoId_Rsa,
                                  96);
  assert(ki);
  GWEN_Crypt_Token_KeyInfo_SetKeyDescr(ki, I18N("Local Crypt Key"));
  GWEN_Crypt_Token_KeyInfo_AddFlags(ki,
				    GWEN_CRYPT_TOKEN_KEYFLAGS_HASACTIONFLAGS |
				    GWEN_CRYPT_TOKEN_KEYFLAGS_CANENCIPHER |
				    GWEN_CRYPT_TOKEN_KEYFLAGS_CANDECIPHER |
				    GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
				    GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER);
  GWEN_CTF_Context_SetLocalCryptKeyInfo(fct, ki);

  /* create key info */
  ki=GWEN_Crypt_Token_KeyInfo_new(0x0003, /* remote sign key */
				  GWEN_Crypt_CryptAlgoId_Rsa,
                                  96);
  assert(ki);
  GWEN_Crypt_Token_KeyInfo_SetKeyDescr(ki, I18N("Remote Sign Key"));

  GWEN_Crypt_Token_KeyInfo_AddFlags(ki,
				    GWEN_CRYPT_TOKEN_KEYFLAGS_HASSIGNCOUNTER |
				    GWEN_CRYPT_TOKEN_KEYFLAGS_HASACTIONFLAGS |
				    GWEN_CRYPT_TOKEN_KEYFLAGS_CANVERIFY |
				    GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
				    GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER);
  GWEN_CTF_Context_SetRemoteSignKeyInfo(fct, ki);

  /* create key info */
  ki=GWEN_Crypt_Token_KeyInfo_new(0x0004, /* remote crypt key */
				  GWEN_Crypt_CryptAlgoId_Rsa,
                                  96);
  assert(ki);
  GWEN_Crypt_Token_KeyInfo_SetKeyDescr(ki, I18N("Remote Crypt Key"));

  GWEN_Crypt_Token_KeyInfo_AddFlags(ki,
				    GWEN_CRYPT_TOKEN_KEYFLAGS_HASACTIONFLAGS |
				    GWEN_CRYPT_TOKEN_KEYFLAGS_CANENCIPHER |
				    GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
				    GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER);
  GWEN_CTF_Context_SetRemoteCryptKeyInfo(fct, ki);

  /* create key info */
  ki=GWEN_Crypt_Token_KeyInfo_new(0x0005, /* local auth key */
				  GWEN_Crypt_CryptAlgoId_Rsa,
                                  96);
  assert(ki);
  GWEN_Crypt_Token_KeyInfo_AddFlags(ki,
				    GWEN_CRYPT_TOKEN_KEYFLAGS_HASACTIONFLAGS |
				    GWEN_CRYPT_TOKEN_KEYFLAGS_CANSIGN |
				    GWEN_CRYPT_TOKEN_KEYFLAGS_CANVERIFY |
				    GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
				    GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER);
  GWEN_CTF_Context_SetLocalAuthKeyInfo(fct, ki);
  GWEN_Crypt_Token_KeyInfo_SetKeyDescr(ki, I18N("Local Auth Key"));

  /* create key info */
  ki=GWEN_Crypt_Token_KeyInfo_new(0x0006, /* remote auth key */
				  GWEN_Crypt_CryptAlgoId_Rsa,
				  96);
  assert(ki);
  GWEN_Crypt_Token_KeyInfo_SetKeyDescr(ki, I18N("Remote Auth Key"));

  GWEN_Crypt_Token_KeyInfo_AddFlags(ki,
				    GWEN_CRYPT_TOKEN_KEYFLAGS_HASACTIONFLAGS |
				    GWEN_CRYPT_TOKEN_KEYFLAGS_CANVERIFY |
				    GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
				    GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER);
  GWEN_CTF_Context_SetRemoteAuthKeyInfo(fct, ki);

  /* add context */
  GWEN_Crypt_TokenFile_AddContext(ct, fct);

  assert(lct->createFn);
  rv=lct->createFn(ct, gid);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int GWENHYWFAR_CB 
GWEN_Crypt_TokenOHBCI_Open(GWEN_CRYPT_TOKEN *ct, int manage, uint32_t gid){
  GWEN_CRYPT_TOKEN_OHBCI *lct;
  int rv;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, GWEN_CRYPT_TOKEN_OHBCI, ct);
  assert(lct);

  assert(lct->openFn);
  rv=lct->openFn(ct, manage, gid);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    return rv;
  }

  return 0;
}



int GWENHYWFAR_CB 
GWEN_Crypt_TokenOHBCI_Close(GWEN_CRYPT_TOKEN *ct, int abandon, uint32_t gid){
  GWEN_CRYPT_TOKEN_OHBCI *lct;
  int rv;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, GWEN_CRYPT_TOKEN_OHBCI, ct);
  assert(lct);

  assert(lct->closeFn);
  rv=lct->closeFn(ct, abandon, gid);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    return rv;
  }

  memset(lct->password, 0, sizeof(lct->password));
  lct->passWordIsSet=0;

  return 0;
}



