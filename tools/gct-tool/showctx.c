/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
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
#include <gwenhywfar/bio_file.h>
#include <gwenhywfar/base64.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>



static void _dumpKeyInfo(const GWEN_CRYPTTOKEN_KEYINFO *ki, const char *name){
  GWEN_TYPE_UINT32 flags;
  const char *s;

  fprintf(stdout, "  %s: %04x, %d bits, %d bytes, %s [",
          name,
          (unsigned int)GWEN_CryptToken_KeyInfo_GetKeyId(ki),
          GWEN_CryptToken_KeyInfo_GetKeySize(ki),
          GWEN_CryptToken_KeyInfo_GetChunkSize(ki),
          GWEN_CryptToken_CryptAlgo_toString(GWEN_CryptToken_KeyInfo_GetCryptAlgo(ki)));
  flags=GWEN_CryptToken_KeyInfo_GetKeyFlags(ki);
  if (flags & GWEN_CRYPTTOKEN_KEYINFO_FLAGS_CAN_SIGN)
    fprintf(stdout, "S");
  if (flags & GWEN_CRYPTTOKEN_KEYINFO_FLAGS_CAN_VERIFY)
    fprintf(stdout, "V");
  if (flags & GWEN_CRYPTTOKEN_KEYINFO_FLAGS_CAN_ENCRYPT)
    fprintf(stdout, "E");
  if (flags & GWEN_CRYPTTOKEN_KEYINFO_FLAGS_CAN_DECRYPT)
    fprintf(stdout, "D");
  fprintf(stdout, "]");
  s=GWEN_CryptToken_KeyInfo_GetDescription(ki);
  if (s)
    fprintf(stdout, " (%s)", s);
  fprintf(stdout, "\n");
}



int showCtx(GWEN_DB_NODE *dbArgs, int argc, char **argv) {
  GWEN_DB_NODE *db;
  const char *ttype;
  const char *tname;
  GWEN_PLUGIN_MANAGER *pm;
  GWEN_PLUGIN *pl;
  GWEN_CRYPTTOKEN *ct;
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
  pm=GWEN_PluginManager_FindPluginManager("crypttoken");
  if (pm==0) {
    DBG_ERROR(0, "Plugin manager not found");
    return 3;
  }

  pl=GWEN_PluginManager_GetPlugin(pm, ttype);
  if (pl==0) {
    DBG_ERROR(0, "Plugin not found");
    return 3;
  }
  DBG_ERROR(0, "Plugin found");

  ct=GWEN_CryptToken_Plugin_CreateToken(pl, 0, tname);
  if (ct==0) {
    DBG_ERROR(0, "Could not create crypt token");
    return 3;
  }

  if (GWEN_DB_GetIntValue(dbArgs, "forcePin", 0, 0))
    GWEN_CryptToken_AddModes(ct, GWEN_CRYPTTOKEN_MODES_FORCE_PIN_ENTRY);

  /* open crypt token for use */
  rv=GWEN_CryptToken_Open(ct, 0);
  if (rv) {
    DBG_ERROR(0, "Could not open token");
    return 3;
  }
  else {
    GWEN_CRYPTTOKEN_CONTEXT_LIST *l;
    GWEN_CRYPTTOKEN_CONTEXT *ctx;

    l=GWEN_CryptToken_Context_List_new();
    rv=GWEN_CryptToken_FillContextList(ct, l);
    if (rv) {
      DBG_ERROR(0, "Error filling context list");
      GWEN_CryptToken_Context_List_free(l);
      GWEN_CryptToken_Close(ct);
      return 3;
    }

    ctx=GWEN_CryptToken_Context_List_First(l);
    while(ctx) {
      if (cid==0 || cid==GWEN_CryptToken_Context_GetId(ctx)) {
        const char *s;
        const GWEN_CRYPTTOKEN_KEYINFO *ki;
        const GWEN_CRYPTTOKEN_SIGNINFO *si;
        const GWEN_CRYPTTOKEN_CRYPTINFO *ci;

        fprintf(stdout, "-------------------------------------------------\n");
         fprintf(stdout, "Context %u",
                (unsigned int)GWEN_CryptToken_Context_GetId(ctx));
        s=GWEN_CryptToken_Context_GetDescription(ctx);
        if (s)
          fprintf(stdout, " (%s)", s);
        fprintf(stdout, "\n");
  
        ki=GWEN_CryptToken_Context_GetSignKeyInfo(ctx);
        if (ki)
          _dumpKeyInfo(ki, "Sign Key   ");
  
        ki=GWEN_CryptToken_Context_GetVerifyKeyInfo(ctx);
        if (ki)
          _dumpKeyInfo(ki, "Verify Key ");
  
        ki=GWEN_CryptToken_Context_GetEncryptKeyInfo(ctx);
        if (ki)
          _dumpKeyInfo(ki, "Encrypt Key");
  
        ki=GWEN_CryptToken_Context_GetDecryptKeyInfo(ctx);
        if (ki)
          _dumpKeyInfo(ki, "Decrypt Key");
  
        si=GWEN_CryptToken_Context_GetSignInfo(ctx);
        if (si) {
          fprintf(stdout, "  Sign Info: %d, %s, %s\n",
                  (unsigned int)GWEN_CryptToken_SignInfo_GetId(si),
                  GWEN_CryptToken_HashAlgo_toString(GWEN_CryptToken_SignInfo_GetHashAlgo(si)),
                  GWEN_CryptToken_PaddAlgo_toString(GWEN_CryptToken_SignInfo_GetPaddAlgo(si)));
        }
  
        ci=GWEN_CryptToken_Context_GetCryptInfo(ctx);
        if (ci) {
          fprintf(stdout, "  Crypt Info: %d, %s, %s\n",
                  (unsigned int)GWEN_CryptToken_CryptInfo_GetId(ci),
                  GWEN_CryptToken_CryptAlgo_toString(GWEN_CryptToken_CryptInfo_GetCryptAlgo(ci)),
                  GWEN_CryptToken_PaddAlgo_toString(GWEN_CryptToken_CryptInfo_GetPaddAlgo(ci)));
        }
        shown++;
      }

      ctx=GWEN_CryptToken_Context_List_Next(ctx);
    } /* while */
    GWEN_CryptToken_Context_List_free(l);
  }


  /* close crypt token */
  rv=GWEN_CryptToken_Close(ct);
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



