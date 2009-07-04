/***************************************************************************
    begin       : Thu Jul 02 2009
    copyright   : (C) 2009 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "tm_member_p.h"
#include "tm_util.h"


#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <assert.h>



GWEN_LIST_FUNCTIONS(TYPEMAKER2_MEMBER, Typemaker2_Member)




TYPEMAKER2_MEMBER *Typemaker2_Member_new() {
  TYPEMAKER2_MEMBER *tm;

  GWEN_NEW_OBJECT(TYPEMAKER2_MEMBER, tm);
  tm->refCount=1;
  GWEN_LIST_INIT(TYPEMAKER2_MEMBER, tm);

  return tm;
}



void Typemaker2_Member_free(TYPEMAKER2_MEMBER *tm) {
  if (tm) {
    assert(tm->refCount);
    if (tm->refCount==1) {
      GWEN_LIST_FINI(TYPEMAKER2_MEMBER, tm);
      free(tm->name);
      free(tm->typeName);
      free(tm->fieldId);
      free(tm->defaultValue);
      free(tm->presetValue);
      free(tm->aedb_type);
      free(tm->enumId);
      tm->refCount=0;
      GWEN_FREE_OBJECT(tm);
    }
    else {
      tm->refCount--;
    }
  }
}



void Typemaker2_Member_Attach(TYPEMAKER2_MEMBER *tm) {
  assert(tm);
  assert(tm->refCount);
  tm->refCount++;
}



const char *Typemaker2_Member_GetName(const TYPEMAKER2_MEMBER *tm) {
  assert(tm);
  assert(tm->refCount);
  return tm->name;
}



void Typemaker2_Member_SetName(TYPEMAKER2_MEMBER *tm, const char *s) {
  assert(tm);
  assert(tm->refCount);
  free(tm->name);
  if (s && *s) tm->name=strdup(s);
  else tm->name=NULL;
}



const char *Typemaker2_Member_GetTypeName(const TYPEMAKER2_MEMBER *tm) {
  assert(tm);
  assert(tm->refCount);
  return tm->typeName;
}



void Typemaker2_Member_SetTypeName(TYPEMAKER2_MEMBER *tm, const char *s) {
  assert(tm);
  assert(tm->refCount);
  free(tm->typeName);
  if (s && *s) tm->typeName=strdup(s);
  else tm->typeName=NULL;
}



const char *Typemaker2_Member_GetFieldId(const TYPEMAKER2_MEMBER *tm) {
  assert(tm);
  assert(tm->refCount);
  return tm->fieldId;
}



void Typemaker2_Member_SetFieldId(TYPEMAKER2_MEMBER *tm, const char *s) {
  assert(tm);
  assert(tm->refCount);
  free(tm->fieldId);
  if (s && *s) tm->fieldId=strdup(s);
  else tm->fieldId=NULL;
}



const char *Typemaker2_Member_GetEnumId(const TYPEMAKER2_MEMBER *tm) {
  assert(tm);
  assert(tm->refCount);
  return tm->enumId;
}



void Typemaker2_Member_SetEnumId(TYPEMAKER2_MEMBER *tm, const char *s) {
  assert(tm);
  assert(tm->refCount);
  free(tm->enumId);
  if (s && *s) tm->enumId=strdup(s);
  else tm->enumId=NULL;
}



uint32_t Typemaker2_Member_GetFlags(const TYPEMAKER2_MEMBER *tm) {
  assert(tm);
  assert(tm->refCount);

  if (tm->flags==0 && tm->typePtr)
    return Typemaker2_Type_GetFlags(tm->typePtr);

  return tm->flags;
}



void Typemaker2_Member_SetFlags(TYPEMAKER2_MEMBER *tm, uint32_t i) {
  assert(tm);
  assert(tm->refCount);
  tm->flags=i;
}



uint32_t Typemaker2_Member_GetSetFlags(const TYPEMAKER2_MEMBER *tm) {
  assert(tm);
  assert(tm->refCount);

  if (tm->setFlags==0 && tm->typePtr)
    return Typemaker2_Type_GetSetFlags(tm->typePtr);

  return tm->setFlags;
}



void Typemaker2_Member_SetSetFlags(TYPEMAKER2_MEMBER *tm, uint32_t i) {
  assert(tm);
  assert(tm->refCount);
  tm->setFlags=i;
}



uint32_t Typemaker2_Member_GetGetFlags(const TYPEMAKER2_MEMBER *tm) {
  assert(tm);
  assert(tm->refCount);

  if (tm->getFlags==0 && tm->typePtr)
    return Typemaker2_Type_GetGetFlags(tm->typePtr);

  return tm->getFlags;
}



void Typemaker2_Member_SetGetFlags(TYPEMAKER2_MEMBER *tm, uint32_t i) {
  assert(tm);
  assert(tm->refCount);
  tm->getFlags=i;
}



uint32_t Typemaker2_Member_GetDupFlags(const TYPEMAKER2_MEMBER *tm) {
  assert(tm);
  assert(tm->refCount);

  if (tm->dupFlags==0 && tm->typePtr)
    return Typemaker2_Type_GetDupFlags(tm->typePtr);

  return tm->dupFlags;
}



void Typemaker2_Member_SetDupFlags(TYPEMAKER2_MEMBER *tm, uint32_t i) {
  assert(tm);
  assert(tm->refCount);
  tm->dupFlags=i;
}



int Typemaker2_Member_GetAccess(const TYPEMAKER2_MEMBER *tm) {
  assert(tm);
  assert(tm->refCount);

  if (tm->access==TypeMaker2_Access_Unknown && tm->typePtr)
    return Typemaker2_Type_GetAccess(tm->typePtr);

  return tm->access;
}



void Typemaker2_Member_SetAccess(TYPEMAKER2_MEMBER *tm, int i) {
  assert(tm);
  assert(tm->refCount);
  tm->access=i;
}



int Typemaker2_Member_GetMaxLen(const TYPEMAKER2_MEMBER *tm) {
  assert(tm);
  assert(tm->refCount);

  return tm->maxLen;
}



void Typemaker2_Member_SetMaxLen(TYPEMAKER2_MEMBER *tm, int i) {
  assert(tm);
  assert(tm->refCount);
  tm->maxLen=i;
}



const char *Typemaker2_Member_GetDefaultValue(const TYPEMAKER2_MEMBER *tm) {
  assert(tm);
  assert(tm->refCount);

  if (tm->defaultValue==NULL && tm->typePtr)
    return Typemaker2_Type_GetDefaultValue(tm->typePtr);
  return tm->defaultValue;
}



void Typemaker2_Member_SetDefaultValue(TYPEMAKER2_MEMBER *tm, const char *s) {
  assert(tm);
  assert(tm->refCount);

  free(tm->defaultValue);
  if (s) tm->defaultValue=strdup(s);
  else tm->defaultValue=NULL;
}



const char *Typemaker2_Member_GetPresetValue(const TYPEMAKER2_MEMBER *tm) {
  assert(tm);
  assert(tm->refCount);

  if (tm->presetValue==NULL && tm->typePtr)
    return Typemaker2_Type_GetPresetValue(tm->typePtr);
  return tm->presetValue;
}



void Typemaker2_Member_SetPresetValue(TYPEMAKER2_MEMBER *tm, const char *s) {
  assert(tm);
  assert(tm->refCount);

  free(tm->presetValue);
  if (s) tm->presetValue=strdup(s);
  else tm->presetValue=NULL;
}



const char *Typemaker2_Member_GetAeDbType(const TYPEMAKER2_MEMBER *tm) {
  assert(tm);
  assert(tm->refCount);

  if (tm->aedb_type==NULL && tm->typePtr)
    return Typemaker2_Type_GetAeDbType(tm->typePtr);

  return tm->aedb_type;
}



void Typemaker2_Member_SetAeDbType(TYPEMAKER2_MEMBER *tm, const char *s) {
  assert(tm);
  assert(tm->refCount);

  free(tm->aedb_type);
  if (s) tm->aedb_type=strdup(s);
  else tm->aedb_type=NULL;
}



TYPEMAKER2_TYPE *Typemaker2_Member_GetTypePtr(const TYPEMAKER2_MEMBER *tm) {
  assert(tm);
  assert(tm->refCount);
  return tm->typePtr;
}



void Typemaker2_Member_SetTypePtr(TYPEMAKER2_MEMBER *tm, TYPEMAKER2_TYPE *ty) {
  assert(tm);
  assert(tm->refCount);
  tm->typePtr=ty;
}



TYPEMAKER2_ENUM *Typemaker2_Member_GetEnumPtr(const TYPEMAKER2_MEMBER *tm) {
  assert(tm);
  assert(tm->refCount);
  return tm->enumPtr;
}



void Typemaker2_Member_SetEnumPtr(TYPEMAKER2_MEMBER *tm, TYPEMAKER2_ENUM *te) {
  assert(tm);
  assert(tm->refCount);
  tm->enumPtr=te;
}



int Typemaker2_Member_GetMemberPosition(const TYPEMAKER2_MEMBER *tm) {
  assert(tm);
  assert(tm->refCount);
  return tm->memberPosition;
}



void Typemaker2_Member_SetMemberPosition(TYPEMAKER2_MEMBER *tm, int i) {
  assert(tm);
  assert(tm->refCount);
  tm->memberPosition=i;
}



int Typemaker2_Member_readXml(TYPEMAKER2_MEMBER *tm, GWEN_XMLNODE *node) {
  const char *s;

  assert(tm);
  assert(tm->refCount);

  s=GWEN_XMLNode_GetProperty(node, "name", NULL);
  if (s==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Member has no id");
    return GWEN_ERROR_BAD_DATA;
  }
  Typemaker2_Member_SetName(tm, s);

  s=GWEN_XMLNode_GetProperty(node, "type", NULL);
  if (s==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Member has no type");
    return GWEN_ERROR_BAD_DATA;
  }
  Typemaker2_Member_SetTypeName(tm, s);

  s=GWEN_XMLNode_GetProperty(node, "enum", NULL);
  Typemaker2_Member_SetEnumId(tm, s);

  s=GWEN_XMLNode_GetProperty(node, "maxlen", NULL);
  if (s && *s) {
    int i;

    if (1==sscanf(s, "%i", &i))
      tm->maxLen=i;
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "Bad value for maxlen [%s]", s);
      return GWEN_ERROR_BAD_DATA;
    }
  }

  /* read flags */
  s=GWEN_XMLNode_GetCharValue(node, "flags", NULL);
  if (s && *s)
    Typemaker2_Member_SetFlags(tm, Typemaker2_FlagsFromString(s));

  /* read setflags */
  s=GWEN_XMLNode_GetCharValue(node, "setflags", NULL);
  if (s && *s)
    Typemaker2_Member_SetSetFlags(tm, Typemaker2_FlagsFromString(s));

  /* read getflags */
  s=GWEN_XMLNode_GetCharValue(node, "getflags", NULL);
  if (s && *s)
    Typemaker2_Member_SetGetFlags(tm, Typemaker2_FlagsFromString(s));

  /* read dupflags */
  s=GWEN_XMLNode_GetCharValue(node, "dupflags", NULL);
  if (s && *s)
    Typemaker2_Member_SetDupFlags(tm, Typemaker2_FlagsFromString(s));

  /* read access */
  s=GWEN_XMLNode_GetCharValue(node, "access", NULL);
  if (s && *s) {
    int i=Typemaker2_AccessFromString(s);
    if (i==TypeMaker2_Access_Unknown) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unknown access type [%s]", s);
      return GWEN_ERROR_BAD_DATA;
    }
    Typemaker2_Member_SetAccess(tm, i);
  }

  /* read default value */
  s=GWEN_XMLNode_GetCharValue(node, "default", NULL);
  if (s && *s)
    Typemaker2_Member_SetDefaultValue(tm, s);

  /* read preset value */
  s=GWEN_XMLNode_GetCharValue(node, "preset", NULL);
  if (s && *s)
    Typemaker2_Member_SetPresetValue(tm, s);

  /* read AEDB type */
  s=GWEN_XMLNode_GetCharValue(node, "aedb_type", NULL);
  if (s && *s)
    Typemaker2_Member_SetAeDbType(tm, s);

  return 0;
}



