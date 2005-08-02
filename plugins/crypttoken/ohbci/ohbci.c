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


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/*#define DEBUG_OHBCI_MODULE */



#include "ohbci_p.h"
#include "i18n_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/padd.h>
#include <gwenhywfar/md.h>
#include <gwenhywfar/waitcallback.h>
#include <gwenhywfar/ct_file.h>

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


GWEN_INHERIT(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_OHBCI)



GWEN_PLUGIN *crypttoken_ohbci_factory(GWEN_PLUGIN_MANAGER *pm,
				      const char *modName,
				      const char *fileName) {
  GWEN_PLUGIN *pl;

  pl=GWEN_CryptTokenOHBCI_Plugin_new(pm, modName, fileName);
  assert(pl);

  return pl;
}



GWEN_PLUGIN *GWEN_CryptTokenOHBCI_Plugin_new(GWEN_PLUGIN_MANAGER *pm,
					     const char *modName,
					     const char *fileName) {
  GWEN_PLUGIN *pl;

  pl=GWEN_CryptToken_Plugin_new(pm,
				GWEN_CryptToken_Device_File,
				modName,
				fileName);

  /* set virtual functions */
  GWEN_CryptToken_Plugin_SetCreateTokenFn(pl,
					  GWEN_CryptTokenOHBCI_Plugin_CreateToken);
  GWEN_CryptToken_Plugin_SetCheckTokenFn(pl,
					 GWEN_CryptTokenOHBCI_Plugin_CheckToken);

  return pl;
}



GWEN_CRYPTTOKEN *GWEN_CryptTokenOHBCI_Plugin_CreateToken(GWEN_PLUGIN *pl,
							 const char *subTypeName,
							 const char *name) {
  GWEN_PLUGIN_MANAGER *pm;
  GWEN_CRYPTTOKEN *ct;

  assert(pl);

  pm=GWEN_Plugin_GetManager(pl);
  assert(pm);

  ct=GWEN_CryptTokenOHBCI_new(pm, subTypeName, name);
  assert(ct);

  return ct;
}



int GWEN_CryptTokenOHBCI_Plugin_CheckToken(GWEN_PLUGIN *pl,
					   GWEN_BUFFER *subTypeName,
					   GWEN_BUFFER *name) {
  FILE *f;
  const char *p;
  char buffer[16];
  int rv;

  if (GWEN_Buffer_GetUsedBytes(name)==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Empty name");
    return GWEN_ERROR_CT_BAD_NAME;
  }

  p=GWEN_Buffer_GetStart(name);
  if (access(p, F_OK)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "File does not exist");
    GWEN_WaitCallback_Log(GWEN_LoggerLevelInfo, "File does not exist");
    return GWEN_ERROR_CT_BAD_NAME;
  }

  if (access(p, R_OK | W_OK)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "File exists but I have no writes on it");
    GWEN_WaitCallback_Log(GWEN_LoggerLevelInfo,
                          "File exists but I have no writes on it");
    return GWEN_ERROR_CT_IO_ERROR;
  }

  f=fopen(p, "rb");
  if (!f) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "File exists, I have all rights but still can't open it");
    GWEN_WaitCallback_Log(GWEN_LoggerLevelNotice,
                          "File exists, I have all rights but "
                          "still can't open it");
    return GWEN_ERROR_CT_IO_ERROR;
  }

  rv=fread(buffer, sizeof(buffer), 1, f);
  fclose(f);
  if (rv!=1) {
    DBG_INFO(GWEN_LOGDOMAIN, "This seems not to be an OpenHBCI keyfile");
    GWEN_WaitCallback_Log(GWEN_LoggerLevelInfo,
                          "This seems not to be an OpenHBCI keyfile");
    return GWEN_ERROR_CT_NOT_SUPPORTED;
  }

  if (rv!=1) {
    DBG_INFO(GWEN_LOGDOMAIN, "This seems not to be an OpenHBCI keyfile (bad size)");
    GWEN_WaitCallback_Log(GWEN_LoggerLevelNotice,
                          "This seems not to be an OpenHBCI keyfile "
                          "(bad size)");
    return GWEN_ERROR_CT_NOT_SUPPORTED;
  }

  if ((unsigned char)(buffer[0])==GWEN_CRYPTTOKEN_OHBCI_TAG_MEDIUM1) {
    DBG_INFO(GWEN_LOGDOMAIN,
             "Old OpenHBCI file detected");
    GWEN_WaitCallback_Log(GWEN_LoggerLevelNotice,
                          "Old OpenHBCI file detected");
    return 0;
  }
  else if ((unsigned char)(buffer[0])==GWEN_CRYPTTOKEN_OHBCI_TAG_MEDIUM2) {
    DBG_INFO(GWEN_LOGDOMAIN,
             "OpenHBCI file (<1.6) detected");
    GWEN_WaitCallback_Log(GWEN_LoggerLevelNotice,
             "OpenHBCI file (<1.6) detected");
    return 0;
  }
  else if ((unsigned char)(buffer[0])==GWEN_CRYPTTOKEN_OHBCI_TAG_MEDIUM3) {
    if ((unsigned char)(buffer[3])==GWEN_CRYPTTOKEN_OHBCI_TAG_HEADER &&
        strncmp(buffer+6,
                GWEN_CRYPTTOKEN_OHBCI_NAME,
                strlen(GWEN_CRYPTTOKEN_OHBCI_NAME))==0) {
      DBG_INFO(GWEN_LOGDOMAIN,
               "New OpenHBCI file (>=1.6) detected");
      GWEN_WaitCallback_Log(GWEN_LoggerLevelNotice,
                            "New OpenHBCI file (>=1.6) detected");
      return 0;
    }
  }

  DBG_INFO(GWEN_LOGDOMAIN,
           "This seems not to be an OpenHBCI keyfile");
  GWEN_WaitCallback_Log(GWEN_LoggerLevelNotice,
                        "This seems not to be an OpenHBCI keyfile");
  return GWEN_ERROR_CT_NOT_SUPPORTED;
}





GWEN_CRYPTTOKEN *GWEN_CryptTokenOHBCI_new(GWEN_PLUGIN_MANAGER *pm,
                                          const char *subTypeName,
                                          const char *name){
  GWEN_CRYPTTOKEN *ct;
  GWEN_CRYPTTOKEN_OHBCI *lct;

  ct=GWEN_CryptTokenFile_new(pm, "ohbci", subTypeName, name);

  GWEN_NEW_OBJECT(GWEN_CRYPTTOKEN_OHBCI, lct);
  GWEN_INHERIT_SETDATA(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_OHBCI,
                       ct, lct,
                       GWEN_CryptTokenOHBCI_FreeData);
  lct->mediumTag=GWEN_CRYPTTOKEN_OHBCI_TAG_MEDIUM3;
  lct->vminor=GWEN_CRYPTTOKEN_OHBCI_VMINOR;
  lct->cryptoTag=GWEN_CRYPTTOKEN_OHBCI_TAG_CRYPT_BF;

  /* get virtual functions */
  lct->openFn=GWEN_CryptToken_GetOpenFn(ct);
  lct->createFn=GWEN_CryptToken_GetCreateFn(ct);
  lct->closeFn=GWEN_CryptToken_GetCloseFn(ct);

  /* set virtual functions */
  GWEN_CryptToken_SetOpenFn(ct, GWEN_CryptTokenOHBCI_Open);
  GWEN_CryptToken_SetCloseFn(ct, GWEN_CryptTokenOHBCI_Close);
  GWEN_CryptToken_SetCreateFn(ct, GWEN_CryptTokenOHBCI_Create);
  GWEN_CryptToken_SetChangePinFn(ct, GWEN_CryptTokenOHBCI_ChangePin);

  GWEN_CryptTokenFile_SetReadFn(ct, GWEN_CryptTokenOHBCI_Read);
  GWEN_CryptTokenFile_SetWriteFn(ct, GWEN_CryptTokenOHBCI_Write);

  return ct;
}



