/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Tue Sep 16 2003
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



#ifndef GWENHYWFAR_SERVICELAYER_P_H
#define GWENHYWFAR_SERVICELAYER_P_H


#include <gwenhywfar/servicelayer.h>
#include "connlayer_p.h"


#define GWEN_SERVICELAYER_CPU_TIMEOUT 500


typedef struct GWEN_GLOBAL_SERVICELAYER GWEN_GLOBAL_SERVICELAYER;


struct GWEN_GLOBAL_SERVICELAYER {
  GWEN_IPCCONNLAYER *connections;
  unsigned int localLayers;
};



struct GWEN_SERVICELAYER {
  unsigned int id;
  unsigned int lastCheckedMsgLayer;
  void *data;
  GWEN_SERVICELAYER_FREEDATA_FN freeDataFn;
};


GWEN_GLOBAL_SERVICELAYER *GWEN_GlobalServiceLayer_new();
void GWEN_GlobalServiceLayer_free(GWEN_GLOBAL_SERVICELAYER *gsl);


GWEN_ERRORCODE
GWEN_GlobalServiceLayer_AddConnection(GWEN_IPCCONNLAYER *conn);


GWEN_IPCCONNLAYER*
  GWEN_GlobalServiceLayer_FindConnection(unsigned int id,
                                         unsigned int libMark,
                                         unsigned int userMark);
GWEN_ERRORCODE GWEN_GlobalServiceLayer_Work(int timeout);
void GWEN_GlobalServiceLayer_CheckClosed();
void GWEN_GlobalServiceLayer_RemoveClosed();
void GWEN_GlobalServiceLayer_Close(unsigned int id,
                                   unsigned int libMark,
                                   unsigned int userMark,
                                   int force);

GWEN_ERRORCODE
  GWEN_GlobalServiceLayer_RegisterLocalLayer(GWEN_SERVICELAYER *sl);
GWEN_ERRORCODE
  GWEN_GlobalServiceLayer_UnregisterLocalLayer(GWEN_SERVICELAYER *sl);



#endif /* GWENHYWFAR_SERVICELAYER_P_H */





