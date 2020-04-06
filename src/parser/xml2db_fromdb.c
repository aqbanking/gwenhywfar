/***************************************************************************
    begin       : Thu Apr 02 2020
    copyright   : (C) 2020 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston,                 *
 *   MA  02111-1307  USA                                                   *
 *                                                                         *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



#include "xml2db_p.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/gwendate.h>


#include <ctype.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int _handleChildren_fromDb(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);
static int _dbEnter(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);
static int _dbForEvery(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);
static int _xmlCreateAndEnterElement(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);
static int _xmlSetCharValue(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);
static int _dbIfCharDataMatches(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);
static int _dbIfNotCharDataMatches(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);
static int _dbIfHasCharData(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);
static int _dbIfNotHasCharData(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);
static int _dbIfPathExists(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);
static int _dbIfNotPathExists(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);

static int _setCharValueByPath(GWEN_XMLNODE *xmlNode, const char *path, const char *value);
static int _convertAndSetCharValue(GWEN_XMLNODE *xmlDocNode, GWEN_XMLNODE *xmlSchemaNode, const char *value);





/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */


int GWEN_XmlFromDb(GWEN_XMLNODE *xmlNodeDestination,
                   GWEN_XMLNODE *xmlNodeSchema,
                   GWEN_DB_NODE *dbSource)
{
  GWEN_XML2DB_CONTEXT *ctx;
  int rv;

  ctx=GWEN_Xml2Db_Context_new(xmlNodeDestination, dbSource);
  ctx->handleChildrenFn=_handleChildren_fromDb;
  rv=GWEN_Xml2Db_Context_HandleChildren(ctx, xmlNodeSchema);
  GWEN_Xml2Db_Context_free(ctx);

  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}





