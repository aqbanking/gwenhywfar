/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Sat Jun 28 2003
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

/*#define DISABLE_DEBUGLOG */

#include "xsd_p.h"
#include "xml_l.h"
#include "gwenhywfar/debug.h"
#include "gwenhywfar/misc.h"
#include "gwenhywfar/text.h"
#include "gwenhywfar/base64.h"
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
#include <unistd.h>

#ifdef HAVE_REGEX_H
# include <regex.h>
#endif



int GWEN_XSD__ValidateElementData(GWEN_XSD_ENGINE *e,
                                  GWEN_TYPE_UINT32 facetMask,
                                  GWEN_XSD_FACETS *xf,
                                  GWEN_BUFFER *vbuf) {
  int charCount;
  double dv;
  int haveDouble;

  haveDouble=0;

  /* length */
  charCount=GWEN_Text_CountUtf8Chars(GWEN_Buffer_GetStart(vbuf),
                                     GWEN_Buffer_GetUsedBytes(vbuf));
  if ((xf->setMask & GWEN_XSD_FACET_LENGTH) &&
      (facetMask & GWEN_XSD_FACET_LENGTH)) {
    if (charCount!=xf->length) {
      DBG_ERROR(GWEN_LOGDOMAIN, "LENGTH: Invalid length (%d!=%d)",
                charCount, xf->length);
      return -1;
    }
  }

  /* maxlength */
  if ((xf->setMask & GWEN_XSD_FACET_MAXLENGTH) &&
      (facetMask & GWEN_XSD_FACET_MAXLENGTH)) {
    if (charCount>xf->maxLength) {
      DBG_ERROR(GWEN_LOGDOMAIN, "MAXLENGTH: Invalid length (%d>%d)",
                charCount, xf->maxLength);
      return -1;
    }
  }

  /* minlength */
  if ((xf->setMask & GWEN_XSD_FACET_MINLENGTH) &&
      (facetMask & GWEN_XSD_FACET_MINLENGTH)) {
    if (charCount<xf->minLength) {
      DBG_ERROR(GWEN_LOGDOMAIN, "MINLENGTH: Invalid length (%d<%d)",
                charCount, xf->minLength);
      return -1;
    }
  }

  /* maxInclusive */
  if ((xf->setMask & GWEN_XSD_FACET_MAXINCLUSIVE) &&
      (facetMask & GWEN_XSD_FACET_MAXINCLUSIVE)) {
    if (!haveDouble) {
      if (GWEN_Text_StringToDouble(GWEN_Buffer_GetStart(vbuf), &dv)) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Data is not a number (%s)",
                  GWEN_Buffer_GetStart(vbuf));
        return -1;
      }
      haveDouble=1;
    }
    if (dv>xf->maxInclusive) {
      DBG_ERROR(GWEN_LOGDOMAIN, "MAXINCLUSIVE: Invalid value (%lf>%lf)",
                dv, xf->maxInclusive);
      return -1;
    }
  }

  /* maxExclusive */
  if ((xf->setMask & GWEN_XSD_FACET_MAXEXCLUSIVE) &&
      (facetMask & GWEN_XSD_FACET_MAXEXCLUSIVE)) {
    if (!haveDouble) {
      if (GWEN_Text_StringToDouble(GWEN_Buffer_GetStart(vbuf), &dv)) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Data is not a number (%s)",
                  GWEN_Buffer_GetStart(vbuf));
        return -1;
      }
      haveDouble=1;
    }
    if (dv>=xf->maxExclusive) {
      DBG_ERROR(GWEN_LOGDOMAIN, "MAXEXCLUSIVE: Invalid value (%lf>=%lf)",
                dv, xf->maxExclusive);
      return -1;
    }
  }

  /* minInclusive */
  if ((xf->setMask & GWEN_XSD_FACET_MININCLUSIVE) &&
      (facetMask & GWEN_XSD_FACET_MININCLUSIVE)) {
    if (!haveDouble) {
      if (GWEN_Text_StringToDouble(GWEN_Buffer_GetStart(vbuf), &dv)) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Data is not a number (%s)",
                  GWEN_Buffer_GetStart(vbuf));
        return -1;
      }
      haveDouble=1;
    }
    if (dv<xf->minInclusive) {
      DBG_ERROR(GWEN_LOGDOMAIN, "MININCLUSIVE: Invalid value (%lf<%lf)",
                dv, xf->minInclusive);
      return -1;
    }
  }

  /* minExclusive */
  if ((xf->setMask & GWEN_XSD_FACET_MINEXCLUSIVE) &&
      (facetMask & GWEN_XSD_FACET_MINEXCLUSIVE)) {
    if (!haveDouble) {
      if (GWEN_Text_StringToDouble(GWEN_Buffer_GetStart(vbuf), &dv)) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Data is not a number (%s)",
                  GWEN_Buffer_GetStart(vbuf));
        return -1;
      }
      haveDouble=1;
    }
    if (dv<=xf->minExclusive) {
      DBG_ERROR(GWEN_LOGDOMAIN, "MINEXCLUSIVE: Invalid value (%lf<=%lf)",
                dv, xf->minExclusive);
      return -1;
    }
  }

  /* totalDigits */
  if ((xf->setMask & GWEN_XSD_FACET_TOTALDIGITS) &&
      (facetMask & GWEN_XSD_FACET_TOTALDIGITS)) {
    int dcount;
    const char *p;

    if (!haveDouble) {
      if (GWEN_Text_StringToDouble(GWEN_Buffer_GetStart(vbuf), &dv)) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Data is not a number (%s)",
                  GWEN_Buffer_GetStart(vbuf));
        return -1;
      }
      haveDouble=1;
    }

    dcount=0;
    p=GWEN_Buffer_GetStart(vbuf);
    while(*p) {
      if (isdigit(*p))
        dcount++;
      p++;
    }

    if (dcount>xf->totalDigits) {
      DBG_ERROR(GWEN_LOGDOMAIN, "TOTALDIGITS: Too many digits (%d>%d)",
                dcount, xf->totalDigits);
      return -1;
    }
  }

  /* fractionDigits */
  if ((xf->setMask & GWEN_XSD_FACET_FRACTIONDIGITS) &&
      (facetMask & GWEN_XSD_FACET_FRACTIONDIGITS)) {
    int dcount;
    const char *p;

    if (!haveDouble) {
      if (GWEN_Text_StringToDouble(GWEN_Buffer_GetStart(vbuf), &dv)) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Data is not a number (%s)",
                  GWEN_Buffer_GetStart(vbuf));
        return -1;
      }
      haveDouble=1;
    }

    dcount=0;
    p=strchr(GWEN_Buffer_GetStart(vbuf), '.');
    if (p) {
      while(*(++p)) {
        if (isdigit(*p))
          dcount++;
      }
    }

    if (dcount>xf->fractionDigits) {
      DBG_ERROR(GWEN_LOGDOMAIN, "FRACTIONDIGITS: Too many digits (%d>%d)",
                dcount, xf->fractionDigits);
      return -1;
    }
  }

  /* enumeration */
  if ((xf->setMask & GWEN_XSD_FACET_ENUMERATION) &&
      (facetMask & GWEN_XSD_FACET_ENUMERATION)) {
    GWEN_STRINGLISTENTRY *se;
    const char *p;

    p=GWEN_Buffer_GetStart(vbuf);
    se=GWEN_StringList_FirstEntry(xf->enumeration);
    assert(se);
    while(se) {
      const char *dp;

      dp=GWEN_StringListEntry_Data(se);
      assert(dp);
      if (strcasecmp(dp, p)==0)
        break;
      se=GWEN_StringListEntry_Next(se);
    } /* while */
    if (!se) {
      DBG_ERROR(GWEN_LOGDOMAIN, "ENUMERATION: Value does not match enum");
      return -1;
    }
  }

  /* pattern */
  if ((xf->setMask & GWEN_XSD_FACET_PATTERN) &&
      (facetMask & GWEN_XSD_FACET_PATTERN)) {
#ifdef HAVE_REGEX_H
    GWEN_STRINGLISTENTRY *se;
    const char *p;

    p=GWEN_Buffer_GetStart(vbuf);
    se=GWEN_StringList_FirstEntry(xf->pattern);
    assert(se);
    while(se) {
      const char *dp;
      regex_t preg;
      int regres;

      dp=GWEN_StringListEntry_Data(se);
      assert(dp);

      regres=regcomp(&preg, dp, REG_EXTENDED);
      if (regres) {
        char errBuf[256];

        regerror(regres, &preg, errBuf, sizeof(errBuf));
        DBG_ERROR(GWEN_LOGDOMAIN, "regcomp(%s): %s", dp,
                  errBuf);
        regfree(&preg);
        return -1;
      }
      regres=regexec(&preg, p, 0, 0, 0);
      if (regres) {
        char errBuf[256];

        regerror(regres, &preg, errBuf, sizeof(errBuf));
        DBG_ERROR(GWEN_LOGDOMAIN, "regexec(%s): %s", dp,
                  errBuf);
        regfree(&preg);
        return -1;
      }

      se=GWEN_StringListEntry_Next(se);
    } /* while */
    if (se) {
      return -1;
    }
#endif
  }

  return 0;
}



