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


#include "gwenbuild/parser/p_buildfiles.h"
#include "gwenbuild/parser/parser.h"
#include "gwenbuild/types/builder_be.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>




static GWB_BUILD_CMD *_parseBuildCmd(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);
static GWB_BUILD_SUBCMD *_parseSubCmd(GWB_BUILD_CMD *bcmd, GWB_CONTEXT *currentContext,
                                      GWEN_XMLNODE *xmlNode, GWEN_DB_NODE *dbForCmd);
static GWEN_DB_NODE *_prepareCmdDb(GWB_BUILD_CMD *bcmd, GWB_CONTEXT *currentContext);
static void _parseAndSetBuildMessage(GWB_BUILD_SUBCMD *buildSubCmd, GWEN_XMLNODE *xmlNode, GWEN_DB_NODE *dbForCmd);
static int _parseBuildInputFiles(GWB_BUILD_CMD *bcmd, GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);
static int _parseBuildOutputFiles(GWB_BUILD_CMD *bcmd, GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);
static GWB_BUILD_SUBCMD *_parseBuildCommand(GWB_BUILD_CMD *bcmd,
                                            GWB_CONTEXT *currentContext,
                                            GWEN_XMLNODE *xmlNode,
                                            GWEN_DB_NODE *dbForCmd);

static int _readFileList(GWEN_XMLNODE *xmlNode,
                         GWEN_DB_NODE *dbVars,
                         const char *sCurrentRelativeDir,
                         GWB_FILE_LIST2 *projectFileList,
                         GWB_FILE_LIST2 *targetFileList,
                         uint32_t flagsToAdd);
static void _addFilePathsToDb(GWB_CONTEXT *currentContext,
                              const GWB_FILE_LIST2 *fileList,
                              GWEN_DB_NODE *db,
                              const char *varName);






int GWB_ParseBuildFiles(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  GWB_TARGET *target;
  GWB_BUILD_CMD *bcmd;
  GWB_BUILD_SUBCMD *buildSubCmd;
  GWEN_DB_NODE *dbForCmd;

  target=GWB_Context_GetCurrentTarget(currentContext);

  bcmd=_parseBuildCmd(project, currentContext, xmlNode);
  if (bcmd==NULL) {
    DBG_INFO(NULL, "here");
    return GWEN_ERROR_GENERIC;
  }

  dbForCmd=_prepareCmdDb(bcmd, currentContext);
  if (dbForCmd==NULL) {
    DBG_INFO(NULL, "here");
    GWB_BuildCmd_free(bcmd);
    return GWEN_ERROR_GENERIC;
  }

  buildSubCmd=_parseSubCmd(bcmd, currentContext, xmlNode, dbForCmd);
  if (buildSubCmd==NULL) {
    DBG_INFO(NULL, "here");
    GWEN_DB_Group_free(dbForCmd);
    GWB_BuildCmd_free(bcmd);
    return GWEN_ERROR_GENERIC;
  }
  _parseAndSetBuildMessage(buildSubCmd, xmlNode, dbForCmd);

  GWB_BuildCmd_AddBuildCommand(bcmd, buildSubCmd);

  if (target)
    GWB_Target_AddExplicitBuild(target, bcmd);
  else
    GWB_Project_AddExplicitBuild(project, bcmd);
  GWEN_DB_Group_free(dbForCmd);

  return 0;
}