int _handleChildren_fromDb(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode)
{
  GWEN_XMLNODE *n;

  n=GWEN_XMLNode_GetFirstTag(xmlNode);
  while (n) {
    const char *name;

    name=GWEN_XMLNode_GetData(n);
    if (name && *name) {
      int rv;

      DBG_INFO(GWEN_LOGDOMAIN, "Handling element \"%s\"", name);
      if (strcasecmp(name, "DbEnter")==0)
        rv=_dbEnter(ctx, n);
      else if (strcasecmp(name, "DbForEvery")==0)
        rv=_dbForEvery(ctx, n);
      else if (strcasecmp(name, "XmlCreateAndEnterElement")==0)
        rv=_xmlCreateAndEnterElement(ctx, n);
      else if (strcasecmp(name, "XmlSetCharValue")==0)
        rv=_xmlSetCharValue(ctx, n);
      else if (strcasecmp(name, "DbIfCharDataMatches")==0)
        rv=_dbIfCharDataMatches(ctx, n);
      else if (strcasecmp(name, "DbIfNotCharDataMatches")==0)
        rv=_dbIfNotCharDataMatches(ctx, n);
      else if (strcasecmp(name, "DbIfHasCharData")==0)
        rv=_dbIfHasCharData(ctx, n);
      else if (strcasecmp(name, "DbIfNotHasCharData")==0)
        rv=_dbIfNotHasCharData(ctx, n);
      else if (strcasecmp(name, "DbIfPathExists")==0)
        rv=_dbIfPathExists(ctx, n);
      else if (strcasecmp(name, "DbIfNotPathExists")==0)
        rv=_dbIfNotPathExists(ctx, n);
      else {
        DBG_ERROR(GWEN_LOGDOMAIN, "Unknown element \"%s\", aborting", name);
        return GWEN_ERROR_INVALID;
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



int _dbIfPathExists(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode)
{
  const char *name;
  GWEN_DB_NODE *dbNew;
  int rv;

  name=GWEN_XMLNode_GetProperty(xmlNode, "name", NULL);
  if (!(name && *name)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Missing or empty name in \"DbIfPathExists\"");
    return GWEN_ERROR_INVALID;
  }

  dbNew=GWEN_DB_GetGroup(ctx->currentDbGroup, GWEN_PATH_FLAGS_NAMEMUSTEXIST, name);
  if (dbNew) {
    /* handle children (nothing special here) */
    rv=GWEN_Xml2Db_Context_HandleChildren(ctx, xmlNode);

    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  return 0;
}



int _dbIfNotPathExists(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode)
{
  const char *name;
  GWEN_DB_NODE *dbNew;
  int rv;

  name=GWEN_XMLNode_GetProperty(xmlNode, "name", NULL);
  if (!(name && *name)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Missing or empty name in \"DbIfPathExists\"");
    return GWEN_ERROR_INVALID;
  }

  dbNew=GWEN_DB_GetGroup(ctx->currentDbGroup, GWEN_PATH_FLAGS_NAMEMUSTEXIST, name);
  if (dbNew==NULL) {
    /* handle children (nothing special here) */
    rv=GWEN_Xml2Db_Context_HandleChildren(ctx, xmlNode);

    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  return 0;
}



int _dbEnter(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode)
{
  const char *name;
  GWEN_DB_NODE *dbLast;
  GWEN_DB_NODE *dbNew;
  int rv;

  name=GWEN_XMLNode_GetProperty(xmlNode, "name", NULL);
  if (!(name && *name)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Missing or empty name in \"DbEnter\"");
    return GWEN_ERROR_INVALID;
  }

  /* save group */
  dbLast=ctx->currentDbGroup;

  dbNew=GWEN_DB_GetGroup(dbLast, GWEN_PATH_FLAGS_NAMEMUSTEXIST, name);
  if (dbNew) {
    /* push group */
    ctx->currentDbGroup=dbNew;

    /* handle children (nothing special here) */
    rv=GWEN_Xml2Db_Context_HandleChildren(ctx, xmlNode);

    /* pop group */
    ctx->currentDbGroup=dbLast;

    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Group \"%s\" does not exist", name);
    return GWEN_ERROR_INVALID;
  }

  return 0;
}



int _dbForEvery(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode)
{
  const char *name;
  GWEN_DB_NODE *dbLast;
  GWEN_DB_NODE *dbNew;
  int rv;

  name=GWEN_XMLNode_GetProperty(xmlNode, "name", NULL);
  if (!(name && *name)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Missing or empty name in \"DbEnter\"");
    return GWEN_ERROR_INVALID;
  }

  /* save group */
  dbLast=ctx->currentDbGroup;

  dbNew=GWEN_DB_FindFirstGroup(dbLast, name);
  while (dbNew) {
    /* push group */
    ctx->currentDbGroup=dbNew;

    /* handle children (nothing special here) */
    rv=GWEN_Xml2Db_Context_HandleChildren(ctx, xmlNode);

    /* pop group */
    ctx->currentDbGroup=dbLast;

    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    dbNew=GWEN_DB_FindNextGroup(dbNew, name);
  }

  return 0;
}



int _dbIfCharDataMatches(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode)
{
  const char *name;
  const char *pattern;
  const char *val;

  name=GWEN_XMLNode_GetProperty(xmlNode, "name", NULL);
  if (!(name && *name)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Missing or empty name in \"dbIfCharDataMatches\"");
    return GWEN_ERROR_INVALID;
  }

  pattern=GWEN_XMLNode_GetProperty(xmlNode, "pattern", NULL);
  if (!(pattern && *pattern)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Missing or empty pattern in \"dbIfCharDataMatches\"");
    return GWEN_ERROR_INVALID;
  }

  val=GWEN_DB_GetCharValue(ctx->currentDbGroup, name, 0, NULL);
  if (val && -1!=GWEN_Text_ComparePattern(val, pattern, 0)) {
    int rv;

    /* pattern matches, handle children  */
    rv=GWEN_Xml2Db_Context_HandleChildren(ctx, xmlNode);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  return 0;
}



int _dbIfNotCharDataMatches(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode)
{
  const char *name;
  const char *pattern;
  const char *val;

  name=GWEN_XMLNode_GetProperty(xmlNode, "name", NULL);
  if (!(name && *name)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Missing or empty name in \"dbIfCharDataMatches\"");
    return GWEN_ERROR_INVALID;
  }

  pattern=GWEN_XMLNode_GetProperty(xmlNode, "pattern", NULL);
  if (!(pattern && *pattern)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Missing or empty pattern in \"dbIfCharDataMatches\"");
    return GWEN_ERROR_INVALID;
  }

  val=GWEN_DB_GetCharValue(ctx->currentDbGroup, name, 0, NULL);
  if (val==NULL || -1==GWEN_Text_ComparePattern(val, pattern, 0)) {
    int rv;

    /* pattern matches, handle children  */
    rv=GWEN_Xml2Db_Context_HandleChildren(ctx, xmlNode);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  return 0;
}



