/***************************************************************************
    begin       : Sat Apr 18 2018
    copyright   : (C) 2020 by Martin Preuss
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


#ifndef GWEN_XMLCMD_GXML_H
#define GWEN_XMLCMD_GXML_H


#include <gwenhywfar/xmlcmd.h>

#include <gwenhywfar/db.h>
#include <gwenhywfar/xml.h>


#ifdef __cplusplus
extern "C" {
#endif


GWENHYWFAR_API GWEN_XMLCOMMANDER *GWEN_XmlCommanderGwenXml_new(GWEN_XMLNODE *documentRoot, GWEN_DB_NODE *dbRoot);


GWENHYWFAR_API GWEN_XMLNODE *GWEN_XmlCommanderGwenXml_GetDocRoot(const GWEN_XMLCOMMANDER *cmd);

GWENHYWFAR_API GWEN_XMLNODE *GWEN_XmlCommanderGwenXml_GetCurrentDocNode(const GWEN_XMLCOMMANDER *cmd);
GWENHYWFAR_API void GWEN_XmlCommanderGwenXml_SetCurrentDocNode(GWEN_XMLCOMMANDER *cmd, GWEN_XMLNODE *n);

GWENHYWFAR_API GWEN_DB_NODE *GWEN_XmlCommanderGwenXml_GetDbRoot(const GWEN_XMLCOMMANDER *cmd);

GWENHYWFAR_API GWEN_DB_NODE *GWEN_XmlCommanderGwenXml_GetCurrentDbGroup(const GWEN_XMLCOMMANDER *cmd);
GWENHYWFAR_API void GWEN_XmlCommanderGwenXml_SetCurrentDbGroup(GWEN_XMLCOMMANDER *cmd, GWEN_DB_NODE *db);

GWENHYWFAR_API GWEN_DB_NODE *GWEN_XmlCommanderGwenXml_GetTempDbRoot(const GWEN_XMLCOMMANDER *cmd);
GWENHYWFAR_API void GWEN_XmlCommanderGwenXml_SetTempDbRoot(GWEN_XMLCOMMANDER *cmd, GWEN_DB_NODE *db);

GWENHYWFAR_API GWEN_DB_NODE *GWEN_XmlCommanderGwenXml_GetCurrentTempDbGroup(const GWEN_XMLCOMMANDER *cmd);
GWENHYWFAR_API void GWEN_XmlCommanderGwenXml_SetCurrentTempDbGroup(GWEN_XMLCOMMANDER *cmd, GWEN_DB_NODE *db);


GWENHYWFAR_API void GWEN_XmlCommanderGwenXml_EnterDocNode(GWEN_XMLCOMMANDER *cmd, GWEN_XMLNODE *xmlNode);
GWENHYWFAR_API void GWEN_XmlCommanderGwenXml_LeaveDocNode(GWEN_XMLCOMMANDER *cmd);




#ifdef __cplusplus
}
#endif



#endif /* GWEN_XML2DB_H */
