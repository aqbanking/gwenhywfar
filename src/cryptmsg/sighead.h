/***************************************************************************
    begin       : Sun Nov 30 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_CRYPTMGR_SIGHEAD_H
#define GWEN_CRYPTMGR_SIGHEAD_H

#include <gwenhywfar/list1.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/gwentime.h>



typedef struct GWEN_SIGHEAD GWEN_SIGHEAD;
GWEN_LIST_FUNCTION_LIB_DEFS(GWEN_SIGHEAD, GWEN_SigHead, GWENHYWFAR_API)


GWEN_SIGHEAD *GWEN_SigHead_new();
void GWEN_SigHead_free(GWEN_SIGHEAD *sh);

GWEN_SIGHEAD *GWEN_SigHead_fromBuffer(const uint8_t *p, uint32_t l);
int GWEN_SigHead_toBuffer(const GWEN_SIGHEAD *sh, GWEN_BUFFER *buf, uint8_t tagType);


const char *GWEN_SigHead_GetKeyName(const GWEN_SIGHEAD *sh);
void GWEN_SigHead_SetKeyName(GWEN_SIGHEAD *sh, const char *s);

int GWEN_SigHead_GetKeyNumber(const GWEN_SIGHEAD *sh);
void GWEN_SigHead_SetKeyNumber(GWEN_SIGHEAD *sh, int i);

int GWEN_SigHead_GetKeyVersion(const GWEN_SIGHEAD *sh);
void GWEN_SigHead_SetKeyVersion(GWEN_SIGHEAD *sh, int i);

const GWEN_TIME *GWEN_SigHead_GetDateTime(const GWEN_SIGHEAD *sh);
void GWEN_SigHead_SetDateTime(GWEN_SIGHEAD *sh, const GWEN_TIME *ti);

int GWEN_SigHead_GetSignatureProfile(const GWEN_SIGHEAD *sh);
void GWEN_SigHead_SetSignatureProfile(GWEN_SIGHEAD *sh, int i);

int GWEN_SigHead_GetSignatureNumber(const GWEN_SIGHEAD *sh);
void GWEN_SigHead_SetSignatureNumber(GWEN_SIGHEAD *sh, int i);


#endif

