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


#include "gwenbuild/parser/p_project.h"
#include "gwenbuild/parser/p_buildfiles.h"
#include "gwenbuild/parser/p_checkfunctions.h"
#include "gwenbuild/parser/p_checkheaders.h"
#include "gwenbuild/parser/p_checklibs.h"
#include "gwenbuild/parser/p_checkcompiler.h"
#include "gwenbuild/parser/p_checkprogs.h"
#include "gwenbuild/parser/p_dependencies.h"
#include "gwenbuild/parser/p_options.h"
#include "gwenbuild/parser/p_target.h"
#include "gwenbuild/utils.h"

#include "gwenbuild/parser/parser.h"

#include <gwenhywfar/debug.h>

#include <unistd.h>



static int _parseVersions(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);
static int _parseChildNodes(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);
static int _writeConfigH(const GWB_PROJECT *project);
static int _parseDefine(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);






int GWB_ParseProject(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  const char *s;
  int rv;
  GWEN_DB_NODE *db;
  uint32_t flags=GWEN_DB_FLAGS_OVERWRITE_VARS;

  db=GWB_Context_GetVars(currentContext);

  s=GWEN_XMLNode_GetProperty(xmlNode, "name", NULL);
  if (!(s && *s)) {
    DBG_ERROR(NULL, "Project has no name");
    return GWEN_ERROR_BAD_DATA;
  }

  GWB_Project_SetProjectName(project, s);
  GWEN_DB_SetCharValue(db, flags, "PACKAGE", s);
  GWB_Project_SetDefineQuoted(project, "PACKAGE", s);

  s=GWEN_XMLNode_GetProperty(xmlNode, "write_config_h", "TRUE");
  if (s && strcasecmp(s, "TRUE")==0)
    GWB_Project_AddFlags(project, GWB_PROJECT_FLAGS_CONFIG_H);

  rv=_parseVersions(project, currentContext, xmlNode);
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    return rv;
  }

  rv=_parseChildNodes(project, currentContext, xmlNode);
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    return rv;
  }

  if (GWB_Project_GetFlags(project) & GWB_PROJECT_FLAGS_CONFIG_H) {
    int rv;

    rv=_writeConfigH(project);
    if (rv<0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error writing config.h, aborting");
      return rv;
    }
  }

  return 0;
}



int _parseVersions(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  const char *s;
  int rv;
  GWEN_DB_NODE *db;
  uint32_t flags=GWEN_DB_FLAGS_OVERWRITE_VARS;

  db=GWB_Context_GetVars(currentContext);

  s=GWEN_XMLNode_GetProperty(xmlNode, "version", NULL);
  if (s && *s) {
    GWEN_DB_NODE *versionDb;

    GWEN_DB_SetCharValue(db, flags, "project_version", s);
    versionDb=GWEN_DB_Group_new("versionDb");
    rv=GWB_Utils_VersionStringToDb(versionDb, NULL, s);
    if (rv<0) {
      DBG_ERROR(NULL, "Invalid version string [%s]", s);
      GWEN_DB_Group_free(versionDb);
      return GWEN_ERROR_BAD_DATA;
    }
    GWB_Project_SetVersion(project,
                           GWEN_DB_GetIntValue(versionDb, "vmajor", 0, 0),
                           GWEN_DB_GetIntValue(versionDb, "vminor", 0, 0),
                           GWEN_DB_GetIntValue(versionDb, "vpatchlevel", 0, 0),
                           GWEN_DB_GetIntValue(versionDb, "vbuild", 0, 0),
                           GWEN_DB_GetCharValue(versionDb, "vtag", 0, NULL));
    GWEN_DB_Group_free(versionDb);
  }
  else {
    GWB_Project_SetVersion(project,
                           GWEN_XMLNode_GetIntProperty(xmlNode, "vmajor", 0),
                           GWEN_XMLNode_GetIntProperty(xmlNode, "vminor", 0),
                           GWEN_XMLNode_GetIntProperty(xmlNode, "vpatchlevel", 0),
                           GWEN_XMLNode_GetIntProperty(xmlNode, "vbuild", 0),
                           GWEN_XMLNode_GetProperty(xmlNode, "vtag", NULL));
    GWB_Utils_VersionToDbVar(db, "project_version",
                             GWB_Project_GetVersionMajor(project),
                             GWB_Project_GetVersionMinor(project),
                             GWB_Project_GetVersionPatchlevel(project),
                             GWB_Project_GetVersionBuild(project),
                             GWB_Project_GetVersionTag(project));
  }
  GWEN_DB_SetCharValue(db, flags, "project_name", GWB_Project_GetProjectName(project));
  GWEN_DB_SetCharValueFromInt(db, flags, "project_vmajor", GWB_Project_GetVersionMajor(project));
  GWEN_DB_SetCharValueFromInt(db, flags, "project_vminor", GWB_Project_GetVersionMinor(project));
  GWEN_DB_SetCharValueFromInt(db, flags, "project_vpatchlevel", GWB_Project_GetVersionPatchlevel(project));
  GWEN_DB_SetCharValueFromInt(db, flags, "project_vbuild", GWB_Project_GetVersionBuild(project));
  s=GWB_Project_GetVersionTag(project);
  if (s && *s)
    GWEN_DB_SetCharValue(db, flags, "project_vtag", s);


  GWB_Project_SetSoVersion(project,
                           GWEN_XMLNode_GetIntProperty(xmlNode, "so_current", 0),
                           GWEN_XMLNode_GetIntProperty(xmlNode, "so_age", 0),
                           GWEN_XMLNode_GetIntProperty(xmlNode, "so_revision", 0));

  GWEN_DB_SetCharValueFromInt(db, flags, "project_so_current", GWB_Project_GetSoVersionCurrent(project));
  GWEN_DB_SetCharValueFromInt(db, flags, "project_so_age", GWB_Project_GetSoVersionAge(project));
  GWEN_DB_SetCharValueFromInt(db, flags, "project_so_revision", GWB_Project_GetSoVersionRevision(project));
  GWEN_DB_SetCharValueFromInt(db, flags, "project_so_effective",
                              GWB_Project_GetSoVersionCurrent(project)-GWB_Project_GetSoVersionAge(project));
  s=GWEN_DB_GetCharValue(db, "project_version", 0, NULL);
  if (s && *s) {
    GWEN_DB_SetCharValue(db, flags, "VERSION", s);
    GWB_Project_SetDefineQuoted(project, "VERSION", s);
  }

  return 0;
}



