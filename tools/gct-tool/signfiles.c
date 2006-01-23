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



static int signFile(GWEN_CRYPTTOKEN *ct,
                    GWEN_CRYPTTOKEN_CONTEXT *ctx,
                    const char *inFile,
                    FILE *f) {
  GWEN_BUFFER *fbuf;
  GWEN_BUFFER *sigbuf;
  GWEN_BUFFER *b64buf;
  GWEN_BUFFEREDIO *bio;
  int fd;
  int rv;

  fd=open(inFile, O_RDONLY);
  if (fd==-1) {
    DBG_ERROR(GWEN_LOGDOMAIN, "ERROR on open(%s): %s",
              inFile, strerror(errno));
    return 1;
  }

  bio=GWEN_BufferedIO_File_new(fd);
  assert(bio);
  GWEN_BufferedIO_SetReadBuffer(bio, 0, 1024);

  fbuf=GWEN_Buffer_new(0, 1025, 0, 1);

  while(1) {
    GWEN_ERRORCODE err;
    char buffer[1024];
    unsigned int bsize;

    bsize=sizeof(buffer);
    err=GWEN_BufferedIO_ReadRaw(bio, buffer, &bsize);
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR_ERR(0, err);
      GWEN_BufferedIO_Abandon(bio);
      GWEN_BufferedIO_free(bio);
      return 1;
    }
    if (bsize==0)
      break;
    GWEN_Buffer_AppendBytes(fbuf, buffer, bsize);
  }
  GWEN_BufferedIO_Close(bio);
  GWEN_BufferedIO_free(bio);

  GWEN_Buffer_Rewind(fbuf);

  sigbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_CryptToken_Sign(ct, ctx,
                          GWEN_Buffer_GetStart(fbuf),
                          GWEN_Buffer_GetUsedBytes(fbuf),
                          sigbuf);
  if (rv) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not sign file data");
    return 3;
  }
  GWEN_Buffer_free(fbuf);

  b64buf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_Base64_Encode((unsigned char*)(GWEN_Buffer_GetStart(sigbuf)),
                        GWEN_Buffer_GetUsedBytes(sigbuf),
                        b64buf, 0);
  if (rv) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not base64-encode signature");
    GWEN_Buffer_free(b64buf);
    GWEN_Buffer_free(sigbuf);
    return 3;
  }
  GWEN_Buffer_free(sigbuf);

  fprintf(f, "%s\t%s\n", inFile, GWEN_Buffer_GetStart(b64buf));
  GWEN_Buffer_free(b64buf);

  return 0;
}





