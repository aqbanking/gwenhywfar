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
#include "utils.h"
#include "c_setup.h"
#include "c_prepare.h"
#include "c_build.h"
#include "c_install.h"
#include "c_clean.h"

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/cgui.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/args.h>
#include <gwenhywfar/i18n.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/directory.h>

#include <unistd.h>

#ifdef HAVE_SIGNAL_H
# include <signal.h>
#endif



#define I18N(msg) GWEN_I18N_Translate(PACKAGE, msg)
#define I18S(msg) msg


#define ARGS_COMMAND_SETUP          0x0001
#define ARGS_COMMAND_PREPARE        0x0002
#define ARGS_COMMAND_BUILD          0x0004
#define ARGS_COMMAND_REPEAT_SETUP   0x0008
#define ARGS_COMMAND_INSTALL        0x0010
#define ARGS_COMMAND_CLEAN          0x0020



static int _readArgsIntoDb(int argc, char **argv, GWEN_DB_NODE *db);
static int _handleStringArgument(int argc, char **argv, int *pIndex, const char *sArg, const char *sArgId,
                                 const char *sVarName, GWEN_DB_NODE *db);
static void _printHelpScreen();




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




int main(int argc, char **argv)
{
  GWEN_DB_NODE *dbArgs;
  int rv;
  int err;
  uint32_t commands=0;
  const char *s;
  GWEN_GUI *gui;

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

  dbArgs=GWEN_DB_Group_new("args");
  rv=_readArgsIntoDb(argc, argv, dbArgs);
  if (rv<0 || rv==1) {
    return 1;
  }

  s=GWEN_DB_GetCharValue(dbArgs, "loglevel", 0, NULL);
  if (s && *s) {
    GWEN_LOGGER_LEVEL level;

    level=GWEN_Logger_Name2Level(s);
    if (level==GWEN_LoggerLevel_Unknown) {
    }
    else
      GWEN_Logger_SetLevel(NULL, level);
  }

  commands|=GWEN_DB_GetIntValue(dbArgs, "setup", 0, 0)?ARGS_COMMAND_SETUP:0;
  commands|=GWEN_DB_GetIntValue(dbArgs, "repeatSetup", 0, 0)?ARGS_COMMAND_REPEAT_SETUP:0;
  commands|=GWEN_DB_GetIntValue(dbArgs, "prepare", 0, 0)?ARGS_COMMAND_PREPARE:0;
  commands|=GWEN_DB_GetIntValue(dbArgs, "build", 0, 0)?ARGS_COMMAND_BUILD:0;
  commands|=GWEN_DB_GetIntValue(dbArgs, "install", 0, 0)?ARGS_COMMAND_INSTALL:0;
  commands|=GWEN_DB_GetIntValue(dbArgs, "clean", 0, 0)?ARGS_COMMAND_CLEAN:0;


  if (commands & ARGS_COMMAND_SETUP) {
    rv=GWB_Setup(dbArgs);
    if (rv!=0) {
      fprintf(stderr, "ERROR: Error on setup build environment.\n");
      return rv;
    }
  }

  if (commands & ARGS_COMMAND_REPEAT_SETUP) {
    rv=GWB_RepeatLastSetup(".gwbuild.args");
    if (rv<0) {
      fprintf(stderr, "ERROR: Error on repeating setup.\n");
      return rv;
    }
  }

  if (commands & ARGS_COMMAND_PREPARE) {
    rv=GWB_Prepare(dbArgs);
    if (rv!=0) {
      fprintf(stderr, "ERROR: Error on preparing build environment.\n");
      return rv;
    }
  }

  if ((commands & ARGS_COMMAND_BUILD) || (commands==0)) {
    rv=GWB_Build(dbArgs);
    if (rv!=0) {
      fprintf(stderr, "ERROR: Error on building.\n");
      return rv;
    }
  }

  if (commands & ARGS_COMMAND_INSTALL) {
    rv=GWB_InstallFiles(".gwbuild.installfiles", getenv("DESTDIR"));
    if (rv!=0) {
      fprintf(stderr, "ERROR: Error on installing.\n");
      return rv;
    }
  }

  if (commands & ARGS_COMMAND_CLEAN) {
    rv=GWB_Clean(".gwbuild.files");
    if (rv!=0) {
      fprintf(stderr, "ERROR: Error on cleaning generated files.\n");
      return rv;
    }
  }


  err=GWEN_Fini();
  if (err) {
    fprintf(stderr,
            "WARNING: Could not deinitialize Gwenhywfar.\n");
  }

  return 0;
}



int _handleStringArgument(int argc, char **argv, int *pIndex, const char *sArg, const char *sArgId, const char *sVarName, GWEN_DB_NODE *db)
{
  int i;

  i=*pIndex;
  if (*sArg==0) {
    i++;
    if (i>=argc) {
      DBG_ERROR(NULL, "Missing argument for \"%s\"", sArgId);
      return GWEN_ERROR_INVALID;
    }
    sArg=argv[i];
  }
  if (sArg && *sArg)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, sVarName, sArg);
  *pIndex=i;
  return 0;
}



