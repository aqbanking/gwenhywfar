/***************************************************************************
    begin       : Mon Dec 01 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_CRYPTMGR_CRYPTHEAD_H
#define GWEN_CRYPTMGR_CRYPTHEAD_H

#include <gwenhywfar/list1.h>
#include <gwenhywfar/buffer.h>


typedef struct GWEN_CRYPTHEAD GWEN_CRYPTHEAD;
GWEN_LIST_FUNCTION_LIB_DEFS(GWEN_CRYPTHEAD, GWEN_CryptHead, GWENHYWFAR_API)


GWEN_CRYPTHEAD *GWEN_CryptHead_new();
void GWEN_CryptHead_free(GWEN_CRYPTHEAD *ch);

GWEN_CRYPTHEAD *GWEN_CryptHead_fromBuffer(const uint8_t *p, uint32_t l);
int GWEN_CryptHead_toBuffer(const GWEN_CRYPTHEAD *ch, GWEN_BUFFER *buf, uint8_t tagType);

const char *GWEN_CryptHead_GetKeyName(const GWEN_CRYPTHEAD *ch);
void GWEN_CryptHead_SetKeyName(GWEN_CRYPTHEAD *ch, const char *s);

int GWEN_CryptHead_GetKeyNumber(const GWEN_CRYPTHEAD *ch);
void GWEN_CryptHead_SetKeyNumber(GWEN_CRYPTHEAD *ch, int i);

int GWEN_CryptHead_GetKeyVersion(const GWEN_CRYPTHEAD *ch);
void GWEN_CryptHead_SetKeyVersion(GWEN_CRYPTHEAD *ch, int i);

int GWEN_CryptHead_GetCryptProfile(const GWEN_CRYPTHEAD *ch);
void GWEN_CryptHead_SetCryptProfile(GWEN_CRYPTHEAD *ch, int i);

const uint8_t *GWEN_CryptHead_GetKeyPtr(const GWEN_CRYPTHEAD *ch);
uint32_t GWEN_CryptHead_GetKeyLen(const GWEN_CRYPTHEAD *ch);
void GWEN_CryptHead_SetKey(GWEN_CRYPTHEAD *ch, const uint8_t *p, uint32_t l);

#endif

