/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id: main.c 1107 2007-01-07 21:17:05Z martin $
 begin       : Tue May 03 2005
 copyright   : (C) 2005 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gwenhywfar/debug.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/cgui.h>

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/args.h>
#include <gwenhywfar/logger.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/misc2.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/ct.h>
#include <gwenhywfar/ctplugin.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/mdigest.h>

#include "src/base/i18n_l.h"

#ifdef OS_WIN32
# define DIRSEP "\\"
#else
# define DIRSEP "/"
#endif

#ifdef HAVE_I18N
# include <libintl.h>
# include <locale.h>
#endif


#define GCT_LOGDOMAIN "GCT"

//#define DEBUG_GCT_TOOL



int signWithNew(GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  const char *ttype;
  const char *tname;
  GWEN_PLUGIN_MANAGER *pm;
  GWEN_PLUGIN *pl;
  GWEN_CRYPT_TOKEN *ct;
  /* unsigned int cid; */
  int rv;
  const GWEN_ARGS args[]= {
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

  /* cid=GWEN_DB_GetIntValue(db, "contextId", 0, 0); */

  ttype=GWEN_DB_GetCharValue(db, "tokenType", 0, 0);
  assert(ttype);

  tname=GWEN_DB_GetCharValue(db, "tokenName", 0, 0);

  /* get crypt token */
  pm=GWEN_PluginManager_FindPluginManager("ct");
  if (pm==0) {
    DBG_ERROR(0, "Plugin manager not found");
    return 3;
  }

  pl=GWEN_PluginManager_GetPlugin(pm, ttype);
  if (pl==0) {
    DBG_ERROR(0, "Plugin not found");
    return 3;
  }
  DBG_INFO(0, "Plugin found");

  ct=GWEN_Crypt_Token_Plugin_CreateToken(pl, tname);
  if (ct==0) {
    DBG_ERROR(0, "Could not create crypt token");
    return 3;
  }

  if (GWEN_DB_GetIntValue(dbArgs, "forcePin", 0, 0))
    GWEN_Crypt_Token_AddModes(ct, GWEN_CRYPT_TOKEN_MODE_FORCE_PIN_ENTRY);

  /* open crypt token for use */
  rv=GWEN_Crypt_Token_Open(ct, 0, 0);
  if (rv) {
    DBG_ERROR(0, "Could not open token");
    return 3;
  }
  else {
    uint8_t clearText[20]= {
      0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
      0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
      0x11, 0x12, 0x13, 0x14
    };
    uint8_t signature[256];
    uint32_t signLen;
    GWEN_CRYPT_PADDALGO *algo;

    algo=GWEN_Crypt_PaddAlgo_new(GWEN_Crypt_PaddAlgoId_Iso9796_1A4);
    signLen=sizeof(signature);
    rv=GWEN_Crypt_Token_Sign(ct,
                             0x81, /* local sign key */
                             algo,
                             clearText,
                             20,
                             signature,
                             &signLen,
                             NULL,
                             0);
    if (rv) {
      DBG_ERROR(0, "Could not sign data (%d)", rv);
      return 3;
    }

    fprintf(stderr, "Signature is:\n");
    GWEN_Text_DumpString((const char *) signature, signLen, 2);

    rv=GWEN_Crypt_Token_Verify(ct, 0x81,
                               algo,
                               clearText,
                               20,
                               signature,
                               signLen,
                               0,
                               0);
    if (rv) {
      DBG_ERROR(0, "Could not verify data (%d)", rv);
      return 3;
    }
    fprintf(stderr, "Signature is ok.\n");

  }

  /* close crypt token */
  rv=GWEN_Crypt_Token_Close(ct, 0, 0);
  if (rv) {
    DBG_ERROR(0, "Could not close token");
    return 3;
  }

  return 0;
}



int cryptWithNew(GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  const char *ttype;
  const char *tname;
  GWEN_PLUGIN_MANAGER *pm;
  GWEN_PLUGIN *pl;
  GWEN_CRYPT_TOKEN *ct;
  /* unsigned int cid; */
  int rv;
  const GWEN_ARGS args[]= {
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

  /* cid=GWEN_DB_GetIntValue(db, "contextId", 0, 0); */

  ttype=GWEN_DB_GetCharValue(db, "tokenType", 0, 0);
  assert(ttype);

  tname=GWEN_DB_GetCharValue(db, "tokenName", 0, 0);

  /* get crypt token */
  pm=GWEN_PluginManager_FindPluginManager("ct");
  if (pm==0) {
    DBG_ERROR(0, "Plugin manager not found");
    return 3;
  }

  pl=GWEN_PluginManager_GetPlugin(pm, ttype);
  if (pl==0) {
    DBG_ERROR(0, "Plugin not found");
    return 3;
  }
  DBG_INFO(0, "Plugin found");

  ct=GWEN_Crypt_Token_Plugin_CreateToken(pl, tname);
  if (ct==0) {
    DBG_ERROR(0, "Could not create crypt token");
    return 3;
  }

  if (GWEN_DB_GetIntValue(dbArgs, "forcePin", 0, 0))
    GWEN_Crypt_Token_AddModes(ct, GWEN_CRYPT_TOKEN_MODE_FORCE_PIN_ENTRY);

  /* open crypt token for use */
  rv=GWEN_Crypt_Token_Open(ct, 0, 0);
  if (rv) {
    DBG_ERROR(0, "Could not open token");
    return 3;
  }
  else {
    uint8_t clearText[16]= {
      0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
      0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10
    };
    uint8_t encrypted[128];
    uint32_t elen;
    uint8_t decrypted[128];
    uint32_t dlen;
    GWEN_CRYPT_PADDALGO *algo;

    algo=GWEN_Crypt_PaddAlgo_new(GWEN_Crypt_PaddAlgoId_LeftZero);
    elen=sizeof(encrypted);
    rv=GWEN_Crypt_Token_Encipher(ct,
                                 0x86,
                                 algo,
                                 clearText,
                                 16,
                                 encrypted,
                                 &elen,
                                 0);
    if (rv) {
      DBG_ERROR(0, "Could not encipher data (%d)", rv);
      return 3;
    }

    fprintf(stderr, "Encrypted data is:\n");
    GWEN_Text_DumpString((const char *) encrypted, elen, 2);

    dlen=sizeof(decrypted);
    rv=GWEN_Crypt_Token_Decipher(ct,
                                 0x86,
                                 algo,
                                 encrypted,
                                 elen,
                                 decrypted,
                                 &dlen,
                                 0);
    if (rv) {
      DBG_ERROR(0, "Could not decipher data (%d)", rv);
      return 3;
    }

    if (memcmp(clearText, decrypted, dlen)) {
      DBG_ERROR(0, "Deciphered data does not equal clear text(%d)", rv);
      return 3;
    }
    fprintf(stderr, "Deciphered data is ok.\n");

  }

  /* close crypt token */
  rv=GWEN_Crypt_Token_Close(ct, 0, 0);
  if (rv) {
    DBG_ERROR(0, "Could not close token");
    return 3;
  }

  return 0;
}








int main(int argc, char **argv)
{
  GWEN_DB_NODE *db;
  const char *cmd;
  int rv;
  int err;
  GWEN_GUI *gui;
  const char *localedir;
  GWEN_STRINGLIST *slist;
  const GWEN_ARGS args[]= {
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,            /* type */
      "cfgfile",                    /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "C",                          /* short option */
      "cfgfile",                    /* long option */
      "Specify the configuration file",     /* short description */
      "Specify the configuration file"      /* long description */
    },
    {
      0, /* flags */
      GWEN_ArgsType_Int,            /* type */
      "forcePin",                   /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      0,                            /* short option */
      "forcepin",                   /* long option */
      "force pin entry",     /* short description */
      "force pin entry even if the error counter is not zero"
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

  err=GWEN_Init();
  if (err) {
    fprintf(stderr, "Could not initialize Gwenhywfar.\n");
    return 2;
  }

  gui=GWEN_Gui_CGui_new();
  GWEN_Gui_SetGui(gui);

  slist =
    GWEN_PathManager_GetPaths(GWEN_PM_LIBNAME, GWEN_PM_LOCALEDIR);

  assert(GWEN_StringList_Count(slist) > 0);
  localedir = GWEN_StringList_FirstString(slist);
#ifdef HAVE_I18N
  setlocale(LC_ALL, "");
  if (bindtextdomain(PACKAGE, localedir)==0)
    fprintf(stderr, "Error binding locale\n");
#endif
  GWEN_StringList_free(slist);

  GWEN_Logger_Open(GCT_LOGDOMAIN, "gct-tool", 0,
                   GWEN_LoggerType_Console,
                   GWEN_LoggerFacility_User);

#ifdef DEBUG_GCT_TOOL
  GWEN_Logger_SetLevel(GCT_LOGDOMAIN, GWEN_LoggerLevel_Info);
  GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevel_Debug);
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Info);
#else
  GWEN_Logger_SetLevel(GCT_LOGDOMAIN, GWEN_LoggerLevel_Warning);
  GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevel_Warning);
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Warning);
#endif

