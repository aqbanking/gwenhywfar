/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sat Nov 15 2003
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

#ifndef GWENHYWFAR_IPCXMSECCTX_H
#define GWENHYWFAR_IPCXMSECCTX_H


#include <gwenhywfar/crypt.h>
#include <gwenhywfar/keyspec.h>
#include <gwenhywfar/seccontext.h>
#include <gwenhywfar/msgengine.h>
#include <gwenhywfar/ipcxmlkeymanager.h>


typedef struct GWEN_IPCXMLSECCTXDATA GWEN_IPCXMLSECCTXDATA;


GWEN_SECCTX *GWEN_IPCXMLSecCtx_new(const char *localName,
                                   const char *remoteName);


int GWEN_IPCXMLSecCtx_AddKey(GWEN_SECCTX *d,
                             const GWEN_CRYPTKEY *k);
int GWEN_IPCXMLSecCtx_DelKey(GWEN_SECCTX *d,
                             const GWEN_CRYPTKEY *k);


GWEN_CRYPTKEY *GWEN_IPCXMLSecCtx_GetSessionKey(GWEN_SECCTX *d);
void GWEN_IPCXMLSecCtx_SetSessionKey(GWEN_SECCTX *d,
                                     GWEN_CRYPTKEY *k);

const char *GWEN_IPCXMLSecCtx_GetServiceCode(GWEN_SECCTX *d);
void GWEN_IPCXMLSecCtx_SetServiceCode(GWEN_SECCTX *d,
                                      const char *s);

#endif






