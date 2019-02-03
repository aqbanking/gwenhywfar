/***************************************************************************
 begin       : Tue Jun 01 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "globals.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/ct.h>
#include <gwenhywfar/ctplugin.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/mdigest.h>
#include <gwenhywfar/syncio.h>
#include <gwenhywfar/syncio_file.h>
#include <gwenhywfar/syncio_buffered.h>





int checkTree(GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  const char *folder;
  GWEN_MDIGEST *md;
  int rv;
  const GWEN_ARGS args[]= {
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
      "folder",                     /* name */
      1,                            /* minnum */
      1,                            /* maxnum */
      "d",                          /* short option */
      "dir",                        /* long option */
      "Specify folder to check",     /* short description */
      "Specify folder to check"      /* long description */
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

  folder=GWEN_DB_GetCharValue(db, "folder", 0, NULL);
  assert(folder);

  /* check hash list */
  md=GWEN_MDigest_Rmd160_new();
  rv=GWEN_MDigest_CheckFileTree(md, folder, "checksums.rmd", 0, 0);
  if (rv<0) {
    fprintf(stderr, "ERROR: Integrity check failed");
    GWEN_MDigest_free(md);
    return 2;
  }
  GWEN_MDigest_free(md);

  fprintf(stdout, "All files found (passed)\n");

  return 0;
}



