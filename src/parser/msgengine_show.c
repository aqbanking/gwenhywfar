/***************************************************************************
 begin       : Fri Jul 04 2003
 copyright   : (C) 2019 by Martin Preuss
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

/* this file is included from msgengine.c */



int GWEN_MsgEngine__ShowElement(GWEN_UNUSED GWEN_MSGENGINE *e,
                                const char *path,
                                GWEN_XMLNODE *node,
                                GWEN_STRINGLIST *sl,
                                uint32_t flags)
{
  const char *name;
  const char *type;
  const char *npath;
  unsigned int minsize;
  unsigned int maxsize;
  unsigned int minnum;
  unsigned int maxnum;
  int j;
  int isSet;
  char nbuffer[256];
  GWEN_STRINGLISTENTRY *en;

  /* get type */
  type=GWEN_XMLNode_GetProperty(node, "type", "ASCII");

  /* get some sizes */
  minsize=atoi(GWEN_XMLNode_GetProperty(node, "minsize", "0"));
  maxsize=atoi(GWEN_XMLNode_GetProperty(node, "maxsize", "0"));
  minnum=atoi(GWEN_XMLNode_GetProperty(node, "minnum", "1"));
  maxnum=atoi(GWEN_XMLNode_GetProperty(node, "maxnum", "1"));

  npath="";
  isSet=0;

  /* get name */
  name=GWEN_XMLNode_GetProperty(node, "name", 0);
  if (path==0)
    path="";

  if (name) {
    /* get value of a config variable */
    if (strlen(path)+strlen(name)+10>=sizeof(nbuffer)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Buffer too small");
      return -1;
    }
    if (*path)
      sprintf(nbuffer, "%s/%s", path, name);
    else
      sprintf(nbuffer, "%s", name);
    npath=nbuffer;
  }

  en=GWEN_StringList_FirstEntry(sl);
  while (en) {
    if (GWEN_StringListEntry_Data(en))
      if (strcasecmp(GWEN_StringListEntry_Data(en), npath)==0) {
        isSet=1;
        break;
      }
    en=GWEN_StringListEntry_Next(en);
  } /* while */

  if (isSet && (flags & GWEN_MSGENGINE_SHOW_FLAGS_NOSET))
    return 0;

  fprintf(stdout, "  %s",
          npath);
  j=GWEN_MSGENGINE_VARNAME_WIDTH-strlen(npath);
  if (j>0) {
    int i;

    for (i=0; i<j; i++)
      fprintf(stdout, " ");
  }
  fprintf(stdout, "| %s", type);
  j=GWEN_MSGENGINE_TYPENAME_WIDTH-strlen(type);
  if (j>0) {
    int i;

    for (i=0; i<j; i++)
      fprintf(stdout, " ");
  }
  fprintf(stdout, "| %4d-%4d", minsize, maxsize);
  fprintf(stdout, " | %3d ", maxnum);
  fprintf(stdout, " |");
  if (minnum==0)
    fprintf(stdout, " optvar");
  if (flags & GWEN_MSGENGINE_SHOW_FLAGS_OPTIONAL)
    fprintf(stdout, " optgrp");

  if (isSet) {
    fprintf(stdout, " set");
  }

  fprintf(stdout, "\n");

  return 0;
}



