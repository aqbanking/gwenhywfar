/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id: stringlist.c 1067 2006-05-22 15:25:23Z christian $
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "gwensignal_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/inherit.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>



GWEN_LIST2_FUNCTIONS(GWEN_SIGNAL, GWEN_Signal)
GWEN_LIST2_FUNCTIONS(GWEN_SLOT, GWEN_Slot)


GWEN_SIGNALOBJECT *GWEN_SignalObject_new(void)
{
  GWEN_SIGNALOBJECT *so;

  GWEN_NEW_OBJECT(GWEN_SIGNALOBJECT, so);
  so->signalList=GWEN_Signal_List2_new();
  so->slotList=GWEN_Slot_List2_new();

  return so;
}



void GWEN_SignalObject_free(GWEN_SIGNALOBJECT *so)
{
  if (so) {
    GWEN_Slot_List2_freeAll(so->slotList);
    GWEN_Signal_List2_freeAll(so->signalList);
    GWEN_FREE_OBJECT(so);
  }
}



uint32_t GWEN_SignalObject_MkTypeId(const char *typeName)
{
  return GWEN_Inherit_MakeId(typeName);
}



GWEN_SIGNAL *GWEN_SignalObject__findSignal(const GWEN_SIGNALOBJECT *so,
                                           const char *name,
                                           uint32_t typeId1,
                                           uint32_t typeId2)
{
  GWEN_SIGNAL_LIST2_ITERATOR *sit;

  assert(so);
  assert(name);

  sit=GWEN_Signal_List2_First(so->signalList);
  if (sit) {
    GWEN_SIGNAL *sig;

    sig=GWEN_Signal_List2Iterator_Data(sit);
    assert(sig);
    while (sig) {
      const char *s;

      s=sig->name;
      assert(s);
      if (strcasecmp(s, name)==0 &&
          (typeId1==0 || typeId1==sig->typeOfArg1) &&
          (typeId2==0 || typeId2==sig->typeOfArg2)) {
        GWEN_Signal_List2Iterator_free(sit);
        return sig;
      }
      sig=GWEN_Signal_List2Iterator_Next(sit);
    }
    GWEN_Signal_List2Iterator_free(sit);
  }

  return 0;
}



GWEN_SIGNAL *GWEN_SignalObject_FindSignal(const GWEN_SIGNALOBJECT *so,
                                          const char *name,
                                          const char *typeOfArg1,
                                          const char *typeOfArg2)
{
  uint32_t typeId1=0;
  uint32_t typeId2=0;

  if (typeOfArg1)
    typeId1=GWEN_SignalObject_MkTypeId(typeOfArg1);
  if (typeOfArg2)
    typeId2=GWEN_SignalObject_MkTypeId(typeOfArg2);
  return GWEN_SignalObject__findSignal(so, name, typeId1, typeId2);
}



GWEN_SLOT *GWEN_SignalObject__findSlot(const GWEN_SIGNALOBJECT *so,
                                       const char *name,
                                       uint32_t typeId1,
                                       uint32_t typeId2)
{
  GWEN_SLOT_LIST2_ITERATOR *sit;

  assert(so);
  assert(name);

  sit=GWEN_Slot_List2_First(so->slotList);
  if (sit) {
    GWEN_SLOT *slot;

    slot=GWEN_Slot_List2Iterator_Data(sit);
    assert(slot);
    while (slot) {
      const char *s;

      s=slot->name;
      assert(s);
      if (strcasecmp(s, name)==0 &&
          (typeId1==0 || typeId1==slot->typeOfArg1) &&
          (typeId2==0 || typeId2==slot->typeOfArg2)) {
        GWEN_Slot_List2Iterator_free(sit);
        return slot;
      }
      slot=GWEN_Slot_List2Iterator_Next(sit);
    }
    GWEN_Slot_List2Iterator_free(sit);
  }

  return 0;
}



GWEN_SLOT *GWEN_SignalObject_FindSlot(const GWEN_SIGNALOBJECT *so,
                                      const char *name,
                                      const char *typeOfArg1,
                                      const char *typeOfArg2)
{
  uint32_t typeId1=0;
  uint32_t typeId2=0;

  if (typeOfArg1)
    typeId1=GWEN_SignalObject_MkTypeId(typeOfArg1);
  if (typeOfArg2)
    typeId2=GWEN_SignalObject_MkTypeId(typeOfArg2);
  return GWEN_SignalObject__findSlot(so, name, typeId1, typeId2);
}



