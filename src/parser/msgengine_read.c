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



#define GWEN_MSGENGINE_MAXBINDATASIZE 10



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */


static int _groupReadGroup(GWEN_MSGENGINE *e,
			   GWEN_BUFFER *msgbuf,
			   const char *type,
			   GWEN_XMLNODE *n,
			   GWEN_DB_NODE *dbData,
			   char currentDelimiter,
			   char currentTerminator,
			   const char *delimiters,
			   uint32_t flags);
static int _groupReadElement(GWEN_MSGENGINE *e,
                             GWEN_BUFFER *msgbuf,
			     GWEN_XMLNODE *n,
                             GWEN_DB_NODE *dbData,
                             char currentDelimiter,
                             char currentTerminator,
                             const char *delimiters,
                             uint32_t flags);

static int _readSingleElement(GWEN_MSGENGINE *e,
			      GWEN_BUFFER *msgbuf,
			      GWEN_XMLNODE *n,
			      GWEN_DB_NODE *dbData,
			      const char *delimiters,
			      uint32_t flags);

static int _skipRestOfSegment(GWEN_MSGENGINE *e, GWEN_BUFFER *mbuf, GWEN_DB_NODE *dbSegmentHead, uint32_t flags);
static int _readValue(GWEN_MSGENGINE *e,
		      GWEN_BUFFER *msgbuf,
		      GWEN_XMLNODE *xmlNode,
		      GWEN_BUFFER *vbuf,
		      const char *delimiters,
		      uint32_t flags);
static int _readBinDataLength(GWEN_BUFFER *msgbuf);
static int _readBinDataIntoBuffer(GWEN_BUFFER *msgbuf, GWEN_BUFFER *vbuf);
static int _readNonBinDataIntoBuffer(GWEN_BUFFER *msgbuf, char escapeChar, const char *delimiters, int fixedSize, GWEN_BUFFER *vbuf);




/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */




/* parse a segment */
int GWEN_MsgEngine_ParseMessage(GWEN_MSGENGINE *e,
                                GWEN_XMLNODE *group,
                                GWEN_BUFFER *msgbuf,
                                GWEN_DB_NODE *msgData,
                                uint32_t flags)
{

  if (GWEN_MsgEngine__ReadGroup(e, msgbuf, group, 0, msgData, e->delimiters, flags)) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error reading group");
    return GWEN_ERROR_GENERIC;
  }

  return 0;
}




int GWEN_MsgEngine_ReadMessage(GWEN_MSGENGINE *e,
                               const char *gtype,
                               GWEN_BUFFER *mbuf,
                               GWEN_DB_NODE *dbData,
                               uint32_t flags)
{
  unsigned int segments=0;

  while (GWEN_Buffer_GetBytesLeft(mbuf)) {
    GWEN_XMLNODE *xmlNode;
    unsigned int posSegmentStart;
    const char *sSegmentCode;
    GWEN_DB_NODE *dbSegmentHead;
    int segVer;

    /* find head segment description */
    dbSegmentHead=GWEN_DB_Group_new("dbSegmentHead");
    xmlNode=GWEN_MsgEngine_FindGroupByProperty(e, "id", 0, "SegHead");
    if (xmlNode==0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Segment description not found");
      GWEN_DB_Group_free(dbSegmentHead);
      return GWEN_ERROR_GENERIC;
    }

    /* parse head segment */
    posSegmentStart=GWEN_Buffer_GetPos(mbuf);
    if (GWEN_MsgEngine_ParseMessage(e, xmlNode, mbuf, dbSegmentHead, flags)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error parsing segment head");
      GWEN_DB_Group_free(dbSegmentHead);
      return GWEN_ERROR_GENERIC;
    }

    /* get segment code */
    segVer=GWEN_DB_GetIntValue(dbSegmentHead, "version", 0, 0);
    sSegmentCode=GWEN_DB_GetCharValue(dbSegmentHead, "code", 0, 0);
    if (!sSegmentCode) {
      DBG_ERROR(GWEN_LOGDOMAIN, "No segment code for %s ? This seems to be a bad msg...", gtype);
      GWEN_Buffer_SetPos(mbuf, posSegmentStart);
      DBG_ERROR(GWEN_LOGDOMAIN, "Full message (pos=%04x)", posSegmentStart);
      GWEN_Text_DumpString(GWEN_Buffer_GetStart(mbuf), GWEN_Buffer_GetUsedBytes(mbuf), 1);
      GWEN_DB_Dump(dbSegmentHead, 1);
      GWEN_DB_Group_free(dbSegmentHead);
      return GWEN_ERROR_GENERIC;
    }

    /* try to find corresponding XML xmlNode */
    xmlNode=GWEN_MsgEngine_FindNodeByProperty(e, gtype, "code", segVer, sSegmentCode);
    if (xmlNode==0) {
      int rv;

      rv=_skipRestOfSegment(e, mbuf, dbSegmentHead, flags);
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	GWEN_DB_Group_free(dbSegmentHead);
	return rv;
      }
    }
    else {
      /* ok, xmlNode available, get the corresponding description and parse the segment */
      const char *id;
      GWEN_DB_NODE *storegrp;

      /* restore start position, since the segment head is part of a full
       * description, so we need to restart reading from the very begin */
      GWEN_Buffer_SetPos(mbuf, posSegmentStart);

      /* create group in DB for this segment */
      id=GWEN_XMLNode_GetProperty(xmlNode, "id", sSegmentCode);
      storegrp=GWEN_DB_GetGroup(dbData, GWEN_PATH_FLAGS_CREATE_GROUP, id);
      assert(storegrp);

      /* store the start position of this segment within the DB */
      GWEN_DB_SetIntValue(storegrp, GWEN_DB_FLAGS_OVERWRITE_VARS, "segment/pos", posSegmentStart);

      /* parse the segment */
      if (GWEN_MsgEngine_ParseMessage(e, xmlNode, mbuf, storegrp, flags)) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Error parsing segment \"%s\" at %d (%x)",
                  sSegmentCode,
                  GWEN_Buffer_GetPos(mbuf)-posSegmentStart,
                  GWEN_Buffer_GetPos(mbuf)-posSegmentStart);
        GWEN_Text_DumpString(GWEN_Buffer_GetStart(mbuf)+posSegmentStart,
                             GWEN_Buffer_GetUsedBytes(mbuf)-posSegmentStart,
			     1);
        DBG_ERROR(GWEN_LOGDOMAIN, "Stored data so far:");
        GWEN_DB_Dump(storegrp, 2);
        GWEN_DB_Group_free(dbSegmentHead);
        return GWEN_ERROR_GENERIC;
      }

      /* store segment size within DB */
      GWEN_DB_SetIntValue(storegrp, GWEN_DB_FLAGS_OVERWRITE_VARS, "segment/length", GWEN_Buffer_GetPos(mbuf)-posSegmentStart);
      segments++;
    }
    GWEN_DB_Group_free(dbSegmentHead);
  } /* while */

  /* done */
  if (segments) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "Parsed %d segments", segments);
    return 0;
  }
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "No segments parsed.");
    return 1;
  }
}



