/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sun Jun 13 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWENHYWFAR_TLV_P_H
#define GWENHYWFAR_TLV_P_H


#include <gwenhywfar/tlv.h>


struct GWEN_TLV {
  GWEN_LIST_ELEMENT(GWEN_TLV)
  int isBerTlv;
  unsigned int tagMode;
  unsigned int tagSize;
  unsigned int tagType;
  unsigned int tagLength;
  void *tagData;
};




#endif /* GWENHYWFAR_TLV_P_H */
