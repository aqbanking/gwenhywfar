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
#include <gwenhywfar/directory.h>

#ifdef HAVE_SIGNAL_H
# include <signal.h>
#endif



#define I18N(msg) GWEN_I18N_Translate(PACKAGE, msg)
#define I18S(msg) msg


#define ARGS_COMMAND_SETUP          0x0001
#define ARGS_COMMAND_PREPARE        0x0002
#define ARGS_COMMAND_BUILD          0x0004
#define ARGS_COMMAND_REPEAT_SETUP   0x0008



static int _setup(GWEN_DB_NODE *dbArgs);
static int _prepare(GWEN_DB_NODE *dbArgs);
static int _build(GWEN_DB_NODE *dbArgs);

static int _repeatLastSetup(const char *fileName);

static int _prepareContextForSetup(GWB_CONTEXT *firstContext, GWEN_DB_NODE *dbArgs);
static void _determineTarget(GWB_CONTEXT *context, GWEN_DB_NODE *dbArgs);
static int _determineTools(GWB_CONTEXT *context, GWEN_DB_NODE *dbArgs);
static int _getAllToolPaths(GWEN_DB_NODE *dbVars, const char *sCompileTarget, const GWEN_STRINGLIST *sl);
static int _getToolPath(GWEN_DB_NODE *dbVars, const char *sTarget, const GWEN_STRINGLIST *sl,
                        const char *sCommand, const char *sEnvVar, const char *sVarName);
static void _copySomeEnvironmentVariablesToDb(GWEN_DB_NODE *db);
static void _copyEnvironmentVariableToDb(GWEN_DB_NODE *db, const char *envName, const char *dbVarName);

static GWB_KEYVALUEPAIR_LIST *_readOptionsFromDb(GWEN_DB_NODE *db);
static int _writeProjectFileList(const GWB_PROJECT *project, const char *fileName);
static int _writeBuildFileList(const GWENBUILD *gwenbuild, const char *fileName);
static GWEN_STRINGLIST *_readBuildFileList(const char *fileName);
static int _writeInstallFileList(const GWB_PROJECT *project, const char *fileName);

static int _buildFilesChanged(const char *fileName);
static int _filesChanged(const char *fileName, GWEN_STRINGLIST *slFileNameList);
static GWB_KEYVALUEPAIR_LIST *_generateInstallList(const GWB_FILE_LIST2 *fileList, const char *initialSourceDir);