int _skipRestOfSegment(GWEN_MSGENGINE *e, GWEN_BUFFER *mbuf, GWEN_DB_NODE *dbSegmentHead, uint32_t flags)
{
  unsigned int ustart;
  const char *sSegmentName;

  sSegmentName=GWEN_DB_GetCharValue(dbSegmentHead, "code", 0, "<unnamed>");
  ustart=GWEN_Buffer_GetPos(mbuf);
  ustart++; /* skip delimiter */
  
  /* xmlNode not found, skip it */
  DBG_NOTICE(GWEN_LOGDOMAIN,
	     "Unknown segment \"%s\" (Segnum=%d, version=%d, ref=%d), skipping",
	     sSegmentName,
	     GWEN_DB_GetIntValue(dbSegmentHead, "seq", 0, -1),
	     GWEN_DB_GetIntValue(dbSegmentHead, "version", 0, -1),
	     GWEN_DB_GetIntValue(dbSegmentHead, "ref", 0, -1));
  if (GWEN_MsgEngine_SkipSegment(e, mbuf, '?', '\'')) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Error skipping segment \"%s\"", sSegmentName);
    return GWEN_ERROR_GENERIC;
  }
  if (flags & GWEN_MSGENGINE_READ_FLAGS_TRUSTINFO) {
    unsigned int usize;
  
    usize=GWEN_Buffer_GetPos(mbuf)-ustart-1;
  #if 0
    GWEN_Text_DumpString(GWEN_Buffer_GetStart(mbuf)+ustart, usize, stderr, 1);
  #endif
    if (GWEN_MsgEngine_AddTrustInfo(e,
				    GWEN_Buffer_GetStart(mbuf)+ustart,
				    usize,
				    sSegmentName,
				    GWEN_MsgEngineTrustLevelHigh,
				    ustart)) {
      DBG_INFO(GWEN_LOGDOMAIN, "called from here");
      return GWEN_ERROR_GENERIC;
    }
  } /* if trustInfo handling wanted */

  return 0;
}



int GWEN_MsgEngine_SkipSegment(GWEN_UNUSED GWEN_MSGENGINE *e,
                               GWEN_BUFFER *msgbuf,
                               unsigned char escapeChar,
                               unsigned char delimiter)
{
  int esc;

  esc=0;
  while (GWEN_Buffer_GetBytesLeft(msgbuf)) {
    int nc;

    if (esc) {
      esc=0;
      /* skip byte */
      nc=GWEN_Buffer_ReadByte(msgbuf);
      if (nc<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", nc);
        return -1;
      }
    }
    else {
      int i;
      unsigned char c;

      i=GWEN_Buffer_ReadByte(msgbuf);
      if (i<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "called from here");
        return 0;
      }
      c=(unsigned int)i;
      if (c==escapeChar) { /* escape */
        esc=1;
      }
      else if (c=='@') {
	int l;

	/* skip binary data */
	l=_readBinDataLength(msgbuf);
	if (l<0) {
	  DBG_ERROR(GWEN_LOGDOMAIN, "\"@num@\" expected (invalid length)");
	  return GWEN_ERROR_GENERIC;
	}
    
	/* read closing @ */
	c=GWEN_Buffer_ReadByte(msgbuf);
	if (c!='@') {
	  DBG_ERROR(GWEN_LOGDOMAIN, "\"@num@\" expected (missing closing @)");
	  return GWEN_ERROR_GENERIC;
	}
    
	if (GWEN_Buffer_GetBytesLeft(msgbuf) < (unsigned) l) {
	  DBG_ERROR(GWEN_LOGDOMAIN, "Premature end of message (binary beyond end)");
	  return GWEN_ERROR_GENERIC;
	}
	if (l>0)
	  GWEN_Buffer_IncrementPos(msgbuf, l);
      }
      else if (c==delimiter) {/* segment-end */
        return 0;
        break;
      }
    }
  } /* while */

  DBG_ERROR(GWEN_LOGDOMAIN, "End of segment not found");
  return GWEN_ERROR_BAD_DATA;
}



