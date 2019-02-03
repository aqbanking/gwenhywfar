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

#ifndef GWENHYWFAR_STRINGLIST2_P_H
#define GWENHYWFAR_STRINGLIST2_P_H

#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/stringlist2.h>
#include <gwenhywfar/types.h>
#include <gwenhywfar/refptr.h>
#include <gwenhywfar/list.h>



struct GWEN_STRINGLIST2 {
  int senseCase;
  GWEN_LIST *listPtr;
};

GWEN_STRINGLIST2_ITERATOR *
GWEN_StringList2__GetString(const GWEN_STRINGLIST2 *sl2,
                            const char *s);


#endif


