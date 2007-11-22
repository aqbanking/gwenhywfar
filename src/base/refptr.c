/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sun Jan 25 2004
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

#include "refptr_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <stdlib.h>



GWEN_REFPTR_POBJECT *GWEN_RefPtrObject_new(void *dp, GWEN_REFPTR_INFO *rpi){
  GWEN_REFPTR_POBJECT *rpo;

  GWEN_NEW_OBJECT(GWEN_REFPTR_POBJECT, rpo);
  rpo->refCount=1;
  rpo->ptr=dp;
  rpo->infoPtr=rpi;
  if (rpi)
    GWEN_RefPtrInfo_Attach(rpi);
  return rpo;
}



void GWEN_RefPtrObject_free(GWEN_REFPTR_POBJECT *rpo){
  if (rpo) {
    assert(rpo->refCount);
    if (--(rpo->refCount)==0) {
      DBG_VERBOUS(GWEN_LOGDOMAIN, "Freeing GWEN_RefPtrObject");
      if ((rpo->flags & GWEN_REFPTR_FLAGS_AUTODELETE) &&
          rpo->infoPtr &&
          rpo->ptr) {
        if (rpo->infoPtr->freeFn)
          rpo->infoPtr->freeFn(rpo->ptr);
      }
      GWEN_FREE_OBJECT(rpo);
    }
  }
}



void GWEN_RefPtrObject_Attach(GWEN_REFPTR_POBJECT *rpo){
  assert(rpo);
  assert(rpo->refCount);
  rpo->refCount++;
}



uint32_t GWEN_RefPtrInfo_GetFlags(const GWEN_REFPTR_INFO *rpi){
  assert(rpi);
  return rpi->flags;
}



void GWEN_RefPtrInfo_SetFlags(GWEN_REFPTR_INFO *rpi, uint32_t fl){
  assert(rpi);
  rpi->flags=fl;
}



void GWEN_RefPtrInfo_AddFlags(GWEN_REFPTR_INFO *rpi, uint32_t fl){
  assert(rpi);
  rpi->flags|=fl;
}



void GWEN_RefPtrInfo_DelFlags(GWEN_REFPTR_INFO *rpi, uint32_t fl){
  assert(rpi);
  rpi->flags&=~fl;
}










GWEN_REFPTR_INFO *GWEN_RefPtrInfo_new(){
  GWEN_REFPTR_INFO *rpi;

  GWEN_NEW_OBJECT(GWEN_REFPTR_INFO, rpi);
  rpi->refCount=1;

  return rpi;
}



void GWEN_RefPtrInfo_free(GWEN_REFPTR_INFO *rpi){
  if (rpi) {
    assert(rpi->refCount);
    if (--(rpi->refCount)==0) {
      DBG_VERBOUS(GWEN_LOGDOMAIN, "Freeing GWEN_RefPtrInfo");
      GWEN_FREE_OBJECT(rpi);
    }
  }
}



void GWEN_RefPtrInfo_Attach(GWEN_REFPTR_INFO *rpi){
  assert(rpi);
  assert(rpi->refCount);
  rpi->refCount++;
}



void GWEN_RefPtrInfo_SetFreeFn(GWEN_REFPTR_INFO *rpi,
                               GWEN_REFPTR_INFO_FREE_FN f){
  assert(rpi);
  rpi->freeFn=f;
}



void GWEN_RefPtrInfo_SetDupFn(GWEN_REFPTR_INFO *rpi,
                              GWEN_REFPTR_INFO_DUP_FN f){
  assert(rpi);
  rpi->dupFn=f;
}













GWEN_REFPTR *GWEN_RefPtr_new(void *dp, GWEN_REFPTR_INFO *rpi){
  GWEN_REFPTR *rp;

  GWEN_NEW_OBJECT(GWEN_REFPTR, rp);
  rp->objectPtr=GWEN_RefPtrObject_new(dp, rpi);
  if (rpi)
    rp->objectPtr->flags=rpi->flags;
  return rp;
}



void GWEN_RefPtr_free(GWEN_REFPTR *rp){
  if (rp) {
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Freeing GWEN_RefPtr");
    GWEN_RefPtrObject_free(rp->objectPtr);
    GWEN_FREE_OBJECT(rp);
  }
}



GWEN_REFPTR *GWEN_RefPtr_dup(const GWEN_REFPTR *rp){
  GWEN_REFPTR *nrp;

  assert(rp);
  GWEN_NEW_OBJECT(GWEN_REFPTR, nrp);
  nrp->objectPtr=rp->objectPtr;
  GWEN_RefPtrObject_Attach(nrp->objectPtr);
  return nrp;
}



GWEN_REFPTR *GWEN_RefPtr_copy(const GWEN_REFPTR *rp){
  GWEN_REFPTR *nrp;

  assert(rp);
  if (rp->objectPtr) {
    if (rp->objectPtr->ptr) {
      if (rp->objectPtr->infoPtr) {
        if (rp->objectPtr->infoPtr->dupFn) {
          void *p;

          p=rp->objectPtr->infoPtr->dupFn(rp->objectPtr->ptr);
          nrp=GWEN_RefPtr_new(p, rp->objectPtr->infoPtr);
          return nrp;
        }
      }
    }
  }
  return 0;
}



void *GWEN_RefPtr_GetData(const GWEN_REFPTR *rp){
  if (!rp)
    return 0;
  if (rp->objectPtr) {
    return rp->objectPtr->ptr;
  }
  return 0;
}



void GWEN_RefPtr_SetData(GWEN_REFPTR *rp, void *dp, GWEN_REFPTR_INFO *rpi){
  assert(rp);

  if (rp->objectPtr) {
    GWEN_RefPtrObject_free(rp->objectPtr);
  }
  rp->objectPtr=GWEN_RefPtrObject_new(dp, rpi);
}



uint32_t GWEN_RefPtr_GetFlags(const GWEN_REFPTR *rp){
  assert(rp);
  if (rp->objectPtr)
    return rp->objectPtr->flags;
  DBG_INFO(GWEN_LOGDOMAIN, "No object in RefPtr");
  return 0;
}



void GWEN_RefPtr_SetFlags(GWEN_REFPTR *rp, uint32_t fl){
  assert(rp);
  if (rp->objectPtr)
    rp->objectPtr->flags=fl;
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "No object in RefPtr");
  }
}



void GWEN_RefPtr_AddFlags(GWEN_REFPTR *rp, uint32_t fl){
  assert(rp);
  if (rp->objectPtr)
    rp->objectPtr->flags|=fl;
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "No object in RefPtr");
  }
}



void GWEN_RefPtr_DelFlags(GWEN_REFPTR *rp, uint32_t fl){
  assert(rp);
  if (rp->objectPtr)
    rp->objectPtr->flags&=~fl;
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "No object in RefPtr");
  }
}