int GWEN_MsgEngine__ReadGroup(GWEN_MSGENGINE *e,
                              GWEN_BUFFER *msgbuf,
                              GWEN_XMLNODE *xmlNode,
                              GWEN_XMLNODE *xmlReferencingNode,
                              GWEN_DB_NODE *dbData,
                              const char *delimiters,
                              uint32_t flags)
{
  const char *p;
  char delimiter;
  char terminator;
  GWEN_XMLNODE *n;
  GWEN_BUFFER *delimBuffer=0;

  /* get some settings */
  if (xmlReferencingNode) {
    /* get delimiter */
    p=GWEN_XMLNode_GetProperty(xmlReferencingNode, "delimiter", GWEN_XMLNode_GetProperty(xmlNode, "delimiter", ""));
    delimiter=*p;

    /* get terminating char, if any */
    p=GWEN_XMLNode_GetProperty(xmlReferencingNode, "terminator", GWEN_XMLNode_GetProperty(xmlNode, "terminator", ""));
    terminator=*p;
  }
  else {
    /* get delimiter */
    p=GWEN_XMLNode_GetProperty(xmlNode, "delimiter", "");
    delimiter=*p;

    /* get terminating char, if any */
    p=GWEN_XMLNode_GetProperty(xmlNode, "terminator", "");
    terminator=*p;
  }

  delimBuffer=GWEN_Buffer_new(0, strlen(delimiters)+2, 0, 1);
  GWEN_Buffer_AppendString(delimBuffer, delimiters);
  if (delimiter)
    GWEN_Buffer_AppendByte(delimBuffer, delimiter);
  if (terminator)
    GWEN_Buffer_AppendByte(delimBuffer, terminator);

  DBG_DEBUG(GWEN_LOGDOMAIN, "Delimiters are \"%s\" and \"%c\"", delimiters, delimiter);

  n=GWEN_XMLNode_GetChild(xmlNode);
  while (n) {
    if (GWEN_XMLNode_GetType(n)==GWEN_XMLNodeTypeTag) {
      const char *type;
      int rv;

      if (GWEN_Buffer_GetBytesLeft(msgbuf)==0)
        break;

      type=GWEN_XMLNode_GetData(n);

#ifdef HEAVY_DEBUG_MSGENGINE
      DBG_NOTICE(GWEN_LOGDOMAIN, "Reading group from here :");
      GWEN_Text_DumpString(GWEN_Buffer_GetStart(msgbuf)+ GWEN_Buffer_GetPos(msgbuf),
                           GWEN_Buffer_GetUsedBytes(msgbuf)-GWEN_Buffer_GetPos(msgbuf),
                           3);
#endif
      if (strcasecmp(type, "ELEM")==0) {
	rv=_groupReadElement(e, msgbuf, n, dbData,
			     delimiter, terminator, GWEN_Buffer_GetStart(delimBuffer),
			     flags);
        if (rv<0) {
          DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
          GWEN_Buffer_free(delimBuffer);
          return rv;
        }
        n=GWEN_XMLNode_Next(n);
      } /* if ELEM */
      else if (strcasecmp(type, "VALUES")==0) {
        n=GWEN_XMLNode_Next(n);
      }
      else if (strcasecmp(type, "DESCR")==0) {
        n=GWEN_XMLNode_Next(n);
      }
      else {
	/* group tag found */
	rv=_groupReadGroup(e, msgbuf, type, n, dbData,
			   delimiter, terminator, GWEN_Buffer_GetStart(delimBuffer),
			   flags);
	if (rv<0) {
	  DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	  GWEN_Buffer_free(delimBuffer);
	  return rv;
	}
	n=GWEN_XMLNode_Next(n);
      } /* if GROUP */
    } /* if TAG */
    else {
      n=GWEN_XMLNode_Next(n);
    }
  } /* while */

  /* check whether there still are nodes which have not been read */
  while (n) {
    if (GWEN_XMLNode_GetType(n)==GWEN_XMLNodeTypeTag) {
      if (strcasecmp(GWEN_XMLNode_GetData(n), "ELEM")==0 ||
          strcasecmp(GWEN_XMLNode_GetData(n), "GROUP")==0) {
        unsigned int i;

	i=GWEN_XMLNode_GetIntProperty(n, "minnum", 1);
	if (i) {
	  DBG_ERROR(GWEN_LOGDOMAIN, "Premature end of message (still tags to parse)");
	  GWEN_XMLNode_Dump(n, 2);
	  GWEN_Buffer_free(delimBuffer);
	  return -1;
	}
      }
    }
    n=GWEN_XMLNode_Next(n);
  }


  if (terminator) {
    /* skip terminator */
    if (GWEN_Buffer_GetBytesLeft(msgbuf)) {
      if (GWEN_Buffer_PeekByte(msgbuf)==terminator) {
        GWEN_Buffer_IncrementPos(msgbuf, 1);
      }
      else {
        DBG_ERROR(GWEN_LOGDOMAIN,
                  "Terminating character missing (pos=%d [%x]) "
                  "expecting \"%c\", got \"%c\")",
                  GWEN_Buffer_GetPos(msgbuf),
                  GWEN_Buffer_GetPos(msgbuf),
                  terminator,
                  GWEN_Buffer_PeekByte(msgbuf));
        GWEN_XMLNode_Dump(xmlNode, 1);
        GWEN_Buffer_free(delimBuffer);
        return -1;
      }
    }
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "Terminating character missing");
      GWEN_Buffer_free(delimBuffer);
      return -1;
    }
  }

  GWEN_Buffer_free(delimBuffer);
  return 0;
}



