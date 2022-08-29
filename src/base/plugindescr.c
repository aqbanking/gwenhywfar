/***************************************************************************
 begin       : Thu Apr 03 2003
 copyright   : (C) 2022 by Martin Preuss
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

#define DISABLE_DEBUGLOG


#include "plugindescr_p.h"
#include "i18n_l.h"
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/i18n.h>



GWEN_LIST_FUNCTIONS(GWEN_PLUGIN_DESCRIPTION, GWEN_PluginDescription)
GWEN_LIST2_FUNCTIONS(GWEN_PLUGIN_DESCRIPTION, GWEN_PluginDescription)



static GWEN_PLUGIN_DESCRIPTION *_pdList2_freeAll_cb(GWEN_PLUGIN_DESCRIPTION *pd, void *);

static int _getLongDescrByFormat(const GWEN_PLUGIN_DESCRIPTION *pd, const char *s, GWEN_BUFFER *buf);
static GWEN_PLUGIN_DESCRIPTION *_readPluginDescriptionFromFile(const char *folder, const char *fullFileName, const char *pluginType);
static GWEN_XMLNODE *_getLocalOrMainPluginNode(GWEN_XMLNODE *pluginDescrNode);




GWEN_PLUGIN_DESCRIPTION *GWEN_PluginDescription_new(void)
{
  GWEN_PLUGIN_DESCRIPTION *pd;

  GWEN_NEW_OBJECT(GWEN_PLUGIN_DESCRIPTION, pd);
  pd->refCount=1;
  DBG_MEM_INC("GWEN_PLUGIN_DESCRIPTION", 0);
  GWEN_LIST_INIT(GWEN_PLUGIN_DESCRIPTION, pd);
  return pd;
}



GWEN_PLUGIN_DESCRIPTION *GWEN_PluginDescription_fromXml(GWEN_XMLNODE *node)
{
  GWEN_PLUGIN_DESCRIPTION *pd;
  const char *p;

  pd=GWEN_PluginDescription_new();
  p=GWEN_XMLNode_GetProperty(node, "name", 0);
  if (!p) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Unnamed plugin");
    GWEN_PluginDescription_free(pd);
    return 0;
  }
  pd->name=strdup(p);
  pd->xmlNode=GWEN_XMLNode_dup(node);

  p=GWEN_XMLNode_GetProperty(node, "i18n", NULL);
  if (!p) {
    DBG_NOTICE(GWEN_LOGDOMAIN, "Plugin has no I18N domain, using GWEN");
    p="gwenhywfar";
  }
  pd->langDomain=strdup(p);

  p=GWEN_XMLNode_GetProperty(node, "type", 0);
  if (!p) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Plugin has no type");
    GWEN_PluginDescription_free(pd);
    return 0;
  }
  pd->type=strdup(p);

  p=GWEN_XMLNode_GetCharValue(node, "version", 0);
  if (p)
    pd->version=strdup(p);
  p=GWEN_XMLNode_GetCharValue(node, "author", 0);
  if (p)
    pd->author=strdup(p);
  p=GWEN_XMLNode_GetCharValue(node, "short", 0);
  if (p)
    pd->shortDescr=strdup(p);
  p=GWEN_XMLNode_GetCharValue(node, "descr", 0);
  if (p)
    pd->longDescr=strdup(p);
  return pd;
}



void GWEN_PluginDescription_free(GWEN_PLUGIN_DESCRIPTION *pd)
{
  if (pd) {
    assert(pd->refCount);
    if (pd->refCount==1) {
      DBG_MEM_DEC("GWEN_PLUGIN_DESCRIPTION");
      GWEN_LIST_FINI(GWEN_PLUGIN_DESCRIPTION, pd);
      free(pd->path);
      GWEN_XMLNode_free(pd->xmlNode);
      free(pd->fileName);
      free(pd->longDescr);
      free(pd->shortDescr);
      free(pd->author);
      free(pd->version);
      free(pd->langDomain);
      free(pd->type);
      free(pd->name);
      pd->refCount=0;
      GWEN_FREE_OBJECT(pd);
    }
    else
      pd->refCount--;
  }
}



void GWEN_PluginDescription_Attach(GWEN_PLUGIN_DESCRIPTION *pd)
{
  assert(pd);
  assert(pd->refCount);
  pd->refCount++;
}



GWEN_PLUGIN_DESCRIPTION *GWEN_PluginDescription_dup(const GWEN_PLUGIN_DESCRIPTION *pd)
{
  GWEN_PLUGIN_DESCRIPTION *np;
  const char *s;

  assert(pd);
  GWEN_NEW_OBJECT(GWEN_PLUGIN_DESCRIPTION, np);
  np->refCount=1;
  DBG_MEM_INC("GWEN_PLUGIN_DESCRIPTION", 0);
  GWEN_LIST_INIT(GWEN_PLUGIN_DESCRIPTION, np);

  s=pd->fileName;
  if (s)
    np->fileName=strdup(s);

  s=pd->path;
  if (s)
    np->path=strdup(s);
  s=pd->name;
  if (s)
    np->name=strdup(s);
  s=pd->type;
  if (s)
    np->type=strdup(s);
  s=pd->langDomain;
  if (s)
    np->langDomain=strdup(s);
  s=pd->shortDescr;
  if (s)
    np->shortDescr=strdup(s);
  s=pd->author;
  if (s)
    np->author=strdup(s);
  s=pd->version;
  if (s)
    np->version=strdup(s);
  s=pd->longDescr;
  if (s)
    np->longDescr=strdup(s);
  np->isActive=pd->isActive;

  if (pd->xmlNode)
    np->xmlNode=GWEN_XMLNode_dup(pd->xmlNode);

  return np;
}



GWEN_PLUGIN_DESCRIPTION *_pdList2_freeAll_cb(GWEN_PLUGIN_DESCRIPTION *pd, GWEN_UNUSED void *user_data)
{
  GWEN_PluginDescription_free(pd);
  return 0;
}



void GWEN_PluginDescription_List2_freeAll(GWEN_PLUGIN_DESCRIPTION_LIST2 *pdl)
{
  GWEN_PluginDescription_List2_ForEach(pdl, _pdList2_freeAll_cb, 0);
  GWEN_PluginDescription_List2_free(pdl);
}



const char *GWEN_PluginDescription_GetPath(const GWEN_PLUGIN_DESCRIPTION *pd)
{
  assert(pd);
  return pd->path;
}



void GWEN_PluginDescription_SetPath(GWEN_PLUGIN_DESCRIPTION *pd, const char *s)
{
  assert(pd);
  free(pd->path);
  if (s)
    pd->path=strdup(s);
  else
    pd->path=0;
}



const char *GWEN_PluginDescription_GetName(const GWEN_PLUGIN_DESCRIPTION *pd)
{
  assert(pd);
  return pd->name;
}



void GWEN_PluginDescription_SetName(GWEN_PLUGIN_DESCRIPTION *pd, const char *s)
{
  assert(pd);
  free(pd->name);
  pd->name=s?strdup(s):NULL;
}



const char *GWEN_PluginDescription_GetType(const GWEN_PLUGIN_DESCRIPTION *pd)
{
  assert(pd);
  return pd->type;
}



void GWEN_PluginDescription_SetType(GWEN_PLUGIN_DESCRIPTION *pd, const char *s)
{
  assert(pd);
  free(pd->type);
  pd->type=s?strdup(s):NULL;
}



const char *GWEN_PluginDescription_GetShortDescr(const GWEN_PLUGIN_DESCRIPTION *pd)
{
  assert(pd);
  return GWEN_I18N_Translate(pd->langDomain, pd->shortDescr);
}



void GWEN_PluginDescription_SetShortDescr(GWEN_PLUGIN_DESCRIPTION *pd, const char *s)
{
  assert(pd);
  free(pd->shortDescr);
  pd->shortDescr=s?strdup(s):NULL;
}



const char *GWEN_PluginDescription_GetAuthor(const GWEN_PLUGIN_DESCRIPTION *pd)
{
  assert(pd);
  return pd->author;
}



void GWEN_PluginDescription_SetAuthor(GWEN_PLUGIN_DESCRIPTION *pd, const char *s)
{
  assert(pd);
  free(pd->author);
  pd->author=s?strdup(s):NULL;
}



const char *GWEN_PluginDescription_GetVersion(const GWEN_PLUGIN_DESCRIPTION *pd)
{
  assert(pd);
  return pd->version;
}



void GWEN_PluginDescription_SetVersion(GWEN_PLUGIN_DESCRIPTION *pd, const char *s)
{
  assert(pd);
  free(pd->version);
  pd->version=s?strdup(s):NULL;
}



const char *GWEN_PluginDescription_GetLongDescr(const GWEN_PLUGIN_DESCRIPTION *pd)
{
  assert(pd);
  return GWEN_I18N_Translate(pd->langDomain, pd->longDescr);
}



void GWEN_PluginDescription_SetLongDescr(GWEN_PLUGIN_DESCRIPTION *pd, const char *s)
{
  assert(pd);
  free(pd->longDescr);
  pd->longDescr=s?strdup(s):NULL;
}



int _getLongDescrByFormat(const GWEN_PLUGIN_DESCRIPTION *pd, const char *s, GWEN_BUFFER *buf)
{
  assert(pd);
  if (pd->xmlNode) {
    GWEN_XMLNODE *n;

    n=GWEN_XMLNode_FindFirstTag(pd->xmlNode, "descr", 0, 0);
    if (n) {
      n=GWEN_XMLNode_FindFirstTag(n, "text", "format", s);
      while (n) {
        if (0==GWEN_XMLNode_GetProperty(n, "lang", 0)) {
          int rv;

          rv=GWEN_XMLNode_toBuffer(n, buf, GWEN_XML_FLAGS_TOLERANT_ENDTAGS);
          if (rv) {
            DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
            return rv;
          }
          return 0;
        }
        n=GWEN_XMLNode_FindNextTag(n, "text", "format", s);
      } /* while */
    }
  }
  return GWEN_ERROR_GENERIC;
}



