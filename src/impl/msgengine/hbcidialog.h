/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sat Nov 08 2003
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

#ifndef GWENHYWFAR_HBCIDLG_H
#define GWENHYWFAR_HBCIDLG_H

#define GWEN_HBCIDIALOG_FLAGS_INITIATOR 0x0001


#include <gwenhywfar/msgengine.h>
#include <gwenhywfar/seccontext.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct GWEN_HBCIDIALOG GWEN_HBCIDIALOG;


typedef void
  (*GWEN_HBCIDLG_FREEDATA_FN)(GWEN_HBCIDIALOG *hdlg);

typedef void
  (*GWEN_HBCIDLG_RESET_FN)(GWEN_HBCIDIALOG *hdlg);



void GWEN_HBCIDialog_SetFreeDataFn(GWEN_HBCIDIALOG *hdlg,
                                   GWEN_HBCIDLG_FREEDATA_FN fn);
void GWEN_HBCIDialog_SetResetFn(GWEN_HBCIDIALOG *hdlg,
                                GWEN_HBCIDLG_RESET_FN fn);
void GWEN_HBCIDialog_SetInheritorData(GWEN_HBCIDIALOG *hdlg,
                                      void *data);
void *GWEN_HBCIDialog_GetInheritorData(GWEN_HBCIDIALOG *hdlg);

GWEN_MSGENGINE *GWEN_HBCIDialog_GetMsgEngine(GWEN_HBCIDIALOG *hdlg);
void GWEN_HBCIDialog_SetMsgEngine(GWEN_HBCIDIALOG *hdlg,
                                  GWEN_MSGENGINE *e);

const char *GWEN_HBCIDialog_GetDialogId(GWEN_HBCIDIALOG *hdlg);
void GWEN_HBCIDialog_SetDialogId(GWEN_HBCIDIALOG *hdlg,
                                 const char *s);

unsigned int GWEN_HBCIDialog_GetLastReceivedMsgNum(GWEN_HBCIDIALOG *hdlg);
void GWEN_HBCIDialog_SetLastReceivedMsgNum(GWEN_HBCIDIALOG *hdlg,
                                           unsigned int i);

unsigned int GWEN_HBCIDialog_GetNextMsgNum(GWEN_HBCIDIALOG *hdlg);
void GWEN_HBCIDialog_SetNextMsgNum(GWEN_HBCIDIALOG *hdlg,
                                   unsigned int i);

void GWEN_HBCIDialog_Reset(GWEN_HBCIDIALOG *hdlg);


GWEN_HBCIDIALOG *GWEN_HBCIDialog_new(GWEN_MSGENGINE *e,
                                     GWEN_SECCTX_MANAGER *scm);
void GWEN_HBCIDialog_free(GWEN_HBCIDIALOG *hdlg);

unsigned int GWEN_HBCIDialog_GetFlags(GWEN_HBCIDIALOG *hdlg);
void GWEN_HBCIDialog_SetFlags(GWEN_HBCIDIALOG *hdlg,
                              unsigned int f);

GWEN_SECCTX_MANAGER*
  GWEN_HBCIDialog_GetSecurityManager(GWEN_HBCIDIALOG *hdlg);


void GWEN_HBCIDialog_Attach(GWEN_HBCIDIALOG *hdlg);
void GWEN_HBCIDialog_Detach(GWEN_HBCIDIALOG *hdlg);


const char *GWEN_HBCIDialog_GetLocalName(GWEN_HBCIDIALOG *hdlg);
void GWEN_HBCIDialog_SetLocalName(GWEN_HBCIDIALOG *hdlg,
                                  const char *s);

const char *GWEN_HBCIDialog_GetRemoteName(GWEN_HBCIDIALOG *hdlg);
void GWEN_HBCIDialog_SetRemoteName(GWEN_HBCIDIALOG *hdlg,
                                   const char *s);


#ifdef __cplusplus
}
#endif


#endif



