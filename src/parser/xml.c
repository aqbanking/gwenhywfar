/***************************************************************************
 begin       : Sat Jun 28 2003
 copyright   : (C) 2003-2010 by Martin Preuss
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

#include "xml_p.h"
#include "xmlctx_l.h"
#include "i18n_l.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/path.h>
#include <gwenhywfar/fastbuffer.h>
#include <gwenhywfar/syncio_file.h>
#include <gwenhywfar/syncio_memory.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif
#include <errno.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif


#define GWEN_XML_BUFFERSIZE 512



GWEN_LIST_FUNCTIONS(GWEN_XMLNODE, GWEN_XMLNode)
GWEN_LIST2_FUNCTIONS(GWEN_XMLNODE, GWEN_XMLNode)

GWEN_LIST_FUNCTIONS(GWEN_XMLNODE_NAMESPACE, GWEN_XMLNode_NameSpace)




GWEN_XMLPROPERTY *GWEN_XMLProperty_new(const char *name, const char *value)
{
  GWEN_XMLPROPERTY *p;

  GWEN_NEW_OBJECT(GWEN_XMLPROPERTY, p);
  if (name)
    p->name=GWEN_Memory_strdup(name);
  if (value)
    p->value=GWEN_Memory_strdup(value);
  return p;
}



void GWEN_XMLProperty_free(GWEN_XMLPROPERTY *p)
{
  if (p) {
    GWEN_Memory_dealloc(p->name);
    GWEN_Memory_dealloc(p->value);
    GWEN_Memory_dealloc(p->nameSpace);
    GWEN_FREE_OBJECT(p);
  }
}



GWEN_XMLPROPERTY *GWEN_XMLProperty_dup(const GWEN_XMLPROPERTY *p)
{
  GWEN_XMLPROPERTY *pp;

  pp=GWEN_XMLProperty_new(p->name, p->value);
  if (p->nameSpace)
    pp->nameSpace=strdup(p->nameSpace);

  return pp;
}



void GWEN_XMLProperty_add(GWEN_XMLPROPERTY *p, GWEN_XMLPROPERTY **head)
{
  GWEN_LIST_ADD(GWEN_XMLPROPERTY, p, head);
}



void GWEN_XMLProperty_insert(GWEN_XMLPROPERTY *p, GWEN_XMLPROPERTY **head)
{
  GWEN_LIST_INSERT(GWEN_XMLPROPERTY, p, head);
}


void GWEN_XMLProperty_del(GWEN_XMLPROPERTY *p, GWEN_XMLPROPERTY **head)
{
  GWEN_LIST_DEL(GWEN_XMLPROPERTY, p, head);
}


void GWEN_XMLProperty_freeAll(GWEN_XMLPROPERTY *p)
{
  while (p) {
    GWEN_XMLPROPERTY *next;

    next=p->next;
    GWEN_XMLProperty_free(p);
    p=next;
  } /* while */
}




GWEN_XMLNODE *GWEN_XMLNode_new(GWEN_XMLNODE_TYPE t, const char *data)
{
  GWEN_XMLNODE *n;

  GWEN_NEW_OBJECT(GWEN_XMLNODE, n);
  GWEN_LIST_INIT(GWEN_XMLNODE, n);
  n->type=t;
  n->children=GWEN_XMLNode_List_new();
  n->headers=GWEN_XMLNode_List_new();
  if (data)
    n->data=GWEN_Memory_strdup(data);
  n->nameSpaces=GWEN_XMLNode_NameSpace_List_new();
  return n;
}


void GWEN_XMLNode_free(GWEN_XMLNODE *n)
{
  if (n) {
    GWEN_LIST_FINI(GWEN_XMLNODE, n);
    GWEN_XMLProperty_freeAll(n->properties);
    GWEN_Memory_dealloc(n->nameSpace);
    GWEN_Memory_dealloc(n->data);
    GWEN_XMLNode_List_free(n->headers);
    GWEN_XMLNode_List_free(n->children);
    GWEN_XMLNode_NameSpace_List_free(n->nameSpaces);
    GWEN_FREE_OBJECT(n);
  }
}


void GWEN_XMLNode_freeAll(GWEN_XMLNODE *n)
{
  while (n) {
    GWEN_XMLNODE *next;

    next=GWEN_XMLNode_List_Next(n);
    GWEN_XMLNode_free(n);
    n=next;
  } /* while */
}


GWEN_XMLNODE *GWEN_XMLNode_dup(const GWEN_XMLNODE *n)
{
  GWEN_XMLNODE *nn, *cn, *ncn;
  const GWEN_XMLPROPERTY *p;
  const GWEN_XMLNODE_NAMESPACE *nns;

  /* duplicate node itself */
  nn=GWEN_XMLNode_new(n->type, n->data);
  if (n->nameSpace)
    nn->nameSpace=strdup(n->nameSpace);

  /* duplicate properties */
  p=n->properties;
  while (p) {
    GWEN_XMLPROPERTY *np;

    np=GWEN_XMLProperty_dup(p);
    GWEN_XMLProperty_add(np, &(nn->properties));
    p=p->next;
  } /* while */

  /* duplicate children */
  cn=GWEN_XMLNode_List_First(n->children);
  while (cn) {
    ncn=GWEN_XMLNode_dup(cn);
    GWEN_XMLNode_AddChild(nn, ncn);
    cn=GWEN_XMLNode_Next(cn);
  } /* while */

  /* duplicate headers */
  cn=GWEN_XMLNode_List_First(n->headers);
  while (cn) {
    ncn=GWEN_XMLNode_dup(cn);
    GWEN_XMLNode_AddHeader(nn, ncn);
    cn=GWEN_XMLNode_Next(cn);
  } /* while */

  /* duplicate namespaces */
  nns=GWEN_XMLNode_NameSpace_List_First(n->nameSpaces);
  while (nns) {
    GWEN_XMLNODE_NAMESPACE *nnns;

    nnns=GWEN_XMLNode_NameSpace_dup(nns);
    GWEN_XMLNode_NameSpace_List_Add(nnns, nn->nameSpaces);
    nns=GWEN_XMLNode_NameSpace_List_Next(nns);
  }

  return nn;
}



const char *GWEN_XMLNode_GetProperty(const GWEN_XMLNODE *n, const char *name,
                                     const char *defaultValue)
{
  GWEN_XMLPROPERTY *p;

  assert(n);
  assert(name);
  p=n->properties;
  while (p) {
    assert(p->name);
    if (strcasecmp(p->name, name)==0)
      break;
    p=p->next;
  } /* while */

  if (p) {
    if (p->value)
      return p->value;
  }
  return defaultValue;
}



