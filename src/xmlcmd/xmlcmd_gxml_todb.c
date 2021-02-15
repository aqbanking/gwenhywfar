/***************************************************************************
    begin       : Sat Apr 18 2018
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



#include "xmlcmd_gxml_todb.h"
#include "xmlcmd_gxml.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/gwendate.h>


#include <ctype.h>




/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int _handleChildren_toDb(GWEN_XMLCOMMANDER *cmd, GWEN_XMLNODE *xmlNode);

static const char *_getCharValueByPath(GWEN_XMLNODE *xmlNode, const char *path, const char *defValue);
static int _convertAndSetCharValue(GWEN_XMLCOMMANDER *cmd, GWEN_XMLNODE *xmlNode, GWEN_DB_NODE *dbCurrent,
                                   const char *value);

static int _handleDbSetCharValue_internal(GWEN_XMLCOMMANDER *cmd, GWEN_XMLNODE *xmlNode, GWEN_DB_NODE *dbCurrent);


static int _handleXmlEnter(GWEN_XMLCOMMANDER *cmd, GWEN_XMLNODE *xmlNode);
static int _handleXmlForEvery(GWEN_XMLCOMMANDER *cmd, GWEN_XMLNODE *xmlNode);
static int _handleDbCreateAndEnterGroup(GWEN_XMLCOMMANDER *cmd, GWEN_XMLNODE *xmlNode);
static int _handleDbCreateAndEnterTempGroup(GWEN_XMLCOMMANDER *cmd, GWEN_XMLNODE *xmlNode);
static int _handleDbSetCharValue(GWEN_XMLCOMMANDER *cmd, GWEN_XMLNODE *xmlNode);
static int _handleDbSetTempCharValue(GWEN_XMLCOMMANDER *cmd, GWEN_XMLNODE *xmlNode);
static int _handleXmlIfCharDataMatches(GWEN_XMLCOMMANDER *cmd, GWEN_XMLNODE *xmlNode);
static int _handleXmlIfNotCharDataMatches(GWEN_XMLCOMMANDER *cmd, GWEN_XMLNODE *xmlNode);
static int _handleXmlIfHasCharData(GWEN_XMLCOMMANDER *cmd, GWEN_XMLNODE *xmlNode);
static int _handleXmlIfNotHasCharData(GWEN_XMLCOMMANDER *cmd, GWEN_XMLNODE *xmlNode);
static int _handleXmlIfPathExists(GWEN_XMLCOMMANDER *cmd, GWEN_XMLNODE *xmlNode);
static int _handleXmlIfNotPathExists(GWEN_XMLCOMMANDER *cmd, GWEN_XMLNODE *xmlNode);





/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



GWEN_XMLCOMMANDER *GWEN_XmlCommanderGwenXml_toDb_new(GWEN_XMLNODE *xmlNodeDocument,
                                                     GWEN_DB_NODE *dbDestination)
{
  GWEN_XMLCOMMANDER *cmd;

  cmd=GWEN_XmlCommanderGwenXml_new(xmlNodeDocument, dbDestination);
  GWEN_XmlCommander_SetHandleChildrenFn(cmd, _handleChildren_toDb);

  return cmd;
}



int _handleChildren_toDb(GWEN_XMLCOMMANDER *cmd, GWEN_XMLNODE *xmlNode)
{
  GWEN_XMLNODE *n;

  n=GWEN_XMLNode_GetFirstTag(xmlNode);
  while (n) {
    const char *name;

    name=GWEN_XMLNode_GetData(n);
    if (name && *name) {
      int rv;

      DBG_INFO(GWEN_LOGDOMAIN, "Handling element \"%s\"", name);
      if (strcasecmp(name, "XmlEnter")==0)
        rv=_handleXmlEnter(cmd, n);
      else if (strcasecmp(name, "XmlForEvery")==0)
        rv=_handleXmlForEvery(cmd, n);
      else if (strcasecmp(name, "DbCreateAndEnterGroup")==0)
        rv=_handleDbCreateAndEnterGroup(cmd, n);
      else if (strcasecmp(name, "DbCreateAndEnterTempGroup")==0)
        rv=_handleDbCreateAndEnterTempGroup(cmd, n);
      else if (strcasecmp(name, "DbSetCharValue")==0)
        rv=_handleDbSetCharValue(cmd, n);
      else if (strcasecmp(name, "DbSetTempCharValue")==0)
        rv=_handleDbSetTempCharValue(cmd, n);
      else if (strcasecmp(name, "XmlIfCharDataMatches")==0)
        rv=_handleXmlIfCharDataMatches(cmd, n);
      else if (strcasecmp(name, "XmlIfNotCharDataMatches")==0)
        rv=_handleXmlIfNotCharDataMatches(cmd, n);
      else if (strcasecmp(name, "XmlIfHasCharData")==0)
        rv=_handleXmlIfHasCharData(cmd, n);
      else if (strcasecmp(name, "XmlIfNotHasCharData")==0)
        rv=_handleXmlIfNotHasCharData(cmd, n);
      else if (strcasecmp(name, "XmlIfPathExists")==0)
        rv=_handleXmlIfPathExists(cmd, n);
      else if (strcasecmp(name, "XmlIfNotPathExists")==0)
        rv=_handleXmlIfNotPathExists(cmd, n);
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




const char *_getCharValueByPath(GWEN_XMLNODE *xmlNode, const char *path, const char *defValue)
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
      n=GWEN_XMLNode_GetNodeByXPath(xmlNode, cpyOfPath, GWEN_PATH_FLAGS_PATHMUSTEXIST);
    }
    else
      n=xmlNode;

    if (n) {
      const char *result;

      result=GWEN_XMLNode_GetProperty(n, property, defValue);
      DBG_INFO(GWEN_LOGDOMAIN, "Got XML property: %s = %s (%s)", property, result, path);
      free(cpyOfPath);
      return result;
    }
    free(cpyOfPath);
    return defValue;
  }
  else
    return GWEN_XMLNode_GetCharValueByPath(xmlNode, path, defValue);
}


/* TODO: optimize later */
int _convertAndSetCharValue(GWEN_UNUSED GWEN_XMLCOMMANDER *cmd, GWEN_XMLNODE *xmlNode,
                            GWEN_DB_NODE *dbCurrent,
                            const char *value)
{
  if (value && *value) {
    const char *name;
    const char *typ;
    const char *mode;
    int doTrim=0;
    GWEN_BUFFER *vbuf;
    GWEN_BUFFER *resultBuf;

    doTrim=GWEN_XMLNode_GetIntProperty(xmlNode, "trim", 0);
    vbuf=GWEN_Buffer_new(0, 256, 0, 1);
    resultBuf=GWEN_Buffer_new(0, 256, 0, 1);

    name=GWEN_XMLNode_GetProperty(xmlNode, "name", NULL);
    if (!(name && *name)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Missing or empty name in \"SetCharValue\"");
      GWEN_Buffer_free(resultBuf);
      GWEN_Buffer_free(vbuf);
      return GWEN_ERROR_INVALID;
    }

    typ=GWEN_XMLNode_GetProperty(xmlNode, "type", "string");
    if (!(typ && *typ)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Empty type in \"SetCharValue\"");
      GWEN_Buffer_free(resultBuf);
      GWEN_Buffer_free(vbuf);
      return GWEN_ERROR_INVALID;
    }

    mode=GWEN_XMLNode_GetProperty(xmlNode, "mode", "add");

    if (strcasecmp(typ, "string")==0) {
      GWEN_Buffer_AppendString(vbuf, value);
      if (doTrim)
        GWEN_Text_CondenseBuffer(vbuf);
    }
    else if (strcasecmp(typ, "date")==0) {
      const char *tmpl;
      GWEN_DATE *dt=NULL;

      tmpl=GWEN_XMLNode_GetProperty(xmlNode, "template", "YYYYMMDD");
      if (!(tmpl && *tmpl)) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Empty date template in \"SetCharValue\"");
        GWEN_Buffer_free(resultBuf);
        GWEN_Buffer_free(vbuf);
        return GWEN_ERROR_INVALID;
      }

      dt=GWEN_Date_fromStringWithTemplate(value, tmpl);
      if (dt) {
        GWEN_Buffer_AppendString(vbuf, GWEN_Date_GetString(dt));
        GWEN_Date_free(dt);
      }
    }
    else if (strcasecmp(typ, "time")==0) {
      const char *tmpl;
      GWEN_TIME *ti=NULL;

      tmpl=GWEN_XMLNode_GetProperty(xmlNode, "template", "YYYYMMDDhhmmss");
      if (!(tmpl && *tmpl)) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Empty time template in \"SetCharValue\"");
        GWEN_Buffer_free(resultBuf);
        GWEN_Buffer_free(vbuf);
        return GWEN_ERROR_INVALID;
      }

      ti=GWEN_Time_fromString(value, tmpl);
      if (ti) {
	GWEN_Time_toString(ti, "YYYYMMDDhhmmss", vbuf);
        GWEN_Time_free(ti);
      }
      else {
	DBG_INFO(GWEN_LOGDOMAIN, "Invalid timespec \"%s\" according to template \"%s\"",
		 value?value:"<empty>", tmpl);
	return GWEN_ERROR_INVALID;
      }
    }

    if (strcasecmp(mode, "add")==0) {
      /* just exchange the buffer */
      GWEN_Buffer_free(resultBuf);
      resultBuf=vbuf;
      vbuf=NULL;
    }
    else if (strcasecmp(mode, "append")==0) {
      const char *s;

      s=GWEN_DB_GetCharValue(dbCurrent, name, 0, NULL);
      if (s && *s) {
        const char *delimiter;

        /* write previous data into resultBuffer */
        GWEN_Buffer_AppendString(resultBuf, s);

        /* possibly write delimiter into resultBuffer */
        delimiter=GWEN_XMLNode_GetProperty(xmlNode, "delimiter", NULL);
        if (delimiter && *delimiter) {
          if (strcasecmp(delimiter, "\\n")==0)
            GWEN_Buffer_AppendByte(resultBuf, '\n');
          else if (strcasecmp(delimiter, "\\t")==0)
            GWEN_Buffer_AppendByte(resultBuf, '\t');
          else
            GWEN_Buffer_AppendString(resultBuf, delimiter);
        }
      } /* if previous value */
      /* write value into resultBuffer */
      GWEN_Buffer_AppendString(resultBuf, GWEN_Buffer_GetStart(vbuf));

      GWEN_DB_DeleteVar(dbCurrent, name);
    }
    else if (strcasecmp(mode, "replace")==0) {
      /* just exchange the buffer */
      GWEN_Buffer_free(resultBuf);
      resultBuf=vbuf;
      vbuf=NULL;
      GWEN_DB_DeleteVar(dbCurrent, name);
    }

    DBG_INFO(GWEN_LOGDOMAIN, "Setting value: %s = %s", name, GWEN_Buffer_GetStart(resultBuf));

    GWEN_DB_SetCharValue(dbCurrent, GWEN_DB_FLAGS_DEFAULT, name, GWEN_Buffer_GetStart(resultBuf));
    GWEN_Buffer_free(resultBuf);
    GWEN_Buffer_free(vbuf);
  }
  return 0;
}







