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


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "net.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/waitcallback.h>
#include <time.h>



static GWEN_TYPE_UINT32 gwen_netconnection__lastlibraryid=0;
static GWEN_NETCONNECTION_LIST *gwen_netconnection__list=0;




/* -------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_Net_ModuleInit(){
  gwen_netconnection__list=GWEN_NetConnection_List_new();
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_Net_ModuleFini(){
  GWEN_NetConnection_List_free(gwen_netconnection__list);
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_TYPE_UINT32 GWEN_Net_GetLibraryId() {
  return ++gwen_netconnection__lastlibraryid;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_Net_AddConnectionToPool(GWEN_NETCONNECTION *conn) {
  assert(conn);
  assert(gwen_netconnection__list);
  GWEN_NetConnection_List_Add(conn, gwen_netconnection__list);
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETCONNECTION_WORKRESULT GWEN_Net_HeartBeat(int timeout){
  if (GWEN_NetConnection_List_GetCount(gwen_netconnection__list)) {
    GWEN_NETCONNECTION_WORKRESULT rv;

    rv=GWEN_NetConnection_Walk(gwen_netconnection__list, timeout);
    if (rv==GWEN_NetConnectionWorkResult_Error) {
      DBG_INFO(GWEN_LOGDOMAIN, "here");
    }
    return rv;
  }
  else {
    /* no connections, but wait for 750 ms to avoid CPU overload */
    DBG_INFO(GWEN_LOGDOMAIN, "No connections, sleeping to avoid CPU overload");
    GWEN_Socket_Select(0, 0, 0, 750);
    return GWEN_NetConnectionWorkResult_NoChange;
  }
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETCONNECTION_LIST *GWEN_Net_GetConnectionPool() {
  assert(gwen_netconnection__list);
  return gwen_netconnection__list;
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_Net_HasActiveConnections(){
  GWEN_NETCONNECTION *conn;
  unsigned int count;

  count=0;
  conn=GWEN_NetConnection_List_First(gwen_netconnection__list);
  while(conn) {
    GWEN_NETTRANSPORT_STATUS st;

    st=GWEN_NetConnection_GetStatus(conn);
    if (st!=GWEN_NetTransportStatusUnconnected &&
        st!=GWEN_NetTransportStatusPDisconnected &&
        st!=GWEN_NetTransportStatusDisabled &&
        st!=GWEN_NetTransportStatusListening) {
      count++;
    }
    conn=GWEN_NetConnection_List_Next(conn);
  } /* while */
  return (count!=0);
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_Net_HasListeningConnections(){
  GWEN_NETCONNECTION *conn;
  unsigned int count;

  count=0;
  conn=GWEN_NetConnection_List_First(gwen_netconnection__list);
  while(conn) {
    GWEN_NETTRANSPORT_STATUS st;

    st=GWEN_NetConnection_GetStatus(conn);
    if (st==GWEN_NetTransportStatusListening) {
      count++;
    }
    conn=GWEN_NetConnection_List_Next(conn);
  } /* while */
  return (count!=0);
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_Net_Dump() {
  GWEN_NETCONNECTION *conn;

  fprintf(stderr, "==============================================\n");
  fprintf(stderr, "Network Status:\n");
  conn=GWEN_NetConnection_List_First(gwen_netconnection__list);
  while(conn) {
    GWEN_NetConnection_Dump(conn);
    conn=GWEN_NetConnection_List_Next(conn);
  }
}




