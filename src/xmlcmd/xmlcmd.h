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


#ifndef GWEN_XMLCMD_H
#define GWEN_XMLCMD_H


#include <gwenhywfar/db.h>
#include <gwenhywfar/xml.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef struct GWEN_XMLCOMMANDER GWEN_XMLCOMMANDER;
GWEN_INHERIT_FUNCTION_LIB_DEFS(GWEN_XMLCOMMANDER, GWENHYWFAR_API)

typedef int (*GWEN_XMLCMD_HANDLECHILDREN_FN)(GWEN_XMLCOMMANDER *cmd, GWEN_XMLNODE *xmlNode);



GWENHYWFAR_API GWEN_XMLCOMMANDER *GWEN_XmlCommander_new(void);

GWENHYWFAR_API void GWEN_XmlCommander_free(GWEN_XMLCOMMANDER *cmd);


GWENHYWFAR_API GWEN_XMLCMD_HANDLECHILDREN_FN GWEN_XmlCommander_SetHandleChildrenFn(GWEN_XMLCOMMANDER *cmd,
                                                                                   GWEN_XMLCMD_HANDLECHILDREN_FN f);



GWENHYWFAR_API int GWEN_XmlCommander_HandleChildren(GWEN_XMLCOMMANDER *cmd, GWEN_XMLNODE *xmlNode);



#ifdef __cplusplus
}
#endif



#endif /* GWEN_XML2DB_H */
