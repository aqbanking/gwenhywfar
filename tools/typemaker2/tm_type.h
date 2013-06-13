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
#include <gwenhywfar/list2.h>

#include <stdio.h>


typedef struct TYPEMAKER2_TYPE TYPEMAKER2_TYPE;

GWEN_LIST_FUNCTION_DEFS(TYPEMAKER2_TYPE, Typemaker2_Type)
GWEN_LIST2_FUNCTION_DEFS(TYPEMAKER2_TYPE, Typemaker2_Type)


/* don't forget to add new flags to tm_util.c ! */
#define TYPEMAKER2_FLAGS_OWN               0x00000001
#define TYPEMAKER2_FLAGS_VOLATILE          0x00000002
#define TYPEMAKER2_FLAGS_CONST             0x00000004
#define TYPEMAKER2_FLAGS_STATIC            0x00000008
#define TYPEMAKER2_FLAGS_DUP               0x00000010
#define TYPEMAKER2_FLAGS_NODUP             0x00000020
#define TYPEMAKER2_FLAGS_ATTRIBUTE         0x00000040
#define TYPEMAKER2_FLAGS_ENUM              0x00000080
#define TYPEMAKER2_FLAGS_DEFINE            0x00000100
#define TYPEMAKER2_FLAGS_ASSIGN            0x00000200
#define TYPEMAKER2_FLAGS_OMIT              0x00000400
#define TYPEMAKER2_FLAGS_COPY              0x00000800
#define TYPEMAKER2_FLAGS_NOCOPY            0x00001000
#define TYPEMAKER2_FLAGS_SORTBYMEMBER      0x00002000
#define TYPEMAKER2_FLAGS_NOCONSTOBJECT     0x00004000

/** special value which is used to overwrite flags from extended types */
#define TYPEMAKER2_FLAGS_NONE              0x00008000

#define TYPEMAKER2_FLAGS_WITH_DB           0x00010000
#define TYPEMAKER2_FLAGS_WITH_XML          0x00020000
#define TYPEMAKER2_FLAGS_WITH_OBJECT       0x00040000
#define TYPEMAKER2_FLAGS_WITH_LIST1        0x00080000
#define TYPEMAKER2_FLAGS_WITH_LIST2        0x00100000
#define TYPEMAKER2_FLAGS_WITH_INHERIT      0x00200000
#define TYPEMAKER2_FLAGS_WITH_IDMAP        0x00400000
#define TYPEMAKER2_FLAGS_WITH_GETBYMEMBER  0x00800000
#define TYPEMAKER2_FLAGS_WITH_TREE         0x01000000
#define TYPEMAKER2_FLAGS_WITH_HASH         0x02000000
#define TYPEMAKER2_FLAGS_WITH_REFCOUNT     0x04000000
#define TYPEMAKER2_FLAGS_WITH_FLAGS        0x08000000
#define TYPEMAKER2_FLAGS_WITH_HASHNUM      0x10000000
#define TYPEMAKER2_FLAGS_WITH_SIGNALS      0x20000000
#define TYPEMAKER2_FLAGS_WITH_SLOTS        0x40000000
#define TYPEMAKER2_FLAGS_WITH_CACHEFNS     0x80000000



enum {
  TypeMaker2_Type_Unknown=0,
  TypeMaker2_Type_Opaque,
  TypeMaker2_Type_Pointer,
  TypeMaker2_Type_Array
};



enum {
  TypeMaker2_Access_Unknown=0,
  TypeMaker2_Access_Public,
  TypeMaker2_Access_Protected,
  TypeMaker2_Access_Library,
  TypeMaker2_Access_Private
};


#include "tm_member.h"
#include "tm_header.h"
#include "tm_enum.h"
#include "tm_define.h"
#include "tm_code.h"
#include "tm_inline.h"
#include "tm_virtualfn.h"
#include "tm_signal.h"
#include "tm_slot.h"




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

