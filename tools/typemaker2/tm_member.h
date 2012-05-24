/***************************************************************************
    begin       : Thu Jul 02 2009
    copyright   : (C) 2009 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



#ifndef TYPEMAKER2_MEMBER_H
#define TYPEMAKER2_MEMBER_H

#include <gwenhywfar/list1.h>
#include <gwenhywfar/xml.h>

#include <stdio.h>


typedef struct TYPEMAKER2_MEMBER TYPEMAKER2_MEMBER;
GWEN_LIST_FUNCTION_DEFS(TYPEMAKER2_MEMBER, Typemaker2_Member);


#include "tm_type.h"
#include "tm_enum.h"
#include "tm_define.h"


TYPEMAKER2_MEMBER *Typemaker2_Member_new();
void Typemaker2_Member_free(TYPEMAKER2_MEMBER *tm);
void Typemaker2_Member_Attach(TYPEMAKER2_MEMBER *tm);


const char *Typemaker2_Member_GetName(const TYPEMAKER2_MEMBER *tm);
void Typemaker2_Member_SetName(TYPEMAKER2_MEMBER *tm, const char *s);

const char *Typemaker2_Member_GetTypeName(const TYPEMAKER2_MEMBER *tm);
void Typemaker2_Member_SetTypeName(TYPEMAKER2_MEMBER *tm, const char *s);

const char *Typemaker2_Member_GetDescription(const TYPEMAKER2_MEMBER *tm);
void Typemaker2_Member_SetDescription(TYPEMAKER2_MEMBER *tm, const char *s);

const char *Typemaker2_Member_GetFieldId(const TYPEMAKER2_MEMBER *tm);
void Typemaker2_Member_SetFieldId(TYPEMAKER2_MEMBER *tm, const char *s);

const char *Typemaker2_Member_GetEnumId(const TYPEMAKER2_MEMBER *tm);
void Typemaker2_Member_SetEnumId(TYPEMAKER2_MEMBER *tm, const char *s);

const char *Typemaker2_Member_GetDefineId(const TYPEMAKER2_MEMBER *tm);
void Typemaker2_Member_SetDefineId(TYPEMAKER2_MEMBER *tm, const char *s);

uint32_t Typemaker2_Member_GetFlags(const TYPEMAKER2_MEMBER *tm);
void Typemaker2_Member_SetFlags(TYPEMAKER2_MEMBER *tm, uint32_t i);

uint32_t Typemaker2_Member_GetSetFlags(const TYPEMAKER2_MEMBER *tm);
void Typemaker2_Member_SetSetFlags(TYPEMAKER2_MEMBER *tm, uint32_t i);

uint32_t Typemaker2_Member_GetGetFlags(const TYPEMAKER2_MEMBER *tm);
void Typemaker2_Member_SetGetFlags(TYPEMAKER2_MEMBER *tm, uint32_t i);

uint32_t Typemaker2_Member_GetDupFlags(const TYPEMAKER2_MEMBER *tm);
void Typemaker2_Member_SetDupFlags(TYPEMAKER2_MEMBER *tm, uint32_t i);

TYPEMAKER2_TYPE *Typemaker2_Member_GetTypePtr(const TYPEMAKER2_MEMBER *tm);
void Typemaker2_Member_SetTypePtr(TYPEMAKER2_MEMBER *tm, TYPEMAKER2_TYPE *ty);

TYPEMAKER2_ENUM *Typemaker2_Member_GetEnumPtr(const TYPEMAKER2_MEMBER *tm);
void Typemaker2_Member_SetEnumPtr(TYPEMAKER2_MEMBER *tm, TYPEMAKER2_ENUM *te);

TYPEMAKER2_DEFINE *Typemaker2_Member_GetDefinePtr(const TYPEMAKER2_MEMBER *tm);
void Typemaker2_Member_SetDefinePtr(TYPEMAKER2_MEMBER *tm, TYPEMAKER2_DEFINE *td);

int Typemaker2_Member_GetAccess(const TYPEMAKER2_MEMBER *tm);
void Typemaker2_Member_SetAccess(TYPEMAKER2_MEMBER *tm, int i);

const char *Typemaker2_Member_GetDefaultValue(const TYPEMAKER2_MEMBER *tm);
void Typemaker2_Member_SetDefaultValue(TYPEMAKER2_MEMBER *tm, const char *s);

const char *Typemaker2_Member_GetPresetValue(const TYPEMAKER2_MEMBER *tm);
void Typemaker2_Member_SetPresetValue(TYPEMAKER2_MEMBER *tm, const char *s);

int Typemaker2_Member_GetMaxLen(const TYPEMAKER2_MEMBER *tm);
void Typemaker2_Member_SetMaxLen(TYPEMAKER2_MEMBER *tm, int i);

const char *Typemaker2_Member_GetAqDbType(const TYPEMAKER2_MEMBER *tm);
void Typemaker2_Member_SetAqDbType(TYPEMAKER2_MEMBER *tm, const char *s);

int Typemaker2_Member_GetMemberPosition(const TYPEMAKER2_MEMBER *tm);
void Typemaker2_Member_SetMemberPosition(TYPEMAKER2_MEMBER *tm, int i);

int Typemaker2_Member_readXml(TYPEMAKER2_MEMBER *tm, GWEN_XMLNODE *node);


void Typemaker2_Member_Dump(TYPEMAKER2_MEMBER *tm, FILE *f, int indent);


#endif



