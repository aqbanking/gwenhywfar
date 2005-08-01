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



int checkToken(GWEN_DB_NODE *dbArgs, int argc, char **argv) {
  GWEN_DB_NODE *db;
  const char *dTypeName;
  const char *tname;
  GWEN_PLUGIN_MANAGER *pm;
  GWEN_BUFFER *bufTypeName;
  GWEN_BUFFER *bufSubTypeName;
  GWEN_BUFFER *bufTokenName;
  GWEN_CRYPTTOKEN_DEVICE devt;
  int rv;

  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeChar,            /* type */
    "deviceType",                 /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "d",                          /* short option */
    "dtype",                    /* long option */
    "Specify the device type",     /* short description */
    "Specify the device type (card, file, none)"   /* long description */
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

  tname=GWEN_DB_GetCharValue(db, "tokenName", 0, 0);
  if (tname)
    dTypeName=GWEN_DB_GetCharValue(db, "deviceType", 0, "file");
  else
    dTypeName=GWEN_DB_GetCharValue(db, "deviceType", 0, "card");
  assert(dTypeName);

  /* get crypt token */
  pm=GWEN_PluginManager_FindPluginManager("crypttoken");
  if (pm==0) {
    DBG_ERROR(0, "Plugin manager not found");
    return 3;
  }

  devt=GWEN_CryptToken_Device_fromString(dTypeName);
  if (devt==GWEN_CryptToken_Device_Unknown) {
    DBG_ERROR(0, "Unknown device type name \"%s\"", dTypeName);
    return 1;
  }

  bufTypeName=GWEN_Buffer_new(0, 256, 0, 1);
  bufSubTypeName=GWEN_Buffer_new(0, 256, 0, 1);
  bufTokenName=GWEN_Buffer_new(0, 256, 0, 1);

  if (tname)
    GWEN_Buffer_AppendString(bufTokenName, tname);

  rv=GWEN_CryptManager_CheckToken(pm, devt,
                                  bufTypeName,
                                  bufSubTypeName,
                                  bufTokenName);
  if (rv) {
    DBG_ERROR(0, "Token is not supported by any plugin");
    GWEN_Buffer_free(bufTokenName);
    GWEN_Buffer_free(bufSubTypeName);
    GWEN_Buffer_free(bufTypeName);
    return 3;
  }

  fprintf(stdout, "Token is supported:\n");
  fprintf(stdout, "  Type Name    : %s\n",
          GWEN_Buffer_GetStart(bufTypeName));
  fprintf(stdout, "  Subtype Name : %s\n",
          GWEN_Buffer_GetStart(bufSubTypeName));
  fprintf(stdout, "  Token Name   : %s\n",
          GWEN_Buffer_GetStart(bufTokenName));

  GWEN_Buffer_free(bufTokenName);
  GWEN_Buffer_free(bufSubTypeName);
  GWEN_Buffer_free(bufTypeName);

  return 0;
}



