/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Thu Nov 06 2003
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


#ifndef GWENHYWFAR_CRYPTSSL_L_H
#define GWENHYWFAR_CRYPTSSL_L_H


#define GWEN_CRYPT_RSA_DEFAULT_KEY_LENGTH  1024
#define GWEN_CRYPT_RSA_MAX_KEY_LENGTH      4096
#define GWEN_CRYPT_RSA_DEFAULT_EXPONENT    65537


#include <gwenhywfar/crypt.h>
#include <stdio.h>

#include <openssl/bn.h>


GWEN_ERRORCODE GWEN_CryptImpl_Init();
GWEN_ERRORCODE GWEN_CryptImpl_Fini();

void GWEN_CryptImpl_Dump_Bignum(BIGNUM *bn, FILE *f, int indent);


#endif


