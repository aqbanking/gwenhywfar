/***************************************************************************
    begin       : Sun Dec 16 2018
    copyright   : (C) 2018 by Martin Preuss
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


#ifndef GWEN_XML2DB_P_H
#define GWEN_XML2DB_P_H

#include <gwenhywfar/xml2db.h>

#include <gwenhywfar/db.h>
#include <gwenhywfar/xml.h>


typedef struct GWEN_XML2DB_CONTEXT GWEN_XML2DB_CONTEXT;

typedef int (*GWEN_XML2DB_HANDLECHILDREN_FN)(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);


struct GWEN_XML2DB_CONTEXT {
  GWEN_XMLNODE *docRoot;            /* provided by caller (dont free) */
  GWEN_XMLNODE *currentDocNode;     /* pointer, dont free */

  GWEN_XMLNODE_LIST2 *xmlNodeStack;  /* do free */

  GWEN_DB_NODE *dbRoot;             /* provided by caller (dont free) */
  GWEN_DB_NODE *currentDbGroup;     /* pointer, dont free */

  GWEN_DB_NODE *tempDbRoot;         /* do free */
  GWEN_DB_NODE *currentTempDbGroup; /* pointer, dont free */

  GWEN_XML2DB_HANDLECHILDREN_FN handleChildrenFn;
};




GWEN_XML2DB_CONTEXT *GWEN_Xml2Db_Context_new(GWEN_XMLNODE *documentRoot, GWEN_DB_NODE *dbRoot);
void GWEN_Xml2Db_Context_free(GWEN_XML2DB_CONTEXT *ctx);

void GWEN_Xml2Db_Context_EnterDocNode(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);
void GWEN_Xml2Db_Context_LeaveDocNode(GWEN_XML2DB_CONTEXT *ctx);

int GWEN_Xml2Db_Context_HandleChildren(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);


#endif /* GWEN_XML2DB_P_H */
