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

#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/bufferedio.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup MOD_XMLNODE_ALL XML Tree
 * @ingroup MOD_PARSER
 *
 */
/*@{*/

/** @defgroup MOD_XMLNODE XML Node
 *
 */
/*@{*/

/** @name Read Flags
 */
/*@{*/
#define GWEN_XML_FLAGS_READ_COMMENTS 0x0001
#define GWEN_XML_FLAGS_DEFAULT 0
/*@}*/


typedef struct GWEN__XMLPROPERTY GWEN_XMLPROPERTY;

/**
 * Node type.
 */
typedef enum {
  GWEN_XMLNodeTypeTag=0,
  GWEN_XMLNodeTypeData,
  GWEN_XMLNodeTypeComment
} GWEN_XMLNODE_TYPE;

typedef struct GWEN__XMLNODE GWEN_XMLNODE;


/** @name Constructors and Destructors
 *
 */
/*@{*/
GWEN_XMLNODE *GWEN_XMLNode_new(GWEN_XMLNODE_TYPE t, const char *data);
void GWEN_XMLNode_free(GWEN_XMLNODE *n);
void GWEN_XMLNode_freeAll(GWEN_XMLNODE *n);
GWEN_XMLNODE *GWEN_XMLNode_dup(GWEN_XMLNODE *n);
/*@}*/


/** @name Managing Properties
 *
 */
/*@{*/
const char *GWEN_XMLNode_GetProperty(GWEN_XMLNODE *n, const char *name,
                                     const char *defaultValue);
void GWEN_XMLNode_SetProperty(GWEN_XMLNODE *n, const char *name, const char *value);
void GWEN_XMLNode_CopyProperties(GWEN_XMLNODE *tn,
                                 GWEN_XMLNODE *sn,
                                 int overwrite);
/*@}*/

/** @name Type And Data
 *
 */
/*@{*/
GWEN_XMLNODE_TYPE GWEN_XMLNode_GetType(GWEN_XMLNODE *n);
const char *GWEN_XMLNode_GetData(GWEN_XMLNODE *n);
void GWEN_XMLNode_SetData(GWEN_XMLNODE *n, const char *data);
/*@}*/

/** @name Iterating Through an XML Tree
 *
 */
/*@{*/
GWEN_XMLNODE *GWEN_XMLNode_Next(GWEN_XMLNODE *n);
GWEN_XMLNODE *GWEN_XMLNode_GetChild(GWEN_XMLNODE *n);
GWEN_XMLNODE *GWEN_XMLNode_GetParent(GWEN_XMLNODE *n);
/*@}*/


/** @name Managing Nodes
 *
 */
/*@{*/
void GWEN_XMLNode_AddChild(GWEN_XMLNODE *n, GWEN_XMLNODE *child);
void GWEN_XMLNode_UnlinkChild(GWEN_XMLNODE *n, GWEN_XMLNODE *child);
void GWEN_XMLNode_RemoveChildren(GWEN_XMLNODE *n);
GWEN_XMLNODE *GWEN_XMLNode_FindNode(GWEN_XMLNODE *n,
                                    GWEN_XMLNODE_TYPE t, const char *data);
/*@}*/


/** @name Readin And Writing From/To Streams
 *
 */
/*@{*/
/**
 * Reads exacrly ONE tag (and all its subtags) from the given bufferedIO.
 */
int GWEN_XML_Parse(GWEN_XMLNODE *n, GWEN_BUFFEREDIO *bio,
                   unsigned int flags);

/**
 * Reads all tags from a file and adds them as children to the given node.
 */
int GWEN_XML_ReadFile(GWEN_XMLNODE *n, const char *filepath,
                      unsigned int flags);
/*@}*/


/** @name Debugging
 *
 */
/*@{*/
void GWEN_XMLNode_Dump(GWEN_XMLNODE *n, FILE *f, int ind);
/*@}*/

/*@}*/ /* defgroup */


/** @defgroup MOD_XMLNODE_PATH XML Node Path
 *
 */
/*@{*/

typedef struct GWEN_XMLNODE_PATH GWEN_XMLNODE_PATH;


GWEN_XMLNODE_PATH *GWEN_XMLNode_Path_new();
GWEN_XMLNODE_PATH *GWEN_XMLNode_Path_dup(GWEN_XMLNODE_PATH *np);
void GWEN_XMLNode_Path_free(GWEN_XMLNODE_PATH *np);

int GWEN_XMLNode_Path_Dive(GWEN_XMLNODE_PATH *np,
                           GWEN_XMLNODE *n);
GWEN_XMLNODE *GWEN_XMLNode_Path_Surface(GWEN_XMLNODE_PATH *np);
void GWEN_XMLNode_Path_Dump(GWEN_XMLNODE_PATH *np);
/*@}*/ /* defgroup */
/*@}*/ /* defgroup (all)*/


#ifdef __cplusplus
}
#endif



#endif
