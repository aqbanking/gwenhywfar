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


#ifndef GWEN_XMLCMD_GXMLP_H
#define GWEN_XMLCMD_GXMLP_H

#include <gwenhywfar/xmlcmd_gxml.h>

#include <gwenhywfar/db.h>
#include <gwenhywfar/xml.h>



typedef struct GWEN_XMLCMD_GXML GWEN_XMLCMD_GXML;
struct GWEN_XMLCMD_GXML {
  GWEN_XMLNODE *docRoot;            /* provided by caller (dont free) */
  GWEN_XMLNODE *currentDocNode;     /* pointer, dont free */

  GWEN_XMLNODE_LIST2 *xmlNodeStack;  /* do free */

  GWEN_DB_NODE *dbRoot;             /* provided by caller (dont free) */
  GWEN_DB_NODE *currentDbGroup;     /* pointer, dont free */

  GWEN_DB_NODE *tempDbRoot;         /* do free */
  GWEN_DB_NODE *currentTempDbGroup; /* pointer, dont free */

};





#endif /* GWEN_XML2DB_P_H */
