/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Thu Apr 03 2003
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

#ifndef GWENHYWFAR_STRINGLIST_P_H
#define GWENHYWFAR_STRINGLIST_P_H

#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/stringlist.h>
#include <gwenhywfar/types.h>


struct GWEN_STRINGLISTENTRYSTRUCT {
  GWEN_STRINGLISTENTRY *next;
  const char *data;
  uint32_t refCount;
};


struct GWEN_STRINGLISTSTRUCT {
  GWEN_STRINGLISTENTRY *first;
  unsigned int count;
  int senseCase;
  int ignoreRefCount;
};


static int GWEN_StringList__compar_asc_nocase(const void *a, const void *b);
static int GWEN_StringList__compar_desc_nocase(const void *a, const void *b);
static int GWEN_StringList__compar_asc_case(const void *a, const void *b);
static int GWEN_StringList__compar_desc_case(const void *a, const void *b);
static int GWEN_StringList__compar_asc_int(const void *a, const void *b);
static int GWEN_StringList__compar_desc_int(const void *a, const void *b);





#endif


