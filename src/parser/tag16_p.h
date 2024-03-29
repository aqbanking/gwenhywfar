/***************************************************************************
    begin       : Sun Jun 13 2004
    copyright   : (C) 2023 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWENHYWFAR_PARSER_TAG16_P_H
#define GWENHYWFAR_PARSER_TAG16_P_H


#include "tag16.h"


struct GWEN_TAG16 {
  GWEN_LIST_ELEMENT(GWEN_TAG16)
  unsigned int tagSize;
  unsigned int tagType;
  unsigned int tagLength;
  const uint8_t *tagData;
  int dataOwned;
};




#endif /* GWENHYWFAR_PARSER_TAG16_P_H */