static int _readArgsIntoDb(int argc, char **argv, GWEN_DB_NODE *db);
static int _handleStringArgument(int argc, char **argv, int *pIndex, const char *sArg, const char *sArgId,
                                 const char *sVarName, GWEN_DB_NODE *db);




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
  commands|=GWEN_DB_GetIntValue(dbArgs, "repeatSetup", 0, 0)?ARGS_COMMAND_REPEAT_SETUP:0;
  commands|=GWEN_DB_GetIntValue(dbArgs, "prepare", 0, 0)?ARGS_COMMAND_PREPARE:0;
  commands|=GWEN_DB_GetIntValue(dbArgs, "build", 0, 0)?ARGS_COMMAND_BUILD:0;


  if (commands & ARGS_COMMAND_SETUP) {
    rv=_setup(dbArgs);
    if (rv!=0) {
      fprintf(stderr, "ERROR: Error on setup build environment.\n");
      return rv;
    }
  }

  if (commands & ARGS_COMMAND_REPEAT_SETUP) {
    rv=_repeatLastSetup(".gwbuild.args");
    if (rv<0) {
      fprintf(stderr, "ERROR: Error on repeating setup.\n");
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
  GWB_CONTEXT *firstContext;
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
  firstContext=GWB_Context_new();
  rv=_prepareContextForSetup(firstContext, dbArgs);
  if (rv<0) {
    fprintf(stderr, "ERROR: Error preparing first context.\n");
    return 2;
  }

  project=GWB_Parser_ReadBuildTree(gwenbuild, firstContext, folder, givenOptionList);
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

  _writeBuildFileList(gwenbuild, ".gwbuild.buildfiles");

  rv=GWEN_DB_WriteFile(dbArgs, ".gwbuild.args", GWEN_DB_FLAGS_DEFAULT);
  if (rv<0) {
    fprintf(stderr, "ERROR: Error writing file list file.\n");
    return 3;
  }

  rv=_writeInstallFileList(project, ".gwbuild.installfiles");
  if (rv<0) {
    fprintf(stderr, "ERROR: Error writing install file list.\n");
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

  if (_buildFilesChanged(".gwbuild.buildfiles")) {
    fprintf(stdout, "Build files changed, repeating last setup command.\n");
    rv=_repeatLastSetup(".gwbuild.args");
    if (rv<0) {
      DBG_INFO(NULL, "here");
      return rv;
    }
  }

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

  if (_buildFilesChanged(".gwbuild.buildfiles")) {
    fprintf(stdout, "Build files changed, repeating last setup command.\n");
    rv=_repeatLastSetup(".gwbuild.args");
    if (rv<0) {
      DBG_INFO(NULL, "here");
      return rv;
    }
  }

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



int _repeatLastSetup(const char *fileName)
{
  int rv;
  GWEN_DB_NODE *db;

  db=GWEN_DB_Group_new("args");
  rv=GWEN_DB_ReadFile(db, fileName, GWEN_DB_FLAGS_DEFAULT);
  if (rv<0) {
    DBG_ERROR(NULL, "Error reading arguments from previous run (file \"%s\")", fileName);
    GWEN_DB_Group_free(db);
    return rv;
  }

  rv=_setup(db);
  if (rv!=0) {
    DBG_INFO(NULL, "here (%d)", rv);
    GWEN_DB_Group_free(db);
    return rv;
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



int _writeBuildFileList(const GWENBUILD *gwenbuild, const char *fileName)
{
  GWEN_STRINGLIST *sl;

  sl=GWBUILD_GetBuildFilenameList(gwenbuild);
  if (sl) {
    GWEN_XMLNODE *xmlRoot;
    GWEN_XMLNODE *xmlFileList;
    GWEN_STRINGLISTENTRY *se;
    int rv;

    xmlRoot=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
    xmlFileList=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "BuildFiles");
    GWEN_XMLNode_AddChild(xmlRoot, xmlFileList);

    se=GWEN_StringList_FirstEntry(sl);
    while(se) {
      const char *s;

      s=GWEN_StringListEntry_Data(se);
      if (s && *s) {
        GWEN_XMLNODE *xmlFile;
        GWEN_XMLNODE *xmlFileName;

        xmlFile=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "File");
        xmlFileName=GWEN_XMLNode_new(GWEN_XMLNodeTypeData, s);
        GWEN_XMLNode_AddChild(xmlFile, xmlFileName);
        GWEN_XMLNode_AddChild(xmlFileList, xmlFile);
      }

      se=GWEN_StringListEntry_Next(se);
    }

    rv=GWEN_XMLNode_WriteFile(xmlRoot, fileName, GWEN_XML_FLAGS_DEFAULT | GWEN_XML_FLAGS_SIMPLE);
    GWEN_XMLNode_free(xmlRoot);
    if (rv<0) {
      DBG_ERROR(NULL, "Error writing build file list to file \"%s\" (%d)", fileName, rv);
      return rv;
    }
  }

  return 0;
}



int _writeInstallFileList(const GWB_PROJECT *project, const char *fileName)
{
  GWB_KEYVALUEPAIR_LIST *genFileList;
  const char *initialSourceDir;

  initialSourceDir=GWB_Context_GetInitialSourceDir(GWB_Project_GetRootContext(project));

  genFileList=_generateInstallList(GWB_Project_GetFileList(project), initialSourceDir);
  if (genFileList) {
    GWB_KEYVALUEPAIR *kvp;
    GWEN_XMLNODE *xmlRoot;
    GWEN_XMLNODE *xmlFileList;
    int rv;

    xmlRoot=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
    xmlFileList=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "InstallFiles");
    GWEN_XMLNode_AddChild(xmlRoot, xmlFileList);

    kvp=GWB_KeyValuePair_List_First(genFileList);
    while(kvp) {
      const char *sDestPath;
      const char *sSrcPath;

      sDestPath=GWB_KeyValuePair_GetKey(kvp);
      sSrcPath=GWB_KeyValuePair_GetValue(kvp);
      if (sDestPath && sSrcPath) {
        GWEN_XMLNODE *xmlFile;

        xmlFile=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "InstallFile");
        GWEN_XMLNode_SetCharValue(xmlFile, "destination", sDestPath);
        GWEN_XMLNode_SetCharValue(xmlFile, "source", sSrcPath);
        GWEN_XMLNode_AddChild(xmlFileList, xmlFile);
      }

      kvp=GWB_KeyValuePair_List_Next(kvp);
    }

    rv=GWEN_XMLNode_WriteFile(xmlRoot, fileName, GWEN_XML_FLAGS_DEFAULT | GWEN_XML_FLAGS_SIMPLE);
    GWEN_XMLNode_free(xmlRoot);
    if (rv<0) {
      DBG_ERROR(NULL, "Error writing build file list to file \"%s\" (%d)", fileName, rv);
      return rv;
    }
  }

  return 0;
}



int _buildFilesChanged(const char *fileName)
{
  GWEN_STRINGLIST *slFileNameList;

  slFileNameList=_readBuildFileList(fileName);
  if (slFileNameList) {
    if (_filesChanged(fileName, slFileNameList)) {
      GWEN_StringList_free(slFileNameList);
      return 1;
    }
    GWEN_StringList_free(slFileNameList);
  }

  return 0;
}



int _filesChanged(const char *fileName, GWEN_STRINGLIST *slFileNameList)
{
  time_t mtSourceFile;
  GWEN_STRINGLISTENTRY *se;

  mtSourceFile=GWBUILD_GetModificationTimeOfFile(fileName);
  if (mtSourceFile==(time_t) 0) {
    DBG_INFO(NULL, "here");
    return 1; /* assume changed */
  }

  se=GWEN_StringList_FirstEntry(slFileNameList);
  while(se) {
    const char *s;

    s=GWEN_StringListEntry_Data(se);
    if (s && *s) {
      time_t mt;

      mt=GWBUILD_GetModificationTimeOfFile(s);
      if (mt!=(time_t) 0) {
        if (difftime(mt, mtSourceFile)>0) {
          DBG_ERROR(NULL, "File \"%s\" changed.", s);
          return 1;
        }
      }
    }

    se=GWEN_StringListEntry_Next(se);
  }

  DBG_DEBUG(NULL, "Files unchanged since last setup.");
  return 0;
}



GWEN_STRINGLIST *_readBuildFileList(const char *fileName)
{
  GWEN_XMLNODE *xmlRoot;
  GWEN_XMLNODE *xmlFileList;
  int rv;

  xmlRoot=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
  rv=GWEN_XML_ReadFile(xmlRoot, fileName, GWEN_XML_FLAGS_DEFAULT | GWEN_XML_FLAGS_SIMPLE);
  if (rv<0) {
    DBG_ERROR(NULL, "Error reading build file list from \"%s\"", fileName);
    GWEN_XMLNode_free(xmlRoot);
    return NULL;
  }

  xmlFileList=GWEN_XMLNode_FindFirstTag(xmlRoot, "BuildFiles", NULL, NULL);
  if (xmlFileList) {
    GWEN_STRINGLIST *sl;
    GWEN_XMLNODE *xmlFile;

    sl=GWEN_StringList_new();
    xmlFile=GWEN_XMLNode_FindFirstTag(xmlFileList, "File", NULL, NULL);
    while(xmlFile) {
      GWEN_XMLNODE *xmlFileName;

      xmlFileName=GWEN_XMLNode_GetFirstData(xmlFile);
      if (xmlFileName) {
        const char *s;

        s=GWEN_XMLNode_GetData(xmlFileName);
        if (s && *s)
          GWEN_StringList_AppendString(sl, s, 0, 1);
      }

      xmlFile=GWEN_XMLNode_FindNextTag(xmlFile, "File", NULL, NULL);
    }

    if (GWEN_StringList_Count(sl)==0) {
      GWEN_StringList_free(sl);
      GWEN_XMLNode_free(xmlRoot);
      return NULL;
    }
    GWEN_XMLNode_free(xmlRoot);
    return sl;
  }

  GWEN_XMLNode_free(xmlRoot);
  return NULL;
}



int _prepareContextForSetup(GWB_CONTEXT *context, GWEN_DB_NODE *dbArgs)
{
  GWEN_DB_NODE *dbVars;
  int rv;

  dbVars=GWB_Context_GetVars(context);
  GWEN_DB_SetCharValue(dbVars, GWEN_DB_FLAGS_OVERWRITE_VARS, "prefix", "/usr/local");

  _copySomeEnvironmentVariablesToDb(GWB_Context_GetVars(context));
  _determineTarget(context, dbArgs);
  rv=_determineTools(context, dbArgs);
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    return rv;
  }
  return 0;
}



