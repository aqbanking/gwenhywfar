/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Wed Nov 19 2003
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



#ifndef GWENHYWFAR_SECCONTEXT_P_H
#define GWENHYWFAR_SECCONTEXT_P_H

#include <gwenhyfwar/seccontext.h>
#include <gwenhyfwar/list.h>


#define GWEN_SECCONTEXT_KEYSIZE 96



struct GWEN_SECCTX {
  char *localName;
  char *remoteName;
  unsigned int localSignSeq;
  unsigned int remoteSignSeq;
  void *data;

  GWEN_SECCTX_PREPARECTX_FN prepareFn;
  GWEN_SECCTX_SIGN_FN signFn;
  GWEN_SECCTX_VERIFY_FN verifyFn;
  GWEN_SECCTX_ENCRYPT_FN encryptFn;
  GWEN_SECCTX_DECRYPT_FN decryptFn;
  GWEN_SECCTX_FREEDATA_FN freeDataFn;

};




struct GWEN_SECCTX_MANAGER {
  GWEN_LIST *contextList;
  char *serviceCode;
  void *data;
  GWEN_SECCTXMGR_GETCONTEXT_FN getContextFn;
  GWEN_SECCTXMGR_ADDCONTEXT_FN addContextFn;
  GWEN_SECCTXMGR_DELCONTEXT_FN delContextFn;
  GWEN_SECCTXMGR_FREEDATA_FN freeDataFn;

};





#endif



