/***************************************************************************
 begin       : Wed Feb 27 2008
 copyright   : (C) 2008 by Martin Preuss
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


#include "xsdnode_p.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>


GWEN_INHERIT_FUNCTIONS(GWEN_XSD_NODE)
GWEN_LIST_FUNCTIONS(GWEN_XSD_NODE, GWEN_XsdNode)



GWEN_XSD_NODE *GWEN_XsdNode_new(GWEN_XSD_NODE *parent,
                                GWEN_XSD_NODETYPE t,
                                const char *name)
{
  GWEN_XSD_NODE *xsdNode;

  GWEN_NEW_OBJECT(GWEN_XSD_NODE, xsdNode);
  GWEN_INHERIT_INIT(GWEN_XSD_NODE, xsdNode);
  GWEN_LIST_INIT(GWEN_XSD_NODE, xsdNode);
  xsdNode->nodeType=t;
  xsdNode->parent=parent;
  xsdNode->children=GWEN_XsdNode_List_new();
  if (name)
    xsdNode->name=strdup(name);

  return xsdNode;
}



void GWEN_XsdNode_free(GWEN_XSD_NODE *xsdNode)
{
  if (xsdNode) {
    GWEN_LIST_FINI(GWEN_XSD_NODE, xsdNode);
    GWEN_INHERIT_FINI(GWEN_XSD_NODE, xsdNode);
    GWEN_XsdNode_List_free(xsdNode->children);
    free(xsdNode->name);
    GWEN_FREE_OBJECT(xsdNode);
  }
}



GWEN_XSD_NODETYPE GWEN_XsdNode_GetNodeType(const GWEN_XSD_NODE *xsdNode)
{
  assert(xsdNode);
  return xsdNode->nodeType;
}



const char *GWEN_XsdNode_GetName(const GWEN_XSD_NODE *xsdNode)
{
  assert(xsdNode);
  return xsdNode->name;
}



uint32_t GWEN_XsdNode_GetFlags(const GWEN_XSD_NODE *xsdNode)
{
  assert(xsdNode);
  return xsdNode->flags;
}



void GWEN_XsdNode_SetFlags(GWEN_XSD_NODE *xsdNode, uint32_t fl)
{
  assert(xsdNode);
  xsdNode->flags=fl;
}



void GWEN_XsdNode_AddFlags(GWEN_XSD_NODE *xsdNode, uint32_t fl)
{
  assert(xsdNode);
  xsdNode->flags|=fl;
}



void GWEN_XsdNode_SubFlags(GWEN_XSD_NODE *xsdNode, uint32_t fl)
{
  assert(xsdNode);
  xsdNode->flags&=~fl;
}



GWEN_XSD_NODE *GWEN_XsdNode_GetParent(const GWEN_XSD_NODE *xsdNode)
{
  assert(xsdNode);
  return xsdNode->parent;
}



GWEN_XSD_NODE_LIST *GWEN_XsdNode_GetChildren(const GWEN_XSD_NODE *xsdNode)
{
  assert(xsdNode);
  return xsdNode->children;
}



void GWEN_XsdNode_AddChild(GWEN_XSD_NODE *xsdNode, GWEN_XSD_NODE *newChild)
{
  assert(xsdNode);
  assert(newChild);
  GWEN_XsdNode_List_Add(newChild, xsdNode->children);
  newChild->parent=xsdNode;
}



void GWEN_XsdNode_Unlink(GWEN_XSD_NODE *xsdNode)
{
  GWEN_XsdNode_List_Del(xsdNode);
  xsdNode->parent=NULL;
}



GWEN_XSDNODE_READ_FN GWEN_XsdNode_SetReadFn(GWEN_XSD_NODE *xsdNode,
                                            GWEN_XSDNODE_READ_FN fn)
{
  GWEN_XSDNODE_READ_FN oldFn;

  assert(xsdNode);
  oldFn=xsdNode->readFn;
  xsdNode->readFn=fn;
  return oldFn;
}



GWEN_XSDNODE_WRITE_FN GWEN_XsdNode_SetWriteFn(GWEN_XSD_NODE *xsdNode,
                                              GWEN_XSDNODE_WRITE_FN fn)
{
  GWEN_XSDNODE_WRITE_FN oldFn;

  assert(xsdNode);
  oldFn=xsdNode->writeFn;
  xsdNode->writeFn=fn;
  return oldFn;
}



int GWEN_XsdNode_Read(GWEN_XSD_NODE *xsdNode,
                      GWEN_XMLNODE *xmlNode,
                      GWEN_DB_NODE *db)
{
  assert(xsdNode);
  if (xsdNode->readFn)
    return xsdNode->readFn(xsdNode, xmlNode, db);
  else
    return GWEN_ERROR_NOT_SUPPORTED;
}



int GWEN_XsdNode_Write(GWEN_XSD_NODE *xsdNode,
                       GWEN_XMLNODE *xmlNode,
                       GWEN_DB_NODE *db)
{
  assert(xsdNode);
  if (xsdNode->writeFn)
    return xsdNode->writeFn(xsdNode, xmlNode, db);
  else
    return GWEN_ERROR_NOT_SUPPORTED;
}




