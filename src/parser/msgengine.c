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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define DISABLE_DEBUGLOG
/*#define HEAVY_DEBUG_MSGENGINE*/


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


GWEN_MSGENGINE *GWEN_MsgEngine_new(void)
{
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


void GWEN_MsgEngine_free(GWEN_MSGENGINE *e)
{
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
        while (td) {
          tdn=td->next;
          GWEN_MsgEngine_TrustedData_free(td);
          td=tdn;
        } /* while */
      }
      GWEN_FREE_OBJECT(e);
    }
  }
}



void GWEN_MsgEngine_Attach(GWEN_MSGENGINE *e)
{
  assert(e);
  e->usage++;
}


void GWEN_MsgEngine_SetEscapeChar(GWEN_MSGENGINE *e, char c)
{
  assert(e);
  e->escapeChar=c;
}



char GWEN_MsgEngine_GetEscapeChar(GWEN_MSGENGINE *e)
{
  assert(e);
  return e->escapeChar;
}



void GWEN_MsgEngine_SetCharsToEscape(GWEN_MSGENGINE *e, const char *c)
{
  assert(e);
  free(e->charsToEscape);
  e->charsToEscape=strdup(c);
}



const char *GWEN_MsgEngine_GetCharsToEscape(GWEN_MSGENGINE *e)
{
  assert(e);
  return e->charsToEscape;
}



void GWEN_MsgEngine_SetDelimiters(GWEN_MSGENGINE *e, const char *s)
{
  assert(e);
  free(e->delimiters);
  if (s)
    e->delimiters=strdup(s);
  else
    e->delimiters=strdup(GWEN_MSGENGINE_DEFAULT_DELIMITERS);
}



const char *GWEN_MsgEngine_GetDelimiters(GWEN_MSGENGINE *e)
{
  assert(e);
  return e->delimiters;
}



