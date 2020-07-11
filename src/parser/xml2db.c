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



#include "xml2db.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/gwendate.h>
#include <gwenhywfar/xmlcmd_gxml_fromdb.h>
#include <gwenhywfar/xmlcmd_gxml_todb.h>


#include <ctype.h>




int GWEN_Xml2Db(GWEN_XMLNODE *xmlNodeDocument,
                GWEN_XMLNODE *xmlNodeSchema,
                GWEN_DB_NODE *dbDestination)
{
  GWEN_XMLCOMMANDER *cmd;
  int rv;

  cmd=GWEN_XmlCommanderGwenXml_toDb_new(xmlNodeDocument, dbDestination);

  rv=GWEN_XmlCommander_HandleChildren(cmd, xmlNodeSchema);
  GWEN_XmlCommander_free(cmd);

  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int GWEN_XmlFromDb(GWEN_XMLNODE *xmlNodeDestination,
		   GWEN_XMLNODE *xmlNodeSchema,
		   GWEN_DB_NODE *dbSource)
{
  GWEN_XMLCOMMANDER *cmd;
  int rv;

  cmd=GWEN_XmlCommanderGwenXml_fromDb_new(xmlNodeDestination, dbSource);

  rv=GWEN_XmlCommander_HandleChildren(cmd, xmlNodeSchema);
  GWEN_XmlCommander_free(cmd);

  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}


