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


#include "ct_p.h"
#include "i18n_l.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>



GWEN_INHERIT_FUNCTIONS(GWEN_CRYPT_TOKEN)
GWEN_LIST_FUNCTIONS(GWEN_CRYPT_TOKEN, GWEN_Crypt_Token)
GWEN_LIST2_FUNCTIONS(GWEN_CRYPT_TOKEN, GWEN_Crypt_Token)





GWEN_CRYPT_TOKEN *GWEN_Crypt_Token_new(GWEN_CRYPT_TOKEN_DEVICE dev,
				       const char *typeName,
				       const char *tokenName) {
  GWEN_CRYPT_TOKEN *ct;

  assert(typeName);

  GWEN_NEW_OBJECT(GWEN_CRYPT_TOKEN, ct);
  ct->refCount=1;
  GWEN_INHERIT_INIT(GWEN_CRYPT_TOKEN, ct);
  GWEN_LIST_INIT(GWEN_CRYPT_TOKEN, ct);

  ct->device=dev;
  ct->typeName=strdup(typeName);
  if (tokenName)
    ct->tokenName=strdup(tokenName);

  return ct;
}



void GWEN_Crypt_Token_free(GWEN_CRYPT_TOKEN *ct) {
  if (ct) {
    assert(ct->refCount);
    if (ct->refCount==1) {
      GWEN_LIST_FINI(GWEN_CRYPT_TOKEN, ct);
      GWEN_INHERIT_FINI(GWEN_CRYPT_TOKEN, ct);
      free(ct->tokenName);
      free(ct->typeName);
      ct->refCount=0;
      GWEN_FREE_OBJECT(ct);
    }
    else {
      ct->refCount--;
    }
  }
}



GWEN_CRYPT_TOKEN_DEVICE GWEN_Crypt_Token_GetDevice(const GWEN_CRYPT_TOKEN *ct) {
  assert(ct);
  assert(ct->refCount);

  return ct->device;
}



const char *GWEN_Crypt_Token_GetTypeName(const GWEN_CRYPT_TOKEN *ct) {
  assert(ct);
  assert(ct->refCount);

  return ct->typeName;
}



const char *GWEN_Crypt_Token_GetTokenName(const GWEN_CRYPT_TOKEN *ct) {
  assert(ct);
  assert(ct->refCount);

  return ct->tokenName;
}



void GWEN_Crypt_Token_SetTokenName(GWEN_CRYPT_TOKEN *ct, const char *s) {
  assert(ct);
  assert(ct->refCount);

  assert(s);

  free(ct->tokenName);
  ct->tokenName=strdup(s);
}



const char *GWEN_Crypt_Token_GetFriendlyName(const GWEN_CRYPT_TOKEN *ct) {
  assert(ct);
  assert(ct->refCount);

  return ct->friendlyName;
}



void GWEN_Crypt_Token_SetFriendlyName(GWEN_CRYPT_TOKEN *ct, const char *s) {
  assert(ct);
  assert(ct->refCount);

  assert(s);

  free(ct->friendlyName);
  ct->friendlyName=strdup(s);
}



uint32_t GWEN_Crypt_Token_GetFlags(const GWEN_CRYPT_TOKEN *ct) {
  assert(ct);
  assert(ct->refCount);

  return ct->flags;
}



void GWEN_Crypt_Token_SetFlags(GWEN_CRYPT_TOKEN *ct, uint32_t f) {
  assert(ct);
  assert(ct->refCount);

  ct->flags=f;
}



void GWEN_Crypt_Token_AddFlags(GWEN_CRYPT_TOKEN *ct, uint32_t f) {
  assert(ct);
  assert(ct->refCount);

  ct->flags|=f;
}



void GWEN_Crypt_Token_SubFlags(GWEN_CRYPT_TOKEN *ct, uint32_t f) {
  assert(ct);
  assert(ct->refCount);

  ct->flags&=~f;
}



uint32_t GWEN_Crypt_Token_GetModes(const GWEN_CRYPT_TOKEN *ct) {
  assert(ct);
  assert(ct->refCount);

  return ct->modes;
}



