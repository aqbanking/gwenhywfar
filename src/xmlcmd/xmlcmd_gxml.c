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



#include "xmlcmd_gxml_p.h"

#include <gwenhywfar/debug.h>


#include <ctype.h>


GWEN_INHERIT(GWEN_XMLCOMMANDER, GWEN_XMLCMD_GXML)




static void GWENHYWFAR_CB _freeData(void *bp, void *p);






GWEN_XMLCOMMANDER *GWEN_XmlCommanderGwenXml_new(GWEN_XMLNODE *documentRoot, GWEN_DB_NODE *dbRoot)
{
  GWEN_XMLCOMMANDER *cmd;
  GWEN_XMLCMD_GXML *xcmd;

  cmd=GWEN_XmlCommander_new();
  GWEN_NEW_OBJECT(GWEN_XMLCMD_GXML, xcmd);
  GWEN_INHERIT_SETDATA(GWEN_XMLCOMMANDER, GWEN_XMLCMD_GXML, cmd, xcmd, _freeData);

  xcmd->docRoot=documentRoot;
  xcmd->xmlNodeStack=GWEN_XMLNode_List2_new();
  xcmd->dbRoot=dbRoot;
  xcmd->tempDbRoot=GWEN_DB_Group_new("dbTempRoot");

  xcmd->currentDbGroup=xcmd->dbRoot;
  xcmd->currentTempDbGroup=xcmd->tempDbRoot;
  xcmd->currentDocNode=documentRoot;

  return cmd;
}



void _freeData(GWEN_UNUSED void *bp, void *p)
{
  GWEN_XMLCMD_GXML *xcmd;

  xcmd=(GWEN_XMLCMD_GXML*) p;

  GWEN_XMLNode_List2_free(xcmd->xmlNodeStack);
  xcmd->xmlNodeStack=NULL;

  GWEN_DB_Group_free(xcmd->tempDbRoot);
  GWEN_FREE_OBJECT(xcmd);
}



GWEN_XMLNODE *GWEN_XmlCommanderGwenXml_GetDocRoot(const GWEN_XMLCOMMANDER *cmd)
{
  GWEN_XMLCMD_GXML *xcmd;

  assert(cmd);
  xcmd=GWEN_INHERIT_GETDATA(GWEN_XMLCOMMANDER, GWEN_XMLCMD_GXML, cmd);
  assert(xcmd);

  return xcmd->docRoot;
}



GWEN_XMLNODE *GWEN_XmlCommanderGwenXml_GetCurrentDocNode(const GWEN_XMLCOMMANDER *cmd)
{
  GWEN_XMLCMD_GXML *xcmd;

  assert(cmd);
  xcmd=GWEN_INHERIT_GETDATA(GWEN_XMLCOMMANDER, GWEN_XMLCMD_GXML, cmd);
  assert(xcmd);

  return xcmd->currentDocNode;
}



void GWEN_XmlCommanderGwenXml_SetCurrentDocNode(GWEN_XMLCOMMANDER *cmd, GWEN_XMLNODE *n)
{
  GWEN_XMLCMD_GXML *xcmd;

  assert(cmd);
  xcmd=GWEN_INHERIT_GETDATA(GWEN_XMLCOMMANDER, GWEN_XMLCMD_GXML, cmd);
  assert(xcmd);

  xcmd->currentDocNode=n;
}



GWEN_DB_NODE *GWEN_XmlCommanderGwenXml_GetDbRoot(const GWEN_XMLCOMMANDER *cmd)
{
  GWEN_XMLCMD_GXML *xcmd;

  assert(cmd);
  xcmd=GWEN_INHERIT_GETDATA(GWEN_XMLCOMMANDER, GWEN_XMLCMD_GXML, cmd);
  assert(xcmd);

  return xcmd->dbRoot;
}



GWEN_DB_NODE *GWEN_XmlCommanderGwenXml_GetCurrentDbGroup(const GWEN_XMLCOMMANDER *cmd)
{
  GWEN_XMLCMD_GXML *xcmd;

  assert(cmd);
  xcmd=GWEN_INHERIT_GETDATA(GWEN_XMLCOMMANDER, GWEN_XMLCMD_GXML, cmd);
  assert(xcmd);

  return xcmd->currentDbGroup;
}



void GWEN_XmlCommanderGwenXml_SetCurrentDbGroup(GWEN_XMLCOMMANDER *cmd, GWEN_DB_NODE *db)
{
  GWEN_XMLCMD_GXML *xcmd;

  assert(cmd);
  xcmd=GWEN_INHERIT_GETDATA(GWEN_XMLCOMMANDER, GWEN_XMLCMD_GXML, cmd);
  assert(xcmd);

  xcmd->currentDbGroup=db;
}



GWEN_DB_NODE *GWEN_XmlCommanderGwenXml_GetTempDbRoot(const GWEN_XMLCOMMANDER *cmd)
{
  GWEN_XMLCMD_GXML *xcmd;

  assert(cmd);
  xcmd=GWEN_INHERIT_GETDATA(GWEN_XMLCOMMANDER, GWEN_XMLCMD_GXML, cmd);
  assert(xcmd);

  return xcmd->tempDbRoot;
}



GWEN_DB_NODE *GWEN_XmlCommanderGwenXml_GetCurrentTempDbGroup(const GWEN_XMLCOMMANDER *cmd)
{
  GWEN_XMLCMD_GXML *xcmd;

  assert(cmd);
  xcmd=GWEN_INHERIT_GETDATA(GWEN_XMLCOMMANDER, GWEN_XMLCMD_GXML, cmd);
  assert(xcmd);

  return xcmd->currentTempDbGroup;
}



void GWEN_XmlCommanderGwenXml_SetCurrentTempDbGroup(GWEN_XMLCOMMANDER *cmd, GWEN_DB_NODE *db)
{
  GWEN_XMLCMD_GXML *xcmd;

  assert(cmd);
  xcmd=GWEN_INHERIT_GETDATA(GWEN_XMLCOMMANDER, GWEN_XMLCMD_GXML, cmd);
  assert(xcmd);

  xcmd->currentTempDbGroup=db;
}



void GWEN_XmlCommanderGwenXml_EnterDocNode(GWEN_XMLCOMMANDER *cmd, GWEN_XMLNODE *xmlNode)
{
  GWEN_XMLCMD_GXML *xcmd;

  assert(cmd);
  xcmd=GWEN_INHERIT_GETDATA(GWEN_XMLCOMMANDER, GWEN_XMLCMD_GXML, cmd);
  assert(xcmd);

  assert(xmlNode);

  GWEN_XMLNode_List2_PushBack(xcmd->xmlNodeStack, xcmd->currentDocNode);
  xcmd->currentDocNode=xmlNode;
}



void GWEN_XmlCommanderGwenXml_LeaveDocNode(GWEN_XMLCOMMANDER *cmd)
{
  GWEN_XMLCMD_GXML *xcmd;
  GWEN_XMLNODE *xmlNode;

  assert(cmd);
  xcmd=GWEN_INHERIT_GETDATA(GWEN_XMLCOMMANDER, GWEN_XMLCMD_GXML, cmd);
  assert(xcmd);

  xmlNode=GWEN_XMLNode_List2_GetBack(xcmd->xmlNodeStack);
  if (xmlNode==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Nothing on stack");
    assert(xmlNode);
  }
  xcmd->currentDocNode=xmlNode;
  GWEN_XMLNode_List2_PopBack(xcmd->xmlNodeStack);
}








