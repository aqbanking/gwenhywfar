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

#include <gwenhywfar/debug.h>
#include <gwenhywfar/directory.h>



static int _parseChildNodes(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);
static int _parseProg(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);
static int _determineProgPath(GWEN_DB_NODE *db, const char *sCmd, const char *sId,
			      const GWEN_STRINGLIST *slAltNames, const GWEN_STRINGLIST *slPaths);
static int _determineNamePath(GWEN_DB_NODE *db, const char *sCmd, const char *sId, const GWEN_STRINGLIST *slPaths);






int GWB_ParseCheckProgs(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
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

      if (strcasecmp(name, "prog")==0)
        rv=_parseProg(project, currentContext, n);
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



int _parseProg(GWEN_UNUSED GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  int rv;
  const char *sCmd;
  const char *sId;
  GWEN_XMLNODE *n;
  GWEN_DB_NODE *db;
  GWEN_STRINGLIST *slAltNames=NULL;
  GWEN_STRINGLIST *slPaths=NULL;


  db=GWB_Context_GetVars(currentContext);

  rv=GWEN_XMLNode_ExpandProperties(xmlNode, db);
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    return rv;
  }

  sCmd=GWEN_XMLNode_GetProperty(xmlNode, "cmd", NULL);
  if (!(sCmd && *sCmd)) {
    DBG_ERROR(NULL, "No cmd in <prog>");
    return GWEN_ERROR_GENERIC;
  }

  sId=GWEN_XMLNode_GetProperty(xmlNode, "id", NULL);
  if (!(sId && *sId)) {
    DBG_ERROR(NULL, "No id in <prog>");
    return GWEN_ERROR_GENERIC;
  }

  n=GWEN_XMLNode_FindFirstTag(xmlNode, "alternativeNames", NULL, NULL);
  if (n)
    slAltNames=GWB_Parser_ReadXmlDataIntoStringList(currentContext, n);

  n=GWEN_XMLNode_FindFirstTag(xmlNode, "paths", NULL, NULL);
  if (n)
    slPaths=GWB_Parser_ReadXmlDataIntoStringList(currentContext, n);
  else {
    slPaths=GWBUILD_GetPathFromEnvironment();
    if (slPaths==NULL)
      slPaths=GWEN_StringList_fromString("/usr/local/bin:/usr/bin:/bin", ": ", 0);
  }

  rv=_determineProgPath(GWB_Context_GetVars(currentContext), sCmd, sId, slAltNames, slPaths);
  if (rv==1) {
    GWB_Parser_SetItemValue(db, sId, "_EXISTS", "TRUE");
    fprintf(stdout, " prog %s: found\n", sCmd);
  }
  else {
    GWB_Parser_SetItemValue(GWB_Context_GetVars(currentContext), sId, "_EXISTS", "FALSE");
    fprintf(stdout, " prog %s: not found\n", sCmd);
  }
  return 0;
}



/* Result <0: ERROR, ==0: Not found, >0; Found */
int _determineProgPath(GWEN_DB_NODE *db, const char *sCmd, const char *sId,
		       const GWEN_STRINGLIST *slAltNames, const GWEN_STRINGLIST *slPaths)
{
  int rv;

  rv=_determineNamePath(db, sCmd, sId, slPaths);
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    return rv;
  }
  else if (rv==1) {
    DBG_DEBUG(NULL, "Found path for prog \"%s\"", sCmd);
    return 1;
  }
  else {
    if (slAltNames) {
      const GWEN_STRINGLISTENTRY *se;
  
      se=GWEN_StringList_FirstEntry(slAltNames);
      while(se) {
	const char *s;
  
	s=GWEN_StringListEntry_Data(se);
	if (s && *s) {
	  rv=_determineNamePath(db, sCmd, sId, slPaths);
	  if (rv<0) {
	    DBG_INFO(NULL, "here (%d)", rv);
	    return rv;
	  }
	  else if (rv>0) {
	    DBG_DEBUG(NULL, "Found path for prog \"%s\"", sCmd);
	    return 1;
	  }
	}
	se=GWEN_StringListEntry_Next(se);
      }
    }
  }

  return 0;
}



/* Result <0: ERROR, ==0: Not found, >0; Found */
int _determineNamePath(GWEN_DB_NODE *db, const char *sCmd, const char *sId, const GWEN_STRINGLIST *slPaths)
{
  GWEN_BUFFER *dbuf;
  int rv;

  dbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_Directory_FindFileInPaths(slPaths, sCmd, dbuf);
  if (rv<0) {
    if (rv==GWEN_ERROR_NOT_FOUND) {
      DBG_DEBUG(NULL, "Command \"%s\" not found", sCmd);
      GWEN_Buffer_free(dbuf);
      return 0;
    }
    else {
      DBG_DEBUG(NULL, "ERROR on GWEN_Directory_FindFileInPaths(\"%s\"): %d", sCmd, rv);
      GWEN_Buffer_free(dbuf);
      return rv;
    }
  }

  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, sId, GWEN_Buffer_GetStart(dbuf));
  GWEN_Buffer_free(dbuf);
  return 1;
}






