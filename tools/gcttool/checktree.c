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





int checkTree(GWEN_DB_NODE *dbArgs, int argc, char **argv) {
  GWEN_DB_NODE *db;
  const char *folder;
  const char *ignFile;
  const char *inFile;
  GWEN_MDIGEST *md;
  GWEN_STRINGLIST *sl;
  GWEN_STRINGLIST *savedList;
  GWEN_STRINGLISTENTRY *se;
  GWEN_SYNCIO *sio;
  GWEN_SYNCIO *baseSio;
  GWEN_BUFFER *tbuf;
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
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "ignFile",                     /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "i",                          /* short option */
    "ignfile",                    /* long option */
    "Specify a filename to ignore",     /* short description */
    "Specify a filename to ignore"      /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "inFile",                     /* name */
    1,                            /* minnum */
    1,                            /* maxnum */
    "f",                          /* short option */
    "infile",                    /* long option */
    "Specify the output file",     /* short description */
    "Specify the output file"      /* long description */
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

  ignFile=GWEN_DB_GetCharValue(db, "ignFile", 0, NULL);

  inFile=GWEN_DB_GetCharValue(db, "inFile", 0, NULL);
  assert(inFile);

  /* hash */
  sl=GWEN_StringList_new();
  md=GWEN_MDigest_Rmd160_new();
  rv=GWEN_MDigest_HashFileTree(md, folder, ignFile, sl);
  if (rv<0) {
    fprintf(stderr, "ERROR: Could not hash folder tree (%d)\n", rv);
    return 2;
  }
  GWEN_MDigest_free(md);


  /* read list from file */
  savedList=GWEN_StringList_new();
  baseSio=GWEN_SyncIo_File_new(inFile, GWEN_SyncIo_File_CreationMode_OpenExisting);
  GWEN_SyncIo_SetFlags(baseSio, GWEN_SYNCIO_FILE_FLAGS_READ);
  sio=GWEN_SyncIo_Buffered_new(baseSio);

  rv=GWEN_SyncIo_Connect(sio);
  if (rv<0) {
    fprintf(stderr, "ERROR: Could not open input file\n");
    return 2;
  }

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  while(1) {
    rv=GWEN_SyncIo_Buffered_ReadLineToBuffer(sio, tbuf);
    if (rv<0) {
      if (rv==GWEN_ERROR_EOF)
	break;
      else {
	fprintf(stderr, "ERROR: Could not read input file (%d)\n", rv);
	return 2;
      }
    }
    else {
      GWEN_StringList_AppendString(savedList, GWEN_Buffer_GetStart(tbuf), 0, 0);
      GWEN_Buffer_Reset(tbuf);
    }
  }
  GWEN_Buffer_free(tbuf);
  GWEN_SyncIo_Disconnect(sio);
  GWEN_SyncIo_free(sio);

  /* check */
  se=GWEN_StringList_FirstEntry(savedList);
  while(se) {
    const char *s;

    s=GWEN_StringListEntry_Data(se);
    if (s && *s) {
      if (0==GWEN_StringList_RemoveString(sl, s)) {
	fprintf(stderr,
		"ERROR: The following hash could not be found: [%s]\n",
		s);
	return 2;
      }
    }
    se=GWEN_StringListEntry_Next(se);
  }
  fprintf(stdout, "All files found (passed)\n");

  if (GWEN_StringList_Count(sl)) {
    fprintf(stderr, "WARNING: File hierarchy contains %d additional files\n",
	    GWEN_StringList_Count(sl));
  }
  GWEN_StringList_free(sl);

  return 0;
}