int _handleXmlEnter(GWEN_XMLCOMMANDER *cmd, GWEN_XMLNODE *xmlNode)
{
  const char *path;
  GWEN_XMLNODE *n;
  int rv;

  path=GWEN_XMLNode_GetProperty(xmlNode, "path", NULL);
  if (path==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Missing path in \"EnterPath\"");
    return GWEN_ERROR_INVALID;
  }

  n=GWEN_XMLNode_GetNodeByXPath(GWEN_XmlCommanderGwenXml_GetCurrentDocNode(cmd), path, GWEN_PATH_FLAGS_PATHMUSTEXIST);
  if (n==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "XmlEnter: Path \"%s\" does not exist", path);
    return GWEN_ERROR_INVALID;
  }

  /* enter given document node */
  GWEN_XmlCommanderGwenXml_EnterDocNode(cmd, n);

  rv=GWEN_XmlCommander_HandleChildren(cmd, xmlNode);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* leave given document node, re-select previously active one, thus restoring status from the beginning */
  GWEN_XmlCommanderGwenXml_LeaveDocNode(cmd);
  return 0;
}



int _handleXmlForEvery(GWEN_XMLCOMMANDER *cmd, GWEN_XMLNODE *xmlNode)
{
  const char *path;
  GWEN_XMLNODE *n;
  int rv;

  path=GWEN_XMLNode_GetProperty(xmlNode, "name", NULL);
  if (path==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Missing name in \"ForEvery\"");
    return GWEN_ERROR_INVALID;
  }

  n=GWEN_XMLNode_FindFirstTag(GWEN_XmlCommanderGwenXml_GetCurrentDocNode(cmd), path, NULL, NULL);
  if (n==NULL) {
    DBG_INFO(GWEN_LOGDOMAIN, "XmlForEvery: Path \"%s\" not found, not entering", path);
    /* GWEN_XMLNode_Dump(cmd->currentDocNode, 2); */
  }
  while (n) {
    DBG_INFO(GWEN_LOGDOMAIN, "XmlForEvery: Entering path \"%s\"", path);

    /* enter given document node */
    GWEN_XmlCommanderGwenXml_EnterDocNode(cmd, n);

    /* handle all children of this parser XML node with the current document node */
    rv=GWEN_XmlCommander_HandleChildren(cmd, xmlNode);

    /* leave given document node, re-select previously active one, thus restoring status from the beginning */
    GWEN_XmlCommanderGwenXml_LeaveDocNode(cmd);

    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    n=GWEN_XMLNode_FindNextTag(n, path, NULL, NULL);
  }

  return 0;
}