int GWEN_XSD__GetBaseValue(GWEN_XSD_ENGINE *e,
                           GWEN_DB_NODE *dbNode,
                           const char *name,
                           int idx,
                           GWEN_BUFFER *vbuf,
                           GWEN_DB_VALUETYPE *vtPtr) {
  const void *p;
  unsigned int len;
  GWEN_DB_VALUETYPE vt;
  char numbuf[32];
  const char *x;

  DBG_NOTICE(GWEN_LOGDOMAIN, "Reading value \"%s[%d]\"", name, idx);
  x=strchr(name,':');
  if (x)
    x++;
  else
    x=name;
  if (!GWEN_DB_ValueExists(dbNode, x, idx)) {
    DBG_INFO(GWEN_LOGDOMAIN, "Value \"%s[%d]\" does not exist",
             x, idx);
    return 1;
  }
  vt=GWEN_DB_GetValueTypeByPath(dbNode, x, idx);
  switch(vt) {
  case GWEN_DB_VALUETYPE_UNKNOWN:
    DBG_ERROR(GWEN_LOGDOMAIN, "Unknown value type");
    return -1;
  case GWEN_DB_VALUETYPE_CHAR:
    p=GWEN_DB_GetCharValue(dbNode, x, idx, 0);
    len=p?strlen(p):0;
    break;
  case GWEN_DB_VALUETYPE_INT:
    snprintf(numbuf, sizeof(numbuf)-1, "%d",
             GWEN_DB_GetIntValue(dbNode, x, idx, 0));
    numbuf[sizeof(numbuf)-1]=0;
    p=numbuf;
    len=strlen(p);
    break;
  case GWEN_DB_VALUETYPE_BIN:
    p=GWEN_DB_GetBinValue(dbNode, x, idx, 0, 0, &len);
    break;
  default:
    DBG_ERROR(GWEN_LOGDOMAIN, "Unsupported value type %d", vt);
    return -1;
  }

  *vtPtr=vt;
  if (p && len)
    GWEN_Buffer_AppendBytes(vbuf, p, len);
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "Empty value for \"%s[%d]\"",
             x, idx);
  }

  return 0;
}



