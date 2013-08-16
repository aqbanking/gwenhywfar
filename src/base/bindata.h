/***************************************************************************
    begin       : Fri Aug 16 2013
    copyright   : (C) 2013 by Martin Preuss
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

#ifndef GWENHYWFAR_BINDATA_H
#define GWENHYWFAR_BINDATA_H


#include <gwenhywfar/types.h>

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
  uint8_t *pointer;
  uint64_t length;
} GWEN_BINDATA;






#ifdef __cplusplus
}
#endif


#endif

