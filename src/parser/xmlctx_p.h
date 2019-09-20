/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id: xsd.h 898 2005-11-03 09:51:39Z cstim $
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

#ifndef GWENHYWFAR_XMLCTX_P_H
#define GWENHYWFAR_XMLCTX_P_H


#include "xmlctx_l.h"


struct GWEN_XML_CONTEXT {
  GWEN_INHERIT_ELEMENT(GWEN_XML_CONTEXT)

  uint32_t flags;
  char *encoding;
  GWEN_XMLNODE *currentNode;
  GWEN_XMLNODE *currentHeader;

  GWEN_XMLCTX_STARTTAG_FN startTagFn;
  GWEN_XMLCTX_ENDTAG_FN endTagFn;
  GWEN_XMLCTX_ADDDATA_FN addDataFn;
  GWEN_XMLCTX_ADDCOMMENT_FN addCommentFn;
  GWEN_XMLCTX_ADDATTR_FN addAttrFn;

  int finishedElements;

  int depth;

  int _refCount;
};



static int GWEN_XmlCtxStore_StartTag(GWEN_XML_CONTEXT *ctx,
                                     const char *tagName);
static int GWEN_XmlCtxStore_EndTag(GWEN_XML_CONTEXT *ctx, int closing);
static int GWEN_XmlCtxStore_AddData(GWEN_XML_CONTEXT *ctx,
                                    const char *data);
static int GWEN_XmlCtxStore_AddComment(GWEN_XML_CONTEXT *ctx,
                                       const char *data);
static int GWEN_XmlCtxStore_AddAttr(GWEN_XML_CONTEXT *ctx,
                                    const char *attrName,
                                    const char *attrData);



#endif

