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


#ifndef GWEN_XMLCMD_LXML_H
#define GWEN_XMLCMD_LXML_H


#include <gwenhywfar/xmlcmd.h>

#include <gwenhywfar/db.h>
#include <gwenhywfar/xml.h>

#include <libxml/tree.h>


#ifdef __cplusplus
extern "C" {
#endif


GWENHYWFAR_API GWEN_XMLCOMMANDER *GWEN_XmlCommanderLibXml_new(xmlNodePtr documentRoot, GWEN_DB_NODE *dbRoot);


GWENHYWFAR_API xmlNodePtr GWEN_XmlCommanderLibXml_GetDocRoot(const GWEN_XMLCOMMANDER *cmd);

GWENHYWFAR_API xmlNodePtr GWEN_XmlCommanderLibXml_GetCurrentDocNode(const GWEN_XMLCOMMANDER *cmd);
GWENHYWFAR_API void GWEN_XmlCommanderLibXml_SetCurrentDocNode(GWEN_XMLCOMMANDER *cmd, xmlNodePtr n);

GWENHYWFAR_API GWEN_DB_NODE *GWEN_XmlCommanderLibXml_GetDbRoot(const GWEN_XMLCOMMANDER *cmd);

GWENHYWFAR_API GWEN_DB_NODE *GWEN_XmlCommanderLibXml_GetCurrentDbGroup(const GWEN_XMLCOMMANDER *cmd);
GWENHYWFAR_API void GWEN_XmlCommanderLibXml_SetCurrentDbGroup(GWEN_XMLCOMMANDER *cmd, GWEN_DB_NODE *db);

GWENHYWFAR_API GWEN_DB_NODE *GWEN_XmlCommanderLibXml_GetTempDbRoot(const GWEN_XMLCOMMANDER *cmd);
GWENHYWFAR_API void GWEN_XmlCommanderLibXml_SetTempDbRoot(GWEN_XMLCOMMANDER *cmd, GWEN_DB_NODE *db);

GWENHYWFAR_API GWEN_DB_NODE *GWEN_XmlCommanderLibXml_GetCurrentTempDbGroup(const GWEN_XMLCOMMANDER *cmd);
GWENHYWFAR_API void GWEN_XmlCommanderLibXml_SetCurrentTempDbGroup(GWEN_XMLCOMMANDER *cmd, GWEN_DB_NODE *db);


GWENHYWFAR_API void GWEN_XmlCommanderLibXml_EnterDocNode(GWEN_XMLCOMMANDER *cmd, xmlNodePtr xNode);
GWENHYWFAR_API void GWEN_XmlCommanderLibXml_LeaveDocNode(GWEN_XMLCOMMANDER *cmd);




/** @name Helper Functions
 *
 */
/*@{*/
GWENHYWFAR_API xmlNodePtr GWEN_XmlCommanderLibXml_GetXmlNode(xmlNodePtr n, const char *path, uint32_t flags);
GWENHYWFAR_API xmlNodePtr GWEN_XmlCommanderLibXml_FindFirstElement(xmlNodePtr parent, const char *elemName);
GWENHYWFAR_API xmlNodePtr GWEN_XmlCommanderLibXml_FindNextElement(xmlNodePtr elem, const char *elemName);

GWENHYWFAR_API int GWEN_XmlCommanderLibXml_SetXmlCharValue(xmlNodePtr n, const char *path, const char *value);
GWENHYWFAR_API const char *GWEN_XmlCommanderLibXml_GetXmlCharValue(xmlNodePtr n, const char *path, const char *defValue);

GWENHYWFAR_API int GWEN_XmlCommanderLibXml_SetIntValue(xmlNodePtr n, const char *path, int value);
GWENHYWFAR_API int GWEN_XmlCommanderLibXml_GetIntValue(xmlNodePtr n, const char *path, int defValue);

GWENHYWFAR_API const char *GWEN_XmlCommanderLibXml_GetXmlCharValueByPath(xmlNodePtr elem, const char *path, const char *defValue);
GWENHYWFAR_API void GWEN_XmlCommanderLibXml_SetXmlCharValueByPath(xmlNodePtr elem, const char *path, const char *value);

/*@}*/






#ifdef __cplusplus
}
#endif



#endif /* GWEN_XML2DB_H */
