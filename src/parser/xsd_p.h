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

#ifndef GWENHYWFAR_XSD_P_H
#define GWENHYWFAR_XSD_P_H

#include <gwenhywfar/xsd.h>
#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/bufferedio.h>
#include <gwenhywfar/stringlist2.h>
#include <gwenhywfar/types.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/xml.h>
#include <gwenhywfar/misc.h>
#include <stdio.h>

#define GWEN_XSD_SCHEMA_NAME "http://www.w3.org/2001/XMLSchema"


typedef struct GWEN_XSD_NAMESPACE GWEN_XSD_NAMESPACE;
typedef struct GWEN_XSD_CONTEXT GWEN_XSD_CONTEXT;
typedef struct GWEN_XSD_FACETS GWEN_XSD_FACETS;

GWEN_LIST_FUNCTION_DEFS(GWEN_XSD_NAMESPACE, GWEN_XSD_NameSpace)
GWEN_LIST_FUNCTION_DEFS(GWEN_XSD_FACETS, GWEN_XSD_Facets)


struct GWEN_XSD_NAMESPACE {
  GWEN_LIST_ELEMENT(GWEN_XSD_NAMESPACE)
  char *id;
  char *outId;
  char *name;
  char *url;
  char *localUrl;
};


struct GWEN_XSD_CONTEXT {
  GWEN_DB_NODE *dbNode;
  GWEN_XMLNODE_LIST2 *pathNodes;
  GWEN_XSD_NAMESPACE_LIST *nameSpaces;
  GWEN_XMLNODE *docRootNode;
};


typedef enum {
  GWEN_XSD_FacetWm_Preserve, /* default */
  GWEN_XSD_FacetWm_Replace,  /* replace all #x9, #xA, #xD with #x20 */
  GWEN_XSD_FacetWm_Collapse  /* removing leading/trailing/unnecessary */
} GWEN_XSD_FACET_WHITESPACE_MODE;


#define GWEN_XSD_FACET_LENGTH            0x00000001
#define GWEN_XSD_FACET_MINLENGTH         0x00000002
#define GWEN_XSD_FACET_MAXLENGTH         0x00000004
#define GWEN_XSD_FACET_WHITESPACE        0x00000008
#define GWEN_XSD_FACET_MAXINCLUSIVE      0x00000010
#define GWEN_XSD_FACET_MAXEXCLUSIVE      0x00000020
#define GWEN_XSD_FACET_MININCLUSIVE      0x00000040
#define GWEN_XSD_FACET_MINEXCLUSIVE      0x00000080
#define GWEN_XSD_FACET_TOTALDIGITS       0x00000100
#define GWEN_XSD_FACET_FRACTIONDIGITS    0x00000200
#define GWEN_XSD_FACET_ENUMERATION       0x00000400
#define GWEN_XSD_FACET_PATTERN           0x00000800


#define GWEN_XSD_FACETS_STRING (\
  GWEN_XSD_FACET_LENGTH |\
  GWEN_XSD_FACET_MINLENGTH  |\
  GWEN_XSD_FACET_MAXLENGTH |\
  GWEN_XSD_FACET_WHITESPACE)

#define GWEN_XSD_FACETS_BOOLEAN (\
  GWEN_XSD_FACET_PATTERN )

#define GWEN_XSD_FACETS_DECIMAL (\
  GWEN_XSD_FACET_MAXINCLUSIVE |\
  GWEN_XSD_FACET_MAXEXCLUSIVE |\
  GWEN_XSD_FACET_MININCLUSIVE |\
  GWEN_XSD_FACET_MINEXCLUSIVE |\
  GWEN_XSD_FACET_TOTALDIGITS |\
  GWEN_XSD_FACET_FRACTIONDIGITS |\
  GWEN_XSD_FACET_ENUMERATION |\
  GWEN_XSD_FACET_PATTERN )

