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



int GWEN_XSD__ListSequence(GWEN_XSD_ENGINE *e,
                           GWEN_XMLNODE *n,
                           GWEN_BUFFER *outBuffer,
                           int indent){
  GWEN_XMLNODE *nn;
  int i;

  nn=GWEN_XMLNode_GetFirstTag(n);
  while(nn) {
    const char *tagName;
    int minOccur;
    int maxOccur;
    const char *x;
    char numbuf[16];
  
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

    GWEN_Buffer_AppendString(outBuffer, "\n");
    for (i=0; i<indent; i++)
      GWEN_Buffer_AppendByte(outBuffer, ' ');
    GWEN_Buffer_AppendByte(outBuffer, toupper(*tagName));
    GWEN_Buffer_AppendString(outBuffer, tagName+1);
    GWEN_Buffer_AppendString(outBuffer, " ");

    /* min and max occurrences */
    if (minOccur==1 && maxOccur==1) {
      GWEN_Buffer_AppendString(outBuffer, "(mandatory)");
    }
    else if (minOccur==0) {
      if (maxOccur==0)
        GWEN_Buffer_AppendString(outBuffer, "(optional, unlimited)");
      else if (maxOccur==1)
        GWEN_Buffer_AppendString(outBuffer, "(optional)");
      else {
        GWEN_Buffer_AppendString(outBuffer, "(optional, up to ");
        snprintf(numbuf, sizeof(numbuf)-1, "%d)", maxOccur);
      }
    }
    else if (maxOccur==0) {
      if (minOccur==0)
        GWEN_Buffer_AppendString(outBuffer, "(optional, unlimited)");
      else if (minOccur==1)
        GWEN_Buffer_AppendString(outBuffer, "(mandatory, unlimited)");
      else {
        GWEN_Buffer_AppendString(outBuffer, "(unlimited, minimum ");
        snprintf(numbuf, sizeof(numbuf)-1, "%d)", minOccur);
      }
    }
    else {
      snprintf(numbuf, sizeof(numbuf)-1, "(%d", minOccur);
      GWEN_Buffer_AppendString(outBuffer, numbuf);
      GWEN_Buffer_AppendString(outBuffer, "-");
      snprintf(numbuf, sizeof(numbuf)-1, "%d)", maxOccur);
      GWEN_Buffer_AppendString(outBuffer, numbuf);
      GWEN_Buffer_AppendByte(outBuffer, ' ');
    }
    GWEN_Buffer_AppendString(outBuffer, "\n");

    if (strcasecmp(tagName, "element")==0) {
      int rv;

      rv=GWEN_XSD__ListElementTypes(e, nn, outBuffer, indent+2);
      if (rv) {
        DBG_INFO(GWEN_LOGDOMAIN, "here");
        return rv;
      }
    }
    else if (strcasecmp(tagName, "group")==0) {
      int rv;

      rv=GWEN_XSD__ListGroupTypes(e, nn, outBuffer, indent+2);
      if (rv) {
        DBG_INFO(GWEN_LOGDOMAIN, "here");
        return rv;
      }
    }
    else if (strcasecmp(tagName, "sequence")==0) {
      int rv;

      rv=GWEN_XSD__ListSequence(e, nn, outBuffer, indent+2);
      if (rv) {
        DBG_INFO(GWEN_LOGDOMAIN, "here");
        return rv;
      }
    }
    else if (strcasecmp(tagName, "choice")==0) {
      int rv;

      rv=GWEN_XSD__ListChoice(e, nn, outBuffer, indent+2);
      if (rv) {
        DBG_INFO(GWEN_LOGDOMAIN, "here");
        return rv;
      }
    }
    else if (strcasecmp(tagName, "any")==0) {
      for (i=0; i<indent+2; i++)
        GWEN_Buffer_AppendByte(outBuffer, ' ');
      GWEN_Buffer_AppendString(outBuffer, "Name : data\n");
      for (i=0; i<indent+2; i++)
        GWEN_Buffer_AppendByte(outBuffer, ' ');
      GWEN_Buffer_AppendString(outBuffer, "Type : unrestricted data\n");
    }
    nn=GWEN_XMLNode_GetNextTag(nn);
  }
  return 0;
}