int _groupReadGroupOrElement(GWEN_MSGENGINE *e,
			     GWEN_BUFFER *msgbuf,
			     GWEN_XMLNODE *xmlNode,
			     GWEN_DB_NODE *dbData,
			     char currentDelimiter,
			     char currentTerminator,
			     const char *delimiters,
			     uint32_t flags)
{
  const char *sEntryType;
  unsigned int minnum;
  unsigned int maxnum;
  unsigned int loopNr;
  
  DBG_VERBOUS(GWEN_LOGDOMAIN, "Reading entry");
  minnum=GWEN_XMLNode_GetIntProperty(xmlNode, "minnum", 1);
  maxnum=GWEN_XMLNode_GetIntProperty(xmlNode, "maxnum", 1);

  sEntryType=GWEN_XMLNode_GetData(xmlNode);

  loopNr=0;
  while (GWEN_Buffer_GetBytesLeft(msgbuf) && (maxnum==0 || loopNr<maxnum)) {
    int c;
    int rv;

    DBG_DEBUG(GWEN_LOGDOMAIN, "Reading entry type %s[%d]", sEntryType, loopNr);

    c=GWEN_Buffer_PeekByte(msgbuf);
    if ((currentTerminator && c==currentTerminator) ||
	(strchr(delimiters, c) && !(currentDelimiter && c==currentDelimiter)))
      /* terminator or higher level delimiter found, we're done */
      break;

    if (strcasecmp(sEntryType, "GROUP")==0) {
      rv=0; // TODO: _readSingleGroupEntry()
    }
    else if (strcasecmp(sEntryType, "ELEM")==0) {
      rv=_readSingleElement(e, msgbuf, xmlNode, dbData, delimiters, flags);
    }
    else {
      DBG_INFO(GWEN_LOGDOMAIN, "Unhandled type \"%s\", ignoring", sEntryType);
      rv=0;
    }
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    }
    loopNr++;
  }

  if (minnum && loopNr<minnum) {
    DBG_ERROR(GWEN_LOGDOMAIN,
	      "Premature end of message (too few repeats, %d found, %d expected)",
	      loopNr, minnum);
    return GWEN_ERROR_GENERIC;
  }

  return 0;
}



int _readSingleElement(GWEN_MSGENGINE *e,
		       GWEN_BUFFER *msgbuf,
		       GWEN_XMLNODE *n,
		       GWEN_DB_NODE *dbData,
		       const char *delimiters,
		       uint32_t flags)
{
  const char *name;
  int rv;
  GWEN_BUFFER *vbuf;

  DBG_VERBOUS(GWEN_LOGDOMAIN, "Reading element");

  /* get some sizes */
  name=GWEN_XMLNode_GetProperty(n, "name", 0);
  DBG_VERBOUS(GWEN_LOGDOMAIN, "Reading value \"%s\" from pos %x", name?name:"<unnamed>", GWEN_Buffer_GetPos(msgbuf));

  vbuf=GWEN_Buffer_new(0, GWEN_MSGENGINE_MAX_VALUE_LEN, 0, 0);
#ifdef HEAVY_DEBUG_MSGENGINE
  DBG_ERROR(GWEN_LOGDOMAIN, "Reading value from here:\n");
  GWEN_Text_DumpString(GWEN_Buffer_GetPosPointer(msgbuf),
		       GWEN_Buffer_GetBytesLeft(msgbuf),
		       1);
#endif

  rv=_readValue(e, msgbuf, n, vbuf, delimiters, flags);
  if (rv==1) {
    DBG_INFO(GWEN_LOGDOMAIN, "Empty value");
  }
  else if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error parsing xmlNode \"%s\" (ELEM)", name);
    GWEN_Buffer_free(vbuf);
    return rv;
  }

  GWEN_Buffer_Rewind(vbuf);
  if (GWEN_Buffer_GetUsedBytes(vbuf)) {
    const char *dtype;

    /* special handling for binary data */
    dtype=GWEN_XMLNode_GetProperty(n, "type", "");
    if (GWEN_MsgEngine__IsBinTyp(e, dtype)) {
      if (GWEN_XMLNode_GetIntProperty(n, "readbin", 1) && e->binTypeReadPtr) {
	rv=e->binTypeReadPtr(e, n, dbData, vbuf);
      }
      else
	rv=1;
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "Called from here");
	GWEN_Buffer_free(vbuf);
	return GWEN_ERROR_GENERIC;
      }
      else if (rv==1) {
	/* bin type not handled, so handle it myself */
	if (name && *name)
	  GWEN_DB_SetBinValue(dbData,
			      GWEN_DB_FLAGS_DEFAULT,
			      name,
			      GWEN_Buffer_GetStart(vbuf),
			      GWEN_Buffer_GetUsedBytes(vbuf));
      }
    } /* if type is bin */
    else if (GWEN_MsgEngine__IsIntTyp(e, dtype)) {
      int z;

      if (1!=sscanf(GWEN_Buffer_GetStart(vbuf), "%d", &z)) {
	DBG_INFO(GWEN_LOGDOMAIN, "Value for \"%s\" is not an integer", name);
	return GWEN_ERROR_GENERIC;
      }
      if (name && *name)
	GWEN_DB_SetIntValue(dbData, GWEN_DB_FLAGS_DEFAULT, name, z);
    } /* if type is int */
    else {
      DBG_DEBUG(GWEN_LOGDOMAIN, "Value is \"%s\"", GWEN_Buffer_GetStart(vbuf));
      if (name && *name)
	GWEN_DB_SetCharValue(dbData,
			     GWEN_DB_FLAGS_DEFAULT,
			     name,
			     GWEN_Buffer_GetStart(vbuf));
    } /* if !bin */
  } /* if data in vbuffer */
  GWEN_Buffer_free(vbuf);

  return 0;
}