int GWEN_XSD__GetElementData(GWEN_XSD_ENGINE *e,
                             GWEN_DB_NODE *dbNode,
                             const char *name,
                             int idx,
                             GWEN_XSD_FACETS *xf,
                             GWEN_BUFFER *dbuf) {
  char *nsName;
  GWEN_XSD_NAMESPACE *ns;
  const char *p;
  const char *typeName;
  GWEN_TYPE_UINT32 facetMask;
  GWEN_DB_VALUETYPE vt;
  int rv;
  GWEN_BUFFER *vbuf;
  int retrieved;

  typeName=xf->baseType;
  if (!typeName) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "No base name for element data type (%s)",
              name);
  }
  p=strchr(typeName, ':');
  assert(p);

  nsName=(char*)malloc(p-typeName+1);
  assert(nsName);
  memmove(nsName, typeName, p-typeName);
  nsName[p-typeName]=0;
  ns=GWEN_XSD__FindNameSpaceById(e, nsName);
  if (!ns) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Namespace \"%s\" not found", nsName);
    free(nsName);
    return -1;
  }
  free(nsName);
  if (strcasecmp(ns->name, GWEN_XSD_SCHEMA_NAME)==0)
    typeName=p+1;

  vbuf=GWEN_Buffer_new(0, 256, 0, 1);
  facetMask=0;
  rv=0;
  retrieved=0;

  /* select facet mask, add settings to FACET etc */
  if (strcasecmp(typeName, "string")==0) {
    facetMask=GWEN_XSD_FACETS_STRING;
    rv=GWEN_XSD__GetBaseValue(e, dbNode, name, idx, vbuf, &vt);
    if (rv) {
      DBG_INFO(GWEN_LOGDOMAIN, "here");
      GWEN_Buffer_free(vbuf);
      return rv;
    }
    retrieved=1;
  }
  else if (strcasecmp(typeName, "boolean")==0) {
    facetMask=GWEN_XSD_FACETS_BOOLEAN;
    rv=GWEN_XSD__GetBaseValue(e, dbNode, name, idx, vbuf, &vt);
    if (rv) {
      DBG_INFO(GWEN_LOGDOMAIN, "here");
      GWEN_Buffer_free(vbuf);
      return rv;
    }
    retrieved=1;
  }
  else if (strcasecmp(typeName, "decimal")==0) {
    facetMask=GWEN_XSD_FACETS_DECIMAL;
  }
  else if (strcasecmp(typeName, "float")==0){
    facetMask=GWEN_XSD_FACETS_FLOAT;
  }
  else if (strcasecmp(typeName, "double")==0){
    facetMask=GWEN_XSD_FACETS_DOUBLE;
  }
  else if (strcasecmp(typeName, "duration")==0){
    facetMask=GWEN_XSD_FACETS_DURATION;
  }
  else if (strcasecmp(typeName, "dateTime")==0){
    facetMask=GWEN_XSD_FACETS_DATETIME;
  }
  else if (strcasecmp(typeName, "time")==0){
    facetMask=GWEN_XSD_FACETS_TIME;
  }
  else if (strcasecmp(typeName, "date")==0){
    facetMask=GWEN_XSD_FACETS_DATE;
  }
  else if (strcasecmp(typeName, "gYearMonth")==0){
    facetMask=GWEN_XSD_FACETS_GYEARMONTH;
  }
  else if (strcasecmp(typeName, "gYear")==0){
    facetMask=GWEN_XSD_FACETS_GYEAR;
  }
  else if (strcasecmp(typeName, "gMonthDay")==0){
    facetMask=GWEN_XSD_FACETS_GMONTHDAY;
  }
  else if (strcasecmp(typeName, "gDay")==0){
    facetMask=GWEN_XSD_FACETS_GDAY;
  }
  else if (strcasecmp(typeName, "gMonth")==0){
    facetMask=GWEN_XSD_FACETS_GMONTH;
  }
  else if (strcasecmp(typeName, "hexBinary")==0){
    GWEN_BUFFER *tbuf;

    facetMask=GWEN_XSD_FACETS_HEXBINARY;
    rv=GWEN_XSD__GetBaseValue(e, dbNode, name, idx, vbuf, &vt);
    if (rv) {
      DBG_INFO(GWEN_LOGDOMAIN, "here");
      GWEN_Buffer_free(vbuf);
      return rv;
    }
    retrieved=1;
    /* convert to hex */
    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    if (GWEN_Text_ToHexBuffer(GWEN_Buffer_GetStart(vbuf),
                              GWEN_Buffer_GetUsedBytes(vbuf),
                              tbuf,
                              0, 0, 0)) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Could not convert binary to hexBinary");
      GWEN_Buffer_free(tbuf);
      GWEN_Buffer_free(vbuf);
      return -1;
    }
    GWEN_Buffer_free(vbuf);
    vbuf=tbuf;
  }
  else if (strcasecmp(typeName, "base64Binary")==0){
    GWEN_BUFFER *tbuf;

    facetMask=GWEN_XSD_FACETS_BASE64BINARY;
    rv=GWEN_XSD__GetBaseValue(e, dbNode, name, idx, vbuf, &vt);
    if (rv) {
      DBG_INFO(GWEN_LOGDOMAIN, "here");
      GWEN_Buffer_free(vbuf);
      return rv;
    }
    retrieved=1;
    /* convert to base64 */
    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    if (GWEN_Base64_Encode(GWEN_Buffer_GetStart(vbuf),
                           GWEN_Buffer_GetUsedBytes(vbuf),
                           tbuf, 0)) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Could not convert binary to hexBinary");
      GWEN_Buffer_free(tbuf);
      GWEN_Buffer_free(vbuf);
      return -1;
    }
    GWEN_Buffer_free(vbuf);
    vbuf=tbuf;
  }
  else if (strcasecmp(typeName, "anyURI")==0){
    facetMask=GWEN_XSD_FACETS_ANYURI;
  }
  else if (strcasecmp(typeName, "QName")==0){
    facetMask=GWEN_XSD_FACETS_QNAME;
  }
  else if (strcasecmp(typeName, "NOTATION")==0) {
    facetMask=GWEN_XSD_FACETS_NOTATION;
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Undefined simpleType \"%s\"", typeName);
    GWEN_Buffer_free(vbuf);
    return -1;
  }

  if (retrieved==0) {
    rv=GWEN_XSD__GetBaseValue(e, dbNode, name, idx, vbuf, &vt);
    if (rv) {
      if (rv==-1) {
        DBG_INFO(GWEN_LOGDOMAIN, "here");
      }
      GWEN_Buffer_free(vbuf);
      return rv;
    }
    retrieved=1;
  }

  /* validate data */
  rv=GWEN_XSD__ValidateElementData(e, facetMask, xf, vbuf);
  if (rv) {
    GWEN_Buffer_free(vbuf);
    return rv;
  }

  /* data is ok, copy it to the destination buffer */
  GWEN_Buffer_AppendBuffer(dbuf, vbuf);
  GWEN_Buffer_free(vbuf);
  return 0;
}