int GWEN_XMLNode_GetIntProperty(const GWEN_XMLNODE *n, const char *name,
                                int defaultValue)
{
  GWEN_XMLPROPERTY *p;

  assert(n);
  assert(name);
  p=n->properties;
  while (p) {
    assert(p->name);
    if (strcasecmp(p->name, name)==0)
      break;
    p=p->next;
  } /* while */

  if (p) {
    if (p->value) {
      int i;

      if (1==sscanf(p->value, "%i", &i))
        return i;
    }
  }
  return defaultValue;
}


void GWEN_XMLNode__SetProperty(GWEN_XMLNODE *n,
                               const char *name, const char *value,
                               int doInsert)
{
  GWEN_XMLPROPERTY *p;

  p=n->properties;
  while (p) {
    assert(p->name);
    if (strcasecmp(p->name, name)==0)
      break;
    p=p->next;
  } /* while */

  if (p) {
    GWEN_Memory_dealloc(p->value);
    if (value)
      p->value=GWEN_Memory_strdup(value);
    else
      p->value=0;
  }
  else {
    p=GWEN_XMLProperty_new(name, value);
    if (doInsert)
      GWEN_XMLProperty_insert(p, &(n->properties));
    else
      GWEN_XMLProperty_add(p, &(n->properties));
  }
}



void GWEN_XMLNode_SetProperty(GWEN_XMLNODE *n,
                              const char *name, const char *value)
{
  GWEN_XMLNode__SetProperty(n, name, value, 0);
}



void GWEN_XMLNode_SetIntProperty(GWEN_XMLNODE *n,
                                 const char *name, int value)
{
  char numbuf[256];

  snprintf(numbuf, sizeof(numbuf)-1, "%i", value);
  numbuf[sizeof(numbuf)-1]=0;
  GWEN_XMLNode__SetProperty(n, name, numbuf, 0);
}



void GWEN_XMLNode_IncUsage(GWEN_XMLNODE *n)
{
  assert(n);
  n->usage++;
}



void GWEN_XMLNode_DecUsage(GWEN_XMLNODE *n)
{
  assert(n);
  if (n->usage==0) {
    DBG_WARN(GWEN_LOGDOMAIN, "Node usage already is zero");
  }
  else
    n->usage--;
}



uint32_t GWEN_XMLNode_GetUsage(const GWEN_XMLNODE *n)
{
  assert(n);
  return n->usage;
}



const char *GWEN_XMLNode_GetData(const GWEN_XMLNODE *n)
{
  assert(n);
  return n->data;
}


void GWEN_XMLNode_SetData(GWEN_XMLNODE *n, const char *data)
{
  assert(n);
  GWEN_Memory_dealloc(n->data);
  if (data)
    n->data=GWEN_Memory_strdup(data);
  else
    n->data=0;
}



const char *GWEN_XMLNode_GetNamespace(const GWEN_XMLNODE *n)
{
  assert(n);
  return n->nameSpace;
}



void GWEN_XMLNode_SetNamespace(GWEN_XMLNODE *n, const char *s)
{
  assert(n);
  GWEN_Memory_dealloc(n->nameSpace);
  if (s)
    n->nameSpace=GWEN_Memory_strdup(s);
  else
    n->nameSpace=NULL;
}



GWEN_XMLNODE *GWEN_XMLNode_GetChild(const GWEN_XMLNODE *n)
{
  assert(n);
  return GWEN_XMLNode_List_First(n->children);
}


GWEN_XMLNODE *GWEN_XMLNode_GetParent(const GWEN_XMLNODE *n)
{
  assert(n);
  return n->parent;
}


void GWEN_XMLNode_AddChild(GWEN_XMLNODE *n, GWEN_XMLNODE *child)
{
  assert(n);
  GWEN_XMLNode_List_Add(child, n->children);
  child->parent=n;
}



void GWEN_XMLNode_AddChildrenOnly(GWEN_XMLNODE *n, GWEN_XMLNODE *nn,
                                  int copythem)
{
  GWEN_XMLNODE *ch;

  assert(n);
  assert(nn);

  ch=GWEN_XMLNode_GetChild(nn);
  while (ch) {
    GWEN_XMLNODE *nc;

    nc=GWEN_XMLNode_Next(ch);
    if (!copythem) {
      GWEN_XMLNode_UnlinkChild(nn, ch);
      GWEN_XMLNode_AddChild(n, ch);
    }
    else {
      GWEN_XMLNode_AddChild(n, GWEN_XMLNode_dup(ch));
    }
    ch=nc;
  } /* while */
}



GWEN_XMLNODE_TYPE GWEN_XMLNode_GetType(const GWEN_XMLNODE *n)
{
  assert(n);
  return n->type;
}


GWEN_XMLNODE *GWEN_XMLNode_Next(const GWEN_XMLNODE *n)
{
  assert(n);
  return GWEN_XMLNode_List_Next(n);
}


void GWEN_XMLNode_Dump(const GWEN_XMLNODE *n, int ind)
{
  GWEN_XMLPROPERTY *p;
  GWEN_XMLNODE *c;
  int i;
  int simpleTag;

  assert(n);

  for (i=0; i<ind; i++)
    fprintf(stderr, " ");

  simpleTag=0;
  if (n->type==GWEN_XMLNodeTypeTag) {
    if (n->data)
      fprintf(stderr, "<%s", n->data);
    else
      fprintf(stderr, "<UNKNOWN");
    p=n->properties;
    while (p) {
      if (p->value)
        fprintf(stderr, " %s=\"%s\"", p->name, p->value);
      else
        fprintf(stderr, " %s", p->name);
      p=p->next;
    }

    if (n->data) {
      if (n->data[0]=='?') {
        simpleTag=1;
        fprintf(stderr, "?");
      }
      else if (n->data[0]=='!') {
        simpleTag=1;
      }
    }

    fprintf(stderr, ">\n");
    if (!simpleTag) {
      c=GWEN_XMLNode_GetChild(n);
      while (c) {
        GWEN_XMLNode_Dump(c, ind+2);
        c=GWEN_XMLNode_Next(c);
      }
      for (i=0; i<ind; i++)
        fprintf(stderr, " ");
      if (n->data)
        fprintf(stderr, "</%s>\n", n->data);
      else
        fprintf(stderr, "</UNKNOWN>\n");
    }
  }
  else if (n->type==GWEN_XMLNodeTypeData) {
    if (n->data) {
      fprintf(stderr, "%s\n", n->data);
    }
  }
  else if (n->type==GWEN_XMLNodeTypeComment) {
    fprintf(stderr, "<!--");
    if (n->data) {
      fprintf(stderr, "%s", n->data);
    }
    fprintf(stderr, "-->\n");
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Unknown tag type (%d)", n->type);
  }
}