int _readArgsIntoDb(int argc, char **argv, GWEN_DB_NODE *db)
{
  int i=1;
  int help=0;

  while(i<argc) {
    const char *s;
    int val;

    s=argv[i];
    if (s) {
      if (*s!='-') {
        /* no option, probably path to source folder */
        GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "folder", s);
        /* folder only needed in setup mode, assume that */
        GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "setup", 1);
      }
      else {
        int rv;

        if (strncmp(s, "-O", 2)==0) {
          rv=_handleStringArgument(argc, argv, &i, s+2, "-O", "option",db);
          if (rv<0)
            return rv;
        }
        if (strncmp(s, "-B", 2)==0) {
          rv=_handleStringArgument(argc, argv, &i, s+2, "-B", "builder",db);
          if (rv<0)
            return rv;
        }
        if (strncmp(s, "-L", 2)==0) {
          rv=_handleStringArgument(argc, argv, &i, s+2, "-L", "loglevel",db);
          if (rv<0)
            return rv;
        }
        if (strncmp(s, "-C", 2)==0) {
          rv=_handleStringArgument(argc, argv, &i, s+2, "-C", "crossCompileFor",db);
          if (rv<0)
            return rv;
        }
        else if (strcmp(s, "--dump")==0)
          GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "dump", 1);
	else if (strcmp(s, "--help")==0) {
	  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "help", 1);
	  help=1;
	}
        else if (strcmp(s, "-p")==0)
          GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "prepare", 1);
        else if (strcmp(s, "-s")==0)
          GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "setup", 1);
        else if (strcmp(s, "-r")==0)
          GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "repeatSetup", 1);
        else if (strcmp(s, "-b")==0)
          GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "build", 1);
        else if (strcmp(s, "-i")==0)
          GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "install", 1);
        else if (strcmp(s, "-c")==0)
          GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "clean", 1);
        else if (strncmp(s, "-j", 2)==0) {
          /* jobs */
          s+=2;
          if (*s==0) {
            i++;
            if (i>=argc) {
              DBG_ERROR(NULL, "Missing argument for \"-j\"");
              return GWEN_ERROR_INVALID;
            }
            s=argv[i];
          }
          if (1!=sscanf(s, "%d", &val)) {
            DBG_ERROR(NULL, "Non-integer argument for \"-j\"");
            return GWEN_ERROR_INVALID;
          }
          GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "jobs", val);
        }
      }
    }
    i++;
  } /* while */

  if (help) {
    _printHelpScreen();
    return 1;
  }

  return 0;
}



void _printHelpScreen()
{
  fprintf(stdout,
	  "\n"
	  "Gwenhywfar Build Tool " GWENHYWFAR_VERSION_FULL_STRING "\n"
	  "\n"
	  "Building a project (e.g. compiling and linking) is done in multiple steps.\n"
	  "\n"
	  "1. Setup Build Environment\n"
	  "-------------------------\n"
	  "A. Create files named 0BUILD inside your project (see project AqFinance for\n"
	  "example files).\n"
	  "B. Create an empty folder and change into it (all next commands are run\n"
	  "from there).\n"
	  "You might want to use a folder like 'build' inside the source tree of\n"
	  "your project.\n"
	  "3. run\n"
	  "      gwbuild PATH_TO_SOURCE_TREE\n"
	  "e.g.  gwbuild ..\n"
	  "\n"
	  "2. Prepare Buidling\n"
	  "-------------------\n"
	  "      gwbuild -p\n"
	  "This is only needed if your project uses typemaker2 to generate c-sources from\n"
	  "XML files.\n"
	  "This command makes typemaker2 create its derived type description files needed\n"
	  "when referencing\n"
	  "typemaker2 generated types inside another typemaker2 generated type.\n"
	  "\n"
	  "3. Build Typemaker2 Files\n"
	  "-------------------------\n"
	  "     gwbuild -Btm2builder\n"
	  "This also is only needed if typemaker2 files are used in your project.\n"
	  "\n"
	  "4. Build All Other Targets\n"
	  "--------------------------\n"
	  "     gwbuild\n"
	  "This uses a single process to compile and link the project files.\n"
	  "If you have multiple processor cores/threads you can build multiple files in\n"
	  "parallel:\n"
	  "     gwbuild -j14\n"
	  "This command uses 14 processes in parallel.\n"
	  "The step can be repeated as often as needed. It will automatically check for\n"
	  "changed files and try to only re-compile/link those modified files and others\n"
	  "which depend on them.\n"
	  "If you change a 0BUILD file gwbuild will automatically call the setup step\n"
	  "using the same\n"
	  "arguments given to the last setup command (-s). This typically leads to all\n"
	  "files being re-build.\n"
	  "\n"
	  "Option List\n"
	  "-----------\n"
	  "-s           setup build environment\n"
	  "-p           run preparation commands (needed e.g. if typemaker2 is used)\n"
	  "-b           build targets\n"
	  "-i           install files\n"
	  "-c           cleanup; delete generated files\n"
	  "-r           repeat setup command using the same arguments given to last setup\n"
	  "-Oname=value specify options (can occur multiple times)\n"
	  "-Bname       Only run commands for the given build (mostly used with\n"
	  "             'tm2builder')\n"
	  "-Lname       Set loglevel (debug, info, notice, warn, error)\n"
	  "-Cname       Crosscompile for given environment (e-g- 'x86_64-w64-mingw32')\n"
	  "-jvalue      Use the given number of parallel process for building\n"
	 );
}