void Typemaker2_Member_Dump(TYPEMAKER2_MEMBER *tm, FILE *f, int indent) {
  if (tm) {
    int i;
    const char *s1, *s2;

    for (i=0; i<indent; i++) fprintf(f, " ");
    fprintf(f, "Member\n");

    for (i=0; i<indent+2; i++) fprintf(f, " ");
    fprintf(f, "Name    : %s\n", (tm->name)?(tm->name):"<null>");

    for (i=0; i<indent+2; i++) fprintf(f, " ");
    fprintf(f, "Typename: %s\n", (tm->typeName)?(tm->typeName):"<null>");

    for (i=0; i<indent+2; i++) fprintf(f, " ");
    fprintf(f, "Field Id: %s\n", (tm->fieldId)?(tm->fieldId):"<null>");

    for (i=0; i<indent+2; i++) fprintf(f, " ");
    fprintf(f, "Access  : %d  [%d]\n", tm->access, Typemaker2_Member_GetAccess(tm));

    for (i=0; i<indent+2; i++) fprintf(f, " ");
    fprintf(f, "Flags   : %08x [%08x]\n", tm->flags, Typemaker2_Member_GetFlags(tm));

    for (i=0; i<indent+2; i++) fprintf(f, " ");
    fprintf(f, "SetFlags: %08x [%08x]\n", tm->setFlags, Typemaker2_Member_GetSetFlags(tm));

    for (i=0; i<indent+2; i++) fprintf(f, " ");
    fprintf(f, "GetFlags: %08x [%08x]\n", tm->getFlags, Typemaker2_Member_GetGetFlags(tm));

    for (i=0; i<indent+2; i++) fprintf(f, " ");
    fprintf(f, "DupFlags: %08x [%08x]\n", tm->dupFlags, Typemaker2_Member_GetDupFlags(tm));

    for (i=0; i<indent+2; i++) fprintf(f, " ");
    s1=tm->defaultValue;
    s2=Typemaker2_Member_GetDefaultValue(tm);
    fprintf(f, "Default : %s [%s]\n", s1?s1:"<null>", s2?s2:"<null>");

    for (i=0; i<indent+2; i++) fprintf(f, " ");
    s1=tm->presetValue;
    s2=Typemaker2_Member_GetPresetValue(tm);
    fprintf(f, "Preset  : %s [%s]\n", s1?s1:"<null>", s2?s2:"<null>");

    for (i=0; i<indent+2; i++) fprintf(f, " ");
    fprintf(f, "Position: %d\n", tm->memberPosition);
  }
}