const char *GWEN_XSD__QualifyNameIfNecessary(GWEN_XSD_ENGINE *e,
					     GWEN_XMLNODE *nElement,
					     const char *name) {
  GWEN_XMLNODE *nc;
  int qualify;

  /* find out whether the current element needs to be qualified */
  qualify=0;
  nc=GWEN_XMLNode_GetParent(nElement);
  assert(nc);

  /* global elements/types are always qualified */
  if (strcasecmp(GWEN_XMLNode_GetData(nc), "file")==0) {
    DBG_NOTICE(GWEN_LOGDOMAIN,
	       "Element \"%s\" is global, needs to be qualified", name);
    qualify=1;
  }
  else {
    while(nc) {
      const char *s;

      s=GWEN_XMLNode_GetProperty(nc, "form", 0);
      if (!s)
	s=GWEN_XMLNode_GetProperty(nc, "elementFormDefault", 0);
      if (s) {
	if (strcasecmp(s, "qualified")==0) {
	  qualify=1;
	  break;
	}
	else if (strcasecmp(s, "unqualified")==0) {
	  qualify=0;
	  break;
	}
	else {
	  DBG_WARN(GWEN_LOGDOMAIN,
		   "Invalid value of property form/defaultElementForm (%s)",
		   s);
	  break;
	}
      }
      nc=GWEN_XMLNode_GetParent(nc);
    } /* while */
  } /* if not global */

  if (qualify==0) {
    DBG_NOTICE(GWEN_LOGDOMAIN,
	       "Element \"%s\" does not need to be qualified", name);
    name=strchr(name, ':');
    assert(name);
    name++;
  }

  return name;
}



GWEN_XMLNODE *GWEN_XSD__CreateXmlNodeInNameSpace(GWEN_XSD_ENGINE *e,
                                                 const char *name) {
  GWEN_XMLNODE *newNode;
  const char *sName;
  const char *nsName;

  /* remove target namespace from name (if match) */
  nsName=GWEN_XSD_GetCurrentTargetNameSpace(e);
  sName=strchr(name, ':');
  if (sName) {
    if (nsName) {
      if (strncasecmp(nsName, name, sName-name)==0)
	sName++;
      else
	sName=name;
    }
    else {
      sName=name;
    }
  }
  else
    sName=name;
  newNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, sName);

  return newNode;
}




