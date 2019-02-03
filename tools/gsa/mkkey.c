/***************************************************************************
 begin       : Sat Jun 25 2011
 copyright   : (C) 2011 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "globals.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/sar.h>
#include <gwenhywfar/cryptmgrkeys.h>
#include <gwenhywfar/cryptkeyrsa.h>




int mkArchiveKey(GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  const char *keyFile;
  GWEN_DB_NODE *dbKey;
  GWEN_CRYPT_KEY *pubKey;
  GWEN_CRYPT_KEY *privKey;
  int rv;
  const GWEN_ARGS args[]= {
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT,     /* flags */
      GWEN_ArgsType_Char,               /* type */
      "keyfile",                        /* name */
      1,                                /* minnum */
      1,                                /* maxnum */
      "k",                              /* short option */
      "keyfile",                        /* long option */
      "Specify the keyfile to use",     /* short description */
      "Specify the keyfile to use"      /* long description */
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

  keyFile=GWEN_DB_GetCharValue(db, "keyFile", 0, NULL);
  assert(keyFile);

  /* create key */
  rv=GWEN_Crypt_KeyRsa_GeneratePair(512, 1, &pubKey, &privKey);
  if (rv<0) {
    fprintf(stderr, "ERROR: Error generating key pair (%d)\n", rv);
    return rv;
  }

  dbKey=GWEN_DB_Group_new("keyfile");
  rv=GWEN_Crypt_KeyRsa_toDb(privKey, dbKey, 0);
  if (rv<0) {
    fprintf(stderr, "ERROR: Error encoding keyfile [%s] (%d)\n", keyFile, rv);
    return 2;
  }

  rv=GWEN_DB_WriteFile(dbKey, keyFile, GWEN_DB_FLAGS_DEFAULT);
  if (rv<0) {
    fprintf(stderr, "ERROR: Error writing keyfile [%s] (%d)\n", keyFile, rv);
    return 2;
  }

  GWEN_DB_Group_free(dbKey);
  GWEN_Crypt_Key_free(privKey);
  GWEN_Crypt_Key_free(pubKey);

  return 0;
}



