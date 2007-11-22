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

#ifndef GWENHYWFAR_XMLCTX_L_H
#define GWENHYWFAR_XMLCTX_L_H


#include "xmlctx.h"



int GWEN_XmlCtx_StartTag(GWEN_XML_CONTEXT *ctx, const char *tagName);
int GWEN_XmlCtx_EndTag(GWEN_XML_CONTEXT *ctx, int closing);
int GWEN_XmlCtx_AddData(GWEN_XML_CONTEXT *ctx, const char *data);
int GWEN_XmlCtx_AddComment(GWEN_XML_CONTEXT *ctx, const char *data);
int GWEN_XmlCtx_AddAttr(GWEN_XML_CONTEXT *ctx,
			const char *attrName,
			const char *attrData);


#endif

