/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Sat Jun 28 2003
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

#ifndef GWENHYWFAR_XSD_H
#define GWENHYWFAR_XSD_H

#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/bufferedio.h>
#include <gwenhywfar/stringlist.h>
#include <gwenhywfar/types.h>
#include <gwenhywfar/xml.h>
#include <stdio.h>


typedef struct GWEN_XSD_ENGINE GWEN_XSD_ENGINE;


typedef const char* (*GWEN_XSD_GETCHARVALUE_FN)(GWEN_XSD_ENGINE *e,
                                                const char *path,
                                                int idx,
                                                const char *defVal);
typedef int (*GWEN_XSD_GETINTVALUE_FN)(GWEN_XSD_ENGINE *e,
                                       const char *path,
                                       int idx,
                                       int defVal);
typedef const void* (*GWEN_XSD_GETBINVALUE_FN)(GWEN_XSD_ENGINE *e,
                                               const char *path,
                                               int idx,
                                               const void *defVal,
                                               unsigned int defValSize,
                                               unsigned int *returnValueSize);

GWEN_XSD_ENGINE *GWEN_XSD_new();
void GWEN_XSD_free(GWEN_XSD_ENGINE *e);





const char* GWEN_XSD_GetCharValue(GWEN_XSD_ENGINE *e,
                                  const char *path,
                                  int idx,
                                  const char *defVal);
int GWEN_XSD_GetIntValue(GWEN_XSD_ENGINE *e,
                         const char *path,
                         int idx,
                         int defVal);
const void *GWEN_XSD_GetBinValue(GWEN_XSD_ENGINE *e,
                                 const char *path,
                                 int idx,
                                 const void *defVal,
                                 unsigned int defValSize,
                                 unsigned int *returnValueSize);

int GWEN_XSD_ImportSchema(GWEN_XSD_ENGINE *e,
                          GWEN_XMLNODE *xmlNode,
                          const char *fname);

const char *GWEN_XSD_GetCurrentTargetNameSpace(const GWEN_XSD_ENGINE *e);
int GWEN_XSD_SetCurrentTargetNameSpace(GWEN_XSD_ENGINE *e, const char *s);

int GWEN_XSD_SetNamespace(GWEN_XSD_ENGINE *e,
                          const char *prefix,
                          const char *name,
                          const char *url,
                          const char *localUrl);

int GWEN_XSD_ImportStart(GWEN_XSD_ENGINE *e);
int GWEN_XSD_ImportEnd(GWEN_XSD_ENGINE *e);


int GWEN_XSD_ProfileToXml(GWEN_XSD_ENGINE *e,
                          GWEN_XMLNODE *n);

int GWEN_XSD_ProfileFromXml(GWEN_XSD_ENGINE *e,
                            GWEN_XMLNODE *nRoot);

int GWEN_XSD_GlobalizeNode(GWEN_XSD_ENGINE *e,
                           GWEN_XMLNODE *node);


#endif
