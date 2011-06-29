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




int verifyArchive(GWEN_DB_NODE *dbArgs, int argc, char **argv) {
  GWEN_DB_NODE *db;
  const char *aname;
  const char *keyFile;
  const char *signer;
  GWEN_DB_NODE *dbKey;
  GWEN_CRYPT_KEY *key;
  GWEN_SAR *sr;
  int rv;
  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT,     /* flags */
    GWEN_ArgsType_Char,               /* type */
    "archive",                        /* name */
    1,                                /* minnum */
    1,                                /* maxnum */
    "a",                              /* short option */
    "archive",                        /* long option */
    "Specify the archive file name",  /* short description */
    "Specify the archive file name"   /* long description */
  },
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
    GWEN_ARGS_FLAGS_HAS_ARGUMENT,     /* flags */
    GWEN_ArgsType_Char,               /* type */
    "signer",                         /* name */
    0,                                /* minnum */
    1,                                /* maxnum */
    "s",                              /* short option */
    "signer",                         /* long option */
    "Specify the signer",             /* short description */
    "Specify the signer"              /* long description */
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

  aname=GWEN_DB_GetCharValue(db, "archive", 0, NULL);
  assert(aname);

  keyFile=GWEN_DB_GetCharValue(db, "keyFile", 0, NULL);
  assert(keyFile);

  signer=GWEN_DB_GetCharValue(db, "keyFile", 0, "Signer");

  dbKey=GWEN_DB_Group_new("keyfile");
  rv=GWEN_DB_ReadFile(dbKey, keyFile, GWEN_DB_FLAGS_DEFAULT);
  if (rv<0) {
    fprintf(stderr, "ERROR: Error reading keyfile [%s] (%d)\n", keyFile, rv);
    return 2;
  }
  key=GWEN_Crypt_KeyRsa_fromDb(dbKey);
  if (key==NULL) {
    fprintf(stderr, "ERROR: Error decoding keyfile [%s] (%d)\n", keyFile, rv);
    return 2;
  }
  GWEN_Crypt_KeyRsa_AddFlags(key, GWEN_CRYPT_KEYRSA_FLAGS_DIRECTSIGN);

  sr=GWEN_Sar_new();
  rv=GWEN_Sar_OpenArchive(sr, aname,
                          GWEN_SyncIo_File_CreationMode_OpenExisting,
                          GWEN_SYNCIO_FILE_FLAGS_READ);
  if (rv<0) {
    fprintf(stderr, "ERROR: Error opening archive (%d)\n", rv);
    return 2;
  }
  else {
    GWEN_CRYPTMGR *cm;

    cm=GWEN_CryptMgrKeys_new(NULL, NULL, signer, key, 1);

    /* verify */
    rv=GWEN_Sar_Verify(sr, cm);
    if (rv<0) {
      fprintf(stderr, "ERROR: Error verifying archive (%d)\n", rv);
      GWEN_CryptMgr_free(cm);
      GWEN_Sar_CloseArchive(sr, 1);
      GWEN_Sar_free(sr);
      return 2;
    }
    GWEN_CryptMgr_free(cm);

    /* close archive */
    rv=GWEN_Sar_CloseArchive(sr, 0);
    if (rv<0) {
      fprintf(stderr, "ERROR: Error closing archive (%d)\n", rv);
      GWEN_Sar_CloseArchive(sr, 1);
      GWEN_Sar_free(sr);
      return 2;
    }

    fprintf(stdout, "Signature is valid.\n");
    return 0;
  }
}