void GWEN_Crypt_Token_SetModes(GWEN_CRYPT_TOKEN *ct, uint32_t f) {
  assert(ct);
  assert(ct->refCount);

  ct->modes=f;
}



void GWEN_Crypt_Token_AddModes(GWEN_CRYPT_TOKEN *ct, uint32_t f) {
  assert(ct);
  assert(ct->refCount);

  ct->modes|=f;
}



void GWEN_Crypt_Token_SubModes(GWEN_CRYPT_TOKEN *ct, uint32_t f) {
  assert(ct);
  assert(ct->refCount);

  ct->modes&=~f;
}



int GWEN_Crypt_Token_Open(GWEN_CRYPT_TOKEN *ct, int admin, uint32_t gid) {
  int rv;

  assert(ct);
  assert(ct->refCount);

  if (ct->openCount) {
    ct->openCount++;
    return 0;
  }

  if (ct->openFn)
    rv=ct->openFn(ct, admin, gid);
  else
    rv=GWEN_ERROR_NOT_IMPLEMENTED;

  if (rv==0)
    ct->openCount++;
  return rv;
}



int GWEN_Crypt_Token_Create(GWEN_CRYPT_TOKEN *ct, uint32_t gid) {
  int rv;

  assert(ct);
  assert(ct->refCount);

  if (ct->createFn)
    rv=ct->createFn(ct, gid);
  else
    rv=GWEN_ERROR_NOT_IMPLEMENTED;

  if (rv==0)
    ct->openCount++;
  return rv;
}



