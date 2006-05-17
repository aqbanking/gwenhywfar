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

#include "smp_type_p.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>


GWEN_INHERIT(GWEN_STO_TYPE, GWEN_SMPSTO_TYPE)


GWEN_STO_TYPE *GWEN_SmpStoType_new(GWEN_STO_STORAGE *storage,
				   GWEN_TYPE_UINT32 id,
				   const char *typeName,
				   const char *name) {
  GWEN_STO_TYPE *ty;
  GWEN_SMPSTO_TYPE *xty;

  ty=GWEN_StoType_new(storage, id, typeName, name);
  GWEN_NEW_OBJECT(GWEN_SMPSTO_TYPE, xty);
  GWEN_INHERIT_SETDATA(GWEN_STO_TYPE, GWEN_SMPSTO_TYPE, ty, xty,
		       GWEN_SmpStoType_FreeData);

  xty->objectIdList=GWEN_IdList_new();

  return ty;
}



GWEN_STO_TYPE *GWEN_SmpStoType_dup(const GWEN_STO_TYPE *oty) {
  GWEN_STO_TYPE *ty;
  GWEN_SMPSTO_TYPE *xty;

  ty=GWEN_StoType_dup(oty);
  GWEN_NEW_OBJECT(GWEN_SMPSTO_TYPE, xty);
  GWEN_INHERIT_SETDATA(GWEN_STO_TYPE, GWEN_SMPSTO_TYPE, ty, xty,
		       GWEN_SmpStoType_FreeData);
  xty->objectIdList=GWEN_IdList_dup(GWEN_SmpStoType_GetObjectIdList(oty));

  return ty;
}



void GWEN_SmpStoType_FreeData(void *bp, void *p) {
  GWEN_SMPSTO_TYPE *xty;

  xty=(GWEN_SMPSTO_TYPE*) p;

  GWEN_IdList_free(xty->objectIdList);

  GWEN_FREE_OBJECT(xty);
}




GWEN_IDLIST *GWEN_SmpStoType_GetObjectIdList(const GWEN_STO_TYPE *ty) {
  GWEN_SMPSTO_TYPE *xty;

  assert(ty);
  xty=GWEN_INHERIT_GETDATA(GWEN_STO_TYPE, GWEN_SMPSTO_TYPE, ty);
  assert(xty);

  return xty->objectIdList;
}










