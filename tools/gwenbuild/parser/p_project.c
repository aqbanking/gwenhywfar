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
#include "gwenbuild/parser/p_checkfunctions.h"
#include "gwenbuild/parser/p_checkheaders.h"
#include "gwenbuild/parser/p_checklibs.h"
#include "gwenbuild/parser/p_dependencies.h"
#include "gwenbuild/parser/p_options.h"
#include "gwenbuild/parser/p_target.h"

#include "gwenbuild/parser/parser.h"

#include <gwenhywfar/debug.h>

#include <unistd.h>



static int _parseChildNodes(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);
static int _writeConfigH(const GWB_PROJECT *project);






int GWB_ParseProject(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  const char *s;
  int rv;
  GWEN_DB_NODE *db;
  uint32_t flags=GWEN_DB_FLAGS_OVERWRITE_VARS;

  s=GWEN_XMLNode_GetProperty(xmlNode, "name", NULL);
  if (!(s && *s)) {
    DBG_ERROR(NULL, "Project has no name");
    return GWEN_ERROR_BAD_DATA;
  }

  GWB_Project_SetProjectName(project, s);

  s=GWEN_XMLNode_GetProperty(xmlNode, "write_config_h", "TRUE");
  if (s && strcasecmp(s, "TRUE")==0)
    GWB_Project_AddFlags(project, GWB_PROJECT_FLAGS_CONFIG_H);

  GWB_Project_SetVersion(project,
                         GWEN_XMLNode_GetIntProperty(xmlNode, "vmajor", 0),
                         GWEN_XMLNode_GetIntProperty(xmlNode, "vminor", 0),
                         GWEN_XMLNode_GetIntProperty(xmlNode, "vpatchlevel", 0),
                         GWEN_XMLNode_GetIntProperty(xmlNode, "vbuild", 0));

  GWB_Project_SetSoVersion(project,
                           GWEN_XMLNode_GetIntProperty(xmlNode, "so_current", 0),
                           GWEN_XMLNode_GetIntProperty(xmlNode, "so_age", 0),
                           GWEN_XMLNode_GetIntProperty(xmlNode, "so_revision", 0));

  db=GWB_Context_GetVars(currentContext);
  GWEN_DB_SetCharValue(db, flags, "project_name", GWB_Project_GetProjectName(project));
  GWEN_DB_SetIntValue(db, flags, "project_vmajor", GWB_Project_GetVersionMajor(project));
  GWEN_DB_SetIntValue(db, flags, "project_vminor", GWB_Project_GetVersionMinor(project));
  GWEN_DB_SetIntValue(db, flags, "project_vpatchlevel", GWB_Project_GetVersionPatchlevel(project));
  GWEN_DB_SetIntValue(db, flags, "project_vbuild", GWB_Project_GetVersionBuild(project));

  GWEN_DB_SetIntValue(db, flags, "project_so_current", GWB_Project_GetSoVersionCurrent(project));
  GWEN_DB_SetIntValue(db, flags, "project_so_age", GWB_Project_GetSoVersionAge(project));
  GWEN_DB_SetIntValue(db, flags, "project_so_revision", GWB_Project_GetSoVersionRevision(project));


  rv=_parseChildNodes(project, currentContext, xmlNode);
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    return rv;
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

      if (strcasecmp(name, "options")==0)
        rv=GWB_ParseOptions(project, currentContext, n);
      else if (strcasecmp(name, "dependencies")==0)
        rv=GWB_ParseDependencies(project, currentContext, n);
      else if (strcasecmp(name, "checkheaders")==0)
        rv=GWB_ParseCheckHeaders(project, currentContext, n);
      else if (strcasecmp(name, "checkfunctions")==0)
        rv=GWB_ParseCheckFunctions(project, currentContext, n);
      else if (strcasecmp(name, "checklibs")==0)
        rv=GWB_ParseCheckLibs(project, currentContext, n);
      else if (strcasecmp(name, "target")==0)
        rv=GWB_ParseTarget(project, currentContext, n);
      else if (strcasecmp(name, "subdirs")==0)
        rv=GWB_Parser_ParseSubdirs(project, currentContext, n, _parseChildNodes);
      else if (strcasecmp(name, "setVar")==0)
        rv=GWB_Parser_ParseSetVar(currentContext, n);
      else {
        DBG_DEBUG(NULL, "Element not handled, ignoring");
        rv=0;
      }
      if (rv<0) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Error in element \"%s\", aborting", name);
        return rv;
      }
    }

    n=GWEN_XMLNode_GetNextTag(n);
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



int _writeConfigH(const GWB_PROJECT *project)
{
  GWB_KEYVALUEPAIR_LIST *defineList;
  GWEN_BUFFER *fileBuffer;
  int rv;

  defineList=GWB_Project_GetDefineList(project);
  fileBuffer=GWEN_Buffer_new(0, 256, 0, 1);

  GWEN_Buffer_AppendString(fileBuffer, "# config.h generated by gwenbuild\n\n");
  if (defineList)
    GWB_KeyValuePair_List_WriteAllPairsToBuffer(defineList, "#define ", " ", "\n", fileBuffer);

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




