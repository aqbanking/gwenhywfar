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

#ifndef GWENHYFWAR_XML_P_H
#define GWENHYFWAR_XML_P_H

#include <chameleon/xml.h>


#ifdef __cplusplus
extern "C" {
#endif

#define XML_MAX_DEPTH 16
#define XML_MAX_TAGNAMELEN 32
#define XML_MAX_VARNAMELEN 32
#define XML_MAX_VALUELEN   64
#define XML_MAX_DATALEN    1024
#define XML_MAX_REMARKLEN  1024

struct _XMLPROPERTY {
  XMLPROPERTY *next;
  char *name;
  char *value;
};


XMLPROPERTY *XMLProperty_new(const char *name, const char *value);
void XMLProperty_free(XMLPROPERTY *p);
XMLPROPERTY *XMLProperty_dup(XMLPROPERTY *p);
void XMLProperty_freeAll(XMLPROPERTY *p);

void XMLProperty_add(XMLPROPERTY *p, XMLPROPERTY **head);
void XMLProperty_del(XMLPROPERTY *p, XMLPROPERTY **head);

struct _XMLNODE {
  XMLNODE *next;
  XMLNODE *child;
  XMLNODE *parent;

  XMLNODE_TYPE type;
  XMLPROPERTY *properties;
  char *data;
};

void XMLNode_add(XMLNODE *n, XMLNODE **head);
void XMLNode_del(XMLNODE *n, XMLNODE **head);


/**
 * Reads a word from the buffered input until one of the delimiters is found.
 * @return <0 on error, >0 if delimiter (code), 0 if stopped for EOF
 */
int XML__ReadWord(BUFFEREDIOTABLE *bio,
                  char chr,
		  const char *delims,
		  char *buffer,
		  unsigned int size);


#ifdef __cplusplus
}
#endif



#endif
