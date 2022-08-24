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



#include "xmlcmd_lxml_p.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>


#include <ctype.h>


GWEN_INHERIT(GWEN_XMLCOMMANDER, GWEN_XMLCMD_LXML)



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static void GWENHYWFAR_CB _freeData(void *bp, void *p);
static void *_handleXmlPath(const char *entry, void *data, int idx, uint32_t flags);

GWEN_XMLCMD_LXML_TWOSTRINGS *_twoStrings_new();
void _twoStrings_free(GWEN_XMLCMD_LXML_TWOSTRINGS *ts);
GWEN_XMLCMD_LXML_TWOSTRINGS *_twoStrings_fromString(const char *source, uint8_t delimiter);




/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */


GWEN_XMLCOMMANDER *GWEN_XmlCommanderLibXml_new(xmlNodePtr documentRoot, GWEN_DB_NODE *dbRoot)
{
  GWEN_XMLCOMMANDER *cmd;
  GWEN_XMLCMD_LXML *xcmd;

  cmd=GWEN_XmlCommander_new();
  GWEN_NEW_OBJECT(GWEN_XMLCMD_LXML, xcmd);
  GWEN_INHERIT_SETDATA(GWEN_XMLCOMMANDER, GWEN_XMLCMD_LXML, cmd, xcmd, _freeData);

  xcmd->docRoot=documentRoot;
  xcmd->dbRoot=dbRoot;
  xcmd->tempDbRoot=GWEN_DB_Group_new("dbTempRoot");

  xcmd->currentDbGroup=xcmd->dbRoot;
  xcmd->currentTempDbGroup=xcmd->tempDbRoot;
  xcmd->currentDocNode=documentRoot;

  return cmd;
}



void _freeData(GWEN_UNUSED void *bp, void *p)
{
  GWEN_XMLCMD_LXML *xcmd;

  xcmd=(GWEN_XMLCMD_LXML*) p;

  GWEN_DB_Group_free(xcmd->tempDbRoot);
  GWEN_FREE_OBJECT(xcmd);
}



xmlNodePtr GWEN_XmlCommanderLibXml_GetDocRoot(const GWEN_XMLCOMMANDER *cmd)
{
  GWEN_XMLCMD_LXML *xcmd;

  assert(cmd);
  xcmd=GWEN_INHERIT_GETDATA(GWEN_XMLCOMMANDER, GWEN_XMLCMD_LXML, cmd);
  assert(xcmd);

  return xcmd->docRoot;
}



xmlNodePtr GWEN_XmlCommanderLibXml_GetCurrentDocNode(const GWEN_XMLCOMMANDER *cmd)
{
  GWEN_XMLCMD_LXML *xcmd;

  assert(cmd);
  xcmd=GWEN_INHERIT_GETDATA(GWEN_XMLCOMMANDER, GWEN_XMLCMD_LXML, cmd);
  assert(xcmd);

  return xcmd->currentDocNode;
}



void GWEN_XmlCommanderLibXml_SetCurrentDocNode(GWEN_XMLCOMMANDER *cmd, xmlNodePtr n)
{
  GWEN_XMLCMD_LXML *xcmd;

  assert(cmd);
  xcmd=GWEN_INHERIT_GETDATA(GWEN_XMLCOMMANDER, GWEN_XMLCMD_LXML, cmd);
  assert(xcmd);

  xcmd->currentDocNode=n;
}



GWEN_DB_NODE *GWEN_XmlCommanderLibXml_GetDbRoot(const GWEN_XMLCOMMANDER *cmd)
{
  GWEN_XMLCMD_LXML *xcmd;

  assert(cmd);
  xcmd=GWEN_INHERIT_GETDATA(GWEN_XMLCOMMANDER, GWEN_XMLCMD_LXML, cmd);
  assert(xcmd);

  return xcmd->dbRoot;
}