int _handleDbCreateAndEnterGroup(GWEN_XMLCOMMANDER *cmd, GWEN_XMLNODE *xmlNode)
{
  const char *name;
  GWEN_DB_NODE *dbLast;
  int rv;

  name=GWEN_XMLNode_GetProperty(xmlNode, "name", NULL);
  if (!(name && *name)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Missing or empty name in \"CreateAnEnterDbGroup\"");
    return GWEN_ERROR_INVALID;
  }

  /* push group */
  dbLast=GWEN_XmlCommanderGwenXml_GetCurrentDbGroup(cmd);

  /* create group */
  GWEN_XmlCommanderGwenXml_SetCurrentDbGroup(cmd, GWEN_DB_GetGroup(dbLast, GWEN_PATH_FLAGS_CREATE_GROUP, name));

  /* handle children (nothing special here) */
  rv=GWEN_XmlCommander_HandleChildren(cmd, xmlNode);

  /* pop group */
  GWEN_XmlCommanderGwenXml_SetCurrentDbGroup(cmd, dbLast);

  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int _handleDbCreateAndEnterTempGroup(GWEN_XMLCOMMANDER *cmd, GWEN_XMLNODE *xmlNode)
{
  const char *name;
  GWEN_DB_NODE *dbLast;
  GWEN_DB_NODE *dbCurrent;
  int rv;

  name=GWEN_XMLNode_GetProperty(xmlNode, "name", NULL);
  if (!(name && *name)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Missing or empty name in \"CreateAnEnterTempDbGroup\"");
    return GWEN_ERROR_INVALID;
  }

  /* push group */
  dbLast=GWEN_XmlCommanderGwenXml_GetCurrentTempDbGroup(cmd);

  /* create group */
  dbCurrent=GWEN_DB_GetGroup(dbLast, GWEN_PATH_FLAGS_CREATE_GROUP, name);
  GWEN_XmlCommanderGwenXml_SetCurrentTempDbGroup(cmd, dbCurrent);

  /* handle children (nothing special here) */
  rv=GWEN_XmlCommander_HandleChildren(cmd, xmlNode);

  /* delete temp group */
  GWEN_DB_UnlinkGroup(dbCurrent);
  GWEN_DB_Group_free(dbCurrent);

  /* pop group */
  GWEN_XmlCommanderGwenXml_SetCurrentTempDbGroup(cmd, dbLast);

  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int _handleDbSetCharValue_internal(GWEN_XMLCOMMANDER *cmd, GWEN_XMLNODE *xmlNode,
                                   GWEN_DB_NODE *dbCurrent)
{
  const char *name;
  const char *value;

  name=GWEN_XMLNode_GetProperty(xmlNode, "name", NULL);
  if (!(name && *name)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Missing or empty name in \"SetCharValue\"");
    return GWEN_ERROR_INVALID;
  }

  value=GWEN_XMLNode_GetProperty(xmlNode, "value", NULL);
  if (value) {
    GWEN_BUFFER *dbuf;
    int rv;

    dbuf=GWEN_Buffer_new(0, 256, 0, 1);
    rv=GWEN_DB_ReplaceVars(GWEN_XmlCommanderGwenXml_GetCurrentTempDbGroup(cmd), value, dbuf);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(dbuf);
      return rv;
    }
    _convertAndSetCharValue(cmd, xmlNode, dbCurrent, GWEN_Buffer_GetStart(dbuf));
    GWEN_Buffer_free(dbuf);
  }
  else {
    const char *path;

    path=GWEN_XMLNode_GetProperty(xmlNode, "path", NULL);
    if (!(path && *path)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Missing or empty path in \"SetCharValue\"");
      return GWEN_ERROR_INVALID;
    }

    value=_getCharValueByPath(GWEN_XmlCommanderGwenXml_GetCurrentDocNode(cmd), path, NULL);
    if (value && *value) {
      _convertAndSetCharValue(cmd, xmlNode, dbCurrent, value);
    }
