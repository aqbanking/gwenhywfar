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



int readKey(GWEN_DB_NODE *dbArgs, int argc, char **argv) {
  GWEN_DB_NODE *db;
  const char *ttype;
  const char *tname;
  GWEN_PLUGIN_MANAGER *pm;
  GWEN_PLUGIN *pl;
  GWEN_CRYPTTOKEN *ct;
  GWEN_TYPE_UINT32 kid;
  int rv;
  const char *outFile;
  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Int,             /* type */
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
    GWEN_ArgsType_Char,            /* type */
    "outFile",                    /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "o",                          /* short option */
    "outfile",                    /* long option */
    "Specify the output file",    /* short description */
    "Specify the output file"     /* long description */
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

  kid=GWEN_DB_GetIntValue(db, "keyId", 0, 0);

  outFile=GWEN_DB_GetCharValue(db, "outfile", 0, 0);

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
    GWEN_CRYPTKEY *key=0;
    GWEN_DB_NODE *db;
    GWEN_ERRORCODE err;
    int fd;

    /* get key */
    rv=GWEN_CryptToken_ReadKey(ct, kid, &key);
    if (rv) {
      if (rv==GWEN_ERROR_NO_DATA) {
        DBG_ERROR(0, "No data for key %d", kid);
        return 3;
      }
      else {
        DBG_ERROR(0, "Could not read key (%d)", rv);
        return 3;
      }
    }
    assert(key);

    db=GWEN_DB_Group_new("key");
    err=GWEN_CryptKey_toDb(key, db, 1);
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR_ERR(0, err);
      GWEN_DB_Group_free(db);
      return 3;
    }

    /* write key to file */
    if (outFile==0)
      fd=fileno(stdout);
    else
      fd=open(outFile, O_RDWR | O_CREAT | O_TRUNC,
              S_IRUSR | S_IWUSR
#ifdef S_IRGRP
              | S_IRGRP
#endif
#ifdef S_IWGRP
              | S_IWGRP
#endif
             );
    if (fd<0) {
      DBG_ERROR(0, "Error selecting output file: %s",
                strerror(errno));
      return 3;
    }
    else {
      GWEN_BUFFEREDIO *bio;

      /* write key */
      bio=GWEN_BufferedIO_File_new(fd);
      if (!outFile)
        GWEN_BufferedIO_SubFlags(bio, GWEN_BUFFEREDIO_FLAGS_CLOSE);
      GWEN_BufferedIO_SetWriteBuffer(bio, 0, 1024);
      if (GWEN_DB_WriteToStream(db, bio, GWEN_DB_FLAGS_DEFAULT)) {
        DBG_ERROR(0, "Error writing key");
        GWEN_DB_Group_free(db);
        GWEN_BufferedIO_Abandon(bio);
        GWEN_BufferedIO_free(bio);
        return 3;
      }
      err=GWEN_BufferedIO_Close(bio);
      if (!GWEN_Error_IsOk(err)) {
        DBG_ERROR_ERR(0, err);
        GWEN_DB_Group_free(db);
        GWEN_BufferedIO_Abandon(bio);
        GWEN_BufferedIO_free(bio);
        return 3;
      }
      GWEN_BufferedIO_free(bio);
    }
    GWEN_DB_Group_free(db);
  }

  /* close crypt token */
  rv=GWEN_CryptToken_Close(ct);
  if (rv) {
    DBG_ERROR(0, "Could not close token");
    return 3;
  }

  return 0;
}