int GWEN_SignalObject_AddSignal(GWEN_SIGNALOBJECT *so, GWEN_SIGNAL *sig)
{
  if (GWEN_SignalObject__findSignal(so, sig->name,
                                    sig->typeOfArg1,
                                    sig->typeOfArg2)) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Signal \"%s\" already exists",
              sig->name);
    return GWEN_ERROR_INVALID;
  }

  sig->signalObject=so;
  GWEN_Signal_List2_PushBack(so->signalList, sig);
  DBG_INFO(GWEN_LOGDOMAIN, "Added signal \"%s\"", sig->name);
  return 0;
}



int GWEN_SignalObject_AddSlot(GWEN_SIGNALOBJECT *so, GWEN_SLOT *slot)
{
  if (GWEN_SignalObject__findSlot(so, slot->name,
                                  slot->typeOfArg1,
                                  slot->typeOfArg2)) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Slot \"%s\" already exists",
              slot->name);
    return GWEN_ERROR_INVALID;
  }
  slot->signalObject=so;
  GWEN_Slot_List2_PushBack(so->slotList, slot);
  DBG_INFO(GWEN_LOGDOMAIN, "Added slot \"%s\"", slot->name);
  return 0;
}



void GWEN_SignalObject_RemoveForDerivedType(GWEN_SIGNALOBJECT *so,
                                            const char *derivedType)
{
  uint32_t typeId=0;
  GWEN_SLOT_LIST2_ITERATOR *slotIt;
  GWEN_SIGNAL_LIST2_ITERATOR *sigIt;

  assert(so);
  if (derivedType)
    typeId=GWEN_SignalObject_MkTypeId(derivedType);

  slotIt=GWEN_Slot_List2_First(so->slotList);
  if (slotIt) {
    GWEN_SLOT *slot;

    slot=GWEN_Slot_List2Iterator_Data(slotIt);
    assert(slot);
    while (slot) {
      const char *s;

      s=slot->name;
      assert(s);
      if (typeId==0 || slot->derivedParentType==typeId) {
        GWEN_Slot_List2_Erase(so->slotList, slotIt);
        GWEN_Slot_free(slot);
        /* iterator now points to the next entry in any case (or NULL) */
        slot=GWEN_Slot_List2Iterator_Data(slotIt);
      }
      else
        slot=GWEN_Slot_List2Iterator_Next(slotIt);
    }
    GWEN_Slot_List2Iterator_free(slotIt);
  }

  sigIt=GWEN_Signal_List2_First(so->signalList);
  if (sigIt) {
    GWEN_SIGNAL *sig;

    sig=GWEN_Signal_List2Iterator_Data(sigIt);
    assert(sig);
    while (sig) {
      const char *s;

      s=sig->name;
      assert(s);
      if (typeId==0 || sig->derivedParentType==typeId) {
        GWEN_Signal_List2_Erase(so->signalList, sigIt);
        GWEN_Signal_free(sig);
        /* iterator now points to the next entry in any case (or NULL) */
        sig=GWEN_Signal_List2Iterator_Data(sigIt);
      }
      else
        sig=GWEN_Signal_List2Iterator_Next(sigIt);
    }
    GWEN_Signal_List2Iterator_free(sigIt);
  }
}





GWEN_SIGNAL *GWEN_Signal_new(GWEN_SIGNALOBJECT *so,
                             const char *derivedType,
                             const char *name,
                             const char *typeOfArg1,
                             const char *typeOfArg2)
{
  GWEN_SIGNAL *sig;

  assert(so);
  assert(name);
  GWEN_NEW_OBJECT(GWEN_SIGNAL, sig)
  sig->_refCount=1;
  sig->connectedSlots=GWEN_Slot_List2_new();
  sig->name=strdup(name);
  if (typeOfArg1)
    sig->typeOfArg1=GWEN_SignalObject_MkTypeId(typeOfArg1);
  if (typeOfArg2)
    sig->typeOfArg2=GWEN_SignalObject_MkTypeId(typeOfArg2);
  if (derivedType)
    sig->derivedParentType=GWEN_SignalObject_MkTypeId(derivedType);

  if (GWEN_SignalObject_AddSignal(so, sig)) {
    GWEN_Signal_free(sig);
    return 0;
  }

  return sig;
}



