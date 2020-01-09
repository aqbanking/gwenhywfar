/***************************************************************************
 begin       : Thu Jan 09 2020
 copyright   : (C) 2020 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "testframework_p.h"
#include "testgui_l.h"

#include <gwenhywfar/args.h>
#include <gwenhywfar/error.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/debug.h>




static int _runModule(GWEN_GUI *gui, GWEN_TEST_MODULE *mod, int depth);
static int _runTest(GWEN_GUI *gui, GWEN_TEST_MODULE *mod, int depth);
static GWEN_GUI *_setupGui(GWEN_TEST_FRAMEWORK *tf, int argc, char **argv);
static void _finishGui(GWEN_GUI *gui);
static GWEN_DB_NODE *_readCommandLine(int argc, char **argv);





int TestFramework_Run(GWEN_TEST_FRAMEWORK *tf, int argc, char **argv)
{
  int rv;
  GWEN_GUI *gui;

  assert(tf);

  gui=_setupGui(tf, argc, argv);
  if (gui==NULL) {
    fprintf(stderr, "ERROR: Could not setup GUI\n");
    return GWEN_ERROR_INVALID;
  }
  rv=_runModule(gui, tf->modulesRoot, 0);
  _finishGui(gui);

  return rv;
}



GWEN_GUI *_setupGui(GWEN_TEST_FRAMEWORK *tf, int argc, char **argv)
{
  GWEN_DB_NODE *dbArgs;
  GWEN_GUI *gui;
  const char *s;
  int i;

  dbArgs=_readCommandLine(argc, argv);
  if (dbArgs==NULL) {
    fprintf(stderr, "ERROR: Could not parse arguments\n");
    TestFramework_free(tf);
    return NULL;
  }

  gui=GWEN_Gui_GetGui();
  if (gui==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Need to create and set a GUI first!");
    return NULL;
  }

  TestGui_Extend(gui);

  s=GWEN_DB_GetCharValue(dbArgs, "logFile", 0, NULL);
  if (s && *s)
    TestGui_SetLogFile(gui, s);
  i=GWEN_DB_GetIntValue(dbArgs, "logLastLines", 0, 100);
  TestGui_SetLogLastX(gui, i);

  return gui;
}



void _finishGui(GWEN_GUI *gui)
{
  Test_Gui_FlushLogs(gui);
  TestGui_Unextend(gui);
}



int _runModule(GWEN_GUI *gui, GWEN_TEST_MODULE *mod, int depth)
{
  int rv;
  GWEN_TEST_MODULE *subMod;
  int subModsFailed=0;

  subMod=GWEN_Test_Module_Tree2_GetFirstChild(mod);
  if (subMod) {
    const char *sName;
    GWEN_BUFFER *fbuf;
    int i;

    /* has sub modules, so is no test */
    sName=GWEN_Test_Module_GetName(mod);
    fbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(fbuf, "==== Module: ");
    if (sName && *sName)
      GWEN_Buffer_AppendString(fbuf, sName);
    GWEN_Buffer_AppendString(fbuf, "==== \n");
    TestGui_AddLogLine(gui, GWEN_Buffer_GetStart(fbuf));
    GWEN_Buffer_free(fbuf);

    for (i=0; i<depth; i++)
      fprintf(stdout, " ");
    fprintf(stdout, "Module %s\n", (sName && *sName)?sName:"");

    subMod=GWEN_Test_Module_Tree2_GetFirstChild(mod);
    while(subMod) {
      rv=_runModule(gui, subMod, depth+1);
      if (rv<0 && rv!=GWEN_ERROR_NOT_IMPLEMENTED) {
        subModsFailed++;
      }
      subMod=GWEN_Test_Module_Tree2_GetNext(subMod);
    }
  
    GWEN_Test_Module_SetResult(mod, subModsFailed);
    return 0;
  }
  else {
    /* is a test with no sub modules, run directly */
    rv=_runTest(gui, mod, depth);
    if (rv<0)
      return rv;

    return 0;
  }
}



