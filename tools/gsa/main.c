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

#define GSA_IS_EXPERIMENTAL
#define DEBUG_GSA_TOOL */

#include <gwenhywfar/debug.h>
#include <gwenhywfar/cgui.h>

#include "funcs.h"
#include "globals.h"




int readFile(const char *fname, GWEN_BUFFER *dbuf)
{
  FILE *f;

  f=fopen(fname, "rb");
  if (f) {
    while (!feof(f)) {
      uint32_t l;
      ssize_t s;
      char *p;

      GWEN_Buffer_AllocRoom(dbuf, 1024);
      l=GWEN_Buffer_GetMaxUnsegmentedWrite(dbuf);
      p=GWEN_Buffer_GetPosPointer(dbuf);
      s=fread(p, 1, l, f);
      if (s==0)
        break;
      if (s==(ssize_t)-1) {
        DBG_INFO(GWEN_LOGDOMAIN,
                 "fread(%s): %s",
                 fname, strerror(errno));
        fclose(f);
        return GWEN_ERROR_IO;
      }

      GWEN_Buffer_IncrementPos(dbuf, s);
      GWEN_Buffer_AdjustUsedBytes(dbuf);
    }

    fclose(f);
    return 0;
  }
  else {
    DBG_INFO(GWEN_LOGDOMAIN,
             "fopen(%s): %s",
             fname, strerror(errno));
    return GWEN_ERROR_IO;
  }
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
    GWEN_FUNCS_ENTRY_DB_NODE_ARGS_HELP("create", createArchive, I18N("This command creates an archive file")),
    GWEN_FUNCS_ENTRY_DB_NODE_ARGS_HELP("add", add2Archive, I18N("Add files and folders to an archive file")),
    GWEN_FUNCS_ENTRY_DB_NODE_ARGS_HELP("list", listArchive, I18N("List files and folders in an archive file")),
    GWEN_FUNCS_ENTRY_DB_NODE_ARGS_HELP("check", checkArchive, I18N("Check integrity of files and folders in an archive file")),
    GWEN_FUNCS_ENTRY_DB_NODE_ARGS("extract", extractArchive),
    GWEN_FUNCS_ENTRY_DB_NODE_ARGS("sign", signArchive),
    GWEN_FUNCS_ENTRY_DB_NODE_ARGS("verify", verifyArchive),
    GWEN_FUNCS_ENTRY_DB_NODE_ARGS("mkkey", mkArchiveKey),
    GWEN_FUNCS_ENTRY_DB_NODE_ARGS("rfi", releaseFillIn),
    GWEN_FUNCS_ENTRY_END(),
  };
  const GWEN_FUNCS *func;

  err=GWEN_Init();
  if (err) {
    fprintf(stderr, "Could not initialize Gwenhywfar.\n");
    return 2;
  }

  gui=GWEN_Gui_CGui_new();
  GWEN_Gui_SetGui(gui);

  GWEN_Logger_Open(GSA_LOGDOMAIN, "gsa", 0,
                   GWEN_LoggerType_Console,
                   GWEN_LoggerFacility_User);

  GWEN_Logger_SetLevel(GSA_LOGDOMAIN, GWEN_LoggerLevel_Warning);
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Warning);

#ifdef GSA_IS_EXPERIMENTAL
  fprintf(stderr, "\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "=================== WARNING ===================\n");
  fprintf(stderr, "This tool is still EXPERIMENTAL !!!\n");
  fprintf(stderr, "Please DON'T USE it with your data files !\n");
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
                             I18N("GWEN's Simple Archiver"));
    GWEN_Buffer_AppendString(ubuf,
                             " (Gwenhywfar v" GWENHYWFAR_VERSION_FULL_STRING ")\n");
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
    fprintf(stderr, "%s\n", GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);

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



