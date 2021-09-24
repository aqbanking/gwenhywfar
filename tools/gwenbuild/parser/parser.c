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
#include "gwenbuild/utils.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/syncio.h>
#include <gwenhywfar/process.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/text.h>

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include <sys/types.h> /* for stat, chmod */
#include <sys/stat.h>
#include <unistd.h>




static int _parseSubdir(GWB_PROJECT *project, GWB_CONTEXT *currentContext, const char *sFolder, GWB_PARSER_PARSE_ELEMENT_FN fn);
static int _parseSetVar(GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);
static int _parseIfVarMatches(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *n, GWB_PARSER_PARSE_ELEMENT_FN fn);
static int _parseIfNotVarMatches(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *n, GWB_PARSER_PARSE_ELEMENT_FN fn);
static int _parseIfVarHasValue(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *n, GWB_PARSER_PARSE_ELEMENT_FN fn);
static int _parseIfNotVarHasValue(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *n, GWB_PARSER_PARSE_ELEMENT_FN fn);
static int _varHasValue(GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);

static int _parseWriteFile(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);
static void _appendVarValue(GWEN_DB_NODE *db, const char *name, const char *newValue);

static int _getFilePermissions(const char *fname);
static int _setFilePermissions(const char *fname, int perms);




GWB_PROJECT *GWB_Parser_ReadBuildTree(GWENBUILD *gwbuild,
                                      GWB_CONTEXT *currentContext,
                                      const char *srcDir,
                                      GWB_KEYVALUEPAIR_LIST *givenOptionList)
{
  GWEN_XMLNODE *xmlNewFile;
  GWEN_XMLNODE *xmlProject;
  GWB_PROJECT *project;
  GWB_FILE *file;
  GWEN_DB_NODE *db;
  int rv;

  GWB_Context_SetInitialSourceDir(currentContext, srcDir);
  GWB_Context_SetTopSourceDir(currentContext, srcDir);
  GWB_Context_SetCurrentSourceDir(currentContext, srcDir);

  db=GWB_Context_GetVars(currentContext);
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "topbuilddir", ".");
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "topsrcdir", srcDir);
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "builddir", ".");
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "srcdir", srcDir);

  xmlNewFile=GWB_Parser_ReadBuildFile(gwbuild, currentContext, GWB_PARSER_FILENAME);
  if (xmlNewFile==NULL) {
    DBG_ERROR(NULL, "No valid node found in build file of folder \"%s\"", srcDir);
    return NULL;
  }

  xmlProject=GWEN_XMLNode_FindFirstTag(xmlNewFile, "project", NULL, NULL);
  if (xmlProject==NULL) {
    DBG_ERROR(NULL, "No project node found in build file of folder \"%s\"", srcDir);
    GWEN_XMLNode_free(xmlNewFile);
    return NULL;
  }

  project=GWB_Project_new(gwbuild, currentContext);
  file=GWB_File_List2_GetOrCreateFile(GWB_Project_GetFileList(project),
                                      GWB_Context_GetCurrentRelativeDir(currentContext),
                                      GWB_PARSER_FILENAME);
  GWB_File_AddFlags(file, GWB_FILE_FLAGS_DIST);

  if (givenOptionList)
    GWB_Project_SetGivenOptionList(project, givenOptionList);

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