#define GWEN_XSD_FACETS_FLOAT (\
  GWEN_XSD_FACET_MAXINCLUSIVE |\
  GWEN_XSD_FACET_MAXEXCLUSIVE |\
  GWEN_XSD_FACET_MININCLUSIVE |\
  GWEN_XSD_FACET_MINEXCLUSIVE |\
  GWEN_XSD_FACET_ENUMERATION |\
  GWEN_XSD_FACET_PATTERN )

#define GWEN_XSD_FACETS_DOUBLE (\
  GWEN_XSD_FACET_MAXINCLUSIVE |\
  GWEN_XSD_FACET_MAXEXCLUSIVE |\
  GWEN_XSD_FACET_MININCLUSIVE |\
  GWEN_XSD_FACET_MINEXCLUSIVE |\
  GWEN_XSD_FACET_ENUMERATION |\
  GWEN_XSD_FACET_PATTERN )

#define GWEN_XSD_FACETS_DURATION (\
  GWEN_XSD_FACET_MAXINCLUSIVE |\
  GWEN_XSD_FACET_MAXEXCLUSIVE |\
  GWEN_XSD_FACET_MININCLUSIVE |\
  GWEN_XSD_FACET_MINEXCLUSIVE |\
  GWEN_XSD_FACET_ENUMERATION |\
  GWEN_XSD_FACET_PATTERN )

#define GWEN_XSD_FACETS_DATETIME (\
  GWEN_XSD_FACET_MAXINCLUSIVE |\
  GWEN_XSD_FACET_MAXEXCLUSIVE |\
  GWEN_XSD_FACET_MININCLUSIVE |\
  GWEN_XSD_FACET_MINEXCLUSIVE |\
  GWEN_XSD_FACET_ENUMERATION |\
  GWEN_XSD_FACET_PATTERN )

#define GWEN_XSD_FACETS_DATE (\
  GWEN_XSD_FACET_MAXINCLUSIVE |\
  GWEN_XSD_FACET_MAXEXCLUSIVE |\
  GWEN_XSD_FACET_MININCLUSIVE |\
  GWEN_XSD_FACET_MINEXCLUSIVE |\
  GWEN_XSD_FACET_ENUMERATION |\
  GWEN_XSD_FACET_PATTERN )

#define GWEN_XSD_FACETS_TIME (\
  GWEN_XSD_FACET_MAXINCLUSIVE |\
  GWEN_XSD_FACET_MAXEXCLUSIVE |\
  GWEN_XSD_FACET_MININCLUSIVE |\
  GWEN_XSD_FACET_MINEXCLUSIVE |\
  GWEN_XSD_FACET_ENUMERATION |\
  GWEN_XSD_FACET_PATTERN )

#define GWEN_XSD_FACETS_GYEARMONTH (\
  GWEN_XSD_FACET_MAXINCLUSIVE |\
  GWEN_XSD_FACET_MAXEXCLUSIVE |\
  GWEN_XSD_FACET_MININCLUSIVE |\
  GWEN_XSD_FACET_MINEXCLUSIVE |\
  GWEN_XSD_FACET_ENUMERATION |\
  GWEN_XSD_FACET_PATTERN )

#define GWEN_XSD_FACETS_GYEAR (\
  GWEN_XSD_FACET_MAXINCLUSIVE |\
  GWEN_XSD_FACET_MAXEXCLUSIVE |\
  GWEN_XSD_FACET_MININCLUSIVE |\
  GWEN_XSD_FACET_MINEXCLUSIVE |\
  GWEN_XSD_FACET_ENUMERATION |\
  GWEN_XSD_FACET_PATTERN )

#define GWEN_XSD_FACETS_GMONTHDAY (\
  GWEN_XSD_FACET_MAXINCLUSIVE |\
  GWEN_XSD_FACET_MAXEXCLUSIVE |\
  GWEN_XSD_FACET_MININCLUSIVE |\
  GWEN_XSD_FACET_MINEXCLUSIVE |\
  GWEN_XSD_FACET_ENUMERATION |\
  GWEN_XSD_FACET_PATTERN )