#ifndef NO_DEPRECATED_SYMBOLS
static int _getLocalizedLongDescrByFormat(const GWEN_PLUGIN_DESCRIPTION *pd, const char *s, const char *lang, GWEN_BUFFER *buf)
{
  assert(pd);

  if (pd->xmlNode) {
    GWEN_XMLNODE *n;

    n=GWEN_XMLNode_FindFirstTag(pd->xmlNode, "descr", 0, 0);
    if (n) {
      n=GWEN_XMLNode_FindFirstTag(n, "text", "lang", lang);
      while (n) {
        const char *fmt;

        fmt=GWEN_XMLNode_GetProperty(n, "format", 0);
        if (fmt && strcasecmp(fmt, s)==0) {
          int rv;

          rv=GWEN_XMLNode_toBuffer(n, buf, GWEN_XML_FLAGS_TOLERANT_ENDTAGS);
          if (rv) {
            DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
            return rv;
          }
          return 0;
        }
        n=GWEN_XMLNode_FindNextTag(n, "text", "lang", lang);
      } /* while */
    }
  }

  return GWEN_ERROR_GENERIC;
}



int GWEN_PluginDescription_GetLongDescrByFormat(const GWEN_PLUGIN_DESCRIPTION *pd, const char *s, GWEN_BUFFER *buf)
{
  GWEN_STRINGLIST *langl;
  int rv;

  langl=GWEN_I18N_GetCurrentLocaleList();
  if (langl) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(langl);
    while (se) {
      const char *l;

      l=GWEN_StringListEntry_Data(se);
      DBG_NOTICE(GWEN_LOGDOMAIN, "Trying locale \"%s\"", l);
      assert(l);

      rv=_getLocalizedLongDescrByFormat(pd, s, l, buf);
      if (rv==0)
        return rv;

      se=GWEN_StringListEntry_Next(se);
    } /* while */
  } /* if language list available */

  /* no localized version found, return text for default language */
  rv=_getLongDescrByFormat(pd, s, buf);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}