GWB_CONTEXT *GWB_Parser_CopyContextForSubdir(const GWB_CONTEXT *sourceContext, const char *folder)
{
  GWB_CONTEXT *newContext;
  GWEN_DB_NODE *db;

  newContext=GWB_Context_dup(sourceContext);

  GWB_Context_AddCurrentRelativeDir(newContext, folder);

  GWB_Context_InsertTopBuildDir(newContext, "..");
  GWB_Context_InsertTopSourceDir(newContext, "..");

  GWB_Context_AddCurrentBuildDir(newContext, folder);

  GWB_Context_InsertCurrentSourceDir(newContext, "..");
  GWB_Context_AddCurrentSourceDir(newContext, folder);

  GWB_Context_ClearSourceFileList2(newContext);
  GWB_Context_ClearIncludeList(newContext);
  GWB_Context_ClearDefineList(newContext);

  db=GWB_Context_GetVars(newContext);
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "topbuilddir", GWB_Context_GetTopBuildDir(newContext));
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "topsrcdir", GWB_Context_GetTopSourceDir(newContext));
  //GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "builddir", GWB_Context_GetCurrentBuildDir(newContext));
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "builddir", ".");
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "srcdir", GWB_Context_GetCurrentSourceDir(newContext));

  GWEN_DB_DeleteGroup(db, "local");

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



GWEN_XMLNODE *GWB_Parser_ReadBuildFile(GWENBUILD *gwbuild, const GWB_CONTEXT *currentContext, const char *fileName)
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

  DBG_DEBUG(NULL, "Reading file %s", GWEN_Buffer_GetStart(fileNameBuf));
  rv=GWEN_XML_ReadFile(xmlDocNode, GWEN_Buffer_GetStart(fileNameBuf), GWEN_XML_FLAGS_DEFAULT);
  if (rv<0) {
    DBG_ERROR(NULL, "Error in XML file \"%s\" (%d)", GWEN_Buffer_GetStart(fileNameBuf), rv);
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
  s=GWEN_XMLNode_GetProperty(xmlGwbuildNode, "requiredVersion", NULL);
  if (s && *s) {
    int vRequired;
    int vCurrent;

    vCurrent=(int) ((GWENHYWFAR_VERSION_MAJOR<<24)+
		    (GWENHYWFAR_VERSION_MINOR<<16)+
                    (GWENHYWFAR_VERSION_PATCHLEVEL<<8)+
                    (GWENHYWFAR_VERSION_BUILD));

    vRequired=GWB_Utils_VersionStringToInt(s);
    if (vRequired<0) {
      DBG_ERROR(NULL, "Invalid required version \"%s\"", s);
      GWEN_XMLNode_free(xmlDocNode);
      GWEN_Buffer_free(fileNameBuf);
      return NULL;
    }
    if (vCurrent<vRequired) {
      DBG_ERROR(NULL, "Minimum GWENBUILD version required is %x", vRequired);
      GWEN_XMLNode_free(xmlDocNode);
      GWEN_Buffer_free(fileNameBuf);
      return NULL;
    }
  }

  if (1) {
    GWEN_BUFFER *buildFilenameBuffer;
    const char *buildDir;
    const char *initialSourceDir;

    initialSourceDir=GWB_Context_GetInitialSourceDir(currentContext);
    buildDir=GWB_Context_GetCurrentBuildDir(currentContext);
    buildFilenameBuffer=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(buildFilenameBuffer, initialSourceDir);
    if (buildDir) {
      GWEN_Buffer_AppendString(buildFilenameBuffer, GWEN_DIR_SEPARATOR_S);
      GWEN_Buffer_AppendString(buildFilenameBuffer, buildDir);
    }
    GWEN_Buffer_AppendString(buildFilenameBuffer, GWEN_DIR_SEPARATOR_S);
    GWEN_Buffer_AppendString(buildFilenameBuffer, fileName);
    GWBUILD_AddBuildFilename(gwbuild, GWEN_Buffer_GetStart(buildFilenameBuffer));
    GWEN_Buffer_free(buildFilenameBuffer);
  }

  GWEN_XMLNode_UnlinkChild(xmlDocNode, xmlGwbuildNode);
  GWEN_XMLNode_free(xmlDocNode);
  GWEN_Buffer_free(fileNameBuf);

  return xmlGwbuildNode;
}