GWEN_XMLNODE *GWEN_XMLNode_FindNode(const GWEN_XMLNODE *node,
                                    GWEN_XMLNODE_TYPE t, const char *data)
{
  GWEN_XMLNODE *n;

  assert(node);
  assert(data);

  n=GWEN_XMLNode_GetChild(node);
  while (n) {
    if (n->type==t)
      if (n->data)
        if (strcasecmp(n->data, data)==0)
          break;
    n=GWEN_XMLNode_Next(n);
  } /* while */

  if (!n) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "Node %d:\"%s\" not found", t, data);
    return 0;
  }

  return n;
}



void GWEN_XMLNode_UnlinkChild(GWEN_XMLNODE *n, GWEN_XMLNODE *child)
{
  assert(n);
  assert(child);
  GWEN_XMLNode_List_Del(child);
  child->parent=0;
}



void GWEN_XMLNode_RemoveChildren(GWEN_XMLNODE *n)
{
  assert(n);
  GWEN_XMLNode_List_Clear(n->children);
}



void GWEN_XMLNode_CopyProperties(GWEN_XMLNODE *tn,
                                 const GWEN_XMLNODE *sn,
                                 int overwrite)
{
  const GWEN_XMLPROPERTY *sp;
  GWEN_XMLPROPERTY *tp;

  assert(tn);
  assert(sn);

  sp=sn->properties;
  while (sp) {
    GWEN_XMLPROPERTY *np;

    assert(sp->name);
    tp=tn->properties;
    /* lookup property in target */
    while (tp) {

      assert(tp->name);
      if (strcasecmp(tp->name, sp->name)==0) {
        /* property already exists */
        if (overwrite) {
          /* overwrite old property */
          GWEN_Memory_dealloc(tp->value);
          tp->value=0;
          if (sp->value)
            tp->value=GWEN_Memory_strdup(sp->value);
        }
        break;
      }
      tp=tp->next;
    } /* while */

    if (!tp) {
      /* property not found, simply copy and add it */
      np=GWEN_XMLProperty_dup(sp);
      GWEN_XMLProperty_add(np, &(tn->properties));
    }

    sp=sp->next;
  } /* while */
}



int GWEN_XMLNode_ExpandProperties(const GWEN_XMLNODE *n, GWEN_DB_NODE *dbVars)
{
  GWEN_XMLPROPERTY *sp;

  assert(n);

  sp=n->properties;
  if (sp) {
    GWEN_BUFFER *tmpBuf;

    tmpBuf=GWEN_Buffer_new(0, 256, 0, 1);

    while(sp) {
      if (sp->value) {
	int rv;

	rv=GWEN_DB_ReplaceVars(dbVars, sp->value, tmpBuf);
	if (rv<0) {
	  DBG_ERROR(GWEN_LOGDOMAIN,
		    "Error expanding value for property \"%s\": [%s] (%d)",
		    (sp->name)?(sp->name):"<no name>",
		    (sp->value)?(sp->value):"<no value>",
		    rv);
	  return rv;
	}
	GWEN_Memory_dealloc(sp->value);
	sp->value=GWEN_Memory_strdup(GWEN_Buffer_GetStart(tmpBuf));
	GWEN_Buffer_Reset(tmpBuf);
      } /* if sp->value */

      sp=sp->next;
    } /* while */
    GWEN_Buffer_free(tmpBuf);
  }

  return 0;
}



GWEN_XMLNODE *GWEN_XMLNode_GetFirstOfType(const GWEN_XMLNODE *n,
                                          GWEN_XMLNODE_TYPE t)
{
  GWEN_XMLNODE *nn;

  assert(n);
  nn=GWEN_XMLNode_GetChild(n);
  while (nn) {
    if (nn->type==t)
      return nn;
    nn=GWEN_XMLNode_Next(nn);
  } /* while */
  return 0;
}



GWEN_XMLNODE *GWEN_XMLNode_GetNextOfType(const GWEN_XMLNODE *n,
                                         GWEN_XMLNODE_TYPE t)
{
  assert(n);
  while (n) {
    if (n->type==t)
      return (GWEN_XMLNODE *)n;
    n=GWEN_XMLNode_Next(n);
  } /* while */
  return 0;
}



GWEN_XMLNODE *GWEN_XMLNode_GetFirstTag(const GWEN_XMLNODE *n)
{
  return GWEN_XMLNode_GetFirstOfType(n, GWEN_XMLNodeTypeTag);
}



GWEN_XMLNODE *GWEN_XMLNode_GetNextTag(const GWEN_XMLNODE *n)
{
  GWEN_XMLNODE *next;

  next=GWEN_XMLNode_Next(n);
  if (!next)
    return 0;
  return GWEN_XMLNode_GetNextOfType(next, GWEN_XMLNodeTypeTag);
}



GWEN_XMLNODE *GWEN_XMLNode_GetFirstData(const GWEN_XMLNODE *n)
{
  return GWEN_XMLNode_GetFirstOfType(n, GWEN_XMLNodeTypeData);
}



GWEN_XMLNODE *GWEN_XMLNode_GetNextData(const GWEN_XMLNODE *n)
{
  GWEN_XMLNODE *next;

  next=GWEN_XMLNode_Next(n);
  if (!next)
    return 0;
  return GWEN_XMLNode_GetNextOfType(next, GWEN_XMLNodeTypeData);
}



GWEN_XMLNODE *GWEN_XMLNode_FindTag(const GWEN_XMLNODE *n,
                                   const char *tname,
                                   const char *pname,
                                   const char *pvalue)
{
  while (n) {
    if (-1!=GWEN_Text_ComparePattern(n->data, tname, 0)) {
      if (pname) {
        const char *p;

        p=GWEN_XMLNode_GetProperty(n, pname, 0);
        if (p) {
          if (!pvalue)
            return (GWEN_XMLNODE *)n;
          if (-1!=GWEN_Text_ComparePattern(pvalue, p, 0))
            return (GWEN_XMLNODE *)n;
        }
        else {
          /* return this node if pvalue is 0 an the property does not exist */
          if (!pvalue)
            return (GWEN_XMLNODE *)n;
        }
      } /* if pname */
      else
        return (GWEN_XMLNODE *)n;
    }
    n=GWEN_XMLNode_GetNextTag(n);
  } /* while */
  return 0;
}



GWEN_XMLNODE *GWEN_XMLNode_FindFirstTag(const GWEN_XMLNODE *n,
                                        const char *tname,
                                        const char *pname,
                                        const char *pvalue)
{
  GWEN_XMLNODE *nn;

  nn=GWEN_XMLNode_GetFirstTag(n);
  if (!nn)
    return 0;
  return GWEN_XMLNode_FindTag(nn,
                              tname,
                              pname,
                              pvalue);
}