#ifdef GCT_IS_EXPERIMENTAL
  fprintf(stderr, "\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "=================== WARNING ===================\n");
  fprintf(stderr, "This tool is still EXPERIMENTAL !!!\n");
  fprintf(stderr, "Please DON'T USE it with your daily key files !\n");
  fprintf(stderr, "===============================================\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "\n");
#endif

  db=GWEN_DB_Group_new("arguments");
  rv=GWEN_Args_Check(argc, argv, 1,
                     GWEN_ARGS_MODE_ALLOW_FREEPARAM |
                     GWEN_ARGS_MODE_STOP_AT_FREEPARAM,
                     args,
                     db);
  if (rv==GWEN_ARGS_RESULT_ERROR) {
    fprintf(stderr, "ERROR: Could not parse arguments main\n");
    return -1;
  }
  else if (rv==GWEN_ARGS_RESULT_HELP) {
    GWEN_BUFFER *ubuf;

    ubuf=GWEN_Buffer_new(0, 1024, 0, 1);
    GWEN_Buffer_AppendString(ubuf,
                             I18N("Usage: "));
    GWEN_Buffer_AppendString(ubuf, argv[0]);
    GWEN_Buffer_AppendString(ubuf,
                             I18N(" [GLOBAL OPTIONS] COMMAND "
                                  "[LOCAL OPTIONS]\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("\nGlobal Options:\n"));
    if (GWEN_Args_Usage(args, ubuf, GWEN_ArgsOutType_Txt)) {
      fprintf(stderr, "ERROR: Could not create help string\n");
      return 1;
    }
    GWEN_Buffer_AppendString(ubuf,
                             I18N("\nCommands:\n\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  create:\n"
                                  "    This command creates a crypt token"
                                  "\n\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  showuser:\n"
                                  "    Display user data stored on the "
                                  "token\n\n"));

    fprintf(stderr, "%s\n", GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    return 0;
  }
  if (rv) {
    argc-=rv-1;
    argv+=rv-1;
  }

  cmd=GWEN_DB_GetCharValue(db, "params", 0, 0);
  if (!cmd) {
    fprintf(stderr, "ERROR: Command needed.\n");
    return 1;
  }

  if (strcasecmp(cmd, "sign")==0) {
    rv=signWithNew(db, argc, argv);
  }
  else if (strcasecmp(cmd, "crypt")==0) {
    rv=cryptWithNew(db, argc, argv);
  }
  else {
    fprintf(stderr, "ERROR: Unknown command \"%s\".\n", cmd);
    rv=1;
  }

  err=GWEN_Fini();
  if (err) {
    fprintf(stderr,
            "WARNING: Could not deinitialize Gwenhywfar.\n");
  }

  return rv;
}