GWEN_STRINGLIST *GWB_Parser_ReadXmlDataIntoStringList(GWEN_DB_NODE *db, GWEN_XMLNODE *xmlNode)
{
  GWEN_BUFFER *buf;

  buf=GWB_Parser_ReadXmlDataIntoBufferAndExpand(db, xmlNode);
  if (buf) {
    GWEN_STRINGLIST *sl;

    sl=GWEN_StringList_fromString(GWEN_Buffer_GetStart(buf), " ", 1);
    if (sl==NULL) {
      DBG_INFO(NULL, "Could not generate string list from data [%s]", GWEN_Buffer_GetStart(buf));
      GWEN_Buffer_free(buf);
      return NULL;
    }
    GWEN_Buffer_free(buf);

    return sl;
  }

  return NULL;
}



GWEN_BUFFER *GWB_Parser_ReadXmlDataIntoBufferAndExpand(GWEN_DB_NODE *db, GWEN_XMLNODE *xmlNode)
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
      if (db) {
	rv=GWEN_DB_ReplaceVars(db, s, buf);
	if(rv<0) {
	  DBG_INFO(NULL, "here (%d)", rv);
	  GWEN_Buffer_free(buf);
	  return NULL;
	}
      }
      else
	GWEN_Buffer_AppendString(buf, s);

      if (GWEN_Buffer_GetUsedBytes(buf)==0) {
        GWEN_Buffer_free(buf);
        return NULL;
      }
      return buf;
    }
  }

  return NULL;
}



GWEN_BUFFER *GWB_Parser_ReadNamedXmlDataIntoBufferAndExpand(GWEN_DB_NODE *db, GWEN_XMLNODE *xmlNode, const char *elem)
{

  GWEN_XMLNODE *n;
  
  n=GWEN_XMLNode_FindFirstTag(xmlNode, elem, NULL, NULL);
  if (n) {
    GWEN_BUFFER *buf;

    buf=GWB_Parser_ReadXmlDataIntoBufferAndExpand(db, n);
    if (buf) {
      if (GWEN_Buffer_GetUsedBytes(buf))
        return buf;
      GWEN_Buffer_free(buf);
    }
  }

  return NULL;
}



int GWB_Parser_ParseSubdirs(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode, GWB_PARSER_PARSE_ELEMENT_FN fn)
{
  GWEN_STRINGLIST *sl;

  sl=GWB_Parser_ReadXmlDataIntoStringList(GWB_Context_GetVars(currentContext), xmlNode);
  if (sl) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(sl);
    while(se) {
      const char *sFolder;

      sFolder=GWEN_StringListEntry_Data(se);
      if (sFolder && *sFolder) {
        int rv;

        rv=GWEN_Directory_GetPath(sFolder, 0);
        if (rv<0) {
          DBG_ERROR(NULL, "Could not create folder \"%s\" in build tree", sFolder);
          return rv;
        }
        if (chdir(sFolder)==-1) {
          DBG_ERROR(NULL, "Could not enter folder \"%s\" in build tree (%s)",
                    sFolder, strerror(errno));
          return GWEN_ERROR_GENERIC;
        }
        rv=_parseSubdir(project, currentContext, sFolder, fn);
        chdir("..");
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
  GWB_FILE *file;

  newContext=GWB_Parser_CopyContextForSubdir(currentContext, sFolder);

  fprintf(stdout, "Entering folder \"%s\"\n", sFolder);
  xmlNewFile=GWB_Parser_ReadBuildFile(GWB_Project_GetGwbuild(project), newContext, GWB_PARSER_FILENAME);
  if (xmlNewFile==NULL) {
    DBG_ERROR(NULL, "No valid node found in build file of subdir \"%s\"", sFolder);
    GWB_Context_free(newContext);
    return GWEN_ERROR_BAD_DATA;
  }

  file=GWB_File_List2_GetOrCreateFile(GWB_Project_GetFileList(project),
                                      GWB_Context_GetCurrentRelativeDir(newContext),
                                      GWB_PARSER_FILENAME);
  GWB_File_AddFlags(file, GWB_FILE_FLAGS_DIST);


  rv=fn(project, newContext, xmlNewFile);
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    GWEN_XMLNode_free(xmlNewFile);
    GWB_Context_free(newContext);
    return rv;
  }
  GWEN_XMLNode_free(xmlNewFile);

  GWB_Context_Tree2_AddChild(currentContext, newContext);
  fprintf(stdout, "Leaving folder  \"%s\"\n", sFolder);
  return 0;
}



int _parseSetVar(GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
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

  if (GWEN_XMLNode_GetProperty(xmlNode, "value", NULL)) {
    DBG_WARN(NULL,
	     "%s: Please dont use property \"value\", just store the value between <setVar> and </setVar>",
	     sName);
  }

  sMode=GWEN_XMLNode_GetProperty(xmlNode, "mode", "replace");

  buf=GWB_Parser_ReadXmlDataIntoBufferAndExpand(GWB_Context_GetVars(currentContext), xmlNode);
  if (buf && GWEN_Buffer_GetUsedBytes(buf)) {
    if (strcasecmp(sMode, "replace")==0)
      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, sName, GWEN_Buffer_GetStart(buf));
    else if (strcasecmp(sMode, "add")==0)
      GWEN_DB_SetCharValue(db, 0, sName, GWEN_Buffer_GetStart(buf));
    else if (strcasecmp(sMode, "append")==0)
      _appendVarValue(db, sName, GWEN_Buffer_GetStart(buf));
    GWEN_Buffer_free(buf);
  }
  else {
    if (strcasecmp(sMode, "replace")==0) {
      GWEN_DB_DeleteVar(db, sName);
    }
  }

  return 0;
}