int GWEN_MsgEngine__ShowGroup(GWEN_MSGENGINE *e,
                              const char *path,
                              GWEN_XMLNODE *node,
                              GWEN_STRINGLIST *sl,
                              uint32_t flags)
{
  GWEN_XMLNODE *n;
  //int isFirstElement;
  /*int omittedElements;*/
  int rv;

  /* setup data */
  n=GWEN_XMLNode_GetChild(node);

  if (path==0)
    path="";
  if (*path=='/')
    path++;

  while (n) {
    if (GWEN_XMLNode_GetType(n)==GWEN_XMLNodeTypeTag) {
      const char *p;

      p=GWEN_XMLNode_GetData(n);
      assert(p);
      DBG_DEBUG(GWEN_LOGDOMAIN, "Checking %s", p);
      if (strcasecmp(p, "VALUES")==0)
        break;
    } /* if tag */
    n=GWEN_XMLNode_Next(n);
  } /* while */

  if (n) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "<preset> found");
    /* <preset> found, handle all values */
    n=GWEN_XMLNode_GetChild(n);
    while (n) {
      if (GWEN_XMLNode_GetType(n)==GWEN_XMLNodeTypeTag) {
        const char *p;

        p=GWEN_XMLNode_GetData(n);
        assert(p);
        if (strcasecmp(p, "VALUE")==0) {
          const char *pname;
          const char *pvalue;

          pname=GWEN_XMLNode_GetProperty(n, "path", 0);
          if (pname) {
            GWEN_XMLNODE *dn;

            /* path found, find data */
            dn=GWEN_XMLNode_GetChild(n);
            while (dn) {
              if (GWEN_XMLNode_GetType(dn)==GWEN_XMLNodeTypeData) {
                pvalue=GWEN_XMLNode_GetData(dn);
                if (pvalue) {
                  char pbuffer[256];

                  /* check whether the value is a property */
                  p=pvalue;
                  while (*p && isspace((int)*p))
                    p++;
                  if (strlen(path)+strlen(pname)+2>sizeof(pbuffer)) {
                    DBG_ERROR(GWEN_LOGDOMAIN, "Buffer too small");
                    return -1;
                  }
                  if (*path)
                    sprintf(pbuffer, "%s/%s", path, pname);
                  else
                    sprintf(pbuffer, "%s", pname);
                  GWEN_StringList_AppendString(sl,
                                               pbuffer,
                                               0,
                                               1);
                }
                break;
              }
              dn=GWEN_XMLNode_Next(dn);
            } /* while dn */
          } /* if path given */
        } /* if VALUE tag */
      } /* if TAG */
      n=GWEN_XMLNode_Next(n);
    } /* while */
  } /* if <preset> found */

  /* now handle all child entries */
  n=GWEN_XMLNode_GetChild(node);
  //isFirstElement=1;
  /*omittedElements=0;*/
  while (n) {
    int t;
    unsigned int minnum;
    unsigned int maxnum;
    int gversion;
    //const char *addEmptyMode;
    unsigned int loopNr;
    unsigned int lflags;

    minnum=atoi(GWEN_XMLNode_GetProperty(n, "minnum", "1"));
    maxnum=atoi(GWEN_XMLNode_GetProperty(n, "maxnum", "1"));
    gversion=atoi(GWEN_XMLNode_GetProperty(n, "version", "0"));
    //addEmptyMode=GWEN_XMLNode_GetProperty(n, "addemptymode","one");

    lflags=flags;

    /*DBG_DEBUG(GWEN_LOGDOMAIN, "Omitted elements: %d", omittedElements);*/
    t=GWEN_XMLNode_GetType(n);
    if (t==GWEN_XMLNodeTypeTag) {
      const char *typ;

      typ=GWEN_XMLNode_GetData(n);
      if (typ==0) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Unnamed tag found (internal error?)");
        return -1;
      }
      if (strcasecmp(typ, "ELEM")==0) {
        /* element tag found */

        /* write element as often as needed */
        rv=GWEN_MsgEngine__ShowElement(e,
                                       path,
                                       n,
                                       sl,
                                       lflags);
        if (rv==-1)
          return -1;
        else {
          //isFirstElement=0;
          /*omittedElements=0;*/
        }
      }
      else if (strcasecmp(typ, "VALUES")==0) {
      }
      else if (strcasecmp(typ, "DESCR")==0) {
      }
      else {
        /* group tag found */
        GWEN_XMLNODE *gn;
        const char *gname;
        const char *gtype;

        if (minnum==0)
          lflags|=GWEN_MSGENGINE_SHOW_FLAGS_OPTIONAL;

        gtype=GWEN_XMLNode_GetProperty(n, "type", 0);
        if (!gtype) {
          /* no "type" property, so use this group directly */
          DBG_DEBUG(GWEN_LOGDOMAIN, "<%s> tag has no \"type\" property", typ);
          gtype="";
          gn=n;
        }
        else {
          gn=GWEN_MsgEngine_FindNodeByProperty(e, typ, "id", gversion, gtype);
          if (!gn) {
            DBG_DEBUG(GWEN_LOGDOMAIN, "Definition for type \"%s\" not found", typ);
            return -1;
          }
        }

        /* write group as often as needed */
        for (loopNr=0; loopNr<maxnum; loopNr++) {
          /* find group */
          char pbuffer[256];
          const char *npath;

          /* get configuration */
          gname=GWEN_XMLNode_GetProperty(n, "name", 0);
          if (gname) {
            if (loopNr==0) {
              if (strlen(path)+strlen(gname)+1>sizeof(pbuffer)) {
                DBG_ERROR(GWEN_LOGDOMAIN, "Buffer too small");
                return -1;
              }
              sprintf(pbuffer, "%s/%s", path, gname);
              npath=pbuffer;
            }
            else {
              /* this is not the first one, so create new name */
              if (strlen(path)+strlen(gname)+10>sizeof(pbuffer)) {
                DBG_ERROR(GWEN_LOGDOMAIN, "Buffer too small");
                return -1;
              }
              if (*path)
                sprintf(pbuffer, "%s/%s%d", path, gname, loopNr);
              else
                sprintf(pbuffer, "%s%d", gname, loopNr);
              /* get the value of the given var */
              npath=pbuffer;
            }
          } /* if name given */
          else
            npath=path;

          /* write group */
          if (GWEN_MsgEngine__ShowGroup(e, npath, gn, sl, lflags)) {
            DBG_INFO(GWEN_LOGDOMAIN, "Could not show group \"%s\"", gtype);
            return -1;
          }
        } /* for */
      }
    }
    n=GWEN_XMLNode_Next(n);
  } /* while */

  return 0;
}



