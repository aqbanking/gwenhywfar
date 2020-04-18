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


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



#include "xmlcmd_lxml_p.h"

#include <gwenhywfar/debug.h>


#include <ctype.h>


GWEN_INHERIT(GWEN_XMLCOMMANDER, GWEN_XMLCMD_LXML)




static void GWENHYWFAR_CB _freeData(void *bp, void *p);






GWEN_XMLCOMMANDER *GWEN_XmlCommanderLibXml_new(xmlNodePtr documentRoot, GWEN_DB_NODE *dbRoot)
{
  GWEN_XMLCOMMANDER *cmd;
  GWEN_XMLCMD_LXML *xcmd;

  cmd=GWEN_XmlCommander_new();
  GWEN_NEW_OBJECT(GWEN_XMLCMD_LXML, xcmd);
  GWEN_INHERIT_SETDATA(GWEN_XMLCOMMANDER, GWEN_XMLCMD_LXML, cmd, xcmd, _freeData);

  xcmd->docRoot=documentRoot;
  xcmd->dbRoot=dbRoot;
  xcmd->tempDbRoot=GWEN_DB_Group_new("dbTempRoot");

  xcmd->currentDbGroup=xcmd->dbRoot;
  xcmd->currentTempDbGroup=xcmd->tempDbRoot;
  xcmd->currentDocNode=documentRoot;

  return cmd;
}



void _freeData(GWEN_UNUSED void *bp, void *p)
{
  GWEN_XMLCMD_LXML *xcmd;

  xcmd=(GWEN_XMLCMD_LXML*) p;

  GWEN_DB_Group_free(xcmd->tempDbRoot);
  GWEN_FREE_OBJECT(xcmd);
}



xmlNodePtr GWEN_XmlCommanderLibXml_GetDocRoot(const GWEN_XMLCOMMANDER *cmd)
{
  GWEN_XMLCMD_LXML *xcmd;

  assert(cmd);
  xcmd=GWEN_INHERIT_GETDATA(GWEN_XMLCOMMANDER, GWEN_XMLCMD_LXML, cmd);
  assert(xcmd);

  return xcmd->docRoot;
}



xmlNodePtr GWEN_XmlCommanderLibXml_GetCurrentDocNode(const GWEN_XMLCOMMANDER *cmd)
{
  GWEN_XMLCMD_LXML *xcmd;

  assert(cmd);
  xcmd=GWEN_INHERIT_GETDATA(GWEN_XMLCOMMANDER, GWEN_XMLCMD_LXML, cmd);
  assert(xcmd);

  return xcmd->currentDocNode;
}



void GWEN_XmlCommanderLibXml_SetCurrentDocNode(GWEN_XMLCOMMANDER *cmd, xmlNodePtr n)
{
  GWEN_XMLCMD_LXML *xcmd;

  assert(cmd);
  xcmd=GWEN_INHERIT_GETDATA(GWEN_XMLCOMMANDER, GWEN_XMLCMD_LXML, cmd);
  assert(xcmd);

  xcmd->currentDocNode=n;
}



GWEN_DB_NODE *GWEN_XmlCommanderLibXml_GetDbRoot(const GWEN_XMLCOMMANDER *cmd)
{
  GWEN_XMLCMD_LXML *xcmd;

  assert(cmd);
  xcmd=GWEN_INHERIT_GETDATA(GWEN_XMLCOMMANDER, GWEN_XMLCMD_LXML, cmd);
  assert(xcmd);

  return xcmd->dbRoot;
}



GWEN_DB_NODE *GWEN_XmlCommanderLibXml_GetCurrentDbGroup(const GWEN_XMLCOMMANDER *cmd)
{
  GWEN_XMLCMD_LXML *xcmd;

  assert(cmd);
  xcmd=GWEN_INHERIT_GETDATA(GWEN_XMLCOMMANDER, GWEN_XMLCMD_LXML, cmd);
  assert(xcmd);

  return xcmd->currentDbGroup;
}



void GWEN_XmlCommanderLibXml_SetCurrentDbGroup(GWEN_XMLCOMMANDER *cmd, GWEN_DB_NODE *db)
{
  GWEN_XMLCMD_LXML *xcmd;

  assert(cmd);
  xcmd=GWEN_INHERIT_GETDATA(GWEN_XMLCOMMANDER, GWEN_XMLCMD_LXML, cmd);
  assert(xcmd);

  xcmd->currentDbGroup=db;
}



GWEN_DB_NODE *GWEN_XmlCommanderLibXml_GetTempDbRoot(const GWEN_XMLCOMMANDER *cmd)
{
  GWEN_XMLCMD_LXML *xcmd;

  assert(cmd);
  xcmd=GWEN_INHERIT_GETDATA(GWEN_XMLCOMMANDER, GWEN_XMLCMD_LXML, cmd);
  assert(xcmd);

  return xcmd->tempDbRoot;
}



GWEN_DB_NODE *GWEN_XmlCommanderLibXml_GetCurrentTempDbGroup(const GWEN_XMLCOMMANDER *cmd)
{
  GWEN_XMLCMD_LXML *xcmd;

  assert(cmd);
  xcmd=GWEN_INHERIT_GETDATA(GWEN_XMLCOMMANDER, GWEN_XMLCMD_LXML, cmd);
  assert(xcmd);

  return xcmd->currentTempDbGroup;
}



void GWEN_XmlCommanderLibXml_SetCurrentTempDbGroup(GWEN_XMLCOMMANDER *cmd, GWEN_DB_NODE *db)
{
  GWEN_XMLCMD_LXML *xcmd;

  assert(cmd);
  xcmd=GWEN_INHERIT_GETDATA(GWEN_XMLCOMMANDER, GWEN_XMLCMD_LXML, cmd);
  assert(xcmd);

  xcmd->currentTempDbGroup=db;
}



void GWEN_XmlCommanderLibXml_EnterDocNode(GWEN_XMLCOMMANDER *cmd, xmlNodePtr xNode)
{
  GWEN_XMLCMD_LXML *xcmd;

  assert(cmd);
  xcmd=GWEN_INHERIT_GETDATA(GWEN_XMLCOMMANDER, GWEN_XMLCMD_LXML, cmd);
  assert(xcmd);

  assert(xNode);

  if (xcmd->currentStackPos<GWEN_XMLCMD_LXML_PATH_MAXDEPTH) {
    xcmd->xmlNodeStack[xcmd->currentStackPos]=xNode;
    xcmd->currentStackPos++;
    xcmd->currentDocNode=xNode;
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Stack full, SNH!");
    abort();
  }
}



void GWEN_XmlCommanderLibXml_LeaveDocNode(GWEN_XMLCOMMANDER *cmd)
{
  GWEN_XMLCMD_LXML *xcmd;
  xmlNodePtr xNode;

  assert(cmd);
  xcmd=GWEN_INHERIT_GETDATA(GWEN_XMLCOMMANDER, GWEN_XMLCMD_LXML, cmd);
  assert(xcmd);

  if (xcmd->currentStackPos>0) {
    xNode=xcmd->xmlNodeStack[xcmd->currentStackPos-1];
    if (xNode==NULL) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Nothing on stack");
      assert(xNode);
    }
    xcmd->currentStackPos--;
    xcmd->currentDocNode=xNode;
  }
}