void GWEN_MsgEngine_SetMode(GWEN_MSGENGINE *e, const char *mode)
{
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


const char *GWEN_MsgEngine_GetMode(GWEN_MSGENGINE *e)
{
  GWEN_DB_NODE *db;

  assert(e);
  db=GWEN_MsgEngine__GetGlobalValues(e);
  return GWEN_DB_GetCharValue(db, "engine/secmode", 0, 0);
}



GWEN_DB_NODE *GWEN_MsgEngine__GetGlobalValues(GWEN_MSGENGINE *e)
{
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



unsigned int GWEN_MsgEngine_GetProtocolVersion(GWEN_MSGENGINE *e)
{
  GWEN_DB_NODE *db;

  assert(e);
  db=GWEN_MsgEngine__GetGlobalValues(e);
  return GWEN_DB_GetIntValue(db, "engine/pversion", 0, 0);
}



void GWEN_MsgEngine_SetProtocolVersion(GWEN_MSGENGINE *e,
                                       unsigned int p)
{
  GWEN_DB_NODE *db;

  assert(e);
  db=GWEN_MsgEngine__GetGlobalValues(e);

  GWEN_DB_SetIntValue(db,
                      GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "engine/pversion",
                      p);
}



GWEN_XMLNODE *GWEN_MsgEngine_GetDefinitions(GWEN_MSGENGINE *e)
{
  assert(e);
  return e->defs;
}


void GWEN_MsgEngine_SetDefinitions(GWEN_MSGENGINE *e,
                                   GWEN_XMLNODE *n,
                                   int take)
{
  assert(e);
  if (e->ownDefs)
    GWEN_XMLNode_free(e->defs);
  e->defs=n;
  e->ownDefs=take;
}



void GWEN_MsgEngine_SetGetGlobalValuesFunction(GWEN_MSGENGINE *e,
                                               GWEN_MSGENGINE_GETGLOBALVALUES_PTR p)
{
  assert(e);
  e->getGlobalValuesPtr=p;
}



GWEN_MSGENGINE_GETGLOBALVALUES_PTR GWEN_MsgEngine_GetGetGlobalValuesFunction(GWEN_MSGENGINE *e)
{
  assert(e);
  return e->getGlobalValuesPtr;
}



void GWEN_MsgEngine_SetTypeReadFunction(GWEN_MSGENGINE *e,
                                        GWEN_MSGENGINE_TYPEREAD_PTR p)
{
  assert(e);
  e->typeReadPtr=p;
}



GWEN_MSGENGINE_TYPEREAD_PTR GWEN_MsgEngine_GetTypeReadFunction(GWEN_MSGENGINE *e)
{
  assert(e);
  return e->typeReadPtr;
}



void GWEN_MsgEngine_SetTypeWriteFunction(GWEN_MSGENGINE *e,
                                         GWEN_MSGENGINE_TYPEWRITE_PTR p)
{
  assert(e);
  e->typeWritePtr=p;
}



GWEN_MSGENGINE_TYPEWRITE_PTR GWEN_MsgEngine_GetTypeWriteFunction(GWEN_MSGENGINE *e)
{
  assert(e);
  return e->typeWritePtr;
}



void GWEN_MsgEngine_SetTypeCheckFunction(GWEN_MSGENGINE *e,
                                         GWEN_MSGENGINE_TYPECHECK_PTR p)
{
  assert(e);
  e->typeCheckPtr=p;
}



GWEN_MSGENGINE_TYPECHECK_PTR GWEN_MsgEngine_GetTypeCheckFunction(GWEN_MSGENGINE *e)
{
  assert(e);
  return e->typeCheckPtr;
}






void GWEN_MsgEngine_SetBinTypeReadFunction(GWEN_MSGENGINE *e,
                                           GWEN_MSGENGINE_BINTYPEREAD_PTR p)
{
  assert(e);
  e->binTypeReadPtr=p;
}



GWEN_MSGENGINE_BINTYPEREAD_PTR GWEN_MsgEngine_GetBinTypeReadFunction(GWEN_MSGENGINE *e)
{
  assert(e);
  return e->binTypeReadPtr;
}



void GWEN_MsgEngine_SetBinTypeWriteFunction(GWEN_MSGENGINE *e,
                                            GWEN_MSGENGINE_BINTYPEWRITE_PTR p)
{
  assert(e);
  e->binTypeWritePtr=p;
}



GWEN_MSGENGINE_BINTYPEWRITE_PTR GWEN_MsgEngine_GetBinTypeWriteFunction(GWEN_MSGENGINE *e)
{
  assert(e);
  return e->binTypeWritePtr;
}



void GWEN_MsgEngine_SetGetCharValueFunction(GWEN_MSGENGINE *e,
                                            GWEN_MSGENGINE_GETCHARVALUE_PTR p)
{
  assert(e);
  e->getCharValuePtr=p;
}



void GWEN_MsgEngine_SetGetIntValueFunction(GWEN_MSGENGINE *e,
                                           GWEN_MSGENGINE_GETINTVALUE_PTR p)
{
  assert(e);
  e->getIntValuePtr=p;
}



void GWEN_MsgEngine_SetFreeDataFunction(GWEN_MSGENGINE *e,
                                        GWEN_MSGENGINE_FREEDATA_PTR p)
{
  assert(e);
  DBG_WARN(GWEN_LOGDOMAIN, "GWEN_MsgEngine_SetFreeDataFunction: Deprecated");
  e->freeDataPtr=p;
}



void *GWEN_MsgEngine_GetInheritorData(const GWEN_MSGENGINE *e)
{
  assert(e);
  return e->inheritorData;
}



void GWEN_MsgEngine_SetInheritorData(GWEN_MSGENGINE *e, void *d)
{
  assert(e);
  DBG_WARN(GWEN_LOGDOMAIN, "GWEN_MsgEngine_SetInheritorData: Deprecated");
  if (e->inheritorData && e->freeDataPtr)
    e->freeDataPtr(e);
  e->inheritorData=d;
}






int GWEN_MsgEngine__IsCharTyp(GWEN_MSGENGINE *e,
                              const char *type)
{
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
                             const char *type)
{
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
                             const char *type)
{
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
                              GWEN_BUFFER *mbuf)
{
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
    //unsigned int dplen;
    const char *stype;

    stype=GWEN_XMLNode_GetProperty(node, "storedAs", type);
    if (GWEN_MsgEngine__IsBinTyp(e, stype)) {
      dp=GWEN_XMLNode_GetData(n);
      //dplen=strlen(dp);
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



GWEN_XMLNODE *GWEN_MsgEngine_FindGroupByProperty(GWEN_MSGENGINE *e,
                                                 const char *pname,
                                                 int version,
                                                 const char *pvalue)
{
  return GWEN_MsgEngine_FindNodeByProperty(e, "GROUP", pname, version, pvalue);
}



GWEN_XMLNODE *GWEN_MsgEngine_FindNodeByProperty(GWEN_MSGENGINE *e,
                                                const char *t,
                                                const char *pname,
                                                int version,
                                                const char *pvalue)
{
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
  strcat(buffer, "S");
  while (n) {
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
  strcat(buffer, "def");
  while (n) {
    if (GWEN_XMLNode_GetType(n)==GWEN_XMLNodeTypeTag) {
      p=GWEN_XMLNode_GetData(n);
      assert(p);
      if (strcasecmp(p, buffer)==0) {
        p=GWEN_XMLNode_GetProperty(n, pname, "");
        if (strcasecmp(p, pvalue)==0) {
          i=atoi(GWEN_XMLNode_GetProperty(n, "pversion", "0"));
          if (proto==0 || (int)proto==i || i==0) {
            i=atoi(GWEN_XMLNode_GetProperty(n, "version", "0"));
            if (version==0 || version==i) {
              p=GWEN_XMLNode_GetProperty(n, "mode", "");
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
                                                           const char *pvalue)
{
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
  strcat(buffer, "S");
  while (n) {
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
  strcat(buffer, "def");
  while (n) {
    if (GWEN_XMLNode_GetType(n)==GWEN_XMLNodeTypeTag) {
      p=GWEN_XMLNode_GetData(n);
      assert(p);
      if (strcasecmp(p, buffer)==0) {
        p=GWEN_XMLNode_GetProperty(n, pname, "");
        if (strcasecmp(p, pvalue)==0) {
          i=atoi(GWEN_XMLNode_GetProperty(n, "pversion", "0"));
          if (proto==0 || (int)proto==i) {
            i=atoi(GWEN_XMLNode_GetProperty(n, "version", "0"));
            if (version==0 || version==i) {
              p=GWEN_XMLNode_GetProperty(n, "mode", "");
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
                                           unsigned int *datasize)
{
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
        if (GWEN_Text_NumToString(z, pbuffer, sizeof(pbuffer), 0)<1) {
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

        switch (vt) {
        case GWEN_DB_NodeType_ValueChar:
          pvalue=GWEN_DB_GetCharValue(globalValues, p, 0, "");
          *datasize=strlen(pvalue);
          break;

        case GWEN_DB_NodeType_ValueInt:
          z=GWEN_DB_GetIntValue(globalValues, p, 0, 0);
          if (GWEN_Text_NumToString(z, pbuffer, sizeof(pbuffer), 0)<1) {
            DBG_ERROR(GWEN_LOGDOMAIN, "Error converting num to string");
            return 0;
          }
          pvalue=pbuffer;
          *datasize=strlen(pvalue);
          break;

        case GWEN_DB_NodeType_ValueBin:
          pvalue=GWEN_DB_GetBinValue(globalValues, p, 0,
                                     0, 0,
                                     datasize);
          break;

        default:
          DBG_ERROR(GWEN_LOGDOMAIN, "Unknown type %s", type);
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
      dtype=GWEN_XMLNode_GetProperty(dnode, "type", "ASCII");

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

      switch (vt) {
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
          if (GWEN_Text_NumToString(z, pbuffer, sizeof(pbuffer), 0)<1) {
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
        DBG_ERROR(GWEN_LOGDOMAIN, "Unhandled type %s", dtype);
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
                                             int topDown)
{
  const char *pvalue;
  GWEN_XMLNODE *pn;
  const char *lastValue;

  DBG_DEBUG(GWEN_LOGDOMAIN, "Searching for value of \"%s\" in properties", name);
  lastValue=0;

  pvalue=GWEN_XMLNode_GetProperty(node, name, 0);
  if (pvalue) {
    if (!topDown)
      return pvalue;
    DBG_DEBUG(GWEN_LOGDOMAIN, "Found a value (%s), but will look further", pvalue);
    lastValue=pvalue;
  }

  pn=refnode;
  while (pn) {
    pvalue=GWEN_XMLNode_GetProperty(pn, name, 0);
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
                                        GWEN_XMLNODE *refnode)
{
  int value;
  GWEN_XMLNODE *pn;
  int highestTrust;

  highestTrust=0;

  value=atoi(GWEN_XMLNode_GetProperty(node, "trustlevel", "0"));
  if (value>highestTrust)
    highestTrust=value;

  pn=node;
  while (pn) {
    value=atoi(GWEN_XMLNode_GetProperty(pn, "trustlevel", "0"));
    if (value>highestTrust)
      highestTrust=value;
    pn=GWEN_XMLNode_GetParent(pn);
  } /* while */

  pn=refnode;
  while (pn) {
    value=atoi(GWEN_XMLNode_GetProperty(pn, "trustlevel", "0"));
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
                                           unsigned int *datasize)
{
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
  while (pn) {
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
        tmpptr=(char *)malloc(i);
        assert(tmpptr);
        sprintf(tmpptr, "%s/%s", ppath, bufferPtr);
        free(bufferPtr);
        bufferPtr=tmpptr;
      }
      else {
        i=strlen(ppath)+strlen(name)+2;
        tmpptr=(char *)malloc(i);
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
                                         unsigned int *datasize)
{
  GWEN_XMLNODE *pn;

  DBG_VERBOUS(GWEN_LOGDOMAIN, "Looking for value of \"%s\" in <VALUES>", name);
  pn=GWEN_XMLNode_GetChild(node);

  while (pn) {
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
        while (n) {
          if (GWEN_XMLNode_GetType(n)==GWEN_XMLNodeTypeTag) {
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
                                       const char *pvalue)
{
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
  strcat(buffer, "S");
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
  while (n) {
    p=GWEN_XMLNode_GetData(n);
    assert(p);
    if (strcasecmp(p, buffer)==0 ||
        strcasecmp(p, t)==0) {
      p=GWEN_XMLNode_GetProperty(n, "id", "");
      if (strcasecmp(p, pvalue)!=0)
        p=GWEN_XMLNode_GetProperty(n, "name", "");
      if (strcasecmp(p, pvalue)==0) {
        i=atoi(GWEN_XMLNode_GetProperty(n, "pversion", "0"));
        if (proto==0 || (int)proto==i || i==0) {
          i=atoi(GWEN_XMLNode_GetProperty(n, "version", "0"));
          if (version==0 || version==i) {
            p=GWEN_XMLNode_GetProperty(n, "mode", "");
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
                                      const char *pvalue)
{
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
  while (n) {
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
    while (n) {
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



int GWEN_MsgEngine_AddDefinitions(GWEN_MSGENGINE *e,
                                  GWEN_XMLNODE *node)
{
  GWEN_XMLNODE *nsrc, *ndst;

  assert(e);
  assert(node);

  if (!e->defs) {
    e->defs=GWEN_XMLNode_dup(node);
    e->ownDefs=1;
    return 0;
  }

  nsrc=GWEN_XMLNode_GetChild(node);
  while (nsrc) {
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





int GWEN_MsgEngine_SetValue(GWEN_MSGENGINE *e,
                            const char *path,
                            const char *value)
{
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
                               int value)
{
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
                                    const char *defValue)
{
  GWEN_DB_NODE *globalValues;

  assert(e);
  globalValues=GWEN_MsgEngine__GetGlobalValues(e);
  assert(globalValues);
  return GWEN_DB_GetCharValue(globalValues,
                              path, 0, defValue);
}



int GWEN_MsgEngine_GetIntValue(GWEN_MSGENGINE *e,
                               const char *path,
                               int defValue)
{
  GWEN_DB_NODE *globalValues;

  assert(e);
  globalValues=GWEN_MsgEngine__GetGlobalValues(e);
  assert(globalValues);
  return GWEN_DB_GetIntValue(globalValues,
                             path, 0, defValue);
}





GWEN_MSGENGINE_TRUSTEDDATA *GWEN_MsgEngine_TrustedData_new(const char *data,
                                                           unsigned int size,
                                                           const char *description,
                                                           GWEN_MSGENGINE_TRUSTLEVEL trustLevel)
{
  GWEN_MSGENGINE_TRUSTEDDATA *td;

  assert(data);
  assert(size);
  GWEN_NEW_OBJECT(GWEN_MSGENGINE_TRUSTEDDATA, td);
  td->data=(char *)malloc(size);
  assert(td->data);
  memmove(td->data, data, size);
  if (description)
    td->description=strdup(description);
  td->trustLevel=trustLevel;
  td->size=size;
  return td;
}



void GWEN_MsgEngine_TrustedData_free(GWEN_MSGENGINE_TRUSTEDDATA *td)
{
  if (td) {
    free(td->data);
    free(td->description);
    free(td->replacement);
    GWEN_FREE_OBJECT(td);
  }
}



GWEN_MSGENGINE_TRUSTEDDATA *GWEN_MsgEngine_TrustedData_GetNext(GWEN_MSGENGINE_TRUSTEDDATA *td)
{
  assert(td);
  return td->next;
}



const char *GWEN_MsgEngine_TrustedData_GetData(GWEN_MSGENGINE_TRUSTEDDATA *td)
{
  assert(td);
  return td->data;
}



unsigned int GWEN_MsgEngine_TrustedData_GetSize(GWEN_MSGENGINE_TRUSTEDDATA *td)
{
  assert(td);
  return td->size;
}



const char *GWEN_MsgEngine_TrustedData_GetDescription(GWEN_MSGENGINE_TRUSTEDDATA *td)
{
  assert(td);
  return td->description;
}



GWEN_MSGENGINE_TRUSTLEVEL GWEN_MsgEngine_TrustedData_GetTrustLevel(GWEN_MSGENGINE_TRUSTEDDATA *td)
{
  assert(td);
  return td->trustLevel;
}



const char *GWEN_MsgEngine_TrustedData_GetReplacement(GWEN_MSGENGINE_TRUSTEDDATA *td)
{
  assert(td);
  return td->replacement;
}



int GWEN_MsgEngine_TrustedData_AddPos(GWEN_MSGENGINE_TRUSTEDDATA *td,
                                      unsigned int pos)
{
  assert(td);
  if (td->posCount>=GWEN_MSGENGINE_TRUSTEDDATA_MAXPOS)
    return -1;
  td->positions[td->posCount++]=pos;
  return 0;
}



int GWEN_MsgEngine_TrustedData_GetFirstPos(GWEN_MSGENGINE_TRUSTEDDATA *td)
{
  assert(td);
  td->posPointer=0;
  return GWEN_MsgEngine_TrustedData_GetNextPos(td);
}



int GWEN_MsgEngine_TrustedData_GetNextPos(GWEN_MSGENGINE_TRUSTEDDATA *td)
{
  assert(td);
  if (td->posPointer>=td->posCount)
    return -1;
  return td->positions[td->posPointer++];
}



int GWEN_MsgEngine_TrustedData_CreateReplacements(GWEN_MSGENGINE_TRUSTEDDATA
                                                  *td)
{
  unsigned int nextNr;
  GWEN_MSGENGINE_TRUSTEDDATA *ntd;
  unsigned int count;

  assert(td);
  count=0;
  ntd=td;
  while (ntd) {
    count++;
    ntd=ntd->next;
  }

  if (count<0x10)
    nextNr=0x01;
  else
    nextNr=0x11;

  ntd=td;
  while (ntd) {
    unsigned int i;
    char numbuffer[32];
    char *rp;
    GWEN_MSGENGINE_TRUSTEDDATA *std;
    int match;

    /* check whether the same data already exists */
    std=td;
    match=0;
    while (std && std!=ntd) {

      match=1;
      if (std->size==ntd->size) {
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
      rp=(char *)malloc(ntd->size+1);
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



GWEN_MSGENGINE_TRUSTEDDATA *GWEN_MsgEngine_TakeTrustInfo(GWEN_MSGENGINE *e)
{
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
                                unsigned int pos)
{
  GWEN_MSGENGINE_TRUSTEDDATA *td;
  int match;

  assert(e);
  assert(data);
  assert(size);

  if (!description)
    description="";

  td=e->trustInfos;
  while (td) {
    unsigned int i;

    /* compare data */
    if (td->size==size &&
        *description && *(td->description) &&
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
    DBG_INFO(GWEN_LOGDOMAIN, "Creating new trustInfo for \"%s\" (%d)", description, size);
    td=GWEN_MsgEngine_TrustedData_new(data, size, description, trustLevel);
    GWEN_LIST_ADD(GWEN_MSGENGINE_TRUSTEDDATA, td, &(e->trustInfos));
  }
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "Reusing trustInfo for \"%s\" (%d)", description, size);
  }
  GWEN_MsgEngine_TrustedData_AddPos(td, pos);
  return 0;
}



#include "msgengine_read.c"
#include "msgengine_write.c"
#include "msgengine_show.c"


