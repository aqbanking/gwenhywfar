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

#ifndef GWENHYWFAR_HBCIDLG_P_H
#define GWENHYWFAR_HBCIDLG_P_H


#include <gwenhyfwar/hbcidialog.h>


struct GWEN_HBCIDIALOG {
  GWEN_MSGENGINE *msgEngine;
  char *dialogId;
  unsigned int lastReceivedMsgNum;
  unsigned int nextMsgNum;

  void *inheritorData;

  GWEN_HBCIDLG_PREPARECTX_FN prepareCtxFn;
  GWEN_HBCIDLG_SIGN_FN signFn;
  GWEN_HBCIDLG_VERIFY_FN verifyFn;
  GWEN_HBCIDLG_ENCRYPT_FN encryptFn;
  GWEN_HBCIDLG_DECRYPT_FN decryptFn;
  GWEN_HBCIDLG_FREEDATA_FN freeDataFn;
};




#endif