GWEN_DB_NODE *GWEN_XmlCommanderLibXml_GetCurrentDbGroup(const GWEN_XMLCOMMANDER *cmd)
{
  GWEN_XMLCMD_LXML *xcmd;

  assert(cmd);
  xcmd=GWEN_INHERIT_GETDATA(GWEN_XMLCOMMANDER, GWEN_XMLCMD_LXML, cmd);
  assert(xcmd);

  return xcmd->currentDbGroup;
}



void GWEN_XmlCommanderLibXml_SetCurrentDbGroup(GWEN_XMLCOMMANDER *cmd, GWEN_DB_NODE *db)
{
  GWEN_XMLCMD_LXML *xcmd;

  assert(cmd);
  xcmd=GWEN_INHERIT_GETDATA(GWEN_XMLCOMMANDER, GWEN_XMLCMD_LXML, cmd);
  assert(xcmd);

  xcmd->currentDbGroup=db;
}



GWEN_DB_NODE *GWEN_XmlCommanderLibXml_GetTempDbRoot(const GWEN_XMLCOMMANDER *cmd)
{
  GWEN_XMLCMD_LXML *xcmd;

  assert(cmd);
  xcmd=GWEN_INHERIT_GETDATA(GWEN_XMLCOMMANDER, GWEN_XMLCMD_LXML, cmd);
  assert(xcmd);

  return xcmd->tempDbRoot;
}



GWEN_DB_NODE *GWEN_XmlCommanderLibXml_GetCurrentTempDbGroup(const GWEN_XMLCOMMANDER *cmd)
{
  GWEN_XMLCMD_LXML *xcmd;

  assert(cmd);
  xcmd=GWEN_INHERIT_GETDATA(GWEN_XMLCOMMANDER, GWEN_XMLCMD_LXML, cmd);
  assert(xcmd);

  return xcmd->currentTempDbGroup;
}



void GWEN_XmlCommanderLibXml_SetCurrentTempDbGroup(GWEN_XMLCOMMANDER *cmd, GWEN_DB_NODE *db)
{
  GWEN_XMLCMD_LXML *xcmd;

  assert(cmd);
  xcmd=GWEN_INHERIT_GETDATA(GWEN_XMLCOMMANDER, GWEN_XMLCMD_LXML, cmd);
  assert(xcmd);

  xcmd->currentTempDbGroup=db;
}



void GWEN_XmlCommanderLibXml_EnterDocNode(GWEN_XMLCOMMANDER *cmd, xmlNodePtr xNode)
{
  GWEN_XMLCMD_LXML *xcmd;

  assert(cmd);
  xcmd=GWEN_INHERIT_GETDATA(GWEN_XMLCOMMANDER, GWEN_XMLCMD_LXML, cmd);
  assert(xcmd);

  assert(xNode);

  if (xcmd->currentStackPos<GWEN_XMLCMD_LXML_PATH_MAXDEPTH) {
    xcmd->xmlNodeStack[xcmd->currentStackPos]=xNode;
    xcmd->currentStackPos++;
    xcmd->currentDocNode=xNode;
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Stack full, SNH!");
    abort();
  }
}



void GWEN_XmlCommanderLibXml_LeaveDocNode(GWEN_XMLCOMMANDER *cmd)
{
  GWEN_XMLCMD_LXML *xcmd;
  xmlNodePtr xNode;

  assert(cmd);
  xcmd=GWEN_INHERIT_GETDATA(GWEN_XMLCOMMANDER, GWEN_XMLCMD_LXML, cmd);
  assert(xcmd);

  if (xcmd->currentStackPos>0) {
    xNode=xcmd->xmlNodeStack[xcmd->currentStackPos-1];
    if (xNode==NULL) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Nothing on stack");
      assert(xNode);
    }
    xcmd->currentStackPos--;
    xcmd->currentDocNode=xNode;
  }
}





