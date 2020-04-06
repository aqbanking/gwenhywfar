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


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



#include "xml2db_p.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/gwendate.h>


#include <ctype.h>







GWEN_XML2DB_CONTEXT *GWEN_Xml2Db_Context_new(GWEN_XMLNODE *documentRoot, GWEN_DB_NODE *dbRoot)
{
  GWEN_XML2DB_CONTEXT *ctx;

  GWEN_NEW_OBJECT(GWEN_XML2DB_CONTEXT, ctx);
  assert(ctx);

  ctx->docRoot=documentRoot;
  ctx->xmlNodeStack=GWEN_XMLNode_List2_new();
  ctx->dbRoot=dbRoot;
  ctx->tempDbRoot=GWEN_DB_Group_new("dbTempRoot");

  ctx->currentDbGroup=ctx->dbRoot;
  ctx->currentTempDbGroup=ctx->tempDbRoot;
  ctx->currentDocNode=documentRoot;

  return ctx;
}



void GWEN_Xml2Db_Context_free(GWEN_XML2DB_CONTEXT *ctx)
{
  if (ctx) {
    GWEN_XMLNode_List2_free(ctx->xmlNodeStack);
    ctx->xmlNodeStack=NULL;

    GWEN_DB_Group_free(ctx->tempDbRoot);
    GWEN_FREE_OBJECT(ctx);
  }
}



void GWEN_Xml2Db_Context_EnterDocNode(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode)
{
  assert(ctx);
  assert(xmlNode);

  GWEN_XMLNode_List2_PushBack(ctx->xmlNodeStack, ctx->currentDocNode);
  ctx->currentDocNode=xmlNode;
}



void GWEN_Xml2Db_Context_LeaveDocNode(GWEN_XML2DB_CONTEXT *ctx)
{
  GWEN_XMLNODE *xmlNode;

  assert(ctx);

  xmlNode=GWEN_XMLNode_List2_GetBack(ctx->xmlNodeStack);
  if (xmlNode==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Nothing on stack");
    assert(xmlNode);
  }
  ctx->currentDocNode=xmlNode;
  GWEN_XMLNode_List2_PopBack(ctx->xmlNodeStack);
}



int GWEN_Xml2Db_Context_HandleChildren(GWEN_XML2DB_CONTEXT *ctx, GWEN_XMLNODE *xmlNode)
{
  if (ctx->handleChildrenFn)
    return (ctx->handleChildrenFn)(ctx, xmlNode);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}