void _determineTarget(GWB_CONTEXT *context, GWEN_DB_NODE *dbArgs)
{
  GWEN_DB_NODE *dbVars;
  const char *sTarget;
  const char *s;

  dbVars=GWB_Context_GetVars(context);

  sTarget=GWEN_DB_GetCharValue(dbArgs, "crossCompileFor", 0, NULL);
  if (sTarget) {
    s=GWBUILD_GetArchFromTriplet(sTarget);
    GWEN_DB_SetCharValue(dbVars, GWEN_DB_FLAGS_OVERWRITE_VARS, "GWBUILD_ARCH", s);
    s=GWBUILD_GetSystemFromTriplet(sTarget);
    GWEN_DB_SetCharValue(dbVars, GWEN_DB_FLAGS_OVERWRITE_VARS, "GWBUILD_SYSTEM", s);
  }
  else {
    GWEN_DB_SetCharValue(dbVars, GWEN_DB_FLAGS_OVERWRITE_VARS, "GWBUILD_ARCH", GWBUILD_GetHostArch());
    GWEN_DB_SetCharValue(dbVars, GWEN_DB_FLAGS_OVERWRITE_VARS, "GWBUILD_SYSTEM", GWBUILD_GetHostSystem());
  }
}



int _determineTools(GWB_CONTEXT *context, GWEN_DB_NODE *dbArgs)
{
  GWEN_DB_NODE *dbVars;
  const char *sTarget;
  GWEN_STRINGLIST *sl;
  int rv;

  dbVars=GWB_Context_GetVars(context);
  sTarget=GWEN_DB_GetCharValue(dbArgs, "crossCompileFor", 0, NULL);

  sl=GWBUILD_GetPathFromEnvironment();
  if (sl==NULL)
    sl=GWEN_StringList_new();

  GWEN_StringList_AppendString(sl, "/usr/local/bin", 0, 1);
  GWEN_StringList_AppendString(sl, "/usr/bin", 0, 1);
  GWEN_StringList_AppendString(sl, "/bin", 0, 1);

  rv=_getAllToolPaths(dbVars, sTarget, sl);
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    GWEN_StringList_free(sl);
    return rv;
  }
  GWEN_StringList_free(sl);

  return 0;
}



