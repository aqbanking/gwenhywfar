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



int showUser(GWEN_DB_NODE *dbArgs, int argc, char **argv) {
  GWEN_DB_NODE *db;
  const char *ttype;
  const char *tname;
  GWEN_PLUGIN_MANAGER *pm;
  GWEN_PLUGIN *pl;
  GWEN_CRYPTTOKEN *ct;
  unsigned int ucid;
  int shown=0;
  int rv;
  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Int,             /* type */
    "userContextId",              /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "i",                          /* short option */
    "id",                         /* long option */
    "User context id (0 for any)",/* short description */
    "User context id (0 for any)" /* long description */
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

  ucid=GWEN_DB_GetIntValue(db, "userContextId", 0, 0);

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
    GWEN_CRYPTTOKEN_USER_LIST *l;
    GWEN_CRYPTTOKEN_USER *u;

    l=GWEN_CryptToken_User_List_new();
    rv=GWEN_CryptToken_FillUserList(ct, l);
    if (rv) {
      DBG_ERROR(0, "Error filling user list (%d)", rv);
      GWEN_CryptToken_User_List_free(l);
      GWEN_CryptToken_Close(ct);
      return 3;
    }

    u=GWEN_CryptToken_User_List_First(l);
    while(u) {
      if (ucid==0 || ucid==GWEN_CryptToken_User_GetId(u)) {
        const char *s;

        fprintf(stdout, "-------------------------------------------------\n");
        fprintf(stdout, "User %u\n",
                (unsigned int)GWEN_CryptToken_User_GetId(u));

        s=GWEN_CryptToken_User_GetServiceId(u);
        if (s)
          fprintf(stdout, "Service Id  : %s\n", s);

        s=GWEN_CryptToken_User_GetUserId(u);
        if (s)
          fprintf(stdout, "User Id     : %s\n", s);

        s=GWEN_CryptToken_User_GetUserName(u);
        if (s)
          fprintf(stdout, "User Name   : %s\n", s);

        s=GWEN_CryptToken_User_GetPeerId(u);
        if (s)
          fprintf(stdout, "Peer Id     : %s\n", s);

        s=GWEN_CryptToken_User_GetPeerName(u);
        if (s)
          fprintf(stdout, "Peer Name   : %s\n", s);

        s=GWEN_CryptToken_User_GetAddress(u);
        if (s) {
          int i;

          fprintf(stdout, "Address     : %s", s);
          i=GWEN_CryptToken_User_GetPort(u);
          if (i)
            fprintf(stdout, " (port %d)", i);
          fprintf(stdout, "\n");
        }

        s=GWEN_CryptToken_User_GetSystemId(u);
        if (s)
          fprintf(stdout, "System Id   : %s\n", s);

        fprintf(stdout, "Context Id  : %d\n",
                GWEN_CryptToken_User_GetContextId(u));

        shown++;
      }

      u=GWEN_CryptToken_User_List_Next(u);
    } /* while */
    GWEN_CryptToken_User_List_free(l);
  }


  /* close crypt token */
  rv=GWEN_CryptToken_Close(ct);
  if (rv) {
    DBG_ERROR(0, "Could not close token");
    return 3;
  }

  if (!shown) {
    if (ucid==0) {
      DBG_ERROR(0, "No user found");
    }
    else {
      DBG_ERROR(0, "User %u not found", ucid);
    }
    return 1;
  }

  return 0;
}



