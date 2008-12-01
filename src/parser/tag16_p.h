/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: tag16_p.h 790 2005-07-23 09:32:08Z aquamaniac $
    begin       : Sun Jun 13 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWENHYWFAR_OHBCI_TAG16_P_H
#define GWENHYWFAR_OHBCI_TAG16_P_H


#include "tag16.h"


struct GWEN_TAG16 {
  GWEN_LIST_ELEMENT(GWEN_TAG16)
  unsigned int tagSize;
  unsigned int tagType;
  unsigned int tagLength;
  void *tagData;
  int dataOwned;
};




#endif /* GWENHYWFAR_OHBCI_TAG16_P_H */
