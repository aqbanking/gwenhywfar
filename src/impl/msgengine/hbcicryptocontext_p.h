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

#ifndef GWENHYWFAR_HBCICRYPTOCONTEXT_P_H
#define GWENHYWFAR_HBCICRYPTOCONTEXT_P_H

#define GWEN_HBCIMSG_DEFAULTSIZE 256

#include <gwenhywfar/hbcimsg.h>
#include <gwenhywfar/error.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/msgengine.h>





struct GWEN_HBCICRYPTOCONTEXT {
  char *serviceCode; /* bank code for HBCI */
  char *pSecurityId;
  unsigned int lSecurityId;
  char *pCryptKey;
  unsigned int lCryptKey;
  unsigned int seq;
  GWEN_KEYSPEC *key;
  char *mode;
};




#endif



