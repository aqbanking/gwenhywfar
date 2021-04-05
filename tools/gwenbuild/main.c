/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "gwenbuild/parser/parser.h"

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/cgui.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/args.h>
#include <gwenhywfar/i18n.h>
#include <gwenhywfar/text.h>

#ifdef HAVE_SIGNAL_H
# include <signal.h>
#endif



#define I18N(msg) GWEN_I18N_Translate(PACKAGE, msg)
#define I18S(msg) msg



static int _setup(GWEN_DB_NODE *dbArgs, int argc, char **argv);
static int _prepare(GWEN_DB_NODE *dbArgs, int argc, char **argv);
static int _build(GWEN_DB_NODE *dbArgs, int argc, char **argv);

static GWB_KEYVALUEPAIR_LIST *_readOptionsFromDb(GWEN_DB_NODE *db);



#ifdef HAVE_SIGNAL_H

struct sigaction sigActionChild;


void _signalHandler(int s) {
  switch(s) {
  case SIGCHLD:
    //fprintf(stderr, "Child exited %d\n", s);
    break;
  default:
    fprintf(stderr, "Received unhandled signal %d\n", s);
    break;
  }
  signal(s, _signalHandler);
}



int _setSingleSignalHandler(struct sigaction *sa, int sig)
{
  sa->sa_handler=_signalHandler;
  sigemptyset(&sa->sa_mask);
  sa->sa_flags=0;
  if (sigaction(sig, sa, 0)) {
    DBG_ERROR(NULL, "Could not setup signal handler for signal %d", sig);
    return GWEN_ERROR_GENERIC;
  }
  return 0;
}



int _setSignalHandlers() {
  int rv;

  rv=_setSingleSignalHandler(&sigActionChild, SIGCHLD);
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    return rv;
  }

  return 0;
}

#endif




int test_ReadProject(int argc, char **argv)
{
  GWENBUILD *gwenbuild;
  GWB_PROJECT *project;
  GWB_BUILD_CONTEXT *buildCtx;
  const char *folder;
  int rv;

  gwenbuild=GWBUILD_new();

  if (argc<2) {
    DBG_ERROR(NULL, "Folder needed");
    return 1;
  }

  folder=argv[1];
  project=GWB_Parser_ReadBuildTree(gwenbuild, folder, NULL);
  if (project==NULL) {
    DBG_ERROR(NULL, "Error reading build files.");
    return 2;
  }

  //DBG_ERROR(NULL, "Tree successfully loaded.");
  //GWB_Project_Dump(project, 2);

  rv=GWBUILD_MakeBuildersForTargets(project);
  if (rv<0) {
    DBG_ERROR(NULL, "Error making builders for targets.");
    return 2;
  }
  DBG_ERROR(NULL, "Targets successfully created.");

  buildCtx=GWBUILD_MakeBuildCommands(project);
  if (buildCtx==NULL) {
    GWB_Project_Dump(project, 2, 0);
    DBG_ERROR(NULL, "Could not generate build commands");
    return 2;
  }
  DBG_ERROR(NULL, "Build commands successfully created.");

  //GWB_Project_Dump(project, 2, 0);

  //DBG_ERROR(NULL, "Build context:");
  //GWB_BuildCtx_Dump(buildCtx, 2);

  GWB_BuildCtx_WriteToXmlFile(buildCtx, "buildctx.xml");
  if (1) {
    GWB_BUILD_CONTEXT *buildCtx2;

    buildCtx2=GWB_BuildCtx_ReadFromXmlFile("buildctx.xml");
    if (buildCtx2==NULL) {
      DBG_ERROR(NULL, "Error reading build context from file");
    }
    else
      GWB_BuildCtx_WriteToXmlFile(buildCtx2, "buildctx2.xml");
  }

  /* prepare */
  rv=GWB_BuildCtx_Run(buildCtx, 10, 1, NULL);
  if (rv<0) {
    DBG_ERROR(NULL, "Error preparing builds.");
    return 2;
  }
  DBG_ERROR(NULL, "Build successfully prepared.");

  /* build */
  rv=GWB_BuildCtx_Run(buildCtx, 10, 0, NULL);
  if (rv<0) {
    DBG_ERROR(NULL, "Error building builds.");
    return 2;
  }
  DBG_ERROR(NULL, "Build successfully done.");

  return 0;
}






