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
#include "gwenbuild/parser/p_project.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/syncio.h>
#include <gwenhywfar/process.h>

#include <unistd.h>
#include <stdlib.h>



static void _copySomeEnvironmentVariablesToDb(GWEN_DB_NODE *db);
static void _copyEnvironmentVariableToDb(GWEN_DB_NODE *db, const char *envName, const char *dbVarName);
static int _parseSubdir(GWB_PROJECT *project, GWB_CONTEXT *currentContext, const char *sFolder, GWB_PARSER_PARSE_ELEMENT_FN fn);
static void _appendVarValue(GWEN_DB_NODE *db, const char *name, const char *newValue);




GWB_PROJECT *GWB_Parser_ReadBuildTree(GWENBUILD *gwbuild, const char *srcDir)
{
  GWB_CONTEXT *currentContext;
  GWEN_XMLNODE *xmlNewFile;
  GWEN_XMLNODE *xmlProject;
  GWB_PROJECT *project;
  int rv;

  currentContext=GWB_Context_new();
  GWB_Context_SetTopSourceDir(currentContext, srcDir);
  GWB_Context_SetCurrentSourceDir(currentContext, srcDir);
  _copySomeEnvironmentVariablesToDb(GWB_Context_GetVars(currentContext));
  GWEN_DB_SetCharValue(GWB_Context_GetVars(currentContext), GWEN_DB_FLAGS_OVERWRITE_VARS, "prefix", "/usr/local");


  xmlNewFile=GWB_Parser_ReadBuildFile(currentContext, GWB_PARSER_FILENAME);
  if (xmlNewFile==NULL) {
    DBG_ERROR(NULL, "No valid node found in build file of folder \"%s\"", srcDir);
    GWB_Context_free(currentContext);
    return NULL;
  }

  xmlProject=GWEN_XMLNode_FindFirstTag(xmlNewFile, "project", NULL, NULL);
  if (xmlProject==NULL) {
    DBG_ERROR(NULL, "No project node found in build file of folder \"%s\"", srcDir);
    GWB_Context_free(currentContext);
    GWEN_XMLNode_free(xmlNewFile);
    return NULL;
  }

  project=GWB_Project_new(gwbuild, currentContext);

  rv=GWB_ParseProject(project, currentContext, xmlProject);
  if (rv<0) {
    DBG_ERROR(NULL, "Error parsing build tree in folder \"%s\"", srcDir);
    GWB_Project_free(project);
    GWEN_XMLNode_free(xmlNewFile);
    return NULL;
  }

  GWEN_XMLNode_free(xmlNewFile);
  return project;
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



GWB_CONTEXT *GWB_Parser_CopyContextForSubdir(const GWB_CONTEXT *sourceContext, const char *folder)
{
  GWB_CONTEXT *newContext;
  GWEN_DB_NODE *db;

  newContext=GWB_Context_dup(sourceContext);

  GWB_Context_AddCurrentRelativeDir(newContext, folder);

  GWB_Context_InsertTopBuildDir(newContext, "..");
  GWB_Context_InsertTopSourceDir(newContext, "..");

  GWB_Context_AddCurrentBuildDir(newContext, folder);
  GWB_Context_AddCurrentSourceDir(newContext, folder);

  GWB_Context_ClearSourceFileList2(newContext);
  GWB_Context_ClearIncludeList(newContext);
  GWB_Context_ClearDefineList(newContext);

  db=GWB_Context_GetVars(newContext);
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "topbuilddir", GWB_Context_GetTopBuildDir(newContext));
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "topsrcdir", GWB_Context_GetTopSourceDir(newContext));
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "builddir", GWB_Context_GetCurrentBuildDir(newContext));
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "srcdir", GWB_Context_GetCurrentSourceDir(newContext));

  return newContext;
}



GWB_CONTEXT *GWB_Parser_CopyContextForTarget(const GWB_CONTEXT *sourceContext)
{
  GWB_CONTEXT *newContext;

  newContext=GWB_Context_dup(sourceContext);

  GWB_Context_ClearSourceFileList2(newContext);
  GWB_Context_ClearIncludeList(newContext);
  GWB_Context_ClearDefineList(newContext);

  return newContext;
}



