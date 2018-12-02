/***************************************************************************
    begin       : Wed Mar 16 2005
    copyright   : (C) 2005 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define DISABLE_DEBUGLOG


#include "ct_keyinfo_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>



GWEN_LIST_FUNCTIONS(GWEN_CRYPT_TOKEN_KEYINFO, GWEN_Crypt_Token_KeyInfo)
GWEN_LIST2_FUNCTIONS(GWEN_CRYPT_TOKEN_KEYINFO, GWEN_Crypt_Token_KeyInfo)




GWEN_CRYPT_TOKEN_KEYINFO *GWEN_Crypt_Token_KeyInfo_new(uint32_t kid,
    GWEN_CRYPT_CRYPTALGOID a,
    int keySize) {
  GWEN_CRYPT_TOKEN_KEYINFO *ki;

  GWEN_NEW_OBJECT(GWEN_CRYPT_TOKEN_KEYINFO, ki)
  ki->refCount=1;
  GWEN_LIST_INIT(GWEN_CRYPT_TOKEN_KEYINFO, ki)

  ki->keyId=kid;
  ki->cryptAlgoId=a;
  ki->keySize=keySize;

  return ki;
}



void GWEN_Crypt_Token_KeyInfo_free(GWEN_CRYPT_TOKEN_KEYINFO *ki) {
  if (ki) {
    assert(ki->refCount);
    if (ki->refCount==1) {
      free(ki->keyDescr);
      if (ki->modulusData)
        free(ki->modulusData);
      ki->modulusData=NULL;
      if (ki->exponentData)
        free(ki->exponentData);
      ki->exponentData=NULL;
      if (ki->certificateData)
        free(ki->certificateData);
      ki->certificateData=NULL;

      ki->refCount=0;
      GWEN_LIST_FINI(GWEN_CRYPT_TOKEN_KEYINFO, ki);
      GWEN_FREE_OBJECT(ki);
    }
    else {
      ki->refCount--;
    }
  }
}



GWEN_CRYPT_TOKEN_KEYINFO *GWEN_Crypt_Token_KeyInfo_dup(const GWEN_CRYPT_TOKEN_KEYINFO *ki) {
  GWEN_CRYPT_TOKEN_KEYINFO *nki;

  nki=GWEN_Crypt_Token_KeyInfo_new(ki->keyId,
                                   ki->cryptAlgoId,
                                   ki->keySize);
  nki->flags=ki->flags;

  if (ki->modulusData && ki->modulusLen) {
    nki->modulusData=(uint8_t*)malloc(ki->modulusLen);
    assert(nki->modulusData);
    memmove(nki->modulusData, ki->modulusData, ki->modulusLen);
    nki->modulusLen=ki->modulusLen;
  }

  if (ki->exponentData && ki->exponentLen) {
    nki->exponentData=(uint8_t*)malloc(ki->exponentLen);
    assert(nki->exponentData);
    memmove(nki->exponentData, ki->exponentData, ki->exponentLen);
    nki->exponentLen=ki->exponentLen;
  }

  if (ki->certificateData && ki->certificateLen) {
    nki->certificateData=(uint8_t*)malloc(ki->certificateLen);
    assert(nki->certificateData);
    memmove(nki->certificateData, ki->certificateData, ki->certificateLen);
    nki->certificateLen=ki->certificateLen;
  }

  if (ki->keyDescr)
    nki->keyDescr=strdup(ki->keyDescr);

  nki->keyNumber=ki->keyNumber;
  nki->keyVersion=ki->keyVersion;
  nki->signCounter=ki->signCounter;

  return nki;
}



uint32_t GWEN_Crypt_Token_KeyInfo_GetKeyId(const GWEN_CRYPT_TOKEN_KEYINFO *ki) {
  assert(ki);
  assert(ki->refCount);
  return ki->keyId;
}



GWEN_CRYPT_CRYPTALGOID GWEN_Crypt_Token_KeyInfo_GetCryptAlgoId(const GWEN_CRYPT_TOKEN_KEYINFO *ki) {
  assert(ki);
  assert(ki->refCount);
  return ki->cryptAlgoId;
}



int GWEN_Crypt_Token_KeyInfo_GetKeySize(const GWEN_CRYPT_TOKEN_KEYINFO *ki) {
  assert(ki);
  assert(ki->refCount);
  return ki->keySize;
}



void GWEN_Crypt_Token_KeyInfo_SetKeySize(GWEN_CRYPT_TOKEN_KEYINFO *ki, int i) {
  assert(ki);
  assert(ki->refCount);
  ki->keySize=i;
}



uint32_t GWEN_Crypt_Token_KeyInfo_GetFlags(const GWEN_CRYPT_TOKEN_KEYINFO *ki) {
  assert(ki);
  assert(ki->refCount);
  return ki->flags;
}



void GWEN_Crypt_Token_KeyInfo_SetFlags(GWEN_CRYPT_TOKEN_KEYINFO *ki, uint32_t f) {
  assert(ki);
  assert(ki->refCount);
  ki->flags=f;
}



void GWEN_Crypt_Token_KeyInfo_AddFlags(GWEN_CRYPT_TOKEN_KEYINFO *ki, uint32_t f) {
  assert(ki);
  assert(ki->refCount);
  ki->flags|=f;
}



void GWEN_Crypt_Token_KeyInfo_SubFlags(GWEN_CRYPT_TOKEN_KEYINFO *ki, uint32_t f) {
  assert(ki);
  assert(ki->refCount);
  ki->flags&=~f;
}



const uint8_t *GWEN_Crypt_Token_KeyInfo_GetModulusData(const GWEN_CRYPT_TOKEN_KEYINFO *ki) {
  assert(ki);
  assert(ki->refCount);
  return ki->modulusData;
}



uint32_t GWEN_Crypt_Token_KeyInfo_GetModulusLen(const GWEN_CRYPT_TOKEN_KEYINFO *ki) {
  assert(ki);
  assert(ki->refCount);
  return ki->modulusLen;
}



void GWEN_Crypt_Token_KeyInfo_SetModulus(GWEN_CRYPT_TOKEN_KEYINFO *ki,
    const uint8_t *p,
    uint32_t len) {
  assert(ki);
  assert(ki->refCount);

  assert(p);
  assert(len);

  if (ki->modulusData)
    free(ki->modulusData);
  ki->modulusData=(uint8_t*) malloc(len);
  assert(ki->modulusData);
  memmove(ki->modulusData, p, len);
  ki->modulusLen=len;
}



const uint8_t *GWEN_Crypt_Token_KeyInfo_GetExponentData(const GWEN_CRYPT_TOKEN_KEYINFO *ki) {
  assert(ki);
  assert(ki->refCount);
  return ki->exponentData;
}



uint32_t GWEN_Crypt_Token_KeyInfo_GetExponentLen(const GWEN_CRYPT_TOKEN_KEYINFO *ki) {
  assert(ki);
  assert(ki->refCount);
  return ki->exponentLen;
}



void GWEN_Crypt_Token_KeyInfo_SetExponent(GWEN_CRYPT_TOKEN_KEYINFO *ki,
    const uint8_t *p,
    uint32_t len) {
  assert(ki);
  assert(ki->refCount);

  assert(p);
  assert(len);

  if (ki->exponentData)
    free(ki->exponentData);
  ki->exponentData=(uint8_t*) malloc(len);
  assert(ki->exponentData);
  memmove(ki->exponentData, p, len);
  ki->exponentLen=len;
}

const uint8_t *GWEN_Crypt_Token_KeyInfo_GetCertificateData(const GWEN_CRYPT_TOKEN_KEYINFO *ki) {
  assert(ki);
  assert(ki->refCount);
  return ki->certificateData;
}



uint32_t GWEN_Crypt_Token_KeyInfo_GetCertificateLen(const GWEN_CRYPT_TOKEN_KEYINFO *ki) {
  assert(ki);
  assert(ki->refCount);
  return ki->certificateLen;
}



void GWEN_Crypt_Token_KeyInfo_SetCertificate(GWEN_CRYPT_TOKEN_KEYINFO *ki,
    const uint8_t *p,
    uint32_t len) {
  assert(ki);
  assert(ki->refCount);

  assert(p);
  assert(len);

  if (ki->certificateData)
    free(ki->certificateData);
  ki->certificateData=(uint8_t*) malloc(len);
  assert(ki->certificateData);
  memmove(ki->certificateData, p, len);
  ki->certificateLen=len;
}

uint32_t GWEN_Crypt_Token_KeyInfo_GetKeyVersion(const GWEN_CRYPT_TOKEN_KEYINFO *ki) {
  assert(ki);
  assert(ki->refCount);

  return ki->keyVersion;
}



void GWEN_Crypt_Token_KeyInfo_SetKeyVersion(GWEN_CRYPT_TOKEN_KEYINFO *ki,
    uint32_t i) {
  assert(ki);
  assert(ki->refCount);

  ki->keyVersion=i;
}



uint32_t GWEN_Crypt_Token_KeyInfo_GetKeyNumber(const GWEN_CRYPT_TOKEN_KEYINFO *ki) {
  assert(ki);
  assert(ki->refCount);

  return ki->keyNumber;
}



void GWEN_Crypt_Token_KeyInfo_SetKeyNumber(GWEN_CRYPT_TOKEN_KEYINFO *ki,
    uint32_t i) {
  assert(ki);
  assert(ki->refCount);

  ki->keyNumber=i;
}



uint32_t GWEN_Crypt_Token_KeyInfo_GetSignCounter(const GWEN_CRYPT_TOKEN_KEYINFO *ki) {
  assert(ki);
  assert(ki->refCount);

  return ki->signCounter;
}



void GWEN_Crypt_Token_KeyInfo_SetSignCounter(GWEN_CRYPT_TOKEN_KEYINFO *ki,
    uint32_t i) {
  assert(ki);
  assert(ki->refCount);

  ki->signCounter=i;
}



const char *GWEN_Crypt_Token_KeyInfo_GetKeyDescr(const GWEN_CRYPT_TOKEN_KEYINFO *ki) {
  assert(ki);
  assert(ki->refCount);

  return ki->keyDescr;
}



void GWEN_Crypt_Token_KeyInfo_SetKeyDescr(GWEN_CRYPT_TOKEN_KEYINFO *ki, const char *s) {
  assert(ki);
  assert(ki->refCount);

  free(ki->keyDescr);
  if (s)
    ki->keyDescr=strdup(s);
  else
    ki->keyDescr=NULL;
}

void GWEN_Crypt_Token_KeyInfo_Dump(GWEN_CRYPT_TOKEN_KEYINFO *ki) {
  const char * s;
  uint32_t flags=0;

  assert(ki);

  fprintf(stdout, "-------------------------------------------------\n");
  fprintf(stdout, "Key %08x\n",
          (unsigned int)GWEN_Crypt_Token_KeyInfo_GetKeyId(ki));

  s=GWEN_Crypt_Token_KeyInfo_GetKeyDescr(ki);
  if (s)
    fprintf(stdout, "Key Descr  : %s\n", s);

  fprintf(stdout, "Crypt Algo : %s\n",
          GWEN_Crypt_CryptAlgoId_toString(GWEN_Crypt_Token_KeyInfo_GetCryptAlgoId(ki)));
  fprintf(stdout, "Key Size   : %d\n", GWEN_Crypt_Token_KeyInfo_GetKeySize(ki));

  fprintf(stdout, "Key Flags  :");
  flags=GWEN_Crypt_Token_KeyInfo_GetFlags(ki);
  if (flags & GWEN_CRYPT_TOKEN_KEYFLAGS_HASSTATUS)
    fprintf(stdout, " STATUS");
  if (flags & GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS)
    fprintf(stdout, " MODULUS");
  if (flags & GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT)
    fprintf(stdout, " EXPONENT");
  if (flags & GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION)
    fprintf(stdout, " KEYVERSION");
  if (flags & GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER)
    fprintf(stdout, " KEYNUMBER");
  if (flags & GWEN_CRYPT_TOKEN_KEYFLAGS_HASSIGNCOUNTER)
    fprintf(stdout, " SIGNCOUNTER");
  if (flags & GWEN_CRYPT_TOKEN_KEYFLAGS_HASACTIONFLAGS) {
    if (flags & GWEN_CRYPT_TOKEN_KEYFLAGS_CANSIGN)
      fprintf(stdout, " SIGN");
    if (flags & GWEN_CRYPT_TOKEN_KEYFLAGS_CANVERIFY)
      fprintf(stdout, " VERIFY");
    if (flags & GWEN_CRYPT_TOKEN_KEYFLAGS_CANENCIPHER)
      fprintf(stdout, " ENCIPHER");
    if (flags & GWEN_CRYPT_TOKEN_KEYFLAGS_CANDECIPHER)
      fprintf(stdout, " DECIPHER");
  }
  fprintf(stdout, "\n");

  if (flags & GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER)
    fprintf(stdout, "Key Number : %d\n", GWEN_Crypt_Token_KeyInfo_GetKeyNumber(ki));
  if (flags & GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION)
    fprintf(stdout, "Key Version: %d\n", GWEN_Crypt_Token_KeyInfo_GetKeyVersion(ki));
  if (flags & GWEN_CRYPT_TOKEN_KEYFLAGS_HASSIGNCOUNTER)
    fprintf(stdout, "Sign Cnt   : %d\n", GWEN_Crypt_Token_KeyInfo_GetSignCounter(ki));
  if (flags & GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS) {
    GWEN_BUFFER *tbuf;
    const uint8_t *p;
    uint32_t len;
    int nbits;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    p=(const uint8_t*)GWEN_Crypt_Token_KeyInfo_GetModulusData(ki);
    len=GWEN_Crypt_Token_KeyInfo_GetModulusLen(ki);

    nbits=len*8;
    while(len && *p==0) {
      p++;
      len--;
      nbits-=8;
    }
    if (len) {
      int i;
      uint8_t mask=0x80;
      uint8_t b=*p;

      for (i=0; i<8; i++) {
        if (b & mask)
          break;
        nbits--;
        mask>>=1;
      }
    }

    fprintf(stdout, "Modulus    : (%d bits)\n", nbits);

    while(len) {
      uint32_t rl;

      rl=(len>16)?16:len;
      GWEN_Text_ToHexBuffer((const char*)p, rl, tbuf, 2, ' ', 0);
      fprintf(stdout, "   %s\n", GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_Reset(tbuf);
      p+=rl;
      len-=rl;
    }
    GWEN_Buffer_free(tbuf);
  }

  if (flags & GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT) {
    GWEN_BUFFER *tbuf;
    const char *p;
    uint32_t len;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    fprintf(stdout, "Exponent   : \n");
    p=(const char*)GWEN_Crypt_Token_KeyInfo_GetExponentData(ki);
    len=GWEN_Crypt_Token_KeyInfo_GetExponentLen(ki);
    while(len) {
      uint32_t rl;

      rl=(len>16)?16:len;
      GWEN_Text_ToHexBuffer(p, rl, tbuf, 2, ' ', 0);
      fprintf(stdout, "   %s\n", GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_Reset(tbuf);
      p+=rl;
      len-=rl;
    }
    GWEN_Buffer_free(tbuf);
  }

  if (flags & GWEN_CRYPT_TOKEN_KEYFLAGS_HASCERTIFICATE) {
    GWEN_BUFFER *tbuf;
    const char *p;
    uint32_t len;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    fprintf(stdout, "Certificate   : \n");
    p=(const char*)GWEN_Crypt_Token_KeyInfo_GetCertificateData(ki);
    len=GWEN_Crypt_Token_KeyInfo_GetCertificateLen(ki);
    while(len) {
      uint32_t rl;

      rl=(len>16)?16:len;
      GWEN_Text_ToHexBuffer(p, rl, tbuf, 2, ' ', 0);
      fprintf(stdout, "   %s\n", GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_Reset(tbuf);
      p+=rl;
      len-=rl;
    }
    GWEN_Buffer_free(tbuf);
  }

}