int _groupReadGroup(GWEN_MSGENGINE *e,
		    GWEN_BUFFER *msgbuf,
		    const char *type,
		    GWEN_XMLNODE *xmlNode,
		    GWEN_DB_NODE *dbData,
		    char currentDelimiter,
		    char currentTerminator,
		    const char *delimiters,
		    uint32_t flags)
{
  GWEN_XMLNODE *xmlReferredNode;
  GWEN_DB_NODE *gcfg;
  const char *gname;
  const char *gtype;
  unsigned int minnum;
  unsigned int maxnum;
  unsigned int gversion;
  int abortLoop;
  unsigned int loopNr;
  
  DBG_VERBOUS(GWEN_LOGDOMAIN, "Reading group");
  minnum=GWEN_XMLNode_GetIntProperty(xmlNode, "minnum", 1);
  maxnum=GWEN_XMLNode_GetIntProperty(xmlNode, "maxnum", 1);
  gversion=GWEN_XMLNode_GetIntProperty(xmlNode, "version", 0);
  gtype=GWEN_XMLNode_GetProperty(xmlNode, "type", 0);
  if (!gtype) {
    /* no "type" property, so use this group directly */
    DBG_INFO(GWEN_LOGDOMAIN, "<%s> tag has no \"type\" property", type);
    gtype="";
    xmlReferredNode=xmlNode;
  }
  else {
    xmlReferredNode=GWEN_MsgEngine_FindNodeByProperty(e, type, "id", gversion, gtype);
    if (!xmlReferredNode) {
      DBG_INFO(GWEN_LOGDOMAIN, "Definition for type \"%s\" not found", type);
      return GWEN_ERROR_GENERIC;
    }
  }
  
  /* get configuration */
  loopNr=0;
  abortLoop=0;
  while ((maxnum==0 || loopNr<maxnum) && !abortLoop) {
    int c;
  
    DBG_DEBUG(GWEN_LOGDOMAIN, "Reading group type %s", gtype);
    if (GWEN_Buffer_GetBytesLeft(msgbuf)==0)
      break;
    c=GWEN_Buffer_PeekByte(msgbuf);
    if (c && strchr(delimiters, c)) {
      abortLoop=1;
    }
    else {
      gname=GWEN_XMLNode_GetProperty(xmlNode, "name", 0);
      if (gname) {
	DBG_DEBUG(GWEN_LOGDOMAIN, "Creating group \"%s\"", gname);
	gcfg=GWEN_DB_GetGroup(dbData, GWEN_PATH_FLAGS_CREATE_GROUP, gname);
	if (!gcfg) {
	  DBG_ERROR(GWEN_LOGDOMAIN, "Could not select group \"%s\"", gname);
	  return GWEN_ERROR_GENERIC;
	}
	DBG_DEBUG(GWEN_LOGDOMAIN, "Created group \"%s\"", gname);
      } /* if name given */
      else
	gcfg=dbData;
  
      /* read group */
      DBG_DEBUG(GWEN_LOGDOMAIN, "Reading group \"%s\"", gname);
      if (GWEN_MsgEngine__ReadGroup(e,
				    msgbuf,
				    xmlReferredNode,
				    xmlNode,
				    gcfg,
				    delimiters,
				    flags)) {
	DBG_INFO(GWEN_LOGDOMAIN, "Could not read group \"%s\"", gtype);
	return GWEN_ERROR_GENERIC;
      }
    }
    if (GWEN_Buffer_GetBytesLeft(msgbuf)) {
      if (currentDelimiter) {
	if (GWEN_Buffer_PeekByte(msgbuf)==currentDelimiter) {
	  GWEN_Buffer_IncrementPos(msgbuf, 1);
	  if (abortLoop && maxnum) {
	    uint32_t loopOpt=loopNr+1;
  
	    if (maxnum-loopOpt>GWEN_Buffer_GetBytesLeft(msgbuf))
	      /* Suspicious but not necessarily invalid, let's see */
	      maxnum=loopOpt+GWEN_Buffer_GetBytesLeft(msgbuf);
	    for (; loopOpt<maxnum; loopOpt++) {
	      if (GWEN_Buffer_PeekByte(msgbuf)!=currentDelimiter)
		break;
	      GWEN_Buffer_IncrementPos(msgbuf, 1);
	    }
	    if (loopOpt+1==maxnum && currentTerminator) {
	      if (GWEN_Buffer_PeekByte(msgbuf)==currentTerminator) {
		GWEN_Buffer_IncrementPos(msgbuf, 1);
		loopOpt++;
	      }
	    }
	    if (loopOpt<maxnum) {
	      DBG_ERROR(GWEN_LOGDOMAIN,
			"Delimiting character missing (pos=%d [%x]) "
			"expecting \"%c\", got \"%c\")",
			GWEN_Buffer_GetPos(msgbuf),
			GWEN_Buffer_GetPos(msgbuf),
			currentDelimiter,
			GWEN_Buffer_PeekByte(msgbuf));
	      GWEN_XMLNode_Dump(xmlNode, 2);
	      return GWEN_ERROR_GENERIC;
	    }
	  }
	}
      }
    }
    loopNr++;
  } /* while */
  if (loopNr<minnum) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Premature end of message (too few group repeats)");
    return GWEN_ERROR_GENERIC;
  }

  return 0;
}



