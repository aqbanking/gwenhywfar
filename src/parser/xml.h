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

#ifndef GWENHYWFAR_XML_H
#define GWENHYWFAR_XML_H

#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/bufferedio.h>
#include <gwenhywfar/stringlist.h>
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
#define GWEN_XML_FLAGS_READ_COMMENTS  0x0001
#define GWEN_XML_FLAGS_SHARE_TOPLEVEL 0x0002
#define GWEN_XML_FLAGS_DEFAULT 0
/*@}*/


GWENHYWFAR_API
typedef struct GWEN__XMLPROPERTY GWEN_XMLPROPERTY;

/**
 * Node type.
 */
GWENHYWFAR_API
typedef enum {
  GWEN_XMLNodeTypeTag=0,
  GWEN_XMLNodeTypeData,
  GWEN_XMLNodeTypeComment
} GWEN_XMLNODE_TYPE;

GWENHYWFAR_API
typedef struct GWEN__XMLNODE GWEN_XMLNODE;


GWENHYWFAR_API
typedef int
    (*GWEN_XML_INCLUDE_FN)(GWEN_XMLNODE *n,
			   const char *path,
                           const char *file,
                           GWEN_STRINGLIST *sl,
			   unsigned int flags);


/** @name Constructors and Destructors
 *
 */
/*@{*/
GWENHYWFAR_API
GWEN_XMLNODE *GWEN_XMLNode_new(GWEN_XMLNODE_TYPE t, const char *data);
GWENHYWFAR_API
void GWEN_XMLNode_free(GWEN_XMLNODE *n);
GWENHYWFAR_API
void GWEN_XMLNode_freeAll(GWEN_XMLNODE *n);
GWENHYWFAR_API
GWEN_XMLNODE *GWEN_XMLNode_dup(GWEN_XMLNODE *n);
/*@}*/


/** @name Managing Properties
 *
 */
/*@{*/
GWENHYWFAR_API
const char *GWEN_XMLNode_GetProperty(GWEN_XMLNODE *n, const char *name,
                                     const char *defaultValue);
GWENHYWFAR_API
void GWEN_XMLNode_SetProperty(GWEN_XMLNODE *n, const char *name, const char *value);
GWENHYWFAR_API
void GWEN_XMLNode_CopyProperties(GWEN_XMLNODE *tn,
                                 GWEN_XMLNODE *sn,
                                 int overwrite);
/*@}*/

/** @name Type And Data
 *
 */
/*@{*/
GWENHYWFAR_API
GWEN_XMLNODE_TYPE GWEN_XMLNode_GetType(GWEN_XMLNODE *n);
GWENHYWFAR_API
const char *GWEN_XMLNode_GetData(GWEN_XMLNODE *n);
GWENHYWFAR_API
void GWEN_XMLNode_SetData(GWEN_XMLNODE *n, const char *data);
/*@}*/

/** @name Iterating Through an XML Tree
 *
 */
/*@{*/
GWENHYWFAR_API
GWEN_XMLNODE *GWEN_XMLNode_Next(GWEN_XMLNODE *n);
GWENHYWFAR_API
GWEN_XMLNODE *GWEN_XMLNode_GetChild(GWEN_XMLNODE *n);
GWENHYWFAR_API
  GWEN_XMLNODE *GWEN_XMLNode_GetParent(GWEN_XMLNODE *n);

GWEN_XMLNODE *GWEN_XMLNode_GetFirstTag(GWEN_XMLNODE *n);
GWEN_XMLNODE *GWEN_XMLNode_GetNextTag(GWEN_XMLNODE *n);

GWEN_XMLNODE *GWEN_XMLNode_GetFirstData(GWEN_XMLNODE *n);
GWEN_XMLNODE *GWEN_XMLNode_GetNextData(GWEN_XMLNODE *n);

/**
 * Searches for the first matching tag below the given one.
 * Lets say you have the following XML file:
 * @code
 *  <DEVICES>
 *    <DEVICE id="dev1" />
 *    <DEVICE id="dev2" />
 *  </DEVICES>
 * @endcode
 * If you are looking for a device called "dev2" then you should call this
 * function like this:
 * @code
 *   tag=GWEN_XMLNode_FindFirstTag(root, "DEVICE", "id", "dev2");
 * @endcode
 * @return pointer to the tag if found, 0 otherwise
 * @param n tag below which to search
 * @param tname tag type name (e.g. if the tag is "<TESTTAG>" then the
 * tag type name if "TESTTAG"). Wildcards (like "*") are allowed.
 * @param pname name of the property to check (if 0 then no property
 * comparison takes place). No wildcards allowed.
 * @param pvalue optional value of the property to compare against, wildcards
 * allowed.
 */