void GWEN_CryptTokenOHBCI_FreeData(void *bp, void *p) {
  GWEN_CRYPTTOKEN_OHBCI *lct;

  lct=(GWEN_CRYPTTOKEN_OHBCI*) p;
  memset(lct->password, 0, sizeof(lct->password));
  GWEN_FREE_OBJECT(lct);
}



int GWEN_CryptTokenOHBCI__DecryptFile(GWEN_CRYPTTOKEN *ct,
                                      GWEN_BUFFER *fbuf,
                                      int trynum){
  GWEN_CRYPTTOKEN_OHBCI *lct;
  GWEN_CRYPTKEY *key;
  GWEN_ERRORCODE err;
  unsigned char password_unsigned[64];
  char password[64];
  GWEN_BUFFER *rawbuf;
  int rv;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_OHBCI, ct);
  assert(lct);

  if (lct->passWordIsSet==0) {
    GWEN_PLUGIN_MANAGER *pm;
    int mres;
    unsigned int pinLength;
    unsigned int k;

    pm=GWEN_CryptToken_GetCryptManager(ct);
    assert(pm);

    /* create key from password */
    memset(lct->password, 0, sizeof(lct->password));

    mres=GWEN_CryptManager_GetPin(pm,
				  ct,
				  GWEN_CryptToken_PinType_Access,
				  GWEN_CryptToken_PinEncoding_ASCII,
				  (trynum?GWEN_CRYPTTOKEN_GETPIN_FLAGS_RETRY:0),
				  password_unsigned,
				  GWEN_CRYPTTOKEN_OHBCI_PINMINLENGTH,
				  sizeof(password_unsigned)-1,
				  &pinLength);
    if (mres) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error asking for PIN, aborting");
      return mres;
    }

    /* Convert the 'unsigned char' buffer to the 'char' buffer */
    for (k=0; k < pinLength; ++k)
      password[k] = password_unsigned[k];
    password[k] = '\0';
    memset(password_unsigned, '\0', pinLength);

    if (strlen(password)<GWEN_CRYPTTOKEN_OHBCI_PINMINLENGTH) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Your program returned a shorter PIN than instructed!");
      return GWEN_ERROR_GENERIC;
    }

    DBG_NOTICE(GWEN_LOGDOMAIN, "Checking...");
    if (lct->cryptoTag==GWEN_CRYPTTOKEN_OHBCI_TAG_CRYPT) {
      DBG_NOTICE(GWEN_LOGDOMAIN, "New OpenHBCI file detected");
      if (GWEN_CryptKey_FromPassword(password,
                                     (unsigned char*)lct->password, sizeof(lct->password))) {
	DBG_ERROR(GWEN_LOGDOMAIN,
		  "Could not create key data from password");
        return GWEN_ERROR_GENERIC;
      }
    }
    else if (lct->cryptoTag==GWEN_CRYPTTOKEN_OHBCI_TAG_CRYPT_OLD) {
      DBG_NOTICE(GWEN_LOGDOMAIN, "Old OpenHBCI file detected");
      if (GWEN_CryptKey_FromPasswordSSL(password,
                                        (unsigned char*)lct->password,
                                        sizeof(lct->password))) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Could not create key data from password");
        return GWEN_ERROR_GENERIC;
      }
    }
    else if (lct->cryptoTag==GWEN_CRYPTTOKEN_OHBCI_TAG_CRYPT_BF) {
      DBG_NOTICE(GWEN_LOGDOMAIN, "New OpenHBCI (1.6+) file detected");
      if (GWEN_CryptKey_FromPassword(password,
                                     (unsigned char*)lct->password,
                                     sizeof(lct->password))) {
        DBG_ERROR(GWEN_LOGDOMAIN,
                  "Could not create key data from password");
        return GWEN_ERROR_GENERIC;
      }
    }
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected crypto tag %d",
                lct->cryptoTag);
      abort();
    }

    lct->passWordIsSet=1;
  }

  if (lct->cryptoTag==GWEN_CRYPTTOKEN_OHBCI_TAG_CRYPT ||
      lct->cryptoTag==GWEN_CRYPTTOKEN_OHBCI_TAG_CRYPT_OLD) {
    key=GWEN_CryptKey_Factory("DES");
  }
  else if (lct->cryptoTag==GWEN_CRYPTTOKEN_OHBCI_TAG_CRYPT_BF) {
    key=GWEN_CryptKey_Factory("BF");
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Unknown crypt tag, should not occur");
    abort();
  }

  if (!key) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not create key");
    memset(password, 0, sizeof(password));
    lct->passWordIsSet=0;
    return GWEN_ERROR_GENERIC;
  }

  err=GWEN_CryptKey_SetData(key, lct->password, sizeof(lct->password));
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(GWEN_LOGDOMAIN, err);
    GWEN_CryptKey_free(key);
    lct->passWordIsSet=0;
    return GWEN_ERROR_GENERIC;
  }

  /* decrypt file */
  DBG_INFO(GWEN_LOGDOMAIN, "Decrypting file");
  rawbuf=GWEN_Buffer_new(0, 1024, 0, 1);
  GWEN_Buffer_Rewind(fbuf);
  err=GWEN_CryptKey_Decrypt(key, fbuf, rawbuf);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(GWEN_LOGDOMAIN, err);
    GWEN_Buffer_free(rawbuf);
    GWEN_CryptKey_free(key);
    memset(password, 0, sizeof(password));
    lct->passWordIsSet=0;
    return GWEN_ERROR_CT_BAD_PIN;
  }

  /* unpadd raw data */
  DBG_INFO(GWEN_LOGDOMAIN, "Unpadding file");
  if (GWEN_Padd_UnpaddWithANSIX9_23(rawbuf)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not unpadd keyfile, i.e. wrong PIN");
    GWEN_Buffer_free(rawbuf);
    GWEN_CryptKey_free(key);
    /* TODO: Set Pin status */
    memset(password, 0, sizeof(password));
    lct->passWordIsSet=0;
    return GWEN_ERROR_CT_BAD_PIN;
  }
  GWEN_CryptKey_free(key);

  /* parse raw data */
  DBG_INFO(GWEN_LOGDOMAIN, "Parsing file");
  GWEN_Buffer_Rewind(rawbuf);

  rv=GWEN_CryptTokenOHBCI__Decode(ct, rawbuf);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(rawbuf);
    /* TODO: Set Pin status */
    memset(password, 0, sizeof(password));
    lct->passWordIsSet=0;
    return GWEN_ERROR_CT_BAD_PIN;
  }
  GWEN_Buffer_free(rawbuf);
  /* TODO: Set Pin status */
  return 0;
}