#endif  // ifndef NO_DEPRECATED_SYMBOLS



const char *GWEN_PluginDescription_GetFileName(const GWEN_PLUGIN_DESCRIPTION *pd)
{
  assert(pd);
  return pd->fileName;
}



void GWEN_PluginDescription_SetFileName(GWEN_PLUGIN_DESCRIPTION *pd, const char *s)
{
  assert(pd);
  free(pd->fileName);
  if (s)
    pd->fileName=strdup(s);
  else
    pd->fileName=0;
}



GWEN_XMLNODE *GWEN_PluginDescription_GetXmlNode(const GWEN_PLUGIN_DESCRIPTION *pd)
{
  assert(pd);
  return pd->xmlNode;
}



GWEN_PLUGIN_DESCRIPTION_LIST2 *GWEN_LoadPluginDescrs(const char *path)
{
  GWEN_PLUGIN_DESCRIPTION_LIST2 *pl;

  pl=GWEN_PluginDescription_List2_new();

  GWEN_LoadPluginDescrsByType(path, 0, pl);
  if (GWEN_PluginDescription_List2_GetSize(pl)==0) {
    GWEN_PluginDescription_List2_free(pl);
    return 0;
  }
  return pl;
}



int GWEN_PluginDescription_IsActive(const GWEN_PLUGIN_DESCRIPTION *pd)
{
  assert(pd);
  return pd->isActive;
}



