/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Thu Dec 18 2003
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


#ifndef GWENHYWFAR_CRYPTSSL_SHA1_P_H
#define GWENHYWFAR_CRYPTSSL_SHA1_P_H


#define GWEN_MD_SHA1_NAME "SHA1"

#include <gwenhywfar/md.h>
#include <openssl/sha.h>
#include <openssl/objects.h>


static GWEN_MD *GWEN_MdSha1_new();
static void GWENHYWFAR_CB GWEN_MdSha1_FreeData(GWEN_MD *md);

static int GWEN_MdSha1_Begin(GWEN_MD *md);
static int GWEN_MdSha1_End(GWEN_MD *md);
static int GWEN_MdSha1_Update(GWEN_MD *md,
                              const char *buf,
                              unsigned int l);











#endif


