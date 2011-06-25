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




int createArchive(GWEN_DB_NODE *dbArgs, int argc, char **argv) {
  GWEN_DB_NODE *db;
  const char *aname;
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

  sr=GWEN_Sar_new();
  rv=GWEN_Sar_CreateArchive(sr, aname);
  if (rv<0) {
    fprintf(stderr, "ERROR: Error creating archive (%d)\n", rv);
    return 2;
  }

  rv=GWEN_Sar_CloseArchive(sr, 0);
  if (rv<0) {
    fprintf(stderr, "ERROR: Error closing archive (%d)\n", rv);
    return 2;
  }


  return 0;
}



