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
static int _readFilesFromDataNode(GWEN_XMLNODE *xmlNode,
                                  GWEN_DB_NODE *dbVars,
                                  const char *sCurrentRelativeDir,
                                  GWB_FILE_LIST2 *projectFileList,
                                  GWB_FILE_LIST2 *targetFileList,
                                  uint32_t flagsToAdd);
static int _readFileListFromTagNode(GWEN_XMLNODE *xmlNode,
                                    const GWB_FILE_LIST2 *projectFileList,
                                    GWB_FILE_LIST2 *targetFileList);
static void _addFilePathsToDb(GWB_CONTEXT *currentContext,
                              const GWB_FILE_LIST2 *fileList,
                              GWEN_DB_NODE *db,
                              const char *varName);






int GWB_ParseBuildFiles(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  GWB_TARGET *target;
  GWEN_XMLNODE *n;
  GWB_BUILD_CMD *bcmd;
  GWB_BUILD_SUBCMD *buildSubCmd=NULL;
  GWEN_DB_NODE *dbForCmd;
  int rv;
  const char *s;

  target=GWB_Context_GetCurrentTarget(currentContext);

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
      return rv;
    }
  }

  n=GWEN_XMLNode_FindFirstTag(xmlNode, "output", NULL, NULL);
  if (n) {
    rv=_parseBuildOutputFiles(bcmd, project, currentContext, n);
    if (rv<0) {
      DBG_INFO(NULL, "here (%d)", rv);
      GWB_BuildCmd_free(bcmd);
      return rv;
    }
  }

  dbForCmd=GWEN_DB_Group_new("dbForCmd");
  _addFilePathsToDb(currentContext, GWB_BuildCmd_GetInFileList2(bcmd), dbForCmd, "INPUT");
  _addFilePathsToDb(currentContext, GWB_BuildCmd_GetOutFileList2(bcmd), dbForCmd, "OUTPUT");

  n=GWEN_XMLNode_FindFirstTag(xmlNode, "cmd", NULL, NULL);
  if (n) {
    buildSubCmd=_parseBuildCommand(bcmd, currentContext, n, dbForCmd);
    if (buildSubCmd==NULL) {
      DBG_ERROR(NULL, "here");
      GWEN_DB_Group_free(dbForCmd);
      GWB_BuildCmd_free(bcmd);
      return GWEN_ERROR_GENERIC;
    }

    s=GWEN_XMLNode_GetProperty(n, "deleteOutFileFirst", "FALSE");
    if (s && strcasecmp(s, "TRUE")==0)
      GWB_BuildCmd_AddFlags(bcmd, GWB_BUILD_CMD_FLAGS_DEL_OUTFILES);
    GWB_BuildCmd_AddBuildCommand(bcmd, buildSubCmd);
  }

  /* pepare build message */
  n=GWEN_XMLNode_FindFirstTag(xmlNode, "buildMessage", NULL, NULL);
  if (n) {
    GWEN_BUFFER *dbuf;

    dbuf=GWB_Parser_ReadXmlDataIntoBufferAndExpand(dbForCmd, n);
    if (dbuf) {
      GWB_BuildSubCmd_SetBuildMessage(buildSubCmd, GWEN_Buffer_GetStart(dbuf));
      GWEN_Buffer_free(dbuf);
    }
  }

  if (target)
    GWB_Target_AddExplicitBuild(target, bcmd);
  else
    GWB_Project_AddExplicitBuild(project, bcmd);
  GWEN_DB_Group_free(dbForCmd);

  return 0;
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
  GWEN_XMLNODE *n;

  n=GWEN_XMLNode_GetChild(xmlNode);
  while(n) {
    GWEN_XMLNODE_TYPE tt;
    int rv;

    tt=GWEN_XMLNode_GetType(n);
    if (tt==GWEN_XMLNodeTypeTag)
      rv=_readFileListFromTagNode(n, projectFileList, targetFileList);
    else if (tt==GWEN_XMLNodeTypeData)
      rv=_readFilesFromDataNode(n,
                                dbVars,
                                sCurrentRelativeDir,
                                projectFileList,
                                targetFileList,
                                flagsToAdd);
    else
      rv=0;
    if (rv<0) {
      DBG_INFO(NULL, "here (%d)", rv);
      return rv;
    }

    n=GWEN_XMLNode_Next(n);
  }

  return 0;
}



int _readFilesFromDataNode(GWEN_XMLNODE *xmlNode,
                           GWEN_DB_NODE *dbVars,
                           const char *sCurrentRelativeDir,
                           GWB_FILE_LIST2 *projectFileList,
                           GWB_FILE_LIST2 *targetFileList,
                           uint32_t flagsToAdd)
{
  const char *s;

  s=GWEN_XMLNode_GetData(xmlNode);
  if (s && *s) {
    int rv;
    GWEN_BUFFER *buf;

    buf=GWEN_Buffer_new(0, 256, 0, 1);
    rv=GWEN_DB_ReplaceVars(dbVars, s, buf);
    if(rv<0) {
      DBG_INFO(NULL, "here (%d)", rv);
      GWEN_Buffer_free(buf);
      return rv;
    }
    if (GWEN_Buffer_GetUsedBytes(buf)) {
      GWEN_STRINGLIST *sl;

      sl=GWEN_StringList_fromString(GWEN_Buffer_GetStart(buf), " ", 1);
      if (sl==NULL) {
        DBG_ERROR(NULL, "Could not generate string list from data [%s]", GWEN_Buffer_GetStart(buf));
        GWEN_Buffer_free(buf);
        return GWEN_ERROR_GENERIC;
      }
      GWBUILD_AddFilesFromStringList(projectFileList,
                                     sCurrentRelativeDir,
                                     sl,
                                     targetFileList,
                                     flagsToAdd,
                                     0);
      GWEN_StringList_free(sl);
    }
    GWEN_Buffer_free(buf);
  }
  return 0;
}



int _readFileListFromTagNode(GWEN_XMLNODE *xmlNode,
                             const GWB_FILE_LIST2 *projectFileList,
                             GWB_FILE_LIST2 *targetFileList)
{
  const char *tagName;

  tagName=GWEN_XMLNode_GetData(xmlNode);
  if (tagName && *tagName && strcasecmp(tagName, "files")==0) {
    int index;
    const char *pattern;

    pattern=GWEN_XMLNode_GetProperty(xmlNode, "match", NULL);
    index=GWEN_XMLNode_GetIntProperty(xmlNode, "index", -1);

    if (index>=0) {
      GWB_FILE *file;

      file=GWB_File_List2_GetAt(projectFileList, index);
      if (file)
        GWB_File_List2_PushBack(targetFileList, file);
    }
    else {
      GWB_FILE_LIST2_ITERATOR *it;

      it=GWB_File_List2_First(projectFileList);
      if (it) {
        GWB_FILE *file;

        file=GWB_File_List2Iterator_Data(it);
        while(file) {
          const char *fname;

          fname=GWB_File_GetName(file);
          if (fname) {
            if (pattern==NULL || -1!=GWEN_Text_ComparePattern(fname, pattern, 0))
              GWB_File_List2_PushBack(targetFileList, file);
          }
          file=GWB_File_List2Iterator_Next(it);
        }

        GWB_File_List2Iterator_free(it);
      }
    }
    return 0;
  }
  else {
    DBG_ERROR(NULL, "Unknown tag name \"%s\"", tagName);
    return GWEN_ERROR_GENERIC;
  }
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



