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

#ifndef GWENHYWFAR_IPCXMLDIALOG_H
#define GWENHYWFAR_IPCXMLDIALOG_H


#include <gwenhyfwar/crypt.h>
#include <gwenhyfwar/hbcidialog.h>
#include <gwenhyfwar/msgengine.h>


typedef struct GWEN_IPCXMLDIALOGDATA GWEN_IPCXMLDIALOGDATA;


GWEN_HBCIDIALOG *GWEN_IPCXMLDialog_new(GWEN_MSGENGINE *e);


GWEN_CRYPTKEY *GWEN_IPCXMLDialog_GetLocalKey(GWEN_HBCIDIALOG *d);
void GWEN_IPCXMLDialog_SetLocalKey(GWEN_HBCIDIALOG *d,
                                   GWEN_CRYPTKEY *k);

GWEN_CRYPTKEY *GWEN_IPCXMLDialog_GetRemoteKey(GWEN_HBCIDIALOG *d);
void GWEN_IPCXMLDialog_SetRemoteKey(GWEN_HBCIDIALOG *d,
                                    GWEN_CRYPTKEY *k);

GWEN_CRYPTKEY *GWEN_IPCXMLDialog_GetSessionKey(GWEN_HBCIDIALOG *d);
void GWEN_IPCXMLDialog_SetSessionKey(GWEN_HBCIDIALOG *d,
                                     GWEN_CRYPTKEY *k);

unsigned int GWEN_IPCXMLDialog_GetFlags(GWEN_HBCIDIALOG *d);
void GWEN_IPCXMLDialog_SetFlags(GWEN_HBCIDIALOG *d,
                                unsigned int f);

unsigned int GWEN_IPCXMLDialog_GetLocalSignSeq(GWEN_HBCIDIALOG *d);
void GWEN_IPCXMLDialog_SetLocalSignSeq(GWEN_HBCIDIALOG *d,
                                       unsigned int i);

unsigned int GWEN_IPCXMLDialog_GetRemoteSignSeq(GWEN_HBCIDIALOG *d);
void GWEN_IPCXMLDialog_SetRemoteSignSeq(GWEN_HBCIDIALOG *d,
                                        unsigned int i);


#endif