int GWEN_XSD__ListChoice(GWEN_XSD_ENGINE *e,
                         GWEN_XMLNODE *n,
                         GWEN_BUFFER *outBuffer,
                         int indent){
  GWEN_XMLNODE *nn;
  int first;
  int i;

  nn=GWEN_XMLNode_GetFirstTag(n);
  first=1;
  while(nn) {
    const char *tagName;
    int minOccur;
    int maxOccur;
    const char *x;
    char numbuf[16];
  
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

    for (i=0; i<indent; i++)
      GWEN_Buffer_AppendByte(outBuffer, ' ');
    if (first) {
      GWEN_Buffer_AppendString(outBuffer, "Choice:\n");
      first=0;
    }
    else {
      GWEN_Buffer_AppendString(outBuffer, "or:\n");
    }

    for (i=0; i<indent+2; i++)
      GWEN_Buffer_AppendByte(outBuffer, ' ');
    GWEN_Buffer_AppendByte(outBuffer, toupper(*tagName));
    GWEN_Buffer_AppendString(outBuffer, tagName+1);
    GWEN_Buffer_AppendString(outBuffer, " ");

    /* min and max occurrences */
    if (minOccur==1 && maxOccur==1) {
      GWEN_Buffer_AppendString(outBuffer, "(mandatory)");
    }
    else if (minOccur==0) {
      if (maxOccur==0)
        GWEN_Buffer_AppendString(outBuffer, "(optional, unlimited)");
      else if (maxOccur==1)
        GWEN_Buffer_AppendString(outBuffer, "(optional)");
      else {
        GWEN_Buffer_AppendString(outBuffer, "(optional, up to ");
        snprintf(numbuf, sizeof(numbuf)-1, "%d)", maxOccur);
      }
    }
    else if (maxOccur==0) {
      if (minOccur==0)
        GWEN_Buffer_AppendString(outBuffer, "(optional, unlimited)");
      else if (minOccur==1)
        GWEN_Buffer_AppendString(outBuffer, "(mandatory, unlimited)");
      else {
        GWEN_Buffer_AppendString(outBuffer, "(unlimited, minimum ");
        snprintf(numbuf, sizeof(numbuf)-1, "%d)", minOccur);
      }
    }
    else {
      snprintf(numbuf, sizeof(numbuf)-1, "(%d", minOccur);
      GWEN_Buffer_AppendString(outBuffer, numbuf);
      GWEN_Buffer_AppendString(outBuffer, "-");
      snprintf(numbuf, sizeof(numbuf)-1, "%d)", maxOccur);
      GWEN_Buffer_AppendString(outBuffer, numbuf);
      GWEN_Buffer_AppendByte(outBuffer, ' ');
    }
    GWEN_Buffer_AppendString(outBuffer, "\n");

    if (strcasecmp(tagName, "element")==0) {
      int rv;

      rv=GWEN_XSD__ListElementTypes(e, nn, outBuffer, indent+4);
      if (rv) {
        DBG_INFO(GWEN_LOGDOMAIN, "here");
        return rv;
      }
    }
    else if (strcasecmp(tagName, "group")==0) {
      int rv;

      rv=GWEN_XSD__ListGroupTypes(e, nn, outBuffer, indent+4);
      if (rv) {
        DBG_INFO(GWEN_LOGDOMAIN, "here");
        return rv;
      }
    }
    else if (strcasecmp(tagName, "sequence")==0) {
      int rv;

      rv=GWEN_XSD__ListSequence(e, nn, outBuffer, indent+4);
      if (rv) {
        DBG_INFO(GWEN_LOGDOMAIN, "here");
        return rv;
      }
    }
    else if (strcasecmp(tagName, "choice")==0) {
      int rv;

      rv=GWEN_XSD__ListChoice(e, nn, outBuffer, indent+4);
      if (rv) {
        DBG_INFO(GWEN_LOGDOMAIN, "here");
        return rv;
      }
    }
    else if (strcasecmp(tagName, "any")==0) {
      for (i=0; i<indent+4; i++)
        GWEN_Buffer_AppendByte(outBuffer, ' ');
      GWEN_Buffer_AppendString(outBuffer, "Name : data\n");
      for (i=0; i<indent+4; i++)
        GWEN_Buffer_AppendByte(outBuffer, ' ');
      GWEN_Buffer_AppendString(outBuffer, "Type : unrestricted data\n");
    }
    nn=GWEN_XMLNode_GetNextTag(nn);
  } /* while nn */
  return 0;
}