int _parseIfVarMatches(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode, GWB_PARSER_PARSE_ELEMENT_FN fn)
{
  const char *sName;
  const char *sPattern;
  const char *sValueInDb;
  GWEN_DB_NODE *db;
  GWEN_XMLNODE *xmlThen;
  GWEN_XMLNODE *xmlElse;
  int rv;

  xmlThen=GWEN_XMLNode_FindFirstTag(xmlNode, "then", NULL, NULL);
  xmlElse=GWEN_XMLNode_FindFirstTag(xmlNode, "else", NULL, NULL);

  db=GWB_Context_GetVars(currentContext);

  sName=GWEN_XMLNode_GetProperty(xmlNode, "name", NULL);
  if (!(sName && *sName)) {
    DBG_ERROR(NULL, "No name for <ifVarMatches>");
    return GWEN_ERROR_GENERIC;
  }

  sPattern=GWEN_XMLNode_GetProperty(xmlNode, "value", NULL);
  if (!(sPattern && *sPattern)) {
    DBG_ERROR(NULL, "No value for <ifVarMatches>");
    return GWEN_ERROR_GENERIC;
  }

  sValueInDb=GWEN_DB_GetCharValue(db, sName, 0, NULL);
  if (!(sValueInDb && *sValueInDb)) {
    DBG_ERROR(NULL, "No value for \"%s\" in db for <ifVarMatches>", sName);
    return GWEN_ERROR_GENERIC;
  }

  if (-1!=GWEN_Text_ComparePattern(sValueInDb, sPattern, 0)) {
    rv=fn(project, currentContext, xmlThen?xmlThen:xmlNode);
    if (rv<0) {
      DBG_INFO(NULL, "here (%d)", rv);
      return rv;
    }
  }
  else {
    if (xmlElse) {
      rv=fn(project, currentContext, xmlElse);
      if (rv<0) {
        DBG_INFO(NULL, "here (%d)", rv);
        return rv;
      }
    }
  }

  return 0;
}



