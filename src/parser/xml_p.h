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


#define GWEN_XML_FLAGS__INTERNAL 0xf0000000
#define GWEN_XML_FLAGS__DATA     0x10000000

#define GWEN_XML_MAX_DEPTH 32

struct GWEN__XMLPROPERTY {
  GWEN_XMLPROPERTY *next;
  char *name;
  char *value;
};


GWEN_XMLPROPERTY *GWEN_XMLProperty_new(const char *name, const char *value);
void GWEN_XMLProperty_free(GWEN_XMLPROPERTY *p);
GWEN_XMLPROPERTY *GWEN_XMLProperty_dup(const GWEN_XMLPROPERTY *p);
void GWEN_XMLProperty_freeAll(GWEN_XMLPROPERTY *p);

void GWEN_XMLProperty_add(GWEN_XMLPROPERTY *p, GWEN_XMLPROPERTY **head);
void GWEN_XMLProperty_insert(GWEN_XMLPROPERTY *p, GWEN_XMLPROPERTY **head);
void GWEN_XMLProperty_del(GWEN_XMLPROPERTY *p, GWEN_XMLPROPERTY **head);

struct GWEN__XMLNODE {
  GWEN_XMLNODE *next;
  GWEN_XMLNODE *child;
  GWEN_XMLNODE *parent;

  GWEN_XMLNODE *header;

  GWEN_XMLNODE_TYPE type;
  GWEN_XMLPROPERTY *properties;
  GWEN_TYPE_UINT32 usage;
  char *data;
};

void GWEN_XMLNode_add(GWEN_XMLNODE *n, GWEN_XMLNODE **head);
void GWEN_XMLNode_del(GWEN_XMLNODE *n, GWEN_XMLNODE **head);


struct GWEN_XMLNODE_PATH {
  unsigned int pos;
  GWEN_XMLNODE *nodes[GWEN_XML_MAX_DEPTH];
};



void GWEN_XMLNode__SetProperty(GWEN_XMLNODE *n,
                               const char *name, const char *value,
                               int doInsert);

/**
 * Reads a word from the buffered input until one of the delimiters is found.
 * @return <0 on error, >0 if delimiter (code), 0 if stopped for EOF
 */
int GWEN_XML__ReadWordBuf(GWEN_BUFFEREDIO *bio,
                          GWEN_TYPE_UINT32 flags,
                          int chr,
                          const char *delims,
                          GWEN_BUFFER *buf);


int GWEN_XML_ReadBIO(GWEN_XMLNODE *n,
                     GWEN_BUFFEREDIO *bio,
                     GWEN_TYPE_UINT32 flags,
                     const char *path,
                     GWEN_STRINGLIST *sl,
                     GWEN_XML_INCLUDE_FN fn);


int GWEN_XML_ReadFileInt(GWEN_XMLNODE *n,
                         const char *path,
                         const char *file,
                         GWEN_STRINGLIST *sl,
                         GWEN_TYPE_UINT32 flags);

int GWEN_XMLNode__WriteToStream(const GWEN_XMLNODE *n,
                                GWEN_BUFFEREDIO *bio,
                                GWEN_TYPE_UINT32 flags,
                                unsigned int ind);


GWEN_XMLNODE *GWEN_XMLNode_GetFirstOfType(const GWEN_XMLNODE *n,
                                          GWEN_XMLNODE_TYPE t);
GWEN_XMLNODE *GWEN_XMLNode_GetNextOfType(const GWEN_XMLNODE *n,
                                         GWEN_XMLNODE_TYPE t);

GWEN_XMLNODE *GWEN_XMLNode_FindTag(const GWEN_XMLNODE *n,
                                   const char *tname,
                                   const char *pname,
                                   const char *pvalue);


/**
 * Removes all namespace declarations which have been declared in a higher
 * level.
 */
int GWEN_XMLNode__CheckNameSpaceDecls1(GWEN_XMLNODE *n,
                                       GWEN_STRINGLIST2 *sl,
                                       const char *currentNameSpace);

/**
 * Moves all namespace declarations from this node to the nodes
 * of first use
 */
int GWEN_XMLNode__CheckNameSpaceDecls3(GWEN_XMLNODE *n);


int GWEN_XMLNode__SetNameSpaces(GWEN_XMLNODE *n,
                                const char *prefix,
                                const char *nspace);
int GWEN_XMLNode__CheckAndSetNameSpace(GWEN_XMLNODE *n,
                                       const char *prefix,
                                       const char *nspace);


#endif
