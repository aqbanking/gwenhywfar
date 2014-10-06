/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id: stringlist_p.h 786 2005-07-09 13:38:17Z aquamaniac $
 begin       : Thu Apr 03 2003
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

#ifndef GWENHYWFAR_SIGNAL_P_H
#define GWENHYWFAR_SIGNAL_P_H

#include "gwensignal.h"


GWEN_LIST2_FUNCTION_DEFS(GWEN_SIGNAL, GWEN_Signal)
GWEN_LIST2_FUNCTION_DEFS(GWEN_SLOT, GWEN_Slot)

void GWEN_Signal_List2_freeAll(GWEN_SIGNAL_LIST2 *slist);
void GWEN_Slot_List2_freeAll(GWEN_SLOT_LIST2 *slist);


struct GWEN_SIGNAL {
  GWEN_SIGNALOBJECT *signalObject;
  char *name;
  uint32_t derivedParentType;
  uint32_t typeOfArg1;
  uint32_t typeOfArg2;
  GWEN_SLOT_LIST2 *connectedSlots;
  uint32_t _refCount;
};

GWEN_SIGNAL *GWEN_Signal__List2_freeAll_cb(GWEN_SIGNAL *sig, void *user_data);
GWEN_SIGNAL *GWEN_Signal__List2_hasSignal_cb(GWEN_SIGNAL *sig,
    void *user_data);
int GWEN_Signal_List2_HasSignal(GWEN_SIGNAL_LIST2 *slist,
                                const GWEN_SIGNAL *sig);

void GWEN_Signal_Attach(GWEN_SIGNAL *sig);


struct GWEN_SLOT {
  GWEN_SIGNALOBJECT *signalObject;
  char *name;
  uint32_t derivedParentType;
  GWEN_SLOT_FUNCTION func;
  void *userData;
  uint32_t typeOfArg1;
  uint32_t typeOfArg2;
  GWEN_SIGNAL_LIST2 *connectedSignals;
  uint32_t _refCount;
};

GWEN_SLOT *GWEN_Slot__List2_freeAll_cb(GWEN_SLOT *slot, void *user_data);
GWEN_SLOT *GWEN_Slot__List2_hasSlot_cb(GWEN_SLOT *slot, void *user_data);
int GWEN_Slot_List2_HasSlot(GWEN_SLOT_LIST2 *slist, const GWEN_SLOT *slot);
void GWEN_Slot_Attach(GWEN_SLOT *slot);




struct GWEN_SIGNALOBJECT {
  GWEN_SLOT_LIST2 *slotList;
  GWEN_SIGNAL_LIST2 *signalList;
};

uint32_t GWEN_SignalObject_MkTypeId(const char *typeName);
GWEN_SIGNAL *GWEN_SignalObject__findSignal(const GWEN_SIGNALOBJECT *so,
    const char *name,
    uint32_t typeId1,
    uint32_t typeId2);
GWEN_SLOT *GWEN_SignalObject__findSlot(const GWEN_SIGNALOBJECT *so,
                                       const char *name,
                                       uint32_t typeId1,
                                       uint32_t typeId2);
int GWEN_SignalObject_AddSignal(GWEN_SIGNALOBJECT *so, GWEN_SIGNAL *sig);
int GWEN_SignalObject_AddSlot(GWEN_SIGNALOBJECT *so, GWEN_SLOT *slot);


#endif

