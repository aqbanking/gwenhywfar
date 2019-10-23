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



int GWEN_MsgEngine__WriteValue(GWEN_MSGENGINE *e,
                               GWEN_BUFFER *gbuf,
                               GWEN_BUFFER *data,
                               GWEN_XMLNODE *node)
{
  unsigned int minsize;
  unsigned int maxsize;
  unsigned int fixSize;
  unsigned int startPos;
  int filler;
  const char *type;
  const char *name;
  int rv;

  /* get some sizes */
  minsize=atoi(GWEN_XMLNode_GetProperty(node, "minsize", "0"));
  maxsize=atoi(GWEN_XMLNode_GetProperty(node, "maxsize", "0"));
  fixSize=atoi(GWEN_XMLNode_GetProperty(node, "size", "0"));
  filler=atoi(GWEN_XMLNode_GetProperty(node, "filler", "0"));
  type=GWEN_XMLNode_GetProperty(node, "type", "ASCII");
  name=GWEN_XMLNode_GetProperty(node, "name", "<unnamed>");
  startPos=GWEN_Buffer_GetPos(gbuf);

  /* check sizes */
  if (minsize && GWEN_Buffer_GetUsedBytes(data)<minsize) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Data too short (minsize is %d)", minsize);
    return -1;
  }
  if (maxsize && GWEN_Buffer_GetUsedBytes(data)>maxsize) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Data too long (maxsize is %d)", maxsize);
    return -1;
  }

  rv=1;
  if (e->typeWritePtr) {
    rv=e->typeWritePtr(e,
                       gbuf,
                       data,
                       node);
  }
  if (rv==-1) {
    DBG_INFO(GWEN_LOGDOMAIN, "External type writing failed");
    return -1;
  }
  else if (rv==1) {
    int i;

    /* type not handled externally, so handle it myself */
    if (strcasecmp(type, "bin")==0) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "Writing binary data (%d bytes added to %d bytes)",
                GWEN_Buffer_GetUsedBytes(data),
                GWEN_Buffer_GetUsedBytes(gbuf));
      if (GWEN_Buffer_AllocRoom(gbuf, 10+GWEN_Buffer_GetUsedBytes(data))) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Buffer too small");
        return -1;
      }
      sprintf(GWEN_Buffer_GetPosPointer(gbuf),
              "@%d@",
              GWEN_Buffer_GetUsedBytes(data));


      i=strlen(GWEN_Buffer_GetPosPointer(gbuf));
      GWEN_Buffer_IncrementPos(gbuf, i);
      GWEN_Buffer_AdjustUsedBytes(gbuf);
      GWEN_Buffer_AppendBuffer(gbuf, data);
    } /* if type is "bin" */
    else if (strcasecmp(type, "num")==0) {
      //int num;
      unsigned int len;
      unsigned int lj;

      //num=atoi(GWEN_Buffer_GetPosPointer(data));
      len=strlen(GWEN_Buffer_GetPosPointer(data));

      if (atoi(GWEN_XMLNode_GetProperty(node, "leftfill", "0"))) {
        if (GWEN_Buffer_AllocRoom(gbuf, maxsize+1)) {
          DBG_ERROR(GWEN_LOGDOMAIN, "Buffer too small");
          return -1;
        }

        /* fill left */
        for (lj=0; lj<(maxsize-len); lj++)
          GWEN_Buffer_AppendByte(gbuf, '0');

        /* write value */
        for (lj=0; lj<len; lj++)
          GWEN_Buffer_AppendByte(gbuf, GWEN_Buffer_ReadByte(data));
      }
      else if (atoi(GWEN_XMLNode_GetProperty(node, "rightfill", "0"))) {
        if (GWEN_Buffer_AllocRoom(gbuf, maxsize+1)) {
          DBG_ERROR(GWEN_LOGDOMAIN, "Buffer too small");
          return -1;
        }

        /* write value */
        for (lj=0; lj<len; lj++)
          GWEN_Buffer_AppendByte(gbuf, GWEN_Buffer_ReadByte(data));

        /* fill right */
        for (lj=0; lj<(maxsize-len); lj++)
          GWEN_Buffer_AppendByte(gbuf, '0');
      }
      else {
        if (GWEN_Buffer_AllocRoom(gbuf, maxsize+1)) {
          DBG_ERROR(GWEN_LOGDOMAIN, "Maxsize in XML file is higher than the buffer size");
          return -1;
        }
        for (lj=0; lj<len; lj++)
          GWEN_Buffer_AppendByte(gbuf, GWEN_Buffer_ReadByte(data));
      }
    } /* if type is num */
    else {
      /* TODO: Check for valids */
      const char *p;
      int lastWasEscape;
      unsigned int pcount;

      p=GWEN_Buffer_GetPosPointer(data);
      pcount=0;
      lastWasEscape=0;
      while (*p && pcount<GWEN_Buffer_GetUsedBytes(data)) {
        int c;

        c=(unsigned char)*p;
        if (lastWasEscape) {
          lastWasEscape=0;
          switch (c) {
          case 'r':
            c='\r';
            break;
          case 'n':
            c='\n';
            break;
          case 'f':
            c='\f';
            break;
          case 't':
            c='\t';
            break;
          default:
            c=(unsigned char)*p;
          } /* switch */
        }
        else {
          if (*p=='\\') {
            lastWasEscape=1;
            c=-1;
          }
          else
            c=(unsigned char)*p;
        }
        if (c!=-1) {
          int needsEscape;

          needsEscape=0;
          if (c==e->escapeChar)
            needsEscape=1;
          else {
            if (e->charsToEscape)
              if (strchr(e->charsToEscape, c))
                needsEscape=1;
          }
          if (needsEscape) {
            /* write escape char */
            if (GWEN_Buffer_AppendByte(gbuf,
                                       e->escapeChar)) {
              return -1;
            }
          }
          if (GWEN_Buffer_AppendByte(gbuf, c)) {
            return -1;
          }
        }
        p++;
        pcount++;
      } /* while */
      if (pcount<GWEN_Buffer_GetUsedBytes(data)) {
        DBG_WARN(GWEN_LOGDOMAIN, "Premature end of string (%d<%d)",
                 pcount, GWEN_Buffer_GetUsedBytes(data));
      }
      if (*p) {
        DBG_WARN(GWEN_LOGDOMAIN,
                 "String for \"%s\" (type %s) is longer than expected "
                 "(no #0 at pos=%d)",
                 name, type,
                 GWEN_Buffer_GetUsedBytes(data)-1);
      }
    } /* if type is not BIN */
  } /* if type not external */
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "Type \"%s\" (for %s) is external (write)",
             type, name);

  } /* if external type */

  /* fill data */
  if (fixSize) {
    uint32_t bs;
    unsigned int j;

    bs=GWEN_Buffer_GetPos(gbuf)-startPos;
    if (bs>fixSize) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Data too long (size is %d, fixed size is %d)",
                bs, fixSize);
      return -1;
    }

    for (j=bs; j<fixSize; j++)
      GWEN_Buffer_AppendByte(gbuf, (unsigned char)filler);
  }

  return 0;
}



