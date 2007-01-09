/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sat Jan 24 2004
    copyright   : (C) 2004 by Martin Preuss
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


#ifndef GWEN_NL_STDIO_P_H
#define GWEN_NL_STDIO_P_H

#include "nl_stdio.h"



typedef struct GWEN_NL_STDIO GWEN_NL_STDIO;
struct GWEN_NL_STDIO {
  GWEN_SOCKET *socketRead;
  GWEN_SOCKET *socketWrite;
};

static void GWENHYWFAR_CB GWEN_NetLayerStdio_FreeData(void *bp, void *p);
static int GWEN_NetLayerStdio_Disconnect(GWEN_NETLAYER *nl);
static int GWEN_NetLayerStdio_Read(GWEN_NETLAYER *nl, char *buffer,
                                   int *bsize);
static int GWEN_NetLayerStdio_Write(GWEN_NETLAYER *nl,
                                    const char *buffer,
                                    int *bsize);

static int GWEN_NetLayerStdio_AddSockets(GWEN_NETLAYER *nl,
                                         GWEN_SOCKETSET *readSet,
                                         GWEN_SOCKETSET *writeSet,
                                         GWEN_SOCKETSET *exSet);

static GWEN_NETLAYER_RESULT GWEN_NetLayerStdio_Work(GWEN_NETLAYER *nl);

#endif /* GWEN_NL_STDIO_P_H */


