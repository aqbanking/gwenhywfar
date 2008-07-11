/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Fri Jul 04 2003
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

//#define DISABLE_DEBUGLOG

#include <gwenhywfar/gwenhywfarapi.h>
#include <msgengine_p.h>
#include <gwenhywfar/xml.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/path.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/buffer.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>


GWEN_INHERIT_FUNCTIONS(GWEN_MSGENGINE)


GWEN_MSGENGINE *GWEN_MsgEngine_new(){
  GWEN_MSGENGINE *e;

  GWEN_NEW_OBJECT(GWEN_MSGENGINE, e);
  GWEN_INHERIT_INIT(GWEN_MSGENGINE, e);
  e->charsToEscape=strdup(GWEN_MSGENGINE_CHARSTOESCAPE);
  e->delimiters=strdup(GWEN_MSGENGINE_DEFAULT_DELIMITERS);
  e->globalValues=GWEN_DB_Group_new("globalvalues");
  e->escapeChar='\\';

  e->usage=1;
  return e;
}


void GWEN_MsgEngine_free(GWEN_MSGENGINE *e){
  if (e) {
    assert(e->usage);
    if (--(e->usage)==0) {
      GWEN_INHERIT_FINI(GWEN_MSGENGINE, e);

      if (e->inheritorData && e->freeDataPtr)
	e->freeDataPtr(e);
      if (e->ownDefs)
	GWEN_XMLNode_free(e->defs);
      free(e->charsToEscape);
      free(e->delimiters);
      GWEN_DB_Group_free(e->globalValues);
      if (e->trustInfos) {
	/* free trustInfos */
	GWEN_MSGENGINE_TRUSTEDDATA *td, *tdn;

	td=e->trustInfos;
	while(td) {
	  tdn=td->next;
	  GWEN_MsgEngine_TrustedData_free(td);
	  td=tdn;
	} /* while */
      }
      GWEN_FREE_OBJECT(e);
    }
  }
}



void GWEN_MsgEngine_Attach(GWEN_MSGENGINE *e){
  assert(e);
  e->usage++;
}


void GWEN_MsgEngine_SetEscapeChar(GWEN_MSGENGINE *e, char c){
  assert(e);
  e->escapeChar=c;
}



char GWEN_MsgEngine_GetEscapeChar(GWEN_MSGENGINE *e){
  assert(e);
  return e->escapeChar;
}



void GWEN_MsgEngine_SetCharsToEscape(GWEN_MSGENGINE *e, const char *c){
  assert(e);
  free(e->charsToEscape);
  e->charsToEscape=strdup(c);
}



const char *GWEN_MsgEngine_GetCharsToEscape(GWEN_MSGENGINE *e){
  assert(e);
  return e->charsToEscape;
}



void GWEN_MsgEngine_SetDelimiters(GWEN_MSGENGINE *e, const char *s){
  assert(e);
  free(e->delimiters);
  if (s)
    e->delimiters=strdup(s);
  else
    e->delimiters=strdup(GWEN_MSGENGINE_DEFAULT_DELIMITERS);
}



const char *GWEN_MsgEngine_GetDelimiters(GWEN_MSGENGINE *e){
  assert(e);
  return e->delimiters;
}



void GWEN_MsgEngine_SetMode(GWEN_MSGENGINE *e, const char *mode){
  GWEN_DB_NODE *db;

  assert(e);
  db=GWEN_MsgEngine__GetGlobalValues(e);

  if (mode)
    GWEN_DB_SetCharValue(db,
                         GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "engine/secmode",
                         mode);
  else
    GWEN_DB_DeleteVar(db, "engine/secmode");
}


const char *GWEN_MsgEngine_GetMode(GWEN_MSGENGINE *e){
  GWEN_DB_NODE *db;

  assert(e);
  db=GWEN_MsgEngine__GetGlobalValues(e);
  return GWEN_DB_GetCharValue(db, "engine/secmode", 0, 0);
}



GWEN_DB_NODE *GWEN_MsgEngine__GetGlobalValues(GWEN_MSGENGINE *e){
  GWEN_DB_NODE *globalValues;

  assert(e);
  if (e->getGlobalValuesPtr) {
    globalValues=e->getGlobalValuesPtr(e);
    if (!globalValues)
      globalValues=e->globalValues;
  }
  else {
    globalValues=e->globalValues;
  }
  assert(globalValues);
  return globalValues;
}



unsigned int GWEN_MsgEngine_GetProtocolVersion(GWEN_MSGENGINE *e){
  GWEN_DB_NODE *db;

  assert(e);
  db=GWEN_MsgEngine__GetGlobalValues(e);
  return GWEN_DB_GetIntValue(db, "engine/pversion", 0, 0);
}



void GWEN_MsgEngine_SetProtocolVersion(GWEN_MSGENGINE *e,
                                       unsigned int p){
  GWEN_DB_NODE *db;

  assert(e);
  db=GWEN_MsgEngine__GetGlobalValues(e);

  GWEN_DB_SetIntValue(db,
                      GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "engine/pversion",
                      p);
}



GWEN_XMLNODE *GWEN_MsgEngine_GetDefinitions(GWEN_MSGENGINE *e){
  assert(e);
  return e->defs;
}


void GWEN_MsgEngine_SetDefinitions(GWEN_MSGENGINE *e,
                                   GWEN_XMLNODE *n,
                                   int take){
  assert(e);
  if (e->ownDefs)
    GWEN_XMLNode_free(e->defs);
  e->defs=n;
  e->ownDefs=take;
}



void
GWEN_MsgEngine_SetGetGlobalValuesFunction(GWEN_MSGENGINE *e,
                                          GWEN_MSGENGINE_GETGLOBALVALUES_PTR p){
  assert(e);
  e->getGlobalValuesPtr=p;
}



GWEN_MSGENGINE_GETGLOBALVALUES_PTR
GWEN_MsgEngine_GetGetGlobalValuesFunction(GWEN_MSGENGINE *e){
  assert(e);
  return e->getGlobalValuesPtr;
}



void GWEN_MsgEngine_SetTypeReadFunction(GWEN_MSGENGINE *e,
                                        GWEN_MSGENGINE_TYPEREAD_PTR p){
  assert(e);
  e->typeReadPtr=p;
}



GWEN_MSGENGINE_TYPEREAD_PTR
GWEN_MsgEngine_GetTypeReadFunction(GWEN_MSGENGINE *e){
  assert(e);
  return e->typeReadPtr;
}



void GWEN_MsgEngine_SetTypeWriteFunction(GWEN_MSGENGINE *e,
                                         GWEN_MSGENGINE_TYPEWRITE_PTR p){
  assert(e);
  e->typeWritePtr=p;
}



GWEN_MSGENGINE_TYPEWRITE_PTR
GWEN_MsgEngine_GetTypeWriteFunction(GWEN_MSGENGINE *e){
  assert(e);
  return e->typeWritePtr;
}



void GWEN_MsgEngine_SetTypeCheckFunction(GWEN_MSGENGINE *e,
                                         GWEN_MSGENGINE_TYPECHECK_PTR p){
  assert(e);
  e->typeCheckPtr=p;
}



GWEN_MSGENGINE_TYPECHECK_PTR
GWEN_MsgEngine_GetTypeCheckFunction(GWEN_MSGENGINE *e){
  assert(e);
  return e->typeCheckPtr;
}






void GWEN_MsgEngine_SetBinTypeReadFunction(GWEN_MSGENGINE *e,
                                           GWEN_MSGENGINE_BINTYPEREAD_PTR p){
  assert(e);
  e->binTypeReadPtr=p;
}



GWEN_MSGENGINE_BINTYPEREAD_PTR
GWEN_MsgEngine_GetBinTypeReadFunction(GWEN_MSGENGINE *e){
  assert(e);
  return e->binTypeReadPtr;
}



void
GWEN_MsgEngine_SetBinTypeWriteFunction(GWEN_MSGENGINE *e,
                                       GWEN_MSGENGINE_BINTYPEWRITE_PTR p){
  assert(e);
  e->binTypeWritePtr=p;
}



GWEN_MSGENGINE_BINTYPEWRITE_PTR
GWEN_MsgEngine_GetBinTypeWriteFunction(GWEN_MSGENGINE *e){
  assert(e);
  return e->binTypeWritePtr;
}



void
GWEN_MsgEngine_SetGetCharValueFunction(GWEN_MSGENGINE *e,
                                       GWEN_MSGENGINE_GETCHARVALUE_PTR p){
  assert(e);
  e->getCharValuePtr=p;
}



void
GWEN_MsgEngine_SetGetIntValueFunction(GWEN_MSGENGINE *e,
                                      GWEN_MSGENGINE_GETINTVALUE_PTR p){
  assert(e);
  e->getIntValuePtr=p;
}



void
GWEN_MsgEngine_SetFreeDataFunction(GWEN_MSGENGINE *e,
                                   GWEN_MSGENGINE_FREEDATA_PTR p){
  assert(e);
  DBG_WARN(GWEN_LOGDOMAIN, "GWEN_MsgEngine_SetFreeDataFunction: Deprecated");
  e->freeDataPtr=p;
}



void *GWEN_MsgEngine_GetInheritorData(const GWEN_MSGENGINE *e){
  assert(e);
  return e->inheritorData;
}



void GWEN_MsgEngine_SetInheritorData(GWEN_MSGENGINE *e, void *d){
  assert(e);
  DBG_WARN(GWEN_LOGDOMAIN, "GWEN_MsgEngine_SetInheritorData: Deprecated");
  if (e->inheritorData && e->freeDataPtr)
    e->freeDataPtr(e);
  e->inheritorData=d;
}