int GWEN_XSD__WriteElementType(GWEN_XSD_ENGINE *e,
                               GWEN_XMLNODE *nElement,
                               GWEN_XMLNODE *nType,
                               GWEN_DB_NODE *dbNode,
                               const char *name,
                               int idx,
                               GWEN_XMLNODE *nStore) {
  GWEN_XMLNODE *nc;
  int isSimple;
  const char *tName;
  const char *eName;
  const char *eType;

  assert(nElement);
  assert(nType);
  tName=GWEN_XMLNode_GetData(nType);
  assert(tName);

  eName=GWEN_XMLNode_GetProperty(nElement, "name", 0);
  eType=GWEN_XMLNode_GetProperty(nElement, "type", 0);

  isSimple=0;
  if (strcasecmp(tName, "complexType")==0) {
    nc=GWEN_XMLNode_FindFirstTag(nType, "simpleContent", 0, 0);
    if (nc) {
      nType=nc;
      isSimple=1;
    }
    else {
      isSimple=-1;

      nc=GWEN_XMLNode_FindFirstTag(nType, "complexContent", 0, 0);
      if (nc)
	nType=nc;
    }
  }
  else if (strcasecmp(tName, "simpleType")==0) {
    isSimple=1;

    nc=GWEN_XMLNode_FindFirstTag(nType, "simpleContent", 0, 0);
    if (nc) {
      nType=nc;
    }
  }
  else {
    if (strcasecmp(GWEN_XMLNode_GetProperty(nType, "abstract", "false"),
		   "true")!=0) {
      GWEN_BUFFER *xpath;

      xpath=GWEN_Buffer_new(0, 256, 0, 1);
      GWEN_XMLNode_GetXPath(e->rootNode, nType, xpath);
      DBG_ERROR(GWEN_LOGDOMAIN, "Undeclared type in \"%s\"",
		GWEN_Buffer_GetStart(xpath));
      GWEN_Buffer_free(xpath);
      return -1;
    }
  }

  if (isSimple==1) {
    int rv;
    GWEN_XSD_FACETS *xf;
    GWEN_BUFFER *dbuf;

    /* simple type */
    xf=GWEN_XSD_Facets_new();
    rv=GWEN_XSD__GetTypeFacets(e, nType, xf);
    if (rv) {
      GWEN_XSD_Facets_free(xf);
      DBG_INFO(GWEN_LOGDOMAIN, "here");
      return rv;
    }

    /* get data */
    dbuf=GWEN_Buffer_new(0, 256, 0, 1);
    rv=GWEN_XSD__GetElementData(e, dbNode, name, idx, xf, dbuf);
    if (rv) {
      GWEN_Buffer_free(dbuf);
      GWEN_XSD_Facets_free(xf);
      return rv;
    }
    else {
      GWEN_XMLNODE *newNode;
      GWEN_XMLNODE *newDNode;
      const char *xmlName;

      /* (un)qualify name in regard to its context */
      xmlName=GWEN_XSD__QualifyNameIfNecessary(e, nElement, name);
      assert(xmlName);

      newNode=GWEN_XSD__CreateXmlNodeInNameSpace(e, xmlName);
      assert(newNode);

      newDNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeData,
                                GWEN_Buffer_GetStart(dbuf));
      GWEN_XMLNode_AddChild(newNode, newDNode);
      GWEN_XMLNode_AddChild(nStore, newNode);
      GWEN_Buffer_free(dbuf);
      GWEN_XSD_Facets_free(xf);
    }
  }
  else if (isSimple==-1) {
    GWEN_XMLNODE *n;
    int rv;
    GWEN_XMLNODE *newNode;
    int nodeCreated;

    /* complex type */
    n=GWEN_XMLNode_FindFirstTag(nType, "extension", 0, 0);
    if (!n)
      n=GWEN_XMLNode_FindFirstTag(nType, "restriction", 0, 0);

    /* create new XML nodes, add them */
    /* TODO: handle namespaces */
    nodeCreated=0;
    if (name) {
      const char *xmlName;

      /* (un)qualify name in regard to its context */
      xmlName=GWEN_XSD__QualifyNameIfNecessary(e, nElement, name);
      assert(xmlName);
      newNode=GWEN_XSD__CreateXmlNodeInNameSpace(e, xmlName);
      nodeCreated=1;
    }
    else
      newNode=nStore;
    rv=GWEN_XSD__WriteNodes(e, n?n:nType, dbNode, newNode);
    if (rv) {
      DBG_INFO(GWEN_LOGDOMAIN, "here");
      if (nodeCreated)
        GWEN_XMLNode_free(newNode);
      return rv;
    }
    if (nodeCreated)
      GWEN_XMLNode_AddChild(nStore, newNode);
  } /* if isSimple==-1 */

  return 0;
}



