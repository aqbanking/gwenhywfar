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

#ifndef GWENHYWFAR_XML_P_H
#define GWENHYWFAR_XML_P_H

#include "xml_l.h"
#include <gwenhywfar/fastbuffer.h>



#define GWEN_XML_FLAGS__INTERNAL 0xf0000000
#define GWEN_XML_FLAGS__DATA     0x10000000

#define GWEN_XML_MAX_DEPTH 32

struct GWEN__XMLPROPERTY {
  GWEN_XMLPROPERTY *next;
  char *name;
  char *value;
  char *nameSpace;
};


GWEN_XMLPROPERTY *GWEN_XMLProperty_new(const char *name, const char *value);
void GWEN_XMLProperty_free(GWEN_XMLPROPERTY *p);
GWEN_XMLPROPERTY *GWEN_XMLProperty_dup(const GWEN_XMLPROPERTY *p);
void GWEN_XMLProperty_freeAll(GWEN_XMLPROPERTY *p);

void GWEN_XMLProperty_add(GWEN_XMLPROPERTY *p, GWEN_XMLPROPERTY **head);
void GWEN_XMLProperty_insert(GWEN_XMLPROPERTY *p, GWEN_XMLPROPERTY **head);
void GWEN_XMLProperty_del(GWEN_XMLPROPERTY *p, GWEN_XMLPROPERTY **head);

struct GWEN__XMLNODE {
  GWEN_LIST_ELEMENT(GWEN_XMLNODE)

  GWEN_XMLNODE_LIST *children;
  GWEN_XMLNODE *parent;

  GWEN_XMLNODE_LIST *headers;
  GWEN_XMLNODE_NAMESPACE_LIST *nameSpaces;

  GWEN_XMLNODE_TYPE type;
  GWEN_XMLPROPERTY *properties;
  uint32_t usage;
  char *data;
  char *nameSpace;
};


struct GWEN_XMLNODE_PATH {
  unsigned int pos;
  GWEN_XMLNODE *nodes[GWEN_XML_MAX_DEPTH];
};



struct GWEN_XMLNODE_NAMESPACE {
  GWEN_LIST_ELEMENT(GWEN_XMLNODE_NAMESPACE)
  char *name;
  char *url;
};



static void GWEN_XMLNode__SetProperty(GWEN_XMLNODE *n,
                                      const char *name, const char *value,
                                      int doInsert);

static int GWEN_XMLNode__WriteToStream(const GWEN_XMLNODE *n,
                                       GWEN_FAST_BUFFER *fb,
                                       uint32_t flags,
                                       const char *encoding,
                                       unsigned int ind);


static GWEN_XMLNODE *GWEN_XMLNode_GetFirstOfType(const GWEN_XMLNODE *n,
                                                 GWEN_XMLNODE_TYPE t);
static GWEN_XMLNODE *GWEN_XMLNode_GetNextOfType(const GWEN_XMLNODE *n,
                                                GWEN_XMLNODE_TYPE t);

static GWEN_XMLNODE *GWEN_XMLNode_FindTag(const GWEN_XMLNODE *n,
                                          const char *tname,
                                          const char *pname,
                                          const char *pvalue);


/**
 * Removes all namespace declarations which have been declared in a higher
 * level.
 */
static int GWEN_XMLNode__CheckNameSpaceDecls1(GWEN_XMLNODE *n,
                                              GWEN_STRINGLIST2 *sl,
                                              const char *currentNameSpace);

/**
 * Moves all namespace declarations from this node to the nodes
 * of first use
 */
static int GWEN_XMLNode__CheckNameSpaceDecls3(GWEN_XMLNODE *n);


static int GWEN_XMLNode__SetNameSpaces(GWEN_XMLNODE *n,
                                       const char *prefix,
                                       const char *nspace);
static int GWEN_XMLNode__CheckAndSetNameSpace(GWEN_XMLNODE *n,
                                              const char *prefix,
                                              const char *nspace);

static int GWEN_XML__ReadData(GWEN_XML_CONTEXT *ctx,
                              GWEN_FAST_BUFFER *fb,
                              uint32_t flags);

static int GWEN_XML__ReadTag(GWEN_XML_CONTEXT *ctx,
                             GWEN_FAST_BUFFER *fb,
                             uint32_t flags,
                             GWEN_BUFFER *workBuf);

static int GWEN_XML__ReadAllFromIo(GWEN_XML_CONTEXT *ctx, GWEN_SYNCIO *sio);


static GWEN_XMLNODE_NAMESPACE *GWEN_XMLGL__GetNameSpaceByPrefix(GWEN_XMLNODE *n,
                                                                const char *prefix);

static int GWEN_XMLGL__SampleNameSpaces(GWEN_XMLNODE *n,
                                        GWEN_XMLNODE_NAMESPACE_LIST *l,
                                        uint32_t *pLastId);

static void GWEN_XMLGL__ClearNameSpaces(GWEN_XMLNODE *n);

static int GWEN_XMLGL__TranslateName(GWEN_XMLNODE *n,
                                     GWEN_XMLNODE_NAMESPACE_LIST *l,
                                     char **pValue);


#endif