GWEN_XMLNODE *GWEN_XMLNode_FindNextTag(const GWEN_XMLNODE *n,
                                       const char *tname,
                                       const char *pname,
                                       const char *pvalue)
{
  GWEN_XMLNODE *nn;

  nn=GWEN_XMLNode_GetNextTag(n);
  if (!nn)
    return 0;
  return GWEN_XMLNode_FindTag(nn,
                              tname,
                              pname,
                              pvalue);
}



const char *GWEN_XMLNode_GetCharValue(const GWEN_XMLNODE *n,
                                      const char *name,
                                      const char *defValue)
{
  GWEN_XMLNODE *nn;

  if (name && *name) {
    nn=GWEN_XMLNode_FindFirstTag(n, name, 0, 0);

    while (nn) {
      GWEN_XMLNODE *dn;

      dn=GWEN_XMLNode_GetFirstData(nn);
      if (dn) {
        if (dn->data)
          return dn->data;
      }
      nn=GWEN_XMLNode_FindNextTag(nn, name, 0, 0);
    }
  }
  else {
    GWEN_XMLNODE *dn;

    dn=GWEN_XMLNode_GetFirstData(n);
    if (dn) {
      if (dn->data)
        return dn->data;
    }
  }
  return defValue;
}



const char *GWEN_XMLNode_GetLocalizedCharValue(const GWEN_XMLNODE *n,
                                               const char *name,
                                               const char *defValue)
{
  GWEN_XMLNODE *nn=0;
  GWEN_STRINGLIST *langl;

  langl=GWEN_I18N_GetCurrentLocaleList();
  if (langl) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(langl);
    while (se) {
      const char *l;

      l=GWEN_StringListEntry_Data(se);
      DBG_DEBUG(GWEN_LOGDOMAIN, "Trying locale \"%s\"", l);
      assert(l);
      nn=GWEN_XMLNode_FindFirstTag(n, name, "lang", l);
      while (nn) {
        GWEN_XMLNODE *dn;

        dn=GWEN_XMLNode_GetFirstData(nn);
        if (dn) {
          if (dn->data && *(dn->data))
            return dn->data;
        }
        nn=GWEN_XMLNode_FindNextTag(nn, name, "lang", l);
      } /* while nn */
      se=GWEN_StringListEntry_Next(se);
    } /* while */
  } /* if language list available */

  /* otherwise try without locale */
  nn=GWEN_XMLNode_FindFirstTag(n, name, 0, 0);
  while (nn) {
    GWEN_XMLNODE *dn;

    dn=GWEN_XMLNode_GetFirstData(nn);
    if (dn) {
      if (dn->data)
        return dn->data;
    }
    nn=GWEN_XMLNode_FindNextTag(nn, name, 0, 0);
  }

  return defValue;
}



void GWEN_XMLNode_SetCharValue(GWEN_XMLNODE *n,
                               const char *name,
                               const char *value)
{
  if (name && *name) {
    GWEN_XMLNODE *nn;

    nn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, name);
    if (value) {
      GWEN_XMLNODE *nnn;

      nnn=GWEN_XMLNode_new(GWEN_XMLNodeTypeData, value);
      GWEN_XMLNode_AddChild(nn, nnn);
    }
    GWEN_XMLNode_AddChild(n, nn);
  }
  else {
    GWEN_XMLNODE *nn;

    nn=GWEN_XMLNode_new(GWEN_XMLNodeTypeData, value);
    GWEN_XMLNode_AddChild(n, nn);
  }
}



int GWEN_XMLNode_GetIntValue(const GWEN_XMLNODE *n,
                             const char *name,
                             int defValue)
{
  const char *p;
  int res;

  p=GWEN_XMLNode_GetCharValue(n, name, 0);
  if (!p)
    return defValue;
  if (1!=sscanf(p, "%i", &res))
    return defValue;
  return res;
}



void GWEN_XMLNode_SetIntValue(GWEN_XMLNODE *n,
                              const char *name,
                              int value)
{
  char numbuf[32];

  snprintf(numbuf, sizeof(numbuf)-1, "%d", value);
  numbuf[sizeof(numbuf)-1]=0;
  GWEN_XMLNode_SetCharValue(n, name, numbuf);
}



int GWEN_XMLNode_SetCharValueByPath(GWEN_XMLNODE *n, uint32_t flags,
                                    const char *name,
                                    const char *value)
{
  GWEN_XMLNODE *nn;

  nn=GWEN_XMLNode_GetNodeByXPath(n, name, 0);
  if (nn) {
    GWEN_XMLNODE *nnn;

    /* clear current entries */
    if (flags & GWEN_XML_PATH_FLAGS_OVERWRITE_VALUES)
      GWEN_XMLNode_List_Clear(nn->children);

    /* create value node */
    nnn=GWEN_XMLNode_new(GWEN_XMLNodeTypeData, value);
    GWEN_XMLNode_AddChild(nn, nnn);

    return 0;
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Unable to create node [%s]", name);
    return GWEN_ERROR_INVALID;
  }
}



const char *GWEN_XMLNode_GetCharValueByPath(GWEN_XMLNODE *n,
                                            const char *name,
                                            const char *defValue)
{
  GWEN_XMLNODE *nn;

  nn=GWEN_XMLNode_GetNodeByXPath(n, name, 0);
  if (nn) {
    GWEN_XMLNODE *dn;

    dn=GWEN_XMLNode_GetFirstData(nn);
    if (dn) {
      if (dn->data)
        return dn->data;
    }
  }

  return defValue;
}



int GWEN_XMLNode_SetIntValueByPath(GWEN_XMLNODE *n, uint32_t flags,
                                   const char *name,
                                   int value)
{
  char numbuf[32];
  int rv;

  /* create int value */
  snprintf(numbuf, sizeof(numbuf)-1, "%d", value);
  numbuf[sizeof(numbuf)-1]=0;

  rv=GWEN_XMLNode_SetCharValueByPath(n, flags, name, numbuf);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return rv;
}



int GWEN_XMLNode_GetIntValueByPath(GWEN_XMLNODE *n,
                                   const char *name,
                                   int defValue)
{
  const char *p;
  int res;

  p=GWEN_XMLNode_GetCharValueByPath(n, name, NULL);
  if (!p)
    return defValue;
  if (1!=sscanf(p, "%i", &res))
    return defValue;
  return res;
}






GWEN_XMLPROPERTY *GWEN_XMLNode_GetFirstProperty(const GWEN_XMLNODE *n)
{
  assert(n);
  return n->properties;
}



GWEN_XMLPROPERTY *GWEN_XMLNode_GetNextProperty(const GWEN_XMLNODE *n,
                                               const GWEN_XMLPROPERTY *pr)
{
  assert(n);
  assert(pr);
  return pr->next;
}



