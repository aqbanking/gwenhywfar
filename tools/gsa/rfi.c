/***************************************************************************
 begin       : Mon Jun 27 2011
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
#include <gwenhywfar/db.h>




int releaseFillIn(GWEN_DB_NODE *dbArgs, int argc, char **argv) {
  GWEN_DB_NODE *db;
  const char *name;
  const char *arch;
  const char *sys;
  const char *inFile;
  const char *clFile;
  const char *outFile;
  const char *url;
  int vMajor;
  int vMinor;
  int vPatchLevel;
  int vBuild;
  int maturityLevel;
  GWEN_DB_NODE *dbVersion;
  int rv;
  GWEN_BUFFER *cbuf;
  char ubuf[512];
  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT,     /* flags */
    GWEN_ArgsType_Char,               /* type */
    "infile",                         /* name */
    1,                                /* minnum */
    1,                                /* maxnum */
    "i",                              /* short option */
    "infile",                         /* long option */
    "Specify the input version file",  /* short description */
    "Specify the input version file"   /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT,     /* flags */
    GWEN_ArgsType_Char,               /* type */
    "changeLogFile",                    /* name */
    1,                                /* minnum */
    1,                                /* maxnum */
    "c",                              /* short option */
    "changelog",                        /* long option */
    "Specify the changelog file",  /* short description */
    "Specify the changelog file"   /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT,     /* flags */
    GWEN_ArgsType_Char,               /* type */
    "url",                            /* name */
    1,                                /* minnum */
    1,                                /* maxnum */
    "u",                              /* short option */
    "url",                            /* long option */
    "Specify the URL of the update server",  /* short description */
    "Specify the URL of the update server"   /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT,     /* flags */
    GWEN_ArgsType_Char,               /* type */
    "outfile",                        /* name */
    1,                                /* minnum */
    1,                                /* maxnum */
    "o",                              /* short option */
    "outfile",                         /* long option */
    "Specify the output version file",  /* short description */
    "Specify the output version file"   /* long description */
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

  inFile=GWEN_DB_GetCharValue(db, "infile", 0, NULL);
  assert(inFile);

  clFile=GWEN_DB_GetCharValue(db, "changeLogFile", 0, NULL);
  assert(clFile);

  outFile=GWEN_DB_GetCharValue(db, "outfile", 0, NULL);
  assert(outFile);

  url=GWEN_DB_GetCharValue(db, "url", 0, NULL);
  assert(url);

  /* read changelog file */
  cbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=readFile(clFile, cbuf);
  if (rv<0) {
    fprintf(stderr, "ERROR: Could not read changelog file [%s] (%d)\n", clFile, rv);
    return 2;
  }

  /* read version file */
  dbVersion=GWEN_DB_Group_new("version");
  rv=GWEN_DB_ReadFile(dbVersion, inFile, GWEN_DB_FLAGS_DEFAULT);
  if (rv<0) {
    fprintf(stderr, "ERROR: Could not read file [%s] (%d)\n", inFile, rv);
    return 2;
  }

  name=GWEN_DB_GetCharValue(dbVersion, "name", 0, NULL);
  if (!(name && *name)) {
    fprintf(stderr, "Missing name in version file.\n");
    return 2;
  }
  arch=GWEN_DB_GetCharValue(dbVersion, "architecture", 0, NULL);
  if (!(arch && *arch)) {
    fprintf(stderr, "Missing architecture in version file.\n");
    return 2;
  }
  sys=GWEN_DB_GetCharValue(dbVersion, "system", 0, NULL);
  if (!(sys && *sys)) {
    fprintf(stderr, "Missing system in version file.\n");
    return 2;
  }
  vMajor=GWEN_DB_GetIntValue(dbVersion, "versionMajor", 0, 0);
  vMinor=GWEN_DB_GetIntValue(dbVersion, "versionMinor", 0, 0);
  vPatchLevel=GWEN_DB_GetIntValue(dbVersion, "versionPatchLevel", 0, 0);
  vBuild=GWEN_DB_GetIntValue(dbVersion, "versionBuild", 0, 0);
  maturityLevel=GWEN_DB_GetIntValue(dbVersion, "maturityLevel", 0, 0);

  /* create URL entry */
  snprintf(ubuf, sizeof(ubuf)-1, "%s/%s-%s-%s-%d.%d.%d.%d-%d.sar.gz",
	   url,
	   name, sys, arch,
	   vMajor, vMinor, vPatchLevel, vBuild,
	   maturityLevel);
  ubuf[sizeof(ubuf)-1]=0;
  GWEN_DB_SetCharValue(dbVersion, GWEN_DB_FLAGS_OVERWRITE_VARS, "url", ubuf);

  /* create changelog entry */
  GWEN_DB_SetCharValue(dbVersion, GWEN_DB_FLAGS_OVERWRITE_VARS, "changeLog", GWEN_Buffer_GetStart(cbuf));
  GWEN_Buffer_free(cbuf);

  /* write version file back */
  rv=GWEN_DB_WriteFile(dbVersion, outFile, GWEN_DB_FLAGS_DEFAULT);
  if (rv<0) {
    fprintf(stderr, "ERROR: Could not write file [%s] (%d)\n", outFile, rv);
    return 2;
  }

  return 0;
}