xmlNodePtr GWEN_XmlCommanderLibXml_GetXmlNode(xmlNodePtr n, const char *path, uint32_t flags)
{
  return (xmlNodePtr)GWEN_Path_HandleWithIdx(path, n, flags, _handleXmlPath);
}



xmlNodePtr GWEN_XmlCommanderLibXml_FindFirstElement(xmlNodePtr parent, const char *elemName)
{
  xmlNodePtr n;

  n=parent->children;
  while (n) {
    if (n->type==XML_ELEMENT_NODE) {
      if (n->name && strcmp((const char *)n->name, elemName)==0) {
        return n;
      }
    }
    n=n->next;
  } /* while */

  return NULL;
}



xmlNodePtr GWEN_XmlCommanderLibXml_FindNextElement(xmlNodePtr elem, const char *elemName)
{
  xmlNodePtr n;

  assert(elem);
  n=elem->next;
  while (n) {
    if (n->type==XML_ELEMENT_NODE) {
      if (n->name && strcmp((const char *)n->name, elemName)==0) {
        return n;
      }
    }
    n=n->next;
  } /* while */

  return NULL;
}



int GWEN_XmlCommanderLibXml_SetXmlCharValue(xmlNodePtr n, const char *path, const char *value)
{
  xmlNodePtr node;

  node=GWEN_XmlCommanderLibXml_GetXmlNode(n, path, 0);
  if (!node) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    return GWEN_ERROR_GENERIC;
  }

  xmlNodeSetContent(node, BAD_CAST value);

  return 0;
}



const char *GWEN_XmlCommanderLibXml_GetXmlCharValue(xmlNodePtr n, const char *path, const char *defValue)
{
  xmlNodePtr node;

  node=GWEN_XmlCommanderLibXml_GetXmlNode(n, path, GWEN_PATH_FLAGS_NAMEMUSTEXIST);
  if (!node) {
    DBG_INFO(GWEN_LOGDOMAIN, "path [%s] not found", path);
    return defValue;
  }
  node=node->children;
  if (node==0)
    return defValue;

  while (node) {
    if (node->type==XML_TEXT_NODE) {
      return (const char *)node->content;
    }
    node=node->next;
  }

  return defValue;
}



int GWEN_XmlCommanderLibXml_SetIntValue(xmlNodePtr n, const char *path, int value)
{
  char numbuf[32];

  snprintf(numbuf, sizeof(numbuf)-1, "%d", value);
  numbuf[sizeof(numbuf)-1]=0;
  return GWEN_XmlCommanderLibXml_SetXmlCharValue(n, path, numbuf);
}



int GWEN_XmlCommanderLibXml_GetIntValue(xmlNodePtr n, const char *path, int defValue)
{
  const char *s;
  int i;

  s=GWEN_XmlCommanderLibXml_GetXmlCharValue(n, path, NULL);
  if (s==NULL)
    return defValue;
  if (1!=sscanf(s, "%i", &i))
    return defValue;
  return i;
}



