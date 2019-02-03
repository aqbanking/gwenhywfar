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
#include <gwenhywfar/base64.h>
#include <gwenhywfar/ctplugin.h>




int createToken(GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  const char *ttype;
  const char *tname;
  GWEN_PLUGIN_MANAGER *pm;
  GWEN_PLUGIN *pl;
  GWEN_CRYPT_TOKEN *ct;
  int rv;
  const GWEN_ARGS args[]= {
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

  ttype=GWEN_DB_GetCharValue(db, "tokenType", 0, 0);
  assert(ttype);

  tname=GWEN_DB_GetCharValue(db, "tokenName", 0, 0);

  /* get crypt token */
  pm=GWEN_PluginManager_FindPluginManager(GWEN_CRYPT_TOKEN_PLUGIN_TYPENAME);
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

  ct=GWEN_Crypt_Token_Plugin_CreateToken(pl, tname);
  if (ct==0) {
    DBG_ERROR(0, "Could not create crypt token");
    return 3;
  }

  /* create crypt token */
  rv=GWEN_Crypt_Token_Create(ct, 0);
  if (rv) {
    DBG_ERROR(0, "Could not create token");
    return 3;
  }

  /* close crypt token */
  rv=GWEN_Crypt_Token_Close(ct, 0, 0);
  if (rv) {
    DBG_ERROR(0, "Could not close token");
    return 3;
  }

  return 0;
}



