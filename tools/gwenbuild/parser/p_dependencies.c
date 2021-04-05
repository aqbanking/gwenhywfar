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


#include "gwenbuild/parser/p_dependencies.h"
#include "gwenbuild/parser/parser.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/process.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/syncio.h>

#include <unistd.h>
#include <ctype.h>



static int _parseChildNodes(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);
int _parseDep(GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);

static int _checkVersion(GWEN_DB_NODE *db,
                         const char *sId,
                         const char *sName,
                         const char *sMinVersion,
                         const char *sMaxVersion);
static int _retrieveCflags(GWEN_DB_NODE *db, const char *sId, const char *sName);
static int _retrieveLdflags(GWEN_DB_NODE *db, const char *sId, const char *sName);
static int _callPkgConfig(GWEN_DB_NODE *db,
                          const char *sId,
                          const char *sName,
                          const char *suffix,
                          const char *args);

void _replaceControlCharsWithBlanks(char *ptr);
int _retrieveVariables(GWB_CONTEXT *currentContext, GWEN_DB_NODE *db, const char *sId, const char *sName, GWEN_XMLNODE *xmlNode);
int _retrieveVariable(GWEN_DB_NODE *db, const char *sId, const char *sName, const char *variableName);






int GWB_ParseDependencies(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
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

      if (strcasecmp(name, "subdirs")==0)
        rv=GWB_Parser_ParseSubdirs(project, currentContext, n, _parseChildNodes);
      else if (strcasecmp(name, "dep")==0)
        rv=_parseDep(currentContext, n);
      else {
        DBG_DEBUG(NULL, "Element not handled");
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



int _parseDep(GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  int rv;
  const char *sId;
  const char *sName;
  const char *sMinVersion;
  const char *sMaxVersion;

  rv=GWEN_XMLNode_ExpandProperties(xmlNode, GWB_Context_GetVars(currentContext));
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

  sMinVersion=GWEN_XMLNode_GetProperty(xmlNode, "minversion", NULL);
  sMaxVersion=GWEN_XMLNode_GetProperty(xmlNode, "maxversion", NULL);

  rv=_checkVersion(GWB_Context_GetVars(currentContext),
                   sId,
                   sName,
                   sMinVersion,
                   sMaxVersion);
  if (rv==0) {
    GWEN_XMLNODE *n;

    rv=_retrieveCflags(GWB_Context_GetVars(currentContext), sId, sName);
    if (rv<0) {
      DBG_INFO(NULL, "here (%d)", rv);
      return rv;
    }
    rv=_retrieveLdflags(GWB_Context_GetVars(currentContext), sId, sName);
    if (rv<0) {
      DBG_INFO(NULL, "here (%d)", rv);
      return rv;
    }

    n=GWEN_XMLNode_FindFirstTag(xmlNode, "variables", NULL, NULL);
    if (n) {
      rv=_retrieveVariables(currentContext, GWB_Context_GetVars(currentContext), sId, sName, n);
      if (rv<0) {
        DBG_INFO(NULL, "here (%d)", rv);
        return rv;
      }
    }
  }
  else if (rv!=GWEN_ERROR_NOT_FOUND) {
    DBG_INFO(NULL, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int _retrieveVariables(GWB_CONTEXT *currentContext, GWEN_DB_NODE *db, const char *sId, const char *sName, GWEN_XMLNODE *xmlNode)
{
  GWEN_STRINGLIST *slVariables;

  slVariables=GWB_Parser_ReadXmlDataIntoStringList(currentContext, xmlNode);
  if (slVariables) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(slVariables);
    while(se) {
      const char *s;

      s=GWEN_StringListEntry_Data(se);
      if (s && *s) {
        int rv;

        rv=_retrieveVariable(db, sId, sName, s);
        if (rv<0) {
          DBG_ERROR(NULL, "Error retrieving variable \"%s\" for dependency \"%s\"", s, sId);
          GWEN_StringList_free(slVariables);
          return rv;
        }
      }
      se=GWEN_StringListEntry_Next(se);
    }
  }

  return 0;
}



int _checkVersion(GWEN_DB_NODE *db,
                  const char *sId,
                  const char *sName,
                  const char *sMinVersion,
                  const char *sMaxVersion)
{
  GWEN_BUFFER *argBuffer;
  GWEN_BUFFER *stdOutBuffer;
  GWEN_BUFFER *stdErrBuffer;
  int rv;

  stdOutBuffer=GWEN_Buffer_new(0, 256, 0, 1);
  stdErrBuffer=GWEN_Buffer_new(0, 256, 0, 1);
  argBuffer=GWEN_Buffer_new(0, 256, 0, 1);

  if (sMinVersion) {
    GWEN_Buffer_AppendString(argBuffer, " --atleast-version=");
    GWEN_Buffer_AppendString(argBuffer, sMinVersion);
    GWEN_Buffer_AppendString(argBuffer, " ");
  }
  if (sMaxVersion) {
    GWEN_Buffer_AppendString(argBuffer, " --max-version=");
    GWEN_Buffer_AppendString(argBuffer, sMaxVersion);
    GWEN_Buffer_AppendString(argBuffer, " ");
  }
  GWEN_Buffer_AppendString(argBuffer, sName);

  DBG_DEBUG(NULL, "Running command with args: [%s]", GWEN_Buffer_GetStart(argBuffer));
  rv=GWEN_Process_RunCommandWaitAndGather("pkg-config", GWEN_Buffer_GetStart(argBuffer),
                                          stdOutBuffer, stdErrBuffer);
  if (rv<0) {
    DBG_ERROR(NULL, "Error running pkg-config (%d)", rv);
    GWEN_Buffer_free(argBuffer);
    GWEN_Buffer_free(stdErrBuffer);
    GWEN_Buffer_free(stdOutBuffer);
    return rv;
  }
  GWEN_Buffer_free(argBuffer);
  GWEN_Buffer_free(stdErrBuffer);
  GWEN_Buffer_free(stdOutBuffer);

  GWB_Parser_SetItemValue(db, sId, "_EXISTS", (rv==0)?"TRUE":"FALSE");
  fprintf(stdout, " dep %s: %s\n", sId, (rv==0)?"found":"not found");

  return (rv==0)?0:GWEN_ERROR_NOT_FOUND;
}



int _retrieveCflags(GWEN_DB_NODE *db, const char *sId, const char *sName)
{
  int rv;

  rv=_callPkgConfig(db, sId, sName, "_CFLAGS", "--cflags");
  if (rv<0) {
    DBG_ERROR(NULL, "Error running pkg-config (%d)", rv);
    return rv;
  }

  return 0;
}



int _retrieveLdflags(GWEN_DB_NODE *db, const char *sId, const char *sName)
{
  int rv;

  rv=_callPkgConfig(db, sId, sName, "_LIBS", "--libs");
  if (rv<0) {
    DBG_ERROR(NULL, "Error running pkg-config (%d)", rv);
    return rv;
  }

  return 0;
}



int _retrieveVariable(GWEN_DB_NODE *db, const char *sId, const char *sName, const char *variableName)
{
  int rv;
  GWEN_BUFFER *varArgBuffer;
  GWEN_BUFFER *varSuffixBuffer;
  const char *s;

  varArgBuffer=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(varArgBuffer, "--variable=");
  GWEN_Buffer_AppendString(varArgBuffer, variableName);

  varSuffixBuffer=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(varSuffixBuffer, "_");
  s=variableName;
  while(*s)
    GWEN_Buffer_AppendByte(varSuffixBuffer, toupper(*(s++)));

  rv=_callPkgConfig(db, sId, sName, GWEN_Buffer_GetStart(varSuffixBuffer), GWEN_Buffer_GetStart(varArgBuffer));
  GWEN_Buffer_free(varSuffixBuffer);
  GWEN_Buffer_free(varArgBuffer);
  if (rv<0) {
    DBG_ERROR(NULL, "Error running pkg-config (%d)", rv);
    return rv;
  }

  return 0;
}



int _callPkgConfig(GWEN_DB_NODE *db,
                   const char *sId,
                   const char *sName,
                   const char *suffix,
                   const char *args)
{
  GWEN_BUFFER *argBuffer;
  GWEN_BUFFER *stdOutBuffer;
  GWEN_BUFFER *stdErrBuffer;
  int rv;

  stdOutBuffer=GWEN_Buffer_new(0, 256, 0, 1);
  stdErrBuffer=GWEN_Buffer_new(0, 256, 0, 1);
  argBuffer=GWEN_Buffer_new(0, 256, 0, 1);

  GWEN_Buffer_AppendString(argBuffer, args);
  GWEN_Buffer_AppendString(argBuffer, " ");
  GWEN_Buffer_AppendString(argBuffer, sName);

  DBG_DEBUG(NULL, "Running command with args: [%s]", GWEN_Buffer_GetStart(argBuffer));
  rv=GWEN_Process_RunCommandWaitAndGather("pkg-config", GWEN_Buffer_GetStart(argBuffer), stdOutBuffer, stdErrBuffer);
  if (rv<0) {
    DBG_ERROR(NULL, "Error running pkg-config (%d)", rv);
    GWEN_Buffer_free(argBuffer);
    GWEN_Buffer_free(stdErrBuffer);
    GWEN_Buffer_free(stdOutBuffer);
    return rv;
  }
  GWEN_Buffer_free(argBuffer);

  _replaceControlCharsWithBlanks(GWEN_Buffer_GetStart(stdOutBuffer));
  GWEN_Text_CondenseBuffer(stdOutBuffer);
  GWB_Parser_SetItemValue(db, sId, suffix, GWEN_Buffer_GetStart(stdOutBuffer));

  GWEN_Buffer_free(stdErrBuffer);
  GWEN_Buffer_free(stdOutBuffer);
  return 0;
}



void _replaceControlCharsWithBlanks(char *ptr)
{
  while(ptr && *ptr) {
    if (*ptr<32)
      *ptr=32;
    ptr++;
  }
}



