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



#include "xmlcmd_p.h"

#include <gwenhywfar/debug.h>


#include <ctype.h>



GWEN_INHERIT_FUNCTIONS(GWEN_XMLCOMMANDER)




GWEN_XMLCOMMANDER *GWEN_XmlCommander_new(void)
{
  GWEN_XMLCOMMANDER *cmd;

  GWEN_NEW_OBJECT(GWEN_XMLCOMMANDER, cmd);
  assert(cmd);
  GWEN_INHERIT_INIT(GWEN_XMLCOMMANDER, cmd);

  return cmd;
}



void GWEN_XmlCommander_free(GWEN_XMLCOMMANDER *cmd)
{
  if (cmd) {
    GWEN_INHERIT_FINI(GWEN_XMLCOMMANDER, cmd);
    GWEN_FREE_OBJECT(cmd);
  }
}



GWEN_XMLCMD_HANDLECHILDREN_FN GWEN_XmlCommander_SetHandleChildrenFn(GWEN_XMLCOMMANDER *cmd, GWEN_XMLCMD_HANDLECHILDREN_FN f)
{
  GWEN_XMLCMD_HANDLECHILDREN_FN oldFn;

  oldFn=cmd->handleChildrenFn;
  cmd->handleChildrenFn=f;
  return oldFn;
}



int GWEN_XmlCommander_HandleChildren(GWEN_XMLCOMMANDER *cmd, GWEN_XMLNODE *xmlNode)
{
  if (cmd->handleChildrenFn)
    return (cmd->handleChildrenFn)(cmd, xmlNode);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}