int GWEN_MsgEngine__WriteValue(GWEN_MSGENGINE *e,
                               GWEN_BUFFER *gbuf,
                               GWEN_BUFFER *data,
                               GWEN_XMLNODE *node) {
  unsigned int minsize;
  unsigned int maxsize;
  unsigned int fixSize;
  unsigned int startPos;
  int filler;
  const char *type;
  const char *name;
  int rv;

  /* get some sizes */
  minsize=atoi(GWEN_XMLNode_GetProperty(node, "minsize","0"));
  maxsize=atoi(GWEN_XMLNode_GetProperty(node, "maxsize","0"));
  fixSize=atoi(GWEN_XMLNode_GetProperty(node, "size","0"));
  filler=atoi(GWEN_XMLNode_GetProperty(node, "filler","0"));
  type=GWEN_XMLNode_GetProperty(node, "type","ASCII");
  name=GWEN_XMLNode_GetProperty(node, "name","<unnamed>");
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
      int num;
      unsigned int len;
      unsigned int lj;

      num=atoi(GWEN_Buffer_GetPosPointer(data));
      len=strlen(GWEN_Buffer_GetPosPointer(data));

      if (atoi(GWEN_XMLNode_GetProperty(node, "leftfill","0"))) {
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
      else if (atoi(GWEN_XMLNode_GetProperty(node, "rightfill","0"))) {
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
      while(*p && pcount<GWEN_Buffer_GetUsedBytes(data)) {
	int c;

	c=(unsigned char)*p;
	if (lastWasEscape) {
	  lastWasEscape=0;
	  switch(c) {
	  case 'r': c='\r'; break;
	  case 'n': c='\n'; break;
	  case 'f': c='\f'; break;
	  case 't': c='\t'; break;
	  default: c=(unsigned char)*p;
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



int GWEN_MsgEngine__IsCharTyp(GWEN_MSGENGINE *e,
                              const char *type) {
  if (e->typeCheckPtr) {
    GWEN_DB_NODE_TYPE vt;

    vt=e->typeCheckPtr(e, type);
    if (vt!=GWEN_DB_NodeType_Unknown) {
      if (vt==GWEN_DB_NodeType_ValueChar)
        return 1;
    }
  }
  return
    (strcasecmp(type, "alpha")==0) ||
    (strcasecmp(type, "ascii")==0) ||
    (strcasecmp(type, "an")==0) ||
    (strcasecmp(type, "float")==0);
}



int GWEN_MsgEngine__IsIntTyp(GWEN_MSGENGINE *e,
                             const char *type) {
  if (e->typeCheckPtr) {
    GWEN_DB_NODE_TYPE vt;

    vt=e->typeCheckPtr(e, type);
    if (vt!=GWEN_DB_NodeType_Unknown) {
      if (vt==GWEN_DB_NodeType_ValueInt)
        return 1;
    }
  }
  return
    (strcasecmp(type, "num")==0);
}



int GWEN_MsgEngine__IsBinTyp(GWEN_MSGENGINE *e,
                             const char *type) {
  if (e->typeCheckPtr) {
    GWEN_DB_NODE_TYPE vt;

    vt=e->typeCheckPtr(e, type);
    if (vt!=GWEN_DB_NodeType_Unknown) {
      if (vt==GWEN_DB_NodeType_ValueBin)
        return 1;
    }
  }
  return
    (strcasecmp(type, "bin")==0);
}



int GWEN_MsgEngine__GetInline(GWEN_MSGENGINE *e,
			      GWEN_XMLNODE *node,
			      GWEN_BUFFER *mbuf) {
  /* get data from within the XML node */
  GWEN_XMLNODE *n;
  const char *type;


  type=GWEN_XMLNode_GetProperty(node, "type", "ascii");
  DBG_DEBUG(GWEN_LOGDOMAIN,
            "Getting data of type \"%s\" from within XML file", type);
  n=GWEN_XMLNode_GetFirstData(node);
  if (!n) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "No child");
    return 1;
  }

  if (GWEN_MsgEngine__IsBinTyp(e, type)) {
    const char *dp;
    unsigned int dplen;
    const char *stype;

    stype=GWEN_XMLNode_GetProperty(node, "storedAs", type);
    if (GWEN_MsgEngine__IsBinTyp(e, stype)) {
      dp=GWEN_XMLNode_GetData(n);
      dplen=strlen(dp);
      if (GWEN_Text_FromHexBuffer(dp, mbuf)) {
	DBG_INFO(GWEN_LOGDOMAIN, "here");
	return -1;
      }
    } /* if stored as bin */
    else {
      /* stored as char */
      GWEN_Buffer_AppendString(mbuf, GWEN_XMLNode_GetData(n));
    }
  } /* if binType */
  else {
    GWEN_Buffer_AppendString(mbuf, GWEN_XMLNode_GetData(n));
  }

  return 0;
}





int GWEN_MsgEngine__WriteElement(GWEN_MSGENGINE *e,
                                 GWEN_BUFFER *gbuf,
                                 GWEN_XMLNODE *node,
                                 GWEN_XMLNODE *rnode,
                                 GWEN_DB_NODE *gr,
                                 int loopNr,
                                 int isOptional,
                                 GWEN_XMLNODE_PATH *nodePath) {
  const char *name;
  const char *type;
  unsigned int minsize;
  unsigned int maxsize;
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
  type=GWEN_XMLNode_GetProperty(node, "type","ASCII");
  DBG_DEBUG(GWEN_LOGDOMAIN, "Type is \"%s\"", type);
  /* get some sizes */
  minsize=atoi(GWEN_XMLNode_GetProperty(node, "minsize","0"));
  maxsize=atoi(GWEN_XMLNode_GetProperty(node, "maxsize","0"));

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
        switch(vt) {
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
                                          sizeof(numbuffer),0)) {
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
          GWEN_DB_Dump(gr, stderr, 4);
	  GWEN_Buffer_free(data);
          return -1;
        }
      }
    }

    if (!data)
      data=GWEN_Buffer_new((char*)pdata,
                           datasize,
                           datasize,
                           0 /* dont take ownership*/ );
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



GWEN_XMLNODE *GWEN_MsgEngine_FindGroupByProperty(GWEN_MSGENGINE *e,
                                                 const char *pname,
                                                 int version,
                                                 const char *pvalue) {
  return GWEN_MsgEngine_FindNodeByProperty(e, "GROUP", pname, version, pvalue);
}



GWEN_XMLNODE *GWEN_MsgEngine_FindNodeByProperty(GWEN_MSGENGINE *e,
                                                const char *t,
                                                const char *pname,
                                                int version,
                                                const char *pvalue) {
  GWEN_XMLNODE *n;
  const char *p;
  int i;
  const char *mode;
  unsigned int proto;
  char buffer[256];

  if ((strlen(t)+4)>sizeof(buffer)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Type name too long.");
    return 0;
  }

  mode=GWEN_MsgEngine_GetMode(e);
  proto=GWEN_MsgEngine_GetProtocolVersion(e);
  if (!e->defs) {
    DBG_INFO(GWEN_LOGDOMAIN, "No definitions available");
    return 0;
  }
  n=e->defs;
  n=GWEN_XMLNode_GetChild(n);

  /* find type+"S" */
  strcpy(buffer, t);
  strcat(buffer,"S");
  while(n) {
    if (GWEN_XMLNode_GetType(n)==GWEN_XMLNodeTypeTag) {
      p=GWEN_XMLNode_GetData(n);
      assert(p);
      if (strcasecmp(p, buffer)==0)
	break;
    }
    n=GWEN_XMLNode_Next(n);
  } /* while */

  if (!n) {
    DBG_INFO(GWEN_LOGDOMAIN, "No definitions available for type \"%s\"", t);
    return 0;
  }

  /* find approppriate group definition */
  if (!mode)
    mode="";
  n=GWEN_XMLNode_GetChild(n);
  if (!n) {
    DBG_INFO(GWEN_LOGDOMAIN, "No definitions inside \"%s\"", buffer);
    return 0;
  }

  /* find type+"def" */
  strcpy(buffer, t);
  strcat(buffer,"def");
  while(n) {
    if (GWEN_XMLNode_GetType(n)==GWEN_XMLNodeTypeTag) {
      p=GWEN_XMLNode_GetData(n);
      assert(p);
      if (strcasecmp(p, buffer)==0) {
	p=GWEN_XMLNode_GetProperty(n, pname,"");
        if (strcasecmp(p, pvalue)==0) {
          i=atoi(GWEN_XMLNode_GetProperty(n, "pversion" ,"0"));
          if (proto==0 || (int)proto==i || i==0) {
            i=atoi(GWEN_XMLNode_GetProperty(n, "version" ,"0"));
            if (version==0 || version==i) {
              p=GWEN_XMLNode_GetProperty(n, "mode","");
              if (strcasecmp(p, mode)==0 || !*p) {
                DBG_DEBUG(GWEN_LOGDOMAIN, "Group definition for \"%s=%s\" found",
                          pname, pvalue);
                return n;
              }
            }
          }
        }
      }
    }
    n=GWEN_XMLNode_Next(n);
  } /* while */

  DBG_INFO(GWEN_LOGDOMAIN, "Group definition for \"%s=%s\"(%d) not found",
           pname,
           pvalue,
           version);
  return 0;
}



GWEN_XMLNODE *GWEN_MsgEngine_FindNodeByPropertyStrictProto(GWEN_MSGENGINE *e,
							   const char *t,
							   const char *pname,
							   int version,
							   const char *pvalue) {
  GWEN_XMLNODE *n;
  const char *p;
  int i;
  const char *mode;
  unsigned int proto;
  char buffer[256];

  if ((strlen(t)+4)>sizeof(buffer)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Type name too long.");
    return 0;
  }

  mode=GWEN_MsgEngine_GetMode(e);
  proto=GWEN_MsgEngine_GetProtocolVersion(e);
  if (!e->defs) {
    DBG_INFO(GWEN_LOGDOMAIN, "No definitions available");
    return 0;
  }
  n=e->defs;
  n=GWEN_XMLNode_GetChild(n);

  /* find type+"S" */
  strcpy(buffer, t);
  strcat(buffer,"S");
  while(n) {
    if (GWEN_XMLNode_GetType(n)==GWEN_XMLNodeTypeTag) {
      p=GWEN_XMLNode_GetData(n);
      assert(p);
      if (strcasecmp(p, buffer)==0)
	break;
    }
    n=GWEN_XMLNode_Next(n);
  } /* while */

  if (!n) {
    DBG_INFO(GWEN_LOGDOMAIN, "No definitions available for type \"%s\"", t);
    return 0;
  }

  /* find approppriate group definition */
  if (!mode)
    mode="";
  n=GWEN_XMLNode_GetChild(n);
  if (!n) {
    DBG_INFO(GWEN_LOGDOMAIN, "No definitions inside \"%s\"", buffer);
    return 0;
  }

  /* find type+"def" */
  strcpy(buffer, t);
  strcat(buffer,"def");
  while(n) {
    if (GWEN_XMLNode_GetType(n)==GWEN_XMLNodeTypeTag) {
      p=GWEN_XMLNode_GetData(n);
      assert(p);
      if (strcasecmp(p, buffer)==0) {
	p=GWEN_XMLNode_GetProperty(n, pname,"");
        if (strcasecmp(p, pvalue)==0) {
          i=atoi(GWEN_XMLNode_GetProperty(n, "pversion" ,"0"));
	  if (proto==0 || (int)proto==i) {
	    i=atoi(GWEN_XMLNode_GetProperty(n, "version" ,"0"));
	    if (version==0 || version==i) {
	      p=GWEN_XMLNode_GetProperty(n, "mode","");
	      if (strcasecmp(p, mode)==0 || !*p) {
		DBG_DEBUG(GWEN_LOGDOMAIN, "Group definition for \"%s=%s\" found",
                          pname, pvalue);
                return n;
	      }
	    }
          }
	}
      }
    }
    n=GWEN_XMLNode_Next(n);
  } /* while */

  DBG_INFO(GWEN_LOGDOMAIN, "Group definition for \"%s=%s\"(%d) not found",
           pname,
           pvalue,
           version);
  return 0;
}



const char *GWEN_MsgEngine__TransformValue(GWEN_MSGENGINE *e,
                                           const char *pvalue,
                                           GWEN_XMLNODE *node,
                                           GWEN_XMLNODE *dnode,
                                           unsigned int *datasize) {
  const char *p;
  static char pbuffer[256];
  GWEN_DB_NODE *globalValues;

  globalValues=GWEN_MsgEngine__GetGlobalValues(e);
  assert(globalValues);

  if (pvalue) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "Transforming value \"%s\"", pvalue);
    /* check whether the value is a property */
    p=pvalue;
    while (*p && isspace((int)*p))
      p++;
    if (*p=='$' || *p=='+') {
      /* global property */
      int incr;

      incr=(*p=='+');
      p++;

      DBG_DEBUG(GWEN_LOGDOMAIN, "Getting global property \"%s\"", p);
      if (incr) {
        int z;

        z=GWEN_DB_GetIntValue(globalValues, p, 0, 0);
        DBG_DEBUG(GWEN_LOGDOMAIN, "Incrementing global property \"%s\" (%d)",
                  p, z);
        if (GWEN_Text_NumToString(z, pbuffer, sizeof(pbuffer),0)<1) {
          DBG_ERROR(GWEN_LOGDOMAIN, "Error converting num to string");
          return 0;
        }

	z++;
        DBG_DEBUG(GWEN_LOGDOMAIN, "Setting global property \"%s\"=%d", p, z);
        GWEN_DB_SetIntValue(globalValues,
                            GWEN_DB_FLAGS_DEFAULT |
                            GWEN_DB_FLAGS_OVERWRITE_VARS,
                            p, z);
        pvalue=pbuffer;
        *datasize=strlen(pvalue);
      }
      else {
        int z;
        GWEN_DB_NODE_TYPE vt;
        const char *type = "should_be_known";
	/* default value; otherwise the compiler issues a warning */

        DBG_DEBUG(GWEN_LOGDOMAIN, "Getting global property \"%s\"", p);
        vt=GWEN_DB_GetVariableType(globalValues, p);
        if (vt==GWEN_DB_NodeType_Unknown) {
          if (!GWEN_DB_VariableExists(globalValues, p)) {
            DBG_ERROR(GWEN_LOGDOMAIN, "Unable to determine type of \"%s\"", p);
            return 0;
          }
          type=GWEN_XMLNode_GetProperty(dnode, "type", "ascii");
          if (GWEN_MsgEngine__IsCharTyp(e, type))
            vt=GWEN_DB_NodeType_ValueChar;
          else if (GWEN_MsgEngine__IsIntTyp(e, type))
            vt=GWEN_DB_NodeType_ValueInt;
          else if (GWEN_MsgEngine__IsBinTyp(e, type))
            vt=GWEN_DB_NodeType_ValueBin;
          else {
            DBG_ERROR(GWEN_LOGDOMAIN,
                      "Unable to determine type of \"%s\" (xml)", p);
            return 0;
          }
        }

        switch(vt) {
        case GWEN_DB_NodeType_ValueChar:
          pvalue=GWEN_DB_GetCharValue(globalValues, p, 0, "");
          *datasize=strlen(pvalue);
          break;

        case GWEN_DB_NodeType_ValueInt:
          z=GWEN_DB_GetIntValue(globalValues, p, 0, 0);
          if (GWEN_Text_NumToString(z, pbuffer, sizeof(pbuffer),0)<1) {
            DBG_ERROR(GWEN_LOGDOMAIN, "Error converting num to string");
            return 0;
          }
          pvalue=pbuffer;
          *datasize=strlen(pvalue);
          break;

        case GWEN_DB_NodeType_ValueBin:
          pvalue=GWEN_DB_GetBinValue(globalValues, p, 0,
                                     0,0,
                                     datasize);
          break;

        default:
          DBG_ERROR(GWEN_LOGDOMAIN,"Unknown type %s", type);
          return 0;
        } /* switch */
      }
      DBG_DEBUG(GWEN_LOGDOMAIN, "Value transformed");
    }
    else if (*p=='%') {
      /* local property */
      p++;

      DBG_DEBUG(GWEN_LOGDOMAIN, "Getting property \"%s\"", p);
      pvalue=GWEN_XMLNode_GetProperty(node, p, 0);
      if (pvalue) {
        *datasize=strlen(pvalue);
        DBG_DEBUG(GWEN_LOGDOMAIN, "Transformed value \"%s\"", pvalue);
      }
      else
        *datasize=0;
    }
    else if (*p=='?') {
      GWEN_DB_NODE_TYPE vt;
      int z;
      const char *dtype;

      /* get type */
      dtype=GWEN_XMLNode_GetProperty(dnode, "type","ASCII");

      /* program variable accessable via callback */
      p++;
      DBG_DEBUG(GWEN_LOGDOMAIN, "Getting program variable \"%s\"", p);

      pvalue=0;
      if (GWEN_MsgEngine__IsCharTyp(e, dtype))
        vt=GWEN_DB_NodeType_ValueChar;
      else if (GWEN_MsgEngine__IsIntTyp(e, dtype))
        vt=GWEN_DB_NodeType_ValueInt;
      else {
        vt=GWEN_DB_NodeType_ValueChar;
      }

      switch(vt) {
      case GWEN_DB_NodeType_ValueChar:
        if (e->getCharValuePtr) {
          pvalue=e->getCharValuePtr(e, p, 0);
          if (pvalue)
            *datasize=strlen(pvalue);
        }
        break;

      case GWEN_DB_NodeType_ValueInt:
        if (e->getIntValuePtr) {
          z=e->getIntValuePtr(e, p, 0);
          if (GWEN_Text_NumToString(z, pbuffer, sizeof(pbuffer),0)<1) {
            DBG_ERROR(GWEN_LOGDOMAIN, "Error converting num to string");
            return 0;
          }
          pvalue=pbuffer;
          *datasize=strlen(pvalue);
        }
        else {
          DBG_NOTICE(GWEN_LOGDOMAIN, "Callback for getIntValue not set");
        }
        break;

      default:
        DBG_ERROR(GWEN_LOGDOMAIN,"Unhandled type %s", dtype);
        return 0;
      } /* switch */

      DBG_DEBUG(GWEN_LOGDOMAIN, "Value transformed");
    }
    else {
      *datasize=strlen(pvalue);
    }
  }
  return pvalue;
}



