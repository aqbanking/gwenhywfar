/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Thu Nov 13 2003
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



#ifndef GWENHYWFAR_MD_P_H
#define GWENHYWFAR_MD_P_H

#include <gwenhywfar/md.h>


struct GWEN_MD {
  unsigned char *pDigest;
  unsigned int lDigest;
  void *data;

  GWEN_MD_BEGIN_FN beginFn;
  GWEN_MD_END_FN endFn;
  GWEN_MD_UPDATE_FN updateFn;
  GWEN_MD_FREEDATA_FN freeDataFn;
};


struct GWEN_MD_PROVIDER {
  GWEN_MD_PROVIDER *next;
  char *name;
  GWEN_MDPROVIDER_NEWMD_FN newMdFn;
};


GWEN_MD_PROVIDER *GWEN_MD_FindProvider(const char *name);



#endif