void GWEN_Signal_free(GWEN_SIGNAL *sig)
{
  if (sig) {
    assert(sig->_refCount);
    if (sig->_refCount==1) {
      GWEN_SLOT_LIST2_ITERATOR *sit;

      /* remove from all connected slots */
      sit=GWEN_Slot_List2_First(sig->connectedSlots);
      if (sit) {
        GWEN_SLOT *slot;

        slot=GWEN_Slot_List2Iterator_Data(sit);
        assert(slot);
        while (slot) {
          GWEN_SLOT *next;

          next=GWEN_Slot_List2Iterator_Next(sit);
          DBG_ERROR(GWEN_LOGDOMAIN,
                    "Disconnecting signal \"%s\" from slot \"%s\"",
                    sig->name, slot->name);
          GWEN_Signal_List2_Remove(slot->connectedSignals, sig);
          slot=next;
        }
        GWEN_Slot_List2Iterator_free(sit);
      }
      GWEN_Slot_List2_free(sig->connectedSlots);

      free(sig->name);
      sig->_refCount=0;
      GWEN_FREE_OBJECT(sig);
    }
    else
      sig->_refCount--;
  }
}



void GWEN_Signal_Attach(GWEN_SIGNAL *sig)
{
  assert(sig);
  assert(sig->_refCount);
  sig->_refCount++;
}



GWEN_SIGNAL *GWEN_Signal__List2_freeAll_cb(GWEN_SIGNAL *sig, GWEN_UNUSED void *user_data)
{
  GWEN_Signal_free(sig);
  return 0;
}



void GWEN_Signal_List2_freeAll(GWEN_SIGNAL_LIST2 *slist)
{
  GWEN_Signal_List2_ForEach(slist, GWEN_Signal__List2_freeAll_cb, 0);
  GWEN_Signal_List2_free(slist);
}



GWEN_SIGNAL *GWEN_Signal__List2_hasSignal_cb(GWEN_SIGNAL *sig,
                                             void *user_data)
{
  if ((void *)sig==user_data)
    return sig;
  return 0;
}



int GWEN_Signal_List2_HasSignal(GWEN_SIGNAL_LIST2 *slist,
                                const GWEN_SIGNAL *sig)
{
  if (GWEN_Signal_List2_ForEach(slist, GWEN_Signal__List2_hasSignal_cb,
                                (void *)sig))
    return 1;
  return 0;
}



GWEN_SIGNALOBJECT *GWEN_Signal_GetSignalObject(const GWEN_SIGNAL *sig)
{
  assert(sig);
  return sig->signalObject;
}



int GWEN_Signal_Connect(GWEN_SIGNAL *sig, GWEN_SLOT *slot)
{
  assert(sig);
  assert(slot);
  if (sig->typeOfArg1!=slot->typeOfArg1) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Signal \"%s\" and slot \"%s\" use different types for "
              "argument 1",
              sig->name, slot->name);
    return GWEN_ERROR_INVALID;
  }
  if (sig->typeOfArg2!=slot->typeOfArg2) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Signal \"%s\" and slot \"%s\" use different types for "
              "argument 2",
              sig->name, slot->name);
    return GWEN_ERROR_INVALID;
  }
  if (GWEN_Signal_List2_HasSignal(slot->connectedSignals, sig)) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Signal \"%s\" and slot \"%s\" already connected",
              sig->name, slot->name);
    return GWEN_ERROR_INVALID;
  }

  if (GWEN_Slot_List2_HasSlot(sig->connectedSlots, slot)) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Signal \"%s\" and slot \"%s\" already connected",
              sig->name, slot->name);
    return GWEN_ERROR_INVALID;
  }

  GWEN_Signal_List2_PushBack(slot->connectedSignals, sig);
  GWEN_Slot_List2_PushBack(sig->connectedSlots, slot);

  return 0;
}



int GWEN_Signal_Disconnect(GWEN_SIGNAL *sig, GWEN_SLOT *slot)
{
  assert(sig);
  assert(slot);
  if (GWEN_Signal_List2_HasSignal(slot->connectedSignals, sig)==0) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Signal \"%s\" and slot \"%s\" are not connected",
              sig->name, slot->name);
    return GWEN_ERROR_INVALID;
  }

  if (GWEN_Slot_List2_HasSlot(sig->connectedSlots, slot)==0) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Signal \"%s\" and slot \"%s\" are not connected",
              sig->name, slot->name);
    return GWEN_ERROR_INVALID;
  }

  GWEN_Signal_List2_Remove(slot->connectedSignals, sig);
  GWEN_Slot_List2_Remove(sig->connectedSlots, slot);

  return 0;
}



