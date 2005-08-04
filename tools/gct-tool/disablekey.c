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



int disableKey(GWEN_DB_NODE *dbArgs, int argc, char **argv) {
  GWEN_DB_NODE *db;
  const char *ttype;
  const char *tname;
  GWEN_PLUGIN_MANAGER *pm;
  GWEN_PLUGIN *pl;
  GWEN_CRYPTTOKEN *ct;
  GWEN_TYPE_UINT32 kid;
  int rv;
  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeInt,             /* type */
    "keyId",                      /* name */
    1,                            /* minnum */
    1,                            /* maxnum */
    "i",                          /* short option */
    "id",                         /* long option */
    "Key id",                     /* short description */
    "Key id"                      /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeChar,            /* type */
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
    GWEN_ArgsTypeChar,            /* type */
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
    GWEN_ArgsTypeInt,             /* type */
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
    if (GWEN_Args_Usage(args, ubuf, GWEN_ArgsOutTypeTXT)) {
      fprintf(stderr, "ERROR: Could not create help string\n");
      return 1;
    }
    fprintf(stderr, "%s\n", GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    return 0;
  }

  kid=GWEN_DB_GetIntValue(db, "keyId", 0, 0);

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

  /* open crypt token for use */
  rv=GWEN_CryptToken_Open(ct, 1);
  if (rv) {
    DBG_ERROR(0, "Could not open token");
    return 3;
  }
  else {
    GWEN_KEYSPEC *ks=0;

    /* get key */
    rv=GWEN_CryptToken_ReadKeySpec(ct, kid, &ks);
    if (rv) {
      if (rv==GWEN_ERROR_NO_DATA) {
        DBG_ERROR(0, "No keyspec for key %d", kid);
        GWEN_CryptToken_Close(ct);
        return 3;
      }
      else {
        DBG_ERROR(0, "Could not read key spec (%d)", rv);
        GWEN_CryptToken_Close(ct);
        return 3;
      }
    }
    assert(ks);
    GWEN_KeySpec_SetStatus(ks, GWEN_CRYPTTOKEN_KEYSTATUS_FREE);
    rv=GWEN_CryptToken_WriteKeySpec(ct, kid, ks);
    if (rv) {
      DBG_ERROR(0, "Could not write key spec (%d)", rv);
      GWEN_KeySpec_free(ks);
      GWEN_CryptToken_Close(ct);
      return 3;
    }
    GWEN_KeySpec_free(ks);
  }

  /* close crypt token */
  rv=GWEN_CryptToken_Close(ct);
  if (rv) {
    DBG_ERROR(0, "Could not close token");
    return 3;
  }

  return 0;
}