int _groupReadElement(GWEN_MSGENGINE *e,
                      GWEN_BUFFER *msgbuf,
                      GWEN_XMLNODE *n,
                      GWEN_DB_NODE *dbData,
                      char currentDelimiter,
                      char currentTerminator,
                      const char *delimiters,
                      uint32_t flags)
{
  unsigned int loopNr;
  unsigned int minnum;
  unsigned int maxnum;
  const char *name;
  int abortLoop;

  DBG_VERBOUS(GWEN_LOGDOMAIN, "Reading element");

  /* get some sizes */
  //minsize=atoi(GWEN_XMLNode_GetProperty(n, "minsize","0"));
  //maxsize=atoi(GWEN_XMLNode_GetProperty(n, "maxsize","0"));
  minnum=GWEN_XMLNode_GetIntProperty(n, "minnum", 1);
  maxnum=GWEN_XMLNode_GetIntProperty(n, "maxnum", 1);
  name=GWEN_XMLNode_GetProperty(n, "name", 0);

  loopNr=0;
  abortLoop=0;
  while ((maxnum==0 || loopNr<maxnum) && !abortLoop) {
    int c;

    DBG_VERBOUS(GWEN_LOGDOMAIN, "Reading %s", name);
    if (GWEN_Buffer_GetBytesLeft(msgbuf)==0)
      break;
    c=GWEN_Buffer_PeekByte(msgbuf);
    if (c==-1) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "called from here");
      return -1;
    }

    DBG_VERBOUS(GWEN_LOGDOMAIN,
                "Checking delimiter at pos %x "
                "(whether \"%c\" is in \"%s\")",
                GWEN_Buffer_GetPos(msgbuf),
                c, delimiters);
    if (c && strchr(delimiters, c)) {
      abortLoop=1;
      DBG_VERBOUS(GWEN_LOGDOMAIN,
                  "Found delimiter (\"%c\" is in \"%s\")",
                  c, delimiters);
    } /* if delimiter found */
    else {
      /* current char is not a delimiter */
      if (name==0) {
        DBG_VERBOUS(GWEN_LOGDOMAIN, "no name");
      }
      else {
        /* name is given */
        int rv;
        const char *dtype;
        GWEN_BUFFER *vbuf;

        DBG_VERBOUS(GWEN_LOGDOMAIN, "Reading value from pos %x", GWEN_Buffer_GetPos(msgbuf));
        vbuf=GWEN_Buffer_new(0, GWEN_MSGENGINE_MAX_VALUE_LEN, 0, 0);
#ifdef HEAVY_DEBUG_MSGENGINE
        DBG_ERROR(GWEN_LOGDOMAIN, "Reading value from here:\n");
        GWEN_Text_DumpString(GWEN_Buffer_GetPosPointer(msgbuf),
                             GWEN_Buffer_GetBytesLeft(msgbuf),
                             1);
#endif

        rv=_readValue(e, msgbuf, n, vbuf, delimiters, flags);
        if (rv==1) {
          DBG_INFO(GWEN_LOGDOMAIN, "Empty value");
        }
        else if (rv==-1) {
          DBG_INFO(GWEN_LOGDOMAIN, "Error parsing xmlNode \"%s\" (ELEM)", name);
          GWEN_Buffer_free(vbuf);
          return -1;
        }

        GWEN_Buffer_Rewind(vbuf);

        /* special handling for binary data */
        dtype=GWEN_XMLNode_GetProperty(n, "type", "");
        if (GWEN_MsgEngine__IsBinTyp(e, dtype)) {
          if (atoi(GWEN_XMLNode_GetProperty(n, "readbin", "1")) && e->binTypeReadPtr) {
            rv=e->binTypeReadPtr(e, n, dbData, vbuf);
          }
          else
            rv=1;
          if (rv==-1) {
            DBG_INFO(GWEN_LOGDOMAIN, "Called from here");
            GWEN_Buffer_free(vbuf);
            return -1;
          }
          else if (rv==1) {
            /* bin type not handled, so handle it myself */
            if (GWEN_DB_SetBinValue(dbData,
                                    GWEN_DB_FLAGS_DEFAULT,
                                    name,
                                    GWEN_Buffer_GetStart(vbuf),
                                    GWEN_Buffer_GetUsedBytes(vbuf))) {
              DBG_INFO(GWEN_LOGDOMAIN, "Could not set value for \"%s\"", name);
              GWEN_Buffer_free(vbuf);
              return -1;
            }
          }
        } /* if type is bin */
        else if (GWEN_MsgEngine__IsIntTyp(e, dtype)) {
          int z;

          if (1!=sscanf(GWEN_Buffer_GetStart(vbuf), "%d", &z)) {
            DBG_INFO(GWEN_LOGDOMAIN, "Value for \"%s\" is not an integer", name);
            return -1;
          }
          if (GWEN_DB_SetIntValue(dbData, GWEN_DB_FLAGS_DEFAULT, name, z)) {
            DBG_INFO(GWEN_LOGDOMAIN, "Could not set int value for \"%s\"", name);
            return -1;
          }
        } /* if type is int */
        else {
          DBG_DEBUG(GWEN_LOGDOMAIN, "Value is \"%s\"", GWEN_Buffer_GetStart(vbuf));
          if (GWEN_DB_SetCharValue(dbData,
                                   GWEN_DB_FLAGS_DEFAULT,
                                   name,
                                   GWEN_Buffer_GetStart(vbuf))) {
            DBG_INFO(GWEN_LOGDOMAIN, "Could not set value for \"%s\"", name);
            return -1;
          }
        } /* if !bin */

        GWEN_Buffer_free(vbuf);
      } /* if name is given */
    } /* if current char is not a delimiter */

    if (GWEN_Buffer_GetBytesLeft(msgbuf)) {
      if (currentDelimiter) {
        if (GWEN_Buffer_PeekByte(msgbuf)==currentDelimiter) {
          GWEN_Buffer_IncrementPos(msgbuf, 1);
          if (abortLoop && maxnum) {
            uint32_t loopOpt=loopNr+1;

            if (maxnum-loopOpt>GWEN_Buffer_GetBytesLeft(msgbuf))
              /* Suspicious but not necessarily invalid, let's see */
              maxnum=loopOpt+GWEN_Buffer_GetBytesLeft(msgbuf);
            for (; loopOpt<maxnum; loopOpt++) {
              if (GWEN_Buffer_PeekByte(msgbuf)!=currentDelimiter)
                break;
              GWEN_Buffer_IncrementPos(msgbuf, 1);
            }
            if (loopOpt+1==maxnum && currentTerminator) {
              if (GWEN_Buffer_PeekByte(msgbuf)==currentTerminator) {
                GWEN_Buffer_IncrementPos(msgbuf, 1);
                loopOpt++;
              }
            }
            if (loopOpt<maxnum) {
              DBG_ERROR(GWEN_LOGDOMAIN,
                        "Delimiting character missing (pos=%d [%x]) "
                        "expecting \"%c\", got \"%c\")",
                        GWEN_Buffer_GetPos(msgbuf),
                        GWEN_Buffer_GetPos(msgbuf),
                        currentDelimiter,
                        GWEN_Buffer_PeekByte(msgbuf));
              GWEN_XMLNode_Dump(n, 2);
              return -1;
            }
          }
        }
      }
    }
    loopNr++;
  } /* while */
  if (loopNr<minnum) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Premature end of message (too few ELEM repeats)");
    GWEN_XMLNode_Dump(n, 2);
    return -1;
  }

  return 0;
}



