/***************************************************************************
    begin       : Sun Nov 30 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWEN_CRYPTMGR_SIGNEDOBJECT_H
#define GWEN_CRYPTMGR_SIGNEDOBJECT_H


#include <gwenhywfar/gwenhywfarapi.h>


typedef struct GWEN_SIGNED_OBJECT GWEN_SIGNED_OBJECT;


#include <gwenhywfar/sighead.h>
#include <gwenhywfar/sigtail.h>


GWEN_SIGNED_OBJECT *GWEN_SignedObject_new();
void GWEN_SignedObject_free(GWEN_SIGNED_OBJECT *so);
GWEN_SIGNED_OBJECT *GWEN_SignedObject_fromBuffer(const uint8_t *p, uint32_t l, int doCopy);
int GWEN_SignedObject_toBuffer(const GWEN_SIGNED_OBJECT *so, GWEN_BUFFER *buf, uint8_t tagType);


const uint8_t *GWEN_SignedObject_GetDataPtr(const GWEN_SIGNED_OBJECT *so);
uint32_t GWEN_SignedObject_GetDataLen(const GWEN_SIGNED_OBJECT *so);
void GWEN_SignedObject_SetData(GWEN_SIGNED_OBJECT *so, const uint8_t *p, uint32_t l);

GWEN_SIGHEAD_LIST *GWEN_SignedObject_GetSigHeadList(const GWEN_SIGNED_OBJECT *so);
void GWEN_SignedObject_AddSigHead(GWEN_SIGNED_OBJECT *so, GWEN_SIGHEAD *sh);

GWEN_SIGTAIL_LIST *GWEN_SignedObject_GetSigTailList(const GWEN_SIGNED_OBJECT *so);
void GWEN_SignedObject_AddSigTail(GWEN_SIGNED_OBJECT *so, GWEN_SIGTAIL *st);

GWEN_SIGHEAD *GWEN_SignedObject_GetSigHeadByNumber(const GWEN_SIGNED_OBJECT *so, int i);
GWEN_SIGTAIL *GWEN_SignedObject_GetSigTailByNumber(const GWEN_SIGNED_OBJECT *so, int i);


#endif