int _parseIfNotVarMatches(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode, GWB_PARSER_PARSE_ELEMENT_FN fn)
{
  const char *sName;
  const char *sPattern;
  const char *sValueInDb;
  GWEN_DB_NODE *db;
  GWEN_XMLNODE *xmlThen;
  GWEN_XMLNODE *xmlElse;
  int rv;

  xmlThen=GWEN_XMLNode_FindFirstTag(xmlNode, "then", NULL, NULL);
  xmlElse=GWEN_XMLNode_FindFirstTag(xmlNode, "else", NULL, NULL);

  db=GWB_Context_GetVars(currentContext);

  sName=GWEN_XMLNode_GetProperty(xmlNode, "name", NULL);
  if (!(sName && *sName)) {
    DBG_ERROR(NULL, "No name for <ifVarMatches>");
    return GWEN_ERROR_GENERIC;
  }

  sPattern=GWEN_XMLNode_GetProperty(xmlNode, "value", NULL);
  if (!(sPattern && *sPattern)) {
    DBG_ERROR(NULL, "No value for <ifVarMatches>");
    return GWEN_ERROR_GENERIC;
  }

  sValueInDb=GWEN_DB_GetCharValue(db, sName, 0, NULL);
  if (!(sValueInDb && *sValueInDb)) {
    DBG_ERROR(NULL, "No value for \"%s\" in db for <ifVarMatches>", sName);
    return GWEN_ERROR_GENERIC;
  }

  if (-1==GWEN_Text_ComparePattern(sValueInDb, sPattern, 0)) {
    rv=fn(project, currentContext, xmlThen?xmlThen:xmlNode);
    if (rv<0) {
      DBG_INFO(NULL, "here (%d)", rv);
      return rv;
    }
  }
  else {
    if (xmlElse) {
      rv=fn(project, currentContext, xmlElse);
      if (rv<0) {
        DBG_INFO(NULL, "here (%d)", rv);
        return rv;
      }
    }
  }

  return 0;
}



int _parseIfVarHasValue(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode, GWB_PARSER_PARSE_ELEMENT_FN fn)
{
  int rv;
  GWEN_XMLNODE *xmlThen;
  GWEN_XMLNODE *xmlElse;

  xmlThen=GWEN_XMLNode_FindFirstTag(xmlNode, "then", NULL, NULL);
  xmlElse=GWEN_XMLNode_FindFirstTag(xmlNode, "else", NULL, NULL);
  rv=_varHasValue(currentContext, xmlNode);
  if (rv<0)
    return rv;
  if (rv) {
    return fn(project, currentContext, xmlThen?xmlThen:xmlNode);
  }
  else {
    if (xmlElse)
      return fn(project, currentContext, xmlElse);
  }
  return 0;
}



int _parseIfNotVarHasValue(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode, GWB_PARSER_PARSE_ELEMENT_FN fn)
{
  int rv;
  GWEN_XMLNODE *xmlThen;
  GWEN_XMLNODE *xmlElse;

  xmlThen=GWEN_XMLNode_FindFirstTag(xmlNode, "then", NULL, NULL);
  xmlElse=GWEN_XMLNode_FindFirstTag(xmlNode, "else", NULL, NULL);
  rv=_varHasValue(currentContext, xmlNode);
  if (rv<0)
    return rv;
  if (rv==0) {
    return fn(project, currentContext, xmlThen?xmlThen:xmlNode);
  }
  else {
    if (xmlElse)
      return fn(project, currentContext, xmlElse);
  }
  return 0;
}



int _varHasValue(GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  const char *sName;
  const char *sPattern;
  GWEN_DB_NODE *db;
  int i;

  db=GWB_Context_GetVars(currentContext);

  sName=GWEN_XMLNode_GetProperty(xmlNode, "name", NULL);
  if (!(sName && *sName)) {
    DBG_ERROR(NULL, "No name for <ifVarMatches>");
    return GWEN_ERROR_GENERIC;
  }

  sPattern=GWEN_XMLNode_GetProperty(xmlNode, "value", NULL);
  if (!(sPattern && *sPattern)) {
    DBG_ERROR(NULL, "No value for <ifVarMatches>");
    return GWEN_ERROR_GENERIC;
  }

  for (i=0; i<100; i++) {
    const char *sValueInDb;

    sValueInDb=GWEN_DB_GetCharValue(db, sName, i, NULL);
    if (!sValueInDb)
      break;
    if (-1!=GWEN_Text_ComparePattern(sValueInDb, sPattern, 0))
      return 1;
  }

  return 0;
}



