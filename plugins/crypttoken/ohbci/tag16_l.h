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


#ifndef GWENHYWFAR_OHBCI_TAG16_L_H
#define GWENHYWFAR_OHBCI_TAG16_L_H

#include <gwenhywfar/buffer.h>
#include <gwenhywfar/misc.h>


typedef struct GWEN_TAG16 GWEN_TAG16;

GWEN_LIST_FUNCTION_DEFS(GWEN_TAG16, GWEN_TAG16)


GWEN_TAG16 *GWEN_TAG16_new();
void GWEN_TAG16_DirectlyToBuffer(unsigned int tagType,
                                const char *p,
                                int size,
                                GWEN_BUFFER *buf);

void GWEN_TAG16_free(GWEN_TAG16 *tlv);

GWEN_TAG16 *GWEN_TAG16_fromBuffer(GWEN_BUFFER *mbuf, int isBerTlv);

unsigned int GWEN_TAG16_GetTagType(const GWEN_TAG16 *tlv);
unsigned int GWEN_TAG16_GetTagLength(const GWEN_TAG16 *tlv);
const void *GWEN_TAG16_GetTagData(const GWEN_TAG16 *tlv);

unsigned int GWEN_TAG16_GetTagSize(const GWEN_TAG16 *tlv);




#endif /* GWENHYWFAR_OHBCI_TAG16_L_H */