int GWEN_CryptTokenOHBCI_Read(GWEN_CRYPTTOKEN *ct, int fd){
  GWEN_CRYPTTOKEN_OHBCI *lct;
  GWEN_BUFFER *rbuf;
  GWEN_BUFFER *fbuf;
  unsigned char c;
  GWEN_TAG16 *tlv;
  int i;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_OHBCI, ct);
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
  if (c!=GWEN_CRYPTTOKEN_OHBCI_TAG_MEDIUM1 &&
      c!=GWEN_CRYPTTOKEN_OHBCI_TAG_MEDIUM2 &&
      c!=GWEN_CRYPTTOKEN_OHBCI_TAG_MEDIUM3) {
    DBG_ERROR(GWEN_LOGDOMAIN, "This seems not to be an OpenHBCI key file");
    GWEN_Buffer_free(rbuf);
    return -1;
  }
  lct->mediumTag=c;
  lct->cryptoTag=0;

  tlv=GWEN_TAG16_fromBuffer(rbuf, 0);
  if (!tlv) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Bad file data");
    GWEN_Buffer_free(rbuf);
    return -1;
  }

  fbuf=GWEN_Buffer_new(0, GWEN_TAG16_GetTagLength(tlv), 0, 1);
  GWEN_Buffer_AppendBytes(fbuf,
                          GWEN_TAG16_GetTagData(tlv),
                          GWEN_TAG16_GetTagLength(tlv));
  GWEN_Buffer_Rewind(fbuf);
  GWEN_Buffer_free(rbuf);
  GWEN_TAG16_free(tlv);
  /* now fbuf contains the data from the crypt TLV */


  for (i=0;;i++) {
    int rv;

    if (i>GWEN_CRYPTTOKEN_OHBCI_MAX_PIN_TRY) {
      DBG_ERROR(GWEN_LOGDOMAIN,
		"No valid PIN within %d tries, giving up", i);
      GWEN_WaitCallback_Log(GWEN_LoggerLevelCritical,
                            I18N("No valid PIN (tried too often), "
				 "aborting."));
      GWEN_Buffer_free(fbuf);
      return GWEN_ERROR_ABORTED;
    }

    switch(lct->mediumTag) {
    case GWEN_CRYPTTOKEN_OHBCI_TAG_MEDIUM1:
      lct->cryptoTag=GWEN_CRYPTTOKEN_OHBCI_TAG_CRYPT_OLD;
      rv=GWEN_CryptTokenOHBCI__DecryptFile(ct, fbuf, i);
      break;
    case GWEN_CRYPTTOKEN_OHBCI_TAG_MEDIUM2:
      lct->cryptoTag=GWEN_CRYPTTOKEN_OHBCI_TAG_CRYPT;
      rv=GWEN_CryptTokenOHBCI__DecryptFile(ct, fbuf, i);
      break;
    case GWEN_CRYPTTOKEN_OHBCI_TAG_MEDIUM3:
      lct->cryptoTag=0;
      rv=GWEN_CryptTokenOHBCI__DecryptFile16(ct, fbuf, i);
      break;
    default:
      rv = GWEN_ERROR_CT_NOT_SUPPORTED;
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
      case GWEN_ERROR_CT_BAD_PIN:
        DBG_ERROR(GWEN_LOGDOMAIN, "Bad pin.");
	GWEN_WaitCallback_Log(GWEN_LoggerLevelCritical,
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



void GWEN_CryptTokenOHBCI__DecodeKey(GWEN_CRYPTTOKEN *ct,
                                     GWEN_TAG16 *keyTlv,
                                     GWEN_DB_NODE *dbKeys,
                                     const char *keyName) {
  GWEN_BUFFER *dbuf;
  const char *p;
  int size;
  GWEN_DB_NODE *node;
  GWEN_TYPE_UINT32 flags;
  const char defaultExpo[3]={0x01, 0x00, 0x01};

  p=GWEN_TAG16_GetTagData(keyTlv);
  size=GWEN_TAG16_GetTagLength(keyTlv);
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
                       "type",
                       "RSA");
  GWEN_DB_SetBinValue(node,
                      GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "data/e",
                      defaultExpo,
                      sizeof(defaultExpo));
  flags=0;
  if (GWEN_CryptToken_GetFlags(ct) &
      GWEN_CRYPTTOKEN_FLAGS_DISABLE_SMALLER_SIGNATURE)
    flags|=GWEN_CRYPT_FLAG_DISABLE_SMALLER_SIGNATURE;
  GWEN_DB_SetIntValue(node,
                      GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "flags",
                      flags);

  while(GWEN_Buffer_GetBytesLeft(dbuf)) {
    const char *pp;
    char *p;
    GWEN_TAG16 *tlv;
    unsigned int l;

    tlv=GWEN_TAG16_fromBuffer(dbuf, 0);
    if (!tlv) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Bad file (no TLV)");
      return;
    }
    p=0;
    pp=(const char*)GWEN_TAG16_GetTagData(tlv);
    l=GWEN_TAG16_GetTagLength(tlv);
    if (pp && l) {
      p=(char*)malloc(l+1);
      assert(p);
      memmove(p, pp, l);
      p[l]=0;
    }
    switch(GWEN_TAG16_GetTagType(tlv)) {
    case GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_ISPUBLIC:
      /* ignore this tag, since it is buggy in OpenHBCI(2) */
      break;

    case GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_ISCRYPT:
      assert(p);
      if (strcasecmp(p, "yes")==0)
        GWEN_DB_SetCharValue(node,
                             GWEN_DB_FLAGS_OVERWRITE_VARS,
                             "name",
                             "V");
      else
        GWEN_DB_SetCharValue(node,
                             GWEN_DB_FLAGS_OVERWRITE_VARS,
                             "name",
                             "S");
      break;

    case GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_OWNER: {
      if (p) {
        GWEN_BUFFER *obuf;
        const char *s;

        /* workaround for a bug in older OpenHBCI versions: here the escape
         * character "?" was falsely included for the owner name */
        obuf=GWEN_Buffer_new(0, 32, 0, 1);
        s=p;
        while(*s) {
          if (*s!='?')
            GWEN_Buffer_AppendByte(obuf, *s);
          s++;
        } /* while */
        GWEN_DB_SetCharValue(node,
                             GWEN_DB_FLAGS_OVERWRITE_VARS,
                             "owner",
                             GWEN_Buffer_GetStart(obuf));
        GWEN_Buffer_free(obuf);
      }
      break;
    }

    case GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_VERSION:
      assert(p);
      GWEN_DB_SetIntValue(node,
                          GWEN_DB_FLAGS_OVERWRITE_VARS,
                          "version",
                          atoi(p));
      break;

    case GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_NUMBER:
      assert(p);
      GWEN_DB_SetIntValue(node,
                          GWEN_DB_FLAGS_OVERWRITE_VARS,
                          "number",
                          atoi(p));
      break;

    case GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_MODULUS:
      if (p && l)
        GWEN_DB_SetBinValue(node,
                            GWEN_DB_FLAGS_OVERWRITE_VARS,
                            "data/n",
                            p, l);
      break;

    case GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_EXP_OLD:
      DBG_INFO(GWEN_LOGDOMAIN,
               "Ignoring old exponent (%d), keeping default", l);
      break;

    case GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_EXP:
      if (p && l)
        GWEN_DB_SetBinValue(node,
                            GWEN_DB_FLAGS_OVERWRITE_VARS,
                            "data/e",
                            p, l);
      break;

    case GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_N:
      if (p && l)
        GWEN_DB_SetBinValue(node,
                            GWEN_DB_FLAGS_OVERWRITE_VARS,
                            "data/n",
                            p, l);
      break;

    case GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_P:
      if (p && l)
        GWEN_DB_SetBinValue(node,
                            GWEN_DB_FLAGS_OVERWRITE_VARS,
                            "data/p",
                            p, l);
      break;

    case GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_Q:
      if (p && l)
        GWEN_DB_SetBinValue(node,
                            GWEN_DB_FLAGS_OVERWRITE_VARS,
                            "data/q",
                            p, l);
      break;

    case GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_D:
      if (p && l)
        GWEN_DB_SetBinValue(node,
                            GWEN_DB_FLAGS_OVERWRITE_VARS,
                            "data/d",
                            p, l);
      break;

    case GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_DMP1:
      if (p && l)
        GWEN_DB_SetBinValue(node,
                            GWEN_DB_FLAGS_OVERWRITE_VARS,
                            "data/dmp1",
                            p, l);
      break;

    case GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_DMQ1:
      if (p && l)
        GWEN_DB_SetBinValue(node,
                            GWEN_DB_FLAGS_OVERWRITE_VARS,
                            "data/dmq1",
                            p, l);
      break;

    case GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_IQMP:
      if (p && l)
        GWEN_DB_SetBinValue(node,
                            GWEN_DB_FLAGS_OVERWRITE_VARS,
                            "data/iqmp",
                            p, l);
      break;

    default:
      DBG_WARN(GWEN_LOGDOMAIN, "Unknown tag %02x", GWEN_TAG16_GetTagType(tlv));
      break;
    } /* switch */

    GWEN_TAG16_free(tlv);
    free(p);
  } /* while */
  GWEN_Buffer_free(dbuf);
}



int GWEN_CryptTokenOHBCI__Decode(GWEN_CRYPTTOKEN *ct, GWEN_BUFFER *dbuf) {
  GWEN_CRYPTTOKEN_OHBCI *lct;
  GWEN_TAG16 *tlv;
  int rv;
  GWEN_DB_NODE *dbKeys;
  GWEN_DB_NODE *dbKey;
  GWEN_CT_FILE_CONTEXT *fct;
  GWEN_CRYPTTOKEN_USER *user;
  const char *peerId=0;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_OHBCI, ct);
  assert(lct);

  tlv=GWEN_TAG16_fromBuffer(dbuf, 0);
  GWEN_Buffer_Rewind(dbuf);
  if (!tlv) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "File doesn't contain a TLV: Either bad pin or bad file");
    return -1;
  }

  if (GWEN_TAG16_GetTagType(tlv)!=GWEN_CRYPTTOKEN_OHBCI_TAG_HEADER &&
      GWEN_TAG16_GetTagType(tlv)!=GWEN_CRYPTTOKEN_OHBCI_TAG_VERSION_MAJOR) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "File doesn't start with version info or header.");
    GWEN_TAG16_free(tlv);
    return -1;
  }
  GWEN_TAG16_free(tlv);

  fct=GWEN_CryptTokenFile_Context_new();
  user=GWEN_CryptToken_User_new();

  /* now parse it */
  dbKeys=GWEN_DB_Group_new("keys");
  while(GWEN_Buffer_GetBytesLeft(dbuf)) {
    int i;
    const char *pp;
    char *p;
    unsigned int l;

    tlv=GWEN_TAG16_fromBuffer(dbuf, 0);
    if (!tlv) {
      DBG_ERROR(GWEN_LOGDOMAIN,
		"File doesn't contain a TLV: Either bad pin or bad file");
      GWEN_CryptTokenFile_Context_free(fct);
      GWEN_CryptToken_User_free(user);
      return GWEN_ERROR_CT_BAD_PIN;
    }
    p=0;
    pp=(const char*)GWEN_TAG16_GetTagData(tlv);
    l=GWEN_TAG16_GetTagLength(tlv);
    if (pp && l) {
      p=(char*)malloc(l+1);
      assert(p);
      memmove(p, pp, l);
      p[l]=0;
    }

    switch(GWEN_TAG16_GetTagType(tlv)) {
    case GWEN_CRYPTTOKEN_OHBCI_TAG_VERSION_MAJOR:
      assert(p);
      i=atoi(p);
      if (i!=GWEN_CRYPTTOKEN_OHBCI_VMAJOR) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Unsupported keyfile version (%d)", i);
        GWEN_WaitCallback_Log(0,
                              "Basically this file type is supported.\n"
                              "However, the major versions do not match,\n"
                              "so this particular version is not supported");
        free(p);
        GWEN_TAG16_free(tlv);
        GWEN_CryptTokenFile_Context_free(fct);
        GWEN_CryptToken_User_free(user);
        return -1;
      }
      break;

    case GWEN_CRYPTTOKEN_OHBCI_TAG_VERSION_MINOR:
      assert(p);
      i=atoi(p);
      if (i>GWEN_CRYPTTOKEN_OHBCI_VMINOR) {
        DBG_WARN(GWEN_LOGDOMAIN,
                 "Keyfile version is higher than mine (%d).\n",
                 i);
        GWEN_WaitCallback_Log(GWEN_LoggerLevelWarning,
                              "This key file file has been created with a "
                              "newer library version.\n");
        free(p);
        GWEN_TAG16_free(tlv);
        GWEN_CryptTokenFile_Context_free(fct);
        GWEN_CryptToken_User_free(user);
        return -1;
      }
      else if (i<GWEN_CRYPTTOKEN_OHBCI_VMINOR) {
        DBG_INFO(GWEN_LOGDOMAIN, "Will update this file upon unmount");
      }
      lct->vminor=i;
      break;

    case GWEN_CRYPTTOKEN_OHBCI_TAG_SEQ:
      assert(p);
      GWEN_CryptTokenFile_Context_SetLocalSignSeq(fct, atoi(p));
      break;

    case GWEN_CRYPTTOKEN_OHBCI_TAG_USER_ID:
      GWEN_CryptToken_User_SetUserId(user, p);
      break;

    case GWEN_CRYPTTOKEN_OHBCI_TAG_INST_COUNTRY:
      /* ignore */
      break;

    case GWEN_CRYPTTOKEN_OHBCI_TAG_INST_CODE:
      GWEN_CryptToken_User_SetServiceId(user, p);
      break;

    case GWEN_CRYPTTOKEN_OHBCI_TAG_INST_SYSTEMID:
      GWEN_CryptToken_User_SetSystemId(user, p);
      break;

    case GWEN_CRYPTTOKEN_OHBCI_TAG_SERVER_ADDR:
      /* new in 1.4 */
      GWEN_CryptToken_User_SetAddress(user, p);
      break;

    case GWEN_CRYPTTOKEN_OHBCI_TAG_SERVER_PORT:
      /* new in 1.4 */
      GWEN_CryptToken_User_SetPort(user, atoi(p));
      break;

    case GWEN_CRYPTTOKEN_OHBCI_TAG_REMOTE_SEQ:
      /* new in 1.4 */
      GWEN_CryptTokenFile_Context_SetRemoteSignSeq(fct, atoi(p));
      break;

    case GWEN_CRYPTTOKEN_OHBCI_TAG_USER_PUBSIGNKEY:
    case GWEN_CRYPTTOKEN_OHBCI_TAG_USER_PRIVSIGNKEY:
      GWEN_CryptTokenOHBCI__DecodeKey(ct, tlv, dbKeys, "localSignKey");
      break;

    case GWEN_CRYPTTOKEN_OHBCI_TAG_USER_PUBCRYPTKEY:
    case GWEN_CRYPTTOKEN_OHBCI_TAG_USER_PRIVCRYPTKEY:
      GWEN_CryptTokenOHBCI__DecodeKey(ct, tlv, dbKeys, "localCryptKey");
      break;

    case GWEN_CRYPTTOKEN_OHBCI_TAG_INST_PUBSIGNKEY:
      GWEN_CryptTokenOHBCI__DecodeKey(ct, tlv, dbKeys, "remoteSignKey");
      break;

    case GWEN_CRYPTTOKEN_OHBCI_TAG_INST_PUBCRYPTKEY:
      GWEN_CryptTokenOHBCI__DecodeKey(ct, tlv, dbKeys, "remoteCryptKey");
      break;

    case GWEN_CRYPTTOKEN_OHBCI_TAG_TEMP_PUBSIGNKEY:
    case GWEN_CRYPTTOKEN_OHBCI_TAG_TEMP_PRIVSIGNKEY:
      DBG_INFO(GWEN_LOGDOMAIN,
               "Ignoring temporary sign keys");
      break;

    case GWEN_CRYPTTOKEN_OHBCI_TAG_TEMP_PUBCRYPTKEY:
    case GWEN_CRYPTTOKEN_OHBCI_TAG_TEMP_PRIVCRYPTKEY:
      DBG_INFO(GWEN_LOGDOMAIN,
	       "Ignoring temporary crypt keys");
      break;
    default:
      DBG_WARN(GWEN_LOGDOMAIN, "Unknown tag %02x",
	       GWEN_TAG16_GetTagType(tlv));
      break;
    } /* switch */

    GWEN_TAG16_free(tlv);
    free(p);
  } /* while */


  /* now check for keys */
  rv=0;

  /* local sign key */
  dbKey=GWEN_DB_GetGroup(dbKeys, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                         "localSignKey");
  if (dbKey) {
    GWEN_CRYPTKEY *key;

    GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "data/public", 0);
    key=GWEN_CryptKey_FromDb(dbKey);
    if (!key) {
      rv=-1;
      DBG_ERROR(GWEN_LOGDOMAIN, "Bad key format");
    }
    else {
      GWEN_CryptTokenFile_Context_SetLocalSignKey(fct, key);
    }
  }

  /* local crypt key */
  dbKey=GWEN_DB_GetGroup(dbKeys, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                         "localCryptKey");
  if (dbKey) {
    GWEN_CRYPTKEY *key;

    GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "data/public", 0);
    key=GWEN_CryptKey_FromDb(dbKey);
    if (!key) {
      rv=-1;
      DBG_ERROR(GWEN_LOGDOMAIN, "Bad key format");
    }
    else {
      GWEN_CryptTokenFile_Context_SetLocalCryptKey(fct, key);
    }
  }

  /* remote sign key */
  dbKey=GWEN_DB_GetGroup(dbKeys, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
			 "remoteSignKey");
  if (dbKey) {
    GWEN_CRYPTKEY *key;

    GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "data/public", 1);
    key=GWEN_CryptKey_FromDb(dbKey);
    if (!key) {
      rv=-1;
      DBG_ERROR(GWEN_LOGDOMAIN, "Bad key format");
    }
    else {
      if (peerId==0)
        peerId=GWEN_CryptKey_GetOwner(key);
      GWEN_CryptTokenFile_Context_SetRemoteSignKey(fct, key);
    }
  }

  /* remote crypt key */
  dbKey=GWEN_DB_GetGroup(dbKeys, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                         "remoteCryptKey");
  if (dbKey) {
    GWEN_CRYPTKEY *key;

    GWEN_DB_SetIntValue(dbKey, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "data/public", 1);
    key=GWEN_CryptKey_FromDb(dbKey);
    if (!key) {
      rv=-1;
      DBG_ERROR(GWEN_LOGDOMAIN, "Bad key format");
    }
    else {
      if (peerId==0)
        peerId=GWEN_CryptKey_GetOwner(key);
      GWEN_CryptTokenFile_Context_SetRemoteCryptKey(fct, key);
    }
  }

  /* finalize user and file context */
  GWEN_CryptToken_User_SetId(user, 1);            /* only one user */
  GWEN_CryptToken_User_SetContextId(user, 1);     /* only one context */
  GWEN_CryptToken_User_SetPeerId(user, peerId); /* only one user */
  GWEN_CryptTokenFile_Context_SetUser(fct, user);

  /* clear context list, add new context */
  GWEN_CryptTokenFile_ClearFileContextList(ct);
  GWEN_CryptTokenFile_AddFileContext(ct, fct);

  GWEN_DB_Group_free(dbKeys);
  return rv;
}