GWEN_XMLNODE *GWEN_XMLNode_FindFirstTag(GWEN_XMLNODE *n,
                                        const char *tname,
                                        const char *pname,
                                        const char *pvalue);

/**
 * Searches for the next matching tag after the given one.
 */
GWEN_XMLNODE *GWEN_XMLNode_FindNextTag(GWEN_XMLNODE *n,
                                       const char *tname,
                                       const char *pname,
                                       const char *pvalue);



/*@}*/


/** @name Managing Nodes
 *
 */
/*@{*/
GWENHYWFAR_API
void GWEN_XMLNode_AddChild(GWEN_XMLNODE *n, GWEN_XMLNODE *child);
GWENHYWFAR_API
void GWEN_XMLNode_UnlinkChild(GWEN_XMLNODE *n, GWEN_XMLNODE *child);
GWENHYWFAR_API
void GWEN_XMLNode_RemoveChildren(GWEN_XMLNODE *n);
GWENHYWFAR_API
GWEN_XMLNODE *GWEN_XMLNode_FindNode(GWEN_XMLNODE *n,
                                    GWEN_XMLNODE_TYPE t, const char *data);
/*@}*/


/** @name Reading And Writing From/To Streams
 *
 */
/*@{*/
/**
 * Reads exactly ONE tag (and all its subtags) from the given bufferedIO.
 */
GWENHYWFAR_API
int GWEN_XML_Parse(GWEN_XMLNODE *n, GWEN_BUFFEREDIO *bio,
                   unsigned int flags);

/**
 * Reads all tags from a file and adds them as children to the given node.
 */
GWENHYWFAR_API
int GWEN_XML_ReadFile(GWEN_XMLNODE *n, const char *filepath,
                      unsigned int flags);

/**
 * Reads the given file. If it the path is absolute it will be used directly.
 * If it is relative then the given search path will be searched if the
 * file with the given name could not be loaded without a search path.
 * @param n XML node to store the read tags in
 * @param filepath name (and optionally path) of the file to read
 * @param flags see @ref GWEN_XML_FLAGS_DEFAULT and others
 * @param searchPath a string list containing multiple multiple directories
 * to scan if the file could not be opened directly
 */
GWENHYWFAR_API
  int GWEN_XML_ReadFileSearch(GWEN_XMLNODE *n, const char *filepath,
                              unsigned int flags,
                              GWEN_STRINGLIST *searchPath);


/*@}*/


/** @name Debugging
 *
 */
/*@{*/
GWENHYWFAR_API
void GWEN_XMLNode_Dump(GWEN_XMLNODE *n, FILE *f, int ind);
/*@}*/

/*@}*/ /* defgroup */


/** @defgroup MOD_XMLNODE_PATH XML Node Path
 *
 */
/*@{*/

GWENHYWFAR_API
typedef struct GWEN_XMLNODE_PATH GWEN_XMLNODE_PATH;


GWENHYWFAR_API
GWEN_XMLNODE_PATH *GWEN_XMLNode_Path_new();
GWENHYWFAR_API
GWEN_XMLNODE_PATH *GWEN_XMLNode_Path_dup(GWEN_XMLNODE_PATH *np);
GWENHYWFAR_API
void GWEN_XMLNode_Path_free(GWEN_XMLNODE_PATH *np);

GWENHYWFAR_API
int GWEN_XMLNode_Path_Dive(GWEN_XMLNODE_PATH *np,
                           GWEN_XMLNODE *n);
GWENHYWFAR_API
GWEN_XMLNODE *GWEN_XMLNode_Path_Surface(GWEN_XMLNODE_PATH *np);
GWENHYWFAR_API
void GWEN_XMLNode_Path_Dump(GWEN_XMLNODE_PATH *np);
/*@}*/ /* defgroup */
/*@}*/ /* defgroup (all)*/


#ifdef __cplusplus
}
#endif



#endif