GWB_BUILD_CMD *_parseBuildCmd(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  GWEN_XMLNODE *n;
  GWB_BUILD_CMD *bcmd;
  int rv;
  const char *s;

  bcmd=GWB_BuildCmd_new();
  GWB_BuildCmd_SetFolder(bcmd, GWB_Context_GetCurrentRelativeDir(currentContext));

  s=GWEN_XMLNode_GetProperty(xmlNode, "name", NULL);
  if (s && *s)
    GWB_BuildCmd_SetBuilderName(bcmd, s);

  s=GWEN_XMLNode_GetProperty(xmlNode, "auto", "TRUE");
  if (s && *s && 0==strcasecmp(s, "TRUE"))
    GWB_BuildCmd_AddFlags(bcmd, GWB_BUILD_CMD_FLAGS_AUTO);

  n=GWEN_XMLNode_FindFirstTag(xmlNode, "input", NULL, NULL);
  if (n) {
    rv=_parseBuildInputFiles(bcmd, project, currentContext, n);
    if (rv<0) {
      DBG_INFO(NULL, "here (%d)", rv);
      GWB_BuildCmd_free(bcmd);
      return NULL;
    }
  }

  n=GWEN_XMLNode_FindFirstTag(xmlNode, "output", NULL, NULL);
  if (n) {
    rv=_parseBuildOutputFiles(bcmd, project, currentContext, n);
    if (rv<0) {
      DBG_INFO(NULL, "here (%d)", rv);
      GWB_BuildCmd_free(bcmd);
      return NULL;
    }
  }

  return bcmd;
}



GWEN_DB_NODE *_prepareCmdDb(GWB_BUILD_CMD *bcmd, GWB_CONTEXT *currentContext)
{
  GWEN_DB_NODE *dbForCmd;

  dbForCmd=GWEN_DB_Group_new("dbForCmd");
  _addFilePathsToDb(currentContext, GWB_BuildCmd_GetInFileList2(bcmd), dbForCmd, "INPUT");
  _addFilePathsToDb(currentContext, GWB_BuildCmd_GetOutFileList2(bcmd), dbForCmd, "OUTPUT");
  return dbForCmd;
}



GWB_BUILD_SUBCMD *_parseSubCmd(GWB_BUILD_CMD *bcmd, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode, GWEN_DB_NODE *dbForCmd)
{
  GWEN_XMLNODE *n;

  n=GWEN_XMLNode_FindFirstTag(xmlNode, "cmd", NULL, NULL);
  if (n) {
    GWB_BUILD_SUBCMD *buildSubCmd;
    const char *s;

    buildSubCmd=_parseBuildCommand(bcmd, currentContext, n, dbForCmd);
    if (buildSubCmd==NULL) {
      DBG_ERROR(NULL, "here");
      return NULL;
    }

    s=GWEN_XMLNode_GetProperty(n, "deleteOutFileFirst", "FALSE");
    if (s && strcasecmp(s, "TRUE")==0)
      GWB_BuildCmd_AddFlags(bcmd, GWB_BUILD_CMD_FLAGS_DEL_OUTFILES);
    return buildSubCmd;
  }
  return 0;
}



void _parseAndSetBuildMessage(GWB_BUILD_SUBCMD *buildSubCmd, GWEN_XMLNODE *xmlNode, GWEN_DB_NODE *dbForCmd)
{
  GWEN_XMLNODE *n;

  n=GWEN_XMLNode_FindFirstTag(xmlNode, "buildMessage", NULL, NULL);
  if (n) {
    GWEN_BUFFER *dbuf;

    dbuf=GWB_Parser_ReadXmlDataIntoBufferAndExpand(dbForCmd, n);
    if (dbuf) {
      GWB_BuildSubCmd_SetBuildMessage(buildSubCmd, GWEN_Buffer_GetStart(dbuf));
      GWEN_Buffer_free(dbuf);
    }
  }
}



int _parseBuildInputFiles(GWB_BUILD_CMD *bcmd, GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  int rv;

  rv=_readFileList(xmlNode,
                   GWB_Context_GetVars(currentContext),
                   GWB_Context_GetCurrentRelativeDir(currentContext),
                   GWB_Project_GetFileList(project),
                   GWB_BuildCmd_GetInFileList2(bcmd),
                   0);
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int _parseBuildOutputFiles(GWB_BUILD_CMD *bcmd, GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  int rv;

  rv=_readFileList(xmlNode,
                   GWB_Context_GetVars(currentContext),
                   GWB_Context_GetCurrentRelativeDir(currentContext),
                   GWB_Project_GetFileList(project),
                   GWB_BuildCmd_GetOutFileList2(bcmd),
                   GWB_FILE_FLAGS_GENERATED);
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    return rv;
  }

  return 0;
}