int GWEN_CryptTokenOHBCI__DecryptFile16(GWEN_CRYPTTOKEN *ct,
                                        GWEN_BUFFER *dbuf,
                                        int tryNum) {
  GWEN_CRYPTTOKEN_OHBCI *lct;
  GWEN_TAG16 *tlv;
  int rv;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_OHBCI, ct);
  assert(lct);

  tlv=GWEN_TAG16_fromBuffer(dbuf, 0);
  GWEN_Buffer_Rewind(dbuf);
  if (!tlv) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "File doesn't contain a TLV: Either bad pin or bad file");
    return GWEN_ERROR_CT_BAD_PIN;
  }

  if (GWEN_TAG16_GetTagType(tlv)!=GWEN_CRYPTTOKEN_OHBCI_TAG_HEADER) {
    DBG_ERROR(GWEN_LOGDOMAIN, "File doesn't start with header tag.");
    GWEN_TAG16_free(tlv);
    return GWEN_ERROR_GENERIC;
  }
  GWEN_TAG16_free(tlv);

  /* now parse it */
  while(GWEN_Buffer_GetBytesLeft(dbuf)) {
    int i;
    const char *pp;
    char *p;
    unsigned int l;

    tlv=GWEN_TAG16_fromBuffer(dbuf, 0);
    if (!tlv) {
      DBG_ERROR(GWEN_LOGDOMAIN,
		"File doesn't contain a TLV: Either bad pin or bad file");
      return GWEN_ERROR_CT_BAD_PIN;
    }
    p=0;
    pp=(const char*)GWEN_TAG16_GetTagData(tlv);
    l=GWEN_TAG16_GetTagLength(tlv);
    if (pp && l) {
      p=(char*)malloc(l+1);
      assert(p);
      memmove(p, pp, l);
      p[l]=0;
    }

    switch(GWEN_TAG16_GetTagType(tlv)) {
    case GWEN_CRYPTTOKEN_OHBCI_TAG_HEADER:
      if (strcasecmp(p, GWEN_CRYPTTOKEN_OHBCI_NAME)!=0) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Bad header (%s)", p);
        free(p);
        GWEN_TAG16_free(tlv);
        return -1;
      }
      break;

    case GWEN_CRYPTTOKEN_OHBCI_TAG_VERSION_MAJOR:
      i=atoi(p);
      if (i!=GWEN_CRYPTTOKEN_OHBCI_VMAJOR) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Unsupported keyfile version (%d)", i);
        GWEN_WaitCallback_Log(0,
                              "Basically this file type is supported.\n"
                              "However, the major versions do not match,\n"
                              "so this particular version is not supported");
        free(p);
        GWEN_TAG16_free(tlv);
        return -1;
      }
      break;

    case GWEN_CRYPTTOKEN_OHBCI_TAG_VERSION_MINOR:
      i=atoi(p);
      if (i>GWEN_CRYPTTOKEN_OHBCI_VMINOR) {
        DBG_WARN(GWEN_LOGDOMAIN,
                 "Keyfile version is higher than mine (%d).\n",
                 i);
        GWEN_WaitCallback_Log(GWEN_LoggerLevelWarning,
                              "This key file file has been created with a "
                              "newer library version.\n");
        free(p);
        GWEN_TAG16_free(tlv);
        return -1;
      }
      else if (i<GWEN_CRYPTTOKEN_OHBCI_VMINOR) {
        DBG_INFO(GWEN_LOGDOMAIN,
                 "Will update this file upon unmount (%d)", i);
      }
      lct->vminor=i;
      break;

    case GWEN_CRYPTTOKEN_OHBCI_TAG_CRYPT_OLD:
    case GWEN_CRYPTTOKEN_OHBCI_TAG_CRYPT:
    case GWEN_CRYPTTOKEN_OHBCI_TAG_CRYPT_BF:
      {
        GWEN_BUFFER *fbuf;

        lct->cryptoTag=GWEN_TAG16_GetTagType(tlv);
        fbuf=GWEN_Buffer_new(0, GWEN_TAG16_GetTagLength(tlv), 0, 1);
        GWEN_Buffer_AppendBytes(fbuf,
                                GWEN_TAG16_GetTagData(tlv),
                                GWEN_TAG16_GetTagLength(tlv));
        GWEN_Buffer_Rewind(fbuf);
        rv=GWEN_CryptTokenOHBCI__DecryptFile(ct, fbuf, tryNum);
        GWEN_Buffer_free(fbuf);
        if (rv) {
          free(p);
          GWEN_TAG16_free(tlv);
          return rv;
        }
        break;
      }

    default:
      DBG_WARN(GWEN_LOGDOMAIN, "Unknown tag %02x",
	       GWEN_TAG16_GetTagType(tlv));
      break;
    } /* switch */

    GWEN_TAG16_free(tlv);
    free(p);
  } /* while */

  return 0;
}



