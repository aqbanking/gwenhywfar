/***************************************************************************
 $RCSfile: account.h,v $
 -------------------
 cvs         : $Id: account.h,v 1.16 2006/02/22 19:38:54 aquamaniac Exp $
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "smp_find_p.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>


GWEN_INHERIT(GWEN_STO_FIND, GWEN_SMPSTO_FIND)


GWEN_STO_FIND *GWEN_SmpStoFind_new(GWEN_TYPE_UINT32 id,
                                   const GWEN_IDLIST *objectIdList) {
  GWEN_STO_FIND *fnd;
  GWEN_SMPSTO_FIND *xf;

  assert(objectIdList);
  fnd=GWEN_StoFind_new(id);
  GWEN_NEW_OBJECT(GWEN_SMPSTO_FIND, xf);
  xf->objectIdList=GWEN_IdList_dup(objectIdList);
  GWEN_INHERIT_SETDATA(GWEN_STO_FIND, GWEN_SMPSTO_FIND, fnd, xf,
                       GWEN_SmpStoFind_FreeData);

  return fnd;
}



void GWEN_SmpStoFind_FreeData(void *bp, void *p) {
  GWEN_SMPSTO_FIND *xf;

  xf=(GWEN_SMPSTO_FIND*) p;
  GWEN_IdList_free(xf->objectIdList);
  GWEN_FREE_OBJECT(xf);
}



GWEN_TYPE_UINT32 GWEN_SmpStoFind_GetFirstId(GWEN_STO_FIND *fnd) {
  GWEN_SMPSTO_FIND *xf;

  assert(fnd);
  xf=GWEN_INHERIT_GETDATA(GWEN_STO_FIND, GWEN_SMPSTO_FIND, fnd);
  assert(xf);

  return GWEN_IdList_GetFirstId(xf->objectIdList);
}



GWEN_TYPE_UINT32 GWEN_SmpStoFind_GetNextId(GWEN_STO_FIND *fnd) {
  GWEN_SMPSTO_FIND *xf;

  assert(fnd);
  xf=GWEN_INHERIT_GETDATA(GWEN_STO_FIND, GWEN_SMPSTO_FIND, fnd);
  assert(xf);

  return GWEN_IdList_GetNextId(xf->objectIdList);
}







