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

#ifndef GWENHYFWAR_XML_H
#define GWENHYFWAR_XML_H

#include <chameleon/chameleonapi.h>
#include <chameleon/bufferedio.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct _XMLPROPERTY XMLPROPERTY;
typedef enum {
  XMLNodeTypeTag=0,
  XMLNodeTypeData,
  XMLNodeTypeComment
} XMLNODE_TYPE;

typedef struct _XMLNODE XMLNODE;

XMLNODE *XMLNode_new(XMLNODE_TYPE t, const char *data);
void XMLNode_free(XMLNODE *n);
void XMLNode_freeAll(XMLNODE *n);
XMLNODE *XMLNode_dup(XMLNODE *n);

XMLNODE *XMLNode_Next(XMLNODE *n);

const char *XMLNode_GetProperty(XMLNODE *n, const char *name,
				const char *defaultValue);
void XMLNode_SetProperty(XMLNODE *n, const char *name, const char *value);

XMLNODE_TYPE XMLNode_GetType(XMLNODE *n);

const char *XMLNode_GetData(XMLNODE *n);
void XMLNode_SetData(XMLNODE *n, const char *data);

XMLNODE *XMLNode_GetChild(XMLNODE *n);
XMLNODE *XMLNode_GetParent(XMLNODE *n);
void XMLNode_AddChild(XMLNODE *n, XMLNODE *child);
void XMLNode_UnlinkChild(XMLNODE *n, XMLNODE *child);

XMLNODE *XMLNode_FindNode(XMLNODE *n,
			  XMLNODE_TYPE t, const char *data);


/**
 * Reads ONE tag (and all its subtags) from the given bufferedIO.
 */
int XML_Parse(XMLNODE *n, BUFFEREDIOTABLE *bio);

/**
 * Reads all tags from a file and adds them as children to the given node.
 */
int XML_ReadFile(XMLNODE *n, const char *filepath);


void XMLNode_Dump(XMLNODE *n, FILE *f, int ind);


#ifdef __cplusplus
}
#endif



#endif