#if 0
    else {
      GWEN_BUFFER *tbuf;

      tbuf=GWEN_Buffer_new(0, 256, 0, 1);

      GWEN_XMLNode_GetXPath(NULL, GWEN_XmlCommanderGwenXml_GetCurrentDocNode(cmd), tbuf);

      DBG_ERROR(GWEN_LOGDOMAIN, "No value in path \"%s\" (%s)", path, GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_free(tbuf);

      /* GWEN_XMLNode_Dump(cmd->currentDocNode, 2); */
    }
#endif
  }

  return 0;
}



int _handleDbSetCharValue(GWEN_XMLCOMMANDER *cmd, GWEN_XMLNODE *xmlNode)
{
  return _handleDbSetCharValue_internal(cmd, xmlNode, GWEN_XmlCommanderGwenXml_GetCurrentDbGroup(cmd));
}



int _handleDbSetTempCharValue(GWEN_XMLCOMMANDER *cmd, GWEN_XMLNODE *xmlNode)
{
  return _handleDbSetCharValue_internal(cmd, xmlNode, GWEN_XmlCommanderGwenXml_GetCurrentTempDbGroup(cmd));
}



int _handleXmlIfCharDataMatches(GWEN_XMLCOMMANDER *cmd, GWEN_XMLNODE *xmlNode)
{
  const char *pattern;
  const char *path;
  const char *value;

  path=GWEN_XMLNode_GetProperty(xmlNode, "path", NULL);
  if (!(path && *path)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Missing or empty path in \"IfCharDataMatches\"");
    return GWEN_ERROR_INVALID;
  }

  pattern=GWEN_XMLNode_GetProperty(xmlNode, "pattern", NULL);
  if (!(pattern && *pattern)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Missing or empty pattern in \"IfCharDataMatches\"");
    return GWEN_ERROR_INVALID;
  }

  value=_getCharValueByPath(GWEN_XmlCommanderGwenXml_GetCurrentDocNode(cmd), path, NULL);
  if (value) {
    if (-1!=GWEN_Text_ComparePattern(value, pattern, 0)) {
      int rv;

      /* pattern matches, handle children  */
      rv=GWEN_XmlCommander_HandleChildren(cmd, xmlNode);
      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
    }
  }

  return 0;
}