int GWEN_CryptTokenOHBCI__EncodeKey(const GWEN_CRYPTKEY *key,
				    unsigned int tagType,
				    int wantPublic,
				    int isCrypt,
				    GWEN_BUFFER *dbuf) {
  GWEN_DB_NODE *dbKey;
  GWEN_ERRORCODE err;
  const void *p;
  unsigned int bs;
  const char *s;
  char *pp;
  GWEN_TYPE_UINT32 pos;
  char numbuf[16];

  if (!key) {
    DBG_INFO(GWEN_LOGDOMAIN, "No key");
    return 0;
  }
  dbKey=GWEN_DB_Group_new("key");
  err=GWEN_CryptKey_ToDb(key, dbKey, wantPublic);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
    GWEN_DB_Group_free(dbKey);
    return -1;
  }

  GWEN_Buffer_AppendByte(dbuf, tagType & 0xff);
  /* remember pos to insert size later */
  pos=GWEN_Buffer_GetPos(dbuf);
  GWEN_Buffer_AppendBytes(dbuf, "00", 2);

  /* always write "NO" for "isPublic", since OpenHBCI always writes "NO"
   * due to a bug */
  GWEN_TAG16_DirectlyToBuffer(GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_ISPUBLIC,
                             "NO",
                             -1,
                             dbuf);
  GWEN_TAG16_DirectlyToBuffer(GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_ISCRYPT,
                             isCrypt?"YES":"NO",
                             -1,
                             dbuf);
  s=GWEN_CryptKey_GetOwner(key);
  if (s && *s)
    GWEN_TAG16_DirectlyToBuffer(GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_OWNER,
                                s,
                                -1,
                                dbuf);

  snprintf(numbuf, sizeof(numbuf), "%d", GWEN_CryptKey_GetNumber(key));
  GWEN_TAG16_DirectlyToBuffer(GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_NUMBER,
                             numbuf,
                             -1,
                             dbuf);
  snprintf(numbuf, sizeof(numbuf), "%d", GWEN_CryptKey_GetVersion(key));
  GWEN_TAG16_DirectlyToBuffer(GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_VERSION,
                             numbuf,
                             -1,
                             dbuf);

  p=GWEN_DB_GetBinValue(dbKey, "data/e", 0, 0, 0, &bs);
  if (p && bs)
    GWEN_TAG16_DirectlyToBuffer(GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_EXP, p, bs, dbuf);

  p=GWEN_DB_GetBinValue(dbKey, "data/n", 0, 0, 0, &bs);
  if (p && bs) {
    GWEN_TAG16_DirectlyToBuffer(GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_MODULUS, p, bs, dbuf);
    GWEN_TAG16_DirectlyToBuffer(GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_N, p, bs, dbuf);
  }
  else {
    DBG_WARN(GWEN_LOGDOMAIN, "No modulus !");
  }

  p=GWEN_DB_GetBinValue(dbKey, "data/p", 0, 0, 0, &bs);
  if (p && bs)
    GWEN_TAG16_DirectlyToBuffer(GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_P, p, bs, dbuf);

  p=GWEN_DB_GetBinValue(dbKey, "data/q", 0, 0, 0, &bs);
  if (p && bs)
    GWEN_TAG16_DirectlyToBuffer(GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_Q, p, bs, dbuf);

  p=GWEN_DB_GetBinValue(dbKey, "data/d", 0, 0, 0, &bs);
  if (p && bs)
    GWEN_TAG16_DirectlyToBuffer(GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_D, p, bs, dbuf);

  p=GWEN_DB_GetBinValue(dbKey, "data/dmp1", 0, 0, 0, &bs);
  if (p && bs)
    GWEN_TAG16_DirectlyToBuffer(GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_DMP1, p, bs, dbuf);

  p=GWEN_DB_GetBinValue(dbKey, "data/dmq1", 0, 0, 0, &bs);
  if (p && bs)
    GWEN_TAG16_DirectlyToBuffer(GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_DMQ1, p, bs, dbuf);

  p=GWEN_DB_GetBinValue(dbKey, "data/iqmp", 0, 0, 0, &bs);
  if (p && bs)
    GWEN_TAG16_DirectlyToBuffer(GWEN_CRYPTTOKEN_OHBCI_TAG_KEY_IQMP, p, bs, dbuf);

  GWEN_DB_Group_free(dbKey);
  bs=(GWEN_Buffer_GetPos(dbuf)-pos)-2;
  pp=GWEN_Buffer_GetStart(dbuf)+pos;
  pp[0]=bs & 0xff;
  pp[1]=(bs>>8) & 0xff;

  return 0;
}



