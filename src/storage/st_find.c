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

#include "st_find_p.h"


#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>



GWEN_INHERIT_FUNCTIONS(GWEN_STO_FIND)
GWEN_LIST_FUNCTIONS(GWEN_STO_FIND, GWEN_StoFind)




GWEN_STO_FIND *GWEN_StoFind_new(GWEN_TYPE_UINT32 id) {
  GWEN_STO_FIND *fnd;

  GWEN_NEW_OBJECT(GWEN_STO_FIND, fnd);
  GWEN_INHERIT_INIT(GWEN_STO_FIND, fnd);
  GWEN_LIST_INIT(GWEN_STO_FIND, fnd);

  fnd->id=id;

  return fnd;
}



void GWEN_StoFind_free(GWEN_STO_FIND *fnd) {
  if (fnd) {
    GWEN_LIST_FINI(GWEN_STO_FIND, fnd);
    GWEN_INHERIT_FINI(GWEN_STO_FIND, fnd);
    GWEN_FREE_OBJECT(fnd);
  }
}



GWEN_TYPE_UINT32 GWEN_StoFind_GetId(const GWEN_STO_FIND *fnd) {
  assert(fnd);
  return fnd->id;
}










