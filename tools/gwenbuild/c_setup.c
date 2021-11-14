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


#include "c_setup.h"
#include "utils.h"
#include "gwenbuild/parser/parser.h"
#include "gwenbuild/buildctx/buildctx_xml.h"
#include "gwenbuild/filenames.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/text.h>

#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>



static int _prepareContextForSetup(GWB_CONTEXT *firstContext, GWEN_DB_NODE *dbArgs);
static void _determineTarget(GWB_CONTEXT *context, GWEN_DB_NODE *dbArgs);
static int _determineTools(GWB_CONTEXT *context, GWEN_DB_NODE *dbArgs);
static int _getAllToolPaths(GWEN_DB_NODE *dbVars, const char *sCompileTarget, const GWEN_STRINGLIST *sl);
static int _getToolPath(GWEN_DB_NODE *dbVars, const char *sTarget, const GWEN_STRINGLIST *sl,
                        const char *sCommand, const char *sEnvVar, const char *sVarName);
static void _copySomeEnvironmentVariablesToDb(GWEN_DB_NODE *db);
static void _copyEnvironmentVariableToDb(GWEN_DB_NODE *db, const char *envName, const char *dbVarName);
static GWB_KEYVALUEPAIR_LIST *_readOptionsFromDb(GWEN_DB_NODE *db);
static int _writeStaticLibHelper(const char *fileName);




int GWB_Setup(GWEN_DB_NODE *dbArgs)
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
  if (GWEN_DB_GetIntValue(dbArgs, "static", 0, 0))
    GWBUILD_AddFlags(gwenbuild, GWENBUILD_FLAGS_STATIC);

  firstContext=GWB_Context_new(gwenbuild);
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

  rv=GWB_BuildCtx_WriteToXmlFile(buildCtx, GWBUILD_FILE_CTX);
  if (rv<0) {
    fprintf(stderr, "ERROR: Error writing build context file.\n");
    return 3;
  }

  rv=GWB_Utils_WriteProjectFileList(project, GWBUILD_FILE_FILES);
  if (rv<0) {
    fprintf(stderr, "ERROR: Error writing file list file.\n");
    return 3;
  }

  GWB_Utils_WriteBuildFileList(gwenbuild, GWBUILD_FILE_BUILDFILES);

  rv=GWEN_DB_WriteFile(dbArgs, GWBUILD_FILE_ARGS, GWEN_DB_FLAGS_DEFAULT);
  if (rv<0) {
    fprintf(stderr, "ERROR: Error writing file list file.\n");
    return 3;
  }

  rv=GWB_Utils_WriteInstallFileList(project, GWBUILD_FILE_INSTALLFILES);
  if (rv<0) {
    fprintf(stderr, "ERROR: Error writing install file list.\n");
    return 3;
  }

#if 0
  rv=GWB_Utils_WriteProjectToFile(project, GWBUILD_FILE_PROJECT);
  if (rv<0) {
    fprintf(stderr, "ERROR: Error writing project to file.\n");
    return 3;
  }
#endif

  rv=GWB_Utils_WriteProjectInfoToFile(project, GWBUILD_FILE_PROJECTINFO);
  if (rv<0) {
    fprintf(stderr, "ERROR: Error writing project info to file.\n");
    return 3;
  }


  rv=GWB_Utils_WriteContextTreeToFile(GWB_Project_GetRootContext(project), GWBUILD_FILE_CTXTREE);

  rv=_writeStaticLibHelper("staticlibhelper.sh");
  if (rv<0) {
    fprintf(stderr, "ERROR: Error writing staticlibhelper.\n");
    return 3;
  }


  if (doDump) {
    GWB_Project_Dump(project, 2, 1);
  }

  return 0;
}