int _readValue(GWEN_MSGENGINE *e,
	       GWEN_BUFFER *msgbuf,
	       GWEN_XMLNODE *xmlNode,
	       GWEN_BUFFER *vbuf,
	       const char *delimiters,
	       uint32_t flags)
{
  unsigned int minsize;
  unsigned int maxsize;
  unsigned int size;
  unsigned int minnum;
  GWEN_MSGENGINE_TRUSTLEVEL trustLevel;
  //unsigned int posInMsg;
  const char *type;
  int rv;
  unsigned int realSize;

  /* get some sizes */
  //posInMsg=GWEN_Buffer_GetPos(msgbuf);
  realSize=0;
  size=atoi(GWEN_XMLNode_GetProperty(xmlNode, "size", "0"));
  minsize=GWEN_XMLNode_GetIntProperty(xmlNode, "minsize", 0);
  maxsize=GWEN_XMLNode_GetIntProperty(xmlNode, "maxsize", 0);
  minnum=GWEN_XMLNode_GetIntProperty(xmlNode, "minnum", 0);
  type=GWEN_XMLNode_GetProperty(xmlNode, "type", "ASCII");

  rv=1;
  if (e->typeReadPtr) {
    rv=e->typeReadPtr(e, msgbuf, xmlNode, vbuf, e->escapeChar, delimiters);
  }
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "External type reading failed on type \"%s\" (%d)", type, rv);
    return rv;
  }
  else if (rv==1) {
    if (strcasecmp(type, "bin")==0)
      rv=_readBinDataIntoBuffer(msgbuf, vbuf);
    else
      rv=_readNonBinDataIntoBuffer(msgbuf, e->escapeChar, delimiters, size, vbuf);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  } /* if type not external */
  else {
    DBG_DEBUG(GWEN_LOGDOMAIN, "External type read (\"%s\")", type);
  }

  realSize=GWEN_Buffer_GetUsedBytes(vbuf);

  /* check the value */
  if (realSize==0) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "Datasize is 0");
    if (minnum==0) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "... but thats ok");
      /* value is empty, and that is allowed */
      return 1;
    }
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "Value missing");
      GWEN_XMLNode_Dump(xmlNode, 1);
      return GWEN_ERROR_GENERIC;
    }
  }

  /* check minimum size */
  if (minsize!=0 && realSize<minsize) {
    DBG_INFO(GWEN_LOGDOMAIN, "Value too short (%d<%d).", realSize, minsize);
    return GWEN_ERROR_GENERIC;
  }

  /* check maximum size */
  if (maxsize!=0 && realSize>maxsize) {
    DBG_INFO(GWEN_LOGDOMAIN, "Value too long (%d>%d).", realSize, maxsize);
    return GWEN_ERROR_GENERIC;
  }

  if (flags & GWEN_MSGENGINE_READ_FLAGS_TRUSTINFO) {
    /* add trust data to msgEngine */
    const char *descr;

    trustLevel=GWEN_MsgEngine_GetHighestTrustLevel(xmlNode, NULL);
    if (trustLevel) {
      unsigned int ustart;

      ustart=GWEN_Buffer_GetPos(msgbuf)-realSize;
      descr=GWEN_XMLNode_GetProperty(xmlNode, "name", 0);
      if (GWEN_MsgEngine_AddTrustInfo(e,
                                      GWEN_Buffer_GetStart(vbuf),
                                      realSize,
                                      descr,
                                      trustLevel,
                                      ustart)) {
        DBG_INFO(GWEN_LOGDOMAIN, "called from here");
        return GWEN_ERROR_GENERIC;
      }
    }
  }

  return 0;
}