int _parseWriteFile(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  const char *fileName;
  const char *currentSrcDir;
  GWEN_BUFFER *fileNameBuffer;
  GWEN_BUFFER *fileBufferIn;
  GWEN_BUFFER *fileBufferOut;
  GWB_FILE *file;
  int sourceFilePerms;
  int rv;

  fileName=GWEN_XMLNode_GetProperty(xmlNode, "name", NULL);
  if (!(fileName && *fileName)) {
    DBG_ERROR(NULL, "No name for <writeFile>");
    return GWEN_ERROR_GENERIC;
  }

  currentSrcDir=GWB_Context_GetCurrentSourceDir(currentContext);

  fileNameBuffer=GWEN_Buffer_new(0, 256, 0, 1);
  if (currentSrcDir && *currentSrcDir) {
    GWEN_Buffer_AppendString(fileNameBuffer, currentSrcDir);
    GWEN_Buffer_AppendString(fileNameBuffer, GWEN_DIR_SEPARATOR_S);
  }
  GWEN_Buffer_AppendString(fileNameBuffer, fileName);
  GWEN_Buffer_AppendString(fileNameBuffer, ".in");

  rv=_getFilePermissions(GWEN_Buffer_GetStart(fileNameBuffer));
  if (rv<0) {
    DBG_ERROR(NULL, "Could not read permissions for \"%s\" (%d)", GWEN_Buffer_GetStart(fileNameBuffer), rv);
    GWEN_Buffer_free(fileNameBuffer);
    return rv;
  }
  sourceFilePerms=rv;

  fileBufferIn=GWEN_Buffer_new(0, 256, 0, 1);

  rv=GWEN_SyncIo_Helper_ReadFile(GWEN_Buffer_GetStart(fileNameBuffer), fileBufferIn);
  if (rv<0) {
    DBG_ERROR(NULL, "Could not read \"%s\" (%d)", GWEN_Buffer_GetStart(fileNameBuffer), rv);
    GWEN_Buffer_free(fileBufferIn);
    GWEN_Buffer_free(fileNameBuffer);
    return rv;
  }

  fileBufferOut=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWB_Parser_ReplaceVarsBetweenAtSigns(GWEN_Buffer_GetStart(fileBufferIn),
                                          fileBufferOut,
                                          GWB_Context_GetVars(currentContext));
  if (rv<0) {
    DBG_ERROR(NULL, "Error translating content of file \"%s\" (%d)", GWEN_Buffer_GetStart(fileNameBuffer), rv);
    GWEN_Buffer_free(fileBufferOut);
    GWEN_Buffer_free(fileBufferIn);
    GWEN_Buffer_free(fileNameBuffer);
    return rv;
  }

  unlink(fileName);
  rv=GWEN_SyncIo_Helper_WriteFile(fileName,
                                  (const uint8_t*)GWEN_Buffer_GetStart(fileBufferOut),
                                  GWEN_Buffer_GetUsedBytes(fileBufferOut));
  if (rv<0) {
    DBG_ERROR(NULL, "Could not write \"%s\" (%d)", fileName, rv);
    GWEN_Buffer_free(fileBufferOut);
    GWEN_Buffer_free(fileBufferIn);
    GWEN_Buffer_free(fileNameBuffer);
    return rv;
  }

  rv=_setFilePermissions(fileName, sourceFilePerms);
  if (rv<0) {
    DBG_ERROR(NULL, "Could not set perms for \"%s\" (%d)", fileName, rv);
    GWEN_Buffer_free(fileBufferOut);
    GWEN_Buffer_free(fileBufferIn);
    GWEN_Buffer_free(fileNameBuffer);
    return rv;
  }

  /* add output file */
  file=GWB_File_List2_GetOrCreateFile(GWB_Project_GetFileList(project),
                                      GWB_Context_GetCurrentRelativeDir(currentContext),
                                      fileName);
  GWB_File_AddFlags(file, GWB_FILE_FLAGS_GENERATED);

  /* add input file */
  GWEN_Buffer_Reset(fileNameBuffer);
  GWEN_Buffer_AppendString(fileNameBuffer, fileName);
  GWEN_Buffer_AppendString(fileNameBuffer, ".in");
  file=GWB_File_List2_GetOrCreateFile(GWB_Project_GetFileList(project),
                                      GWB_Context_GetCurrentRelativeDir(currentContext),
                                      GWEN_Buffer_GetStart(fileNameBuffer));
  GWB_File_AddFlags(file, GWB_FILE_FLAGS_DIST);

  GWEN_Buffer_free(fileBufferOut);
  GWEN_Buffer_free(fileBufferIn);
  GWEN_Buffer_free(fileNameBuffer);

  return 0;
}



