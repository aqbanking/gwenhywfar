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

#include <gwenhywfar/xml.h>


#ifdef __cplusplus
extern "C" {
#endif

/* Watch out! These buffer length can easily be exceeded since XML
   itself does not give you any limit on the length of particular
   varilable or element names! This would mean that a valid XML file
   is failed to be parsed. */
#define GWEN_XML_MAX_DEPTH 32
#define GWEN_XML_MAX_TAGNAMELEN 64
#define GWEN_XML_MAX_VARNAMELEN 128
/* The above value was already exceeded (when it was 32) for a PUBLIC
   identifier in the <!DOCTYPE> element */
#define GWEN_XML_MAX_VALUELEN   128
#define GWEN_XML_MAX_DATALEN    2048
#define GWEN_XML_MAX_REMARKLEN  16384

struct GWEN__XMLPROPERTY {
  GWEN_XMLPROPERTY *next;
  char *name;
  char *value;
};


GWEN_XMLPROPERTY *GWEN_XMLProperty_new(const char *name, const char *value);
void GWEN_XMLProperty_free(GWEN_XMLPROPERTY *p);
GWEN_XMLPROPERTY *GWEN_XMLProperty_dup(GWEN_XMLPROPERTY *p);
void GWEN_XMLProperty_freeAll(GWEN_XMLPROPERTY *p);

void GWEN_XMLProperty_add(GWEN_XMLPROPERTY *p, GWEN_XMLPROPERTY **head);
void GWEN_XMLProperty_del(GWEN_XMLPROPERTY *p, GWEN_XMLPROPERTY **head);

struct GWEN__XMLNODE {
  GWEN_XMLNODE *next;
  GWEN_XMLNODE *child;
  GWEN_XMLNODE *parent;

  GWEN_XMLNODE_TYPE type;
  GWEN_XMLPROPERTY *properties;
  char *data;
};

void GWEN_XMLNode_add(GWEN_XMLNODE *n, GWEN_XMLNODE **head);
void GWEN_XMLNode_del(GWEN_XMLNODE *n, GWEN_XMLNODE **head);


struct GWEN_XMLNODE_PATH {
  unsigned int pos;
  GWEN_XMLNODE *nodes[GWEN_XML_MAX_DEPTH];
};


/**
 * Reads a word from the buffered input until one of the delimiters is found.
 * @return <0 on error, >0 if delimiter (code), 0 if stopped for EOF
 */
int GWEN_XML__ReadWord(GWEN_BUFFEREDIO *bio,
                       char chr,
                       const char *delims,
                       char *buffer,
                       unsigned int size);


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


GWEN_XMLNODE *GWEN_XMLNode_GetFirstOfType(GWEN_XMLNODE *n,
                                          GWEN_XMLNODE_TYPE t);
GWEN_XMLNODE *GWEN_XMLNode_GetNextOfType(GWEN_XMLNODE *n,
                                         GWEN_XMLNODE_TYPE t);

GWEN_XMLNODE *GWEN_XMLNode_FindTag(GWEN_XMLNODE *n,
                                   const char *tname,
                                   const char *pname,
                                   const char *pvalue);



#ifdef __cplusplus
}
#endif



#endif
