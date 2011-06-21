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






int showUser(GWEN_DB_NODE *dbArgs, int argc, char **argv) {
  GWEN_DB_NODE *db;
  const char *ttype;
  const char *tname;
  GWEN_CRYPT_TOKEN *ct;
  unsigned int cid;
  int shown=0;
  int rv;
  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Int,             /* type */
    "contextId",                  /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "i",                          /* short option */
    "id",                         /* long option */
    "Context id (0 for any)",     /* short description */
    "Context id (0 for any)"      /* long description */
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

  cid=GWEN_DB_GetIntValue(db, "contextId", 0, 0);

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
    uint32_t ctxIds[32];
    uint32_t ctxCount;
    uint32_t i;

    ctxCount=32;
    rv=GWEN_Crypt_Token_GetContextIdList(ct, ctxIds, &ctxCount, 0);
    if (rv<0) {
      DBG_ERROR(0, "Error filling context list");
      GWEN_Crypt_Token_Close(ct, 0, 0);
      return 3;
    }
    for (i=0; i<ctxCount; i++) {
      if (cid==0 || cid==ctxIds[i]) {
	const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
	const char *s;
        uint32_t kid;

	ctx=GWEN_Crypt_Token_GetContext(ct, ctxIds[i], 0);
	if (ctx) {
	  fprintf(stdout, "-------------------------------------------------\n");
	  fprintf(stdout, "Context %u\n",
		  (unsigned int)GWEN_Crypt_Token_Context_GetId(ctx));
	  s=GWEN_Crypt_Token_Context_GetServiceId(ctx);
	  if (s)
	    fprintf(stdout, "Service        : %s\n", s);
	  s=GWEN_Crypt_Token_Context_GetUserId(ctx);
	  if (s)
	    fprintf(stdout, "User Id        : %s\n", s);
	  s=GWEN_Crypt_Token_Context_GetCustomerId(ctx);
	  if (s)
	    fprintf(stdout, "Customer Id    : %s\n", s);
	  s=GWEN_Crypt_Token_Context_GetUserName(ctx);
	  if (s)
	    fprintf(stdout, "User Name      : %s\n", s);
	  s=GWEN_Crypt_Token_Context_GetPeerId(ctx);
	  if (s)
	    fprintf(stdout, "Peer Id        : %s\n", s);
	  s=GWEN_Crypt_Token_Context_GetPeerName(ctx);
	  if (s)
	    fprintf(stdout, "Peer Name      : %s\n", s);
	  s=GWEN_Crypt_Token_Context_GetAddress(ctx);
	  if (s)
	    fprintf(stdout, "Address        : %s\n", s);
	  fprintf(stdout, "Port           : %d\n", GWEN_Crypt_Token_Context_GetPort(ctx));
	  s=GWEN_Crypt_Token_Context_GetSystemId(ctx);
	  if (s)
	    fprintf(stdout, "System Id      : %s\n", s);
	  kid=GWEN_Crypt_Token_Context_GetSignKeyId(ctx);
	  if (kid!=0)
	    fprintf(stdout, "Sign Key Id    : %d\n", kid);
	  kid=GWEN_Crypt_Token_Context_GetVerifyKeyId(ctx);
	  if (kid!=0)
	    fprintf(stdout, "Verify Key Id  : %d\n", kid);
	  kid=GWEN_Crypt_Token_Context_GetEncipherKeyId(ctx);
	  if (kid!=0)
	    fprintf(stdout, "Encipher Key Id: %d\n", kid);
	  kid=GWEN_Crypt_Token_Context_GetDecipherKeyId(ctx);
	  if (kid!=0)
	    fprintf(stdout, "Decipher Key Id: %d\n", kid);

	  shown++;
	}
	else {
          fprintf(stderr, "Context %d not found (%d)\n", i, ctxIds[i]);
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
    if (cid==0) {
      DBG_ERROR(0, "No context found");
    }
    else {
      DBG_ERROR(0, "Context %u not found", cid);
    }
    return 1;
  }

  return 0;

}