int _getFilePermissions(const char *fname)
{
  struct stat st;

  if (lstat(fname, &st) == -1) {
    DBG_ERROR(NULL, "Error on stat(\"%s\"): %d (%s)", fname, errno, strerror(errno));
    return GWEN_ERROR_IO;
  }

  return (int)((st.st_mode & ~S_IFMT));
}



int _setFilePermissions(const char *fname, int perms)
{
  if (chmod(fname, perms) == -1) {
    DBG_ERROR(NULL, "Error on chmod(\"%s\"): %d (%s)", fname, errno, strerror(errno));
    return GWEN_ERROR_IO;
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




int GWB_Parser_ReplaceVarsBetweenAtSigns(const char *s, GWEN_BUFFER *dbuf, GWEN_DB_NODE *db)
{
  const char *p;

#if 0
  DBG_ERROR(NULL, "Using vars:");
  GWEN_DB_Dump(db, 2);
#endif

  p=s;
  while (*p) {
    if (*p=='@') {
      p++;
      if (*p=='@')
        GWEN_Buffer_AppendByte(dbuf, '@');
      else if (!isalpha(*p)) {
        GWEN_Buffer_AppendByte(dbuf, '@');
        GWEN_Buffer_AppendByte(dbuf, *p);
      }
      else {
        const char *pStart;

        pStart=p;
        while (*p && *p!='@' && (isalnum(*p) || *p=='_'))
          p++;
        if (*p=='@') {
          int len;
          char *rawName;
          const char *value;

          len=p-pStart;
          if (len<1) {
            DBG_ERROR(GWEN_LOGDOMAIN, "Empty variable name in code");
            return GWEN_ERROR_BAD_DATA;
          }
          rawName=(char *) malloc(len+1);
          assert(rawName);
          memmove(rawName, pStart, len);
          rawName[len]=0;

          /*DBG_ERROR(NULL, "Setting data from variable \"%s\"", rawName);*/
          value=GWEN_DB_GetCharValue(db, rawName, 0, NULL);
          if (value)
            GWEN_Buffer_AppendString(dbuf, value);
          else {
            DBG_WARN(NULL, "Warning: Empty value for DB var \"%s\"", rawName);
          }
          free(rawName);
        }
        else {
          DBG_ERROR(GWEN_LOGDOMAIN, "Not interpreting at sign as start of variable name, adding to output.");
          p=pStart;
          GWEN_Buffer_AppendByte(dbuf, '@');
          GWEN_Buffer_AppendByte(dbuf, *p);
        }
      }
      p++;
    }
    else {
      GWEN_Buffer_AppendByte(dbuf, *p);
      p++;
    }
  }

  return 0;
}



int GWB_Parser_ParseWellKnownElements(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *n, GWB_PARSER_PARSE_ELEMENT_FN fn)
{
  const char *name;

  name=GWEN_XMLNode_GetData(n);
  if (name && *name) {
    int rv;

    DBG_DEBUG(NULL, "Handling element \"%s\"", name);

    if (strcasecmp(name, "writeFile")==0)
      rv=_parseWriteFile(project, currentContext, n);
    else if (strcasecmp(name, "setVar")==0)
      rv=_parseSetVar(currentContext, n);
    else if (strcasecmp(name, "ifVarMatches")==0)
      rv=_parseIfVarMatches(project, currentContext, n, fn);
    else if (strcasecmp(name, "ifNotVarMatches")==0)
      rv=_parseIfNotVarMatches(project, currentContext, n, fn);
    else if (strcasecmp(name, "ifVarHasValue")==0)
      rv=_parseIfVarHasValue(project, currentContext, n, fn);
    else if (strcasecmp(name, "ifNotVarHasValue")==0)
      rv=_parseIfNotVarHasValue(project, currentContext, n, fn);
    else {
      DBG_DEBUG(NULL, "Element not handled here, ignoring");
      rv=1;
    }
    if (rv<0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error in element \"%s\", aborting", name);
      return rv;
    }
  }

  return 0;
}



int GWB_Parser_ParseSourcesOrHeaders(GWB_PROJECT *project,
                                     GWB_CONTEXT *currentContext,
                                     GWEN_XMLNODE *xmlNode,
                                     int alwaysDist,
                                     int isSource)
{
  uint32_t flags=0;
  int rv;
  const char *s;
  const char *installPath;
  const char *fileType;
  const char *currentFolder;
  const char *builder;
  GWEN_STRINGLIST *fileNameList;

  rv=GWEN_XMLNode_ExpandProperties(xmlNode, GWB_Context_GetVars(currentContext));
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    return rv;
  }

  currentFolder=GWB_Context_GetCurrentRelativeDir(currentContext);

  fileType=GWEN_XMLNode_GetProperty(xmlNode, "type", NULL);
  builder=GWEN_XMLNode_GetProperty(xmlNode, "builder", NULL);

  installPath=GWEN_XMLNode_GetProperty(xmlNode, "install", NULL);
  if (installPath && *installPath)
    flags|=GWB_FILE_FLAGS_INSTALL;

  s=GWEN_XMLNode_GetProperty(xmlNode, "generated", "FALSE");
  if (s && *s && (strcasecmp(s, "true")==0 || strcasecmp(s, "yes")==0))
    flags|=GWB_FILE_FLAGS_GENERATED;

  s=GWEN_XMLNode_GetProperty(xmlNode, "dist", alwaysDist?"TRUE":"FALSE");
  if (s && *s && (strcasecmp(s, "true")==0 || strcasecmp(s, "yes")==0))
    flags|=GWB_FILE_FLAGS_DIST;

  fileNameList=GWB_Parser_ReadXmlDataIntoStringList(GWB_Context_GetVars(currentContext), xmlNode);
  if (fileNameList) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(fileNameList);
    while(se) {
      const char *sFileName;

      sFileName=GWEN_StringListEntry_Data(se);
      if (sFileName && *sFileName) {
        GWB_FILE *file;

        file=GWB_File_List2_GetOrCreateFile(GWB_Project_GetFileList(project), currentFolder, sFileName);
        GWB_File_AddFlags(file, flags);
        if (installPath)
          GWB_File_SetInstallPath(file, installPath);
        if (fileType)
          GWB_File_SetFileType(file, fileType);
        if (builder)
          GWB_File_SetBuilder(file, builder);
        if (isSource)
          GWB_Context_AddSourceFile(currentContext, file);
      }

      se=GWEN_StringListEntry_Next(se);
    }
    GWEN_StringList_free(fileNameList);
  }

  return 0;
}


