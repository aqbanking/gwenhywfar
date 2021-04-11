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


#include "gwenbuild/parser/p_checkcompiler.h"
#include "gwenbuild/parser/parser.h"
#include "gwenbuild/tools/trycompile.h"

#include <gwenhywfar/debug.h>

#include <ctype.h>



static int _parseChildNodes(GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);
static int _parseArg(GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);






int GWB_ParseCheckCompiler(GWEN_UNUSED GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  int rv;

  rv=_parseChildNodes(currentContext, xmlNode);
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int _parseChildNodes(GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  GWEN_XMLNODE *n;

  n=GWEN_XMLNode_GetFirstTag(xmlNode);
  while (n) {
    const char *name;

    name=GWEN_XMLNode_GetData(n);
    if (name && *name) {
      int rv;

      DBG_DEBUG(NULL, "Handling element \"%s\"", name);

      if (strcasecmp(name, "arg")==0)
        rv=_parseArg(currentContext, n);
      else {
        DBG_INFO(NULL, "Element not handled");
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



int _parseArg(GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  int rv;
  const char *sName;
  GWEN_DB_NODE *db;
  GWEN_BUFFER *argsBuffer;

  db=GWB_Context_GetVars(currentContext);

  rv=GWEN_XMLNode_ExpandProperties(xmlNode, db);
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    return rv;
  }

  sName=GWEN_XMLNode_GetProperty(xmlNode, "name", NULL);
  if (!(sName && *sName)) {
    DBG_ERROR(NULL, "Dependency has no name");
    return GWEN_ERROR_GENERIC;
  }

  argsBuffer=GWB_Parser_ReadXmlDataIntoBufferAndExpand(GWB_Context_GetVars(currentContext), xmlNode);
  if (argsBuffer) {
    rv=GWB_Tools_CheckCompilerArgs(currentContext, GWEN_Buffer_GetStart(argsBuffer));
    if (rv<0) {
      DBG_INFO(NULL, "here (%d)", rv);
      return rv;
    }
    else if (rv==0) {
      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, sName, "TRUE");
      fprintf(stdout, " compiler args %s: TRUE\n", sName);
    }
    else {
      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, sName, "FALSE");
      fprintf(stdout, " compiler args %s: FALSE\n", sName);
    }
  }
  return 0;
}



