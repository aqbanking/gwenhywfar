/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sat Nov 08 2003
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


#ifndef GWENHYFWAR_IPCXMLMSGLAYER_P_H
#define GWENHYFWAR_IPCXMLMSGLAYER_P_H

#include "ipcxmlmsglayer.h"
#include <gwenhyfwar/transportlayer.h>
#include <gwenhyfwar/msglayer.h>
#include <gwenhyfwar/msgengine.h>

#define GWEN_IPCXMLMSGLAYER_MSGSIZE 512


/**
 * This is the module specific data for the msgLayer.
 */
struct GWEN_IPCXMLMSGLAYERDATA {
  GWEN_MSGENGINE *msgEngine; /* not owned ! */
  GWEN_IPCMSG *currentMsg;
  int readingSize; /* if !=0 then we are still reading the size */
  unsigned int bytesToRead; /* bytes still to read */

};
GWEN_IPCXMLMSGLAYERDATA *GWEN_IPCXMLMsgLayerData_new(GWEN_MSGENGINE *msgEngine);
void GWEN_IPCXMLMsgLayerData_free(GWEN_IPCXMLMSGLAYERDATA *mcd);

void GWEN_IPCXMLMsgLayer_free(GWEN_IPCMSGLAYER *ml);
GWEN_ERRORCODE GWEN_IPCXMLMsgLayer_Work(GWEN_IPCMSGLAYER *ml,
                                        int rd);
GWEN_ERRORCODE GWEN_IPCXMLMsgLayer_Accept(GWEN_IPCMSGLAYER *ml,
                                          GWEN_IPCTRANSPORTLAYER *tl,
                                          GWEN_IPCMSGLAYER **m);













#endif

