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






int hashTree(GWEN_DB_NODE *dbArgs, int argc, char **argv) {
  GWEN_DB_NODE *db;
  const char *folder;
  GWEN_MDIGEST *md;
  GWEN_STRINGLIST *sl;
  GWEN_STRINGLISTENTRY *se;
  GWEN_BUFFER *tbuf;
  FILE *f;
  int rv;
  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "folder",                     /* name */
    1,                            /* minnum */
    1,                            /* maxnum */
    "d",                          /* short option */
    "dir",                        /* long option */
    "Specify folder to hash",     /* short description */
    "Specify folder to hash"      /* long description */
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

  /* hash */
  sl=GWEN_StringList_new();
  md=GWEN_MDigest_Rmd160_new();
  rv=GWEN_MDigest_HashFileTree(md, folder, "checksums.rmd", sl);
  if (rv<0) {
    fprintf(stderr, "ERROR: Could not hash folder tree (%d)\n", rv);
    return 2;
  }

  /* write checksum file */
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(tbuf, folder);
  GWEN_Buffer_AppendString(tbuf, GWEN_DIR_SEPARATOR_S "checksums.rmd");
  f=fopen(GWEN_Buffer_GetStart(tbuf), "w+");
  if (f==NULL) {
    fprintf(stderr, "ERROR: Could not open output file: %s\n", strerror(errno));
    return 2;
  }

  se=GWEN_StringList_FirstEntry(sl);
  while(se) {
    const char *s;

    s=GWEN_StringListEntry_Data(se);
    if (s && *s)
      fprintf(f, "%s\n", s);
    se=GWEN_StringListEntry_Next(se);
  }
  GWEN_MDigest_free(md);
  GWEN_StringList_free(sl);

  if (fclose(f)) {
    fprintf(stderr, "ERROR: Could not close output file: %s\n", strerror(errno));
    return 2;
  }
  GWEN_Buffer_free(tbuf);

  return 0;
}



