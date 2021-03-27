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


#include "gwenbuild/parser/p_target.h"
#include "gwenbuild/parser/parser.h"

#include <gwenhywfar/debug.h>



static GWB_TARGET *_readTarget(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);
static int _parseChildNodes(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);
static int _parseSourcesOrHeaders(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);
static int _parseUsedTargets(GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);
static int _parseIncludes(GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);






int GWB_ParseTarget(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  int rv;
  GWB_TARGET *target;
  GWB_CONTEXT *newContext;

  rv=GWEN_XMLNode_ExpandProperties(xmlNode, GWB_Context_GetVars(currentContext));
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    return rv;
  }

  target=_readTarget(project, currentContext, xmlNode);


  newContext=GWB_Parser_CopyContextForTarget(currentContext);
  GWB_Context_SetCurrentTarget(newContext, target);
  GWB_Target_SetContext(target, newContext);

  rv=_parseChildNodes(project, newContext, xmlNode);
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    GWB_Context_free(newContext);
    GWB_Target_free(target);
    return rv;
  }

  GWB_Project_AddTarget(project, target);
  GWB_Context_Tree2_AddChild(currentContext, newContext);

  return 0;
}



GWB_TARGET *_readTarget(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  GWB_TARGET *target;
  const char *s;
  GWBUILD_TARGETTYPE targetType;
  uint32_t flags=GWEN_DB_FLAGS_OVERWRITE_VARS;
  GWEN_DB_NODE *db;

  target=GWB_Target_new(project);

  s=GWEN_XMLNode_GetProperty(xmlNode, "name", NULL);
  if (!(s && *s)) {
    DBG_ERROR(NULL, "Target has no name");
    return NULL;
  }
  GWB_Target_SetName(target, s);

  s=GWEN_XMLNode_GetProperty(xmlNode, "type", NULL);
  if (!(s && *s)) {
    DBG_ERROR(NULL, "Target has no type");
    GWB_Target_free(target);
    return NULL;
  }
  targetType=GWBUILD_TargetType_fromString(s);
  if (targetType==GWBUILD_TargetType_Invalid) {
    DBG_ERROR(NULL, "Bad target type \"%s\"", s?s:"<empty>");
    GWB_Target_free(target);
    return NULL;
  }
  GWB_Target_SetTargetType(target, targetType);

  GWB_Target_SetSoVersion(target,
                          GWEN_XMLNode_GetIntProperty(xmlNode, "so_current", 0),
                          GWEN_XMLNode_GetIntProperty(xmlNode, "so_age", 0),
                          GWEN_XMLNode_GetIntProperty(xmlNode, "so_revision", 0));

  db=GWB_Context_GetVars(currentContext);
  GWEN_DB_SetCharValue(db, flags, "target_name", GWB_Target_GetName(target));
  GWEN_DB_SetIntValue(db, flags, "target_so_current", GWB_Target_GetSoVersionCurrent(target));
  GWEN_DB_SetIntValue(db, flags, "target_so_age", GWB_Target_GetSoVersionAge(target));
  GWEN_DB_SetIntValue(db, flags, "target_so_revision", GWB_Target_GetSoVersionRevision(target));

  return target;
}



int _parseChildNodes(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  GWEN_XMLNODE *n;

  n=GWEN_XMLNode_GetFirstTag(xmlNode);
  while (n) {
    const char *name;

    name=GWEN_XMLNode_GetData(n);
    if (name && *name) {
      int rv;

      DBG_INFO(NULL, "Handling element \"%s\"", name);

      if (strcasecmp(name, "subdirs")==0)
        rv=GWB_Parser_ParseSubdirs(project, currentContext, n, _parseChildNodes);
      else if ((strcasecmp(name, "sources")==0) || (strcasecmp(name, "headers")==0))
        rv=_parseSourcesOrHeaders(project, currentContext, n);
      else if (strcasecmp(name, "setVar")==0)
        rv=GWB_Parser_ParseSetVar(currentContext, n);
      else if (strcasecmp(name, "useTargets")==0)
        rv=_parseUsedTargets(currentContext, n);
      else if (strcasecmp(name, "includes")==0)
        rv=_parseIncludes(currentContext, n);
      else if (strcasecmp(name, "target")==0)
        rv=GWB_ParseTarget(project, currentContext, n);
      else {
        DBG_ERROR(NULL, "Element not handled");
        rv=0;
      }
      if (rv<0) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Error in element \"%s\", aborting", name);
        return rv;
      }
    }

    n=GWEN_XMLNode_GetNextTag(n);
  }

  return 0;
}