void GWEN_PluginDescription_SetIsActive(GWEN_PLUGIN_DESCRIPTION *pd, int i)
{
  assert(pd);
  pd->isActive=i;
}



int GWEN_LoadPluginDescrsByType(const char *folder, const char *pluginType, GWEN_PLUGIN_DESCRIPTION_LIST2 *pdl)
{
  GWEN_STRINGLIST *fileNameList;
  int rv;

  fileNameList=GWEN_StringList_new();
  rv=GWEN_Directory_GetFileEntries(folder?folder:"", fileNameList, "*.xml");
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "No matching files found in %s", folder?folder:"<empty>");
    GWEN_StringList_free(fileNameList);
    return rv;
  }

  if (GWEN_StringList_Count(fileNameList)) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(fileNameList);
    if (se) {
      GWEN_BUFFER *nbuf;
      unsigned int pathLen;

      nbuf=GWEN_Buffer_new(0, 256, 0, 1);
      if (folder && *folder) {
        GWEN_Buffer_AppendString(nbuf, folder);
        GWEN_Buffer_AppendString(nbuf, GWEN_DIR_SEPARATOR_S);
      }
      pathLen=GWEN_Buffer_GetUsedBytes(nbuf);

      while(se) {
        const char *fileName;

        fileName=GWEN_StringListEntry_Data(se);
        DBG_INFO(GWEN_LOGDOMAIN, "Checking entry \"%s\" (%s)\n", fileName, GWEN_Buffer_GetStart(nbuf));
        if (fileName && *fileName) {
          GWEN_PLUGIN_DESCRIPTION *pd;

          GWEN_Buffer_AppendString(nbuf, fileName);
          pd=_readPluginDescriptionFromFile(folder, GWEN_Buffer_GetStart(nbuf), pluginType);
          if (pd) {
            DBG_INFO(GWEN_LOGDOMAIN,
                     "Adding loaded plugin description %s/%s (%s)",
                     pluginType, GWEN_PluginDescription_GetName(pd), GWEN_Buffer_GetStart(nbuf));
            GWEN_PluginDescription_List2_PushBack(pdl, pd);
          }
          else {
            DBG_INFO(GWEN_LOGDOMAIN, "Could not read description from \"%s\"\n", fileName);
          }
          GWEN_Buffer_Crop(nbuf, 0, pathLen);
          GWEN_Buffer_SetPos(nbuf, pathLen);
        }
        se=GWEN_StringListEntry_Next(se);
      } /* while(se) */
      GWEN_Buffer_free(nbuf);
    } /* if (se) */
  } /* if stringlist not empty */
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "Not matching files in \"%s\"\n", folder);
  }
  GWEN_StringList_free(fileNameList);
  return 0;
}