int GWEN_MsgEngine__WriteElement(GWEN_MSGENGINE *e,
                                 GWEN_BUFFER *gbuf,
                                 GWEN_XMLNODE *node,
                                 GWEN_DB_NODE *gr,
                                 int loopNr,
                                 int isOptional,
                                 GWEN_XMLNODE_PATH *nodePath)
{
  const char *name;
  const char *type;
  //unsigned int minsize;
  //unsigned int maxsize;
  char numbuffer[256];
  const char *pdata;
  unsigned int datasize;
  GWEN_BUFFER *data;
  GWEN_BUFFER *tdata;
  int handled;

  pdata=0;
  handled=0;
  data=0;
  tdata=0;

  /* get type */
  type=GWEN_XMLNode_GetProperty(node, "type", "ASCII");
  DBG_DEBUG(GWEN_LOGDOMAIN, "Type is \"%s\"", type);
  /* get some sizes */
  //minsize=atoi(GWEN_XMLNode_GetProperty(node, "minsize","0"));
  //maxsize=atoi(GWEN_XMLNode_GetProperty(node, "maxsize","0"));

  if (e->binTypeWritePtr &&
      GWEN_MsgEngine__IsBinTyp(e, type) &&
      atoi(GWEN_XMLNode_GetProperty(node, "writebin", "1"))) {
    int rv;

    data=GWEN_Buffer_new(0,
                         64,
                         0,
                         1);

    rv=e->binTypeWritePtr(e, node, gr, data);
    if (rv==-1) {
      /* error */
      DBG_INFO(GWEN_LOGDOMAIN, "called from here");
      return -1;
    }
    else if (rv==0) {
      handled=1;
    }
    else if (rv==1) {
      GWEN_Buffer_free(data);
      data=0;
    }
  }

  if (!handled) {
    /* get name */
    name=GWEN_XMLNode_GetProperty(node, "name", 0);
    if (!name) {
      int rv;

      /* get data from within the XML node */
      tdata=GWEN_Buffer_new(0, 32, 0, 1);
      GWEN_Buffer_SetStep(tdata, 256);
      rv=GWEN_MsgEngine__GetInline(e, node, tdata);
      if (rv==0) {
        pdata=GWEN_Buffer_GetStart(tdata);
        datasize=GWEN_Buffer_GetUsedBytes(tdata);
      }
      else {
        GWEN_Buffer_free(tdata);
        tdata=0;
        pdata="";
        datasize=0;
      }
    } /* if (!name) */
    else {
      const char *nptr;

      DBG_DEBUG(GWEN_LOGDOMAIN, "Name provided (%s), loop is %d", name, loopNr);
      nptr=name;

      if (gr) {
        GWEN_DB_NODE_TYPE vt;
        int idata;

        /* Variable type of DB takes precedence
         */
        vt=GWEN_DB_GetValueTypeByPath(gr, nptr, loopNr);
        if (vt==GWEN_DB_NodeType_Unknown) {
          if (GWEN_MsgEngine__IsCharTyp(e, type))
            vt=GWEN_DB_NodeType_ValueChar;
          else if (GWEN_MsgEngine__IsIntTyp(e, type))
            vt=GWEN_DB_NodeType_ValueInt;
          else if (GWEN_MsgEngine__IsBinTyp(e, type))
            vt=GWEN_DB_NodeType_ValueBin;
          else {
            DBG_INFO(GWEN_LOGDOMAIN,
                     "Unable to determine parameter "
                     "type (%s), assuming \"char\" for this matter", type);
            vt=GWEN_DB_NodeType_ValueChar;
          }
        }

        /* get the value of the given var from the db */
        switch (vt) {
        case GWEN_DB_NodeType_ValueChar:
          DBG_DEBUG(GWEN_LOGDOMAIN, "Type of \"%s\" is char", name);
          pdata=GWEN_DB_GetCharValue(gr, nptr, loopNr, 0);
          if (pdata) {
            DBG_DEBUG(GWEN_LOGDOMAIN, "Value of \"%s\" is %s", nptr, pdata);
            datasize=strlen(pdata);
          }
          else
            datasize=0;
          break;

        case GWEN_DB_NodeType_ValueInt:
          DBG_DEBUG(GWEN_LOGDOMAIN, "Type of \"%s\" is int", name);
          if (GWEN_DB_ValueExists(gr, nptr, loopNr)) {
            idata=GWEN_DB_GetIntValue(gr, nptr, loopNr, 0);
            if (-1==GWEN_Text_NumToString(idata, numbuffer,
                                          sizeof(numbuffer), 0)) {
              DBG_ERROR(GWEN_LOGDOMAIN, "Buffer too small");
              GWEN_Buffer_free(data);
              return -1;
            }
            DBG_DEBUG(GWEN_LOGDOMAIN, "Value of \"%s\" is %d", nptr, idata);
            pdata=numbuffer;
            datasize=strlen(numbuffer);
          }
          break;

        case GWEN_DB_NodeType_ValueBin:
          DBG_DEBUG(GWEN_LOGDOMAIN, "Type of \"%s\" is bin", name);
          pdata=GWEN_DB_GetBinValue(gr, nptr, loopNr, 0, 0, &datasize);
          break;

        default:
          DBG_WARN(GWEN_LOGDOMAIN, "Unsupported parameter type (%d)", vt);
          break;
        } /* switch vt */
      } /* if gr */

      if (!pdata) {
        GWEN_XMLNODE_PATH *copyOfNodePath;

        copyOfNodePath=GWEN_XMLNode_Path_dup(nodePath);

        /* still no data, try to get it from the XML file */
        DBG_DEBUG(GWEN_LOGDOMAIN, "Searching for value of \"%s\"", name);
        pdata=GWEN_MsgEngine__SearchForValue(e,
                                             node, copyOfNodePath, nptr,
                                             &datasize);
        GWEN_XMLNode_Path_free(copyOfNodePath);
        if (pdata) {
          DBG_DEBUG(GWEN_LOGDOMAIN, "Found value of \"%s\"", name);
        }
      }

      if (!pdata) {
        int rv;

        /* get data from within the XML node */
        tdata=GWEN_Buffer_new(0, 32, 0, 1);
        GWEN_Buffer_SetStep(tdata, 256);
        rv=GWEN_MsgEngine__GetInline(e, node, tdata);
        if (rv==0) {
          pdata=GWEN_Buffer_GetStart(tdata);
          datasize=GWEN_Buffer_GetUsedBytes(tdata);
        }
        else {
          GWEN_Buffer_free(tdata);
          tdata=0;
        }
      }

      if (pdata==0) {
        if (isOptional) {
          DBG_INFO(GWEN_LOGDOMAIN, "Value not found, omitting element \"%s[%d]\"",
                   name, loopNr);
          GWEN_Buffer_free(data);
          return 1;
        }
        else {
          DBG_ERROR(GWEN_LOGDOMAIN,
                    "Value for element \"%s[%d]\" (mode \"%s\") not found",
                    name, loopNr,
                    GWEN_MsgEngine_GetMode(e));
          GWEN_DB_Dump(gr, 4);
          GWEN_Buffer_free(data);
          return -1;
        }
      }
    }

    if (!data)
      data=GWEN_Buffer_new((char *)pdata,
                           datasize,
                           datasize,
                           0 /* dont take ownership*/);
  }

  /* write value */
  if (GWEN_MsgEngine__WriteValue(e,
                                 gbuf,
                                 data,
                                 node)!=0) {
    DBG_INFO(GWEN_LOGDOMAIN, "Could not write value");
    GWEN_Buffer_free(data);
    GWEN_Buffer_free(tdata);
    return -1;
  }
  GWEN_Buffer_free(data);
  GWEN_Buffer_free(tdata);

  return 0;
}