int GWB_RepeatLastSetup(const char *fileName)
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

  rv=GWB_Setup(db);
  if (rv!=0) {
    DBG_INFO(NULL, "here (%d)", rv);
    GWEN_DB_Group_free(db);
    return rv;
  }
  GWEN_DB_Group_free(db);

  return 0;
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
  GWENBUILD *gwenbuild;
  GWEN_DB_NODE *dbVars;
  const char *sTarget;
  const char *sTargetSystem;
  const char *s;
  int systemIsWindows=0;

  dbVars=GWB_Context_GetVars(context);

  gwenbuild=GWB_Context_GetGwenbuild(context);

  sTarget=GWEN_DB_GetCharValue(dbArgs, "crossCompileFor", 0, NULL);
  if (sTarget) {
    s=GWBUILD_GetArchFromTriplet(sTarget);
    GWEN_DB_SetCharValue(dbVars, GWEN_DB_FLAGS_OVERWRITE_VARS, "GWBUILD_ARCH", s);
    sTargetSystem=GWBUILD_GetSystemFromTriplet(sTarget);
  }
  else {
    GWEN_DB_SetCharValue(dbVars, GWEN_DB_FLAGS_OVERWRITE_VARS, "GWBUILD_ARCH", GWBUILD_GetHostArch());
    sTargetSystem=GWBUILD_GetHostSystem();
  }
  systemIsWindows=(strcasecmp(sTargetSystem, "windows")==0)?1:0;

  if (GWBUILD_GetFlags(gwenbuild) & GWENBUILD_FLAGS_STATIC)
    GWEN_DB_SetCharValue(dbVars, GWEN_DB_FLAGS_OVERWRITE_VARS, "GWBUILD_LIBTYPE", "staticlib");
  else
    GWEN_DB_SetCharValue(dbVars, GWEN_DB_FLAGS_OVERWRITE_VARS, "GWBUILD_LIBTYPE", "sharedlib");

  GWEN_DB_SetCharValue(dbVars, GWEN_DB_FLAGS_OVERWRITE_VARS, "GWBUILD_SYSTEM", sTargetSystem);
  GWBUILD_SetTargetSystem(gwenbuild, sTargetSystem);
  GWBUILD_SetTargetIsWindows(gwenbuild, systemIsWindows);
  GWEN_DB_SetCharValue(dbVars, GWEN_DB_FLAGS_OVERWRITE_VARS, "GWBUILD_SYSTEMTYPE", systemIsWindows?"windows":"posix");
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

  /* optional tools */
  rv=_getToolPath(dbVars, sCompileTarget, sl, "msgfmt", "MSGFMT", "GWBUILD_TOOL_MSGFMT");
  if (rv<0) {
    DBG_WARN(NULL, "Tool msgfmt not found, limited i18n support.");
    GWEN_DB_SetCharValue(dbVars, GWEN_DB_FLAGS_OVERWRITE_VARS, "msgfmt_EXISTS", "FALSE");
  }
  else
    GWEN_DB_SetCharValue(dbVars, GWEN_DB_FLAGS_OVERWRITE_VARS, "msgfmt_EXISTS", "TRUE");

  /* add more tools here */


  /* done */
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



int _writeStaticLibHelper(const char *fileName)
{
  int rv;
  static char *staticLibHelperContent=
    "#!/bin/sh\n"
    "\n"
    "TOOL=$1\n"
    "LIB=$2\n"
    "\n"
    "$TOOL -t $LIB | xargs ar rvs $LIB.new && mv -v $LIB.new $LIB";

  unlink(fileName);
  rv=GWEN_SyncIo_Helper_WriteFile(fileName, (const uint8_t*) staticLibHelperContent, strlen(staticLibHelperContent));
  if (rv<0) {
    DBG_ERROR(NULL, "Error writing static lib helper to \"%s\" (%d)", fileName, rv);
    return rv;
  }
  if (chmod(fileName, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP)==-1) {
    DBG_ERROR(NULL, "Error on chmod(%s): %s", fileName, strerror(errno));
    return GWEN_ERROR_GENERIC;
  }

  return 0;
}