GWEN_PLUGIN_DESCRIPTION *_readPluginDescriptionFromFile(const char *folder, const char *fullFileName, const char *pluginType)
{
  GWEN_XMLNODE *fileNode;
  
  fileNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
  if (GWEN_XML_ReadFile(fileNode, fullFileName,
                        GWEN_XML_FLAGS_DEFAULT | GWEN_XML_FLAGS_HANDLE_HEADERS | GWEN_XML_FLAGS_HANDLE_OPEN_HTMLTAGS)) {
    DBG_WARN(GWEN_LOGDOMAIN, "Bad file \"%s\"", fullFileName);
  }
  else {
    GWEN_XMLNODE *pluginDescrNode;
    GWEN_XMLNODE *pluginNode=NULL;

    pluginDescrNode=GWEN_XMLNode_FindFirstTag(fileNode, "PluginDescr", NULL, NULL);
    if (!pluginDescrNode)
      pluginDescrNode=fileNode;

    pluginNode=_getLocalOrMainPluginNode(pluginDescrNode);
    if (pluginNode) {
      GWEN_PLUGIN_DESCRIPTION *pd;
      int typeMatches=1;
  
      if (pluginType) {
        const char *ft;
  
        ft=GWEN_XMLNode_GetProperty(pluginNode, "type", NULL);
        if (!ft)
          typeMatches=0;
        else if (strcasecmp(ft, pluginType)!=0) {
          typeMatches=0;
        }
      } /* if pluginType specified */
      if (typeMatches) {
        pd=GWEN_PluginDescription_fromXml(pluginNode);
        if (!pd) {
          DBG_WARN(GWEN_LOGDOMAIN, "Bad plugin description");
        }
        else {
          GWEN_PluginDescription_SetFileName(pd, fullFileName);
          GWEN_PluginDescription_SetPath(pd, folder);
          GWEN_XMLNode_free(fileNode);
          return pd;
        }
      } /* if typeMatches */
      else {
        DBG_INFO(GWEN_LOGDOMAIN, "Ignoring file \"%s\" (bad/missing pluginType)", fullFileName);
      }
    }
    else {
      DBG_WARN(GWEN_LOGDOMAIN, "File \"%s\" does not contain a plugin description", fullFileName);
    }
  }
  GWEN_XMLNode_free(fileNode);
  return NULL;
}



GWEN_XMLNODE *_getLocalOrMainPluginNode(GWEN_XMLNODE *pluginDescrNode)
{
  GWEN_XMLNODE *n=NULL;
  GWEN_STRINGLIST *languageList;

  languageList=GWEN_I18N_GetCurrentLocaleList();
  if (languageList) {
    GWEN_STRINGLISTENTRY *se;
  
    se=GWEN_StringList_FirstEntry(languageList);
    while (se) {
      const char *l;
  
      l=GWEN_StringListEntry_Data(se);
      DBG_DEBUG(GWEN_LOGDOMAIN, "Trying locale \"%s\"", l);
      assert(l);
      n=GWEN_XMLNode_FindFirstTag(pluginDescrNode, "plugin", "lang", l);
      if (n)
        break;
      se=GWEN_StringListEntry_Next(se);
    } /* while */
  } /* if language list available */
  
  if (!n)
    n=GWEN_XMLNode_FindFirstTag(pluginDescrNode, "plugin", 0, 0);

  return n;
}








