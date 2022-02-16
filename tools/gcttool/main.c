/***************************************************************************
 begin       : Tue May 03 2005
 copyright   : (C) 2005-2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/*#define GCT_IS_EXPERIMENTAL
#define DEBUG_GCT_TOOL */

#include <gwenhywfar/debug.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/cgui.h>

#include "funcs.h"
#include "globals.h"

#ifdef OS_WIN32
# define DIRSEP "\\"
#else
# define DIRSEP "/"
#endif

#ifdef HAVE_I18N
# include <libintl.h>
# include <locale.h>
#endif


#ifdef HAVE_OPENSSL
# include <gwenhywfar/gui_be.h>

# include <openssl/err.h>
# include <openssl/ssl.h>
# include <openssl/des.h>
#endif




#ifdef HAVE_OPENSSL
int getKeyDataFromTextOpenSSL(GWEN_UNUSED GWEN_GUI *gui,
                              const char *password,
                              unsigned char *buffer,
                              unsigned int bsize)
{
  DES_cblock left, right;
  int i;

  if (bsize!=16) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Buffer must be exact 16 bytes in length");
    return -1;
  }
  DES_string_to_2keys(password, &left, &right);
  for (i=0; i<8; i++)
    *(buffer++)=left[i];
  for (i=0; i<8; i++)
    *(buffer++)=right[i];
  return 0;
}

#endif


GWEN_CRYPT_TOKEN *getCryptToken(const char *ttype,
                                const char *tname)
{
  GWEN_PLUGIN_MANAGER *pm;
  GWEN_PLUGIN *pl;
  GWEN_CRYPT_TOKEN *ct;

  /* get crypt token */
  pm=GWEN_PluginManager_FindPluginManager("ct");
  if (pm==0) {
    DBG_ERROR(0, "Plugin manager not found");
    return NULL;
  }

  pl=GWEN_PluginManager_GetPlugin(pm, ttype);
  if (pl==0) {
    DBG_ERROR(0, "Plugin not found");

    fprintf(stderr, "%s",
            I18N("The plugin could not be found.\n"
                 "Please make sure that you have the following "
                 "packages installed (at least on Debian/Ubuntu):\n"
                 "- libchipcard-plugins-libgwenhywfar (for chipcards)\n"
                 "- libaqbanking-plugins-libgwenhywfar\n"));
    return NULL;
  }
  DBG_INFO(0, "Plugin found");

  ct=GWEN_Crypt_Token_Plugin_CreateToken(pl, tname);
  if (ct==0) {
    DBG_ERROR(0, "Could not create crypt token");
    return NULL;
  }

  return ct;
}



int main(int argc, char **argv)
{
  GWEN_DB_NODE *db;
  const char *cmd;
  int rv;
  int err;
  GWEN_GUI *gui;
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
  const GWEN_FUNCS funcs[]= {
    GWEN_Funcs_Entry_DB_NODE_Args_Help("create", createToken, I18N("This command creates a crypt token")),
    GWEN_Funcs_Entry_DB_NODE_Args_Help("showuser", showUser, I18N("Display user data stored on the token")),
    GWEN_Funcs_Entry_DB_NODE_Args_Help("showkey", showKey, I18N("Display key info stored on the token")),
    GWEN_Funcs_Entry_DB_NODE_Args_Help("genkey", genKey, I18N("Generate keys on crypt token")),
    GWEN_Funcs_Entry_DB_NODE_Args_Help("activateKey", activateKey, I18N("Activate keys on crypt token")),
    GWEN_Funcs_Entry_DB_NODE_Args_Help("update", updateToken, I18N("Update Crypt Token to newer version (e.g. OpenHBCI key files")),
    GWEN_Funcs_Entry_DB_NODE_Args_Help("setsignseq", setSignSeq, I18N("Set signature counter")),
    GWEN_Funcs_Entry_DB_NODE_Args_Help("changepin", changePin, I18N("Change pin of the crypt token")),
    GWEN_Funcs_Entry_DB_NODE_Args("setkey", setKey),
    GWEN_Funcs_Entry_DB_NODE_Args("hashTree", hashTree),
    GWEN_Funcs_Entry_DB_NODE_Args("checkTree", checkTree),
    GWEN_Funcs_Entry_DB_NODE_Args_Help("showpasswords", showPasswords, I18N("Display passwords store in a GWEN_PASSWD_STORE file")),
    GWEN_Funcs_Entry_End(),
  };
  const GWEN_FUNCS *func;

  err=GWEN_Init();
  if (err) {
    fprintf(stderr, "Could not initialize Gwenhywfar.\n");
    return 2;
  }

  gui=GWEN_Gui_CGui_new();
  GWEN_Gui_SetGui(gui);

#ifdef HAVE_OPENSSL
  /* for conversion purposes */
  SSL_load_error_strings();
  SSL_library_init();
  GWEN_Gui_SetKeyDataFromTextOpenSslFn(gui, getKeyDataFromTextOpenSSL);
#endif

  GWEN_Logger_Open(GCT_LOGDOMAIN, "gct-tool", 0,
                   GWEN_LoggerType_Console,
                   GWEN_LoggerFacility_User);

  GWEN_Logger_SetLevel(GCT_LOGDOMAIN, GWEN_LoggerLevel_Warning);
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Warning);

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
    fprintf(stderr, "%s\n", I18N("\nCommands:\n\n"));
    GWEN_Funcs_Usage_With_Help(funcs);
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

  func=GWEN_Funcs_Find(funcs, cmd);
  if (func!=NULL) {
    rv=GWEN_Funcs_Call_DB_NODE_Args(func, db, argc, argv);
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



