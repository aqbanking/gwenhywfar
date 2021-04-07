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


#define ARGS_COMMAND_SETUP   0x0001
#define ARGS_COMMAND_PREPARE 0x0002
#define ARGS_COMMAND_BUILD   0x0004


int _setup(GWEN_DB_NODE *dbArgs);
int _prepare(GWEN_DB_NODE *dbArgs);
int _build(GWEN_DB_NODE *dbArgs);


static GWB_KEYVALUEPAIR_LIST *_readOptionsFromDb(GWEN_DB_NODE *db);
static int _writeProjectFileList(const GWB_PROJECT *project, const char *fileName);
int _readArgsIntoDb(int argc, char **argv, GWEN_DB_NODE *db);




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
  if (rv<0) {
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
  commands|=GWEN_DB_GetIntValue(dbArgs, "prepare", 0, 0)?ARGS_COMMAND_PREPARE:0;
  commands|=GWEN_DB_GetIntValue(dbArgs, "build", 0, 0)?ARGS_COMMAND_BUILD:0;


  if (commands & ARGS_COMMAND_SETUP) {
    rv=_setup(dbArgs);
    if (rv!=0) {
      fprintf(stderr, "ERROR: Error on setup build environment.\n");
      return rv;
    }
  }

  if (commands & ARGS_COMMAND_PREPARE) {
    rv=_prepare(dbArgs);
    if (rv!=0) {
      fprintf(stderr, "ERROR: Error on preparing build environment.\n");
      return rv;
    }
  }

  if ((commands & ARGS_COMMAND_BUILD) || (commands==0)) {
    rv=_build(dbArgs);
    if (rv!=0) {
      fprintf(stderr, "ERROR: Error on building.\n");
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




int _setup(GWEN_DB_NODE *dbArgs)
{
  GWENBUILD *gwenbuild;
  GWB_PROJECT *project;
  GWB_BUILD_CONTEXT *buildCtx;
  const char *folder;
  int doDump;
  GWB_KEYVALUEPAIR_LIST *givenOptionList;
  int rv;

  doDump=GWEN_DB_GetIntValue(dbArgs, "dump", 0, 0);

  folder=GWEN_DB_GetCharValue(dbArgs, "folder", 0, NULL);
  if (!(folder && *folder)) {
    fprintf(stderr, "ERROR: Folder needed.\n");
    return 1;
  }
  if (strcasecmp(folder, ".")==0) {
    fprintf(stderr, "ERROR: Only building outside source folder supported.\n");
    return 1;
  }

  givenOptionList=_readOptionsFromDb(dbArgs);


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

  rv=GWB_BuildCtx_WriteToXmlFile(buildCtx, ".gwbuild.ctx");
  if (rv<0) {
    fprintf(stderr, "ERROR: Error writing build context file.\n");
    return 3;
  }

  rv=_writeProjectFileList(project, ".gwbuild.files");
  if (rv<0) {
    fprintf(stderr, "ERROR: Error writing file list file.\n");
    return 3;
  }

  rv=GWEN_DB_WriteFile(dbArgs, ".gwbuild.args", GWEN_DB_FLAGS_DEFAULT);
  if (rv<0) {
    fprintf(stderr, "ERROR: Error writing file list file.\n");
    return 3;
  }

  if (doDump) {
    GWB_Project_Dump(project, 2, 1);
  }

  return 0;
}



int _prepare(GWEN_UNUSED GWEN_DB_NODE *dbArgs)
{
  GWB_BUILD_CONTEXT *buildCtx;
  int rv;

  buildCtx=GWB_BuildCtx_ReadFromXmlFile(".gwbuild.ctx");
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



int _build(GWEN_DB_NODE *dbArgs)
{
  GWB_BUILD_CONTEXT *buildCtx;
  int rv;
  int numThreads;
  const char *builderName;

  numThreads=GWEN_DB_GetIntValue(dbArgs, "jobs", 0, 1);
  builderName=GWEN_DB_GetCharValue(dbArgs, "builder", 0, NULL);

  buildCtx=GWB_BuildCtx_ReadFromXmlFile(".gwbuild.ctx");
  if (buildCtx==NULL) {
    fprintf(stderr, "ERROR: Error reading build context from file.\n");
    return 2;
  }

  /* build */
  rv=GWB_BuildCtx_Run(buildCtx, numThreads, 0, builderName);
  if (rv<0) {
    fprintf(stderr, "ERROR: Error building builds.\n");
    return 3;
  }

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



int _writeProjectFileList(const GWB_PROJECT *project, const char *fileName)
{
  GWB_FILE_LIST2 *fileList;

  fileList=GWB_Project_GetFileList(project);
  if (fileList) {
    GWEN_XMLNODE *xmlNode;
    GWEN_XMLNODE *xmlFileList;
    int rv;
  
    xmlNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
    xmlFileList=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "FileList");
    GWB_File_List2_WriteXml(fileList, xmlFileList, "file");
    GWEN_XMLNode_AddChild(xmlNode, xmlFileList);
  
    rv=GWEN_XMLNode_WriteFile(xmlNode, fileName, GWEN_XML_FLAGS_DEFAULT | GWEN_XML_FLAGS_SIMPLE);
    GWEN_XMLNode_free(xmlNode);
    if (rv<0) {
      DBG_ERROR(NULL, "Error writing FileList to file \"%s\" (%d)", fileName, rv);
      return rv;
    }

  }

  return 0;
}



int _readArgsIntoDb(int argc, char **argv, GWEN_DB_NODE *db)
{
  int i=1;

  while(i<argc) {
    const char *s;
    int val;

    s=argv[i];
    if (s) {
      if (*s!='-') {
        /* no option, probably path to source folder */
        GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "folder", s);
      }
      else {
        if (strncasecmp(s, "-O", 2)==0) {
          /* options */
          s+=2;
          if (*s==0) {
            i++;
            if (i>=argc) {
              DBG_ERROR(NULL, "Missing argument for \"-O\"");
              return GWEN_ERROR_INVALID;
            }
            s=argv[i];
          }
          if (s && *s)
            GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, "option", s);
        }
        if (strncasecmp(s, "-B", 2)==0) {
          /* builder */
          s+=2;
          if (*s==0) {
            i++;
            if (i>=argc) {
              DBG_ERROR(NULL, "Missing argument for \"-B\"");
              return GWEN_ERROR_INVALID;
            }
            s=argv[i];
          }
          if (s && *s)
            GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, "builder", s);
        }
        if (strncasecmp(s, "-L", 2)==0) {
          /* loglevel */
          s+=2;
          if (*s==0) {
            i++;
            if (i>=argc) {
              DBG_ERROR(NULL, "Missing argument for \"-L\"");
              return GWEN_ERROR_INVALID;
            }
            s=argv[i];
          }
          if (s && *s)
            GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT, "loglevel", s);
        }
        else if (strcasecmp(s, "--dump")==0)
          GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "dump", 1);
        else if (strcasecmp(s, "-p")==0)
          GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "prepare", 1);
        else if (strcasecmp(s, "-s")==0)
          GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "setup", 1);
        else if (strcasecmp(s, "-b")==0)
          GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "build", 1);
        else if (strncasecmp(s, "-j", 2)==0) {
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

  return 0;
}



