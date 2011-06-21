/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id: create.c 892 2005-11-03 00:20:45Z aquamaniac $
 begin       : Tue May 03 2005
 copyright   : (C) 2005 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "globals.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/ct.h>
#include <gwenhywfar/ctplugin.h>
#include <gwenhywfar/text.h>






int showKey(GWEN_DB_NODE *dbArgs, int argc, char **argv) {
  GWEN_DB_NODE *db;
  const char *ttype;
  const char *tname;
  GWEN_CRYPT_TOKEN *ct;
  unsigned int keyId;
  int shown=0;
  int rv;
  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Int,             /* type */
    "keyId",                      /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "k",                          /* short option */
    "key",                        /* long option */
    "Key id (0 for any)",         /* short description */
    "Key id (0 for any)"          /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,            /* type */
    "tokenType",                  /* name */
    1,                            /* minnum */
    1,                            /* maxnum */
    "t",                          /* short option */
    "ttype",                    /* long option */
    "Specify the crypt token type",     /* short description */
    "Specify the crypt token type"      /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,            /* type */
    "tokenName",                  /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "n",                          /* short option */
    "tname",                    /* long option */
    "Specify the crypt token name",     /* short description */
    "Specify the crypt token name"      /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HELP | GWEN_ARGS_FLAGS_LAST, /* flags */
    GWEN_ArgsType_Int,             /* type */
    "help",                       /* name */
    0,                            /* minnum */
    0,                            /* maxnum */
    "h",                          /* short option */
    "help",                       /* long option */
    "Show this help screen",      /* short description */
    "Show this help screen"       /* long description */
  }
  };

  db=GWEN_DB_GetGroup(dbArgs, GWEN_DB_FLAGS_DEFAULT, "local");
  rv=GWEN_Args_Check(argc, argv, 1,
                     GWEN_ARGS_MODE_ALLOW_FREEPARAM,
                     args,
                     db);
  if (rv==GWEN_ARGS_RESULT_ERROR) {
    fprintf(stderr, "ERROR: Could not parse arguments\n");
    return 1;
  }
  else if (rv==GWEN_ARGS_RESULT_HELP) {
    GWEN_BUFFER *ubuf;

    ubuf=GWEN_Buffer_new(0, 1024, 0, 1);
    if (GWEN_Args_Usage(args, ubuf, GWEN_ArgsOutType_Txt)) {
      fprintf(stderr, "ERROR: Could not create help string\n");
      return 1;
    }
    fprintf(stderr, "%s\n", GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    return 0;
  }

  keyId=GWEN_DB_GetIntValue(db, "keyId", 0, 0);

  ttype=GWEN_DB_GetCharValue(db, "tokenType", 0, 0);
  assert(ttype);

  tname=GWEN_DB_GetCharValue(db, "tokenName", 0, 0);

  /* get crypt token */
  ct=getCryptToken(ttype, tname);
  if (ct==0)
    return 3;

  if (GWEN_DB_GetIntValue(dbArgs, "forcePin", 0, 0))
    GWEN_Crypt_Token_AddModes(ct, GWEN_CRYPT_TOKEN_MODE_FORCE_PIN_ENTRY);

  /* open crypt token for use */
  rv=GWEN_Crypt_Token_Open(ct, 0, 0);
  if (rv) {
    DBG_ERROR(0, "Could not open token");
    return 3;
  }
  else {
    uint32_t keyIds[64];
    uint32_t keyCount;
    uint32_t i;

    keyCount=64;
    rv=GWEN_Crypt_Token_GetKeyIdList(ct, keyIds, &keyCount, 0);
    if (rv<0) {
      DBG_ERROR(0, "Error filling key list");
      GWEN_Crypt_Token_Close(ct, 0, 0);
      return 3;
    }
    for (i=0; i<keyCount; i++) {
      if (keyId==0 || keyId==keyIds[i]) {
	const GWEN_CRYPT_TOKEN_KEYINFO *ki;
	uint32_t flags;
        const char *s;

	ki=GWEN_Crypt_Token_GetKeyInfo(ct, keyIds[i], 0xffffffff, 0);
	if (ki) {
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

	  shown++;
	}
      }
    }
  }

  /* close crypt token */
  rv=GWEN_Crypt_Token_Close(ct, 0, 0);
  if (rv) {
    DBG_ERROR(0, "Could not close token");
    return 3;
  }

  if (!shown) {
    if (keyId==0) {
      DBG_ERROR(0, "No key found");
    }
    else {
      DBG_ERROR(0, "Key %u not found", keyId);
    }
    return 1;
  }

  return 0;
}