GWB_BUILD_SUBCMD *_parseBuildCommand(GWB_BUILD_CMD *bcmd,
                                     GWB_CONTEXT *currentContext,
                                     GWEN_XMLNODE *xmlNode,
                                     GWEN_DB_NODE *dbForCmd)
{
  GWEN_BUFFER *dbuf;
  const char *toolName;
  GWB_BUILD_SUBCMD *buildSubCmd;
  const char *s;
  int rv;

  rv=GWEN_XMLNode_ExpandProperties(xmlNode, GWB_Context_GetVars(currentContext));
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    return NULL;
  }

  toolName=GWEN_XMLNode_GetProperty(xmlNode, "tool", NULL);
  if (!(toolName && *toolName)) {
    DBG_ERROR(NULL, "No tool in <cmd>");
    return NULL;
  }

  dbuf=GWB_Parser_ReadXmlDataIntoBufferAndExpand(dbForCmd, xmlNode);
  buildSubCmd=GWB_BuildSubCmd_new();
  GWB_BuildSubCmd_SetCommand(buildSubCmd, toolName);
  if (dbuf) {
    GWB_BuildSubCmd_SetArguments(buildSubCmd, GWEN_Buffer_GetStart(dbuf));
    GWEN_Buffer_free(dbuf);
  }

  s=GWEN_XMLNode_GetProperty(xmlNode, "checkDates", "TRUE");
  if (s && strcasecmp(s, "TRUE")==0)
    GWB_BuildCmd_AddFlags(bcmd, GWB_BUILD_CMD_FLAGS_CHECK_DATES);

  return buildSubCmd;
}



int _readFileList(GWEN_XMLNODE *xmlNode,
                  GWEN_DB_NODE *dbVars,
                  const char *sCurrentRelativeDir,
                  GWB_FILE_LIST2 *projectFileList,
                  GWB_FILE_LIST2 *targetFileList,
                  uint32_t flagsToAdd)
{
  GWEN_STRINGLIST *sl;

  sl=GWB_Parser_ReadXmlDataIntoStringList(dbVars, xmlNode);
  if (sl) {
    GWBUILD_AddFilesFromStringList(projectFileList,
                                   sCurrentRelativeDir,
                                   sl,
                                   targetFileList,
                                   flagsToAdd,
                                   0);
    GWEN_StringList_free(sl);
  }

  return 0;
}



void _addFilePathsToDb(GWB_CONTEXT *currentContext,
                       const GWB_FILE_LIST2 *fileList,
                       GWEN_DB_NODE *db,
                       const char *varName)
{
  GWB_FILE_LIST2_ITERATOR *it;

  it=GWB_File_List2_First(fileList);
  if (it) {
    const GWB_FILE *file;
    GWEN_BUFFER *fbuf;

    fbuf=GWEN_Buffer_new(0, 256, 0, 1);
    file=GWB_File_List2Iterator_Data(it);
    while(file) {
      const char *folder;
      int useBuildDir;

      useBuildDir=GWB_File_GetFlags(file) & GWB_FILE_FLAGS_GENERATED;
      folder=GWB_File_GetFolder(file);
      GWB_Builder_AddRelativeFolderToBuffer(currentContext, folder, useBuildDir, fbuf);
      if (GWEN_Buffer_GetUsedBytes(fbuf))
        GWEN_Buffer_AppendString(fbuf, GWEN_DIR_SEPARATOR_S);
      GWEN_Buffer_AppendString(fbuf, GWB_File_GetName(file));
      GWEN_DB_SetCharValue(db, 0, varName, GWEN_Buffer_GetStart(fbuf));

      GWEN_Buffer_Reset(fbuf);
      file=GWB_File_List2Iterator_Next(it);
    }
    GWEN_Buffer_free(fbuf);

    GWB_File_List2Iterator_free(it);
  }

}