int GWEN_XSD__ListNodes(GWEN_XSD_ENGINE *e,
			GWEN_XMLNODE *n,
			GWEN_BUFFER *outBuffer,
			int indent){
  GWEN_XMLNODE *nn;
  int rv;

  nn=GWEN_XMLNode_FindFirstTag(n, "sequence", 0, 0);
  if (nn) {
    rv=GWEN_XSD__ListSequence(e, nn, outBuffer, indent);
    if (rv)
      return rv;
  }
  else {
    nn=GWEN_XMLNode_FindFirstTag(n, "choice", 0, 0);
    if (nn) {
      rv=GWEN_XSD__ListChoice(e, nn, outBuffer, indent);
      if (rv)
        return rv;
    }
  }

  return 0;
}




int GWEN_XSD__ListElementType(GWEN_XSD_ENGINE *e,
			      GWEN_XMLNODE *nElement,
			      GWEN_XMLNODE *nType,
			      const char *name,
			      GWEN_BUFFER *outBuffer,
			      int indent){
  GWEN_XMLNODE *nc;
  int isSimple;
  const char *tName;

  assert(nElement);
  assert(nType);
  tName=GWEN_XMLNode_GetData(nType);
  assert(tName);

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
    int i;
    int rv;
    GWEN_XSD_FACETS *xf;
    const char *p;

    /* simple type */
    xf=GWEN_XSD_Facets_new();
    rv=GWEN_XSD__GetTypeFacets(e, nType, xf);
    if (rv) {
      GWEN_XSD_Facets_free(xf);
      DBG_INFO(GWEN_LOGDOMAIN, "here");
      return rv;
    }
    for (i=0; i<indent; i++)
      GWEN_Buffer_AppendByte(outBuffer, ' ');
    GWEN_Buffer_AppendString(outBuffer, "Name : ");
    /* name */
    p=strchr(name, ':');
    if (p)
      p++;
    else
      p=name;
    GWEN_Buffer_AppendString(outBuffer, p);
    GWEN_Buffer_AppendString(outBuffer, "\n");
    /* base type */
    if (xf->baseType) {
      for (i=0; i<indent; i++)
	GWEN_Buffer_AppendByte(outBuffer, ' ');
      GWEN_Buffer_AppendString(outBuffer, "Type : ");
      p=strchr(xf->baseType, ':');
      if (p)
	p++;
      else
	p=xf->baseType;
      GWEN_Buffer_AppendByte(outBuffer, '\"');
      GWEN_Buffer_AppendString(outBuffer, p);
      GWEN_Buffer_AppendByte(outBuffer, '\"');
    }
    else
      GWEN_Buffer_AppendString(outBuffer, "(unknown)");
    GWEN_Buffer_AppendByte(outBuffer, ' ');

    GWEN_Buffer_AppendString(outBuffer, "\n");

    /* facets */
    for (i=0; i<indent; i++)
      GWEN_Buffer_AppendByte(outBuffer, ' ');
    GWEN_Buffer_AppendString(outBuffer, "Restrictions:\n");

    /* length */
    if (xf->length) {
      char numbuf[16];

      for (i=0; i<indent+2; i++)
	GWEN_Buffer_AppendByte(outBuffer, ' ');
      GWEN_Buffer_AppendString(outBuffer, "Length: ");
      snprintf(numbuf, sizeof(numbuf)-1, "%d", xf->length);
      GWEN_Buffer_AppendString(outBuffer, numbuf);
      GWEN_Buffer_AppendString(outBuffer, "\n");
    }

    /* minLength */
    if (xf->minLength) {
      char numbuf[16];

      for (i=0; i<indent+4; i++)
	GWEN_Buffer_AppendByte(outBuffer, ' ');
      GWEN_Buffer_AppendString(outBuffer, "MinLength: ");
      snprintf(numbuf, sizeof(numbuf)-1, "%d", xf->minLength);
      GWEN_Buffer_AppendString(outBuffer, numbuf);
      GWEN_Buffer_AppendString(outBuffer, "\n");
    }

    /* maxLength */
    if (xf->maxLength) {
      char numbuf[16];

      for (i=0; i<indent+4; i++)
	GWEN_Buffer_AppendByte(outBuffer, ' ');
      GWEN_Buffer_AppendString(outBuffer, "MinLength: ");
      snprintf(numbuf, sizeof(numbuf)-1, "%d", xf->maxLength);
      GWEN_Buffer_AppendString(outBuffer, numbuf);
      GWEN_Buffer_AppendString(outBuffer, "\n");
    }

    /* maxInclusive */
    if (xf->maxInclusive) {
      char numbuf[16];

      for (i=0; i<indent+4; i++)
	GWEN_Buffer_AppendByte(outBuffer, ' ');
      GWEN_Buffer_AppendString(outBuffer, "MaxInclusive: ");
      snprintf(numbuf, sizeof(numbuf)-1, "%lf", xf->maxInclusive);
      GWEN_Buffer_AppendString(outBuffer, numbuf);
      GWEN_Buffer_AppendString(outBuffer, "\n");
    }

    /* maxExclusive */
    if (xf->maxExclusive) {
      char numbuf[16];

      for (i=0; i<indent+4; i++)
	GWEN_Buffer_AppendByte(outBuffer, ' ');
      GWEN_Buffer_AppendString(outBuffer, "MaxExclusive: ");
      snprintf(numbuf, sizeof(numbuf)-1, "%lf", xf->maxExclusive);
      GWEN_Buffer_AppendString(outBuffer, numbuf);
      GWEN_Buffer_AppendString(outBuffer, "\n");
    }

    /* minInclusive */
    if (xf->minInclusive) {
      char numbuf[16];

      for (i=0; i<indent+4; i++)
	GWEN_Buffer_AppendByte(outBuffer, ' ');
      GWEN_Buffer_AppendString(outBuffer, "MinInclusive: ");
      snprintf(numbuf, sizeof(numbuf)-1, "%lf", xf->minInclusive);
      GWEN_Buffer_AppendString(outBuffer, numbuf);
      GWEN_Buffer_AppendString(outBuffer, "\n");
    }

    /* minExclusive */
    if (xf->minExclusive) {
      char numbuf[16];

      for (i=0; i<indent+4; i++)
	GWEN_Buffer_AppendByte(outBuffer, ' ');
      GWEN_Buffer_AppendString(outBuffer, "MinExclusive: ");
      snprintf(numbuf, sizeof(numbuf)-1, "%lf", xf->minExclusive);
      GWEN_Buffer_AppendString(outBuffer, numbuf);
      GWEN_Buffer_AppendString(outBuffer, "\n");
    }

    /* totalDigits */
    if (xf->totalDigits) {
      char numbuf[16];

      for (i=0; i<indent+4; i++)
	GWEN_Buffer_AppendByte(outBuffer, ' ');
      GWEN_Buffer_AppendString(outBuffer, "TotalDigits: ");
      snprintf(numbuf, sizeof(numbuf)-1, "%d", xf->totalDigits);
      GWEN_Buffer_AppendString(outBuffer, numbuf);
      GWEN_Buffer_AppendString(outBuffer, "\n");
    }

    /* fractionDigits */
    if (xf->fractionDigits) {
      char numbuf[16];

      for (i=0; i<indent+4; i++)
	GWEN_Buffer_AppendByte(outBuffer, ' ');
      GWEN_Buffer_AppendString(outBuffer, "FractionDigits: ");
      snprintf(numbuf, sizeof(numbuf)-1, "%d", xf->fractionDigits);
      GWEN_Buffer_AppendString(outBuffer, numbuf);
      GWEN_Buffer_AppendString(outBuffer, "\n");
    }

    /* enumeration */
    if (GWEN_StringList_Count(xf->enumeration)) {
      GWEN_STRINGLISTENTRY *se;

      for (i=0; i<indent+4; i++)
	GWEN_Buffer_AppendByte(outBuffer, ' ');
      GWEN_Buffer_AppendString(outBuffer, "Valid values:\n");
      se=GWEN_StringList_FirstEntry(xf->enumeration);
      assert(se);
      while(se) {
	const char *el;

	el=GWEN_StringListEntry_Data(se);
	assert(el);
	for (i=0; i<indent+6; i++)
	  GWEN_Buffer_AppendByte(outBuffer, ' ');
	GWEN_Buffer_AppendString(outBuffer, el);
	GWEN_Buffer_AppendString(outBuffer, "\n");
        se=GWEN_StringListEntry_Next(se);
      } /* while se */
    } /* if enumerations */

    /* pattern */
    if (GWEN_StringList_Count(xf->pattern)) {
      GWEN_STRINGLISTENTRY *se;

      for (i=0; i<indent+4; i++)
	GWEN_Buffer_AppendByte(outBuffer, ' ');
      GWEN_Buffer_AppendString(outBuffer, "Valid patterns:\n");
      se=GWEN_StringList_FirstEntry(xf->pattern);
      assert(se);
      while(se) {
	const char *el;

	el=GWEN_StringListEntry_Data(se);
	assert(el);
	for (i=0; i<indent+6; i++)
	  GWEN_Buffer_AppendByte(outBuffer, ' ');
	GWEN_Buffer_AppendByte(outBuffer, '\"');
	GWEN_Buffer_AppendString(outBuffer, el);
	GWEN_Buffer_AppendByte(outBuffer, '\"');
	GWEN_Buffer_AppendString(outBuffer, "\n");
        se=GWEN_StringListEntry_Next(se);
      } /* while se */
    } /* if enumerations */

    GWEN_XSD_Facets_free(xf);
  }
  else if (isSimple==-1) {
    int i;
    GWEN_XMLNODE *n;
    const char *p;
    int rv;

    /* complex type */
    /* name */
    for (i=0; i<indent; i++)
      GWEN_Buffer_AppendByte(outBuffer, ' ');
    GWEN_Buffer_AppendString(outBuffer, "Name : ");
    p=strchr(name, ':');
    if (p)
      p++;
    else
      p=name;
    GWEN_Buffer_AppendString(outBuffer, p);
    GWEN_Buffer_AppendString(outBuffer, "/ \n");

    n=GWEN_XMLNode_FindFirstTag(nType, "extension", 0, 0);
    if (!n)
      n=GWEN_XMLNode_FindFirstTag(nType, "restriction", 0, 0);

    rv=GWEN_XSD__ListNodes(e, n?n:nType, outBuffer, indent);
    if (rv) {
      DBG_INFO(GWEN_LOGDOMAIN, "here");
      return rv;
    }
  } /* if isSimple==-1 */

  return 0;
}



