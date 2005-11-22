/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Thu Apr 03 2003
 copyright   : (C) 2003 by Martin Preuss
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

#include "plugindescr_p.h"
#include "i18n_l.h"
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/bio_buffer.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <strings.h>



GWEN_LIST_FUNCTIONS(GWEN_PLUGIN_DESCRIPTION, GWEN_PluginDescription)
GWEN_LIST2_FUNCTIONS(GWEN_PLUGIN_DESCRIPTION, GWEN_PluginDescription)



GWEN_PLUGIN_DESCRIPTION *GWEN_PluginDescription_new(GWEN_XMLNODE *node){
  GWEN_PLUGIN_DESCRIPTION *pd;
  const char *p;

  GWEN_NEW_OBJECT(GWEN_PLUGIN_DESCRIPTION, pd);
  DBG_MEM_INC("GWEN_PLUGIN_DESCRIPTION", 0);
  GWEN_LIST_INIT(GWEN_PLUGIN_DESCRIPTION, pd);
  p=GWEN_XMLNode_GetProperty(node, "name", 0);
  if (!p) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Unnamed plugin");
    GWEN_PluginDescription_free(pd);
    return 0;
  }
  pd->name=strdup(p);
  pd->xmlNode=GWEN_XMLNode_dup(node);
  p=GWEN_XMLNode_GetProperty(node, "type", 0);
  if (!p) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Plugin has no type");
    GWEN_PluginDescription_free(pd);
    return 0;
  }
  pd->type=strdup(p);
  p=GWEN_XMLNode_GetLocalizedCharValue(node, "version", 0);
  if (p)
    pd->version=strdup(p);
  p=GWEN_XMLNode_GetLocalizedCharValue(node, "author", 0);
  if (p)
    pd->author=strdup(p);
  p=GWEN_XMLNode_GetLocalizedCharValue(node, "short", 0);
  if (p)
    pd->shortDescr=strdup(p);
  p=GWEN_XMLNode_GetLocalizedCharValue(node, "descr", 0);
  if (p)
    pd->longDescr=strdup(p);
  return pd;
}



void GWEN_PluginDescription_free(GWEN_PLUGIN_DESCRIPTION *pd){
  if (pd) {
    DBG_MEM_DEC("GWEN_PLUGIN_DESCRIPTION");
    GWEN_LIST_FINI(GWEN_PLUGIN_DESCRIPTION, pd);
    free(pd->path);
    GWEN_XMLNode_free(pd->xmlNode);
    free(pd->fileName);
    free(pd->longDescr);
    free(pd->shortDescr);
    free(pd->author);
    free(pd->version);
    free(pd->type);
    free(pd->name);
    GWEN_FREE_OBJECT(pd);
  }
}



GWEN_PLUGIN_DESCRIPTION*
GWEN_PluginDescription_dup(const GWEN_PLUGIN_DESCRIPTION *pd) {
  GWEN_PLUGIN_DESCRIPTION *np;
  const char *s;

  assert(pd);
  GWEN_NEW_OBJECT(GWEN_PLUGIN_DESCRIPTION, np);
  DBG_MEM_INC("GWEN_PLUGIN_DESCRIPTION", 0);
  GWEN_LIST_INIT(GWEN_PLUGIN_DESCRIPTION, np);

  s=pd->fileName;
  if (s) np->fileName=strdup(s);

  s=pd->path;
  if (s) np->path=strdup(s);
  s=pd->name;
  if (s) np->name=strdup(s);
  s=pd->type;
  if (s) np->type=strdup(s);
  s=pd->shortDescr;
  if (s) np->shortDescr=strdup(s);
  s=pd->author;
  if (s) np->author=strdup(s);
  s=pd->version;
  if (s) np->version=strdup(s);
  s=pd->longDescr;
  if (s) np->longDescr=strdup(s);
  np->isActive=pd->isActive;
  if (pd->xmlNode)
    np->xmlNode=GWEN_XMLNode_dup(pd->xmlNode);

  return np;
}



GWEN_PLUGIN_DESCRIPTION*
GWEN_PluginDescription_List2_freeAll_cb(GWEN_PLUGIN_DESCRIPTION *pd, 
					void *user_data) {
  GWEN_PluginDescription_free(pd);
  return 0;
}