int _handleXmlIfNotCharDataMatches(GWEN_XMLCOMMANDER *cmd, GWEN_XMLNODE *xmlNode)
{
  const char *pattern;
  const char *path;
  const char *value;

  path=GWEN_XMLNode_GetProperty(xmlNode, "path", NULL);
  if (!(path && *path)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Missing or empty path in \"IfNotCharDataMatches\"");
    return GWEN_ERROR_INVALID;
  }

  pattern=GWEN_XMLNode_GetProperty(xmlNode, "pattern", NULL);
  if (!(pattern && *pattern)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Missing or empty pattern in \"IfNotCharDataMatches\"");
    return GWEN_ERROR_INVALID;
  }

  value=_getCharValueByPath(GWEN_XmlCommanderGwenXml_GetCurrentDocNode(cmd), path, NULL);
  if (value) {
    if (-1==GWEN_Text_ComparePattern(value, pattern, 0)) {
      int rv;

      /* pattern doesnt match, handle children  */
      rv=GWEN_XmlCommander_HandleChildren(cmd, xmlNode);
      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
    }
  }

  return 0;
}



int _handleXmlIfHasCharData(GWEN_XMLCOMMANDER *cmd, GWEN_XMLNODE *xmlNode)
{
  const char *path;
  const char *value;

  path=GWEN_XMLNode_GetProperty(xmlNode, "path", NULL);
  if (!(path && *path)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Missing or empty path in \"IfCharDataMatches\"");
    return GWEN_ERROR_INVALID;
  }

  value=_getCharValueByPath(GWEN_XmlCommanderGwenXml_GetCurrentDocNode(cmd), path, NULL);
  if (value && *value) {
    int rv;

    /* there is a value, handle children  */
    rv=GWEN_XmlCommander_HandleChildren(cmd, xmlNode);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "XmlIfHasCharData: No value for path \"%s\"", path);
  }

  return 0;
}



