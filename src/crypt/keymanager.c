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


#include "keymanager_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>



GWEN_KEYMANAGER *GWEN_KeyManager_new(){
  GWEN_KEYMANAGER *km;

  GWEN_NEW_OBJECT(GWEN_KEYMANAGER, km);
  return km;
}



void GWEN_KeyManager_SetGetKeyFn(GWEN_KEYMANAGER *km,
                                 GWEN_KEYMANAGER_GETKEYFN fn){
  assert(km);
  km->getKeyFn=fn;
}



void GWEN_KeyManager_SetAddKeyFn(GWEN_KEYMANAGER *km,
                                 GWEN_KEYMANAGER_ADDKEYFN fn){
  assert(km);
  km->addKeyFn=fn;
}



void GWEN_KeyManager_SetDelKeyFn(GWEN_KEYMANAGER *km,
                                 GWEN_KEYMANAGER_DELKEYFN fn){
  assert(km);
  km->delKeyFn=fn;
}



void GWEN_KeyManager_SetFreeDataFn(GWEN_KEYMANAGER *km,
                                   GWEN_KEYMANAGER_FREEDATAFN fn){
  assert(km);
  km->freeDataFn=fn;
}



void GWEN_KeyManager_SetData(GWEN_KEYMANAGER *km,
                             void *data){
  assert(km);
  if (km->data && km->freeDataFn)
    km->freeDataFn(km);
  km->data=data;
}



void *GWEN_KeyManager_GetData(GWEN_KEYMANAGER *km){
  assert(km);
  return km->data;
}



void GWEN_KeyManager_free(GWEN_KEYMANAGER *km){
  if (km) {
    if (km->data && km->freeDataFn)
      km->freeDataFn(km);
    free(km);
  }
}



const GWEN_CRYPTKEY *GWEN_KeyManager_GetKey(GWEN_KEYMANAGER *km,
                                            const GWEN_KEYSPEC *ks){
  assert(km);
  assert(km->getKeyFn);
  return km->getKeyFn(km, ks);
}



int GWEN_KeyManager_AddKey(GWEN_KEYMANAGER *km,
                           const GWEN_CRYPTKEY *key){
  assert(km);
  assert(km->addKeyFn);
  return km->addKeyFn(km, key);
}



int GWEN_KeyManager_DeleteKey(GWEN_KEYMANAGER *km,
                              const GWEN_CRYPTKEY *key){
  assert(km);
  assert(km->delKeyFn);
  return km->delKeyFn(km, key);
}