const char *GWEN_MsgEngine_SearchForProperty(GWEN_XMLNODE *node,
                                             GWEN_XMLNODE *refnode,
                                             const char *name,
                                             int topDown) {
  const char *pvalue;
  GWEN_XMLNODE *pn;
  const char *lastValue;

  DBG_DEBUG(GWEN_LOGDOMAIN, "Searching for value of \"%s\" in properties", name);
  lastValue=0;

  pvalue=GWEN_XMLNode_GetProperty(node, name,0);
  if (pvalue) {
    if (!topDown)
      return pvalue;
    DBG_DEBUG(GWEN_LOGDOMAIN, "Found a value (%s), but will look further", pvalue);
    lastValue=pvalue;
  }

  pn=refnode;
  while(pn) {
    pvalue=GWEN_XMLNode_GetProperty(pn, name,0);
    if (pvalue) {
      if (!topDown)
        return pvalue;
      DBG_DEBUG(GWEN_LOGDOMAIN, "Found a value (%s), but will look further", pvalue);
      lastValue=pvalue;
    }
    pn=GWEN_XMLNode_GetParent(pn);
  } /* while */
  return lastValue;
}



int GWEN_MsgEngine_GetHighestTrustLevel(GWEN_XMLNODE *node,
					GWEN_XMLNODE *refnode) {
  int value;
  GWEN_XMLNODE *pn;
  int highestTrust;

  highestTrust=0;

  value=atoi(GWEN_XMLNode_GetProperty(node, "trustlevel","0"));
  if (value>highestTrust)
    highestTrust=value;

  pn=node;
  while(pn) {
    value=atoi(GWEN_XMLNode_GetProperty(pn, "trustlevel","0"));
    if (value>highestTrust)
      highestTrust=value;
    pn=GWEN_XMLNode_GetParent(pn);
  } /* while */

  pn=refnode;
  while(pn) {
    value=atoi(GWEN_XMLNode_GetProperty(pn, "trustlevel","0"));
    if (value>highestTrust)
      highestTrust=value;
    pn=GWEN_XMLNode_GetParent(pn);
  } /* while */
  return highestTrust;
}



const char *GWEN_MsgEngine__SearchForValue(GWEN_MSGENGINE *e,
                                           GWEN_XMLNODE *node,
                                           GWEN_XMLNODE_PATH *nodePath,
                                           const char *name,
                                           unsigned int *datasize) {
  const char *pvalue;
  GWEN_XMLNODE *pn;
  char *bufferPtr;
  int topDown;
  const char *lastValue;
  unsigned int lastDataSize;
  unsigned int ldatasize;

  DBG_DEBUG(GWEN_LOGDOMAIN, "Searching for value of \"%s\" in <VALUES>",
            name);
  if (!node) {
    DBG_WARN(GWEN_LOGDOMAIN, "No node !");
  }
  topDown=atoi(GWEN_XMLNode_GetProperty(node, "topdown", "0"));
  lastValue=0;
  lastDataSize=0;

  bufferPtr=0;

  /*pn=GWEN_XMLNode_GetParent(node);*/
  pn=GWEN_XMLNode_Path_Surface(nodePath);
  while(pn) {
    const char *ppath;
    /*
    if (GWEN_XMLNode_GetType(pn)==GWEN_XMLNodeTypeTag) {
      DBG_NOTICE(GWEN_LOGDOMAIN, "Checking node \"%s\"",
                 GWEN_XMLNode_GetData(pn));
                 }*/
    pvalue=GWEN_MsgEngine__findInValues(e, pn, node, name, &ldatasize);
    if (pvalue) {
      if (!topDown) {
        free(bufferPtr);
        *datasize=ldatasize;
        return pvalue;
      }
      DBG_DEBUG(GWEN_LOGDOMAIN, "Found a value, but will look further");
      lastValue=pvalue;
      lastDataSize=ldatasize;
    }

    ppath=GWEN_XMLNode_GetProperty(pn, "name", "");

    if (*ppath) {
      int i;
      char *tmpptr;

      if (bufferPtr) {
        i=strlen(bufferPtr)+strlen(ppath)+2;
        tmpptr=(char*)malloc(i);
        assert(tmpptr);
        sprintf(tmpptr, "%s/%s", ppath, bufferPtr);
        free(bufferPtr);
        bufferPtr=tmpptr;
      }
      else {
        i=strlen(ppath)+strlen(name)+2;
        tmpptr=(char*)malloc(i);
        assert(tmpptr);
        sprintf(tmpptr, "%s/%s", ppath, name);
        bufferPtr=tmpptr;
      }
      name=bufferPtr;
    }
    pn=GWEN_XMLNode_Path_Surface(nodePath);
  } /* while */

  free(bufferPtr);
  if (!lastValue)
    *datasize=0;
  else
    *datasize=lastDataSize;
  return lastValue;
}



