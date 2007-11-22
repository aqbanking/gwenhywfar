/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Tue Dec 03 2002
    copyright   : (C) 2002 by Martin Preuss
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

/**
 * @file chameleon/directory_p.h
 * @short This file contains the directory handling module
 */


#ifndef GWEN_DIRECTORY_P_H
#define GWEN_DIRECTORY_P_H


#include <sys/types.h>
#include <dirent.h>
#include <windows.h>

#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/directory.h>


#ifdef __cplusplus
extern "C" {
#endif

struct GWEN_DIRECTORY {
  char pattern[256];
  HANDLE handle;
  char lastName[256];
};


#ifdef __cplusplus
}
#endif


#endif /* GWEN_DIRECTORY_P_H */




