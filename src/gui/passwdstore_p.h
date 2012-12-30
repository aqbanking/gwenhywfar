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

#ifndef GWENHYWFAR_PASSWDSTORE_P_H
#define GWENHYWFAR_PASSWDSTORE_P_H


#include <gwenhywfar/passwdstore.h>
#include <gwenhywfar/db.h>


#define GWEN_PASSWDSTORE_PW_ITERATIONS     1467
#define GWEN_PASSWDSTORE_CRYPT_ITERATIONS  648

#define GWEN_PASSWDSTORE_PWLEN 129


struct GWEN_PASSWD_STORE {
  char *fileName;
  char pw[GWEN_PASSWDSTORE_PWLEN];
  GWEN_DB_NODE *dbPasswords;
  int isNew;
};



#endif
