/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Nov 17 2003
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

#ifndef GWENHYWFAR_IPCXMLKEYMANAGER_P_H
#define GWENHYWFAR_IPCXMLKEYMANAGER_P_H


#include <gwenhywfar/ipcxmlkeymanager.h>
#include <gwenhywfar/list.h>


typedef struct GWEN_IPCXMLKEYMANAGERDATA GWEN_IPCXMLKEYMANAGERDATA;

struct GWEN_IPCXMLKEYMANAGERDATA {
  GWEN_LIST *keys;

};

const GWEN_CRYPTKEY *GWEN_IPCXMLKeyManager_GetKey(GWEN_KEYMANAGER *km,
                                                  const GWEN_KEYSPEC *ks);
int GWEN_IPCXMLKeyManager_AddKey(GWEN_KEYMANAGER *km,
                                 const GWEN_CRYPTKEY *key);
int GWEN_IPCXMLKeyManager_DeleteKey(GWEN_KEYMANAGER *km,
                                    const GWEN_CRYPTKEY *key);
void GWEN_IPCXMLKeyManager_FreeData(GWEN_KEYMANAGER *km);

GWEN_IPCXMLKEYMANAGERDATA *GWEN_IPCXMLKeyManagerData_new();

#endif