int GWEN_MsgEngine__WriteGroup(GWEN_MSGENGINE *e,
                               GWEN_BUFFER *gbuf,
                               GWEN_XMLNODE *node,
                               GWEN_XMLNODE *rnode,
                               GWEN_DB_NODE *gr,
                               int groupIsOptional,
                               GWEN_XMLNODE_PATH *nodePath)
{
  GWEN_XMLNODE *n;
  const char *p;
  char delimiter;
  char terminator;
  int isFirstElement;
  int omittedElements;
  int hasEntries;


  /* get some settings */
  if (rnode) {
    /* get delimiter */
    p=GWEN_XMLNode_GetProperty(rnode,
                               "delimiter",
                               GWEN_XMLNode_GetProperty(node,
                                                        "delimiter",
                                                        ""));
    delimiter=*p;

    /* get terminating char, if any */
    p=GWEN_XMLNode_GetProperty(rnode,
                               "terminator",
                               GWEN_XMLNode_GetProperty(node,
                                                        "terminator",
                                                        ""));
    terminator=*p;
  }
  else {
    /* get delimiter */
    p=GWEN_XMLNode_GetProperty(node,
                               "delimiter",
                               "");
    delimiter=*p;

    /* get terminating char, if any */
    p=GWEN_XMLNode_GetProperty(node, "terminator", "");
    terminator=*p;
  }

  /* handle all child entries */
  n=GWEN_XMLNode_GetChild(node);
  isFirstElement=1;
  omittedElements=0;
  hasEntries=0;
  if (!n) {
    DBG_INFO(GWEN_LOGDOMAIN, "No subnodes !");
  }
  while (n) {
    int t;
    unsigned int minnum;
    unsigned int maxnum;
    int gversion;
    const char *addEmptyMode;
    unsigned int loopNr;

    minnum=atoi(GWEN_XMLNode_GetProperty(n, "minnum", "1"));
    maxnum=atoi(GWEN_XMLNode_GetProperty(n, "maxnum", "1"));
    gversion=atoi(GWEN_XMLNode_GetProperty(n, "version", "0"));
    addEmptyMode=GWEN_XMLNode_GetProperty(n, "addemptymode", "one");

    DBG_DEBUG(GWEN_LOGDOMAIN, "Omitted elements: %d", omittedElements);
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
        int j;
        int rv;

        DBG_VERBOUS(GWEN_LOGDOMAIN, "Found an element");
        /* write element as often as needed */
        for (loopNr=0; loopNr<maxnum; loopNr++) {
          unsigned int posBeforeElement;

          posBeforeElement=GWEN_Buffer_GetPos(gbuf);

          /* write delimiter, if needed */
          if (delimiter) {
            DBG_VERBOUS(GWEN_LOGDOMAIN, "Appending %d delimiters",
                        omittedElements);
            for (j=0; j<omittedElements; j++) {
              if (GWEN_Buffer_AppendByte(gbuf, delimiter)) {
                DBG_INFO(GWEN_LOGDOMAIN, "called from here");
                return -1;
              }
            }
            if (!isFirstElement)
              if (GWEN_Buffer_AppendByte(gbuf, delimiter)) {
                DBG_INFO(GWEN_LOGDOMAIN, "called from here");
                return -1;
              }
          }

          rv=GWEN_MsgEngine__WriteElement(e,
                                          gbuf,
                                          n,
                                          gr,
                                          loopNr,
                                          loopNr>=minnum ||
                                          (groupIsOptional && !hasEntries),
                                          nodePath);
          if (rv==-1) {
            DBG_INFO(GWEN_LOGDOMAIN, "Error writing element");
            DBG_INFO(GWEN_LOGDOMAIN, "Node is:");
            GWEN_XMLNode_Dump(n, 1);
            if (gr) {
              DBG_INFO(GWEN_LOGDOMAIN, "Data is:");
              GWEN_DB_Dump(gr, 1);
            }
            return -1;
          }
          else if (rv==0) {
            isFirstElement=0;
            omittedElements=0;
            hasEntries=1;
            DBG_DEBUG(GWEN_LOGDOMAIN, "Element written");
          }
          else {
            /* element is optional, not found */
            /* restore position */
            GWEN_Buffer_SetPos(gbuf, posBeforeElement);
            GWEN_Buffer_Crop(gbuf, 0, posBeforeElement);

            if (strcasecmp(addEmptyMode, "max")==0) {
              DBG_DEBUG(GWEN_LOGDOMAIN, "Adding max empty");
              omittedElements+=(maxnum-loopNr);
            }
            else if (strcasecmp(addEmptyMode, "min")==0) {
              DBG_DEBUG(GWEN_LOGDOMAIN, "Adding min empty");
              if (loopNr<minnum)
                omittedElements+=(minnum-loopNr);
            }
            else if (strcasecmp(addEmptyMode, "one")==0) {
              if (loopNr==0)
                omittedElements++;
            }
            else if (strcasecmp(addEmptyMode, "none")==0) {
            }
            else {
              DBG_ERROR(GWEN_LOGDOMAIN, "Unknown addemptymode \"%s\"",
                        addEmptyMode);
              return -1;
            }
            break;
          }
        } /* for */
      }
      else if (strcasecmp(typ, "VALUES")==0) {
      }
      else if (strcasecmp(typ, "DESCR")==0) {
      }
      else {
        /* group tag found */
        GWEN_XMLNODE *gn;
        GWEN_DB_NODE *gcfg;
        const char *gname;
        const char *gtype;
        unsigned int posBeforeGroup;

        DBG_VERBOUS(GWEN_LOGDOMAIN, "Found a group");

        gcfg=0;
        gtype=GWEN_XMLNode_GetProperty(n, "type", 0);
        if (!gtype) {
          /* no "type" property, so use this group directly */
          DBG_INFO(GWEN_LOGDOMAIN, "<%s> tag has no \"type\" property", typ);
          gtype="";
          gn=n;
        }
        else {
          DBG_VERBOUS(GWEN_LOGDOMAIN, "<%s> tag is of type \"%s\"", typ, gtype);
          gn=GWEN_MsgEngine_GetGroup(e, n, nodePath, typ,
                                     gversion, gtype);
          if (!gn) {
            DBG_INFO(GWEN_LOGDOMAIN, "Definition for type \"%s\" not found", typ);
            return -1;
          }
        }

        gname=NULL;
        gcfg=NULL;
        if (gr) {
          gname=GWEN_XMLNode_GetProperty(n, "name", 0);
          if (gname) {
            DBG_VERBOUS(GWEN_LOGDOMAIN, "Group \"%s\" using special data", gname);
            gcfg=GWEN_DB_FindFirstGroup(gr, gname);
          }
          else {
            DBG_DEBUG(GWEN_LOGDOMAIN, "Unnamed group, using basic data");
            /* TODO: check for maxnum==1, since only then the following line makes sense */
            gcfg=gr;
          }
        }

        /* write group as often as needed */
        for (loopNr=0; loopNr<maxnum; loopNr++) {
          int rv;
          int groupIsEmpty;

          groupIsEmpty=0;
          posBeforeGroup=GWEN_Buffer_GetPos(gbuf);

          /* write delimiter, if needed */
          if (delimiter) {
            int j;

            DBG_VERBOUS(GWEN_LOGDOMAIN, "Appending %d delimiters",
                        omittedElements);
            for (j=0; j<omittedElements; j++) {
              if (GWEN_Buffer_AppendByte(gbuf, delimiter)) {
                DBG_INFO(GWEN_LOGDOMAIN, "called from here");
                return -1;
              }
            }
            if (!isFirstElement)
              if (GWEN_Buffer_AppendByte(gbuf, delimiter)) {
                DBG_INFO(GWEN_LOGDOMAIN, "called from here");
                return -1;
              }
          }

          /* find next matching group */
          if (gcfg==0) {
            DBG_DEBUG(GWEN_LOGDOMAIN, "No group found");
            if (loopNr>=minnum)
              groupIsEmpty=1;
          }

          if (groupIsEmpty) {
            /* empty group, flag as such */
            rv=1;
          }
          else {
            int dive;

            /* write group */
            if (GWEN_XMLNode_Path_Dive(nodePath, n)) {
              DBG_INFO(GWEN_LOGDOMAIN, "Called from here");
              return -1;
            }
            if (n==gn)
              dive=1;
            else {
              if (GWEN_XMLNode_Path_Dive(nodePath, gn)) {
                DBG_INFO(GWEN_LOGDOMAIN, "Called from here");
                return -1;
              }
              dive=2;
            }
            rv=GWEN_MsgEngine__WriteGroup(e,
                                          gbuf,
                                          gn,
                                          n,
                                          gcfg,
                                          loopNr>=minnum || groupIsOptional,
                                          nodePath);
            GWEN_XMLNode_Path_Surface(nodePath);
            if (dive==2)
              GWEN_XMLNode_Path_Surface(nodePath);
          }

          if (rv==-1) {
            DBG_INFO(GWEN_LOGDOMAIN, "Could not write group \"%s\"", gtype);
            if (gn) {
              DBG_INFO(GWEN_LOGDOMAIN, "Node is:");
              GWEN_XMLNode_Dump(gn, 1);
            }
            if (n) {
              DBG_INFO(GWEN_LOGDOMAIN, "Referring node is:");
              GWEN_XMLNode_Dump(n, 1);
            }
            if (gr) {
              DBG_INFO(GWEN_LOGDOMAIN, "Data is:");
              GWEN_DB_Dump(gr, 1);
            }
            return -1;
          }
          else if (rv==0) {
            isFirstElement=0;
            omittedElements=0;
            hasEntries=1;
            DBG_DEBUG(GWEN_LOGDOMAIN, "Element written");
          }
          else {
            /* group is optional, not found */
            /* restore position */
            GWEN_Buffer_SetPos(gbuf, posBeforeGroup);
            GWEN_Buffer_Crop(gbuf, 0, posBeforeGroup);

            if (strcasecmp(addEmptyMode, "max")==0) {
              DBG_DEBUG(GWEN_LOGDOMAIN, "Adding max empty");
              omittedElements+=(maxnum-loopNr);
            }
            else if (strcasecmp(addEmptyMode, "min")==0) {
              DBG_DEBUG(GWEN_LOGDOMAIN, "Adding min empty");
              if (loopNr<minnum)
                omittedElements+=(minnum-loopNr);
            }
            else if (strcasecmp(addEmptyMode, "one")==0) {
              if (loopNr==0)
                omittedElements++;
            }
            else if (strcasecmp(addEmptyMode, "none")==0) {
            }
            else {
              DBG_ERROR(GWEN_LOGDOMAIN, "Unknown addemptymode \"%s\"",
                        addEmptyMode);
              return -1;
            }
            break;
          }

          /* use next group next time if any */
          if (gcfg && gname)
            gcfg=GWEN_DB_FindNextGroup(gcfg, gname);
        } /* for */
      } /* if "GROUP" */
    } /* if TAG */
    else if (t==GWEN_XMLNodeTypeData) {
    }
    else {
      DBG_DEBUG(GWEN_LOGDOMAIN, "Unhandled node type %d", t);
    }
    n=GWEN_XMLNode_Next(n);
  } /* while */

  /* write terminating character, if any */
  if (terminator) {
    if (GWEN_Buffer_AppendByte(gbuf, terminator)) {
      DBG_INFO(GWEN_LOGDOMAIN, "called from here");
      return -1;
    }
  }

  if (!hasEntries) {
    DBG_INFO(GWEN_LOGDOMAIN, "No entries in node");
  }
  return hasEntries?0:1;
}



