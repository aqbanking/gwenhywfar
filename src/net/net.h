/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Feb 09 2004
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

#ifndef GWEN_NET_H
#define GWEN_NET_H



#include <gwenhywfar/net.h>
#include <gwenhywfar/netconnection.h>

GWEN_ERRORCODE GWEN_Net_ModuleInit();
GWEN_ERRORCODE GWEN_Net_ModuleFini();


GWEN_TYPE_UINT32 GWEN_Net_GetLibraryId();
void GWEN_Net_AddConnectionToPool(GWEN_NETCONNECTION *conn);


GWEN_NETCONNECTION_WORKRESULT GWEN_Net_HeartBeat(int timeout);

GWEN_NETCONNECTION_LIST *GWEN_Net_GetConnectionPool();


int GWEN_Net_HasActiveConnections();
int GWEN_Net_HasListeningConnections();





#endif /* GWEN_NET_H */

