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

#ifndef GWENHYWFAR_HBCICRYPTOCONTEXT_H
#define GWENHYWFAR_HBCICRYPTOCONTEXT_H


#include <gwenhyfwar/keyspec.h>


typedef struct GWEN_HBCICRYPTOCONTEXT GWEN_HBCICRYPTOCONTEXT;


GWEN_HBCICRYPTOCONTEXT *GWEN_HBCICryptoContext_new();
void GWEN_HBCICryptoContext_free(GWEN_HBCICRYPTOCONTEXT *ctx);


const char*
  GWEN_HBCICryptoContext_GetSecurityIdPtr(GWEN_HBCICRYPTOCONTEXT *ctx);

void GWEN_HBCICryptoContext_SetSecurityId(GWEN_HBCICRYPTOCONTEXT *ctx,
                                          const void *p, unsigned int l);
unsigned int
  GWEN_HBCICryptoContext_GetSecurityIdSize(GWEN_HBCICRYPTOCONTEXT *ctx);

unsigned int
  GWEN_HBCICryptoContext_GetSequenceNum(GWEN_HBCICRYPTOCONTEXT *ctx);
void GWEN_HBCICryptoContext_SetSequenceNum(GWEN_HBCICRYPTOCONTEXT *ctx,
                                           unsigned int i);

const char*
  GWEN_HBCICryptoContext_GetServiceCode(GWEN_HBCICRYPTOCONTEXT *ctx);
void GWEN_HBCICryptoContext_SetServiceCode(GWEN_HBCICRYPTOCONTEXT *ctx,
                                           const char *s);

GWEN_KEYSPEC *GWEN_HBCICryptoContext_GetKeySpec(GWEN_HBCICRYPTOCONTEXT *ctx);

/**
 * Stores a copy of the given keyspec.
 */
void GWEN_HBCICryptoContext_SetKeySpec(GWEN_HBCICRYPTOCONTEXT *ctx,
                                       const GWEN_KEYSPEC *ks);

void GWEN_HBCICryptoContext_SetCryptKey(GWEN_HBCICRYPTOCONTEXT *ctx,
                                        const char *p,
                                        unsigned int size);
const char *
  GWEN_HBCICryptoContext_GetCryptKeyPtr(GWEN_HBCICRYPTOCONTEXT *ctx);

unsigned int
  GWEN_HBCICryptoContext_GetCryptKeySize(GWEN_HBCICRYPTOCONTEXT *ctx);

const char *GWEN_HBCICryptoContext_GetMode(GWEN_HBCICRYPTOCONTEXT *ctx);
void GWEN_HBCICryptoContext_SetMode(GWEN_HBCICRYPTOCONTEXT *ctx,
                                    const char *m);


#endif

