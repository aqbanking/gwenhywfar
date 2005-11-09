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


#ifndef GWEN_NL_FILE_P_H
#define GWEN_NL_FILE_P_H

#include "nl_file.h"



typedef struct GWEN_NL_FILE GWEN_NL_FILE;
struct GWEN_NL_FILE {
  int fdRead;
  int fdWrite;
  int closeOnFree;
};

void GWEN_NetLayerFile_FreeData(void *bp, void *p);

int GWEN_NetLayerFile_Connect(GWEN_NETLAYER *nl);
int GWEN_NetLayerFile_Disconnect(GWEN_NETLAYER *nl);

int GWEN_NetLayerFile_Read(GWEN_NETLAYER *nl, char *buffer,
                           int *bsize);
int GWEN_NetLayerFile_Write(GWEN_NETLAYER *nl,
                            const char *buffer,
                            int *bsize);

GWEN_NETLAYER_RESULT GWEN_NetLayerFile_Work(GWEN_NETLAYER *nl);


#endif /* GWEN_NL_FILE_P_H */