int GWEN_Signal_Emit(GWEN_SIGNAL *sig,
                     void *pArg1, void *pArg2, int iArg3, int iArg4)
{
  GWEN_SLOT_LIST2_ITERATOR *sit;
  int result=0;

  assert(sig);
  sit=GWEN_Slot_List2_First(sig->connectedSlots);
  if (sit) {
    GWEN_SLOT *slot;

    slot=GWEN_Slot_List2Iterator_Data(sit);
    assert(slot);
    while (slot) {
      if (slot->func) {
        int rv;

        DBG_DEBUG(GWEN_LOGDOMAIN,
                  "Sending signal \"%s\" to slot \"%s\" (%p)",
                  sig->name, slot->name, slot);
        rv=slot->func(slot, slot->userData, pArg1, pArg2, iArg3, iArg4);
        if (rv>0) {
          DBG_DEBUG(GWEN_LOGDOMAIN,
                    "Slot \"%s\" (%p) returned an error (%d)",
                    slot->name, slot, rv);
          result=rv;
        }
      }
      slot=GWEN_Slot_List2Iterator_Next(sit);
    }
    GWEN_Slot_List2Iterator_free(sit);
  }

  return result;
}






GWEN_SLOT *GWEN_Slot_new(GWEN_SIGNALOBJECT *so,
                         const char *derivedType,
                         const char *name,
                         const char *typeOfArg1,
                         const char *typeOfArg2,
                         GWEN_SLOT_FUNCTION fn,
                         void *userData)
{
  GWEN_SLOT *slot;

  assert(name);
  GWEN_NEW_OBJECT(GWEN_SLOT, slot)
  slot->_refCount=1;
  slot->connectedSignals=GWEN_Signal_List2_new();
  slot->name=strdup(name);
  if (typeOfArg1)
    slot->typeOfArg1=GWEN_SignalObject_MkTypeId(typeOfArg1);
  if (typeOfArg2)
    slot->typeOfArg2=GWEN_SignalObject_MkTypeId(typeOfArg2);
  if (derivedType)
    slot->derivedParentType=GWEN_SignalObject_MkTypeId(derivedType);
  slot->func=fn;
  slot->userData=userData;

  if (GWEN_SignalObject_AddSlot(so, slot)) {
    GWEN_Slot_free(slot);
    return 0;
  }

  return slot;
}



void GWEN_Slot_free(GWEN_SLOT *slot)
{
  if (slot) {
    assert(slot->_refCount);
    if (slot->_refCount==1) {
      GWEN_SIGNAL_LIST2_ITERATOR *sit;

      /* remove from all connected signals */
      sit=GWEN_Signal_List2_First(slot->connectedSignals);
      if (sit) {
        GWEN_SIGNAL *sig;

        sig=GWEN_Signal_List2Iterator_Data(sit);
        assert(sig);
        while (sig) {
          DBG_ERROR(GWEN_LOGDOMAIN,
                    "Disconnecting slot \"%s\" from signal \"%s\"",
                    slot->name, sig->name);
          GWEN_Slot_List2_Remove(sig->connectedSlots, slot);
          sig=GWEN_Signal_List2Iterator_Next(sit);
        }
        GWEN_Signal_List2Iterator_free(sit);
      }
      GWEN_Signal_List2_free(slot->connectedSignals);

      free(slot->name);
      slot->_refCount=0;
      GWEN_FREE_OBJECT(slot);
    }
    else
      slot->_refCount--;
  }
}



void GWEN_Slot_Attach(GWEN_SLOT *slot)
{
  assert(slot);
  assert(slot->_refCount);
  slot->_refCount++;
}



GWEN_SLOT *GWEN_Slot__List2_freeAll_cb(GWEN_SLOT *slot, GWEN_UNUSED void *user_data)
{
  GWEN_Slot_free(slot);
  return 0;
}



void GWEN_Slot_List2_freeAll(GWEN_SLOT_LIST2 *slist)
{
  GWEN_Slot_List2_ForEach(slist, GWEN_Slot__List2_freeAll_cb, 0);
  GWEN_Slot_List2_free(slist);
}



GWEN_SLOT *GWEN_Slot__List2_hasSlot_cb(GWEN_SLOT *slot,
                                       void *user_data)
{
  if ((void *)slot==user_data)
    return slot;
  return 0;
}



int GWEN_Slot_List2_HasSlot(GWEN_SLOT_LIST2 *slist,
                            const GWEN_SLOT *slot)
{
  if (GWEN_Slot_List2_ForEach(slist, GWEN_Slot__List2_hasSlot_cb,
                              (void *)slot))
    return 1;
  return 0;
}



GWEN_SIGNALOBJECT *GWEN_Slot_GetSignalObject(const GWEN_SLOT *slot)
{
  assert(slot);
  return slot->signalObject;
}







