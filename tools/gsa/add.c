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
#include <gwenhywfar/directory.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <errno.h>
#include <string.h>



static int addToList(const char *fname, int recursive, GWEN_STRINGLIST *sl)
{
  struct stat st;
  int rv;

  /* stat file to be added */
#if _BSD_SOURCE || _XOPEN_SOURCE >= 500 || _XOPEN_SOURCE && _XOPEN_SOURCE_EXTENDED
  rv=lstat(fname, &st);
#else
  rv=stat(fname, &st);
#endif
  if (rv) {
    DBG_ERROR(GSA_LOGDOMAIN, "stat(%s): %d (%s)",
              fname, errno, strerror(errno));
    fprintf(stderr, "Ignoring file \"%s\"\n", fname);
  }
  else {
    /* always append this entry */
    GWEN_StringList_AppendString(sl, fname, 0, 1);
    if (recursive && S_ISDIR(st.st_mode)) {
      GWEN_STRINGLIST *sll;
      GWEN_STRINGLISTENTRY *se;
      GWEN_DIRECTORY *d;
      int rv;
      char buffer[256];
      GWEN_BUFFER *tbuf;
      uint32_t pos;

      /* add entries */
      sll=GWEN_StringList_new();
      d=GWEN_Directory_new();
      rv=GWEN_Directory_Open(d, fname);
      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        GWEN_Directory_free(d);
        GWEN_StringList_free(sll);
        return rv;
      }

      while (0==GWEN_Directory_Read(d, buffer, sizeof(buffer))) {
        if (strcmp(buffer, ".")!=0 &&
            strcmp(buffer, "..")!=0)
          GWEN_StringList_AppendString(sll, buffer, 0, 1);
      }

      GWEN_Directory_Close(d);
      GWEN_Directory_free(d);

      /* recurse */
      tbuf=GWEN_Buffer_new(0, 256, 0, 1);
      GWEN_Buffer_AppendString(tbuf, fname);
      GWEN_Buffer_AppendString(tbuf, GWEN_DIR_SEPARATOR_S);
      pos=GWEN_Buffer_GetPos(tbuf);
      se=GWEN_StringList_FirstEntry(sll);
      while (se) {
        const char *s;

        s=GWEN_StringListEntry_Data(se);
        if (s && *s) {
          GWEN_Buffer_AppendString(tbuf, s);
          rv=addToList(GWEN_Buffer_GetStart(tbuf), recursive, sl);
          if (rv<0) {
            DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
            GWEN_Buffer_free(tbuf);
            GWEN_StringList_free(sll);
            return rv;
          }
        }
        GWEN_Buffer_Crop(tbuf, 0, pos);
        se=GWEN_StringListEntry_Next(se);
      } /* while se */
      GWEN_Buffer_free(tbuf);
      GWEN_StringList_free(sll);
    } /* if dir and recursive */
  } /* if stat was ok */

  return 0;
}




int add2Archive(GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  const char *aname;
  GWEN_SAR *sr;
  int rv;
  int recursive;
  int verbosity;
  const GWEN_ARGS args[]= {
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
      0, /* flags */
      GWEN_ArgsType_Int,                /* type */
      "recursive",                      /* name */
      0,                                /* minnum */
      1,                                /* maxnum */
      "r",                              /* short option */
      "recursive",                      /* long option */
      "add folders recursively",        /* short description */
      "add folders recursively"         /* long description */
    },
    {
      0, /* flags */
      GWEN_ArgsType_Int,                /* type */
      "verbosity",                      /* name */
      0,                                /* minnum */
      10,                                /* maxnum */
      "v",                              /* short option */
      NULL,                             /* long option */
      "set verbosity",                  /* short description */
      "set verbosity"                   /* long description */
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

  recursive=GWEN_DB_GetIntValue(db, "recursive", 0, 0);
  verbosity=GWEN_DB_GetIntValue(db, "verbosity", 0, 0);

  sr=GWEN_Sar_new();
  rv=GWEN_Sar_OpenArchive(sr, aname,
                          GWEN_SyncIo_File_CreationMode_OpenExisting,
                          GWEN_SYNCIO_FILE_FLAGS_READ|GWEN_SYNCIO_FILE_FLAGS_WRITE);
  if (rv<0) {
    fprintf(stderr, "ERROR: Error opening archive (%d)\n", rv);
    return 2;
  }
  else {
    int i;
    GWEN_STRINGLIST *sl;
    GWEN_STRINGLISTENTRY *se;

    sl=GWEN_StringList_new();
    for (i=0; ; i++) {
      const char *fname;

      fname=GWEN_DB_GetCharValue(db, "params", i, 0);
      if (fname && *fname) {
        rv=addToList(fname, recursive, sl);
        if (rv<0) {
          fprintf(stderr, "ERROR: Error adding entry \"%s\" to archive \"%s\" (%d)\n",
                  fname, aname, rv);
          GWEN_StringList_free(sl);
          return 2;
        }
      }
      else
        break;
    }

    se=GWEN_StringList_FirstEntry(sl);
    while (se) {
      const char *s;

      s=GWEN_StringListEntry_Data(se);
      if (s && *s) {
        rv=GWEN_Sar_AddFile(sr, s);
        if (rv<0) {
          fprintf(stderr, "ERROR: Error adding file \"%s\" to archive \"%s\" (%d)\n",
                  s, aname, rv);
          GWEN_Sar_CloseArchive(sr, 1);
          GWEN_Sar_free(sr);
          return 2;
        }
        if (verbosity>0) {
          fprintf(stdout, "added \"%s\"\n", s);
        }
      }
      se=GWEN_StringListEntry_Next(se);
    } /* while se */

    GWEN_StringList_free(sl);

    rv=GWEN_Sar_CloseArchive(sr, 0);
    if (rv<0) {
      fprintf(stderr, "ERROR: Error closing archive (%d)\n", rv);
      return 2;
    }

    return 0;
  }
}