const char *GWEN_XmlCommanderLibXml_GetXmlCharValueByPath(xmlNodePtr elem, const char *path, const char *defValue)
{
  GWEN_XMLCMD_LXML_TWOSTRINGS *tsLevel1;

  tsLevel1=_twoStrings_fromString(path, '@');
  if (tsLevel1->string2) {
    xmlNodePtr n;

    /* really two strings, so string1 is node, string2 is property name */
    n=GWEN_XmlCommanderLibXml_GetXmlNode(elem, tsLevel1->string1, GWEN_PATH_FLAGS_PATHMUSTEXIST);
    if (n) {
      GWEN_XMLCMD_LXML_TWOSTRINGS *tsLevel2;
      const char *result=NULL;

      tsLevel2=_twoStrings_fromString(tsLevel1->string2, ':');
      if (tsLevel2->string2) {
        /* really two strings, so string1 is namespace, string2 is property name */
        result=(const char*) xmlGetNsProp(n, BAD_CAST tsLevel2->string2, BAD_CAST tsLevel2->string1);
      }
      else {
        /* only one string, so no namespace given */
        result=(const char*) xmlGetNoNsProp(n, BAD_CAST tsLevel2->string1);
      }
      if (result) {
        _twoStrings_free(tsLevel2);
        _twoStrings_free(tsLevel1);
        return result;
      } /* else fall-through */
      _twoStrings_free(tsLevel2);
    } /* else fall-through */
  }
  else {
    xmlNodePtr n;

    /* only one string, so string1 is node */
    n=GWEN_XmlCommanderLibXml_GetXmlNode(elem, tsLevel1->string1, GWEN_PATH_FLAGS_PATHMUSTEXIST);
    if (n) {
      n=n->children;
      while (n) {
        if (n->type==XML_TEXT_NODE) {
          _twoStrings_free(tsLevel1);
          return (const char *)n->content;
        }
        n=n->next;
      }
    }
  }
  _twoStrings_free(tsLevel1);


  DBG_INFO(GWEN_LOGDOMAIN, "path [%s] not found", path);
  return defValue;
}



void GWEN_XmlCommanderLibXml_SetXmlCharValueByPath(xmlNodePtr elem, const char *path, const char *value)
{
  GWEN_XMLCMD_LXML_TWOSTRINGS *tsLevel1;

  tsLevel1=_twoStrings_fromString(path, '@');
  if (tsLevel1->string2) {
    xmlNodePtr n;

    /* really two strings, so string1 is node, string2 is property name */
    n=GWEN_XmlCommanderLibXml_GetXmlNode(elem, tsLevel1->string1, 0);
    if (n) {
      GWEN_XMLCMD_LXML_TWOSTRINGS *tsLevel2;

      tsLevel2=_twoStrings_fromString(tsLevel1->string2, ':');
      if (tsLevel2->string2) {
        xmlNsPtr nameSpace;

        /* really two strings, so string1 is namespace, string2 is property name */
        nameSpace=xmlSearchNs(n->doc, n, BAD_CAST tsLevel2->string1);
        xmlNewNsProp(n, nameSpace, BAD_CAST tsLevel2->string2, BAD_CAST value);
      }
      else {
        /* only one string, so no namespace given */
        xmlNewProp(n, BAD_CAST tsLevel2->string1, BAD_CAST value);
      }
      _twoStrings_free(tsLevel2);
    } /* else fall-through */
  }
  else {
    xmlNodePtr n;

    /* only one string, so string1 is node */
    n=GWEN_XmlCommanderLibXml_GetXmlNode(elem, tsLevel1->string1, 0);
    if (n)
      xmlNodeSetContent(n, BAD_CAST value);
  }
  _twoStrings_free(tsLevel1);
}






