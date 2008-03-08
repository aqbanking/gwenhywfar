/***************************************************************************
 begin       : Wed Feb 27 2008
 copyright   : (C) 2008 by Martin Preuss
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




GWEN_XMLNODE_NAMESPACE *GWEN_XMLGL__FindNameSpaceByUrl(GWEN_XMLNODE_NAMESPACE_LIST *l,
						       const char *s) {
  GWEN_XMLNODE_NAMESPACE *ns;

  assert(l);
  ns=GWEN_XMLNode_NameSpace_List_First(l);
  while(ns) {
    const char *d;

    d=GWEN_XMLNode_NameSpace_GetUrl(ns);
    if (d && strcasecmp(d, s)==0)
      return ns;
    ns=GWEN_XMLNode_NameSpace_List_Next(ns);
  }

  return NULL;
}



GWEN_XMLNODE_NAMESPACE *GWEN_XMLGL__GetNameSpaceByPrefix(GWEN_XMLNODE *n,
							 const char *prefix) {
  while(n) {
    if (n->type==GWEN_XMLNodeTypeTag) {
      GWEN_XMLNODE_NAMESPACE *ns;

      DBG_ERROR(0, "Checking in node [%s]", GWEN_XMLNode_GetData(n));
      ns=GWEN_XMLNode_NameSpace_List_First(n->nameSpaces);
      while(ns) {
	const char *d;

	d=GWEN_XMLNode_NameSpace_GetName(ns);
	if (d && strcasecmp(d, prefix)==0)
	  return ns;
	ns=GWEN_XMLNode_NameSpace_List_Next(ns);
      }
    }

    n=n->parent;
  }

  return NULL;
}



int GWEN_XMLGL__TranslateName(GWEN_XMLNODE *n,
			      GWEN_XMLNODE_NAMESPACE_LIST *l,
			      char **pValue) {
  GWEN_XMLNODE_NAMESPACE *ns;
  char *dcopy=NULL;
  char *v;
  const char *prefix;
  const char *name;
  
  /* split into prefix and value */
  dcopy=strdup(*pValue);
  v=strchr(dcopy, ':');
  if (v) {
    *v=0;
    prefix=dcopy;
    name=v+1;
  }
  else {
    prefix="";
    name=dcopy;
  }

  /* find definition for namespace in this and parent nodes */
  ns=GWEN_XMLGL__GetNameSpaceByPrefix(n, prefix);
  if (ns) {
    GWEN_XMLNODE_NAMESPACE *newNs;

    /* find new namespace in list of redefined namespaces */
    newNs=GWEN_XMLGL__FindNameSpaceByUrl(l, GWEN_XMLNode_NameSpace_GetUrl(ns));
    if (newNs) {
      char *newValue;

      /* translate prefix part of the name */
      newValue=(char*)malloc(strlen(GWEN_XMLNode_NameSpace_GetName(newNs))+
			     strlen(name)+1+1);
      assert(newValue);
      strcpy(newValue, GWEN_XMLNode_NameSpace_GetName(newNs));
      strcat(newValue, ":");
      strcat(newValue, name);
      free(*pValue);
      *pValue=newValue;
    }
    else {
      DBG_ERROR(GWEN_LOGDOMAIN,
		"Namespace for [%s] not in list, should not happen.",
		GWEN_XMLNode_NameSpace_GetUrl(ns));
      abort();
    }
    free(dcopy);
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "No definition for namespace \"%s\" found", prefix);
    free(dcopy);
    return GWEN_ERROR_NO_DATA;
  }

  return 0;
}



int GWEN_XMLGL__SampleNameSpaces(GWEN_XMLNODE *n,
				 GWEN_XMLNODE_NAMESPACE_LIST *l,
				 uint32_t *pLastId) {
  GWEN_XMLNODE *nn;

  if (n->type==GWEN_XMLNodeTypeTag) {
    GWEN_XMLNODE_NAMESPACE *ns;
    GWEN_XMLPROPERTY *pr;
    int rv;
  
    ns=GWEN_XMLNode_NameSpace_List_First(n->nameSpaces);
    while(ns) {
      const char *url;
  
      url=GWEN_XMLNode_NameSpace_GetUrl(ns);
      if (url) {
	if (GWEN_XMLGL__FindNameSpaceByUrl(l, url)==NULL) {
	  char namebuf[32];
	  GWEN_XMLNODE_NAMESPACE *newNs;
  
	  snprintf(namebuf, sizeof(namebuf)-1, "_%d_", ++(*pLastId));
	  newNs=GWEN_XMLNode_NameSpace_new(namebuf, url);
	  GWEN_XMLNode_NameSpace_List_Add(newNs, l);
	}
      }
      ns=GWEN_XMLNode_NameSpace_List_Next(ns);
    }
  
    /* translate some properties */
    pr=n->properties;
    while(pr) {
      if (pr->name && pr->value) {
	if (strcasecmp(pr->name, "type")==0 ||
	    strcasecmp(pr->name, "ref")==0 ||
	    strcasecmp(pr->name, "base")==0) {
	  rv=GWEN_XMLGL__TranslateName(n, l, &(pr->value));
	  if (rv) {
	    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
            return rv;
	  }
	}
      }
      pr=pr->next;
    }
  
    /* translate this node */
    if (n->data) {
      rv=GWEN_XMLGL__TranslateName(n, l, &(n->data));
      if (rv) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }
    }
  }

  /* sample and rename children */
  nn=GWEN_XMLNode_List_First(n->children);
  while(nn) {
    int rv;

    rv=GWEN_XMLGL__SampleNameSpaces(nn, l, pLastId);
    if (rv)
      return rv;
    nn=GWEN_XMLNode_List_Next(nn);
  }

  return 0;
}



void GWEN_XMLGL__ClearNameSpaces(GWEN_XMLNODE *n) {
  GWEN_XMLNODE *nn;

  GWEN_XMLNode_NameSpace_List_Clear(n->nameSpaces);

  nn=GWEN_XMLNode_List_First(n->children);
  while(nn) {
    GWEN_XMLGL__ClearNameSpaces(nn);
    nn=GWEN_XMLNode_List_Next(nn);
  }
}



int GWEN_XMLNode_GlobalizeWithList(GWEN_XMLNODE *n,
				   GWEN_XMLNODE_NAMESPACE_LIST *l,
				   uint32_t *pLastId) {
  int rv;

  rv=GWEN_XMLGL__SampleNameSpaces(n, l, pLastId);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  GWEN_XMLGL__ClearNameSpaces(n);

  return 0;
}



int GWEN_XMLNode_Globalize(GWEN_XMLNODE *n) {
  GWEN_XMLNODE_NAMESPACE_LIST *l;
  uint32_t lastId=0;
  int rv;

  l=GWEN_XMLNode_NameSpace_List_new();
  rv=GWEN_XMLNode_GlobalizeWithList(n, l, &lastId);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_XMLNode_NameSpace_List_free(l);
    return rv;
  }
  GWEN_XMLNode_NameSpace_List_free(n->nameSpaces);
  n->nameSpaces=l;

  return 0;
}