int GWEN_MsgEngine_ShowMessage(GWEN_MSGENGINE *e,
                               const char *typ,
                               const char *msgName,
                               int msgVersion,
                               uint32_t flags)
{
  GWEN_XMLNODE *group;
  GWEN_STRINGLIST *sl;
  int i, j;
  const char *p;

  sl=GWEN_StringList_new();

  fprintf(stdout, "Message \"%s\" version %d\n",
          msgName, msgVersion);
  for (i=0; i<76; i++)
    fprintf(stdout, "=");
  fprintf(stdout, "\n");
  p="        Variable";
  fprintf(stdout, "%s", p);
  i=GWEN_MSGENGINE_VARNAME_WIDTH-strlen(p);
  for (j=0; j<i; j++)
    fprintf(stdout, " ");

  fprintf(stdout, "  |");
  p=" Type";
  fprintf(stdout, "%s", p);
  i=GWEN_MSGENGINE_TYPENAME_WIDTH-strlen(p);
  for (j=0; j<i; j++)
    fprintf(stdout, " ");

  fprintf(stdout, " |   Size    | Num  | Flags\n");
  for (i=0; i<76; i++)
    fprintf(stdout, "-");
  fprintf(stdout, "\n");

  group=GWEN_MsgEngine_FindNodeByProperty(e, typ, "id", msgVersion, msgName);
  if (!group) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Group \"%s\" not found\n", msgName);
    GWEN_StringList_free(sl);
    return -1;
  }

  if (GWEN_MsgEngine__ShowGroup(e,
                                "",
                                group,
                                sl,
                                flags)) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error showing group \"%s\"", msgName);
    GWEN_StringList_free(sl);
    return -1;
  }

  GWEN_StringList_free(sl);

  return 0;
}



