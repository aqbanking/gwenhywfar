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


#ifndef GWEN_KEYSPEC_H
#define GWEN_KEYSPEC_H

#include <stdio.h>


typedef struct GWEN_KEYSPEC GWEN_KEYSPEC;


GWEN_KEYSPEC *GWEN_KeySpec_new();
GWEN_KEYSPEC *GWEN_KeySpec_dup(const GWEN_KEYSPEC *ks);
void GWEN_KeySpec_free(GWEN_KEYSPEC *ks);

const char *GWEN_KeySpec_GetKeyType(const GWEN_KEYSPEC *ks);
void GWEN_KeySpec_SetKeyType(GWEN_KEYSPEC *ks,
                                    const char *s);

const char *GWEN_KeySpec_GetKeyName(const GWEN_KEYSPEC *ks);
void GWEN_KeySpec_SetKeyName(GWEN_KEYSPEC *ks,
                                    const char *s);

const char *GWEN_KeySpec_GetOwner(const GWEN_KEYSPEC *ks);
void GWEN_KeySpec_SetOwner(GWEN_KEYSPEC *ks,
                           const char *s);

unsigned int GWEN_KeySpec_GetNumber(const GWEN_KEYSPEC *ks);
void GWEN_KeySpec_SetNumber(GWEN_KEYSPEC *ks,
                                   unsigned int i);

unsigned int GWEN_KeySpec_GetVersion(const GWEN_KEYSPEC *ks);
void GWEN_KeySpec_SetVersion(GWEN_KEYSPEC *ks,
                             unsigned int i);


GWEN_KEYSPEC *GWEN_KeySpec_Next(GWEN_KEYSPEC *ks);
const GWEN_KEYSPEC *GWEN_KeySpec_ConstNext(const GWEN_KEYSPEC *ks);

void GWEN_KeySpec_Add(GWEN_KEYSPEC *ks,
                             GWEN_KEYSPEC **head);
void GWEN_KeySpec_Del(GWEN_KEYSPEC *ks,
                             GWEN_KEYSPEC **head);

void GWEN_KeySpec_Clear(GWEN_KEYSPEC **head);


void GWEN_KeySpec_Dump(const GWEN_KEYSPEC *ks, FILE *f, int indent);



#endif /* GWEN_KEYSPEC_H */