void GWEN_PluginDescription_List2_freeAll(GWEN_PLUGIN_DESCRIPTION_LIST2 *pdl){
  GWEN_PluginDescription_List2_ForEach
    (pdl,
     GWEN_PluginDescription_List2_freeAll_cb, 
     0);
  GWEN_PluginDescription_List2_free(pdl);
}



const char *GWEN_PluginDescription_GetPath(const GWEN_PLUGIN_DESCRIPTION *pd){
  assert(pd);
  return pd->path;
}



void GWEN_PluginDescription_SetPath(GWEN_PLUGIN_DESCRIPTION *pd,
                                    const char *s){
  assert(pd);
  free(pd->path);
  if (s) pd->path=strdup(s);
  else pd->path=0;
}



const char *GWEN_PluginDescription_GetName(const GWEN_PLUGIN_DESCRIPTION *pd){
  assert(pd);
  return pd->name;
}



const char *GWEN_PluginDescription_GetType(const GWEN_PLUGIN_DESCRIPTION *pd){
  assert(pd);
  return pd->type;
}



const char*
GWEN_PluginDescription_GetShortDescr(const GWEN_PLUGIN_DESCRIPTION *pd){
  assert(pd);
  return pd->shortDescr;
}



const char*
GWEN_PluginDescription_GetAuthor(const GWEN_PLUGIN_DESCRIPTION *pd){
  assert(pd);
  return pd->author;
}



const char*
GWEN_PluginDescription_GetVersion(const GWEN_PLUGIN_DESCRIPTION *pd){
  assert(pd);
  return pd->version;
}



const char*
GWEN_PluginDescription_GetLongDescr(const GWEN_PLUGIN_DESCRIPTION *pd){
  assert(pd);
  return pd->longDescr;
}



int
GWEN_PluginDescription__GetLongDescrByFormat(const GWEN_PLUGIN_DESCRIPTION *pd,
                                             const char *s,
                                             GWEN_BUFFER *buf){
  GWEN_XMLNODE *n;

  assert(pd);
  assert(pd->xmlNode);

  n=GWEN_XMLNode_FindFirstTag(pd->xmlNode, "descr", 0, 0);
  if (n) {
    n=GWEN_XMLNode_FindFirstTag(n, "text", "format", s);
    while (n) {
      if (0==GWEN_XMLNode_GetProperty(n, "lang", 0)) {
        GWEN_BUFFEREDIO *bio;

        bio=GWEN_BufferedIO_Buffer2_new(buf, 0);
        GWEN_BufferedIO_SetWriteBuffer(bio, 0, 256);
        if (GWEN_XMLNode_WriteToStream(n, bio,
                                       GWEN_XML_FLAGS_HANDLE_OPEN_HTMLTAGS)) {
          DBG_INFO(GWEN_LOGDOMAIN, "here");
          GWEN_BufferedIO_Abandon(bio);
          GWEN_BufferedIO_free(bio);
          return -1;
        }
        if (GWEN_BufferedIO_Close(bio)) {
          DBG_INFO(GWEN_LOGDOMAIN, "here");
          GWEN_BufferedIO_free(bio);
          return -1;
        }
        GWEN_BufferedIO_free(bio);
        return 0;
      }
      n=GWEN_XMLNode_FindNextTag(n, "text", "format", s);
    } /* while */
  }

  return -1;
}



int
GWEN_PluginDescription__GetLocalizedLongDescrByFormat(const GWEN_PLUGIN_DESCRIPTION *pd,
                                                      const char *s,
                                                      const char *lang,
                                                      GWEN_BUFFER *buf){
  GWEN_XMLNODE *n;

  assert(pd);
  assert(pd->xmlNode);

  n=GWEN_XMLNode_FindFirstTag(pd->xmlNode, "descr", 0, 0);
  if (n) {
    n=GWEN_XMLNode_FindFirstTag(n, "text", "lang", lang);
    while (n) {
      const char *fmt;

      fmt=GWEN_XMLNode_GetProperty(n, "format", 0);
      if (fmt && strcasecmp(fmt, s)==0) {
        GWEN_BUFFEREDIO *bio;

        bio=GWEN_BufferedIO_Buffer2_new(buf, 0);
        GWEN_BufferedIO_SetWriteBuffer(bio, 0, 256);
        if (GWEN_XMLNode_WriteToStream(n, bio,
                                       GWEN_XML_FLAGS_HANDLE_OPEN_HTMLTAGS)) {
          DBG_INFO(GWEN_LOGDOMAIN, "here");
          GWEN_BufferedIO_Abandon(bio);
          GWEN_BufferedIO_free(bio);
          return -1;
        }
        if (GWEN_BufferedIO_Close(bio)) {
          DBG_INFO(GWEN_LOGDOMAIN, "here");
          GWEN_BufferedIO_free(bio);
          return -1;
        }
        GWEN_BufferedIO_free(bio);
        return 0;
      }
      n=GWEN_XMLNode_FindNextTag(n, "text", "lang", lang);
    } /* while */
  }

  return -1;
}