int _runTest(GWEN_GUI *gui, GWEN_TEST_MODULE *mod, int depth)
{
  const char *sName;
  GWEN_BUFFER *fbuf;
  int rv;

  sName=GWEN_Test_Module_GetName(mod);
  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(fbuf, "---- Starting Test: ");
  if (sName && *sName)
    GWEN_Buffer_AppendString(fbuf, sName);
  GWEN_Buffer_AppendString(fbuf, "---- \n");
  TestGui_AddLogLine(gui, GWEN_Buffer_GetStart(fbuf));
  GWEN_Buffer_Reset(fbuf);

  GWEN_Buffer_FillWithBytes(fbuf, ' ', depth);
  if (sName && *sName)
    GWEN_Buffer_AppendString(fbuf, sName);
  else
    GWEN_Buffer_AppendString(fbuf, "Test");
  GWEN_Buffer_AppendString(fbuf, ": ");

  rv=GWEN_Test_Module_Test(mod);
  if (rv!=GWEN_ERROR_NOT_IMPLEMENTED) {
    if (rv<0) {
      GWEN_Test_Module_SetResult(mod, rv);
      TestGui_AddLogLine(gui, "->FAILED!\n");
      GWEN_Buffer_AppendString(fbuf, "FAILED!");
      fprintf(stdout, "%s\n", GWEN_Buffer_GetStart(fbuf));
      GWEN_Buffer_free(fbuf);
      return rv;
    }
    TestGui_AddLogLine(gui, "->passed\n");
    GWEN_Buffer_AppendString(fbuf, "passed.");
    fprintf(stdout, "%s\n", GWEN_Buffer_GetStart(fbuf));
  }
  GWEN_Buffer_Reset(fbuf);

  GWEN_Buffer_AppendString(fbuf, "---- Ended Test   : ");
  if (sName && *sName)
    GWEN_Buffer_AppendString(fbuf, sName);
  GWEN_Buffer_AppendString(fbuf, "---- \n");
  TestGui_AddLogLine(gui, GWEN_Buffer_GetStart(fbuf));
  GWEN_Buffer_free(fbuf);
  return 0;
}



void TestFramework_AddModule(GWEN_TEST_FRAMEWORK *tf, GWEN_TEST_MODULE *mod)
{
  assert(tf);
  assert(tf->modulesRoot);
  GWEN_Test_Module_Tree2_AddChild(tf->modulesRoot, mod);
}



GWEN_TEST_FRAMEWORK *TestFramework_new()
{
  GWEN_TEST_FRAMEWORK *tf;

  GWEN_NEW_OBJECT(GWEN_TEST_FRAMEWORK, tf);
  assert(tf);
  tf->modulesRoot=GWEN_Test_Module_new();
  GWEN_Test_Module_SetName(tf->modulesRoot, "Root");

  return tf;
}



void TestFramework_free(GWEN_TEST_FRAMEWORK *tf)
{
  if (tf) {
    GWEN_Test_Module_free(tf->modulesRoot);
    GWEN_FREE_OBJECT(tf);
  }
}



GWEN_TEST_MODULE *TestFramework_GetModulesRoot(const GWEN_TEST_FRAMEWORK *tf)
{
  assert(tf);
  return tf->modulesRoot;
}





GWEN_DB_NODE *_readCommandLine(int argc, char **argv)
{
  GWEN_DB_NODE *db;
  int rv;
  const GWEN_ARGS args[]= {
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
      "logFile",                    /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "L",                          /* short option */
      "logFile",                    /* long option */
      "Specify the log file (default: stdout)", /* short description */
      "Specify the log file (default: stdout))"  /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Int,            /* type */
      "logLastLines",               /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      0,                            /* short option */
      "logLastLines",                    /* long option */
      "Set size of log line buffer (default: 100 lines)", /* short description */
      "Set size of log line buffer (default: 100 lines)"  /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HELP | GWEN_ARGS_FLAGS_LAST, /* flags */
      GWEN_ArgsType_Int,            /* type */
      "help",                       /* name */
      0,                            /* minnum */
      0,                            /* maxnum */
      "h",                          /* short option */
      "help",                       /* long option */
      "Show this help screen",      /* short description */
      "Show this help screen"       /* long description */
    }
  };

  db=GWEN_DB_Group_new("args");
  rv=GWEN_Args_Check(argc, argv, 1,
                     0 /*GWEN_ARGS_MODE_ALLOW_FREEPARAM*/,
                     args,
                     db);
  if (rv==GWEN_ARGS_RESULT_ERROR) {
    fprintf(stderr, "ERROR: Could not parse arguments\n");
    GWEN_DB_Group_free(db);
    return NULL;
  }
  else if (rv==GWEN_ARGS_RESULT_HELP) {
    GWEN_BUFFER *ubuf;

    ubuf=GWEN_Buffer_new(0, 1024, 0, 1);
    if (GWEN_Args_Usage(args, ubuf, GWEN_ArgsOutType_Txt)) {
      fprintf(stderr, "ERROR: Could not create help string\n");
      GWEN_DB_Group_free(db);
      return NULL;
    }
    fprintf(stdout, "%s\n", GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    GWEN_DB_Group_free(db);
    return NULL;
  }

  return db;
}




