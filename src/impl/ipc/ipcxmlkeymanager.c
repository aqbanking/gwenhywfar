/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Nov 17 2003
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

#include "ipcxmlkeymanager_p.h"
#include <gwenhyfwar/misc.h>
#include <gwenhyfwar/debug.h>
#include <gwenhyfwar/text.h>



GWEN_IPCXMLKEYMANAGERDATA *GWEN_IPCXMLKeyManagerData_new(){
  GWEN_IPCXMLKEYMANAGERDATA *kd;

  GWEN_NEW_OBJECT(GWEN_IPCXMLKEYMANAGERDATA, kd);
  return kd;
}



void GWEN_IPCXMLKeyManager_FreeData(GWEN_KEYMANAGER *km){
  if (km) {
    GWEN_IPCXMLKEYMANAGERDATA *kd;

    assert(km);
    kd=(GWEN_IPCXMLKEYMANAGERDATA*)GWEN_KeyManager_GetData(km);
    if (kd) {
      GWEN_LIST_ITERATOR *it;
      GWEN_CRYPTKEY *key;

      it=GWEN_List_First(kd->keys);
      if (it) {
        key=(GWEN_CRYPTKEY *)GWEN_ListIterator_Data(it);
        while(key) {
          GWEN_CryptKey_free(key);
          key=(GWEN_CRYPTKEY *)GWEN_ListIterator_Next(it);
        } /* while */
      }
      GWEN_List_free(kd->keys);
      free(kd);
    }
  }
}



GWEN_CRYPTKEY*
GWEN_IPCXMLKeyManager_FindKey(GWEN_IPCXMLKEYMANAGERDATA *kd,
                              const GWEN_KEYSPEC *ks){
  GWEN_LIST_ITERATOR *it;
  GWEN_CRYPTKEY *key;

  it=GWEN_List_First(kd->keys);
  if (it) {
    key=(GWEN_CRYPTKEY*)GWEN_ListIterator_Data(it);
    while(key) {
      const GWEN_KEYSPEC *ks2;

      ks2=GWEN_CryptKey_GetKeySpec(key);
      DBG_DEBUG(0, "Comparing these:");
      DBG_DEBUG(0, "Key1: %s:%s:%d:%d",
                GWEN_KeySpec_GetOwner(ks),
                GWEN_KeySpec_GetKeyName(ks),
                GWEN_KeySpec_GetNumber(ks),
                GWEN_KeySpec_GetVersion(ks));
      DBG_DEBUG(0, "Key2: %s:%s:%d:%d",
                GWEN_KeySpec_GetOwner(ks2),
                GWEN_KeySpec_GetKeyName(ks2),
                GWEN_KeySpec_GetNumber(ks2),
                GWEN_KeySpec_GetVersion(ks2));

      if (GWEN_Text_Compare(GWEN_KeySpec_GetOwner(ks),
                            GWEN_KeySpec_GetOwner(ks2),1)==0)
        if (GWEN_Text_Compare(GWEN_KeySpec_GetKeyName(ks),
                              GWEN_KeySpec_GetKeyName(ks2),1)==0)
          if (GWEN_KeySpec_GetNumber(ks)==GWEN_KeySpec_GetNumber(ks2))
            if (GWEN_KeySpec_GetVersion(ks)==GWEN_KeySpec_GetVersion(ks2)) {
              GWEN_ListIterator_free(it);
              return key;
            }
      key=(GWEN_CRYPTKEY*)GWEN_ListIterator_Next(it);
    } /* while */
    GWEN_ListIterator_free(it);
  }

  return 0;
}



const GWEN_CRYPTKEY *GWEN_IPCXMLKeyManager_GetKey(GWEN_KEYMANAGER *km,
                                                  const GWEN_KEYSPEC *ks){
  GWEN_IPCXMLKEYMANAGERDATA *kd;

  assert(km);
  kd=(GWEN_IPCXMLKEYMANAGERDATA*)GWEN_KeyManager_GetData(km);
  assert(kd);

  return GWEN_IPCXMLKeyManager_FindKey(kd, ks);
}



int GWEN_IPCXMLKeyManager_AddKey(GWEN_KEYMANAGER *km,
                                 const GWEN_CRYPTKEY *key){
  GWEN_IPCXMLKEYMANAGERDATA *kd;

  assert(km);
  kd=(GWEN_IPCXMLKEYMANAGERDATA*)GWEN_KeyManager_GetData(km);
  assert(kd);

  if (GWEN_IPCXMLKeyManager_FindKey(kd, GWEN_CryptKey_GetKeySpec(key))) {
    DBG_ERROR(0, "Key already exists");
    return -1;
  }

  GWEN_List_PushBack(kd->keys, GWEN_CryptKey_dup(key));
  return 0;
}



int GWEN_IPCXMLKeyManager_DeleteKey(GWEN_KEYMANAGER *km,
                                    const GWEN_CRYPTKEY *delkey){
  GWEN_LIST_ITERATOR *it;
  GWEN_CRYPTKEY *key;
  const GWEN_KEYSPEC *ks;
  GWEN_IPCXMLKEYMANAGERDATA *kd;

  assert(km);
  kd=(GWEN_IPCXMLKEYMANAGERDATA*)GWEN_KeyManager_GetData(km);
  assert(kd);

  ks=GWEN_CryptKey_GetKeySpec(delkey);
  it=GWEN_List_First(kd->keys);
  if (it) {
    key=(GWEN_CRYPTKEY*)GWEN_ListIterator_Data(it);
    while(key) {
      const GWEN_KEYSPEC *ks2;

      ks2=GWEN_CryptKey_GetKeySpec(key);
      if (strcasecmp(GWEN_KeySpec_GetOwner(ks),
                     GWEN_KeySpec_GetOwner(ks2))==0)
        if (strcasecmp(GWEN_KeySpec_GetKeyName(ks),
                       GWEN_KeySpec_GetKeyName(ks2))==0)
          if (GWEN_KeySpec_GetNumber(ks)==GWEN_KeySpec_GetNumber(ks2))
            if (GWEN_KeySpec_GetVersion(ks)==GWEN_KeySpec_GetVersion(ks2)) {
              GWEN_List_Erase(kd->keys, it);
              GWEN_ListIterator_free(it);
              GWEN_CryptKey_free(key);
              return 0;
            }
      key=(GWEN_CRYPTKEY*)GWEN_ListIterator_Next(it);
    } /* while */
    GWEN_ListIterator_free(it);
  }

  DBG_ERROR(0, "Key not enlisted");
  return -1;
}



GWEN_KEYMANAGER *GWEN_IPCXMLKeyManager_new(){
  GWEN_KEYMANAGER *km;
  GWEN_IPCXMLKEYMANAGERDATA *kd;

  km=GWEN_KeyManager_new();
  kd=GWEN_IPCXMLKeyManagerData_new();
  kd->keys=GWEN_List_new();
  GWEN_KeyManager_SetData(km, kd);

  GWEN_KeyManager_SetGetKeyFn(km, GWEN_IPCXMLKeyManager_GetKey);
  GWEN_KeyManager_SetAddKeyFn(km, GWEN_IPCXMLKeyManager_AddKey);
  GWEN_KeyManager_SetDelKeyFn(km, GWEN_IPCXMLKeyManager_DeleteKey);
  GWEN_KeyManager_SetFreeDataFn(km, GWEN_IPCXMLKeyManager_FreeData);

  return km;
}














