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
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/directory.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>



GWEN_LIST_FUNCTIONS(GWEN_PLUGIN_DESCRIPTION, GWEN_PluginDescription);
GWEN_LIST2_FUNCTIONS(GWEN_PLUGIN_DESCRIPTION, GWEN_PluginDescription);



GWEN_PLUGIN_DESCRIPTION *GWEN_PluginDescription_new(GWEN_XMLNODE *node){
  GWEN_PLUGIN_DESCRIPTION *pd;
  const char *p;

  GWEN_NEW_OBJECT(GWEN_PLUGIN_DESCRIPTION, pd);
  GWEN_LIST_INIT(GWEN_PLUGIN_DESCRIPTION, pd);
  p=GWEN_XMLNode_GetProperty(node, "name", 0);
  if (!p) {
    DBG_ERROR(0, "Unnamed plugin");
    GWEN_PluginDescription_free(pd);
    return 0;
  }
  pd->name=strdup(p);
  p=GWEN_XMLNode_GetCharValue(node, "type", 0);
  if (!p) {
    DBG_ERROR(0, "Plugin has no type");
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



void GWEN_PluginDescription_free(GWEN_PLUGIN_DESCRIPTION *pd){
  if (pd) {
    GWEN_LIST_FINI(GWEN_PLUGIN_DESCRIPTION, pd);
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
GWEN_PluginDescription_freeAll_cb(GWEN_PLUGIN_DESCRIPTION *pd) {
  GWEN_PluginDescription_free(pd);
  return 0;
}



void GWEN_PluginDescription_freeAll(GWEN_PLUGIN_DESCRIPTION_LIST2 *pdl){
  GWEN_PluginDescription_List2_ForEach(pdl,
                                       GWEN_PluginDescription_freeAll_cb);
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




GWEN_PLUGIN_DESCRIPTION_LIST2 *GWEN_LoadPluginDescrs(const char *path) {
  GWEN_DIRECTORYDATA *d;
  GWEN_BUFFER *nbuf;
  char nbuffer[64];
  unsigned int pathLen;
  GWEN_PLUGIN_DESCRIPTION_LIST2 *pdl;

  if (!path)
    path="";

  /* create path */
  nbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(nbuf, path);
  pathLen=GWEN_Buffer_GetUsedBytes(nbuf);

  d=GWEN_Directory_new();
  if (GWEN_Directory_Open(d, GWEN_Buffer_GetStart(nbuf))) {
    DBG_INFO(0, "Path \"%s\" is not available",
	     GWEN_Buffer_GetStart(nbuf));
    GWEN_Buffer_free(nbuf);
    GWEN_Directory_free(d);
    return 0;
  }

  pdl=GWEN_PluginDescription_List2_new();
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
	    DBG_ERROR(0, "stat(%s): %s",
		      GWEN_Buffer_GetStart(nbuf),
		      strerror(errno));
	  }
	  else {
	    if (!S_ISDIR(st.st_mode)) {
	      GWEN_XMLNODE *node;

	      node=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
	      if (GWEN_XML_ReadFile(node,
				    GWEN_Buffer_GetStart(nbuf),
				    GWEN_XML_FLAGS_DEFAULT)) {
		DBG_WARN(0, "Bad file \"%s\"", GWEN_Buffer_GetStart(nbuf));
	      }
	      else {
		GWEN_XMLNODE *n;

                n=GWEN_XMLNode_FindFirstTag(node, "plugin", 0, 0);
		if (n) {
		  GWEN_PLUGIN_DESCRIPTION *pd;

		  pd=GWEN_PluginDescription_new(n);
		  if (!pd) {
		    DBG_WARN(0, "Bad plugin description");
		  }
		  else {
                    GWEN_PluginDescription_List2_PushBack(pdl, pd);
                  }
                }
                else {
                  DBG_WARN(0,
                           "File \"%s\" does not contain plugin "
                           "description",
                           GWEN_Buffer_GetStart(nbuf));
                }
              }
              GWEN_XMLNode_free(node);
            } /* if !dir */
          } /* if stat was ok */
        } /* if XML */
      } /* if name has more than 3 chars */
    } /* if not "." and not ".." */
  } /* while */
  GWEN_Directory_Close(d);
  GWEN_Directory_free(d);
  GWEN_Buffer_free(nbuf);

  if (GWEN_PluginDescription_List2_GetSize(pdl)==0) {
    GWEN_PluginDescription_List2_free(pdl);
    return 0;
  }

  return pdl;
}