int _dbIfHasCharData(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode)
{
  const char *name;
  const char *val;

  name=GWEN_XMLNode_GetProperty(xmlNode, "name", NULL);
  if (!(name && *name)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Missing or empty name in \"dbIfCharDataMatches\"");
    return GWEN_ERROR_INVALID;
  }

  val=GWEN_DB_GetCharValue(ctx->currentDbGroup, name, 0, NULL);
  if (val) {
    int rv;

    /* pattern matches, handle children  */
    rv=GWEN_Xml2Db_Context_HandleChildren(ctx, xmlNode);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  return 0;
}



int _dbIfNotHasCharData(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode)
{
  const char *name;
  const char *val;

  name=GWEN_XMLNode_GetProperty(xmlNode, "name", NULL);
  if (!(name && *name)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Missing or empty name in \"dbIfCharDataMatches\"");
    return GWEN_ERROR_INVALID;
  }

  val=GWEN_DB_GetCharValue(ctx->currentDbGroup, name, 0, NULL);
  if (val==NULL) {
    int rv;

    /* pattern matches, handle children  */
    rv=GWEN_Xml2Db_Context_HandleChildren(ctx, xmlNode);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  return 0;
}



int _xmlCreateAndEnterElement(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode)
{
  const char *path;
  GWEN_XMLNODE *n;
  int rv;

  path=GWEN_XMLNode_GetProperty(xmlNode, "path", NULL);
  if (path==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Missing path in \"XmlCreateAndEnterElement\"");
    return GWEN_ERROR_INVALID;
  }

  n=GWEN_XMLNode_GetNodeByXPath(ctx->currentDocNode, path, GWEN_PATH_FLAGS_CREATE_GROUP | GWEN_PATH_FLAGS_CREATE_VAR);
  if (n==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Path \"%s\" does not exist", path);
    return GWEN_ERROR_INVALID;
  }

  /* enter given document node */
  GWEN_Xml2Db_Context_EnterDocNode(ctx, n);

  rv=GWEN_Xml2Db_Context_HandleChildren(ctx, xmlNode);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* leave given document node, re-select previously active one, thus restoring status from the beginning */
  GWEN_Xml2Db_Context_LeaveDocNode(ctx);
  return 0;
}



int _xmlSetCharValue(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode)
{
  const char *path;
  const char *value;

  path=GWEN_XMLNode_GetProperty(xmlNode, "path", NULL);
  if (!(path && *path)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Missing or empty path in \"XmlSetCharValue\"");
    return GWEN_ERROR_INVALID;
  }

  value=GWEN_XMLNode_GetProperty(xmlNode, "value", NULL);
  if (value) {
    GWEN_BUFFER *dbuf;
    int rv;

    dbuf=GWEN_Buffer_new(0, 256, 0, 1);
    rv=GWEN_DB_ReplaceVars(ctx->currentDbGroup, value, dbuf);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(dbuf);
      return rv;
    }
    _convertAndSetCharValue(ctx->currentDocNode, xmlNode, GWEN_Buffer_GetStart(dbuf));
    GWEN_Buffer_free(dbuf);
  }
  else {
    const char *name;

    name=GWEN_XMLNode_GetProperty(xmlNode, "name", NULL);
    if (!(name && *name)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Missing or name path in \"XmlSetCharValue\"");
      return GWEN_ERROR_INVALID;
    }

    value=GWEN_DB_GetCharValue(ctx->currentDbGroup, name, 0, NULL);
    if (value && *value) {
      _convertAndSetCharValue(ctx->currentDocNode, xmlNode, value);
    }