const char *Typemaker2_Type_GetAqDbType(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetAqDbType(TYPEMAKER2_TYPE *ty, const char *s);

const char *Typemaker2_Type_GetBaseFileName(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetBaseFileName(TYPEMAKER2_TYPE *ty, const char *s);

uint32_t Typemaker2_Type_GetFlags(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetFlags(TYPEMAKER2_TYPE *ty, uint32_t i);

uint32_t Typemaker2_Type_GetSetFlags(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetSetFlags(TYPEMAKER2_TYPE *ty, uint32_t i);

uint32_t Typemaker2_Type_GetGetFlags(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetGetFlags(TYPEMAKER2_TYPE *ty, uint32_t i);

uint32_t Typemaker2_Type_GetDupFlags(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetDupFlags(TYPEMAKER2_TYPE *ty, uint32_t i);

uint32_t Typemaker2_Type_GetCopyFlags(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetCopyFlags(TYPEMAKER2_TYPE *ty, uint32_t i);

int Typemaker2_Type_GetAccess(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetAccess(TYPEMAKER2_TYPE *ty, int i);

int Typemaker2_Type_GetUsePrivateConstructor(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetUsePrivateConstructor(TYPEMAKER2_TYPE *ty, int i);

const char *Typemaker2_Type_GetDescription(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetDescription(TYPEMAKER2_TYPE *ty, const char *s);

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


TYPEMAKER2_MEMBER_LIST *Typemaker2_Type_GetMembers(const TYPEMAKER2_TYPE *ty);
TYPEMAKER2_HEADER_LIST *Typemaker2_Type_GetHeaders(const TYPEMAKER2_TYPE *ty);
TYPEMAKER2_ENUM_LIST *Typemaker2_Type_GetEnums(const TYPEMAKER2_TYPE *ty);
TYPEMAKER2_DEFINE_LIST *Typemaker2_Type_GetDefines(const TYPEMAKER2_TYPE *ty);
TYPEMAKER2_CODE_LIST *Typemaker2_Type_GetCodeDefs(const TYPEMAKER2_TYPE *ty);
TYPEMAKER2_INLINE_LIST *Typemaker2_Type_GetInlines(const TYPEMAKER2_TYPE *ty);
TYPEMAKER2_VIRTUALFN_LIST *Typemaker2_Type_GetVirtualFns(const TYPEMAKER2_TYPE *ty);
TYPEMAKER2_SIGNAL_LIST *Typemaker2_Type_GetSignals(const TYPEMAKER2_TYPE *ty);
TYPEMAKER2_SLOT_LIST *Typemaker2_Type_GetSlots(const TYPEMAKER2_TYPE *ty);


int Typemaker2_Type_GetNonVolatileMemberCount(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetNonVolatileMemberCount(TYPEMAKER2_TYPE *ty, int i);


TYPEMAKER2_TYPE *Typemaker2_Type_GetExtendsPtr(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetExtendsPtr(TYPEMAKER2_TYPE *ty, TYPEMAKER2_TYPE *p);

TYPEMAKER2_TYPE *Typemaker2_Type_GetBaseTypePtr(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetBaseTypePtr(TYPEMAKER2_TYPE *ty, TYPEMAKER2_TYPE *p);

TYPEMAKER2_TYPE *Typemaker2_Type_GetInheritsPtr(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetInheritsPtr(TYPEMAKER2_TYPE *ty, TYPEMAKER2_TYPE *p);

const char *Typemaker2_Type_GetFieldCountId(const TYPEMAKER2_TYPE *ty);
void Typemaker2_Type_SetFieldCountId(TYPEMAKER2_TYPE *ty, const char *s);


TYPEMAKER2_ENUM *Typemaker2_Type_FindEnum(TYPEMAKER2_TYPE *ty, const char *s);
TYPEMAKER2_CODE *Typemaker2_Type_FindCodeForMember(const TYPEMAKER2_TYPE *ty,
                                                   const TYPEMAKER2_MEMBER *tm,
						   const char *id);


int Typemaker2_Type_readXml(TYPEMAKER2_TYPE *ty, GWEN_XMLNODE *node, const char *wantedLang);

void Typemaker2_Type_Dump(TYPEMAKER2_TYPE *ty, FILE *f, int indent);



#endif