int _parseChildNodes(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  GWEN_XMLNODE *n;

  n=GWEN_XMLNode_GetFirstTag(xmlNode);
  while (n) {
    const char *name;

    DBG_DEBUG(NULL, "- Reading child node");
    name=GWEN_XMLNode_GetData(n);
    if (name && *name) {
      int rv;

      DBG_DEBUG(NULL, "Handling element \"%s\"", name);

      if (strcasecmp(name, "option")==0)
        rv=GWB_ParseOption(project, currentContext, n);
      else if (strcasecmp(name, "dependencies")==0)
        rv=GWB_ParseDependencies(project, currentContext, n);
      else if (strcasecmp(name, "checkheaders")==0)
        rv=GWB_ParseCheckHeaders(project, currentContext, n);
      else if (strcasecmp(name, "checkfunctions")==0)
        rv=GWB_ParseCheckFunctions(project, currentContext, n);
      else if (strcasecmp(name, "checklibs")==0)
        rv=GWB_ParseCheckLibs(project, currentContext, n);
      else if (strcasecmp(name, "checkprogs")==0)
        rv=GWB_ParseCheckProgs(project, currentContext, n);
      else if (strcasecmp(name, "checkCompiler")==0)
        rv=GWB_ParseCheckCompiler(project, currentContext, n);
      else if (strcasecmp(name, "target")==0)
        rv=GWB_ParseTarget(project, currentContext, n);
      else if (strcasecmp(name, "define")==0)
        rv=_parseDefine(project, currentContext, n);
      else if (strcasecmp(name, "buildFiles")==0)
        rv=GWB_ParseBuildFiles(project, currentContext, n);
      else if (strcasecmp(name, "extradist")==0)
        rv=GWB_Parser_ParseSourcesOrHeaders(project, currentContext, n, 1, 0);
      else if (strcasecmp(name, "data")==0)
        rv=GWB_Parser_ParseSourcesOrHeaders(project, currentContext, n, 1, 0);
      else if (strcasecmp(name, "subdirs")==0)
        rv=GWB_Parser_ParseSubdirs(project, currentContext, n, _parseChildNodes);
      else
        rv=GWB_Parser_ParseWellKnownElements(project, currentContext, n, _parseChildNodes);
      if (rv<0) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Error in element \"%s\", aborting", name);
        return rv;
      }
    }

    n=GWEN_XMLNode_GetNextTag(n);
  }

  return 0;
}



int _writeConfigH(const GWB_PROJECT *project)
{
  GWB_KEYVALUEPAIR_LIST *defineList;
  GWEN_BUFFER *fileBuffer;
  int rv;

  defineList=GWB_Project_GetDefineList(project);
  fileBuffer=GWEN_Buffer_new(0, 256, 0, 1);

  GWEN_Buffer_AppendString(fileBuffer, "/* config.h generated by gwenbuild */\n\n");
  if (defineList)
    GWB_KeyValuePair_List_WriteAllPairsToBuffer(defineList, "#define ", " ", "\n", fileBuffer);
  GWEN_Buffer_AppendString(fileBuffer, "\n");

  unlink("config.h");
  rv=GWEN_SyncIo_Helper_WriteFile("config.h",
                                  (const uint8_t*)GWEN_Buffer_GetStart(fileBuffer),
                                  GWEN_Buffer_GetUsedBytes(fileBuffer));
  if (rv<0) {
    DBG_ERROR(NULL, "Could not write \"config.h\" (%d)", rv);
    GWEN_Buffer_free(fileBuffer);
    return rv;
  }

  GWEN_Buffer_free(fileBuffer);

  return 0;
}



int _parseDefine(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  int rv;
  const char *varName;
  const char *value;
  const char *quoted;

  rv=GWEN_XMLNode_ExpandProperties(xmlNode, GWB_Context_GetVars(currentContext));
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    return rv;
  }

  quoted=GWEN_XMLNode_GetProperty(xmlNode, "quoted", "FALSE");
  varName=GWEN_XMLNode_GetProperty(xmlNode, "name", NULL);
  value=GWEN_XMLNode_GetProperty(xmlNode, "value", NULL);
  if (!(varName && *varName)) {
    DBG_ERROR(NULL, "Missing variable name in DEFINE");
    return GWEN_ERROR_GENERIC;
  }
  if (quoted && strcasecmp(quoted, "TRUE")==0)
    GWB_Project_SetDefineQuoted(project, varName, value);
  else
    GWB_Project_SetDefine(project, varName, value);


  return 0;
}