const char *GWEN_MsgEngine__findInValues(GWEN_MSGENGINE *e,
                                         GWEN_XMLNODE *node,
                                         GWEN_XMLNODE *dnode,
                                         const char *name,
                                         unsigned int *datasize) {
  GWEN_XMLNODE *pn;

  DBG_VERBOUS(GWEN_LOGDOMAIN, "Looking for value of \"%s\" in <VALUES>", name);
  pn=GWEN_XMLNode_GetChild(node);

  while(pn) {
    if (GWEN_XMLNode_GetType(pn)==GWEN_XMLNodeTypeTag) {
      GWEN_XMLNODE *n;
      const char *p;

      p=GWEN_XMLNode_GetData(pn);
      assert(p);
      DBG_DEBUG(GWEN_LOGDOMAIN, "Checking %s", p);
      if (strcasecmp(p, "VALUES")==0) {
	DBG_DEBUG(GWEN_LOGDOMAIN, "<values> found");
	/* <preset> found, check all values */
	n=GWEN_XMLNode_GetChild(pn);
	while(n) {
	  if (GWEN_XMLNode_GetType(n)==GWEN_XMLNodeTypeTag) {
	    const char *p;

	    p=GWEN_XMLNode_GetData(n);
	    assert(p);
	    if (strcasecmp(p, "VALUE")==0) {
	      const char *pname;
	      const char *pvalue;

	      pname=GWEN_XMLNode_GetProperty(n, "path", 0);
	      if (pname) {
                DBG_DEBUG(GWEN_LOGDOMAIN, "Comparing against \"%s\"", pname);
		if (strcasecmp(name, pname)==0) {
		  GWEN_XMLNODE *dn;

		  dn=GWEN_XMLNode_GetChild(n);
		  while (dn) {
                    if (GWEN_XMLNode_GetType(dn)==GWEN_XMLNodeTypeData) {
                      pvalue=GWEN_XMLNode_GetData(dn);
                      if (pvalue) {
                        DBG_DEBUG(GWEN_LOGDOMAIN, "Transforming \"%s\"", pvalue);
                        pvalue=GWEN_MsgEngine__TransformValue(e,
                                                              pvalue,
                                                              node,
                                                              dnode,
                                                              datasize);
                      }
                      if (pvalue)
			return pvalue;
		    }
		    dn=GWEN_XMLNode_Next(dn);
		  } /* while dn */
		} /* if path matches name */
	      } /* if path given */
	    } /* if VALUE tag */
	  } /* if TAG */
	  n=GWEN_XMLNode_Next(n);
	} /* while */
        break;           /*  REMOVE this to check multiple groups */
      } /* if <preset> found */
    } /* if tag */
    pn=GWEN_XMLNode_Next(pn);
  } /* while node */

  DBG_DEBUG(GWEN_LOGDOMAIN, "No value found for \"%s\" in <VALUES>", name);
  return 0;
}



GWEN_XMLNODE *GWEN_MsgEngine__GetGroup(GWEN_MSGENGINE *e,
                                       GWEN_XMLNODE *node,
                                       const char *t,
                                       int version,
                                       const char *pvalue) {
  GWEN_XMLNODE *n;
  const char *p;
  int i;
  const char *mode;
  unsigned int proto;
  char buffer[256];

  if ((strlen(t)+4)>sizeof(buffer)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Type name too long.");
    return 0;
  }

  mode=GWEN_MsgEngine_GetMode(e);
  proto=GWEN_MsgEngine_GetProtocolVersion(e);

  /* find type+"S" */
  strcpy(buffer, t);
  strcat(buffer,"S");
  n=GWEN_XMLNode_FindFirstTag(node, buffer, 0, 0);
  if (!n) {
    DBG_DEBUG(GWEN_LOGDOMAIN,
	      "No definitions here for type \"%s\"", t);
    return 0;
  }

  /* find approppriate group definition */
  if (!mode)
    mode="";
  n=GWEN_XMLNode_GetFirstTag(n);
  if (!n) {
    DBG_INFO(GWEN_LOGDOMAIN, "No definitions inside \"%s\"", buffer);
    return 0;
  }

  /* find type+"def" */
  strcpy(buffer, t);
  strcat(buffer, "def");
  while(n) {
    p=GWEN_XMLNode_GetData(n);
    assert(p);
    if (strcasecmp(p, buffer)==0 ||
	strcasecmp(p, t)==0) {
      p=GWEN_XMLNode_GetProperty(n, "id", "");
      if (strcasecmp(p, pvalue)!=0)
	p=GWEN_XMLNode_GetProperty(n, "name", "");
      if (strcasecmp(p, pvalue)==0) {
	i=atoi(GWEN_XMLNode_GetProperty(n, "pversion" ,"0"));
	if (proto==0 || (int)proto==i || i==0) {
	  i=atoi(GWEN_XMLNode_GetProperty(n, "version" ,"0"));
	  if (version==0 || version==i) {
	    p=GWEN_XMLNode_GetProperty(n, "mode","");
	    if (strcasecmp(p, mode)==0 || !*p) {
	      DBG_DEBUG(GWEN_LOGDOMAIN,
			"Group definition for \"%s=%s\" found",
			t, pvalue);
	      return n;
	    }
	  }
	}
      }
    }
    n=GWEN_XMLNode_GetNextTag(n);
  } /* while */

  DBG_DEBUG(GWEN_LOGDOMAIN,
	    "Group definition for \"%s=%s\"(%d) not found here",
	    t,
	    pvalue,
	    version);
  return 0;
}



GWEN_XMLNODE *GWEN_MsgEngine_GetGroup(GWEN_MSGENGINE *e,
				      GWEN_XMLNODE *node,
				      const GWEN_XMLNODE_PATH *nodePath,
				      const char *t,
				      int version,
				      const char *pvalue) {
  GWEN_XMLNODE *n;
  GWEN_XMLNODE *nLast = 0;
  GWEN_XMLNODE *nRes = 0;
  GWEN_XMLNODE_PATH *pathCopy;

  assert(node);
  assert(nodePath);
  assert(t);
  assert(pvalue);

  pathCopy=GWEN_XMLNode_Path_dup(nodePath);
  n=GWEN_XMLNode_Path_Surface(pathCopy);
  /* first try all nodes along the path */
  while(n) {
    nLast=n;
    nRes=GWEN_MsgEngine__GetGroup(e, n, t, version, pvalue);
    if (nRes)
      break;
    n=GWEN_XMLNode_Path_Surface(pathCopy);
  }
  GWEN_XMLNode_Path_free(pathCopy);
  if (nRes) {
    /* already found */
    if (nRes==node) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Loop detected.");
      return 0;
    }
    return nRes;
  }

  if (nLast)
    n=nLast;
  else
    n=node;

  if (n) {
    n=GWEN_XMLNode_GetParent(n);
    while(n) {
      nRes=GWEN_MsgEngine__GetGroup(e, n, t, version, pvalue);
      if (nRes)
	break;
      n=GWEN_XMLNode_GetParent(n);
    }
  }

  /* try root as a last resort */
  if (!nRes && e->defs)
    nRes=GWEN_MsgEngine__GetGroup(e, e->defs, t, version, pvalue);

  if (!nRes) {
    DBG_DEBUG(GWEN_LOGDOMAIN,
	      "Group definition for \"%s=%s\"(%d) not found",
	      t,
	      pvalue,
	      version);
    return 0;
  }
  if (nRes==node) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Loop detected.");
    return 0;
  }
  return nRes;
}