int main2(int argc, char **argv)
{
  int rv;
  GWEN_GUI *gui;

#ifdef HAVE_SIGNAL_H
  signal(SIGCHLD, _signalHandler);
  //_setSignalHandlers();
#endif

  rv=GWEN_Init();
  if (rv<0) {
    DBG_ERROR(NULL, "Error on GWEN_Init (%d)", rv);
    return 2;
  }

  gui=GWEN_Gui_CGui_new();
  GWEN_Gui_SetGui(gui);

  GWEN_Logger_Open(0, "gwenbuild", 0,
                   GWEN_LoggerType_Console,
                   GWEN_LoggerFacility_User);

  GWEN_Logger_SetLevel(NULL, GWEN_LoggerLevel_Info);

  rv=test_ReadProject(argc, argv);
  if (rv) {
    DBG_ERROR(NULL, "here (%d)", rv);
    return 3;
  }


  return 0;
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

#ifdef HAVE_SIGNAL_H
  signal(SIGCHLD, _signalHandler);
  //_setSignalHandlers();
#endif

  err=GWEN_Init();
  if (err) {
    fprintf(stderr, "Could not initialize Gwenhywfar.\n");
    return 2;
  }

  gui=GWEN_Gui_CGui_new();
  GWEN_Gui_SetGui(gui);

  GWEN_Logger_Open(NULL, "gwenbuild", 0,
                   GWEN_LoggerType_Console,
                   GWEN_LoggerFacility_User);

  GWEN_Logger_SetLevel(NULL, GWEN_LoggerLevel_Warning);
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Warning);

  db=GWEN_DB_Group_new("arguments");
  rv=GWEN_Args_Check(argc, argv, 1,
                     GWEN_ARGS_MODE_ALLOW_FREEPARAM |
                     GWEN_ARGS_MODE_STOP_AT_FREEPARAM,
                     args,
                     db);
  if (rv==GWEN_ARGS_RESULT_ERROR) {
    fprintf(stderr, "ERROR: Could not parse arguments main\n");
    GWEN_DB_Group_free(db);
    return 1;
  }
  else if (rv==GWEN_ARGS_RESULT_HELP) {
    GWEN_BUFFER *ubuf;

    ubuf=GWEN_Buffer_new(0, 1024, 0, 1);
    GWEN_Buffer_AppendString(ubuf,
                             I18N("GWEN's Build Tool"));
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
      GWEN_DB_Group_free(db);
      return 1;
    }
    GWEN_Buffer_AppendString(ubuf,
                             I18N("\nCommands:\n\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  setup:\n"
                                  "    Setup build system (e.g. read all 0BUILD files for your project and create build commands)"
                                  "\n\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  prepare:\n"
                                  "    Prepare sources (e.g. typemaker2 generated files)"
                                  "\n\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  build:\n"
                                  "    Build your project\n\n"));

    fprintf(stderr, "%s\n", GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    GWEN_DB_Group_free(db);
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

  if (strcasecmp(cmd, "setup")==0) {
    rv=_setup(db, argc, argv);
  }
  else if (strcasecmp(cmd, "prepare")==0) {
    rv=_prepare(db, argc, argv);
  }
  else if (strcasecmp(cmd, "build")==0) {
    rv=_build(db, argc, argv);
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



int _setup(GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  GWENBUILD *gwenbuild;
  GWB_PROJECT *project;
  GWB_BUILD_CONTEXT *buildCtx;
  const char *folder;
  int doDump;
  GWB_KEYVALUEPAIR_LIST *givenOptionList;
  int rv;
  const GWEN_ARGS args[]= {
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,            /* type */
      "folder",                       /* name */
      1,                            /* minnum */
      1,                            /* maxnum */
      "f",                          /* short option */
      "folder",                       /* long option */
      "Folder containing sources",  /* short description */
      "Folder containing sources"   /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,            /* type */
      "option",                      /* name */
      0,                            /* minnum */
      0,                            /* maxnum */
      "O",                          /* short option */
      "option",                      /* long option */
      "Set option (OPTION=VALUE)",  /* short description */
      "Set option (OPTION=VALUE)"   /* long description */
    },
    {
      0,                            /* flags */
      GWEN_ArgsType_Int,            /* type */
      "dump",                       /* name */
      0,                            /* minnum */
      0,                            /* maxnum */
      NULL,                          /* short option */
      "dump",                      /* long option */
      "Dump read build context",  /* short description */
      "Dump read build context"   /* long description */
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
                     0 /*GWEN_ARGS_MODE_ALLOW_FREEPARAM*/,
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
    fprintf(stdout,
            I18N("This command sets up the build folder.\n"
                 "Arguments:\n"
                 "%s\n"),
            GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    return 0;
  }

  doDump=GWEN_DB_GetIntValue(db, "dump", 0, 0);

  folder=GWEN_DB_GetCharValue(db, "folder", 0, NULL);
  if (!(folder && *folder)) {
    fprintf(stderr, "ERROR: Folder needed.\n");
    return 1;
  }
  if (strcasecmp(folder, ".")==0) {
    fprintf(stderr, "ERROR: Only building outside source folder supported.\n");
    return 1;
  }

  givenOptionList=_readOptionsFromDb(db);


  gwenbuild=GWBUILD_new();

  project=GWB_Parser_ReadBuildTree(gwenbuild, folder, givenOptionList);
  if (project==NULL) {
    fprintf(stderr, "ERROR: Error reading build files.\n");
    return 2;
  }

  rv=GWBUILD_MakeBuildersForTargets(project);
  if (rv<0) {
    fprintf(stderr, "ERROR: Error making builders for targets.\n");
    return 2;
  }

  buildCtx=GWBUILD_MakeBuildCommands(project);
  if (buildCtx==NULL) {
    fprintf(stderr, "ERROR: Could not generate build commands.\n");
    return 2;
  }

  rv=GWB_BuildCtx_WriteToXmlFile(buildCtx, "buildctx.xml");
  if (rv<0) {
    fprintf(stderr, "ERROR: Error writing build context file.\n");
    return 3;
  }

  if (doDump) {
    GWB_Project_Dump(project, 2, 1);
  }

  return 0;
}



int _prepare(GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  GWB_BUILD_CONTEXT *buildCtx;
  int rv;
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

  db=GWEN_DB_GetGroup(dbArgs, GWEN_DB_FLAGS_DEFAULT, "local");
  rv=GWEN_Args_Check(argc, argv, 1,
                     0 /*GWEN_ARGS_MODE_ALLOW_FREEPARAM*/,
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
    fprintf(stdout,
            I18N("This command sets up the build folder.\n"
                 "Arguments:\n"
                 "%s\n"),
            GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    return 0;
  }

  buildCtx=GWB_BuildCtx_ReadFromXmlFile("buildctx.xml");
  if (buildCtx==NULL) {
    fprintf(stderr, "ERROR: Error reading build context from file.\n");
    return 1;
  }

  /* prepare */
  rv=GWB_BuildCtx_Run(buildCtx, 10, 1, NULL);
  if (rv<0) {
    fprintf(stderr, "ERROR: Error preparing builds.\n");
    return 2;
  }

  return 0;
}



int _build(GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  GWB_BUILD_CONTEXT *buildCtx;
  int rv;
  int numThreads;
  const char *builderName;
  const GWEN_ARGS args[]= {
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Int,            /* type */
      "jobs",                       /* name */
      0,                            /* minnum */
      0,                            /* maxnum */
      "j",                          /* short option */
      "jobs",                       /* long option */
      "Specify number of parallel jobs",      /* short description */
      "Specify number of parallel jobs"       /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,            /* type */
      "builder",                    /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "B",                          /* short option */
      "builder",                    /* long option */
      "Specify builder for which to build (CBuilder, Tm2Builder)",      /* short description */
      "Specify builder for which to build (CBuilder, Tm2Builder"       /* long description */
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
                     0 /*GWEN_ARGS_MODE_ALLOW_FREEPARAM*/,
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
    fprintf(stdout,
            I18N("This command sets up the build folder.\n"
                 "Arguments:\n"
                 "%s\n"),
            GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    GWEN_DB_Group_free(db);
    return 0;
  }

  numThreads=GWEN_DB_GetIntValue(db, "jobs", 0, 1);
  builderName=GWEN_DB_GetCharValue(db, "builder", 0, NULL);

  buildCtx=GWB_BuildCtx_ReadFromXmlFile("buildctx.xml");
  if (buildCtx==NULL) {
    fprintf(stderr, "ERROR: Error reading build context from file.\n");
    GWEN_DB_Group_free(db);
    return 1;
  }

  /* build */
  rv=GWB_BuildCtx_Run(buildCtx, numThreads, 0, builderName);
  if (rv<0) {
    fprintf(stderr, "ERROR: Error building builds.\n");
    GWEN_DB_Group_free(db);
    return 2;
  }

  GWEN_DB_Group_free(db);
  return 0;
}




GWB_KEYVALUEPAIR_LIST *_readOptionsFromDb(GWEN_DB_NODE *db)
{
  int i;
  GWB_KEYVALUEPAIR_LIST *kvpList;

  kvpList=GWB_KeyValuePair_List_new();
  for (i=0; i<100; i++) {
    const char *sOption;
    GWEN_STRINGLIST *sl;

    sOption=GWEN_DB_GetCharValue(db, "option", i, NULL);
    if (sOption==NULL)
      break;

    sl=GWEN_StringList_fromString2(sOption, "=", 0, GWEN_TEXT_FLAGS_DEL_QUOTES | GWEN_TEXT_FLAGS_CHECK_BACKSLASH);
    if (sl) {
      const char *name;
      const char *value;

      name=GWEN_StringList_StringAt(sl, 0);
      value=GWEN_StringList_StringAt(sl, 1);
      if (name) {
        GWB_KEYVALUEPAIR *kvp;

        kvp=GWB_KeyValuePair_new(name, value);
        GWB_KeyValuePair_List_Add(kvp, kvpList);
      }
      GWEN_StringList_free(sl);
    }
  } /* for */

  if (GWB_KeyValuePair_List_GetCount(kvpList)==0) {
    GWB_KeyValuePair_List_free(kvpList);
    return NULL;
  }

  return kvpList;
}