const char *GWEN_XMLProperty_GetName(const GWEN_XMLPROPERTY *pr)
{
  assert(pr);
  return pr->name;
}



const char *GWEN_XMLProperty_GetValue(const GWEN_XMLPROPERTY *pr)
{
  assert(pr);
  return pr->value;
}



int GWEN_XMLNode_IsChildOf(const GWEN_XMLNODE *parent,
                           const GWEN_XMLNODE *child)
{
  GWEN_XMLNODE *n;

  if (!child || !parent || child==parent)
    return 0;
  n=child->parent;
  while (n) {
    if (n==parent)
      return 1;
    n=n->parent;
  }
  return 0;
}



int GWEN_XMLNode_GetXPath(const GWEN_XMLNODE *n1,
                          const GWEN_XMLNODE *n2,
                          GWEN_BUFFER *nbuf)
{
  GWEN_BUFFER *lbuf;
  const GWEN_XMLNODE *ln1;
  const GWEN_XMLNODE *ln2;

  if (!n1 && !n2) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Both nodes are NULL");
    return -1;
  }

  if (!n1) {
    n1=n2;
    while (n1->parent)
      n1=n1->parent;
  }

  if (!n2) {
    n2=n1;
    while (n2->parent)
      n2=n2->parent;
  }

  if (n2==n1) {
    GWEN_Buffer_AppendString(nbuf, "here()");
    return 0;
  }

  lbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_ReserveBytes(lbuf, 128);

  ln1=n1->parent;
  if (ln1) {
    GWEN_Buffer_AppendString(lbuf, "../");
    while (ln1) {
      if (ln1==n2) {
        /* found n2 */
        GWEN_Buffer_AppendBuffer(nbuf, lbuf);
        GWEN_Buffer_free(lbuf);
        return 0;
      }
      if (GWEN_XMLNode_IsChildOf(ln1, n2))
        break;
      ln1=ln1->parent;
      GWEN_Buffer_AppendString(lbuf, "../");
    }

    if (!ln1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Nodes do not share root node");
      GWEN_Buffer_free(lbuf);
      return -1;
    }

    /* append path to n1 */
    GWEN_Buffer_AppendBuffer(nbuf, lbuf);
  }
  DBG_DEBUG(GWEN_LOGDOMAIN, "Path so far: %s", GWEN_Buffer_GetStart(lbuf));

  /* get path to n2 */
  GWEN_Buffer_Reset(lbuf);

  ln2=n2;
  while (ln2) {
    GWEN_XMLNODE *tn;
    int idx;
    char idxbuf[32];

    if (ln2->parent==ln1)
      break;

    /* count occurences of this tag in this level */
    idx=1;
    tn=ln2->parent;
    if (tn) {
      tn=GWEN_XMLNode_FindFirstTag(tn, ln2->data, 0, 0);

      while (tn) {
        if (tn==ln2)
          break;
        idx++;
        tn=GWEN_XMLNode_FindNextTag(tn, ln2->data, 0, 0);
      }
    }

    snprintf(idxbuf, sizeof(idxbuf), "[%d]", idx);
    idxbuf[sizeof(idxbuf)-1]=0;
    GWEN_Buffer_InsertString(lbuf, idxbuf);
    GWEN_Buffer_InsertString(lbuf, GWEN_XMLNode_GetData(ln2));
    GWEN_Buffer_InsertByte(lbuf, '/');
    ln2=ln2->parent;
  }
  /*DBG_ERROR(GWEN_LOGDOMAIN, "Path so far: %s", GWEN_Buffer_GetStart(lbuf)); */
  assert(ln2);

  /* append path to n2 */
  GWEN_Buffer_AppendBuffer(nbuf, lbuf);
  GWEN_Buffer_free(lbuf);
  return 0;
}



void *GWEN_XMLNode_HandlePath(const char *entry,
                              void *data,
                              int idx,
                              uint32_t flags)
{
  GWEN_XMLNODE *n;
  GWEN_XMLNODE *nn;
  int i;

  n=(GWEN_XMLNODE *)data;

  if (flags & GWEN_PATH_FLAGS_VARIABLE) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "GWEN_PATH_FLAGS_VARIABLE not allowed for XPATH");
    return 0;
  }

  if (flags & GWEN_PATH_FLAGS_ROOT) {
    while (n->parent)
      n=n->parent;
    if (*entry=='/')
      entry++;
  }

  if (strcasecmp(entry, "..")==0) {
    return n->parent;
  }
  else if (strcasecmp(entry, ".")==0 ||
           strcasecmp(entry, "here()")==0) {
    return n;
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
    if (flags & GWEN_PATH_FLAGS_VARIABLE) {
      /* not allowed for now */
      return 0;
    }
    else {
      if (idx!=0) {
        DBG_ERROR(GWEN_LOGDOMAIN,
                  "Can not create tag with index!=1 (%s)", entry);
        return 0;
      }
      DBG_VERBOUS(GWEN_LOGDOMAIN,
                  "Unconditionally creating tag \"%s\"", entry);
      nn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, entry);
      GWEN_XMLNode_AddChild(n, nn);
      return nn;
    }
  }

  /* find the node */
  i=idx;
  nn=GWEN_XMLNode_FindFirstTag(n, entry, 0, 0);
  while (nn && i--) {
    nn=GWEN_XMLNode_FindNextTag(nn, entry, 0, 0);
  }

  if (!nn) {
    /* node not found, check, if we are allowed to create it */
    if (
      (!(flags & GWEN_PATH_FLAGS_LAST) &&
       (flags & GWEN_PATH_FLAGS_PATHMUSTEXIST)) ||
      (flags & GWEN_PATH_FLAGS_NAMEMUSTEXIST)
    ) {
      DBG_VERBOUS(GWEN_LOGDOMAIN,
                  "Tag \"%s\" does not exist", entry);
      return 0;
    }
    /* create the new variable/group */
    if (idx!=0) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Can not create tag with index!=1 (%s)", entry);
      return 0;
    }
    DBG_VERBOUS(GWEN_LOGDOMAIN,
                "Tag \"%s\" not found, creating", entry);
    nn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, entry);
    GWEN_XMLNode_AddChild(n, nn);
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



GWEN_XMLNODE *GWEN_XMLNode_GetNodeByXPath(GWEN_XMLNODE *n,
                                          const char *path,
                                          uint32_t flags)
{
  return (GWEN_XMLNODE *)GWEN_Path_HandleWithIdx(path,
                                                 (void *)n,
                                                 flags,
                                                 GWEN_XMLNode_HandlePath);
}



GWEN_XMLNODE *GWEN_XMLNode_GetHeader(const GWEN_XMLNODE *n)
{
  assert(n);
  return GWEN_XMLNode_List_First(n->headers);
}



