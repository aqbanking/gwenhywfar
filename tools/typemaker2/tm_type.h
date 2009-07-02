/***************************************************************************
    begin       : Wed Jul 01 2009
    copyright   : (C) 2009 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef TYPEMAKER2_TYPE_H
#define TYPEMAKER2_TYPE_H

#include <gwenhywfar/stringlist.h>
#include <gwenhywfar/list1.h>

#include <stdio.h>


typedef struct TYPEMAKER2_TYPE TYPEMAKER2_TYPE;
GWEN_LIST_FUNCTION_DEFS(TYPEMAKER2_TYPE, Typemaker2_Type);

#define TYPEMAKER2_FLAGS_OWN          0x00000001
#define TYPEMAKER2_FLAGS_VOLATILE     0x00000002
#define TYPEMAKER2_FLAGS_CONST        0x00000004
#define TYPEMAKER2_FLAGS_STATIC       0x00000008
#define TYPEMAKER2_FLAGS_DUP          0x00000010

#define TYPEMAKER2_FLAGS_WITH_DB      0x00000020
#define TYPEMAKER2_FLAGS_WITH_XML     0x00000040
#define TYPEMAKER2_FLAGS_WITH_OBJECT  0x00000080
#define TYPEMAKER2_FLAGS_WITH_LIST1   0x00000100
#define TYPEMAKER2_FLAGS_WITH_LIST2   0x00000200
#define TYPEMAKER2_FLAGS_WITH_INHERIT 0x00000400
#define TYPEMAKER2_FLAGS_WITH_IDMAP   0x00000800

/** special value which is used to overwrite flags from extended types */
#define TYPEMAKER2_FLAGS_NONE         0x80000000


enum {
  TypeMaker2_Type_Unknown=0,
  TypeMaker2_Type_Opaque,
  TypeMaker2_Type_Pointer,
  TypeMaker2_Type_Array
};



enum {
  TypeMaker2_Access_Unknown=0,
  TypeMaker2_Access_Public,
  TypeMaker2_Access_Library,
  TypeMaker2_Access_Protected,
  TypeMaker2_Access_Private
};


#include "tm_member.h"
#include "tm_header.h"



TYPEMAKER2_TYPE *Typemaker2_Type_new();
void Typemaker2_Type_free(TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_Attach(TYPEMAKER2_TYPE *ty);


const char *Typemaker2_Type_GetName(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetName(TYPEMAKER2_TYPE *ty, const char *s);

const char *Typemaker2_Type_GetExtends(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetExtends(TYPEMAKER2_TYPE *ty, const char *s);

const char *Typemaker2_Type_GetBaseType(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetBaseType(TYPEMAKER2_TYPE *ty, const char *s);


int Typemaker2_Type_GetType(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetType(TYPEMAKER2_TYPE *ty, int i);

uint32_t Typemaker2_Type_GetFlags(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetFlags(TYPEMAKER2_TYPE *ty, uint32_t i);

uint32_t Typemaker2_Type_GetSetFlags(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetSetFlags(TYPEMAKER2_TYPE *ty, uint32_t i);

uint32_t Typemaker2_Type_GetGetFlags(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetGetFlags(TYPEMAKER2_TYPE *ty, uint32_t i);

uint32_t Typemaker2_Type_GetDupFlags(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetDupFlags(TYPEMAKER2_TYPE *ty, uint32_t i);

int Typemaker2_Type_GetAccess(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetAccess(TYPEMAKER2_TYPE *ty, int i);


const char *Typemaker2_Type_GetDefaultValue(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetDefaultValue(TYPEMAKER2_TYPE *ty, const char *s);

const char *Typemaker2_Type_GetPresetValue(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetPresetValue(TYPEMAKER2_TYPE *ty, const char *s);

const char *Typemaker2_Type_GetIdentifier(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetIdentifier(TYPEMAKER2_TYPE *ty, const char *s);

const char *Typemaker2_Type_GetPrefix(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetPrefix(TYPEMAKER2_TYPE *ty, const char *s);

const char *Typemaker2_Type_GetInherits(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetInherits(TYPEMAKER2_TYPE *ty, const char *s);

const char *Typemaker2_Type_GetBaseType(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetBaseType(TYPEMAKER2_TYPE *ty, const char *s);

const char *Typemaker2_Type_GetExtends(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetExtends(TYPEMAKER2_TYPE *ty, const char *s);


const char *Typemaker2_Type_GetCodeConstruct(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetCodeConstruct(TYPEMAKER2_TYPE *ty, const char *s);

const char *Typemaker2_Type_GetCodeDestruct(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetCodeDestruct(TYPEMAKER2_TYPE *ty, const char *s);

const char *Typemaker2_Type_GetCodeAssign(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetCodeAssign(TYPEMAKER2_TYPE *ty, const char *s);

const char *Typemaker2_Type_GetCodeDup(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetCodeDup(TYPEMAKER2_TYPE *ty, const char *s);

const char *Typemaker2_Type_GetCodeFromDb(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetCodeFromDb(TYPEMAKER2_TYPE *ty, const char *s);

const char *Typemaker2_Type_GetCodeToDb(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetCodeToDb(TYPEMAKER2_TYPE *ty, const char *s);


const char *Typemaker2_Type_GetCodeFromXml(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetCodeFromXml(TYPEMAKER2_TYPE *ty, const char *s);

const char *Typemaker2_Type_GetCodeToXml(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetCodeToXml(TYPEMAKER2_TYPE *ty, const char *s);

const char *Typemaker2_Type_GetCodeToXml(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetCodeToXml(TYPEMAKER2_TYPE *ty, const char *s);

const char *Typemaker2_Type_GetCodeFromObject(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetCodeFromObject(TYPEMAKER2_TYPE *ty, const char *s);

const char *Typemaker2_Type_GetCodeToObject(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetCodeToObject(TYPEMAKER2_TYPE *ty, const char *s);


TYPEMAKER2_MEMBER_LIST *Typemaker2_Type_GetMembers(const TYPEMAKER2_TYPE *ty);


int Typemaker2_Type_GetNonVolatileMemberCount(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetNonVolatileMemberCount(TYPEMAKER2_TYPE *ty, int i);


TYPEMAKER2_TYPE *Typemaker2_Type_GetExtendsPtr(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetExtendsPtr(TYPEMAKER2_TYPE *ty, TYPEMAKER2_TYPE *p);

TYPEMAKER2_TYPE *Typemaker2_Type_GetBaseTypePtr(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetBaseTypePtr(TYPEMAKER2_TYPE *ty, TYPEMAKER2_TYPE *p);

TYPEMAKER2_TYPE *Typemaker2_Type_GetInheritsPtr(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetInheritsPtr(TYPEMAKER2_TYPE *ty, TYPEMAKER2_TYPE *p);


int Typemaker2_Type_readXml(TYPEMAKER2_TYPE *ty, GWEN_XMLNODE *node, const char *wantedLang);


void Typemaker2_Type_Dump(TYPEMAKER2_TYPE *ty, FILE *f, int indent);



#endif



