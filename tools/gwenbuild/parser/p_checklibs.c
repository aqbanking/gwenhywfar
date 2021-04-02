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


#include "gwenbuild/parser/p_checkheaders.h"
#include "gwenbuild/parser/parser.h"
#include "gwenbuild/tools/trylink.h"

#include <gwenhywfar/debug.h>



static int _parseChildNodes(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);
static int _parseLib(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);
static GWEN_BUFFER *_createTestCodeForLib(const char *fnName);






int GWB_ParseCheckLibs(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  int rv;

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

      DBG_DEBUG(NULL, "Handling element \"%s\"", name);

      if (strcasecmp(name, "lib")==0)
        rv=_parseLib(project, currentContext, n);
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



int _parseLib(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  int rv;
  const char *sId;
  const char *sName;
  const char *sFunction;
  GWEN_DB_NODE *db;

  db=GWB_Context_GetVars(currentContext);

  rv=GWEN_XMLNode_ExpandProperties(xmlNode, db);
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    return rv;
  }

  sId=GWEN_XMLNode_GetProperty(xmlNode, "id", NULL);
  if (!(sId && *sId)) {
    DBG_ERROR(NULL, "Dependency has no id");
    return GWEN_ERROR_GENERIC;
  }

  sName=GWEN_XMLNode_GetProperty(xmlNode, "name", NULL);
  if (!(sName && *sName)) {
    DBG_ERROR(NULL, "Dependency has no name");
    return GWEN_ERROR_GENERIC;
  }


  sFunction=GWEN_XMLNode_GetProperty(xmlNode, "function", NULL);
  if (sFunction && *sFunction) {
    GWEN_BUFFER *codeBuf;

    codeBuf=_createTestCodeForLib(sFunction);
    rv=GWB_Tools_TryLink(GWB_Project_GetGwbuild(project), GWEN_Buffer_GetStart(codeBuf), sName);
    GWEN_Buffer_free(codeBuf);
  }
  else
    rv=GWB_Tools_TryLink(GWB_Project_GetGwbuild(project), "int main(int argc, char **argv) {return 0;}", sName);
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    return rv;
  }
  else if (rv==0) {
    GWEN_BUFFER *dbuf;

    GWB_Parser_SetItemValue(db, sId, "_EXISTS", "TRUE");
    dbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(dbuf, "-l");
    GWEN_Buffer_AppendString(dbuf, sName);
    GWB_Parser_SetItemValue(db, sId, "_LIBS", GWEN_Buffer_GetStart(dbuf));
    GWEN_Buffer_free(dbuf);

    fprintf(stdout, " lib %s: found\n", sName);
  }
  else {
    GWB_Parser_SetItemValue(GWB_Context_GetVars(currentContext), sId, "_EXISTS", "FALSE");
    fprintf(stdout, " lib %s: not found\n", sName);
  }
  return 0;
}



GWEN_BUFFER *_createTestCodeForLib(const char *fnName)
{
  GWEN_BUFFER *codeBuf;

  codeBuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendArgs(codeBuf, "int %s(void);\n", fnName);
  GWEN_Buffer_AppendArgs(codeBuf, "int main(int argc, char **argv) {return %s();}", fnName);
  return codeBuf;
}





