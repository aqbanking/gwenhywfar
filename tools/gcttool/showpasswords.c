/***************************************************************************
 begin       : Mon Oct 10 2016
 copyright   : (C) 2016 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "globals.h"

#include <gwenhywfar/debug.h>
//#include <gwenhywfar/ct.h>
//#include <gwenhywfar/ctplugin.h>
//#include <gwenhywfar/text.h>
#include <gwenhywfar/passwdstore.h>




/*
 * Show passwords from a GWEN_PASSWD_STORE file.
 */


int showPasswords(GWEN_DB_NODE *dbArgs, int argc, char **argv) {
  GWEN_DB_NODE *db;
  const char *file;
  GWEN_PASSWD_STORE *sto;
  GWEN_STRINGLIST *sl;
  GWEN_STRINGLISTENTRY *se;
  int rv;
  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "file",                       /* name */
    1,                            /* minnum */
    1,                            /* maxnum */
    "f",                          /* short option */
    "file",                       /* long option */
    "Specify the password file",     /* short description */
    "Specify the password file"      /* long description */
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

  file=GWEN_DB_GetCharValue(db, "file", 0, NULL);
  assert(file);

  sto=GWEN_PasswordStore_new(file);
  sl=GWEN_StringList_new();

  rv=GWEN_PasswordStore_GetTokenList(sto, sl);
  if (rv<0) {
    fprintf(stderr, "ERROR: Could not get token list: %d\n", rv);
    GWEN_PasswordStore_free(sto);
    return 3;
  }

  se=GWEN_StringList_FirstEntry(sl);
  while(se) {
    const char *s;

    s=GWEN_StringListEntry_Data(se);
    if (s && *s) {
      char passwd[512];

      rv=GWEN_PasswordStore_GetPassword(sto, s, passwd, 1, sizeof(passwd)-1);
      if (rv<0) {
	fprintf(stderr, "ERROR: Could not get password for token \"%s\": %d\n", s, rv);
	GWEN_PasswordStore_free(sto);
	return 3;
      }
      passwd[sizeof(passwd)-1]=0;
      fprintf(stdout, "\"%s\" -> \"%s\"\n", s, passwd);
      memset(passwd, 0, sizeof(passwd));
    }
    se=GWEN_StringListEntry_Next(se);
  }
  GWEN_StringList_free(sl);

  GWEN_PasswordStore_free(sto);

  return 0;
}