void *_handleXmlPath(const char *entry, void *data, int idx, uint32_t flags)
{
  xmlNodePtr n;
  xmlNodePtr nn;
  int i;
  xmlNsPtr nameSpace=NULL;
  const char *p;
  const char *name;

  n=(xmlNodePtr)data;

  name=entry;
  p=strchr(entry, ':');
  if (p) {
    char prefix[32];
    int plen;

    plen=p-entry;
    if (plen) {
      if (plen>=sizeof(prefix)) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Prefix too long (%d>%d)", (int)plen, (int)sizeof(prefix));
        return 0;
      }
      strncpy(prefix, entry, plen);
      prefix[plen]=0;
      nameSpace=xmlSearchNs(n->doc, n, BAD_CAST prefix);
      if (!nameSpace) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Namespace \"%s\" not found", prefix);
        return 0;
      }
    }
    name=p+1;
  }

  /* check whether we are allowed to simply create the node */
  if (
    ((flags & GWEN_PATH_FLAGS_LAST) &&
     (((flags & GWEN_PATH_FLAGS_VARIABLE) &&
       (flags & GWEN_PATH_FLAGS_CREATE_VAR)) ||
      (!(flags & GWEN_PATH_FLAGS_VARIABLE) &&
       (flags & GWEN_PATH_FLAGS_CREATE_GROUP)))
    ) ||
    (
      !(flags & GWEN_PATH_FLAGS_LAST) &&
      (flags & GWEN_PATH_FLAGS_PATHCREATE))
  ) {
    /* simply create the new variable/group */
    if (idx!=0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Index is not 0, not creating %s[%d]", entry, idx);
      return 0;
    }
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Unconditionally creating entry \"%s\"", entry);
    nn=xmlNewChild(n, nameSpace, BAD_CAST name, NULL);
    return nn;
  }

  /* find the node */

  nn=n->children;
  i=idx;
  while (nn) {
    if (nn->type==XML_ELEMENT_NODE) {
      if (nn->name && strcmp((const char *)nn->name, name)==0) {
        if (i--==0)
          break;
      }
    }
    nn=nn->next;
  } /* while */

  if (!nn) {
    /* node not found, check, if we are allowed to create it */
    if (
      (!(flags & GWEN_PATH_FLAGS_LAST) &&
       (flags & GWEN_PATH_FLAGS_PATHMUSTEXIST)) ||
      (flags & GWEN_PATH_FLAGS_NAMEMUSTEXIST)
    ) {
      DBG_VERBOUS(GWEN_LOGDOMAIN, "Entry \"%s\" does not exist", entry);
      return 0;
    }
    /* create the new variable/group */
    if (idx!=0) {
      DBG_INFO(GWEN_LOGDOMAIN, "Index is not 0, not creating %s[%d]",
               entry, idx);
      return 0;
    }
    DBG_VERBOUS(GWEN_LOGDOMAIN,
                "Entry \"%s\" not found, creating", entry);
    nn=xmlNewChild(n, nameSpace, BAD_CAST name, NULL);
  } /* if node not found */
  else {
    /* node does exist, check whether this is ok */
    if (
      ((flags & GWEN_PATH_FLAGS_LAST) &&
       (flags & GWEN_PATH_FLAGS_NAMEMUSTNOTEXIST)) ||
      (!(flags & GWEN_PATH_FLAGS_LAST) &&
       (flags & GWEN_PATH_FLAGS_PATHMUSTNOTEXIST))
    ) {
      DBG_VERBOUS(GWEN_LOGDOMAIN, "Entry \"%s\" already exists", entry);
      return 0;
    }
  }

  return nn;
}







GWEN_XMLCMD_LXML_TWOSTRINGS *_twoStrings_new()
{
  GWEN_XMLCMD_LXML_TWOSTRINGS *ts;

  GWEN_NEW_OBJECT(GWEN_XMLCMD_LXML_TWOSTRINGS, ts);
  return ts;
}



void _twoStrings_free(GWEN_XMLCMD_LXML_TWOSTRINGS *ts)
{
  if (ts) {
    if (ts->ownStringsBitmap & 1) {
      if (ts->string1)
        free(ts->string1);
    }
    if (ts->ownStringsBitmap & 2) {
      if (ts->string2)
        free(ts->string2);
    }
    GWEN_FREE_OBJECT(ts);
  }
}



GWEN_XMLCMD_LXML_TWOSTRINGS *_twoStrings_fromString(const char *source, uint8_t delimiter)
{
  const char *s;
  GWEN_XMLCMD_LXML_TWOSTRINGS *ts;

  ts=_twoStrings_new();

  s=strchr(source, delimiter);
  if (s) {
    int idx;

    idx=s-source;
    ts->string1=GWEN_Text_strndup(source, idx);
    ts->ownStringsBitmap|=1;
    s++;
    if (s &&*s)
      ts->string2=(char*)s;

    return ts;
  }
  else {
    ts->string1=(char*)source;
    return ts;
  }
}




