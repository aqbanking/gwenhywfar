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


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "keyspec_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>


#ifdef GWEN_MEMTRACE
static unsigned int GWEN_KeySpec_Count=0;
#endif


GWEN_KEYSPEC *GWEN_KeySpec_new(){
  GWEN_KEYSPEC *ks;

  GWEN_NEW_OBJECT(GWEN_KEYSPEC, ks);
#ifdef GWEN_MEMTRACE
  GWEN_KeySpec_Count++;
  DBG_INFO(0, "New KeySpec (now %d)", GWEN_KeySpec_Count);
#endif
  return ks;
}



void GWEN_KeySpec_free(GWEN_KEYSPEC *ks){
  if (ks) {
#ifdef GWEN_MEMTRACE
    assert(GWEN_KeySpec_Count);
    GWEN_KeySpec_Count--;
    DBG_INFO(0, "Free KeySpec (now %d)", GWEN_KeySpec_Count);
#endif
    free(ks->keyType);
    free(ks->keyName);
    free(ks->owner);
    free(ks);
  }
}



GWEN_KEYSPEC *GWEN_KeySpec_dup(const GWEN_KEYSPEC *ks){
  GWEN_KEYSPEC *newKs;

  assert(ks);
  newKs=GWEN_KeySpec_new();
  if (ks->keyType)
    newKs->keyType=strdup(ks->keyType);
  if (ks->keyName)
    newKs->keyName=strdup(ks->keyName);
  if (ks->owner)
    newKs->owner=strdup(ks->owner);
  newKs->number=ks->number;
  newKs->version=ks->version;
  return newKs;
}



GWEN_KEYSPEC *GWEN_KeySpec_Next(GWEN_KEYSPEC *ks){
  assert(ks);
  return ks->next;
}


const GWEN_KEYSPEC *GWEN_KeySpec_ConstNext(const GWEN_KEYSPEC *ks){
  assert(ks);
  return ks->next;
}



void GWEN_KeySpec_Add(GWEN_KEYSPEC *ks,
                             GWEN_KEYSPEC **head){
  assert(ks);
  assert(head);
  GWEN_LIST_ADD(GWEN_KEYSPEC, ks, head);
}



void GWEN_KeySpec_Del(GWEN_KEYSPEC *ks,
                             GWEN_KEYSPEC **head){
  assert(ks);
  assert(head);
  GWEN_LIST_DEL(GWEN_KEYSPEC, ks, head);
}



const char *GWEN_KeySpec_GetKeyType(const GWEN_KEYSPEC *ks){
  assert(ks);
  return ks->keyType;
}



void GWEN_KeySpec_SetKeyType(GWEN_KEYSPEC *ks,
                                    const char *s){
  assert(ks);
  assert(s);
  free(ks->keyType);
  ks->keyType=strdup(s);
}



const char *GWEN_KeySpec_GetKeyName(const GWEN_KEYSPEC *ks){
  assert(ks);
  return ks->keyName;
}



void GWEN_KeySpec_SetKeyName(GWEN_KEYSPEC *ks,
                                    const char *s){
  assert(ks);
  assert(s);
  free(ks->keyName);
  ks->keyName=strdup(s);
}



const char *GWEN_KeySpec_GetOwner(const GWEN_KEYSPEC *ks){
  assert(ks);
  return ks->owner;
}



void GWEN_KeySpec_SetOwner(GWEN_KEYSPEC *ks,
                                  const char *s){
  assert(ks);
  assert(s);
  free(ks->owner);
  ks->owner=strdup(s);
}



unsigned int GWEN_KeySpec_GetNumber(const GWEN_KEYSPEC *ks){
  assert(ks);
  return ks->number;
}



void GWEN_KeySpec_SetNumber(GWEN_KEYSPEC *ks,
                                   unsigned int i){
  assert(ks);
  ks->number=i;
}



unsigned int GWEN_KeySpec_GetVersion(const GWEN_KEYSPEC *ks){
  assert(ks);
  return ks->version;
}



void GWEN_KeySpec_SetVersion(GWEN_KEYSPEC *ks,
                             unsigned int i){
  assert(ks);
  ks->version=i;
}



void GWEN_KeySpec_Clear(GWEN_KEYSPEC **head){
  GWEN_KEYSPEC *ks;

  ks=*head;
  DBG_INFO(0, "Clearing list of KeySpecs");
  while(ks) {
    GWEN_KEYSPEC *nks;

    nks=ks->next;
    GWEN_KeySpec_free(ks);
    ks=nks;
  } /* while */
  DBG_INFO(0, "Clearing list of KeySpecs: done");
  *head=0;
}



void GWEN_KeySpec_Dump(const GWEN_KEYSPEC *ks, FILE *f, int indent){
  unsigned int i;

  assert(ks);
  for (i=0; i<indent; i++) fprintf(f, " ");
  fprintf(f, "GWEN_KeySpec\n");
  for (i=0; i<indent; i++) fprintf(f, " ");
  fprintf(f, "--------------------------------------------------\n");
  for (i=0; i<indent; i++) fprintf(f, " ");
  fprintf(f, "KeyType : %s\n", ks->keyType);
  for (i=0; i<indent; i++) fprintf(f, " ");
  fprintf(f, "KeyName : %s\n", ks->keyName);
  for (i=0; i<indent; i++) fprintf(f, " ");
  fprintf(f, "Owner   : %s\n", ks->owner);
  for (i=0; i<indent; i++) fprintf(f, " ");
  fprintf(f, "Number  : %d\n", ks->number);
  for (i=0; i<indent; i++) fprintf(f, " ");
  fprintf(f, "Version : %d\n", ks->version);
}