int GWEN_Crypt_Token_Close(GWEN_CRYPT_TOKEN *ct, int abandon, uint32_t gid) {
  assert(ct);
  assert(ct->refCount);

  if (ct->openCount>1 && !abandon) {
    ct->openCount--;
    return 0;
  }

  if (ct->closeFn) {
    int rv;

    rv=ct->closeFn(ct, abandon, gid);
    if (abandon)
      ct->openCount=0;
    else if (rv==0)
      ct->openCount--;
    return rv;
  }
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Crypt_Token_IsOpen(const GWEN_CRYPT_TOKEN *ct) {
  assert(ct);
  assert(ct->refCount);

  return (ct->openCount!=0);
}



int GWEN_Crypt_Token_GetKeyIdList(GWEN_CRYPT_TOKEN *ct,
				  uint32_t *pIdList,
				  uint32_t *pCount,
				  uint32_t gid) {
  assert(ct);
  assert(ct->refCount);

  if (ct->openCount<1)
    return GWEN_ERROR_NOT_OPEN;

  if (ct->getKeyIdListFn)
    return ct->getKeyIdListFn(ct, pIdList, pCount, gid);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



const GWEN_CRYPT_TOKEN_KEYINFO* GWEN_Crypt_Token_GetKeyInfo(GWEN_CRYPT_TOKEN *ct,
							    uint32_t id,
                                                            uint32_t flags,
							    uint32_t gid) {
  assert(ct);
  assert(ct->refCount);

  if (ct->openCount<1) {
    DBG_INFO(GWEN_LOGDOMAIN, "Token not open");
    return NULL;
  }

  if (ct->getKeyInfoFn)
    return ct->getKeyInfoFn(ct, id, flags, gid);
  else
    return NULL;
}



int GWEN_Crypt_Token_SetKeyInfo(GWEN_CRYPT_TOKEN *ct,
				uint32_t id,
				const GWEN_CRYPT_TOKEN_KEYINFO *ki,
				uint32_t gid) {
  assert(ct);
  assert(ct->refCount);

  if (ct->openCount<1)
    return GWEN_ERROR_NOT_OPEN;

  if (ct->setKeyInfoFn)
    return ct->setKeyInfoFn(ct, id, ki, gid);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Crypt_Token_GetContextIdList(GWEN_CRYPT_TOKEN *ct,
				      uint32_t *pIdList,
				      uint32_t *pCount,
				      uint32_t gid) {
  assert(ct);
  assert(ct->refCount);

  if (ct->openCount<1)
    return GWEN_ERROR_NOT_OPEN;

  if (ct->getContextIdListFn)
    return ct->getContextIdListFn(ct, pIdList, pCount, gid);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



const GWEN_CRYPT_TOKEN_CONTEXT* GWEN_Crypt_Token_GetContext(GWEN_CRYPT_TOKEN *ct,
							    uint32_t id,
							    uint32_t gid) {
  assert(ct);
  assert(ct->refCount);

  if (ct->openCount<1) {
    DBG_INFO(GWEN_LOGDOMAIN, "Token not open");
    return NULL;
  }

  if (ct->getContextFn)
    return ct->getContextFn(ct, id, gid);
  else
    return NULL;
}



int GWEN_Crypt_Token_SetContext(GWEN_CRYPT_TOKEN *ct,
				uint32_t id,
				const GWEN_CRYPT_TOKEN_CONTEXT *ctx,
				uint32_t gid) {
  assert(ct);
  assert(ct->refCount);

  if (ct->openCount<1)
    return GWEN_ERROR_NOT_OPEN;

  if (ct->setContextFn)
    return ct->setContextFn(ct, id, ctx, gid);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Crypt_Token_Sign(GWEN_CRYPT_TOKEN *ct,
			  uint32_t keyId,
			  GWEN_CRYPT_PADDALGO *a,
			  const uint8_t *pInData,
			  uint32_t inLen,
			  uint8_t *pSignatureData,
			  uint32_t *pSignatureLen,
			  uint32_t *pSeqCounter,
			  uint32_t gid) {
  assert(ct);
  assert(ct->refCount);

  if (ct->openCount<1)
    return GWEN_ERROR_NOT_OPEN;

  if (ct->signFn)
    return ct->signFn(ct, keyId, a, pInData, inLen, pSignatureData, pSignatureLen,
		      pSeqCounter, gid);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Crypt_Token_Verify(GWEN_CRYPT_TOKEN *ct,
			    uint32_t keyId,
			    GWEN_CRYPT_PADDALGO *a,
			    const uint8_t *pInData,
			    uint32_t inLen,
			    const uint8_t *pSignatureData,
			    uint32_t signatureLen,
			    uint32_t seqCounter,
			    uint32_t gid) {
  assert(ct);
  assert(ct->refCount);

  if (ct->openCount<1)
    return GWEN_ERROR_NOT_OPEN;

  if (ct->verifyFn)
    return ct->verifyFn(ct, keyId, a, pInData, inLen, pSignatureData, signatureLen,
			seqCounter, gid);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Crypt_Token_Encipher(GWEN_CRYPT_TOKEN *ct,
			      uint32_t keyId,
			      GWEN_CRYPT_PADDALGO *a,
			      const uint8_t *pInData,
			      uint32_t inLen,
			      uint8_t *pOutData,
			      uint32_t *pOutLen,
			      uint32_t gid) {
  assert(ct);
  assert(ct->refCount);

  if (ct->openCount<1)
    return GWEN_ERROR_NOT_OPEN;

  if (ct->encipherFn)
    return ct->encipherFn(ct, keyId, a, pInData, inLen, pOutData, pOutLen, gid);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Crypt_Token_Decipher(GWEN_CRYPT_TOKEN *ct,
			      uint32_t keyId,
			      GWEN_CRYPT_PADDALGO *a,
			      const uint8_t *pInData,
			      uint32_t inLen,
			      uint8_t *pOutData,
			      uint32_t *pOutLen,
			      uint32_t gid) {
  assert(ct);
  assert(ct->refCount);

  if (ct->openCount<1)
    return GWEN_ERROR_NOT_OPEN;

  if (ct->decipherFn)
    return ct->decipherFn(ct, keyId, a, pInData, inLen, pOutData, pOutLen, gid);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}




int GWEN_Crypt_Token_GenerateKey(GWEN_CRYPT_TOKEN *ct,
				 uint32_t keyId,
				 const GWEN_CRYPT_CRYPTALGO *a,
				 uint32_t gid) {
  assert(ct);
  assert(ct->refCount);

  if (ct->openCount<1)
    return GWEN_ERROR_NOT_OPEN;

  if (ct->generateKeyFn)
    return ct->generateKeyFn(ct, keyId, a, gid);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int GWEN_Crypt_Token_ChangePin(GWEN_CRYPT_TOKEN *ct, int admin, uint32_t gid) {
  assert(ct);
  assert(ct->refCount);

  if (ct->openCount<1)
    return GWEN_ERROR_NOT_OPEN;

  if (ct->changePinFn)
    return ct->changePinFn(ct, admin, gid);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int  GWEN_Crypt_Token_ActivateKey(GWEN_CRYPT_TOKEN *ct, uint32_t id, uint32_t gid) {
  assert(ct);
  assert(ct->refCount);

  if (ct->openCount<1)
    return GWEN_ERROR_NOT_OPEN;

  if (ct->activateKeyFn)
    return ct->activateKeyFn(ct, id ,gid);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}






GWEN_CRYPT_TOKEN_OPEN_FN GWEN_Crypt_Token_SetOpenFn(GWEN_CRYPT_TOKEN *ct,
						    GWEN_CRYPT_TOKEN_OPEN_FN f) {
  GWEN_CRYPT_TOKEN_OPEN_FN of;

  assert(ct);
  assert(ct->refCount);
  of=ct->openFn;
  ct->openFn=f;

  return of;
}



GWEN_CRYPT_TOKEN_CREATE_FN GWEN_Crypt_Token_SetCreateFn(GWEN_CRYPT_TOKEN *ct,
							GWEN_CRYPT_TOKEN_CREATE_FN f) {
  GWEN_CRYPT_TOKEN_CREATE_FN of;

  assert(ct);
  assert(ct->refCount);
  of=ct->createFn;
  ct->createFn=f;

  return of;

}



GWEN_CRYPT_TOKEN_CLOSE_FN GWEN_Crypt_Token_SetCloseFn(GWEN_CRYPT_TOKEN *ct,
						      GWEN_CRYPT_TOKEN_CLOSE_FN f) {
  GWEN_CRYPT_TOKEN_CLOSE_FN of;

  assert(ct);
  assert(ct->refCount);
  of=ct->closeFn;
  ct->closeFn=f;

  return of;
}



GWEN_CRYPT_TOKEN_GETKEYIDLIST_FN
GWEN_Crypt_Token_SetGetKeyIdListFn(GWEN_CRYPT_TOKEN *ct,
				   GWEN_CRYPT_TOKEN_GETKEYIDLIST_FN f) {
  GWEN_CRYPT_TOKEN_GETKEYIDLIST_FN of;

  assert(ct);
  assert(ct->refCount);
  of=ct->getKeyIdListFn;
  ct->getKeyIdListFn=f;

  return of;
}



GWEN_CRYPT_TOKEN_GETKEYINFO_FN
GWEN_Crypt_Token_SetGetKeyInfoFn(GWEN_CRYPT_TOKEN *ct,
				 GWEN_CRYPT_TOKEN_GETKEYINFO_FN f) {
  GWEN_CRYPT_TOKEN_GETKEYINFO_FN of;

  assert(ct);
  assert(ct->refCount);
  of=ct->getKeyInfoFn;
  ct->getKeyInfoFn=f;

  return of;
}



GWEN_CRYPT_TOKEN_SETKEYINFO_FN GWEN_Crypt_Token_SetSetKeyInfoFn(GWEN_CRYPT_TOKEN *ct,
								GWEN_CRYPT_TOKEN_SETKEYINFO_FN f) {
  GWEN_CRYPT_TOKEN_SETKEYINFO_FN of;

  assert(ct);
  assert(ct->refCount);
  of=ct->setKeyInfoFn;
  ct->setKeyInfoFn=f;

  return of;
}



GWEN_CRYPT_TOKEN_GETCONTEXTIDLIST_FN
GWEN_Crypt_Token_SetGetContextIdListFn(GWEN_CRYPT_TOKEN *ct,
				       GWEN_CRYPT_TOKEN_GETCONTEXTIDLIST_FN f) {
  GWEN_CRYPT_TOKEN_GETCONTEXTIDLIST_FN of;

  assert(ct);
  assert(ct->refCount);
  of=ct->getContextIdListFn;
  ct->getContextIdListFn=f;

  return of;
}



GWEN_CRYPT_TOKEN_GETCONTEXT_FN
GWEN_Crypt_Token_SetGetContextFn(GWEN_CRYPT_TOKEN *ct,
				 GWEN_CRYPT_TOKEN_GETCONTEXT_FN f) {
  GWEN_CRYPT_TOKEN_GETCONTEXT_FN of;

  assert(ct);
  assert(ct->refCount);
  of=ct->getContextFn;
  ct->getContextFn=f;

  return of;
}



GWEN_CRYPT_TOKEN_SETCONTEXT_FN
GWEN_Crypt_Token_SetSetContextFn(GWEN_CRYPT_TOKEN *ct,
				 GWEN_CRYPT_TOKEN_SETCONTEXT_FN f) {
  GWEN_CRYPT_TOKEN_SETCONTEXT_FN of;

  assert(ct);
  assert(ct->refCount);
  of=ct->setContextFn;
  ct->setContextFn=f;

  return of;
}



GWEN_CRYPT_TOKEN_SIGN_FN GWEN_Crypt_Token_SetSignFn(GWEN_CRYPT_TOKEN *ct,
						    GWEN_CRYPT_TOKEN_SIGN_FN f) {
  GWEN_CRYPT_TOKEN_SIGN_FN of;

  assert(ct);
  assert(ct->refCount);
  of=ct->signFn;
  ct->signFn=f;

  return of;
}



GWEN_CRYPT_TOKEN_VERIFY_FN GWEN_Crypt_Token_SetVerifyFn(GWEN_CRYPT_TOKEN *ct,
							GWEN_CRYPT_TOKEN_VERIFY_FN f) {
  GWEN_CRYPT_TOKEN_VERIFY_FN of;

  assert(ct);
  assert(ct->refCount);
  of=ct->verifyFn;
  ct->verifyFn=f;

  return of;
}



GWEN_CRYPT_TOKEN_ENCIPHER_FN GWEN_Crypt_Token_SetEncipherFn(GWEN_CRYPT_TOKEN *ct,
							    GWEN_CRYPT_TOKEN_ENCIPHER_FN f) {
  GWEN_CRYPT_TOKEN_ENCIPHER_FN of;

  assert(ct);
  assert(ct->refCount);
  of=ct->encipherFn;
  ct->encipherFn=f;

  return of;
}



GWEN_CRYPT_TOKEN_DECIPHER_FN GWEN_Crypt_Token_SetDecipherFn(GWEN_CRYPT_TOKEN *ct,
							    GWEN_CRYPT_TOKEN_DECIPHER_FN f) {
  GWEN_CRYPT_TOKEN_DECIPHER_FN of;

  assert(ct);
  assert(ct->refCount);
  of=ct->decipherFn;
  ct->decipherFn=f;

  return of;
}



GWEN_CRYPT_TOKEN_GENERATEKEY_FN
GWEN_Crypt_Token_SetGenerateKeyFn(GWEN_CRYPT_TOKEN *ct,
				  GWEN_CRYPT_TOKEN_GENERATEKEY_FN f) {
  GWEN_CRYPT_TOKEN_GENERATEKEY_FN of;

  assert(ct);
  assert(ct->refCount);
  of=ct->generateKeyFn;
  ct->generateKeyFn=f;

  return of;
}



GWEN_CRYPT_TOKEN_CHANGEPIN_FN GWEN_Crypt_Token_SetChangePinFn(GWEN_CRYPT_TOKEN *ct,
							      GWEN_CRYPT_TOKEN_CHANGEPIN_FN f) {
  GWEN_CRYPT_TOKEN_CHANGEPIN_FN of;

  assert(ct);
  assert(ct->refCount);
  of=ct->changePinFn;
  ct->changePinFn=f;

  return of;
}



GWEN_CRYPT_TOKEN_ACTIVATEKEY_FN GWEN_Crypt_Token_SetActivateKeyFn(GWEN_CRYPT_TOKEN *ct,
								  GWEN_CRYPT_TOKEN_ACTIVATEKEY_FN f) {
  GWEN_CRYPT_TOKEN_ACTIVATEKEY_FN of;

  assert(ct);
  assert(ct->refCount);
  of=ct->activateKeyFn;
  ct->activateKeyFn=f;

  return of;
}





int GWEN_Crypt_Token__CreatePasswordName(GWEN_CRYPT_TOKEN *ct,
					 GWEN_CRYPT_PINTYPE pt,
					 GWEN_BUFFER *nbuf) {
  const char *tname;
  const char *dname;

  tname=GWEN_Crypt_Token_GetTypeName(ct);
  assert(tname);
  dname=GWEN_Crypt_Token_GetTokenName(ct);
  if (!dname) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Token has no name");
    return GWEN_ERROR_INVALID;
  }

  GWEN_Buffer_AppendString(nbuf, "PASSWORD_");
  GWEN_Buffer_AppendString(nbuf, tname);
  GWEN_Buffer_AppendString(nbuf, "_");
  GWEN_Buffer_AppendString(nbuf, dname);
  if (pt==GWEN_Crypt_PinType_Manage)
    GWEN_Buffer_AppendString(nbuf, ":MANAGE");

  return 0;
}



int GWEN_Crypt_Token_GetPin(GWEN_CRYPT_TOKEN *ct,
			    GWEN_CRYPT_PINTYPE pt,
			    GWEN_CRYPT_PINENCODING pe,
			    uint32_t flags,
			    unsigned char *pwbuffer,
			    unsigned int minLength,
			    unsigned int maxLength,
			    unsigned int *pinLength,
			    uint32_t gid) {
  int rv;
  const char *dname;
  const char *mode;
  const char *numeric_warning = "";
  char buffer[512];
  GWEN_BUFFER *nameBuffer;

  assert(ct);
  assert(ct->refCount);

  dname=GWEN_Crypt_Token_GetFriendlyName(ct);
  if (!dname || !*dname)
    dname=GWEN_Crypt_Token_GetTokenName(ct);

  if (pt==GWEN_Crypt_PinType_Access)
    mode=I18N("access password");
  else if (pt==GWEN_Crypt_PinType_Manage)
    mode=I18N("manager password");
  else
    mode=I18N("password");

  buffer[0]=0;
  buffer[sizeof(buffer)-1]=0;
  if (flags & GWEN_GUI_INPUT_FLAGS_NUMERIC) {
    numeric_warning = I18N("\nYou must only enter numbers, not letters.");
  }

  if (flags & GWEN_GUI_INPUT_FLAGS_CONFIRM) {
    snprintf(buffer, sizeof(buffer)-1,
	     I18N("Please enter a new %s for \n"
		  "%s\n"
		  "The password must be at least %d characters long.%s"
		  "<html>"
		  "Please enter a new %s for <i>%s</i>. "
		  "The password must be at least %d characters long.%s"
		  "</html>"),
	     mode,
	     dname,
	     minLength,
	     numeric_warning,
	     mode,
	     dname,
	     minLength,
	     numeric_warning);
  }
  else {
    snprintf(buffer, sizeof(buffer)-1,
	     I18N("Please enter the %s for \n"
		  "%s\n"
		  "%s<html>"
		  "Please enter the %s for <i>%s</i>.%s"
		  "</html>"),
	     mode,
	     dname,
	     numeric_warning,
	     mode,
	     dname,
	     numeric_warning);
  }

  nameBuffer=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Crypt_Token__CreatePasswordName(ct, pt, nameBuffer);
  rv=GWEN_Gui_GetPassword(flags,
			  GWEN_Buffer_GetStart(nameBuffer),
			  I18N("Enter Password"),
			  buffer,
			  (char*)pwbuffer,
			  minLength,
			  maxLength, gid);
  GWEN_Buffer_free(nameBuffer);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  *pinLength=strlen((char*)pwbuffer);

  if (pe!=GWEN_Crypt_PinEncoding_Ascii) {
    rv=GWEN_Crypt_TransformPin(GWEN_Crypt_PinEncoding_Ascii,
			       pe,
			       pwbuffer,
			       maxLength,
			       pinLength);
    if (rv) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  return 0;
}



int GWEN_Crypt_Token_SetPinStatus(GWEN_CRYPT_TOKEN *ct,
				  GWEN_CRYPT_PINTYPE pt,
				  GWEN_CRYPT_PINENCODING pe,
				  GWEN_UNUSED uint32_t flags,
				  const unsigned char *buffer,
				  unsigned int pinLength,
				  int isOk,
				  uint32_t gid) {
  GWEN_BUFFER *nameBuffer;
  int rv;
  unsigned char ibuffer[256];

  assert(ct);
  assert(ct->refCount);

  if (pe!=GWEN_Crypt_PinEncoding_Ascii) {
    if (pinLength>=sizeof(ibuffer)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Pin too long");
      return GWEN_ERROR_BUFFER_OVERFLOW;
    }
    memset(ibuffer, 0, sizeof(ibuffer));
    memmove(ibuffer, buffer, pinLength);
    rv=GWEN_Crypt_TransformPin(pe,
			       GWEN_Crypt_PinEncoding_Ascii,
			       ibuffer,
			       sizeof(ibuffer)-1,
			       &pinLength);
    if (rv) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    buffer=ibuffer;
  }

  nameBuffer=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Crypt_Token__CreatePasswordName(ct, pt, nameBuffer);
  rv=GWEN_Gui_SetPasswordStatus(GWEN_Buffer_GetStart(nameBuffer),
				(const char*)buffer,
				isOk?GWEN_Gui_PasswordStatus_Ok:
				GWEN_Gui_PasswordStatus_Bad, gid);
  memset(ibuffer, 0, sizeof(ibuffer));
  GWEN_Buffer_free(nameBuffer);
  return rv;

}



uint32_t GWEN_Crypt_Token_BeginEnterPin(GWEN_CRYPT_TOKEN *ct,
					GWEN_UNUSED GWEN_CRYPT_PINTYPE pt,
					uint32_t gid) {
  char buffer[512];

  assert(ct);
  assert(ct->refCount);

  buffer[0]=0;
  buffer[sizeof(buffer)-1]=0;

  snprintf(buffer, sizeof(buffer)-1, "%s",
	   I18N("Please enter your PIN into the card reader."
		"<html>"
		"Please enter your PIN into the card reader."
		"</html>"));
  return GWEN_Gui_ShowBox(GWEN_GUI_SHOWBOX_FLAGS_BEEP,
			  I18N("Secure PIN Input"),
			  buffer, gid);
}



int GWEN_Crypt_Token_EndEnterPin(GWEN_CRYPT_TOKEN *ct,
				 GWEN_UNUSED GWEN_CRYPT_PINTYPE pt,
				 GWEN_UNUSED int ok,
				 uint32_t id) {
  assert(ct);
  assert(ct->refCount);

  GWEN_Gui_HideBox(id);

  return 0;
}



int GWEN_Crypt_Token_InsertToken(GWEN_CRYPT_TOKEN *ct, uint32_t gid) {
  int rv;
  char buffer[512];
  const char *dname;

  assert(ct);
  assert(ct->refCount);

  buffer[0]=0;
  buffer[sizeof(buffer)-1]=0;

  dname=GWEN_Crypt_Token_GetFriendlyName(ct);
  if (!dname || !*dname)
    dname=GWEN_Crypt_Token_GetTokenName(ct);

  if (GWEN_Crypt_Token_GetDevice(ct)==GWEN_Crypt_Token_Device_File)
    snprintf(buffer, sizeof(buffer)-1,
	     I18N("Please insert the security disc\nfor %s"
		  "<html>"
		  "Please insert the security disc for <i>%s</i>"
		  "</html>"), dname, dname);
  else
    snprintf(buffer, sizeof(buffer)-1,
	     I18N("Please insert the chip card\nfor %s"
		  "<html>"
		  "Please insert the chip card for <i>%s</i>"
		  "</html>"), dname, dname);

  rv=GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_WARN |
			 GWEN_GUI_MSG_FLAGS_SEVERITY_NORMAL |
			 GWEN_GUI_MSG_FLAGS_CONFIRM_B1,
			 I18N("Insert Medium"),
			 buffer,
			 I18N("OK"), I18N("Abort"), 0, gid);
  if (rv==2) {
    DBG_ERROR(GWEN_LOGDOMAIN, "User aborted");
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Notice,
			 I18N("Aborted by user."));
    return GWEN_ERROR_USER_ABORTED;
  }
  else if (rv!=1) {
    GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_ERROR |
			GWEN_GUI_MSG_FLAGS_SEVERITY_DANGEROUS |
			GWEN_GUI_MSG_FLAGS_CONFIRM_B1,
			I18N("Error"),
			I18N("An internal error occurred."),
			I18N("Dismiss"), 0, 0, gid);
    return -1;
  }

  return 0;
}



int GWEN_Crypt_Token_InsertCorrectToken(GWEN_CRYPT_TOKEN *ct, uint32_t gid) {
  int rv;
  char buffer[512];
  const char *dname;

  assert(ct);
  assert(ct->refCount);

  buffer[0]=0;
  buffer[sizeof(buffer)-1]=0;

  dname=GWEN_Crypt_Token_GetFriendlyName(ct);
  if (!dname || !*dname)
    dname=GWEN_Crypt_Token_GetTokenName(ct);

  if (GWEN_Crypt_Token_GetDevice(ct)==GWEN_Crypt_Token_Device_File)
    snprintf(buffer, sizeof(buffer)-1,
	     I18N("Please insert the correct security disc\nfor %s"
		  "<html>"
		  "Please insert the correct security disc for <i>%s</i>"
		  "</html>"), dname, dname);
  else {
    if (dname && *dname) {
      snprintf(buffer, sizeof(buffer)-1,
	       I18N("The wrong chipcard has been inserted.\n"
		    "Please insert the chipcard with the number\n"
		    "  %s\n"
		    "into the card reader.\n"
		    "<html>"
		    "<p>The wrong card has been inserted.</p>"
		    "<p>Please insert the chipcard with the number"
		    "<b>%s</b> into the card reader.</p>"
		    "</html>"),
	       dname,
	       dname);
    }
    else
      snprintf(buffer, sizeof(buffer)-1,
	       I18N("Please insert the correct chipcard\nfor %s"
		    "<html>"
		    "Please insert the correct chipcard for <i>%s</i>"
		    "</html>"), dname, dname);
  }

  rv=GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_WARN |
			 GWEN_GUI_MSG_FLAGS_SEVERITY_DANGEROUS |
			 GWEN_GUI_MSG_FLAGS_CONFIRM_B1,
			 I18N("Insert Medium"),
			 buffer,
			 I18N("OK"), I18N("Abort"), 0, gid);
  if (rv==2) {
    DBG_ERROR(GWEN_LOGDOMAIN, "User aborted");
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Notice,
			 I18N("Aborted by user."));
    return GWEN_ERROR_USER_ABORTED;
  }
  else if (rv!=1) {
    GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_ERROR |
			GWEN_GUI_MSG_FLAGS_SEVERITY_DANGEROUS |
			GWEN_GUI_MSG_FLAGS_CONFIRM_B1,
			I18N("Error"),
			I18N("An internal error occurred."),
			I18N("Dismiss"), 0, 0, gid);
    return -1;
  }

  return 0;
}



GWEN_CRYPT_TOKEN_DEVICE GWEN_Crypt_Token_Device_fromString(const char *s){
  assert(s);
  if (strcasecmp(s, "none")==0)
    return GWEN_Crypt_Token_Device_None;
  else if (strcasecmp(s, "file")==0)
    return GWEN_Crypt_Token_Device_File;
  else if (strcasecmp(s, "card")==0)
    return GWEN_Crypt_Token_Device_Card;
  else if (strcasecmp(s, "any")==0)
    return GWEN_Crypt_Token_Device_Any;
  return GWEN_Crypt_Token_Device_Unknown;
}



const char *GWEN_Crypt_Token_Device_toString(GWEN_CRYPT_TOKEN_DEVICE d){
  switch(d) {
  case GWEN_Crypt_Token_Device_None:
    return "none";
  case GWEN_Crypt_Token_Device_File:
    return "file";
  case GWEN_Crypt_Token_Device_Card:
    return "card";
  case GWEN_Crypt_Token_Device_Any:
    return "any";
  default:
    return "unknown";
  }
}