int GWEN_XSD__WriteSequence(GWEN_XSD_ENGINE *e,
                            GWEN_XMLNODE *n,
                            GWEN_DB_NODE *dbNode,
                            GWEN_XMLNODE *nStore) {
  GWEN_XMLNODE *nn;

  nn=GWEN_XMLNode_GetFirstTag(n);
  while(nn) {
    const char *tagName;
    const char *tName;
    int minOccur;
    int maxOccur;
    const char *x;
    int idx;
    GWEN_XMLNODE *nNextStore;
    int nodeCreated;

    assert(n);
    nNextStore=nStore;
    nodeCreated=0;
    tName=GWEN_XMLNode_GetProperty(nn, "name", 0);
    x=GWEN_XMLNode_GetProperty(nn, "minOccurs", "1");
    if (1!=sscanf(x, "%i", &minOccur)) {
      if (strcasecmp(x, "unbounded")==0)
        minOccur=0;
      else {
        DBG_ERROR(GWEN_LOGDOMAIN, "Bad minOccurrs property");
        return -1;
      }
    }
    x=GWEN_XMLNode_GetProperty(n, "maxOccurs", "1");
    if (1!=sscanf(x, "%i", &maxOccur)) {
      if (strcasecmp(x, "unbounded")==0)
        maxOccur=0;
      else {
        DBG_ERROR(GWEN_LOGDOMAIN, "Bad maxOccurrs property");
        return -1;
      }
    }

    tagName=GWEN_XMLNode_GetData(nn);
    assert(tagName);

    idx=0;
    if (strcasecmp(tagName, "element")==0) {
      int rv;
      const char *xType;
      const char *xName;
      GWEN_DB_NODE *gnode;
      int isComplex;

      if (!tName)
        xName=GWEN_XMLNode_GetProperty(nn, "ref", 0);
      else
        xName=tName;
      x=strchr(xName, ':');
      if (x)
        xName=x+1;

      xType=GWEN_XMLNode_GetProperty(nn, "type", 0);
      isComplex=atoi(GWEN_XMLNode_GetProperty(nn, "_isComplex", "0"));

      if (isComplex && xName) {
        DBG_NOTICE(GWEN_LOGDOMAIN, "Selecting group \"%s\"", xName);
        gnode=GWEN_DB_FindFirstGroup(dbNode, xName);
        if (!gnode) {
          DBG_INFO(GWEN_LOGDOMAIN,
                   "DB group \"%s\" not found",
                   xName);
        }
      }
      else
        gnode=dbNode;

      if (gnode) {
        idx=0;
        for (;;) {
          if (maxOccur && idx>maxOccur)
            break;

          rv=GWEN_XSD__WriteElementTypes(e, nn, gnode, idx, nNextStore);
          if (rv) {
            if (rv==-1) {
              DBG_INFO(GWEN_LOGDOMAIN, "here");
              return rv;
            }
            else
              break;
          }
          idx++;
          if (isComplex && xName) {
            /* get next data group */
            DBG_NOTICE(GWEN_LOGDOMAIN, "Selecting next group \"%s\"",
                       xName);
            gnode=GWEN_DB_FindNextGroup(gnode, xName);
            if (!gnode) {
              DBG_INFO(GWEN_LOGDOMAIN,
                       "No next DB group \"%s\"",
                       xName);
              break;
            }
          } /* if xName && !xType */
        } /* for */
      } /* if gnode */
    } /* if element */

    else if (strcasecmp(tagName, "group")==0) {
      int rv;
      GWEN_DB_NODE *gnode;
      const char *xName;
      int isComplex;

      xName=tName;
      x=strchr(xName, ':');
      if (x)
        xName=++x;

      isComplex=atoi(GWEN_XMLNode_GetProperty(nn, "_isComplex", "0"));

      if (tName && isComplex) {
        x=strchr(tName, ':');
        assert(x);
        x++;
        nNextStore=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, x);
        if (1) {
          char debugbuf[256];
          snprintf(debugbuf, sizeof(debugbuf)-1,
                   "%s:%d:%s",
                   __FILE__, __LINE__, __FUNCTION__);
          GWEN_XMLNode_SetProperty(nNextStore, "_debug", debugbuf);
        }
        nodeCreated=1;
      }

      if (isComplex && xName) {
        DBG_NOTICE(GWEN_LOGDOMAIN, "Selecting group \"%s\"", xName);
        gnode=GWEN_DB_FindFirstGroup(dbNode, xName);
        if (!gnode) {
          DBG_INFO(GWEN_LOGDOMAIN,
                   "DB group \"%s\" not found",
                   xName);
        }
      }
      else
        gnode=dbNode;

      if (gnode) {
        idx=0;
        for (;;) {
          if (maxOccur && idx>maxOccur)
            break;

          rv=GWEN_XSD__WriteGroupTypes(e, nn, gnode, idx, nNextStore);
          if (rv) {
            if (rv==-1) {
              DBG_INFO(GWEN_LOGDOMAIN, "here");
              if (nodeCreated)
                GWEN_XMLNode_free(nNextStore);
              return rv;
            }
            else
              break;
          }
          idx++;
          if (x) {
            /* get next data group */
            DBG_NOTICE(GWEN_LOGDOMAIN, "Selecting next group \"%s\"",
                       x);
            gnode=GWEN_DB_FindNextGroup(gnode, x);
            if (!gnode) {
              DBG_INFO(GWEN_LOGDOMAIN,
                       "No next DB group \"%s\"",
                       x);
              break;
            }
          } /* if xName && !xType */
        } /* for */
      } /* if gnode */
    } /* if group */

    else if (strcasecmp(tagName, "sequence")==0) {
      int rv;

      rv=GWEN_XSD__WriteSequence(e, nn, dbNode, nStore);
      if (rv==0)
        break;
      if (rv==-1) {
        DBG_INFO(GWEN_LOGDOMAIN, "here");
        return rv;
      }
    }

    else if (strcasecmp(tagName, "choice")==0) {
      int rv;

      rv=GWEN_XSD__WriteChoice(e, nn, dbNode, nStore);
      if (rv==0)
        break;
      if (rv==-1) {
        DBG_INFO(GWEN_LOGDOMAIN, "here");
        return rv;
      }
    }
    else if (strcasecmp(tagName, "any")==0) {
      /* TODO: get data from variable "data" */
    }

    /* check occurrences */
    if (minOccur==0 && !idx) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "No occurrences of %s \"%s\", omitting it",
                tagName, tName);
      if (nodeCreated) {
        GWEN_XMLNode_free(nNextStore);
        nNextStore=0;
        nodeCreated=0;
      }
    }
    else {
      if (minOccur && idx<minOccur) {
        DBG_ERROR(GWEN_LOGDOMAIN,
                  "Too few occurrences of %s \"%s\" (%d<%d)",
                  tagName, tName, idx, minOccur);
        if (nodeCreated)
          GWEN_XMLNode_free(nNextStore);
        return -1;
      }
      if (maxOccur && idx>maxOccur) {
        DBG_ERROR(GWEN_LOGDOMAIN,
                  "Too many occurrences of %s \"%s\" (%d>%d)",
                  tagName, tName, idx, maxOccur);
        if (nodeCreated)
          GWEN_XMLNode_free(nNextStore);
        return -1;
      }
    }

    if (nodeCreated)
      GWEN_XMLNode_AddChild(nStore, nNextStore);

    nn=GWEN_XMLNode_GetNextTag(nn);
  }

  return 0;
}