#define GWEN_XSD_FACETS_GMONTH (\
  GWEN_XSD_FACET_MAXINCLUSIVE |\
  GWEN_XSD_FACET_MAXEXCLUSIVE |\
  GWEN_XSD_FACET_MININCLUSIVE |\
  GWEN_XSD_FACET_MINEXCLUSIVE |\
  GWEN_XSD_FACET_ENUMERATION |\
  GWEN_XSD_FACET_PATTERN )

#define GWEN_XSD_FACETS_GDAY (\
  GWEN_XSD_FACET_MAXINCLUSIVE |\
  GWEN_XSD_FACET_MAXEXCLUSIVE |\
  GWEN_XSD_FACET_MININCLUSIVE |\
  GWEN_XSD_FACET_MINEXCLUSIVE |\
  GWEN_XSD_FACET_ENUMERATION |\
  GWEN_XSD_FACET_PATTERN )

#define GWEN_XSD_FACETS_HEXBINARY (\
  GWEN_XSD_FACET_LENGTH |\
  GWEN_XSD_FACET_MINLENGTH  |\
  GWEN_XSD_FACET_MAXLENGTH |\
  GWEN_XSD_FACET_WHITESPACE)

#define GWEN_XSD_FACETS_BASE64BINARY (\
  GWEN_XSD_FACET_LENGTH |\
  GWEN_XSD_FACET_MINLENGTH  |\
  GWEN_XSD_FACET_MAXLENGTH |\
  GWEN_XSD_FACET_WHITESPACE)

#define GWEN_XSD_FACETS_ANYURI (\
  GWEN_XSD_FACET_LENGTH |\
  GWEN_XSD_FACET_MINLENGTH  |\
  GWEN_XSD_FACET_MAXLENGTH |\
  GWEN_XSD_FACET_WHITESPACE)

#define GWEN_XSD_FACETS_QNAME (\
  GWEN_XSD_FACET_LENGTH |\
  GWEN_XSD_FACET_MINLENGTH  |\
  GWEN_XSD_FACET_MAXLENGTH |\
  GWEN_XSD_FACET_WHITESPACE)

#define GWEN_XSD_FACETS_NOTATION (\
  GWEN_XSD_FACET_LENGTH |\
  GWEN_XSD_FACET_MINLENGTH  |\
  GWEN_XSD_FACET_MAXLENGTH |\
  GWEN_XSD_FACET_WHITESPACE)


struct GWEN_XSD_FACETS {
  GWEN_LIST_ELEMENT(GWEN_XSD_FACETS)
  GWEN_TYPE_UINT32 fixedMask;
  GWEN_TYPE_UINT32 setMask;
  int length;
  int minLength;
  int maxLength;
  GWEN_XSD_FACET_WHITESPACE_MODE whiteSpaceMode;

  double maxInclusive;
  double maxExclusive;
  double minInclusive;
  double minExclusive;
  int totalDigits;
  int fractionDigits;

  GWEN_STRINGLIST *enumeration;
  GWEN_STRINGLIST *pattern;
  char *baseType;
};


GWEN_XSD_FACETS *GWEN_XSD_Facets_new();
void GWEN_XSD_Facets_free(GWEN_XSD_FACETS *xf);




struct GWEN_XSD_ENGINE {
  GWEN_XMLNODE *rootNode;
  GWEN_XSD_NAMESPACE_LIST *nameSpaces;
  GWEN_TYPE_UINT32 nextNameSpaceId;
  int derivedTypesImported;
  char *currentTargetNameSpace;

  /* runtime */

};



GWEN_XSD_NAMESPACE *GWEN_XSD_NameSpace_new(const char *id,
                                           const char *name,
                                           const char *url,
                                           const char *localUrl);
GWEN_XSD_NAMESPACE *GWEN_XSD_NameSpace_dup(const GWEN_XSD_NAMESPACE *ons);
void GWEN_XSD_NameSpace_free(GWEN_XSD_NAMESPACE *ns);
int GWEN_XSD_NameSpace_toXml(GWEN_XSD_NAMESPACE *ns, GWEN_XMLNODE *n);
GWEN_XSD_NAMESPACE *GWEN_XSD_NameSpace_fromXml(GWEN_XMLNODE *n);




