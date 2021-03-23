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



static GWB_TARGET *_readTarget(GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);
static int _parseChildNodes(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);






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

  target=_readTarget(currentContext, xmlNode);


  newContext=GWB_Parser_CopyContextForTarget(currentContext);
  GWB_Context_SetCurrentTarget(newContext, target);

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



GWB_TARGET *_readTarget(GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  GWB_TARGET *target;
  const char *s;
  GWBUILD_TARGETTYPE targetType;
  uint32_t flags=GWEN_DB_FLAGS_OVERWRITE_VARS;
  GWEN_DB_NODE *db;

  target=GWB_Target_new();

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

  DBG_INFO(NULL, "Entering");

  n=GWEN_XMLNode_GetFirstTag(xmlNode);
  while (n) {
    const char *name;

    name=GWEN_XMLNode_GetData(n);
    if (name && *name) {
      int rv;

      DBG_INFO(NULL, "Handling element \"%s\"", name);

      if (strcasecmp(name, "subdirs")==0)
        rv=GWB_Parser_ParseSubdirs(project, currentContext, n, _parseChildNodes);
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

  DBG_INFO(NULL, "Leaving");
  return 0;
}



