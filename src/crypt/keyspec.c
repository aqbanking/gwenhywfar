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

#include <strings.h>


#include "keyspec_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>


GWEN_INHERIT_FUNCTIONS(GWEN_KEYSPEC)
GWEN_LIST2_FUNCTIONS(GWEN_KEYSPEC, GWEN_KeySpec)


GWEN_KEYSPEC *GWEN_KeySpec_new(){
  GWEN_KEYSPEC *ks;

  GWEN_NEW_OBJECT(GWEN_KEYSPEC, ks);
  DBG_MEM_INC("GWEN_KEYSPEC", 0);
  GWEN_INHERIT_INIT(GWEN_KEYSPEC, ks);
  ks->number=1;
  ks->version=1;
  return ks;
}



void GWEN_KeySpec_free(GWEN_KEYSPEC *ks){
  if (ks) {
    DBG_MEM_DEC("GWEN_KEYSPEC");
    GWEN_INHERIT_FINI(GWEN_KEYSPEC, ks);
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
  newKs->status=ks->status;
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



void GWEN_KeySpec_SetOwner(GWEN_KEYSPEC *ks, const char *s){
  assert(ks);
  free(ks->owner);
  if (s) ks->owner=strdup(s);
  else ks->owner=0;
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



int GWEN_KeySpec_GetStatus(const GWEN_KEYSPEC *ks){
  assert(ks);
  return ks->status;
}



void GWEN_KeySpec_SetStatus(GWEN_KEYSPEC *ks, int i){
  assert(ks);
  ks->status=i;
}



void GWEN_KeySpec_Clear(GWEN_KEYSPEC **head){
  GWEN_KEYSPEC *ks;

  ks=*head;
  DBG_INFO(GWEN_LOGDOMAIN, "Clearing list of KeySpecs");
  while(ks) {
    GWEN_KEYSPEC *nks;

    nks=ks->next;
    GWEN_KeySpec_free(ks);
    ks=nks;
  } /* while */
  DBG_INFO(GWEN_LOGDOMAIN, "Clearing list of KeySpecs: done");
  *head=0;
}



void GWEN_KeySpec_Dump(const GWEN_KEYSPEC *ks, FILE *f, unsigned int indent){
  unsigned int i;

  assert(ks);
  for (i=0; i<indent; i++) fprintf(f, " ");
  fprintf(f, "GWEN_KeySpec\n");
  for (i=0; i<indent; i++) fprintf(f, " ");
  fprintf(f, "--------------------------------------------------\n");
  for (i=0; i<indent; i++) fprintf(f, " ");
  fprintf(f, "Status : %02x\n", ks->status);
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



int GWEN_KeySpec_ToDb(const GWEN_KEYSPEC *ks, GWEN_DB_NODE *n) {
  GWEN_DB_SetCharValue(n,
                       GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "type", ks->keyType);
  GWEN_DB_SetCharValue(n,
                       GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "name", ks->keyName);
  GWEN_DB_SetCharValue(n,
                       GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "owner", ks->owner);
  GWEN_DB_SetIntValue(n,
                      GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "number", ks->number);
  GWEN_DB_SetIntValue(n,
                      GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "version", ks->version);
  GWEN_DB_SetIntValue(n,
                      GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "status", ks->status);

  return 0;
}



int GWEN_KeySpec_FromDb(GWEN_KEYSPEC *ks, GWEN_DB_NODE *db){
  assert(ks);
  assert(db);
  GWEN_KeySpec_SetKeyName(ks,
                          GWEN_DB_GetCharValue(db, "name", 0, ""));
  GWEN_KeySpec_SetOwner(ks,
                        GWEN_DB_GetCharValue(db, "owner", 0, ""));
  GWEN_KeySpec_SetNumber(ks,
                         GWEN_DB_GetIntValue(db, "number", 0, 0));
  GWEN_KeySpec_SetVersion(ks,
                          GWEN_DB_GetIntValue(db, "version", 0, 0));
  GWEN_KeySpec_SetStatus(ks,
                         GWEN_DB_GetIntValue(db, "status", 0, 0));
  return 0;
}



GWEN_KEYSPEC *GWEN_KeySpec_List2__freeAll_cb(GWEN_KEYSPEC *st, void *user_data) {
  GWEN_KeySpec_free(st);
return 0;
}


void GWEN_KeySpec_List2_freeAll(GWEN_KEYSPEC_LIST2 *stl) {
  if (stl) {
    GWEN_KeySpec_List2_ForEach(stl, GWEN_KeySpec_List2__freeAll_cb, 0);
    GWEN_KeySpec_List2_free(stl); 
  }
}



int GWEN_KeySpec_toDb(const GWEN_KEYSPEC *ks, GWEN_DB_NODE *n) {
  return GWEN_KeySpec_ToDb(ks, n);
}



GWEN_KEYSPEC *GWEN_KeySpec_fromDb(GWEN_DB_NODE *n) {
  GWEN_KEYSPEC *ks;
  int rv;

  ks=GWEN_KeySpec_new();
  rv=GWEN_KeySpec_FromDb(ks, n);
  if (rv) {
    GWEN_KeySpec_free(ks);
    return 0;
  }

  return ks;
}