int GWEN_XSD__GetStringValue(GWEN_XSD_ENGINE *e,
                             GWEN_DB_NODE *dbNode,
                             const char *name,
                             const char *tname,
                             GWEN_BUFFER *vbuf);

int GWEN_XSD__GetBooleanValue(GWEN_XSD_ENGINE *e,
                              GWEN_DB_NODE *dbNode,
                              const char *name,
                              const char *tname,
                              GWEN_BUFFER *vbuf);


GWEN_XSD_NAMESPACE *GWEN_XSD__FindNameSpaceById(GWEN_XSD_NAMESPACE_LIST *l,
                                                const char *name);
GWEN_XSD_NAMESPACE *GWEN_XSD__FindNameSpaceByName(GWEN_XSD_NAMESPACE_LIST *l,
                                                  const char *name);



int GWEN_XSD__RemoveNamespace(GWEN_XSD_ENGINE *e,
			      const char *xsdPrefix,
                              GWEN_XMLNODE *xmlNode);

/**
 * renames all newPrefixes of a document to the oldPrefixes (because the
 * oldPrefix already exists).
 */
int GWEN_XSD__ExchangeNamespace(GWEN_XSD_ENGINE *e,
                                const char *newPrefix,
                                const char *oldPrefix,
                                GWEN_XMLNODE *xmlNode,
                                int refOnly);

int GWEN_XSD__ExchangeNamespaceOnProperty(GWEN_XSD_ENGINE *e,
                                          const char *name,
                                          const char *newPrefix,
                                          const char *oldPrefix,
                                          GWEN_XMLNODE *xmlNode);

int GWEN_XSD__ImportSchema(GWEN_XSD_ENGINE *e,
                           GWEN_XMLNODE *xmlNode,
                           const char *fname);

int GWEN_XSD__FinishNode(GWEN_XSD_ENGINE *e, GWEN_XMLNODE *n);
int GWEN_XSD__FinishXsdDoc(GWEN_XSD_ENGINE *e);

int GWEN_XSD__GetSimpleTypeNodes(GWEN_XSD_ENGINE *e,
                                 GWEN_XMLNODE_LIST2 *nodeList,
                                 GWEN_XMLNODE *xmlNode,
                                 const char *tname,
                                 GWEN_BUFFER *baseName);

GWEN_XMLNODE *GWEN_XSD_GetTypeNode(GWEN_XSD_ENGINE *e,
                                   const char *name);
GWEN_XMLNODE *GWEN_XSD_GetElementNode(GWEN_XSD_ENGINE *e,
                                      const char *name);
GWEN_XMLNODE *GWEN_XSD_GetGroupNode(GWEN_XSD_ENGINE *e,
                                    const char *name);



int GWEN_XSD_ListTypes(GWEN_XSD_ENGINE *e,
                       const char *nameSpace,
                       const char *name,
                       GWEN_BUFFER *outBuffer);
int GWEN_XSD__ListTypes(GWEN_XSD_ENGINE *e,
                        const char *name,
                        GWEN_BUFFER *outBuffer,
                        int indent);
int GWEN_XSD__ListElementType(GWEN_XSD_ENGINE *e,
                              GWEN_XMLNODE *nElement,
                              GWEN_XMLNODE *nType,
                              const char *name,
                              GWEN_BUFFER *outBuffer,
                              int indent);
int GWEN_XSD__ListElementTypes(GWEN_XSD_ENGINE *e,
			       GWEN_XMLNODE *nElement,
			       GWEN_BUFFER *outBuffer,
                               int indent);
int GWEN_XSD__ListGroupTypes(GWEN_XSD_ENGINE *e,
                             GWEN_XMLNODE *nGroup,
                             GWEN_BUFFER *outBuffer,
                             int indent);

int GWEN_XSD__ListNodes(GWEN_XSD_ENGINE *e,
                        GWEN_XMLNODE *n,
                        GWEN_BUFFER *outBuffer,
                        int indent);

int GWEN_XSD__ListSequence(GWEN_XSD_ENGINE *e,
                           GWEN_XMLNODE *n,
                           GWEN_BUFFER *outBuffer,
                           int indent);
