/***************************************************************************
    begin       : Mon Dec 01 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWEN_CRYPTMGR_ENCRYPTEDOBJECT_H
#define GWEN_CRYPTMGR_ENCRYPTEDOBJECT_H


#include <gwenhywfar/gwenhywfarapi.h>


typedef struct GWEN_ENCRYPTED_OBJECT GWEN_ENCRYPTED_OBJECT;


#include <gwenhywfar/crypthead.h>


GWEN_ENCRYPTED_OBJECT *GWEN_EncryptedObject_new();
void GWEN_EncryptedObject_free(GWEN_ENCRYPTED_OBJECT *eo);
GWEN_ENCRYPTED_OBJECT *GWEN_EncryptedObject_fromBuffer(const uint8_t *p, uint32_t l, int doCopy);
int GWEN_EncryptedObject_toBuffer(const GWEN_ENCRYPTED_OBJECT *eo, GWEN_BUFFER *buf, uint8_t tagType);


const uint8_t *GWEN_EncryptedObject_GetDataPtr(const GWEN_ENCRYPTED_OBJECT *eo);
uint32_t GWEN_EncryptedObject_GetDataLen(const GWEN_ENCRYPTED_OBJECT *eo);
void GWEN_EncryptedObject_SetData(GWEN_ENCRYPTED_OBJECT *eo, const uint8_t *p, uint32_t l);

GWEN_CRYPTHEAD *GWEN_EncryptedObject_GetCryptHead(const GWEN_ENCRYPTED_OBJECT *eo);
void GWEN_EncryptedObject_SetCryptHead(GWEN_ENCRYPTED_OBJECT *eo, GWEN_CRYPTHEAD *ch);


#endif