void GWEN_XMLNode_AddHeader(GWEN_XMLNODE *n, GWEN_XMLNODE *nh)
{
  assert(n);
  assert(nh);
  GWEN_XMLNode_List_Add(nh, n->headers);
}



void GWEN_XMLNode_DelHeader(GWEN_XMLNODE *n, GWEN_XMLNODE *nh)
{
  assert(n);
  assert(nh);
  GWEN_XMLNode_List_Del(nh);
}



void GWEN_XMLNode_ClearHeaders(GWEN_XMLNODE *n)
{
  assert(n);
  GWEN_XMLNode_List_Clear(n->headers);
}



GWEN_XMLNODE_NAMESPACE_LIST *GWEN_XMLNode_GetNameSpaces(const GWEN_XMLNODE *n)
{
  assert(n);
  return n->nameSpaces;
}



GWEN_XMLNODE_NAMESPACE *GWEN_XMLNode_FindNameSpaceByName(const GWEN_XMLNODE *n,
                                                         const char *s)
{
  GWEN_XMLNODE_NAMESPACE *ns;

  assert(n);
  ns=GWEN_XMLNode_NameSpace_List_First(n->nameSpaces);
  while (ns) {
    const char *d;

    d=GWEN_XMLNode_NameSpace_GetName(ns);
    if (d && strcasecmp(d, s)==0)
      return ns;
    ns=GWEN_XMLNode_NameSpace_List_Next(ns);
  }

  return NULL;
}



GWEN_XMLNODE_NAMESPACE *GWEN_XMLNode_FindNameSpaceByUrl(const GWEN_XMLNODE *n,
                                                        const char *s)
{
  GWEN_XMLNODE_NAMESPACE *ns;

  assert(n);
  ns=GWEN_XMLNode_NameSpace_List_First(n->nameSpaces);
  while (ns) {
    const char *d;

    d=GWEN_XMLNode_NameSpace_GetUrl(ns);
    if (d && strcasecmp(d, s)==0)
      return ns;
    ns=GWEN_XMLNode_NameSpace_List_Next(ns);
  }

  return NULL;
}



void GWEN_XMLNode_AddNameSpace(GWEN_XMLNODE *n, const GWEN_XMLNODE_NAMESPACE *ns)
{
  assert(n);
  assert(ns);
  GWEN_XMLNode_NameSpace_List_Add(GWEN_XMLNode_NameSpace_dup(ns), n->nameSpaces);
}











int GWEN_XML_AddNameSpace(GWEN_STRINGLIST2 *sl,
                          const char *prefix,
                          const char *name)
{
  GWEN_BUFFER *nbuf;
  int rv;

  nbuf=GWEN_Buffer_new(0, 32, 0, 1);
  if (prefix)
    GWEN_Buffer_AppendString(nbuf, prefix);
  GWEN_Buffer_AppendByte(nbuf, ':');
  GWEN_Buffer_AppendString(nbuf, name);
  rv=GWEN_StringList2_AppendString(sl, GWEN_Buffer_GetStart(nbuf), 0,
                                   GWEN_StringList2_IntertMode_NoDouble);
  GWEN_Buffer_free(nbuf);
  return rv;
}



const char *GWEN_XML_FindNameSpaceByPrefix(GWEN_STRINGLIST2 *sl,
                                           const char *s)
{
  GWEN_STRINGLIST2_ITERATOR *it;

  it=GWEN_StringList2_First(sl);
  if (it) {
    const char *t;

    t=GWEN_StringList2Iterator_Data(it);
    assert(t);
    while (t) {
      const char *p;

      p=strchr(t, ':');
      assert(p);
      if ((s==0 && p==t) || (s && strncasecmp(t, s, p-t)==0))
        return t;
      t=GWEN_StringList2Iterator_Next(it);
    } /* while */
    GWEN_StringList2Iterator_free(it);
  }
  return 0;
}



const char *GWEN_XML_FindNameSpaceByName(GWEN_STRINGLIST2 *sl,
                                         const char *s)
{
  GWEN_STRINGLIST2_ITERATOR *it;

  it=GWEN_StringList2_First(sl);
  if (it) {
    const char *t;

    t=GWEN_StringList2Iterator_Data(it);
    assert(t);
    while (t) {
      const char *p;

      p=strchr(t, ':');
      assert(p);
      p++;
      if (strcasecmp(p, s)==0) {
        GWEN_StringList2Iterator_free(it);
        return t;
      }
      t=GWEN_StringList2Iterator_Next(it);
    } /* while */
    GWEN_StringList2Iterator_free(it);
  }
  return 0;
}



const char *GWEN_XML_FindNameSpace(GWEN_STRINGLIST2 *sl,
                                   const char *prefix,
                                   const char *name)
{
  GWEN_BUFFER *nbuf;
  GWEN_STRINGLIST2_ITERATOR *it;

  nbuf=GWEN_Buffer_new(0, 32, 0, 1);
  if (prefix)
    GWEN_Buffer_AppendString(nbuf, prefix);
  GWEN_Buffer_AppendByte(nbuf, ':');
  GWEN_Buffer_AppendString(nbuf, name);

  it=GWEN_StringList2_First(sl);
  if (it) {
    const char *t;

    t=GWEN_StringList2Iterator_Data(it);
    assert(t);
    while (t) {
      const char *p;

      p=strchr(t, ':');
      assert(p);
      p++;
      if (strcasecmp(p, GWEN_Buffer_GetStart(nbuf))==0) {
        GWEN_StringList2Iterator_free(it);
        GWEN_Buffer_free(nbuf);
        return t;
      }
      t=GWEN_StringList2Iterator_Next(it);
    } /* while */
    GWEN_StringList2Iterator_free(it);
  }

  GWEN_Buffer_free(nbuf);
  return 0;
}