int GWEN_XSD__ListChoice(GWEN_XSD_ENGINE *e,
                         GWEN_XMLNODE *n,
                         GWEN_BUFFER *outBuffer,
                         int indent);


int GWEN_XSD__GetTypeFacets(GWEN_XSD_ENGINE *e,
                            GWEN_XMLNODE *nType,
                            GWEN_XSD_FACETS *xf);




int GWEN_XSD__GetBaseValue(GWEN_XSD_ENGINE *e,
                           GWEN_DB_NODE *dbNode,
                           const char *name,
                           int idx,
                           GWEN_BUFFER *vbuf,
                           GWEN_DB_VALUETYPE *vtPtr);

int GWEN_XSD__ValidateElementData(GWEN_XSD_ENGINE *e,
                                  GWEN_TYPE_UINT32 facetMask,
                                  GWEN_XSD_FACETS *xf,
                                  GWEN_BUFFER *vbuf);

int GWEN_XSD__GetElementData(GWEN_XSD_ENGINE *e,
                             GWEN_DB_NODE *dbNode,
                             const char *name,
                             int idx,
                             GWEN_XSD_FACETS *xf,
                             GWEN_BUFFER *dbuf);

int GWEN_XSD__WriteElementType(GWEN_XSD_ENGINE *e,
                               GWEN_XMLNODE *nElement,
                               GWEN_XMLNODE *nType,
                               GWEN_DB_NODE *dbNode,
                               const char *name,
                               int idx,
                               GWEN_XMLNODE *nStore);

int GWEN_XSD__WriteNode(GWEN_XSD_ENGINE *e,
                        GWEN_XMLNODE *nn,
                        GWEN_DB_NODE *dbNode,
                        GWEN_XMLNODE *nStore,
                        int defaultMinOccur);

int GWEN_XSD__WriteNodes(GWEN_XSD_ENGINE *e,
                         GWEN_XMLNODE *n,
                         GWEN_DB_NODE *dbNode,
                         GWEN_XMLNODE *nStore);

int GWEN_XSD__WriteSequence(GWEN_XSD_ENGINE *e,
                            GWEN_XMLNODE *n,
                            GWEN_DB_NODE *dbNode,
                            GWEN_XMLNODE *nStore);
int GWEN_XSD__WriteChoice(GWEN_XSD_ENGINE *e,
                          GWEN_XMLNODE *n,
                          GWEN_DB_NODE *dbNode,
                          GWEN_XMLNODE *nStore);


int GWEN_XSD__WriteElementTypes(GWEN_XSD_ENGINE *e,
                                GWEN_XMLNODE *nElement,
                                GWEN_DB_NODE *dbNode,
                                int idx,
                                GWEN_XMLNODE *nStore);

int GWEN_XSD__WriteGroupTypes(GWEN_XSD_ENGINE *e,
                              GWEN_XMLNODE *nGroup,
                              GWEN_DB_NODE *dbNode,
                              int idx,
                              GWEN_XMLNODE *nStore);

int GWEN_XSD_WriteElement(GWEN_XSD_ENGINE *e,
                          const char *nameSpace,
                          const char *name,
                          GWEN_DB_NODE *dbNode,
                          GWEN_XMLNODE *nStore,
                          int whistlesAndBells);

GWEN_XMLNODE *GWEN_XSD__CreateXmlNodeInNameSpace(GWEN_XSD_ENGINE *e,
                                                 const char *name);
const char *GWEN_XSD__QualifyNameIfNecessary(GWEN_XSD_ENGINE *e,
                                             GWEN_XMLNODE *n,
                                             const char *name);

int GWEN_XSD__ImportDerivedTypes(GWEN_XSD_ENGINE *e);


int GWEN_XSD__GlobalizeNode(GWEN_XSD_ENGINE *e,
                            GWEN_XMLNODE *node,
                            GWEN_STRINGLIST2 *nodeNameSpaces,
                            GWEN_XSD_NAMESPACE_LIST *docNameSpaces,
                            int *lastNameSpaceId);


#endif