int GWEN_CryptTokenOHBCI_Encode(GWEN_CRYPTTOKEN *ct, GWEN_BUFFER *dbuf) {
  GWEN_CRYPTTOKEN_OHBCI *lct;
  char numbuf[16];
  const char *p;
  GWEN_CT_FILE_CONTEXT_LIST *fctl;
  GWEN_CT_FILE_CONTEXT *fct;
  GWEN_CRYPTTOKEN_USER *user;
  GWEN_CRYPTKEY *key;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_OHBCI, ct);
  assert(lct);

  /* get the only context */
  fctl=GWEN_CryptTokenFile_GetFileContextList(ct);
  if (!fctl) {
    DBG_ERROR(GWEN_LOGDOMAIN,
	      "Crypt token does not contain a file context");
    return GWEN_ERROR_INVALID;
  }
  fct=GWEN_CryptTokenFile_Context_List_First(fctl);
  if (!fct) {
    DBG_ERROR(GWEN_LOGDOMAIN,
	      "Crypt token does not contain a file context");
    return GWEN_ERROR_INVALID;
  }

  user=GWEN_CryptTokenFile_Context_GetUser(fct);
  if (!user) {
    DBG_ERROR(GWEN_LOGDOMAIN,
	      "File context does not contain a user");
    return GWEN_ERROR_INVALID;
  }

  /* write header again */
  GWEN_TAG16_DirectlyToBuffer(GWEN_CRYPTTOKEN_OHBCI_TAG_HEADER,
                              GWEN_CRYPTTOKEN_OHBCI_NAME,
                              -1, dbuf);

  if (lct->mediumTag!=GWEN_CRYPTTOKEN_OHBCI_TAG_MEDIUM3) {
    /* do not include version info here for ohbci files after 1.5 */
    snprintf(numbuf, sizeof(numbuf), "%d", GWEN_CRYPTTOKEN_OHBCI_VMAJOR);
    GWEN_TAG16_DirectlyToBuffer(GWEN_CRYPTTOKEN_OHBCI_TAG_VERSION_MAJOR,
                                numbuf, -1, dbuf);

    snprintf(numbuf, sizeof(numbuf), "%d", GWEN_CRYPTTOKEN_OHBCI_VMINOR);
    GWEN_TAG16_DirectlyToBuffer(GWEN_CRYPTTOKEN_OHBCI_TAG_VERSION_MINOR,
                                numbuf, -1, dbuf);
  }

  snprintf(numbuf, sizeof(numbuf), "%d",
	   GWEN_CryptTokenFile_Context_GetLocalSignSeq(fct));
  GWEN_TAG16_DirectlyToBuffer(GWEN_CRYPTTOKEN_OHBCI_TAG_SEQ,
			      numbuf, -1, dbuf);

  key=GWEN_CryptTokenFile_Context_GetLocalSignKey(fct);
  if (GWEN_CryptTokenOHBCI__EncodeKey(key,
				      GWEN_CRYPTTOKEN_OHBCI_TAG_USER_PUBSIGNKEY,
				      1, 0, dbuf)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not encode key");
    return -1;
  }

  if (GWEN_CryptTokenOHBCI__EncodeKey(key,
				      GWEN_CRYPTTOKEN_OHBCI_TAG_USER_PRIVSIGNKEY,
				      0, 0, dbuf)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not encode key");
    return -1;
  }

  key=GWEN_CryptTokenFile_Context_GetLocalCryptKey(fct);
  if (GWEN_CryptTokenOHBCI__EncodeKey(key,
                                GWEN_CRYPTTOKEN_OHBCI_TAG_USER_PUBCRYPTKEY,
                                1, 1, dbuf)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not encode key");
    return -1;
  }

  if (GWEN_CryptTokenOHBCI__EncodeKey(key,
                                GWEN_CRYPTTOKEN_OHBCI_TAG_USER_PRIVCRYPTKEY,
                                0, 1, dbuf)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not encode key");
    return -1;
  }

  p=GWEN_CryptToken_User_GetUserId(user);
  if (p && *p)
    GWEN_TAG16_DirectlyToBuffer(GWEN_CRYPTTOKEN_OHBCI_TAG_USER_ID,
                                p, -1, dbuf);

  key=GWEN_CryptTokenFile_Context_GetRemoteSignKey(fct);
  if (key && GWEN_CryptKey_GetOwner(key)==0)
    GWEN_CryptKey_SetOwner(key, GWEN_CryptToken_User_GetPeerId(user));
  if (GWEN_CryptTokenOHBCI__EncodeKey(key,
				      GWEN_CRYPTTOKEN_OHBCI_TAG_INST_PUBSIGNKEY,
				      1, 0, dbuf)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not encode key");
    return -1;
  }

  key=GWEN_CryptTokenFile_Context_GetRemoteCryptKey(fct);
  if (key && GWEN_CryptKey_GetOwner(key)==0)
    GWEN_CryptKey_SetOwner(key, GWEN_CryptToken_User_GetPeerId(user));
  if (GWEN_CryptTokenOHBCI__EncodeKey(key,
				      GWEN_CRYPTTOKEN_OHBCI_TAG_INST_PUBCRYPTKEY,
				      1, 1, dbuf)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not encode key");
    return -1;
  }

  snprintf(numbuf, sizeof(numbuf), "%d", 280);
  GWEN_TAG16_DirectlyToBuffer(GWEN_CRYPTTOKEN_OHBCI_TAG_INST_COUNTRY,
			      numbuf, -1, dbuf);

  p=GWEN_CryptToken_User_GetServiceId(user);
  if (p && *p)
    GWEN_TAG16_DirectlyToBuffer(GWEN_CRYPTTOKEN_OHBCI_TAG_INST_CODE,
				p, -1, dbuf);

  p=GWEN_CryptToken_User_GetSystemId(user);
  if (p && *p)
    GWEN_TAG16_DirectlyToBuffer(GWEN_CRYPTTOKEN_OHBCI_TAG_INST_SYSTEMID,
				p, -1, dbuf);

  /* new in 1.4 */
  p=GWEN_CryptToken_User_GetAddress(user);
  if (p && *p) {
    GWEN_TAG16_DirectlyToBuffer(GWEN_CRYPTTOKEN_OHBCI_TAG_SERVER_ADDR,
				p, -1, dbuf);
    snprintf(numbuf, sizeof(numbuf), "%d",
	     GWEN_CryptToken_User_GetPort(user));
    GWEN_TAG16_DirectlyToBuffer(GWEN_CRYPTTOKEN_OHBCI_TAG_SERVER_PORT,
				numbuf, -1, dbuf);
  }

  /* new in 1.4 */
  snprintf(numbuf, sizeof(numbuf), "%d",
	   GWEN_CryptTokenFile_Context_GetRemoteSignSeq(fct));
  GWEN_TAG16_DirectlyToBuffer(GWEN_CRYPTTOKEN_OHBCI_TAG_REMOTE_SEQ,
			      numbuf, -1, dbuf);

  return 0;
}