int GWEN_MsgEngine__WriteGroup(GWEN_MSGENGINE *e,
                               GWEN_BUFFER *gbuf,
                               GWEN_XMLNODE *node,
                               GWEN_XMLNODE *rnode,
                               GWEN_DB_NODE *gr,
                               int groupIsOptional,
                               GWEN_XMLNODE_PATH *nodePath) {
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
    p=GWEN_XMLNode_GetProperty(node, "terminator","");
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
  while(n) {
    int t;
    unsigned int minnum;
    unsigned int maxnum;
    int gversion;
    const char *addEmptyMode;
    unsigned int loopNr;

    minnum=atoi(GWEN_XMLNode_GetProperty(n, "minnum","1"));
    maxnum=atoi(GWEN_XMLNode_GetProperty(n, "maxnum","1"));
    gversion=atoi(GWEN_XMLNode_GetProperty(n, "version","0"));
    addEmptyMode=GWEN_XMLNode_GetProperty(n, "addemptymode","one");

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

          /*
           Replaced the following code because it did not work correctly
           with segment HKDAN... (aquamaniac, 2005/07/09)

          if (!isFirstElement && delimiter) {
            DBG_VERBOUS(GWEN_LOGDOMAIN, "Appending %d delimiters", omittedElements+1);
            for (j=0; j<omittedElements+1; j++) {
              if (GWEN_Buffer_AppendByte(gbuf, delimiter)) {
                DBG_INFO(GWEN_LOGDOMAIN, "called from here");
		return -1;
              }
            }
          }*/

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
                                          rnode,
                                          gr,
                                          loopNr,
                                          loopNr>=minnum ||
                                          (groupIsOptional && !hasEntries),
                                          nodePath);
          if (rv==-1) {
            DBG_INFO(GWEN_LOGDOMAIN, "Error writing element");
            DBG_INFO(GWEN_LOGDOMAIN, "Node is:");
            GWEN_XMLNode_Dump(n, stderr, 1);
            if (gr) {
              DBG_INFO(GWEN_LOGDOMAIN, "Data is:");
              GWEN_DB_Dump(gr, stderr, 1);
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
	gtype=GWEN_XMLNode_GetProperty(n, "type",0);
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

        gname=0;
        gcfg=0;
        if (gr) {
          gname=GWEN_XMLNode_GetProperty(n, "name",0);
          if (gname) {
            DBG_VERBOUS(GWEN_LOGDOMAIN, "Group \"%s\" using special data", gname);
            gcfg=GWEN_DB_GetFirstGroup(gr);
          }
          else {
            DBG_DEBUG(GWEN_LOGDOMAIN, "Unnamed group, using basic data");
            gcfg=gr;
          }
        }

	/* write group as often as needed */
	for (loopNr=0; loopNr<maxnum; loopNr++) {
          int rv;
          int groupIsEmpty;

          groupIsEmpty=0;
          posBeforeGroup=GWEN_Buffer_GetPos(gbuf);

          /* find next matching group */
          if (gname) {
            DBG_DEBUG(GWEN_LOGDOMAIN, "Finding next group named \"%s\"", gname);
            while(gcfg) {
              if (strcasecmp(GWEN_DB_GroupName(gcfg), gname)==0)
                break;
              gcfg=GWEN_DB_GetNextGroup(gcfg);
              if (gcfg==0) {
                DBG_DEBUG(GWEN_LOGDOMAIN, "No group found");
                if (loopNr>=minnum)
                  groupIsEmpty=1;
              }
            } /* while */
          }

          if (!groupIsEmpty) {
            int dive;

            /* write delimiter, if needed */
            if (!isFirstElement && delimiter) {
              int j;

              DBG_VERBOUS(GWEN_LOGDOMAIN, "Appending %d delimiters", omittedElements+1);
              for (j=0; j<omittedElements+1; j++) {
                if (GWEN_Buffer_AppendByte(gbuf, delimiter)) {
                  return -1;
                }
              }
              omittedElements=0;
            }
            else
              isFirstElement=0;

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
            if (rv==-1){
              DBG_INFO(GWEN_LOGDOMAIN, "Could not write group \"%s\"", gtype);
              if (gn) {
                DBG_INFO(GWEN_LOGDOMAIN, "Node is:");
                GWEN_XMLNode_Dump(gn, stderr, 1);
              }
              if (n) {
                DBG_INFO(GWEN_LOGDOMAIN, "Referring node is:");
                GWEN_XMLNode_Dump(n, stderr, 1);
              }
              if (gr) {
                DBG_INFO(GWEN_LOGDOMAIN, "Data is:");
                GWEN_DB_Dump(gr, stderr, 1);
              }
              return -1;
            }
            else if (rv==0) {
              hasEntries=1;
            }
            else
              groupIsEmpty=1;
          }

          if (groupIsEmpty) {
            DBG_DEBUG(GWEN_LOGDOMAIN, "Empty Group");
            GWEN_Buffer_SetPos(gbuf, posBeforeGroup);
            GWEN_Buffer_Crop(gbuf, 0, posBeforeGroup);

            if (loopNr>=minnum) {
              DBG_DEBUG(GWEN_LOGDOMAIN, "No data for group \"%s[%d]\", omitting",
                        gname, loopNr);
	      if (strcasecmp(addEmptyMode, "max")==0) {
                DBG_VERBOUS(GWEN_LOGDOMAIN, "Adding max empty");
                omittedElements+=(maxnum-loopNr);
	      }
	      else if (strcasecmp(addEmptyMode, "min")==0) {
                DBG_VERBOUS(GWEN_LOGDOMAIN, "Adding min empty");
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
	    else {
	      DBG_ERROR(GWEN_LOGDOMAIN, "No data for group \"%s[%d]\"",
			gname, loopNr);
	      return -1;
	    }
          } /* if empty group */
          /* use next group next time if any */
          if (gcfg)
            gcfg=GWEN_DB_GetNextGroup(gcfg);
        } /* for */
      }
    }
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
                                         GWEN_DB_NODE *msgData){
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
  if (rv){
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
                                 GWEN_DB_NODE *msgData) {
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



int GWEN_MsgEngine_AddDefinitions(GWEN_MSGENGINE *e,
                                  GWEN_XMLNODE *node) {
  GWEN_XMLNODE *nsrc, *ndst;

  assert(e);
  assert(node);

  if (!e->defs) {
    e->defs=GWEN_XMLNode_dup(node);
    e->ownDefs=1;
    return 0;
  }

  nsrc=GWEN_XMLNode_GetChild(node);
  while(nsrc) {
    if (GWEN_XMLNode_GetType(nsrc)==GWEN_XMLNodeTypeTag) {
      ndst=GWEN_XMLNode_FindNode(e->defs, GWEN_XMLNodeTypeTag,
                                 GWEN_XMLNode_GetData(nsrc));
      if (ndst) {
	GWEN_XMLNODE *n;

        n=GWEN_XMLNode_GetChild(nsrc);
	while (n) {
	  GWEN_XMLNODE *newNode;

          DBG_DEBUG(GWEN_LOGDOMAIN, "Adding node \"%s\"", GWEN_XMLNode_GetData(n));
          newNode=GWEN_XMLNode_dup(n);
          GWEN_XMLNode_AddChild(ndst, newNode);
	  n=GWEN_XMLNode_Next(n);
	} /* while n */
      }
      else {
	GWEN_XMLNODE *newNode;

        DBG_DEBUG(GWEN_LOGDOMAIN, "Adding branch \"%s\"", GWEN_XMLNode_GetData(nsrc));
	newNode=GWEN_XMLNode_dup(nsrc);
        GWEN_XMLNode_AddChild(e->defs, newNode);
      }
    } /* if TAG */
    nsrc=GWEN_XMLNode_Next(nsrc);
  } /* while */

  return 0;
}



int GWEN_MsgEngine__ShowElement(GWEN_MSGENGINE *e,
                                const char *path,
                                GWEN_XMLNODE *node,
                                GWEN_STRINGLIST *sl,
                                uint32_t flags) {
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
  type=GWEN_XMLNode_GetProperty(node, "type","ASCII");

  /* get some sizes */
  minsize=atoi(GWEN_XMLNode_GetProperty(node, "minsize","0"));
  maxsize=atoi(GWEN_XMLNode_GetProperty(node, "maxsize","0"));
  minnum=atoi(GWEN_XMLNode_GetProperty(node, "minnum","1"));
  maxnum=atoi(GWEN_XMLNode_GetProperty(node, "maxnum","1"));

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
  while(en) {
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
  fprintf(stdout," | %3d ", maxnum);
  fprintf(stdout," |");
  if (minnum==0)
    fprintf(stdout," optvar");
  if (flags & GWEN_MSGENGINE_SHOW_FLAGS_OPTIONAL)
    fprintf(stdout," optgrp");

  if (isSet) {
    fprintf(stdout," set");
  }

  fprintf(stdout,"\n");

  return 0;
}



int GWEN_MsgEngine__ShowGroup(GWEN_MSGENGINE *e,
                              const char *path,
                              GWEN_XMLNODE *node,
                              GWEN_XMLNODE *rnode,
                              GWEN_STRINGLIST *sl,
                              uint32_t flags) {
  GWEN_XMLNODE *n;
  int isFirstElement;
  int omittedElements;
  int rv;

  /* setup data */
  n=GWEN_XMLNode_GetChild(node);

  if (path==0)
    path="";
  if (*path=='/')
    path++;

  while(n) {
    if (GWEN_XMLNode_GetType(n)==GWEN_XMLNodeTypeTag) {
      const char *p;

      p=GWEN_XMLNode_GetData(n);
      assert(p);
      DBG_DEBUG(GWEN_LOGDOMAIN, "Checking %s",p);
      if (strcasecmp(p, "VALUES")==0)
	break;
    } /* if tag */
    n=GWEN_XMLNode_Next(n);
  } /* while */

  if (n) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "<preset> found");
    /* <preset> found, handle all values */
    n=GWEN_XMLNode_GetChild(n);
    while(n) {
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
  isFirstElement=1;
  omittedElements=0;
  while(n) {
    int t;
    unsigned int minnum;
    unsigned int maxnum;
    int gversion;
    const char *addEmptyMode;
    unsigned int loopNr;
    unsigned int lflags;

    minnum=atoi(GWEN_XMLNode_GetProperty(n, "minnum","1"));
    maxnum=atoi(GWEN_XMLNode_GetProperty(n, "maxnum","1"));
    gversion=atoi(GWEN_XMLNode_GetProperty(n, "version","0"));
    addEmptyMode=GWEN_XMLNode_GetProperty(n, "addemptymode","one");

    lflags=flags;

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

	/* write element as often as needed */
	rv=GWEN_MsgEngine__ShowElement(e,
                                       path,
                                       n,
                                       sl,
                                       lflags);
        if (rv==-1)
	  return -1;
	else {
	  isFirstElement=0;
	  omittedElements=0;
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

	gtype=GWEN_XMLNode_GetProperty(n, "type",0);
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
	  gname=GWEN_XMLNode_GetProperty(n, "name",0);
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
	  if (GWEN_MsgEngine__ShowGroup(e,
                                        npath,
                                        gn,
                                        n,
                                        sl,
                                        lflags)) {
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
                               uint32_t flags) {
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
    fprintf(stdout," ");

  fprintf(stdout,"  |");
  p=" Type";
  fprintf(stdout, "%s", p);
  i=GWEN_MSGENGINE_TYPENAME_WIDTH-strlen(p);
  for (j=0; j<i; j++)
    fprintf(stdout," ");

  fprintf(stdout," |   Size    | Num  | Flags\n");
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
                                0,
                                sl,
                                flags)) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error showing group \"%s\"", msgName);
    GWEN_StringList_free(sl);
    return -1;
  }

  GWEN_StringList_free(sl);

  return 0;
}



int GWEN_MsgEngine__ListElement(GWEN_MSGENGINE *e,
                                const char *path,
                                GWEN_XMLNODE *node,
                                GWEN_STRINGLIST *sl,
                                GWEN_XMLNODE *listNode,
                                uint32_t flags) {
  const char *name;
  const char *type;
  const char *npath;
  int isSet;
  char nbuffer[256];
  GWEN_STRINGLISTENTRY *en;
  GWEN_XMLNODE *nn;

  /* get type */
  type=GWEN_XMLNode_GetProperty(node, "type","ASCII");

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
  while(en) {
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
                              GWEN_XMLNODE *rnode,
                              GWEN_STRINGLIST *sl,
                              GWEN_XMLNODE *listNode,
                              uint32_t flags) {
  GWEN_XMLNODE *n;
  int rv;

  /* setup data */
  n=GWEN_XMLNode_GetChild(node);

  if (path==0)
    path="";
  if (*path=='/')
    path++;

  while(n) {
    if (GWEN_XMLNode_GetType(n)==GWEN_XMLNodeTypeTag) {
      const char *p;

      p=GWEN_XMLNode_GetData(n);
      assert(p);
      DBG_DEBUG(GWEN_LOGDOMAIN, "Checking %s",p);
      if (strcasecmp(p, "VALUES")==0)
	break;
    } /* if tag */
    n=GWEN_XMLNode_Next(n);
  } /* while */

  if (n) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "<values> found");
    /* <values> found, handle all values */
    n=GWEN_XMLNode_GetChild(n);
    while(n) {
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
  while(n) {
    int t;
    int gversion;
    unsigned int lflags;

    gversion=atoi(GWEN_XMLNode_GetProperty(n, "version","0"));
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

        gtype=GWEN_XMLNode_GetProperty(n, "type",0);
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
        gname=GWEN_XMLNode_GetProperty(n, "name",0);
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
        if (GWEN_MsgEngine__ListGroup(e,
                                      npath,
                                      gn,
                                      n,
                                      sl,
                                      nn,
                                      lflags)) {
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
                                         uint32_t flags) {
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

  if (GWEN_MsgEngine__ListGroup(e,
                                "",
                                group,
                                0,
                                sl,
                                listNode,
                                flags)) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error showing group \"%s\"", msgName);
    GWEN_StringList_free(sl);
    GWEN_XMLNode_free(listNode);
    return 0;
  }

  GWEN_StringList_free(sl);

  return listNode;
}







int GWEN_MsgEngine__ReadValue(GWEN_MSGENGINE *e,
			      GWEN_BUFFER *msgbuf,
                              GWEN_XMLNODE *node,
                              GWEN_XMLNODE *rnode,
			      GWEN_BUFFER *vbuf,
                              const char *delimiters,
                              uint32_t flags) {
  unsigned int minsize;
  unsigned int maxsize;
  unsigned int size;
  unsigned int minnum;
  GWEN_MSGENGINE_TRUSTLEVEL trustLevel;
  unsigned int posInMsg;
  const char *type;
  int rv;
  unsigned int realSize;

  /* get some sizes */
  posInMsg=GWEN_Buffer_GetPos(msgbuf);
  realSize=0;
  size=atoi(GWEN_XMLNode_GetProperty(node, "size","0"));
  minsize=atoi(GWEN_XMLNode_GetProperty(node, "minsize","0"));
  maxsize=atoi(GWEN_XMLNode_GetProperty(node, "maxsize","0"));
  minnum=atoi(GWEN_XMLNode_GetProperty(node, "minnum","1"));
  type=GWEN_XMLNode_GetProperty(node, "type","ASCII");

  rv=1;
  if (e->typeReadPtr) {
    rv=e->typeReadPtr(e,
		      msgbuf,
		      node,
		      vbuf,
		      e->escapeChar,
		      delimiters);
  }
  if (rv==-1) {
    DBG_INFO(GWEN_LOGDOMAIN, "External type reading failed on type \"%s\"", type);
    return -1;
  }
  else if (rv==1) {
    if (strcasecmp(type, "bin")==0) {
      if (GWEN_Buffer_GetBytesLeft(msgbuf)==0) {
	DBG_ERROR(GWEN_LOGDOMAIN, "Premature end of message (@num@ expected)");
	return -1;
      }
      else {
	char lbuffer[16];
	int c;
	char *p;
        int l;

	p=lbuffer;
	c=GWEN_Buffer_ReadByte(msgbuf);
	if (c!='@') {
	  DBG_ERROR(GWEN_LOGDOMAIN, "\"@num@\" expected");
	  return -1;
	}

        c=0;
	while(GWEN_Buffer_GetBytesLeft(msgbuf)>0) {
	  c=GWEN_Buffer_ReadByte(msgbuf);
	  if (c==-1) {
	    DBG_ERROR(GWEN_LOGDOMAIN, "\"@\" expected");
	    return -1;
	  }
	  if (c=='@')
            break;
	  *p=(char)c;
	  p++;
	} /* while */
        *p=0;
	if (c!='@') {
	  DBG_ERROR(GWEN_LOGDOMAIN, "\"@num@\" expected");
	  return -1;
	}
	if (sscanf(lbuffer, "%d", &l)!=1) {
	  DBG_ERROR(GWEN_LOGDOMAIN, "Bad number format");
	  return -1;
	}
	DBG_DEBUG(GWEN_LOGDOMAIN, "Reading binary: %d bytes from pos %d (msgsize=%d)",
		 l,
		 GWEN_Buffer_GetPos(msgbuf),
		 GWEN_Buffer_GetUsedBytes(msgbuf));
	if (GWEN_Buffer_GetBytesLeft(msgbuf) < (unsigned) l) {
	  DBG_ERROR(GWEN_LOGDOMAIN, "Premature end of message (binary beyond end)");
	  return -1;
	}
	if (GWEN_Buffer_AppendBytes(vbuf,
				    GWEN_Buffer_GetPosPointer(msgbuf),
				    l)) {
	  DBG_DEBUG(GWEN_LOGDOMAIN, "Called from here");
	  return -1;
	}
	GWEN_Buffer_IncrementPos(msgbuf,l);
      }
    } /* if bin */
    else {
      /* type is not bin */
      int lastWasEscape;
      int isEscaped;
      int br;

      isEscaped=0;
      lastWasEscape=0;

      br=0;
      while(GWEN_Buffer_GetBytesLeft(msgbuf) &&
	    (size==0 || br<size)) {
	int c;

	c=GWEN_Buffer_ReadByte(msgbuf);
	if (lastWasEscape) {
	  lastWasEscape=0;
	  isEscaped=1;
	}
	else {
	  isEscaped=0;
	  if (c==e->escapeChar) {
	    lastWasEscape=1;
	    c=-1;
	  }
	}
	if (c!=-1) {
          if (!isEscaped && (c && strchr(delimiters, c)!=0)) {
	    /* delimiter found, step back */
	    GWEN_Buffer_DecrementPos(msgbuf,1);
	    break;
	  }
	  else {
	    if (c=='\\' || iscntrl(c)) {
	      DBG_WARN(GWEN_LOGDOMAIN,
		       "Found a bad character (%02x) in type \"%s\", "
		       "converting to SPACE",
		       (unsigned int)c,
		       type);
	      c=' ';
	    }
	    if (GWEN_Buffer_AppendByte(vbuf, c)) {
	      DBG_DEBUG(GWEN_LOGDOMAIN, "Called from here");
	      return -1;
	    }
            br++;
	  }
	}
      } /* while */
    } /* if !bin */
  } /* if type not external */
  else {
    DBG_DEBUG(GWEN_LOGDOMAIN, "Type \"%s\" is external (read)", type);
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
      GWEN_XMLNode_Dump(node, stderr, 1);
      return -1;
    }
  }

  /* check minimum size */
  if (minsize!=0 && realSize<minsize) {
    DBG_INFO(GWEN_LOGDOMAIN, "Value too short (%d<%d).",
             realSize,
             minsize);
    return -1;
  }

  /* check maximum size */
  if (maxsize!=0 && realSize>maxsize) {
    DBG_INFO(GWEN_LOGDOMAIN, "Value too long (%d>%d).",
             realSize, maxsize);
    return -1;
  }

  if (flags & GWEN_MSGENGINE_READ_FLAGS_TRUSTINFO) {
    /* add trust data to msgEngine */
    const char *descr;

    trustLevel=GWEN_MsgEngine_GetHighestTrustLevel(node, rnode);
    if (trustLevel) {
      unsigned int ustart;

      ustart=GWEN_Buffer_GetPos(msgbuf)-realSize;
      descr=GWEN_XMLNode_GetProperty(node, "name",0);
      if (GWEN_MsgEngine_AddTrustInfo(e,
                                      GWEN_Buffer_GetStart(vbuf),
                                      realSize,
                                      descr,
                                      trustLevel,
                                      ustart)) {
        DBG_INFO(GWEN_LOGDOMAIN, "called from here");
        return -1;
      }
    }
  }

  return 0;
}



int GWEN_MsgEngine__ReadGroup(GWEN_MSGENGINE *e,
			      GWEN_BUFFER *msgbuf,
                              GWEN_XMLNODE *node,
                              GWEN_XMLNODE *rnode,
                              GWEN_DB_NODE *gr,
                              const char *delimiters,
                              uint32_t flags) {
  unsigned int minsize;
  unsigned int maxsize;
  unsigned int minnum;
  unsigned int maxnum;
  const char *name;
  const char *p;
  char delimiter;
  char terminator;
  GWEN_XMLNODE *n;
  int abortLoop;
  GWEN_BUFFER *delimBuffer=0;

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
    p=GWEN_XMLNode_GetProperty(node, "terminator","");
    terminator=*p;
  }

  delimBuffer=GWEN_Buffer_new(0, strlen(delimiters)+2, 0, 1);
  GWEN_Buffer_AppendString(delimBuffer, delimiters);
  if (delimiter)
    GWEN_Buffer_AppendByte(delimBuffer, delimiter);
  if (terminator)
    GWEN_Buffer_AppendByte(delimBuffer, terminator);

  DBG_DEBUG(GWEN_LOGDOMAIN, "Delimiters are \"%s\" and \"%c\"",
	    delimiters, delimiter);

  n=GWEN_XMLNode_GetChild(node);
  while (n) {
    if (GWEN_XMLNode_GetType(n)==GWEN_XMLNodeTypeTag) {
      const char *type;

      if (GWEN_Buffer_GetBytesLeft(msgbuf)==0)
	break;

      type=GWEN_XMLNode_GetData(n);

      /*
      DBG_NOTICE(GWEN_LOGDOMAIN, "Reading group from here :");
      GWEN_Text_DumpString(GWEN_Buffer_GetStart(msgbuf)+
                           GWEN_Buffer_GetPos(msgbuf),
                           GWEN_Buffer_GetUsedBytes(msgbuf)-
                           GWEN_Buffer_GetPos(msgbuf),
                           stderr, 3);
      */
      if (strcasecmp(type, "ELEM")==0) {
	unsigned int loopNr;

	/* get some sizes */
	minsize=atoi(GWEN_XMLNode_GetProperty(n, "minsize","0"));
	maxsize=atoi(GWEN_XMLNode_GetProperty(n, "maxsize","0"));
	minnum=atoi(GWEN_XMLNode_GetProperty(n, "minnum","1"));
	maxnum=atoi(GWEN_XMLNode_GetProperty(n, "maxnum","1"));
	name=GWEN_XMLNode_GetProperty(n, "name", 0);

	loopNr=0;
	abortLoop=0;
	while((maxnum==0 || loopNr<maxnum) && !abortLoop) {
          int c;

	  DBG_VERBOUS(GWEN_LOGDOMAIN, "Reading %s", name);
          if (GWEN_Buffer_GetBytesLeft(msgbuf)==0)
            break;
	  c=GWEN_Buffer_PeekByte(msgbuf);
	  if (c==-1) {
	    DBG_DEBUG(GWEN_LOGDOMAIN, "called from here");
	    GWEN_Buffer_free(delimBuffer);
	    return -1;
	  }

          DBG_VERBOUS(GWEN_LOGDOMAIN,
		      "Checking delimiter at pos %x "
		      "(whether \"%c\" is in \"%s\")",
		      GWEN_Buffer_GetPos(msgbuf),
		      c, GWEN_Buffer_GetStart(delimBuffer));
	  if (c && strchr(GWEN_Buffer_GetStart(delimBuffer), c)) {
	    abortLoop=1;
	    DBG_VERBOUS(GWEN_LOGDOMAIN,
			"Found delimiter (\"%c\" is in \"%s\")",
			c, GWEN_Buffer_GetStart(delimBuffer));
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

	      DBG_VERBOUS(GWEN_LOGDOMAIN, "Reading value from pos %x",
			  GWEN_Buffer_GetPos(msgbuf));
	      vbuf=GWEN_Buffer_new(0,
				   GWEN_MSGENGINE_MAX_VALUE_LEN,
				   0,0);
              /*
	      DBG_ERROR(GWEN_LOGDOMAIN, "Reading value from here:\n");
	      GWEN_Text_DumpString(GWEN_Buffer_GetPosPointer(msgbuf),
				   GWEN_Buffer_GetBytesLeft(msgbuf),
				   stderr, 1);*/

	      rv=GWEN_MsgEngine__ReadValue(e,
					   msgbuf,
					   n,
					   rnode,
					   vbuf,
                                           GWEN_Buffer_GetStart(delimBuffer),
					   //":+'",
					   flags);
	      if (rv==1) {
		DBG_INFO(GWEN_LOGDOMAIN, "Empty value");
	      }
	      else if (rv==-1) {
                DBG_INFO(GWEN_LOGDOMAIN, "Error parsing node \"%s\" (%s)",
                         name,
                         type);
		GWEN_Buffer_free(vbuf);
		GWEN_Buffer_free(delimBuffer);
		return -1;
              }

              GWEN_Buffer_Rewind(vbuf);

              /* special handling for binary data */
              dtype=GWEN_XMLNode_GetProperty(n, "type", "");
              if (GWEN_MsgEngine__IsBinTyp(e, dtype)) {
		if (atoi(GWEN_XMLNode_GetProperty(n, "readbin", "1")) &&
                    e->binTypeReadPtr) {
                  rv=e->binTypeReadPtr(e, n, gr, vbuf);
                }
		else
                  rv=1;
                if (rv==-1) {
                  DBG_INFO(GWEN_LOGDOMAIN, "Called from here");
		  GWEN_Buffer_free(vbuf);
		  GWEN_Buffer_free(delimBuffer);
                  return -1;
                }
                else if (rv==1) {
                  /* bin type not handled, so handle it myself */
                  if (GWEN_DB_SetBinValue(gr,
                                          GWEN_DB_FLAGS_DEFAULT,
					  name,
					  GWEN_Buffer_GetStart(vbuf),
					  GWEN_Buffer_GetUsedBytes(vbuf))) {
		    DBG_INFO(GWEN_LOGDOMAIN, "Could not set value for \"%s\"", name);
		    GWEN_Buffer_free(vbuf);
		    GWEN_Buffer_free(delimBuffer);
		    return -1;
		  }
		}
              } /* if type is bin */
              else if (GWEN_MsgEngine__IsIntTyp(e, dtype)) {
                int z;

		if (1!=sscanf(GWEN_Buffer_GetStart(vbuf), "%d", &z)) {
		  DBG_INFO(GWEN_LOGDOMAIN, "Value for \"%s\" is not an integer",
			   name);
		  GWEN_Buffer_free(delimBuffer);
		  return -1;
		}
		if (GWEN_DB_SetIntValue(gr,
                                        GWEN_DB_FLAGS_DEFAULT,
					name, z)) {
                  DBG_INFO(GWEN_LOGDOMAIN, "Could not set int value for \"%s\"", name);
		  GWEN_Buffer_free(delimBuffer);
                  return -1;
                }
              } /* if type is int */
              else {
		DBG_DEBUG(GWEN_LOGDOMAIN, "Value is \"%s\"",
			  GWEN_Buffer_GetStart(vbuf));
                if (GWEN_DB_SetCharValue(gr,
					 GWEN_DB_FLAGS_DEFAULT,
					 name,
					 GWEN_Buffer_GetStart(vbuf))){
		  DBG_INFO(GWEN_LOGDOMAIN, "Could not set value for \"%s\"", name);
		  GWEN_Buffer_free(delimBuffer);
		  return -1;
		}
              } /* if !bin */

	      GWEN_Buffer_free(vbuf);
	    } /* if name is given */
          } /* if current char is not a delimiter */

	  if (GWEN_Buffer_GetBytesLeft(msgbuf)) {
            if (delimiter) {
	      if (GWEN_Buffer_PeekByte(msgbuf)==delimiter) {
		GWEN_Buffer_IncrementPos(msgbuf,1);
	      }
	    }
	  }
	  loopNr++;
	} /* while */
	if (loopNr<minnum) {
          DBG_ERROR(GWEN_LOGDOMAIN, "Premature end of message (too few ELEM repeats)");
          GWEN_XMLNode_Dump(n, stderr, 2);
	  GWEN_Buffer_free(delimBuffer);
	  return -1;
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
	GWEN_XMLNODE *gn;
	GWEN_DB_NODE *gcfg;
	const char *gname;
	const char *gtype;
	unsigned int gversion;
	unsigned int loopNr;

	minnum=atoi(GWEN_XMLNode_GetProperty(n, "minnum","1"));
	maxnum=atoi(GWEN_XMLNode_GetProperty(n, "maxnum","1"));
	gversion=atoi(GWEN_XMLNode_GetProperty(n, "version","0"));
	gtype=GWEN_XMLNode_GetProperty(n, "type",0);
	if (!gtype) {
	  /* no "type" property, so use this group directly */
	  DBG_INFO(GWEN_LOGDOMAIN, "<%s> tag has no \"type\" property", type);
	  gtype="";
	  gn=n;
	}
	else {
          gn=GWEN_MsgEngine_FindNodeByProperty(e, type, "id",
                                               gversion, gtype);
	  if (!gn) {
	    DBG_INFO(GWEN_LOGDOMAIN, "Definition for type \"%s\" not found", type);
	    GWEN_Buffer_free(delimBuffer);
	    return -1;
	  }
	}

	/* get configuration */
	loopNr=0;
	abortLoop=0;
        while((maxnum==0 || loopNr<maxnum) && !abortLoop) {
          int c;

          DBG_DEBUG(GWEN_LOGDOMAIN, "Reading group type %s", gtype);
	  if (GWEN_Buffer_GetBytesLeft(msgbuf)==0)
            break;
          c=GWEN_Buffer_PeekByte(msgbuf);
	  if (c && strchr(GWEN_Buffer_GetStart(delimBuffer), c)) {
            abortLoop=1;
          }
          else {
	    gname=GWEN_XMLNode_GetProperty(n, "name",0);
	    if (gname) {
              DBG_DEBUG(GWEN_LOGDOMAIN, "Creating group \"%s\"", gname);
              gcfg=GWEN_DB_GetGroup(gr,
                                    GWEN_PATH_FLAGS_CREATE_GROUP,
                                    gname);
	      if (!gcfg) {
		DBG_ERROR(GWEN_LOGDOMAIN, "Could not select group \"%s\"",
			  gname);
		GWEN_Buffer_free(delimBuffer);
		return -1;
              }
              DBG_DEBUG(GWEN_LOGDOMAIN, "Created group \"%s\"", gname);
	    } /* if name given */
	    else
	      gcfg=gr;

	    /* read group */
	    DBG_DEBUG(GWEN_LOGDOMAIN, "Reading group \"%s\"", gname);
	    if (GWEN_MsgEngine__ReadGroup(e,
                                          msgbuf,
					  gn,
					  n,
					  gcfg,
					  GWEN_Buffer_GetStart(delimBuffer),
					  flags)) {
	      DBG_INFO(GWEN_LOGDOMAIN, "Could not read group \"%s\"", gtype);
	      GWEN_Buffer_free(delimBuffer);
	      return -1;
	    }
	  }
	  if (GWEN_Buffer_GetBytesLeft(msgbuf)) {
	    if (delimiter) {
	      if (GWEN_Buffer_PeekByte(msgbuf)==delimiter) {
		GWEN_Buffer_IncrementPos(msgbuf, 1);
	      }
	    }
	  }
	  loopNr++;
	} /* while */
	if (loopNr<minnum) {
	  DBG_ERROR(GWEN_LOGDOMAIN, "Premature end of message (too few group repeats)");
	  GWEN_Buffer_free(delimBuffer);
	  return -1;
	}
	n=GWEN_XMLNode_Next(n);
      } /* if GROUP */
    } /* if TAG */
    else {
      n=GWEN_XMLNode_Next(n);
    }
  } /* while */

  /* check whether there still are nodes which have not been read */
  while(n) {
    if (GWEN_XMLNode_GetType(n)==GWEN_XMLNodeTypeTag) {
      if (strcasecmp(GWEN_XMLNode_GetData(n), "ELEM")==0 ||
	  strcasecmp(GWEN_XMLNode_GetData(n), "GROUP")==0) {
	unsigned int i;

	i=atoi(GWEN_XMLNode_GetProperty(n, "minnum", "1"));
	if (i) {
          DBG_ERROR(GWEN_LOGDOMAIN, "Premature end of message (still tags to parse)");
          GWEN_XMLNode_Dump(n, stderr, 2);
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
	GWEN_XMLNode_Dump(node, stderr, 1);
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



int GWEN_MsgEngine_ParseMessage(GWEN_MSGENGINE *e,
				GWEN_XMLNODE *group,
                                GWEN_BUFFER *msgbuf,
                                GWEN_DB_NODE *msgData,
                                uint32_t flags){

  if (GWEN_MsgEngine__ReadGroup(e,
                                msgbuf,
                                group,
                                0,
                                msgData,
                                e->delimiters,
                                flags)) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error reading group");
    return -1;
  }

  return 0;
}



int GWEN_MsgEngine_SetValue(GWEN_MSGENGINE *e,
                            const char *path,
                            const char *value){
  GWEN_DB_NODE *globalValues;

  assert(e);
  globalValues=GWEN_MsgEngine__GetGlobalValues(e);
  assert(globalValues);
  return GWEN_DB_SetCharValue(globalValues,
                              GWEN_DB_FLAGS_DEFAULT |
                              GWEN_DB_FLAGS_OVERWRITE_VARS,
                              path, value);
}



int GWEN_MsgEngine_SetIntValue(GWEN_MSGENGINE *e,
                               const char *path,
                               int value){
  GWEN_DB_NODE *globalValues;

  assert(e);
  globalValues=GWEN_MsgEngine__GetGlobalValues(e);
  assert(globalValues);
  return GWEN_DB_SetIntValue(globalValues,
                             GWEN_DB_FLAGS_DEFAULT |
                             GWEN_DB_FLAGS_OVERWRITE_VARS,
                             path, value);
}



const char *GWEN_MsgEngine_GetValue(GWEN_MSGENGINE *e,
                                    const char *path,
                                    const char *defValue){
  GWEN_DB_NODE *globalValues;

  assert(e);
  globalValues=GWEN_MsgEngine__GetGlobalValues(e);
  assert(globalValues);
  return GWEN_DB_GetCharValue(globalValues,
                              path, 0, defValue);
}



int GWEN_MsgEngine_GetIntValue(GWEN_MSGENGINE *e,
                               const char *path,
                               int defValue){
  GWEN_DB_NODE *globalValues;

  assert(e);
  globalValues=GWEN_MsgEngine__GetGlobalValues(e);
  assert(globalValues);
  return GWEN_DB_GetIntValue(globalValues,
                             path, 0, defValue);
}



/* --------------------------------------------------------------- FUNCTION */
int GWEN_MsgEngine_SkipSegment(GWEN_MSGENGINE *e,
                               GWEN_BUFFER *msgbuf,
                               unsigned char escapeChar,
                               unsigned char delimiter) {
  int esc;

  esc=0;
  while(GWEN_Buffer_GetBytesLeft(msgbuf)) {
    if (esc) {
      esc=0;
    }
    else {
      int i;
      unsigned char c;

      i=GWEN_Buffer_ReadByte(msgbuf);
      if (i==-1) {
        DBG_INFO(GWEN_LOGDOMAIN, "called from here");
        return 0;
      }
      c=(unsigned int)i;
      if (c==escapeChar) { /* escape */
        esc=1;
      }
      else if (c=='@') {
      /* skip binary data */
        char lbuffer[16];
        char *p;
        int l;
        int nc;

        p=lbuffer;
        while(1) {
          nc=GWEN_Buffer_ReadByte(msgbuf);
          if (nc==-1) {
            DBG_ERROR(GWEN_LOGDOMAIN, "\"@num@\" expected");
            return -1;
          }
          if (nc=='@')
            break;
          *p=nc;
          p++;
        } /* while */
        *p=0;
        if (sscanf(lbuffer, "%d", &l)!=1) {
          DBG_ERROR(GWEN_LOGDOMAIN, "Bad number format");
          return -1;
        }
        if (GWEN_Buffer_GetUsedBytes(msgbuf)-GWEN_Buffer_GetPos(msgbuf) 
	    < (unsigned) l) {
          DBG_ERROR(GWEN_LOGDOMAIN, "Premature end of message (binary beyond end)");
          return -1;
        }
        GWEN_Buffer_IncrementPos(msgbuf, l);
      }
      else if (c==delimiter) {/* segment-end */
        return 0;
        break;
      }
    }
  } /* while */

  DBG_ERROR(GWEN_LOGDOMAIN, "End of segment not found");
  return -1;
}



/* --------------------------------------------------------------- FUNCTION */
int GWEN_MsgEngine_ReadMessage(GWEN_MSGENGINE *e,
                               const char *gtype,
                               GWEN_BUFFER *mbuf,
                               GWEN_DB_NODE *gr,
                               uint32_t flags) {
  unsigned int segments;

  segments=0;

  while(GWEN_Buffer_GetBytesLeft(mbuf)) {
    GWEN_XMLNODE *node;
    unsigned int posBak;
    const char *p;
    GWEN_DB_NODE *tmpdb;
    int segVer;

    /* find head segment description */
    tmpdb=GWEN_DB_Group_new("tmpdb");
    node=GWEN_MsgEngine_FindGroupByProperty(e,
                                            "id",
                                            0,
                                            "SegHead");
    if (node==0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Segment description not found");
      GWEN_DB_Group_free(tmpdb);
      return -1;
    }

    /* parse head segment */
    posBak=GWEN_Buffer_GetPos(mbuf);
    if (GWEN_MsgEngine_ParseMessage(e,
                                    node,
                                    mbuf,
                                    tmpdb,
                                    flags)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error parsing segment head");
      GWEN_DB_Group_free(tmpdb);
      return -1;
    }

    /* get segment code */
    segVer=GWEN_DB_GetIntValue(tmpdb,
                               "version",
                               0,
                               0);
    p=GWEN_DB_GetCharValue(tmpdb,
                           "code",
                           0,
			   0);
    if (!p) {
      DBG_ERROR(GWEN_LOGDOMAIN, "No segment code for %s ? This seems to be a bad msg...",
		gtype);
      GWEN_Buffer_SetPos(mbuf, posBak);
      DBG_ERROR(GWEN_LOGDOMAIN, "Full message (pos=%04x)", posBak);
      GWEN_Text_DumpString(GWEN_Buffer_GetStart(mbuf),
			   GWEN_Buffer_GetUsedBytes(mbuf),
			   stderr, 1);
      GWEN_DB_Dump(tmpdb, stderr, 1);
      GWEN_DB_Group_free(tmpdb);
      return -1;
    }

    /* try to find corresponding XML node */
    node=GWEN_MsgEngine_FindNodeByProperty(e,
                                           gtype,
                                           "code",
					   segVer,
                                           p);
    if (node==0) {
      unsigned int ustart;

      ustart=GWEN_Buffer_GetPos(mbuf);
      ustart++; /* skip delimiter */

      /* node not found, skip it */
      DBG_NOTICE(GWEN_LOGDOMAIN,
		 "Unknown segment \"%s\" (Segnum=%d, version=%d, ref=%d)",
		 p,
		 GWEN_DB_GetIntValue(tmpdb, "seq", 0, -1),
		 GWEN_DB_GetIntValue(tmpdb, "version", 0, -1),
		 GWEN_DB_GetIntValue(tmpdb, "ref", 0, -1));
      if (GWEN_MsgEngine_SkipSegment(e, mbuf, '?', '\'')) {
	DBG_ERROR(GWEN_LOGDOMAIN, "Error skipping segment \"%s\"", p);
	GWEN_DB_Group_free(tmpdb);
	return -1;
      }
      if (flags & GWEN_MSGENGINE_READ_FLAGS_TRUSTINFO) {
        unsigned int usize;

	usize=GWEN_Buffer_GetPos(mbuf)-ustart-1;
#if 0
	GWEN_Text_DumpString(GWEN_Buffer_GetStart(mbuf)+ustart,
			     usize,
			     stderr, 1);
#endif
        if (GWEN_MsgEngine_AddTrustInfo(e,
					GWEN_Buffer_GetStart(mbuf)+ustart,
                                        usize,
                                        p,
                                        GWEN_MsgEngineTrustLevelHigh,
                                        ustart)) {
          DBG_INFO(GWEN_LOGDOMAIN, "called from here");
          GWEN_DB_Group_free(tmpdb);
          return -1;
        }
      } /* if trustInfo handling wanted */
    }
    else {
      /* ok, node available, get the corresponding description and parse
       * the segment */
      const char *id;
      GWEN_DB_NODE *storegrp;
      unsigned int startPos;

      /* restore start position, since the segment head is part of a full
       * description, so we need to restart reading from the very begin */
      GWEN_Buffer_SetPos(mbuf, posBak);

      /* create group in DB for this segment */
      id=GWEN_XMLNode_GetProperty(node, "id", p);
      storegrp=GWEN_DB_GetGroup(gr,
                                GWEN_PATH_FLAGS_CREATE_GROUP,
                                id);
      assert(storegrp);

      /* store the start position of this segment within the DB */
      startPos=GWEN_Buffer_GetPos(mbuf);
      GWEN_DB_SetIntValue(storegrp,
                          GWEN_DB_FLAGS_OVERWRITE_VARS,
                          "segment/pos",
                          startPos);

      /* parse the segment */
      if (GWEN_MsgEngine_ParseMessage(e,
                                      node,
                                      mbuf,
                                      storegrp,
                                      flags)) {
	DBG_ERROR(GWEN_LOGDOMAIN, "Error parsing segment \"%s\" at %d (%x)",
		  p,
		  GWEN_Buffer_GetPos(mbuf)-startPos,
		  GWEN_Buffer_GetPos(mbuf)-startPos);
	GWEN_Text_DumpString(GWEN_Buffer_GetStart(mbuf)+startPos,
			     GWEN_Buffer_GetUsedBytes(mbuf)-startPos,
			     stderr, 1);
	DBG_ERROR(GWEN_LOGDOMAIN, "Stored data so far:");
	GWEN_DB_Dump(storegrp, stderr, 2);
	GWEN_DB_Group_free(tmpdb);
	return -1;
      }

      /* store segment size within DB */
      GWEN_DB_SetIntValue(storegrp,
                          GWEN_DB_FLAGS_OVERWRITE_VARS,
                          "segment/length",
                          GWEN_Buffer_GetPos(mbuf)-startPos);
      segments++;
    }
    GWEN_DB_Group_free(tmpdb);
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








GWEN_MSGENGINE_TRUSTEDDATA*
GWEN_MsgEngine_TrustedData_new(const char *data,
                               unsigned int size,
                               const char *description,
                               GWEN_MSGENGINE_TRUSTLEVEL trustLevel){
  GWEN_MSGENGINE_TRUSTEDDATA *td;

  assert(data);
  assert(size);
  GWEN_NEW_OBJECT(GWEN_MSGENGINE_TRUSTEDDATA, td);
  td->data=(char*)malloc(size);
  assert(td->data);
  memmove(td->data, data, size);
  if (description)
    td->description=strdup(description);
  td->trustLevel=trustLevel;
  td->size=size;
  return td;
}



void GWEN_MsgEngine_TrustedData_free(GWEN_MSGENGINE_TRUSTEDDATA *td){
  if (td) {
    free(td->data);
    free(td->description);
    free(td->replacement);
    GWEN_FREE_OBJECT(td);
  }
}



GWEN_MSGENGINE_TRUSTEDDATA*
GWEN_MsgEngine_TrustedData_GetNext(GWEN_MSGENGINE_TRUSTEDDATA *td){
  assert(td);
  return td->next;
}



const char*
GWEN_MsgEngine_TrustedData_GetData(GWEN_MSGENGINE_TRUSTEDDATA *td){
  assert(td);
  return td->data;
}



unsigned int
GWEN_MsgEngine_TrustedData_GetSize(GWEN_MSGENGINE_TRUSTEDDATA *td){
  assert(td);
  return td->size;
}



const char*
GWEN_MsgEngine_TrustedData_GetDescription(GWEN_MSGENGINE_TRUSTEDDATA *td){
  assert(td);
  return td->description;
}



GWEN_MSGENGINE_TRUSTLEVEL
GWEN_MsgEngine_TrustedData_GetTrustLevel(GWEN_MSGENGINE_TRUSTEDDATA *td){
  assert(td);
  return td->trustLevel;
}



const char*
GWEN_MsgEngine_TrustedData_GetReplacement(GWEN_MSGENGINE_TRUSTEDDATA *td){
  assert(td);
  return td->replacement;
}



int GWEN_MsgEngine_TrustedData_AddPos(GWEN_MSGENGINE_TRUSTEDDATA *td,
                                      unsigned int pos){
  assert(td);
  if (td->posCount>=GWEN_MSGENGINE_TRUSTEDDATA_MAXPOS)
    return -1;
  td->positions[td->posCount++]=pos;
  return 0;
}



int GWEN_MsgEngine_TrustedData_GetFirstPos(GWEN_MSGENGINE_TRUSTEDDATA *td){
  assert(td);
  td->posPointer=0;
  return GWEN_MsgEngine_TrustedData_GetNextPos(td);
}



int GWEN_MsgEngine_TrustedData_GetNextPos(GWEN_MSGENGINE_TRUSTEDDATA *td){
  assert(td);
  if (td->posPointer>=td->posCount)
    return -1;
  return td->positions[td->posPointer++];
}



int
GWEN_MsgEngine_TrustedData_CreateReplacements(GWEN_MSGENGINE_TRUSTEDDATA
                                              *td){
  unsigned int nextNr;
  GWEN_MSGENGINE_TRUSTEDDATA *ntd;
  unsigned int count;

  assert(td);
  count=0;
  ntd=td;
  while(ntd) {
    count++;
    ntd=ntd->next;
  }

  if (count<0x10)
    nextNr=0x01;
  else
    nextNr=0x11;

  ntd=td;
  while(ntd) {
    unsigned int i;
    char numbuffer[32];
    char *rp;
    GWEN_MSGENGINE_TRUSTEDDATA *std;
    int match;

    /* check whether the same data already exists */
    std=td;
    match=0;
    while(std && std!=ntd) {

      match=1;
      if (std->size==ntd->size) {
        unsigned int i;

        for (i=0; i<td->size; i++) {
          if (std->data[i]!=ntd->data[i]) {
            match=0;
            break;
          }
        } /* for */
      }
      else
        match=0;

      if (match)
        break;
      std=std->next;
    } /* while */

    if (match) {
      /* copy the found match */
      rp=strdup(std->replacement);
    }
    else {
      /* this is a new one */
      rp=(char*)malloc(ntd->size+1);
      assert(rp);

      if (ntd->size==1) {
        if (count>=0x10)
          nextNr+=0x10;
      }
      sprintf(numbuffer, "%02X", nextNr++);
      for (i=0; i<ntd->size; i++) {
        if (count<0x10)
          rp[i]=numbuffer[1];
        else
          rp[i]=numbuffer[1-(i&1)];
      } /* for */
      rp[i]=0;
    }
    /*
     DBG_DEBUG(GWEN_LOGDOMAIN, "Replacement: \"%s\" for \"%s\" (%d)", rp,
     ntd->description,
     ntd->size);
     */
    free(ntd->replacement);
    ntd->replacement=rp;

    ntd=ntd->next;
  } /* while */
  return 0;
}



GWEN_MSGENGINE_TRUSTEDDATA *GWEN_MsgEngine_TakeTrustInfo(GWEN_MSGENGINE *e){
  GWEN_MSGENGINE_TRUSTEDDATA *td;

  assert(e);
  td=e->trustInfos;
  e->trustInfos=0;
  return td;
}




int GWEN_MsgEngine_AddTrustInfo(GWEN_MSGENGINE *e,
                                const char *data,
                                unsigned int size,
                                const char *description,
                                GWEN_MSGENGINE_TRUSTLEVEL trustLevel,
                                unsigned int pos) {
  GWEN_MSGENGINE_TRUSTEDDATA *td;
  int match;

  assert(e);
  assert(data);
  assert(size);

  if (!description)
    description="";

  td=e->trustInfos;
  while(td) {
    unsigned int i;

    /* compare data */
    if (td->size==size &&
        *description &&
        *(td->description) &&
        trustLevel==td->trustLevel &&
        strcasecmp(description, td->description)==0) {
      match=1;
      for (i=0; i<td->size; i++) {
        if (td->data[i]!=data[i]) {
          match=0;
          break;
        }
      } /* for */
    }
    else
      match=0;

    if (match)
      break;
    td=td->next;
  } /* while */

  if (!td) {
    DBG_INFO(GWEN_LOGDOMAIN, "Creating new trustInfo for \"%s\" (%d)",
             description, size);
    td=GWEN_MsgEngine_TrustedData_new(data,
                                      size,
                                      description,
                                      trustLevel);
    GWEN_LIST_ADD(GWEN_MSGENGINE_TRUSTEDDATA, td, &(e->trustInfos));
  }
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "Reusing trustInfo for \"%s\" (%d)",
             description, size);
  }
  GWEN_MsgEngine_TrustedData_AddPos(td, pos);
  return 0;
}