#if 0
    else {
      GWEN_BUFFER *tbuf;

      tbuf=GWEN_Buffer_new(0, 256, 0, 1);

      GWEN_XMLNode_GetXPath(NULL, ctx->currentDocNode, tbuf);

      DBG_ERROR(GWEN_LOGDOMAIN, "No value in path \"%s\" (%s)", path, GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_free(tbuf);

      /* GWEN_XMLNode_Dump(ctx->currentDocNode, 2); */
      return GWEN_ERROR_INVALID;
    }
#endif
  }

  return 0;
}



int _setCharValueByPath(GWEN_XMLNODE *xmlNode, const char *path, const char *value)
{
  const char *s;

  s=strchr(path, '@');
  if (s) {
    int idx;
    char *cpyOfPath;
    char *property;
    GWEN_XMLNODE *n;


    idx=s-path;
    cpyOfPath=strdup(path);
    assert(cpyOfPath);
    cpyOfPath[idx]=0;
    property=cpyOfPath+idx+1;

    if (*cpyOfPath) {
      n=GWEN_XMLNode_GetNodeByXPath(xmlNode, cpyOfPath, 0);
    }
    else
      n=xmlNode;

    if (n) {
      GWEN_XMLNode_SetProperty(n, property, value);
      free(cpyOfPath);
      return 0;
    }
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "Path \"%s\" could not be created", path);
      free(cpyOfPath);
      return GWEN_ERROR_GENERIC;
    }
  }
  else {
    int rv;

    rv=GWEN_XMLNode_SetCharValueByPath(xmlNode, 0, path, value);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    return 0;
  }
}



int _convertAndSetCharValue(GWEN_XMLNODE *xmlDocNode, GWEN_XMLNODE *xmlSchemaNode, const char *value)
{
  if (value && *value) {
    const char *name;
    const char *typ;
    int doTrim=0;
    GWEN_BUFFER *resultBuf;
    int rv;

    doTrim=GWEN_XMLNode_GetIntProperty(xmlSchemaNode, "trim", 0);
    resultBuf=GWEN_Buffer_new(0, 256, 0, 1);

    name=GWEN_XMLNode_GetProperty(xmlSchemaNode, "path", NULL);
    if (!(name && *name)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Missing or empty path in \"XmlSetCharValue\"");
      GWEN_Buffer_free(resultBuf);
      return GWEN_ERROR_INVALID;
    }

    typ=GWEN_XMLNode_GetProperty(xmlSchemaNode, "type", "string");
    if (!(typ && *typ)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Empty type in \"XmlSetCharValue\"");
      GWEN_Buffer_free(resultBuf);
      return GWEN_ERROR_INVALID;
    }

    if (strcasecmp(typ, "string")==0) {
      /* escape */
      GWEN_Text_EscapeXmlToBuffer(value, resultBuf);
      if (doTrim)
        GWEN_Text_CondenseBuffer(resultBuf);
    }
    else if (strcasecmp(typ, "date")==0) {
      const char *tmpl;
      GWEN_DATE *dt=NULL;

      tmpl=GWEN_XMLNode_GetProperty(xmlSchemaNode, "template", "YYYYMMDD");
      if (!(tmpl && *tmpl)) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Empty template in \"SetCharValue\"");
        GWEN_Buffer_free(resultBuf);
        return GWEN_ERROR_INVALID;
      }

      dt=GWEN_Date_fromString(value);
      if (dt) {
        GWEN_Date_toStringWithTemplate(dt, tmpl, resultBuf);
        GWEN_Date_free(dt);
      }
      else {
        DBG_ERROR(GWEN_LOGDOMAIN, "Invalid date string for [%s] (value=[%s], tmpl=[%s])", name, value, tmpl);
      }
    }

    if (GWEN_Buffer_GetUsedBytes(resultBuf)) {
      DBG_INFO(GWEN_LOGDOMAIN, "Setting value: %s = %s", name, GWEN_Buffer_GetStart(resultBuf));

      rv=_setCharValueByPath(xmlDocNode, name, GWEN_Buffer_GetStart(resultBuf));
      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        GWEN_Buffer_free(resultBuf);
        return rv;
      }
    }
    else {
      DBG_INFO(GWEN_LOGDOMAIN, "Not setting empty value for [%s]", name);
    }
    GWEN_Buffer_free(resultBuf);
  }
  return 0;
}