int GWEN_XMLNode__CheckNameSpaceDecls1(GWEN_XMLNODE *n,
                                       GWEN_STRINGLIST2 *sl,
                                       const char *currentNameSpace)
{
  GWEN_XMLPROPERTY *pr;
  GWEN_XMLNODE *nn;
  char *localNameSpace;

  localNameSpace=0;

  /* remove all unnecessary namespace declarations from this node */
  pr=n->properties;
  while (pr) {
    GWEN_XMLPROPERTY *prNext;

    prNext=pr->next;
    if (strcasecmp(pr->name, "xmlns")==0) {
      /* default namespace changed ? */
      if (localNameSpace) {
        if (strcasecmp(pr->value, localNameSpace)==0) {
          /* already mentioned name space, remove duplicate property */
          GWEN_XMLProperty_del(pr, &n->properties);
          GWEN_XMLProperty_free(pr);
        }
        else {
          /* current namespace changed */
          GWEN_Memory_dealloc(localNameSpace);
          localNameSpace=GWEN_Memory_strdup(pr->value);
        }
      }
      else if (currentNameSpace) {
        if (strcasecmp(pr->value, currentNameSpace)==0) {
          /* already active name space, remove property */
          GWEN_XMLProperty_del(pr, &n->properties);
          GWEN_XMLProperty_free(pr);
        }
        else {
          /* current namespace changed */
          GWEN_Memory_dealloc(localNameSpace);
          localNameSpace=GWEN_Memory_strdup(pr->value);
        }
      }
      else {
        /* set current namespace */
        GWEN_Memory_dealloc(localNameSpace);
        localNameSpace=GWEN_Memory_strdup(pr->value);
      }
    }
    else if (strncasecmp(pr->name, "xmlns:", 6)==0) {
      const char *prefix;
      const char *x;

      prefix=strchr(pr->name, ':');
      prefix++;

      /* check for redefinition */
      x=GWEN_XML_FindNameSpaceByName(sl, prefix);
      if (x) {
        const char *p;

        /* prefix already in use, check whether it is the same namespace */
        p=strchr(x, ':');
        assert(p);
        p++;
        if (strcasecmp(p, pr->value)!=0) {
          GWEN_BUFFER *xpath;

          /* same prefix, different namespace */
          xpath=GWEN_Buffer_new(0, 256, 0, 1);
          GWEN_XMLNode_GetXPath(0, n, xpath);
          DBG_ERROR(GWEN_LOGDOMAIN,
                    "Redefinition of namespace prefix \"%s\" in \"%s\"",
                    prefix, GWEN_Buffer_GetStart(xpath));
          GWEN_Buffer_free(xpath);
          return -1;
        } /* if different namespace for same prefix */
        else {
          /* already in list, remove property here */
          GWEN_XMLProperty_del(pr, &n->properties);
          GWEN_XMLProperty_free(pr);
        }
      }
      else {
        GWEN_XML_AddNameSpace(sl, prefix, pr->value);
      }
    }
    pr=prNext;
  } /* while */

  /* do the same on all sub nodes */
  nn=GWEN_XMLNode_GetFirstTag(n);
  while (nn) {
    int rv;

    rv=GWEN_XMLNode__CheckNameSpaceDecls1(nn, sl,
                                          localNameSpace?localNameSpace:
                                          currentNameSpace);
    if (rv) {
      GWEN_Memory_dealloc(localNameSpace);
      return rv;
    }
    nn=GWEN_XMLNode_GetNextTag(nn);
  }

  GWEN_Memory_dealloc(localNameSpace);
  return 0;
}



int GWEN_XMLNode__CheckAndSetNameSpace(GWEN_XMLNODE *n,
                                       const char *prefix,
                                       const char *nspace)
{
  GWEN_XMLPROPERTY *pr;
  const char *p;
  int inUse;

  inUse=0;
  /* check current tag for prefix */
  if (prefix) {
    p=strchr(n->data, ':');
    if (p) {
      if (strncasecmp(n->data, prefix, p-n->data)==0) {
        DBG_DEBUG(GWEN_LOGDOMAIN, "Prefix \"%s\" used in tag \"%s\"",
                  prefix, n->data);
        inUse=1;
      }
    }

    if (!inUse) {
      /* check all attributes for prefixes */
      pr=n->properties;
      while (pr) {
        p=strchr(pr->name, ':');
        if (p) {
          if (strncasecmp(pr->name, prefix, p-pr->name)==0) {
            DBG_DEBUG(GWEN_LOGDOMAIN,
                      "Prefix \"%s\" used in attribute \"%s\" of tag \"%s\"",
                      prefix, pr->name, n->data);
            inUse=1;
            break;
          }
          else {
            DBG_DEBUG(GWEN_LOGDOMAIN,
                      "Prefix \"%s\" not used in attribute \"%s\" of tag \"%s\"",
                      prefix, pr->name, n->data);
          }
        }
        pr=pr->next;
      } /* while */
    }
  } /* if prefix */
  else {
    /* no prefix, check whether the current element hasn't any */
    p=strchr(n->data, ':');
    if (!p) {
      /* current tag has no prefix, check whether we have a namespace
       * declaration here */
      if (GWEN_XMLNode_GetProperty(n, "xmlns", 0)==0) {
        /* no, so the current namespace from above is used */
        DBG_DEBUG(GWEN_LOGDOMAIN,
                  "No prefix, current namespace is used");
        inUse=1;
      }
    }
  } /* if no prefix */

  if (inUse) {
    GWEN_BUFFER *nbuf;

    nbuf=GWEN_Buffer_new(0, 32, 0, 1);
    GWEN_Buffer_AppendString(nbuf, "xmlns");
    if (prefix) {
      GWEN_Buffer_AppendByte(nbuf, ':');
      GWEN_Buffer_AppendString(nbuf, prefix);
    }
    GWEN_XMLNode__SetProperty(n, GWEN_Buffer_GetStart(nbuf), nspace, 1);
    GWEN_Buffer_free(nbuf);
    return 1;
  }

  return 0;
}



int GWEN_XMLNode__SetNameSpaces(GWEN_XMLNODE *n,
                                const char *prefix,
                                const char *nspace)
{
  GWEN_XMLNODE *nn;
  int rv;

  rv=GWEN_XMLNode__CheckAndSetNameSpace(n, prefix, nspace);
  if (rv)
    return rv;

  nn=GWEN_XMLNode_GetFirstTag(n);
  while (nn) {
    rv=GWEN_XMLNode__CheckAndSetNameSpace(nn, prefix, nspace);
    if (rv==-1)
      return rv;
    else if (rv==0) {
      /* check children */
      rv=GWEN_XMLNode__SetNameSpaces(nn, prefix, nspace);
      if (rv)
        return rv;
    }

    nn=GWEN_XMLNode_GetNextTag(nn);
  }

  return 0;
}



int GWEN_XMLNode__CheckNameSpaceDecls3(GWEN_XMLNODE *n)
{
  GWEN_XMLPROPERTY *pr;
  GWEN_XMLNODE *nn;
  int rv;

  /* move all namespace declarations from this node to the nodes
   * of first use */
  pr=n->properties;
  while (pr) {
    GWEN_XMLPROPERTY *prNext;

    prNext=pr->next;
    if (strcasecmp(pr->name, "xmlns")==0 ||
        strncasecmp(pr->name, "xmlns:", 6)==0) {
      const char *prefix;

      prefix=strchr(pr->name, ':');
      if (prefix)
        prefix++;

      GWEN_XMLProperty_del(pr, &n->properties);
      rv=GWEN_XMLNode__SetNameSpaces(n, prefix, pr->value);
      DBG_DEBUG(GWEN_LOGDOMAIN, "Removing property \"%s\"",
                pr->name);
      GWEN_XMLProperty_free(pr);
      if (rv==-1)
        return rv;
    }
    pr=prNext;
  } /* while */

  /* do the same on all sub nodes */
  nn=GWEN_XMLNode_GetFirstTag(n);
  while (nn) {
    rv=GWEN_XMLNode__CheckNameSpaceDecls3(nn);
    if (rv) {
      return rv;
    }
    nn=GWEN_XMLNode_GetNextTag(nn);
  }

  return 0;
}