int GWEN_MsgEngine__ListElement(GWEN_UNUSED GWEN_MSGENGINE *e,
                                const char *path,
                                GWEN_XMLNODE *node,
                                GWEN_STRINGLIST *sl,
                                GWEN_XMLNODE *listNode,
                                uint32_t flags)
{
  const char *name;
  //const char *type;
  const char *npath;
  int isSet;
  char nbuffer[256];
  GWEN_STRINGLISTENTRY *en;
  GWEN_XMLNODE *nn;

  /* get type */
  //type=GWEN_XMLNode_GetProperty(node, "type","ASCII");

  npath="";
  isSet=0;

  /* get name */
  name=GWEN_XMLNode_GetProperty(node, "name", 0);
  if (path==0)
    path="";

  if (name) {
    /* get value of a config variable */
    if (strlen(path)+strlen(name)+10>=sizeof(nbuffer)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Buffer too small");
      return -1;
    }
    if (*path)
      sprintf(nbuffer, "%s/%s", path, name);
    else
      sprintf(nbuffer, "%s", name);
    npath=nbuffer;
  }

  en=GWEN_StringList_FirstEntry(sl);
  while (en) {
    if (GWEN_StringListEntry_Data(en))
      if (strcasecmp(GWEN_StringListEntry_Data(en), npath)==0) {
        isSet=1;
        break;
      }
    en=GWEN_StringListEntry_Next(en);
  } /* while */

  if (isSet && (flags & GWEN_MSGENGINE_SHOW_FLAGS_NOSET))
    return 0;

  nn=GWEN_XMLNode_dup(node);
  if (isSet)
    GWEN_XMLNode_SetProperty(nn, "GWEN_set", "1");
  GWEN_XMLNode_SetProperty(nn, "GWEN_path", npath);
  GWEN_XMLNode_AddChild(listNode, nn);

  return 0;
}