int signFiles(GWEN_DB_NODE *dbArgs, int argc, char **argv) {
  GWEN_DB_NODE *db;
  const char *outFile;
  const char *inFile;
  const char *ttype;
  const char *tname;
  const char *padding;
  const char *hashing;
  int context;
  GWEN_CRYPTTOKEN_HASHALGO ha=GWEN_CryptToken_HashAlgo_RMD160;
  GWEN_CRYPTTOKEN_PADDALGO pa=GWEN_CryptToken_PaddAlgo_ANSIX9_23;
  GWEN_PLUGIN_MANAGER *pm;
  GWEN_PLUGIN *pl;
  GWEN_CRYPTTOKEN *ct;
  GWEN_CRYPTTOKEN_CONTEXT *ctx;
  int rv;
  int i;
  FILE *f;
  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeChar,            /* type */
    "outfile",                    /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "o",                          /* short option */
    "outfile",                    /* long option */
    "Specify the output file",     /* short description */
    "Specify the output file"      /* long description */
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
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeChar,            /* type */
    "context",                    /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "c",                          /* short option */
    "context",                    /* long option */
    "Specify the crypt token context",     /* short description */
    "Specify the crypt token context"      /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeChar,            /* type */
    "paddAlgo",                   /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "padding",                    /* long option */
    "Specify the padding to be used",     /* short description */
    "Specify the padding to be used"      /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeChar,            /* type */
    "hashAlgo",                   /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    0,                            /* short option */
    "hashing",                    /* long option */
    "Specify the hashing to be used",     /* short description */
    "Specify the hashing to be used"      /* long description */
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

  /* check for input files */
  inFile=GWEN_DB_GetCharValue(db, "params", 0, 0);
  if (!inFile) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No input files specified");
    return 1;
  }

  /* open output file, if necessary */
  outFile=GWEN_DB_GetCharValue(db, "outFile", 0, 0);
  if (outFile && *outFile) {
    f=fopen(outFile, "w+");
    if (!f) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "ERROR on fopen(%s): %s",
                outFile,
                strerror(errno));
      return 2;
    }
  }
  else
    f=stdout;

  ttype=GWEN_DB_GetCharValue(db, "tokenType", 0, 0);
  assert(ttype);

  tname=GWEN_DB_GetCharValue(db, "tokenName", 0, 0);
  hashing=GWEN_DB_GetCharValue(db, "hashalgo", 0, "rmd160");
  if (hashing) {
    ha=GWEN_CryptToken_HashAlgo_fromString(hashing);
    if (ha==GWEN_CryptToken_HashAlgo_Unknown) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Unknown hashing algo \"%s\"",
                hashing);
      return 1;
    }
  }
  padding=GWEN_DB_GetCharValue(db, "paddalgo", 0, "ansix9_23");
  if (padding) {
    pa=GWEN_CryptToken_PaddAlgo_fromString(padding);
    if (pa==GWEN_CryptToken_PaddAlgo_Unknown) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Unknown padding algo \"%s\"",
                padding);
      return 1;
    }
  }


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

  /* get context */
  context=GWEN_DB_GetIntValue(db, "context", 0, 0);
  if (context==0) {
    GWEN_CRYPTTOKEN_CONTEXT_LIST *ctxl;
    const GWEN_CRYPTTOKEN_CONTEXT *cctx;

    /* select the first context */
    ctxl=GWEN_CryptToken_Context_List_new();
    rv=GWEN_CryptToken_FillContextList(ct, ctxl);
    if (rv) {
      DBG_ERROR(0, "Could not get context list");
      GWEN_CryptToken_Close(ct);
      return 3;
    }

    cctx=GWEN_CryptToken_Context_List_First(ctxl);
    if (!cctx) {
      DBG_ERROR(GWEN_LOGDOMAIN, "No context found");
      GWEN_CryptToken_Close(ct);
      return 3;
    }
    ctx=GWEN_CryptToken_Context_dup(cctx);
  }
  else {
    const GWEN_CRYPTTOKEN_CONTEXT *cctx;

    cctx=GWEN_CryptToken_GetContextById(ct, context);
    if (!cctx) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Context %d found", context);
      GWEN_CryptToken_Close(ct);
      return 3;
    }
    ctx=GWEN_CryptToken_Context_dup(cctx);
  }
  assert(ctx);

  if (1) {
    const GWEN_CRYPTTOKEN_SIGNINFO *csi;
    GWEN_CRYPTTOKEN_SIGNINFO *si;

    csi=GWEN_CryptToken_Context_GetSignInfo(ctx);
    if (!csi) {
      DBG_ERROR(GWEN_LOGDOMAIN, "No sign info stored in context %d",
                GWEN_CryptToken_Context_GetId(ctx));
      GWEN_CryptToken_Close(ct);
      return 3;
    }

    si=GWEN_CryptToken_SignInfo_dup(csi);
    GWEN_CryptToken_SignInfo_SetHashAlgo(si, ha);
    GWEN_CryptToken_SignInfo_SetPaddAlgo(si, pa);

    GWEN_CryptToken_Context_SetSignInfo(ctx, si);
    GWEN_CryptToken_SignInfo_free(si);
  }

  /* sign every input file */
  for (i=0; ; i++) {
    inFile=GWEN_DB_GetCharValue(db, "params", i, 0);
    if (!inFile)
      break;
    rv=signFile(ct, ctx, inFile, f);
    if (rv) {
      return 3;
    }
  }

  /* close output file if necessary */
  if (outFile) {
    if (fclose(f)) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "ERROR on fclose(%s): %s",
                outFile,
                strerror(errno));
      return 2;
    }
  }

  return 0;
}