int GWEN_CryptTokenOHBCI_Write(GWEN_CRYPTTOKEN *ct, int fd){
  GWEN_CRYPTTOKEN_OHBCI *lct;
  int rv;
  GWEN_BUFFER *fbuf;
  GWEN_BUFFER *rawbuf;
  GWEN_CRYPTKEY *key;
  GWEN_ERRORCODE err;
  char *p;
  unsigned int bs;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_OHBCI, ct);
  assert(lct);

  if (!GWEN_CryptToken_GetTokenName(ct)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No medium name given");
    return -1;
  }

  /* create raw data */
  rawbuf=GWEN_Buffer_new(0, 1024, 0, 1);
  rv=GWEN_CryptTokenOHBCI_Encode(ct, rawbuf);
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
  if (!lct->passWordIsSet) {
    unsigned char password_unsigned[64];
    char password[64];
    GWEN_PLUGIN_MANAGER *pm;
    int mres;
    unsigned int pinLength, k;
    GWEN_TYPE_UINT32 pflags;

    pm=GWEN_CryptToken_GetCryptManager(ct);
    assert(pm);

    /* create key from password */
    memset(lct->password, 0, sizeof(lct->password));
    memset(password_unsigned, 0, sizeof(password_unsigned));

    pflags=0;
    if (lct->justCreated)
      pflags|=GWEN_CRYPTTOKEN_GETPIN_FLAGS_CONFIRM;
    mres=GWEN_CryptManager_GetPin(pm,
				  ct,
				  GWEN_CryptToken_PinType_Access,
				  GWEN_CryptToken_PinEncoding_ASCII,
                                  pflags,
				  password_unsigned,
				  GWEN_CRYPTTOKEN_OHBCI_PINMINLENGTH,
				  sizeof(password_unsigned)-1,
				  &pinLength);
    if (mres) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error asking for PIN, aborting");
      GWEN_Buffer_free(rawbuf);
      return mres;
    }

    /* Convert the 'unsigned char' buffer to the 'char' buffer */
    for (k=0; k < pinLength; ++k)
      password[k] = password_unsigned[k];
    password[k] = '\0';
    memset(password_unsigned, '\0', pinLength);

    if (strlen(password)<GWEN_CRYPTTOKEN_OHBCI_PINMINLENGTH) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Your program returned a shorter PIN than instructed!");
      GWEN_Buffer_free(rawbuf);
      return GWEN_ERROR_GENERIC;
    }

    if (lct->cryptoTag==GWEN_CRYPTTOKEN_OHBCI_TAG_CRYPT ||
        lct->cryptoTag==GWEN_CRYPTTOKEN_OHBCI_TAG_CRYPT_BF) {
      if (GWEN_CryptKey_FromPassword(password,
                                     (unsigned char*)lct->password,
                                     sizeof(lct->password))) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Could not create key data from password");
        return GWEN_ERROR_GENERIC;
      }
    }
    else if (lct->cryptoTag==GWEN_CRYPTTOKEN_OHBCI_TAG_CRYPT_OLD) {
      if (GWEN_CryptKey_FromPasswordSSL(password,
                                        (unsigned char*)lct->password,
                                        sizeof(lct->password))) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Could not create key data from password");
        return GWEN_ERROR_GENERIC;
      }
    }
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected crypto tag %d",
                lct->cryptoTag);
      abort();
    }

    /* overwrite password as soon as possible */
    /* TODO: Set Pin Status */
    memset(password, 0, sizeof(password));
    lct->passWordIsSet=1;
  } /* if password is not set */

  if (lct->cryptoTag==GWEN_CRYPTTOKEN_OHBCI_TAG_CRYPT ||
      lct->cryptoTag==GWEN_CRYPTTOKEN_OHBCI_TAG_CRYPT_OLD) {
    key=GWEN_CryptKey_Factory("DES");
  }
  else if (lct->cryptoTag==GWEN_CRYPTTOKEN_OHBCI_TAG_CRYPT_BF) {
    key=GWEN_CryptKey_Factory("BF");
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
  err=GWEN_CryptKey_SetData(key, lct->password, sizeof(lct->password));
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(GWEN_LOGDOMAIN, err);
    GWEN_CryptKey_free(key);
    GWEN_Buffer_free(rawbuf);
    return GWEN_ERROR_GENERIC;
  }

  /* padd raw data */
  if (GWEN_Padd_PaddWithANSIX9_23(rawbuf)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not padd keyfile");
    GWEN_CryptKey_free(key);
    GWEN_Buffer_free(rawbuf);
    return GWEN_ERROR_GENERIC;
  }

  /* encrypt file */
  fbuf=GWEN_Buffer_new(0, 1024, 0, 1);
  GWEN_Buffer_ReserveBytes(fbuf, 4);

  err=GWEN_CryptKey_Encrypt(key, rawbuf, fbuf);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(GWEN_LOGDOMAIN, err);
    GWEN_Buffer_free(fbuf);
    GWEN_CryptKey_free(key);
    GWEN_Buffer_free(rawbuf);
    return GWEN_ERROR_GENERIC;
  }
  GWEN_Buffer_free(rawbuf);
  GWEN_CryptKey_free(key);

  GWEN_Buffer_Rewind(fbuf);
  bs=GWEN_Buffer_GetUsedBytes(fbuf);
  GWEN_Buffer_InsertBytes(fbuf, "000", 3);
  p=GWEN_Buffer_GetStart(fbuf);
  p[0]=(unsigned char)(lct->cryptoTag);
  p[1]=(unsigned char)(bs & 0xff);
  p[2]=(unsigned char)((bs>>8) & 0xff);

  if (lct->mediumTag==GWEN_CRYPTTOKEN_OHBCI_TAG_MEDIUM3) {
    char numbuf[16];
    GWEN_BUFFER *dbuf;

    /* this is a new medium type, create envelope */
    dbuf=GWEN_Buffer_new(0, 2048, 0, 1);
    /* prepare container tag */
    GWEN_Buffer_AppendBytes(dbuf, "000", 3);

    GWEN_TAG16_DirectlyToBuffer(GWEN_CRYPTTOKEN_OHBCI_TAG_HEADER,
                                GWEN_CRYPTTOKEN_OHBCI_NAME, -1, dbuf);
    snprintf(numbuf, sizeof(numbuf), "%d", GWEN_CRYPTTOKEN_OHBCI_VMAJOR);
    GWEN_TAG16_DirectlyToBuffer(GWEN_CRYPTTOKEN_OHBCI_TAG_VERSION_MAJOR,
                                numbuf, -1, dbuf);

    snprintf(numbuf, sizeof(numbuf), "%d", GWEN_CRYPTTOKEN_OHBCI_VMINOR);
    GWEN_TAG16_DirectlyToBuffer(GWEN_CRYPTTOKEN_OHBCI_TAG_VERSION_MINOR,
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
	      GWEN_CryptToken_GetTokenName(ct),
	      strerror(errno));
    GWEN_Buffer_free(fbuf);
    return GWEN_ERROR_GENERIC;
  }

  while (1) {
    rv=write(fd,
             GWEN_Buffer_GetPosPointer(fbuf),
             GWEN_Buffer_GetBytesLeft(fbuf));
    if (rv==-1) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "write(%s): %s",
                GWEN_CryptToken_GetTokenName(ct),
		strerror(errno));
      GWEN_Buffer_free(fbuf);
      return GWEN_ERROR_GENERIC;
    }
    else if (rv==0)
      break;
    GWEN_Buffer_IncrementPos(fbuf, rv);
  } /* while */

  GWEN_Buffer_free(fbuf);

  lct->justCreated=0;

  return 0;
}



