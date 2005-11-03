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


#include "net2_l.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/waitcallback.h>
#include <time.h>



static GWEN_NETLAYER_LIST *gwen_netlayer__list=0;




/* -------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_Net_ModuleInit(){
  gwen_netlayer__list=GWEN_NetLayer_List_new();
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_ERRORCODE GWEN_Net_ModuleFini(){
  GWEN_NetLayer_List_free(gwen_netlayer__list);
  return 0;
}



/* -------------------------------------------------------------- FUNCTION */
void GWEN_Net_AddConnectionToPool(GWEN_NETLAYER *nl) {
  assert(nl);
  assert(gwen_netlayer__list);
  GWEN_NetLayer_List_Add(nl, gwen_netlayer__list);
}



/* -------------------------------------------------------------- FUNCTION */
GWEN_NETLAYER_RESULT GWEN_Net_HeartBeat(int timeout){
  if (GWEN_NetLayer_List_GetCount(gwen_netlayer__list)) {
    GWEN_NETLAYER_RESULT rv;

    rv=GWEN_NetLayer_Walk(gwen_netlayer__list, timeout);
    if (rv==GWEN_NetLayerResult_Error) {
      DBG_INFO(GWEN_LOGDOMAIN, "here");
    }
    return rv;
  }
  else {
    /* no connections, but wait for 750 ms to avoid CPU overload */
    DBG_INFO(GWEN_LOGDOMAIN, "No connections, sleeping to avoid CPU overload");
    GWEN_Socket_Select(0, 0, 0, 750);
    return GWEN_NetLayerResult_Idle;
  }
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_Net_HasActiveConnections(){
  GWEN_NETLAYER *nl;
  unsigned int count;

  count=0;
  nl=GWEN_NetLayer_List_First(gwen_netlayer__list);
  while(nl) {
    GWEN_NETLAYER_STATUS st;

    st=GWEN_NetLayer_GetStatus(nl);
    if (st!=GWEN_NetLayerStatus_Unconnected &&
        st!=GWEN_NetLayerStatus_Disconnected &&
        st!=GWEN_NetLayerStatus_Disabled &&
        st!=GWEN_NetLayerStatus_Listening) {
      count++;
    }
    nl=GWEN_NetLayer_List_Next(nl);
  } /* while */
  return (count!=0);
}



/* -------------------------------------------------------------- FUNCTION */
int GWEN_Net_HasListeningConnections(){
  GWEN_NETLAYER *nl;
  unsigned int count;

  count=0;
  nl=GWEN_NetLayer_List_First(gwen_netlayer__list);
  while(nl) {
    GWEN_NETLAYER_STATUS st;

    st=GWEN_NetLayer_GetStatus(nl);
    if (st==GWEN_NetLayerStatus_Listening) {
      count++;
    }
    nl=GWEN_NetLayer_List_Next(nl);
  } /* while */
  return (count!=0);
}