int GWEN_XMLNode_NormalizeNameSpaces(GWEN_XMLNODE *n)
{
  const char *ns;
  int rv;
  GWEN_STRINGLIST2 *sl;

  ns=GWEN_XMLNode_GetProperty(n, "xmlns", 0);
  sl=GWEN_StringList2_new();
  rv=GWEN_XMLNode__CheckNameSpaceDecls1(n, sl, ns);
  GWEN_StringList2_free(sl);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    return rv;
  }

  //rv=GWEN_XMLNode__CheckNameSpaceDecls2(n, ns);
  rv=GWEN_XMLNode__CheckNameSpaceDecls3(n);
  if (rv==-1)
    return rv;
  return 0;
}



int GWEN_XMLNode_StripNamespaces(GWEN_XMLNODE *n)
{
  if (n && n->type==GWEN_XMLNodeTypeTag && n->data) {
    GWEN_XMLNODE *nn;
    GWEN_XMLPROPERTY *pp;

    if (n->nameSpace==0) {
      char *p;

      p=strchr(n->data, ':');
      if (p) {
        int len=p-n->data;
        char *s;

        n->nameSpace=(char *)GWEN_Memory_malloc(len);
        assert(n->nameSpace);
        memmove(n->nameSpace, n->data, len);
        n->nameSpace[len-1]=0;
        s=GWEN_Memory_strdup(p+1);
        free(n->data);
        n->data=s;
      }
    }

    pp=n->properties;
    while (pp) {
      if (pp->nameSpace==0) {
        char *p;

        p=strchr(pp->name, ':');
        if (p) {
          int len=p-pp->name;
          char *s;

          pp->nameSpace=(char *)GWEN_Memory_malloc(len);
          assert(pp->nameSpace);
          memmove(pp->nameSpace, pp->name, len);
          pp->nameSpace[len-1]=0;
          s=GWEN_Memory_strdup(p+1);
          free(pp->name);
          pp->name=s;
        }
      }

      pp=pp->next;
    }

    nn=GWEN_XMLNode_List_First(n->children);
    while (nn) {
      int rv;

      rv=GWEN_XMLNode_StripNamespaces(nn);
      if (rv<0) {
        DBG_DEBUG(GWEN_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
      nn=GWEN_XMLNode_List_Next(nn);
    }
  }

  return 0;
}









GWEN_XMLNODE_PATH *GWEN_XMLNode_Path_new(void)
{
  GWEN_XMLNODE_PATH *p;

  GWEN_NEW_OBJECT(GWEN_XMLNODE_PATH, p);
  return p;
}



GWEN_XMLNODE_PATH *GWEN_XMLNode_Path_dup(const GWEN_XMLNODE_PATH *np)
{
  GWEN_XMLNODE_PATH *p;
  unsigned int i;

  GWEN_NEW_OBJECT(GWEN_XMLNODE_PATH, p);
  p->pos=np->pos;
  for (i=0; i<np->pos; i++) {
    p->nodes[i]=np->nodes[i];
  }
  return p;
}



void GWEN_XMLNode_Path_free(GWEN_XMLNODE_PATH *np)
{
  GWEN_FREE_OBJECT(np);
}



int GWEN_XMLNode_Path_Dive(GWEN_XMLNODE_PATH *np,
                           GWEN_XMLNODE *n)
{
  unsigned int i;

  if (np->pos>=GWEN_XML_MAX_DEPTH) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Path too deep");
    return 1;
  }

  /* check for double entries */
  for (i=0; i<np->pos; i++) {
    assert(np->nodes[i]!=n);
  }
  np->nodes[np->pos++]=n;
  DBG_DEBUG(GWEN_LOGDOMAIN, "Dived to %d", np->pos);
  return 0;
}



GWEN_XMLNODE *GWEN_XMLNode_Path_Surface(GWEN_XMLNODE_PATH *np)
{
  if (np->pos==0) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "Root reached");
    return 0;
  }
  DBG_DEBUG(GWEN_LOGDOMAIN, "Surfaced to %d", np->pos-1);
  return np->nodes[--np->pos];
}



void GWEN_XMLNode_Path_Dump(GWEN_XMLNODE_PATH *np)
{
  unsigned int i;

  if (np->pos==0) {
    DBG_NOTICE(GWEN_LOGDOMAIN, "Empty path");
  }
  for (i=0; i<np->pos; i++) {
    DBG_NOTICE(GWEN_LOGDOMAIN, "Path entry %d:", i);
    GWEN_XMLNode_Dump(np->nodes[i], 1);
  }
}











GWEN_XMLNODE_NAMESPACE *GWEN_XMLNode_NameSpace_new(const char *name,
                                                   const char *url)
{
  GWEN_XMLNODE_NAMESPACE *ns;

  GWEN_NEW_OBJECT(GWEN_XMLNODE_NAMESPACE, ns);
  GWEN_LIST_INIT(GWEN_XMLNODE_NAMESPACE, ns);

  if (name)
    ns->name=GWEN_Memory_strdup(name);
  if (url)
    ns->url=GWEN_Memory_strdup(url);

  return ns;
}



void GWEN_XMLNode_NameSpace_free(GWEN_XMLNODE_NAMESPACE *ns)
{
  if (ns) {
    GWEN_LIST_FINI(GWEN_XMLNODE_NAMESPACE, ns);
    free(ns->url);
    free(ns->name);
    GWEN_FREE_OBJECT(ns);
  }
}



GWEN_XMLNODE_NAMESPACE *GWEN_XMLNode_NameSpace_dup(const GWEN_XMLNODE_NAMESPACE *ns)
{
  GWEN_XMLNODE_NAMESPACE *nns;

  assert(ns);
  nns=GWEN_XMLNode_NameSpace_new(ns->name, ns->url);
  return nns;
}



const char *GWEN_XMLNode_NameSpace_GetName(const GWEN_XMLNODE_NAMESPACE *ns)
{
  assert(ns);
  return ns->name;
}



const char *GWEN_XMLNode_NameSpace_GetUrl(const GWEN_XMLNODE_NAMESPACE *ns)
{
  assert(ns);
  return ns->url;
}






#include "xmlrw.c"
#include "xmlglobalize.c"



