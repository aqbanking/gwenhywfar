/***************************************************************************
    begin       : Sat Dec 16 2012
    copyright   : (C) 2012 by Martin Preuss
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

#ifndef GWENHYWFAR_PASSWDSTORE_H
#define GWENHYWFAR_PASSWDSTORE_H


#include <gwenhywfar/gwenhywfarapi.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef struct GWEN_PASSWD_STORE GWEN_PASSWD_STORE;


GWENHYWFAR_API
GWEN_PASSWD_STORE *GWEN_PasswordStore_new(const char *fname);

GWENHYWFAR_API
void GWEN_PasswordStore_free(GWEN_PASSWD_STORE *sto);


GWENHYWFAR_API
int GWEN_PasswordStore_SetPassword(GWEN_PASSWD_STORE *sto, const char *token, const char *secret);

GWENHYWFAR_API
int GWEN_PasswordStore_GetPassword(GWEN_PASSWD_STORE *sto, const char *token, char *buffer, int minLen, int maxLen);



#ifdef __cplusplus
}
#endif



#endif