int GWEN_XSD__WriteChoice(GWEN_XSD_ENGINE *e,
                          GWEN_XMLNODE *n,
                          GWEN_DB_NODE *dbNode,
                          GWEN_XMLNODE *nStore) {
  GWEN_XMLNODE *nn;
  int first;

  nn=GWEN_XMLNode_GetFirstTag(nn);
  first=1;
  while(nn) {
    const char *tagName;
    int minOccur;
    int maxOccur;
    const char *x;
  
    assert(n);
    x=GWEN_XMLNode_GetProperty(n, "minOccurs", "1");
    if (1!=sscanf(x, "%i", &minOccur)) {
      if (strcasecmp(x, "unbounded")==0)
        minOccur=0;
      else {
        DBG_ERROR(GWEN_LOGDOMAIN, "Bad minOccurrs property");
        return -1;
      }
    }
    x=GWEN_XMLNode_GetProperty(n, "maxOccurs", "1");
    if (1!=sscanf(x, "%i", &maxOccur)) {
      if (strcasecmp(x, "unbounded")==0)
        maxOccur=0;
      else {
        DBG_ERROR(GWEN_LOGDOMAIN, "Bad maxOccurrs property");
        return -1;
      }
    }

    tagName=GWEN_XMLNode_GetData(nn);
    assert(tagName);

    if (strcasecmp(tagName, "element")==0) {
      int rv;

      rv=GWEN_XSD__WriteElementTypes(e, nn, dbNode, 0 /* idx */, nStore);
      if (rv==0)
        break;
      if (rv==-1) {
        DBG_INFO(GWEN_LOGDOMAIN, "here");
        return rv;
      }
    }
    else if (strcasecmp(tagName, "group")==0) {
      int rv;

      rv=GWEN_XSD__WriteGroupTypes(e, nn, dbNode, 0 /*idx*/, nStore);
      if (rv==0)
        break;
      if (rv==-1) {
        DBG_INFO(GWEN_LOGDOMAIN, "here");
        return rv;
      }
    }
    else if (strcasecmp(tagName, "sequence")==0) {
      int rv;

      rv=GWEN_XSD__WriteSequence(e, nn, dbNode, nStore);
      if (rv==0)
        break;
      if (rv==-1) {
        DBG_INFO(GWEN_LOGDOMAIN, "here");
        return rv;
      }
    }
    else if (strcasecmp(tagName, "choice")==0) {
      int rv;

      rv=GWEN_XSD__WriteChoice(e, nn, dbNode, nStore);
      if (rv==0)
        break;
      if (rv==-1) {
        DBG_INFO(GWEN_LOGDOMAIN, "here");
        return rv;
      }
    }
    else if (strcasecmp(tagName, "any")==0) {
      /* TODO: get data from variable "data" */
    }
    nn=GWEN_XMLNode_GetNextTag(nn);
  } /* while nn */

  return 0;
}



int GWEN_XSD__WriteNodes(GWEN_XSD_ENGINE *e,
                         GWEN_XMLNODE *n,
                         GWEN_DB_NODE *dbNode,
                         GWEN_XMLNODE *nStore) {
  GWEN_XMLNODE *nn;
  int rv;

  nn=GWEN_XMLNode_FindFirstTag(n, "sequence", 0, 0);
  if (nn) {
    rv=GWEN_XSD__WriteSequence(e, nn, dbNode, nStore);
    if (rv)
      return rv;
  }
  else {
    nn=GWEN_XMLNode_FindFirstTag(n, "choice", 0, 0);
    if (nn) {
      rv=GWEN_XSD__WriteChoice(e, nn, dbNode, nStore);
      if (rv)
        return rv;
    }
  }

  return 0;
}



int GWEN_XSD__WriteElementTypes(GWEN_XSD_ENGINE *e,
                                GWEN_XMLNODE *nElement,
                                GWEN_DB_NODE *dbNode,
                                int idx,
                                GWEN_XMLNODE *nStore){
  const char *eName;
  const char *eType;
  GWEN_XMLNODE *nType;
  int rv;

  /* resolve references */
  for (;;) {
    const char *ref;

    ref=GWEN_XMLNode_GetProperty(nElement, "ref", 0);
    if (!ref)
      break;
    DBG_INFO(GWEN_LOGDOMAIN, "Resolving reference to \"%s\"", ref);
    nElement=GWEN_XSD_GetElementNode(e, ref);
    if (!nElement) {
      DBG_ERROR(GWEN_LOGDOMAIN,
		"Invalid reference to missing element \"%s\"",
		ref);
      return -1;
    }
  }

  eName=GWEN_XMLNode_GetProperty(nElement, "name", 0);
  eType=GWEN_XMLNode_GetProperty(nElement, "type", 0);
  DBG_ERROR(GWEN_LOGDOMAIN, "Tag is %s (%s)", eName, eType);
  if (eType) {
    nType=GWEN_XSD_GetTypeNode(e, eType);
    if (!nType) {
      /* TODO: check for internal type */
      DBG_ERROR(GWEN_LOGDOMAIN, "Unknown type \"%s\"", eType);
      return -1;
    }
  }
  else {
    /* anonymous type, defined inside */
    nType=GWEN_XMLNode_FindFirstTag(nElement, "complexType", 0, 0);
    if (!nType)
      nType=GWEN_XMLNode_FindFirstTag(nElement, "simpleType", 0, 0);
    if (!nType) {
      GWEN_BUFFER *xpath;

      xpath=GWEN_Buffer_new(0, 256, 0, 1);
      GWEN_XMLNode_GetXPath(e->rootNode, nElement, xpath);
      DBG_ERROR(GWEN_LOGDOMAIN, "Undeclared element in \"%s\"",
		GWEN_Buffer_GetStart(xpath));
      GWEN_Buffer_free(xpath);
      return -1;
    }
  }

  rv=GWEN_XSD__WriteElementType(e, nElement, nType,
                                dbNode, eName, idx,
                                nStore);
  if (rv)
    return rv;

  return 0;
}