int
GWEN_PluginDescription_GetLongDescrByFormat(const GWEN_PLUGIN_DESCRIPTION *pd,
                                            const char *s,
                                            GWEN_BUFFER *buf){
  GWEN_STRINGLIST *langl;
  int rv;

  langl=GWEN_I18N_GetCurrentLocaleList();
  if (langl) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(langl);
    while(se) {
      const char *l;

      l=GWEN_StringListEntry_Data(se);
      DBG_NOTICE(GWEN_LOGDOMAIN, "Trying locale \"%s\"", l);
      assert(l);

      rv=GWEN_PluginDescription__GetLocalizedLongDescrByFormat(pd,
                                                               s,
                                                               l,
                                                               buf);
      if (rv==0)
        return rv;

      se=GWEN_StringListEntry_Next(se);
    } /* while */
  } /* if language list available */

  /* no localized version found, return text for default language */
  rv=GWEN_PluginDescription__GetLongDescrByFormat(pd, s, buf);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}


const char*
GWEN_PluginDescription_GetFileName(const GWEN_PLUGIN_DESCRIPTION *pd){
  assert(pd);
  return pd->fileName;
}



void GWEN_PluginDescription_SetFileName(GWEN_PLUGIN_DESCRIPTION *pd,
					const char *s){
  assert(pd);
  free(pd->fileName);
  if (s) pd->fileName=strdup(s);
  else pd->fileName=0;
}



GWEN_XMLNODE*
GWEN_PluginDescription_GetXmlNode(const GWEN_PLUGIN_DESCRIPTION *pd){
  assert(pd);
  return pd->xmlNode;
}



GWEN_PLUGIN_DESCRIPTION_LIST2 *GWEN_LoadPluginDescrs(const char *path) {
  GWEN_PLUGIN_DESCRIPTION_LIST2 *pl;
  int rv;

  pl=GWEN_PluginDescription_List2_new();

  rv=GWEN_LoadPluginDescrsByType(path, 0, pl);
  if (GWEN_PluginDescription_List2_GetSize(pl)==0) {
    GWEN_PluginDescription_List2_free(pl);
    return 0;
  }
  return pl;
}



int GWEN_PluginDescription_IsActive(const GWEN_PLUGIN_DESCRIPTION *pd){
  assert(pd);
  return pd->isActive;
}



void GWEN_PluginDescription_SetIsActive(GWEN_PLUGIN_DESCRIPTION *pd, int i){
  assert(pd);
  pd->isActive=i;
}



