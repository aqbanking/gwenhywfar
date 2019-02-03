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



typedef struct {
  GWEN_XMLNODE *docRoot;            /* provided by caller (dont free) */
  GWEN_XMLNODE *currentDocNode;     /* pointer, dont free */

  GWEN_XMLNODE_LIST2 *xmlNodeStack;  /* do free */

  GWEN_DB_NODE *dbRoot;             /* provided by caller (dont free) */
  GWEN_DB_NODE *currentDbGroup;     /* pointer, dont free */

  GWEN_DB_NODE *tempDbRoot;         /* do free */
  GWEN_DB_NODE *currentTempDbGroup; /* pointer, dont free */
} GWEN_XML2DB_CONTEXT;




static GWEN_XML2DB_CONTEXT *GWEN_Xml2Db_Context_new(GWEN_XMLNODE *documentRoot, GWEN_DB_NODE *dbRoot);
static void GWEN_Xml2Db_Context_free(GWEN_XML2DB_CONTEXT *ctx);

static void GWEN_Xml2Db_Context_EnterDocNode(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);
static void GWEN_Xml2Db_Context_LeaveDocNode(GWEN_XML2DB_CONTEXT *ctx);


static const char *GWEN_Xml2Db_GetCharValueByPath(GWEN_XMLNODE *xmlNode, const char *path, const char *defValue);
static int GWEN_Xml2Db_ConvertAndSetCharValue(GWEN_XML2DB_CONTEXT *ctx,
                                              GWEN_XMLNODE *xmlNode, GWEN_DB_NODE *dbCurrent,
                                              const char *value);

static int GWEN_Xml2Db_Handle_DbSetCharValue_internal(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode,
                                                      GWEN_DB_NODE *dbCurrent);


static int GWEN_Xml2Db_Handle_XmlEnter(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);
static int GWEN_Xml2Db_Handle_XmlForEvery(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);
static int GWEN_Xml2Db_Handle_DbCreateAndEnterGroup(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);
static int GWEN_Xml2Db_Handle_DbCreateAndEnterTempGroup(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);
static int GWEN_Xml2Db_Handle_DbSetCharValue(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);
static int GWEN_Xml2Db_Handle_DbSetTempCharValue(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);
static int GWEN_Xml2Db_Handle_XmlIfCharDataMatches(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);
static int GWEN_Xml2Db_Handle_XmlIfNotCharDataMatches(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);
static int GWEN_Xml2Db_Handle_XmlIfHasCharData(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);
static int GWEN_Xml2Db_Handle_XmlIfNotHasCharData(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);
static int GWEN_Xml2Db_Handle_XmlIfPathExists(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);
static int GWEN_Xml2Db_Handle_XmlIfNotPathExists(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);

static int GWEN_Xml2Db_HandleChildren(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);



#endif /* GWEN_XML2DB_P_H */