GWEN_XMLNODE *GWB_Parser_ReadBuildFile(const GWB_CONTEXT *currentContext, const char *fileName)
{
  GWEN_BUFFER *fileNameBuf;
  const char *s;
  int rv;
  GWEN_XMLNODE *xmlDocNode;
  GWEN_XMLNODE *xmlGwbuildNode;

  fileNameBuf=GWEN_Buffer_new(0, 256, 0, 1);
  s=GWB_Context_GetCurrentSourceDir(currentContext);
  if (s && *s) {
    GWEN_Buffer_AppendString(fileNameBuf, s);
    GWEN_Buffer_AppendString(fileNameBuf, GWEN_DIR_SEPARATOR_S);
  }
  GWEN_Buffer_AppendString(fileNameBuf, fileName);

  xmlDocNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");

  DBG_ERROR(NULL, "Reading file %s", GWEN_Buffer_GetStart(fileNameBuf));
  rv=GWEN_XML_ReadFile(xmlDocNode, GWEN_Buffer_GetStart(fileNameBuf), GWEN_XML_FLAGS_DEFAULT);
  if (rv<0) {
    DBG_INFO(NULL, "here %d)", rv);
    GWEN_Buffer_free(fileNameBuf);
    return NULL;
  }

  xmlGwbuildNode=GWEN_XMLNode_FindFirstTag(xmlDocNode, "gwbuild", NULL, NULL);
  if (xmlGwbuildNode==NULL) {
    DBG_ERROR(NULL, "File \"%s\" does not contain an \"gwbuild\" element", GWEN_Buffer_GetStart(fileNameBuf));
    GWEN_XMLNode_free(xmlDocNode);
    GWEN_Buffer_free(fileNameBuf);
    return NULL;
  }

  GWEN_XMLNode_UnlinkChild(xmlDocNode, xmlGwbuildNode);
  GWEN_XMLNode_free(xmlDocNode);
  GWEN_Buffer_free(fileNameBuf);

  return xmlGwbuildNode;
}



GWEN_STRINGLIST *GWB_Parser_ReadXmlDataIntoStringList(const GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  GWEN_BUFFER *buf;

  buf=GWB_Parser_ReadXmlDataIntoBufferAndExpand(currentContext, xmlNode);
  if (buf) {
    GWEN_STRINGLIST *sl;

    sl=GWEN_StringList_fromString(GWEN_Buffer_GetStart(buf), " ", 1);
    if (sl==NULL) {
      DBG_ERROR(NULL, "Could not generate string list from data [%s]", GWEN_Buffer_GetStart(buf));
      GWEN_Buffer_free(buf);
      return NULL;
    }
    GWEN_Buffer_free(buf);

    return sl;
  }

  return NULL;
}



GWEN_BUFFER *GWB_Parser_ReadXmlDataIntoBufferAndExpand(const GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  GWEN_XMLNODE *xmlData;

  xmlData=GWEN_XMLNode_GetFirstData(xmlNode);
  if (xmlData) {
    const char *s;

    s=GWEN_XMLNode_GetData(xmlData);
    if (s && *s) {
      int rv;
      GWEN_BUFFER *buf;

      buf=GWEN_Buffer_new(0, 256, 0, 1);
      rv=GWEN_DB_ReplaceVars(GWB_Context_GetVars(currentContext), s, buf);
      if(rv<0) {
        DBG_INFO(NULL, "here (%d)", rv);
        GWEN_Buffer_free(buf);
        return NULL;
      }

      return buf;
    }
  }

  return NULL;
}



