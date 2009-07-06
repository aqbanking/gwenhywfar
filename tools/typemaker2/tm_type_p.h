/***************************************************************************
    begin       : Wed Jul 01 2009
    copyright   : (C) 2009 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef TYPEMAKER2_TYPE_P_H
#define TYPEMAKER2_TYPE_P_H

#include "tm_type.h"


struct TYPEMAKER2_TYPE {
  GWEN_LIST_ELEMENT(TYPEMAKER2_TYPE)

  char *name;

  uint32_t flags;
  uint32_t setFlags;
  uint32_t getFlags;
  uint32_t dupFlags;

  int access;

  int type;

  char *extends;
  char *baseType;
  char *inherits;

  char *identifier;
  char *prefix;

  char *code_construct;
  char *code_destruct;
  char *code_assign;
  char *code_dup;
  char *code_compare;
  char *code_fromDb;
  char *code_toDb;
  char *code_fromXml;
  char *code_toXml;
  char *code_toObject;
  char *code_fromObject;

  char *defaultValue;
  char *presetValue;

  char *aedb_type;
  char *baseFileName;;

  TYPEMAKER2_HEADER_LIST *headers;
  TYPEMAKER2_ENUM_LIST *enums;
  TYPEMAKER2_DEFINE_LIST *defines;

  GWEN_STRINGLIST *structIncludes;
  GWEN_STRINGLIST *privateIncludes;
  GWEN_STRINGLIST *libraryIncludes;
  GWEN_STRINGLIST *publicIncludes;
  GWEN_STRINGLIST *codeIncludes;

  TYPEMAKER2_MEMBER_LIST *members;

  TYPEMAKER2_TYPE *extendsPtr;
  TYPEMAKER2_TYPE *baseTypePtr;
  TYPEMAKER2_TYPE *inheritsPtr;

  int nonVolatileMemberCount;

  char *fieldCountId;

  int refCount;
};



#endif