int _getAllToolPaths(GWEN_DB_NODE *dbVars, const char *sCompileTarget, const GWEN_STRINGLIST *sl)
{
  int rv;

  rv=_getToolPath(dbVars, sCompileTarget, sl, "gcc", "CC", "GWBUILD_TOOL_CC");
  if (rv<0)
    return rv;

  rv=_getToolPath(dbVars, sCompileTarget, sl, "g++", "CXX", "GWBUILD_TOOL_CXX");
  if (rv<0)
    return rv;

  rv=_getToolPath(dbVars, sCompileTarget, sl, "ld", "LD", "GWBUILD_TOOL_LD");
  if (rv<0)
    return rv;

  rv=_getToolPath(dbVars, sCompileTarget, sl, "ar", "AR", "GWBUILD_TOOL_AR");
  if (rv<0)
    return rv;

  rv=_getToolPath(dbVars, sCompileTarget, sl, "ranlib", "RANLIB", "GWBUILD_TOOL_RANLIB");
  if (rv<0)
    return rv;

  rv=_getToolPath(dbVars, NULL, sl, "typemaker2", "TYPEMAKER2", "GWBUILD_TOOL_TYPEMAKER2");
  if (rv<0)
    return rv;


  return 0;
}



int _getToolPath(GWEN_DB_NODE *dbVars, const char *sCompileTarget, const GWEN_STRINGLIST *sl,
                 const char *sCommand, const char *sEnvVar, const char *sVarName)
{
  GWEN_BUFFER *fbufIn;
  GWEN_BUFFER *fbufOut;
  const char *s;
  int rv;

  fbufIn=GWEN_Buffer_new(0, 256, 0, 1);

  if (sCompileTarget && *sCompileTarget) {
    GWEN_Buffer_AppendString(fbufIn, sCompileTarget);
    GWEN_Buffer_AppendString(fbufIn, "-");
  }

  s=getenv(sEnvVar);
  if (s)
    GWEN_Buffer_AppendString(fbufIn, s);
  else
    GWEN_Buffer_AppendString(fbufIn, sCommand);

  fbufOut=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_Directory_FindFileInPaths(sl, GWEN_Buffer_GetStart(fbufIn), fbufOut);
  if (rv<0) {
    DBG_ERROR(NULL, "Tool \"%s\" not available (%d)", GWEN_Buffer_GetStart(fbufIn), rv);
    GWEN_Buffer_free(fbufOut);
    GWEN_Buffer_free(fbufIn);
    return rv;
  }

  GWEN_DB_SetCharValue(dbVars, GWEN_DB_FLAGS_OVERWRITE_VARS, sVarName, GWEN_Buffer_GetStart(fbufOut));
  GWEN_Buffer_free(fbufOut);
  GWEN_Buffer_free(fbufIn);
  return 0;
}