int GWEN_LoadPluginDescrsByType(const char *path,
                                const char *type,
                                GWEN_PLUGIN_DESCRIPTION_LIST2 *pdl){
  GWEN_DIRECTORYDATA *d;
  GWEN_BUFFER *nbuf;
  char nbuffer[64];
  unsigned int pathLen;

  if (!path)
    path="";

  /* create path */
  nbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(nbuf, path);
  pathLen=GWEN_Buffer_GetUsedBytes(nbuf);

  d=GWEN_Directory_new();
  if (GWEN_Directory_Open(d, GWEN_Buffer_GetStart(nbuf))) {
    DBG_INFO(GWEN_LOGDOMAIN,
             "Path \"%s\" is not available",
	     GWEN_Buffer_GetStart(nbuf));
    GWEN_Buffer_free(nbuf);
    GWEN_Directory_free(d);
    return -1;
  }

  while(!GWEN_Directory_Read(d,
			     nbuffer,
			     sizeof(nbuffer))) {
    if (strcmp(nbuffer, ".") &&
	strcmp(nbuffer, "..")) {
      int nlen;

      nlen=strlen(nbuffer);
      if (nlen>3) {
	if (strcasecmp(nbuffer+nlen-4, ".xml")==0) {
          struct stat st;

	  GWEN_Buffer_Crop(nbuf, 0, pathLen);
	  GWEN_Buffer_SetPos(nbuf, pathLen);
	  GWEN_Buffer_AppendByte(nbuf, '/');
	  GWEN_Buffer_AppendString(nbuf, nbuffer);

	  if (stat(GWEN_Buffer_GetStart(nbuf), &st)) {
	    DBG_ERROR(GWEN_LOGDOMAIN, "stat(%s): %s",
		      GWEN_Buffer_GetStart(nbuf),
		      strerror(errno));
	  }
	  else {
	    if (!S_ISDIR(st.st_mode)) {
              GWEN_XMLNODE *fileNode;

              fileNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
              if (GWEN_XML_ReadFile(fileNode,
				    GWEN_Buffer_GetStart(nbuf),
                                    GWEN_XML_FLAGS_DEFAULT |
                                    GWEN_XML_FLAGS_HANDLE_HEADERS |
                                    GWEN_XML_FLAGS_HANDLE_OPEN_HTMLTAGS)) {
                DBG_WARN(GWEN_LOGDOMAIN,
                         "Bad file \"%s\"", GWEN_Buffer_GetStart(nbuf));
              }
	      else {
                GWEN_XMLNODE *node;
                GWEN_XMLNODE *n;
                GWEN_STRINGLIST *langl;

                n=0;
                node=GWEN_XMLNode_FindFirstTag(fileNode, "PluginDescr", 0, 0);
                if (!node)
                  node=fileNode;
                langl=GWEN_I18N_GetCurrentLocaleList();
                if (langl) {
                  GWEN_STRINGLISTENTRY *se;

                  se=GWEN_StringList_FirstEntry(langl);
                  while(se) {
                    const char *l;

                    l=GWEN_StringListEntry_Data(se);
                    DBG_NOTICE(GWEN_LOGDOMAIN, "Trying locale \"%s\"", l);
                    assert(l);
                    n=GWEN_XMLNode_FindFirstTag(node, "plugin", "lang", l);
                    if (n)
                      break;
                    se=GWEN_StringListEntry_Next(se);
                  } /* while */
                } /* if language list available */

                if (!n)
                  n=GWEN_XMLNode_FindFirstTag(node, "plugin", 0, 0);
                if (n) {
                  GWEN_PLUGIN_DESCRIPTION *pd;
                  int loadIt;

                  loadIt=1;
                  if (type) {
                    const char *ft;

                    ft=GWEN_XMLNode_GetProperty(n, "type", 0);
                    if (!ft)
                      loadIt=0;
                    else if (strcasecmp(ft, type)!=0){
                      loadIt=0;
                    }
                  } /* if type specified */
                  if (loadIt) {
                    pd=GWEN_PluginDescription_new(n);
                    if (!pd) {
                      DBG_WARN(GWEN_LOGDOMAIN, "Bad plugin description");
                    }
                    else {
                      GWEN_PluginDescription_SetFileName
                        (pd, GWEN_Buffer_GetStart(nbuf));
                      GWEN_Buffer_Crop(nbuf, 0, pathLen);
                      GWEN_Buffer_SetPos(nbuf, pathLen);
                      GWEN_PluginDescription_SetPath
                        (pd, GWEN_Buffer_GetStart(nbuf));
                      GWEN_PluginDescription_List2_PushBack(pdl, pd);
                    }
                  } /* if loadIt */
                  else {
                    DBG_INFO(GWEN_LOGDOMAIN,
                             "Ignoring file \"%s\" (bad/missing type)",
                             GWEN_Buffer_GetStart(nbuf));
                  }
                }
                else {
                  DBG_WARN(GWEN_LOGDOMAIN,
                           "File \"%s\" does not contain a plugin "
                           "description",
                           GWEN_Buffer_GetStart(nbuf));
                }
              }
              GWEN_XMLNode_free(fileNode);
            } /* if !dir */
          } /* if stat was ok */
        } /* if XML */
      } /* if name has more than 3 chars */
    } /* if not "." and not ".." */
  } /* while */
  GWEN_Directory_Close(d);
  GWEN_Directory_free(d);
  GWEN_Buffer_free(nbuf);

  return 0;
}