int GWEN_CryptTokenOHBCI_ChangePin(GWEN_CRYPTTOKEN *ct){
  unsigned char password_unsigned[64];
  char password[64];
  GWEN_CRYPTTOKEN_OHBCI *lct;
  GWEN_PLUGIN_MANAGER *pm;
  int mres;
  unsigned int pinLength, k;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_OHBCI, ct);
  assert(lct);

  /* create key from password */
  pm=GWEN_CryptToken_GetCryptManager(ct);
  assert(pm);

  memset(password_unsigned, 0, sizeof(password_unsigned));

  mres=GWEN_CryptManager_GetPin(pm,
				ct,
				GWEN_CryptToken_PinType_Access,
				GWEN_CryptToken_PinEncoding_ASCII,
				GWEN_CRYPTTOKEN_GETPIN_FLAGS_CONFIRM,
				password_unsigned,
				GWEN_CRYPTTOKEN_OHBCI_PINMINLENGTH,
				sizeof(password_unsigned)-1,
				&pinLength);
  if (mres) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Error asking for PIN, aborting");
    return mres;
  }

  /* Convert the 'unsigned char' buffer to the 'char' buffer */
  for (k=0; k < pinLength; ++k)
    password[k] = password_unsigned[k];
  password[k] = '\0';
  memset(password_unsigned, '\0', pinLength);

  if (strlen(password)<GWEN_CRYPTTOKEN_OHBCI_PINMINLENGTH) {
    DBG_ERROR(GWEN_LOGDOMAIN,
	      "Your program returned a shorter PIN than instructed!");
    return GWEN_ERROR_GENERIC;
  }

  if (lct->cryptoTag==GWEN_CRYPTTOKEN_OHBCI_TAG_CRYPT) {
    if (GWEN_CryptKey_FromPassword(password,
                                   (unsigned char*)lct->password,
                                   sizeof(lct->password))) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Could not create key data from password");
      return -1;
    }
  }
  else if (lct->cryptoTag==GWEN_CRYPTTOKEN_OHBCI_TAG_CRYPT_OLD) {
    if (GWEN_CryptKey_FromPasswordSSL(password,
                                      (unsigned char*)lct->password,
                                      sizeof(lct->password))) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Could not create key data from password");
      return -1;
    }
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected crypto tag %d", lct->cryptoTag);
    abort();
  }
  /* overwrite password as soon as possible */
  /* TODO: Set  PIN status */
  memset(password, 0, sizeof(password));
  lct->passWordIsSet=1;

  return 0;
}



int GWEN_CryptTokenOHBCI_Create(GWEN_CRYPTTOKEN *ct){
  GWEN_CRYPTTOKEN_OHBCI *lct;
  int rv;
  GWEN_CT_FILE_CONTEXT *fct;
  GWEN_CRYPTTOKEN_USER *user;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_OHBCI, ct);
  assert(lct);

  lct->justCreated=1;

  fct=GWEN_CryptTokenFile_Context_new();
  user=GWEN_CryptToken_User_new();

  GWEN_CryptTokenFile_Context_SetUser(fct, user);
  GWEN_CryptTokenFile_ClearFileContextList(ct);
  GWEN_CryptTokenFile_AddFileContext(ct, fct);

  assert(lct->createFn);
  rv=lct->createFn(ct);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    return rv;
  }

  return 0;
}



int GWEN_CryptTokenOHBCI_Open(GWEN_CRYPTTOKEN *ct, int manage){
  GWEN_CRYPTTOKEN_OHBCI *lct;
  int rv;
  GWEN_PLUGIN_MANAGER *pm;
  GWEN_PLUGIN_DESCRIPTION *pd;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_OHBCI, ct);
  assert(lct);

  assert(lct->openFn);
  rv=lct->openFn(ct, manage);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    return rv;
  }

  pm=GWEN_CryptToken_GetCryptManager(ct);
  assert(pm);
  pd=GWEN_PluginManager_GetPluginDescr(pm, GWEN_CryptToken_GetTokenType(ct));
  if (pd) {
    GWEN_XMLNODE *n;
    GWEN_XMLNODE *ctNode=0;

    n=GWEN_PluginDescription_GetXmlNode(pd);
    assert(n);
    n=GWEN_XMLNode_FindFirstTag(n, "crypttokens", 0, 0);
    if (n) {
      const char *st;

      st=GWEN_CryptToken_GetTokenSubType(ct);
      if (st && *st)
        ctNode=GWEN_XMLNode_FindFirstTag(n,
                                         "crypttoken",
                                         "subTypeName",
                                         st);
      if (ctNode==0)
        ctNode=GWEN_XMLNode_FindFirstTag(n, "crypttoken", 0, 0);
    }
    if (ctNode) {
      rv=GWEN_CryptToken_ReadXml(ct, ctNode);
      if (rv) {
	DBG_ERROR(GWEN_LOGDOMAIN,
		  "Error reading CryptToken data from XML (%d)",
		  rv);
	GWEN_PluginDescription_free(pd);
	return rv;
      }
      GWEN_PluginDescription_free(pd);
    }
    else {
      DBG_ERROR(GWEN_LOGDOMAIN,
		"Plugin description for crypt token type \"%s\" does "
		"not contain \"crypttoken\" element.",
		GWEN_CryptToken_GetTokenType(ct));
      return GWEN_ERROR_INVALID;
    }
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN,
	      "Could not find plugin description for crypt token type \"%s\"",
	      GWEN_CryptToken_GetTokenType(ct));
    return GWEN_ERROR_INVALID;
  }
  return 0;
}



int GWEN_CryptTokenOHBCI_Close(GWEN_CRYPTTOKEN *ct){
  GWEN_CRYPTTOKEN_OHBCI *lct;
  int rv;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPTTOKEN, GWEN_CRYPTTOKEN_OHBCI, ct);
  assert(lct);

  assert(lct->closeFn);
  rv=lct->closeFn(ct);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    return rv;
  }

  return 0;
}