int GWEN_XSD__ListElementTypes(GWEN_XSD_ENGINE *e,
			       GWEN_XMLNODE *nElement,
			       GWEN_BUFFER *outBuffer,
			       int indent){
  const char *eName;
  const char *eType;
  GWEN_XMLNODE *nType;
  int rv;


  eName=GWEN_XMLNode_GetProperty(nElement, "name", 0);
  if (!eName)
    eName=GWEN_XMLNode_GetProperty(nElement, "ref", 0);

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

  rv=GWEN_XSD__ListElementType(e, nElement, nType, eName, outBuffer, indent);
  if (rv)
    return rv;

  return 0;
}



int GWEN_XSD__ListGroupTypes(GWEN_XSD_ENGINE *e,
			     GWEN_XMLNODE *nGroup,
			     GWEN_BUFFER *outBuffer,
			     int indent){
  int rv;
  const char *eName;

  eName=GWEN_XMLNode_GetProperty(nGroup, "name", 0);
  if (eName) {
    int i;
    const char *p;

    for (i=0; i<indent; i++)
      GWEN_Buffer_AppendByte(outBuffer, ' ');
    GWEN_Buffer_AppendString(outBuffer, "Name : ");
    /* name */
    p=strchr(eName, ':');
    if (p)
      p++;
    else
      p=eName;
    GWEN_Buffer_AppendString(outBuffer, p);
    GWEN_Buffer_AppendString(outBuffer, "\n");
  }

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

  rv=GWEN_XSD__ListNodes(e, nGroup, outBuffer, indent);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    return rv;
  }

  return 0;
}



int GWEN_XSD__ListTypes(GWEN_XSD_ENGINE *e,
			const char *name,
			GWEN_BUFFER *outBuffer,
			int indent){
  GWEN_XMLNODE *nElement;

  nElement=GWEN_XSD_GetElementNode(e, name);
  if (!nElement) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Type \"%s\" not found", name);
    return -1;
  }
  return GWEN_XSD__ListElementTypes(e, nElement, outBuffer, indent);
}



int GWEN_XSD_ListTypes(GWEN_XSD_ENGINE *e,
		       const char *nameSpace,
		       const char *name,
		       GWEN_BUFFER *outBuffer) {
  GWEN_XSD_NAMESPACE *ns;
  GWEN_BUFFER *nbuf;
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

  rv=GWEN_XSD__ListTypes(e, GWEN_Buffer_GetStart(nbuf), outBuffer, 0);
  GWEN_Buffer_free(nbuf);
  return rv;
}


