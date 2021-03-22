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
#include "gwenbuild/parser/p_dependencies.h"
#include "gwenbuild/parser/p_options.h"
#include "gwenbuild/parser/p_target.h"

#include "gwenbuild/parser/parser.h"

#include <gwenhywfar/debug.h>



static int _parseChildNodes(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);






int GWB_ParseProject(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  const char *s;
  int rv;

  s=GWEN_XMLNode_GetProperty(xmlNode, "name", NULL);
  if (!(s && *s)) {
    DBG_ERROR(NULL, "Project has no name");
    return GWEN_ERROR_BAD_DATA;
  }

  GWB_Project_SetProjectName(project, s);

  GWB_Project_SetVersion(project,
                         GWEN_XMLNode_GetIntProperty(xmlNode, "vmajor", 0),
                         GWEN_XMLNode_GetIntProperty(xmlNode, "vminor", 0),
                         GWEN_XMLNode_GetIntProperty(xmlNode, "vpatchlevel", 0),
                         GWEN_XMLNode_GetIntProperty(xmlNode, "vbuild", 0));

  GWB_Project_SetSoVersion(project,
                           GWEN_XMLNode_GetIntProperty(xmlNode, "so_current", 0),
                           GWEN_XMLNode_GetIntProperty(xmlNode, "so_age", 0),
                           GWEN_XMLNode_GetIntProperty(xmlNode, "so_revision", 0));

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

    name=GWEN_XMLNode_GetData(n);
    if (name && *name) {
      int rv;

      DBG_INFO(NULL, "Handling element \"%s\"", name);

      if (strcasecmp(name, "options")==0)
        rv=GWB_ParseOptions(project, currentContext, n);
      else if (strcasecmp(name, "dependencies")==0)
        rv=GWB_ParseDependencies(project, currentContext, n);
      else if (strcasecmp(name, "checkheaders")==0)
        rv=GWB_ParseCheckHeaders(project, currentContext, n);
      else if (strcasecmp(name, "checkfunctions")==0)
        rv=GWB_ParseCheckFunctions(project, currentContext, n);
      else if (strcasecmp(name, "target")==0)
        rv=GWB_ParseTarget(project, currentContext, n);
      else if (strcasecmp(name, "subdir")==0)
        rv=GWB_Parser_ParseSubdirs(project, currentContext, n, _parseChildNodes);
      else {
        DBG_ERROR(NULL, "Element not handled, ignoring");
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


