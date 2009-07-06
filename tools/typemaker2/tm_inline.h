/***************************************************************************
    begin       : Thu Jul 02 2009
    copyright   : (C) 2009 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



#ifndef TYPEMAKER2_INLINE_H
#define TYPEMAKER2_INLINE_H

#include <gwenhywfar/list1.h>
#include <gwenhywfar/xml.h>


typedef struct TYPEMAKER2_INLINE TYPEMAKER2_INLINE;
GWEN_LIST_FUNCTION_DEFS(TYPEMAKER2_INLINE, Typemaker2_Inline)


enum {
  Typemaker2_InlineLocation_Unknown=-1,
  Typemaker2_InlineLocation_Header=0,
  Typemaker2_InlineLocation_Code
};


TYPEMAKER2_INLINE *Typemaker2_Inline_new();
void Typemaker2_Inline_free(TYPEMAKER2_INLINE *th);
void Typemaker2_Inline_Attach(TYPEMAKER2_INLINE *th);


const char *Typemaker2_Inline_GetContent(const TYPEMAKER2_INLINE *th);
void Typemaker2_Inline_SetContent(TYPEMAKER2_INLINE *th, const char *s);

int Typemaker2_Inline_GetAccess(const TYPEMAKER2_INLINE *th);
void Typemaker2_Inline_SetAccess(TYPEMAKER2_INLINE *th, int i);

int Typemaker2_Inline_GetLocation(const TYPEMAKER2_INLINE *th);
void Typemaker2_Inline_SetLocation(TYPEMAKER2_INLINE *th, int i);

int Typemaker2_Inline_GetTypeFlagsMask(const TYPEMAKER2_INLINE *th);
int Typemaker2_Inline_GetTypeFlagsValue(const TYPEMAKER2_INLINE *th);

TYPEMAKER2_INLINE *Typemaker2_Inline_fromXml(GWEN_XMLNODE *node);



#endif