int _parseSourcesOrHeaders(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  GWB_TARGET *target;
  uint32_t flags=0;
  int rv;
  const char *s;
  const char *installPath;
  const char *fileType;
  const char *currentFolder;
  GWEN_STRINGLIST *fileNameList;

  target=GWB_Context_GetCurrentTarget(currentContext);
  if (target==NULL) {
    DBG_ERROR(NULL, "No target in current context, SNH!");
    return GWEN_ERROR_INTERNAL;
  }

  rv=GWEN_XMLNode_ExpandProperties(xmlNode, GWB_Context_GetVars(currentContext));
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    return rv;
  }

  currentFolder=GWB_Context_GetCurrentRelativeDir(currentContext);

  fileType=GWEN_XMLNode_GetProperty(xmlNode, "type", NULL);

  installPath=GWEN_XMLNode_GetProperty(xmlNode, "install", NULL);
  if (installPath && *installPath)
    flags|=GWB_FILE_FLAGS_INSTALL;

  s=GWEN_XMLNode_GetProperty(xmlNode, "dist", NULL);
  if (s && *s && (strcasecmp(s, "true")==0 || strcasecmp(s, "yes")==0))
    flags|=GWB_FILE_FLAGS_DIST;

  fileNameList=GWB_Parser_ReadXmlDataIntoStringList(currentContext, xmlNode);
  if (fileNameList) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(fileNameList);
    while(se) {
      const char *sFileName;

      sFileName=GWEN_StringListEntry_Data(se);
      if (sFileName && *sFileName) {
        GWB_FILE *file;

        file=GWB_Project_GetFileByPathAndName(project, currentFolder, sFileName);
        if (file==NULL) {
          file=GWB_File_new(currentFolder, sFileName, 0);
          GWB_Project_AddFile(project, file);
        }
        GWB_File_AddFlags(file, flags);
        if (installPath)
          GWB_File_SetInstallPath(file, installPath);
        if (fileType)
          GWB_File_SetFileType(file, fileType);
        GWB_Context_AddSourceFile(currentContext, file);
        //GWB_Target_AddSourceFile(target, file);
      }

      se=GWEN_StringListEntry_Next(se);
    }
    GWEN_StringList_free(fileNameList);
  }

  return 0;
}



int _parseUsedTargets(GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  GWB_TARGET *target;
  int rv;
  GWEN_STRINGLIST *targetNameList;

  target=GWB_Context_GetCurrentTarget(currentContext);
  if (target==NULL) {
    DBG_ERROR(NULL, "No target in current context, SNH!");
    return GWEN_ERROR_INTERNAL;
  }

  rv=GWEN_XMLNode_ExpandProperties(xmlNode, GWB_Context_GetVars(currentContext));
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    return rv;
  }

  targetNameList=GWB_Parser_ReadXmlDataIntoStringList(currentContext, xmlNode);
  if (targetNameList) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(targetNameList);
    while(se) {
      const char *sTargetName;

      sTargetName=GWEN_StringListEntry_Data(se);
      if (sTargetName && *sTargetName)
        GWB_Target_AddUsedTargetName(target, sTargetName);

      se=GWEN_StringListEntry_Next(se);
    }
    GWEN_StringList_free(targetNameList);
  }

  return 0;
}



int _parseIncludes(GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  GWB_TARGET *target;
  int rv;
  const char *builderType;
  GWEN_STRINGLIST *entryList;

  target=GWB_Context_GetCurrentTarget(currentContext);
  if (target==NULL) {
    DBG_ERROR(NULL, "No target in current context, SNH!");
    return GWEN_ERROR_INTERNAL;
  }

  rv=GWEN_XMLNode_ExpandProperties(xmlNode, GWB_Context_GetVars(currentContext));
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    return rv;
  }

  builderType=GWEN_XMLNode_GetProperty(xmlNode, "type", "c");

  entryList=GWB_Parser_ReadXmlDataIntoStringList(currentContext, xmlNode);
  if (entryList) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(entryList);
    while(se) {
      const char *sEntry;

      sEntry=GWEN_StringListEntry_Data(se);
      if (sEntry && *sEntry)
        GWB_Context_AddInclude(currentContext, builderType, sEntry);

      se=GWEN_StringListEntry_Next(se);
    }
    GWEN_StringList_free(entryList);
  }

  return 0;
}