int GWEN_XSD__WriteGroupTypes(GWEN_XSD_ENGINE *e,
                              GWEN_XMLNODE *nGroup,
                              GWEN_DB_NODE *dbNode,
                              int idx,
                              GWEN_XMLNODE *nStore){
  int rv;

  /* resolve references */
  for (;;) {
    const char *ref;

    ref=GWEN_XMLNode_GetProperty(nGroup, "ref", 0);
    if (!ref)
      break;
    DBG_INFO(GWEN_LOGDOMAIN, "Resolving reference to \"%s\"", ref);
    nGroup=GWEN_XSD_GetGroupNode(e, ref);
    if (!nGroup) {
      DBG_ERROR(GWEN_LOGDOMAIN,
		"Invalid reference to missing group \"%s\"",
		ref);
      return -1;
    }
  }

  rv=GWEN_XSD__WriteNodes(e, nGroup, dbNode, nStore);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    return rv;
  }

  return 0;
}



int GWEN_XSD_WriteElement(GWEN_XSD_ENGINE *e,
                          const char *nameSpace,
                          const char *name,
                          GWEN_DB_NODE *dbNode,
                          GWEN_XMLNODE *nStore,
                          int whistlesAndBells) {
  GWEN_XSD_NAMESPACE *ns;
  GWEN_BUFFER *nbuf;
  GWEN_XMLNODE *nElement;
  GWEN_XMLNODE *nLocalStore;
  int rv;

  ns=GWEN_XSD__FindNameSpaceByName(e, nameSpace);
  if (!ns) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Namespace \"%s\" not found", nameSpace);
    return -1;
  }

  nbuf=GWEN_Buffer_new(0, 32, 0, 1);
  GWEN_Buffer_AppendString(nbuf, ns->id);
  GWEN_Buffer_AppendString(nbuf, ":");
  GWEN_Buffer_AppendString(nbuf, name);

  nElement=GWEN_XSD_GetElementNode(e, GWEN_Buffer_GetStart(nbuf));
  if (!nElement) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Type \"%s\" not found",
              GWEN_Buffer_GetStart(nbuf));
    GWEN_Buffer_free(nbuf);
    return -1;
  }

  nLocalStore=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "localRoot");
  rv=GWEN_XSD__WriteElementTypes(e, nElement, dbNode, 0, nLocalStore);
  if (rv==0) {
    GWEN_XMLNODE *nNew;

    nNew=GWEN_XMLNode_GetFirstTag(nLocalStore);
    if (nNew) {
      GWEN_XSD_NAMESPACE *ns;
      GWEN_XMLNODE *nFile;

      nFile=nElement;
      while(nFile) {
        const char *s;

        s=GWEN_XMLNode_GetData(nFile);
        assert(s);
        if (strcasecmp(s, "file")==0)
          break;
        nFile=GWEN_XMLNode_GetParent(nFile);
      }

      if (whistlesAndBells) {
        if (e->currentTargetNameSpace) {
          ns=GWEN_XSD__FindNameSpaceById(e, e->currentTargetNameSpace);
          if (ns) {
            GWEN_XMLNode_SetProperty(nNew, "xmlns", ns->name);
          }
        }

        if (nFile) {
          GWEN_BUFFER *buf;
          const char *s;
          char *x;

          buf=GWEN_Buffer_new(0, 32, 0, 1);
          s=GWEN_XMLNode_GetProperty(nFile, "fileNameSpace", 0);
          assert(s);
          GWEN_Buffer_AppendString(buf, s);
          GWEN_Buffer_AppendString(buf, "/");
          s=GWEN_XMLNode_GetProperty(nFile, "fileName", 0);
          assert(s);
          GWEN_Buffer_AppendString(buf, s);
          /* double the buffer content (needs a temporary copy because
           * adding bytes to a GWEN_BUFFER might make it realloc()) */
          x=strdup(GWEN_Buffer_GetStart(buf));
          GWEN_Buffer_AppendString(buf, " ");
          GWEN_Buffer_AppendString(buf, x);
          free(x);
          GWEN_XMLNode_SetProperty(nNew, "xmlns:xsi",
                                   "http://www.w3.org/2001/XMLSchema-instance");
          GWEN_XMLNode_SetProperty(nNew,
                                   "xsi:schemaLocation",
                                   GWEN_Buffer_GetStart(buf));
          GWEN_Buffer_free(buf);
        }

        ns=GWEN_XSD_NameSpace_List_First(e->nameSpaces);
        while(ns) {
          GWEN_BUFFER *nsBuf;

          nsBuf=GWEN_Buffer_new(0, 32, 0, 1);
          GWEN_Buffer_AppendString(nsBuf, "xmlns:");
          GWEN_Buffer_AppendString(nsBuf, ns->id);
          GWEN_XMLNode_SetProperty(nNew, GWEN_Buffer_GetStart(nsBuf),
                                   ns->name);
          GWEN_Buffer_free(nsBuf);
          ns=GWEN_XSD_NameSpace_List_Next(ns);
        }
      } /* if whistlesAndBells */
    }
    GWEN_XMLNode_AddChildrenOnly(nStore, nLocalStore, 0);
    GWEN_XMLNode_free(nLocalStore);
  }

  GWEN_Buffer_free(nbuf);
  return rv;
}