int GWB_Parser_ParseSubdirs(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode, GWB_PARSER_PARSE_ELEMENT_FN fn)
{
  GWEN_STRINGLIST *sl;

  sl=GWB_Parser_ReadXmlDataIntoStringList(currentContext, xmlNode);
  if (sl) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(sl);
    while(se) {
      const char *sFolder;

      sFolder=GWEN_StringListEntry_Data(se);
      if (sFolder && *sFolder) {
        int rv;

        rv=_parseSubdir(project, currentContext, sFolder, fn);
        if (rv<0) {
          DBG_INFO(NULL, "here (%d)", rv);
          return rv;
        }
      }
      se=GWEN_StringListEntry_Next(se);
    }
    GWEN_StringList_free(sl);
  }

  return 0;
}



int _parseSubdir(GWB_PROJECT *project, GWB_CONTEXT *currentContext, const char *sFolder, GWB_PARSER_PARSE_ELEMENT_FN fn)
{

  GWB_CONTEXT *newContext;
  GWEN_XMLNODE *xmlNewFile;
  int rv;

  newContext=GWB_Parser_CopyContextForSubdir(currentContext, sFolder);

  xmlNewFile=GWB_Parser_ReadBuildFile(newContext, GWB_PARSER_FILENAME);
  if (xmlNewFile==NULL) {
    DBG_ERROR(NULL, "No valid node found in build file of subdir \"%s\"", sFolder);
    GWB_Context_free(newContext);
    return GWEN_ERROR_BAD_DATA;
  }

  rv=fn(project, newContext, xmlNewFile);
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    GWEN_XMLNode_free(xmlNewFile);
    GWB_Context_free(newContext);
    return rv;
  }
  GWEN_XMLNode_free(xmlNewFile);

  GWB_Context_Tree2_AddChild(currentContext, newContext);
  return 0;
}



int GWB_Parser_ParseSetVar(GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  const char *sName;
  const char *sMode;
  GWEN_BUFFER *buf;
  GWEN_DB_NODE *db;

  db=GWB_Context_GetVars(currentContext);

  sName=GWEN_XMLNode_GetProperty(xmlNode, "name", NULL);
  if (!(sName && *sName)) {
    DBG_ERROR(NULL, "No name for <setVar>");
    return GWEN_ERROR_GENERIC;
  }

  sMode=GWEN_XMLNode_GetProperty(xmlNode, "mode", "replace");

  buf=GWB_Parser_ReadXmlDataIntoBufferAndExpand(currentContext, xmlNode);
  if (buf && GWEN_Buffer_GetUsedBytes(buf)) {
    if (strcasecmp(sMode, "replace")==0)
      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, sName, GWEN_Buffer_GetStart(buf));
    else if (strcasecmp(sMode, "add")==0)
      GWEN_DB_SetCharValue(db, 0, sName, GWEN_Buffer_GetStart(buf));
    else if (strcasecmp(sMode, "append")==0)
      _appendVarValue(db, sName, GWEN_Buffer_GetStart(buf));
    GWEN_Buffer_free(buf);
  }

  return 0;
}



void _appendVarValue(GWEN_DB_NODE *db, const char *name, const char *newValue)
{
  const char *s;

  s=GWEN_DB_GetCharValue(db, name, 0, NULL);
  if (s && *s) {
    GWEN_BUFFER *buf;

    buf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(buf, s);
    GWEN_Buffer_AppendString(buf, " ");
    GWEN_Buffer_AppendString(buf, newValue);
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, name, GWEN_Buffer_GetStart(buf));
    GWEN_Buffer_free(buf);
  }
  else
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, name, newValue);
}



void GWB_Parser_SetItemValue(GWEN_DB_NODE *db, const char *sId, const char *suffix, const char *value)
{
  GWEN_BUFFER *varNameBuffer;

  varNameBuffer=GWEN_Buffer_new(0, 64, 0, 1);
  GWEN_Buffer_AppendString(varNameBuffer, sId);
  GWEN_Buffer_AppendString(varNameBuffer, suffix);
  GWEN_DB_SetCharValue(db,
                       GWEN_DB_FLAGS_OVERWRITE_VARS,
                       GWEN_Buffer_GetStart(varNameBuffer),
                       value);
  GWEN_Buffer_free(varNameBuffer);
}



