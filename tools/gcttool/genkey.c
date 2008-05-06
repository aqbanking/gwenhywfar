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






int genKey(GWEN_DB_NODE *dbArgs, int argc, char **argv) {
  GWEN_DB_NODE *db;
  const char *ttype;
  const char *tname;
  GWEN_CRYPT_TOKEN *ct;
  unsigned int keyId;
  unsigned int keySize;
  GWEN_CRYPT_CRYPTALGOID algoId;
  int rv;
  const char *s;
  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Int,            /* type */
    "keyId",                      /* name */
    1,                            /* minnum */
    1,                            /* maxnum */
    "k",                          /* short option */
    "key",                        /* long option */
    "Key id",                     /* short description */
    "Key id"                      /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
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
    GWEN_ArgsType_Char,           /* type */
    "tokenName",                  /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "n",                          /* short option */
    "tname",                      /* long option */
    "Specify the crypt token name",     /* short description */
    "Specify the crypt token name"      /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "algo",                       /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "a",                          /* short option */
    "algo",                       /* long option */
    "Specify the algorithm",      /* short description */
    "Specify the algorithm"       /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Int,            /* type */
    "keysize",                    /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "s",                          /* short option */
    "keysize",                    /* long option */
    "Key size in bytes",          /* short description */
    "Key size in bytes"           /* long description */
  },
  {
    0,                            /* flags */
    GWEN_ArgsType_Int,            /* type */
    "exp65537",                   /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "e",                          /* short option */
    "exp65537",                   /* long option */
    "Use default exponent of 65537", /* short description */
    "Use default exponent of 65537"  /* long description */
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
  if (keyId==0) {
    DBG_ERROR(0, "Key Id must not be zero");
    return 1;
  }

  s=GWEN_DB_GetCharValue(db, "algo", 0, "rsa");
  if (!s) {
    DBG_ERROR(0, "Algo id missing");
    return 1;
  }
  algoId=GWEN_Crypt_CryptAlgoId_fromString(s);
  if (algoId==GWEN_Crypt_CryptAlgoId_Unknown) {
    DBG_ERROR(0, "Bad algo id [%s]", s);
    return 1;
  }

  keySize=GWEN_DB_GetIntValue(db, "keySize", 0, 96);
  if (keySize==0) {
    DBG_ERROR(0, "Invalid key size %d", keySize);
    return 1;
  }

  ttype=GWEN_DB_GetCharValue(db, "tokenType", 0, 0);
  assert(ttype);

  tname=GWEN_DB_GetCharValue(db, "tokenName", 0, 0);

  /* get crypt token */
  ct=getCryptToken(ttype, tname);
  if (ct==0)
    return 3;

  if (GWEN_DB_GetIntValue(dbArgs, "forcePin", 0, 0))
    GWEN_Crypt_Token_AddModes(ct, GWEN_CRYPT_TOKEN_MODE_FORCE_PIN_ENTRY);

  if (GWEN_DB_GetIntValue(db, "exp65537", 0, 0))
    GWEN_Crypt_Token_AddModes(ct, GWEN_CRYPT_TOKEN_MODE_EXP_65537);

  /* open crypt token for use */
  rv=GWEN_Crypt_Token_Open(ct, 1, 0);
  if (rv) {
    DBG_ERROR(0, "Could not open token");
    return 3;
  }
  else {
    GWEN_CRYPT_CRYPTALGO *algo;

    algo=GWEN_Crypt_CryptAlgo_new(algoId, GWEN_Crypt_CryptMode_None);
    GWEN_Crypt_CryptAlgo_SetChunkSize(algo, keySize);

    /* generate key */
    rv=GWEN_Crypt_Token_GenerateKey(ct, keyId, algo, 0);
    if (rv) {
      DBG_ERROR(GWEN_LOGDOMAIN,
		"Error generating key (%d)", rv);
      GWEN_Crypt_CryptAlgo_free(algo);
      return 3;
    }
  }

  /* close crypt token */
  rv=GWEN_Crypt_Token_Close(ct, 0, 0);
  if (rv) {
    DBG_ERROR(0, "Could not close token");
    return 3;
  }

  fprintf(stderr, "Key %d (%s, %d bytes) successfully generated.\n",
	  keyId, GWEN_Crypt_CryptAlgoId_toString(algoId), keySize);

  return 0;
}