int _readBinDataLength(GWEN_BUFFER *msgbuf)
{
  int result=0;
  int i=0;

  while (i<GWEN_MSGENGINE_MAXBINDATASIZE) {
    int nc;

    nc=GWEN_Buffer_PeekByte(msgbuf);
    if (nc<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", nc);
      return GWEN_ERROR_BAD_DATA;
    }

    if (!isdigit(nc))
      return result;

    nc=GWEN_Buffer_ReadByte(msgbuf);

    nc-='0';
    result*=10;
    result+=nc;
    i++;
  }

  DBG_ERROR(GWEN_LOGDOMAIN, "To many bytes in bindata size spec (%d so far)", i);
  return GWEN_ERROR_BAD_DATA;
}



int _readBinDataIntoBuffer(GWEN_BUFFER *msgbuf, GWEN_BUFFER *vbuf)
{
  if (GWEN_Buffer_GetBytesLeft(msgbuf)==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Premature end of message (@num@ expected)");
    return GWEN_ERROR_GENERIC;
  }
  else {
    int c;
    int l;

    /* read opening @ */
    c=GWEN_Buffer_ReadByte(msgbuf);
    if (c!='@') {
      DBG_ERROR(GWEN_LOGDOMAIN, "\"@num@\" expected (missing opening @)");
      return GWEN_ERROR_GENERIC;
    }

    l=_readBinDataLength(msgbuf);
    if (l<0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "\"@num@\" expected (invalid length)");
      return GWEN_ERROR_GENERIC;
    }

    /* read closing @ */
    c=GWEN_Buffer_ReadByte(msgbuf);
    if (c!='@') {
      DBG_ERROR(GWEN_LOGDOMAIN, "\"@num@\" expected (missing closing @)");
      return GWEN_ERROR_GENERIC;
    }

    DBG_DEBUG(GWEN_LOGDOMAIN, "Reading binary: %d bytes from pos %d (msgsize=%d)",
	      l,
	      GWEN_Buffer_GetPos(msgbuf),
	      GWEN_Buffer_GetUsedBytes(msgbuf));
    if (GWEN_Buffer_GetBytesLeft(msgbuf) < (unsigned) l) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Premature end of message (binary beyond end)");
      return GWEN_ERROR_GENERIC;
    }
    if (GWEN_Buffer_AppendBytes(vbuf, GWEN_Buffer_GetPosPointer(msgbuf), l)) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "Called from here");
      return GWEN_ERROR_GENERIC;
    }
    GWEN_Buffer_IncrementPos(msgbuf, l);

    return l;
  }
}



int _readNonBinDataIntoBuffer(GWEN_BUFFER *msgbuf, char escapeChar, const char *delimiters, int fixedSize, GWEN_BUFFER *vbuf)
{
  /* type is not bin */
  int lastWasEscape;
  int isEscaped;
  int bytesRead;
  
  isEscaped=0;
  lastWasEscape=0;
  
  bytesRead=0;
  while (GWEN_Buffer_GetBytesLeft(msgbuf) && (fixedSize==0 || fixedSize<bytesRead)) {
    int c;

    c=GWEN_Buffer_ReadByte(msgbuf);
    if (lastWasEscape) {
      lastWasEscape=0;
      isEscaped=1;
    }
    else {
      isEscaped=0;
      if (c==escapeChar) {
	lastWasEscape=1;
	c=-1;
      }
    }
    if (c!=-1) {
      if (!isEscaped && (c && strchr(delimiters, c)!=0)) {
	/* delimiter found, step back */
	GWEN_Buffer_DecrementPos(msgbuf, 1);
	break;
      }
      else {
	if (c=='\\' || iscntrl(c)) {
	  DBG_WARN(GWEN_LOGDOMAIN, "Found a bad character (%02x), converting to SPACE", (unsigned int)c);
	  c=' ';
	}
	if (GWEN_Buffer_AppendByte(vbuf, c)) {
	  DBG_DEBUG(GWEN_LOGDOMAIN, "Called from here");
	  return GWEN_ERROR_GENERIC;
	}
	bytesRead++;
      }
    }
  } /* while */

  return bytesRead;
}