int GWEN_MsgEngine_CreateMessageFromNode(GWEN_MSGENGINE *e,
                                         GWEN_XMLNODE *node,
                                         GWEN_BUFFER *gbuf,
                                         GWEN_DB_NODE *msgData)
{
  GWEN_XMLNODE_PATH *np;
  int rv;

  assert(e);
  assert(node);
  assert(msgData);

  np=GWEN_XMLNode_Path_new();
  GWEN_XMLNode_Path_Dive(np, node);
  rv=GWEN_MsgEngine__WriteGroup(e,
                                gbuf,
                                node,
                                0,
                                msgData,
                                0,
                                np);
  GWEN_XMLNode_Path_free(np);
  if (rv) {
    const char *p;

    p=GWEN_XMLNode_GetData(node);
    if (p) {
      DBG_INFO(GWEN_LOGDOMAIN, "Error writing group \"%s\"", p);
    }
    else {
      DBG_INFO(GWEN_LOGDOMAIN, "Error writing group");
    }
    return -1;
  }

  return 0;
}



int GWEN_MsgEngine_CreateMessage(GWEN_MSGENGINE *e,
                                 const char *msgName,
                                 int msgVersion,
                                 GWEN_BUFFER *gbuf,
                                 GWEN_DB_NODE *msgData)
{
  GWEN_XMLNODE *group;

  group=GWEN_MsgEngine_FindGroupByProperty(e, "id", msgVersion, msgName);
  if (!group) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Group \"%s\" not found\n", msgName);
    return -1;
  }
  return GWEN_MsgEngine_CreateMessageFromNode(e,
                                              group,
                                              gbuf,
                                              msgData);
}


