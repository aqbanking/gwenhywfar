/***************************************************************************
    begin       : Thu Jul 02 2009
    copyright   : (C) 2009 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



#ifndef TYPEMAKER2_MEMBER_P_H
#define TYPEMAKER2_MEMBER_P_H

#include "tm_member.h"


struct TYPEMAKER2_MEMBER {
  GWEN_LIST_ELEMENT(TYPEMAKER2_MEMBER)

  char *name;
  char *typeName;

  int access;

  uint32_t flags;
  uint32_t setFlags;
  uint32_t getFlags;
  uint32_t dupFlags;

  int maxLen;

  char *defaultValue;
  char *presetValue;

  char *aedb_type;
  char *enumId;
  char *defineId;

  TYPEMAKER2_TYPE *typePtr;

  TYPEMAKER2_ENUM *enumPtr;
  TYPEMAKER2_DEFINE *definePtr;

  int refCount;

  int memberPosition;
  char *fieldId;

};



#endif