int GWEN_MsgEngine__ListGroup(GWEN_MSGENGINE *e,
                              const char *path,
                              GWEN_XMLNODE *node,
                              GWEN_STRINGLIST *sl,
                              GWEN_XMLNODE *listNode,
                              uint32_t flags)
{
  GWEN_XMLNODE *n;
  int rv;

  /* setup data */
  n=GWEN_XMLNode_GetChild(node);

  if (path==0)
    path="";
  if (*path=='/')
    path++;

  while (n) {
    if (GWEN_XMLNode_GetType(n)==GWEN_XMLNodeTypeTag) {
      const char *p;

      p=GWEN_XMLNode_GetData(n);
      assert(p);
      DBG_DEBUG(GWEN_LOGDOMAIN, "Checking %s", p);
      if (strcasecmp(p, "VALUES")==0)
        break;
    } /* if tag */
    n=GWEN_XMLNode_Next(n);
  } /* while */

  if (n) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "<values> found");
    /* <values> found, handle all values */
    n=GWEN_XMLNode_GetChild(n);
    while (n) {
      if (GWEN_XMLNode_GetType(n)==GWEN_XMLNodeTypeTag) {
        const char *p;

        p=GWEN_XMLNode_GetData(n);
        assert(p);
        if (strcasecmp(p, "VALUE")==0) {
          const char *pname;
          const char *pvalue;

          pname=GWEN_XMLNode_GetProperty(n, "path", 0);
          if (pname) {
            GWEN_XMLNODE *dn;

            /* path found, find data */
            dn=GWEN_XMLNode_GetChild(n);
            while (dn) {
              if (GWEN_XMLNode_GetType(dn)==GWEN_XMLNodeTypeData) {
                pvalue=GWEN_XMLNode_GetData(dn);
                if (pvalue) {
                  char pbuffer[256];

                  /* check whether the value is a property */
                  p=pvalue;
                  while (*p && isspace((int)*p))
                    p++;
                  if (strlen(path)+strlen(pname)+2>sizeof(pbuffer)) {
                    DBG_ERROR(GWEN_LOGDOMAIN, "Buffer too small");
                    return -1;
                  }
                  if (*path)
                    sprintf(pbuffer, "%s/%s", path, pname);
                  else
                    sprintf(pbuffer, "%s", pname);
                  DBG_INFO(GWEN_LOGDOMAIN, "Found preset value for %s", pbuffer);
                  GWEN_StringList_AppendString(sl,
                                               pbuffer,
                                               0,
                                               1);
                }
                break;
              }
              dn=GWEN_XMLNode_Next(dn);
            } /* while dn */
          } /* if path given */
        } /* if VALUE tag */
      } /* if TAG */
      n=GWEN_XMLNode_Next(n);
    } /* while */
  } /* if <values> found */

  /* now handle all child entries */
  n=GWEN_XMLNode_GetChild(node);
  while (n) {
    int t;
    int gversion;
    unsigned int lflags;

    gversion=atoi(GWEN_XMLNode_GetProperty(n, "version", "0"));
    lflags=flags;

    t=GWEN_XMLNode_GetType(n);
    if (t==GWEN_XMLNodeTypeTag) {
      const char *typ;

      typ=GWEN_XMLNode_GetData(n);
      if (typ==0) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Unnamed tag found (internal error?)");
        return -1;
      }
      if (strcasecmp(typ, "ELEM")==0) {
        /* element tag found */

        /* list element */
        rv=GWEN_MsgEngine__ListElement(e,
                                       path,
                                       n,
                                       sl,
                                       listNode,
                                       lflags);
        if (rv==-1)
          return -1;
      }
      else if (strcasecmp(typ, "VALUES")==0) {
      }
      else if (strcasecmp(typ, "DESCR")==0) {
      }
      else {
        /* group tag found */
        GWEN_XMLNODE *gn;
        GWEN_XMLNODE *nn;
        const char *gname;
        const char *gtype;
        char pbuffer[256];
        const char *npath;

        gtype=GWEN_XMLNode_GetProperty(n, "type", 0);
        if (!gtype) {
          /* no "type" property, so use this group directly */
          DBG_DEBUG(GWEN_LOGDOMAIN, "<%s> tag has no \"type\" property", typ);
          gtype="";
          gn=n;
        }
        else {
          gn=GWEN_MsgEngine_FindNodeByProperty(e, typ, "id", gversion, gtype);
          if (!gn) {
            DBG_DEBUG(GWEN_LOGDOMAIN, "Definition for type \"%s\" not found", typ);
            return -1;
          }
        }

        /* get configuration */
        gname=GWEN_XMLNode_GetProperty(n, "name", 0);
        if (gname) {
          if (strlen(path)+strlen(gname)+1>sizeof(pbuffer)) {
            DBG_ERROR(GWEN_LOGDOMAIN, "Buffer too small");
            return -1;
          }

          if (*path)
            sprintf(pbuffer, "%s/%s", path, gname);
          else
            sprintf(pbuffer, "%s", gname);
          npath=pbuffer;
        } /* if name given */
        else
          npath=path;

        nn=GWEN_XMLNode_dup(n);
        if (gn!=n)
          GWEN_XMLNode_CopyProperties(nn, gn, 0);
        GWEN_XMLNode_SetProperty(nn, "GWEN_path", npath);
        GWEN_XMLNode_AddChild(listNode, nn);

        /* write group */
        if (GWEN_MsgEngine__ListGroup(e, npath, gn, sl, nn, lflags)) {
          DBG_INFO(GWEN_LOGDOMAIN, "Could not list group \"%s\"", gtype);
          return -1;
        }
      }
    }
    n=GWEN_XMLNode_Next(n);
  } /* while */

  return 0;
}



GWEN_XMLNODE *GWEN_MsgEngine_ListMessage(GWEN_MSGENGINE *e,
                                         const char *typ,
                                         const char *msgName,
                                         int msgVersion,
                                         uint32_t flags)
{
  GWEN_XMLNODE *group;
  GWEN_STRINGLIST *sl;
  GWEN_XMLNODE *listNode;

  group=GWEN_MsgEngine_FindNodeByProperty(e, typ, "id", msgVersion, msgName);
  if (!group)
    group=GWEN_MsgEngine_FindNodeByProperty(e, typ, "code",
                                            msgVersion, msgName);
  if (!group) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Group \"%s\" (version %d) not found\n",
              msgName, msgVersion);
    return 0;
  }

  sl=GWEN_StringList_new();
  /* copy group, but remove all children (we only want the properties) */
  listNode=GWEN_XMLNode_dup(group);
  GWEN_XMLNode_RemoveChildren(listNode);

  if (GWEN_MsgEngine__ListGroup(e, "", group, sl, listNode, flags)) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error showing group \"%s\"", msgName);
    GWEN_StringList_free(sl);
    GWEN_XMLNode_free(listNode);
    return 0;
  }

  GWEN_StringList_free(sl);

  return listNode;
}