void _copySomeEnvironmentVariablesToDb(GWEN_DB_NODE *db)
{
  _copyEnvironmentVariableToDb(db, "CFLAGS", "CFLAGS");
  _copyEnvironmentVariableToDb(db, "CXXFLAGS", "CXXFLAGS");
  _copyEnvironmentVariableToDb(db, "CPPFLAGS", "CPPFLAGS");
  _copyEnvironmentVariableToDb(db, "LDFLAGS", "LDFLAGS");
  _copyEnvironmentVariableToDb(db, "TM2FLAGS", "TM2FLAGS");
}





void _copyEnvironmentVariableToDb(GWEN_DB_NODE *db, const char *envName, const char *dbVarName)
{
  const char *s;

  s=getenv(envName);
  if (s && *s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, dbVarName, s);
}



GWB_KEYVALUEPAIR_LIST *_generateInstallList(const GWB_FILE_LIST2 *fileList, const char *initialSourceDir)
{
  if (fileList) {
    GWB_FILE_LIST2_ITERATOR *it;

    it=GWB_File_List2_First(fileList);
    if (it) {
      GWB_FILE *file;
      GWB_KEYVALUEPAIR_LIST *kvpList;
      GWEN_BUFFER *keyBuf;
      GWEN_BUFFER *valueBuf;

      keyBuf=GWEN_Buffer_new(0, 256, 0, 1);
      valueBuf=GWEN_Buffer_new(0, 256, 0, 1);
      kvpList=GWB_KeyValuePair_List_new();
      file=GWB_File_List2Iterator_Data(it);
      while(file) {
        if (GWB_File_GetFlags(file) & GWB_FILE_FLAGS_INSTALL) {
          const char *s;
          GWB_KEYVALUEPAIR *kvp;

          s=GWB_File_GetInstallPath(file);
          if (s && *s) {
            GWEN_Buffer_AppendString(keyBuf, s);
            GWEN_Buffer_AppendString(keyBuf, GWEN_DIR_SEPARATOR_S);
            GWEN_Buffer_AppendString(keyBuf, GWB_File_GetName(file));

            GWB_File_WriteFileNameToTopBuildDirString(file, initialSourceDir, valueBuf);
            kvp=GWB_KeyValuePair_new(GWEN_Buffer_GetStart(keyBuf),  GWEN_Buffer_GetStart(valueBuf));
            GWB_KeyValuePair_List_Add(kvp, kvpList);
            GWEN_Buffer_Reset(valueBuf);
            GWEN_Buffer_Reset(keyBuf);
          }
        }
        file=GWB_File_List2Iterator_Next(it);
      }
      GWEN_Buffer_free(valueBuf);
      GWEN_Buffer_free(keyBuf);
      GWB_File_List2Iterator_free(it);
      if (GWB_KeyValuePair_List_GetCount(kvpList)==0) {
        GWB_KeyValuePair_List_free(kvpList);
        return NULL;
      }
      return kvpList;
    }
  }

  return NULL;
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
        /* folder only needed in setup mode, assume that */
        GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "setup", 1);
      }
      else {
        int rv;

        if (strncasecmp(s, "-O", 2)==0) {
          rv=_handleStringArgument(argc, argv, &i, s+2, "-O", "option",db);
          if (rv<0)
            return rv;
        }
        if (strncasecmp(s, "-B", 2)==0) {
          rv=_handleStringArgument(argc, argv, &i, s+2, "-B", "builder",db);
          if (rv<0)
            return rv;
        }
        if (strncasecmp(s, "-L", 2)==0) {
          rv=_handleStringArgument(argc, argv, &i, s+2, "-L", "loglevel",db);
          if (rv<0)
            return rv;
        }
        if (strncasecmp(s, "-C", 2)==0) {
          rv=_handleStringArgument(argc, argv, &i, s+2, "-C", "crossCompileFor",db);
          if (rv<0)
            return rv;
        }
        else if (strcasecmp(s, "--dump")==0)
          GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "dump", 1);
        else if (strcasecmp(s, "-p")==0)
          GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "prepare", 1);
        else if (strcasecmp(s, "-s")==0)
          GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "setup", 1);
        else if (strcasecmp(s, "-r")==0)
          GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "repeatSetup", 1);
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



