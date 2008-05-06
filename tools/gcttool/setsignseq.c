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






int setSignSeq(GWEN_DB_NODE *dbArgs, int argc, char **argv) {
  GWEN_DB_NODE *db;
  const char *ttype;
  const char *tname;
  GWEN_CRYPT_TOKEN *ct;
  unsigned int keyId;
  unsigned int seq;
  int rv;
  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Int,             /* type */
    "keyId",                      /* name */
    1,                            /* minnum */
    1,                            /* maxnum */
    "k",                          /* short option */
    "key",                        /* long option */
    "Key id (0 for any)",         /* short description */
    "Key id (0 for any)"          /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Int,             /* type */
    "seq",                         /* name */
    1,                            /* minnum */
    1,                            /* maxnum */
    "s",                          /* short option */
    "seq",                        /* long option */
    "New sequence counter value",         /* short description */
    "New sequence counter value"          /* long description */
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
  seq=GWEN_DB_GetIntValue(db, "seq", 0, 0);

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
    const GWEN_CRYPT_TOKEN_KEYINFO *ki;

    ki=GWEN_Crypt_Token_GetKeyInfo(ct, keyId,
				   GWEN_CRYPT_TOKEN_KEYFLAGS_HASSIGNCOUNTER,
				   0);
    if (ki) {
      GWEN_CRYPT_TOKEN_KEYINFO *nki;

      nki=GWEN_Crypt_Token_KeyInfo_dup(ki);
      GWEN_Crypt_Token_KeyInfo_SetSignCounter(nki, seq);
      rv=GWEN_Crypt_Token_SetKeyInfo(ct, keyId, nki, 0);
      GWEN_Crypt_Token_KeyInfo_free(nki);
      if (rv) {
	DBG_ERROR(GWEN_LOGDOMAIN, "Unable to set sign counter (%d)", rv);
	return 4;
      }
    }
  }

  /* close crypt token */
  rv=GWEN_Crypt_Token_Close(ct, 0, 0);
  if (rv) {
    DBG_ERROR(0, "Could not close token");
    return 3;
  }

  return 0;
}