int _handleXmlIfNotHasCharData(GWEN_XMLCOMMANDER *cmd, GWEN_XMLNODE *xmlNode)
{
  const char *path;
  const char *value;

  path=GWEN_XMLNode_GetProperty(xmlNode, "path", NULL);
  if (!(path && *path)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Missing or empty path in \"IfNotCharDataMatches\"");
    return GWEN_ERROR_INVALID;
  }

  value=_getCharValueByPath(GWEN_XmlCommanderGwenXml_GetCurrentDocNode(cmd), path, NULL);
  if (!(value && *value)) {
    int rv;

    /* there is a value, handle children  */
    rv=GWEN_XmlCommander_HandleChildren(cmd, xmlNode);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  return 0;
}



int _handleXmlIfPathExists(GWEN_XMLCOMMANDER *cmd, GWEN_XMLNODE *xmlNode)
{
  const char *path;

  path=GWEN_XMLNode_GetProperty(xmlNode, "path", NULL);
  if (!(path && *path)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Missing or empty path in \"IfPathExists\"");
    return GWEN_ERROR_INVALID;
  }

  if (GWEN_XMLNode_GetNodeByXPath(GWEN_XmlCommanderGwenXml_GetCurrentDocNode(cmd), path, GWEN_PATH_FLAGS_PATHMUSTEXIST)) {
    int rv;

    /* path exists, handle children  */
    rv=GWEN_XmlCommander_HandleChildren(cmd, xmlNode);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "XmlIfPathExists: Path \"%s\" does not exist", path);
  }

  return 0;
}



int _handleXmlIfNotPathExists(GWEN_XMLCOMMANDER *cmd, GWEN_XMLNODE *xmlNode)
{
  const char *path;

  path=GWEN_XMLNode_GetProperty(xmlNode, "path", NULL);
  if (!(path && *path)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Missing or empty path in \"IfNotPathExists\"");
    return GWEN_ERROR_INVALID;
  }

  if (NULL==GWEN_XMLNode_GetNodeByXPath(GWEN_XmlCommanderGwenXml_GetCurrentDocNode(cmd), path, GWEN_PATH_FLAGS_PATHMUSTEXIST)) {
    int rv;

    /* path does not exist, handle children  */
    rv=GWEN_XmlCommander_HandleChildren(cmd, xmlNode);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "Path \"%s\" exists", path);
  }

  return 0;
}


