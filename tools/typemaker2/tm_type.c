/***************************************************************************
    begin       : Wed Jul 01 2009
    copyright   : (C) 2009 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "tm_type_p.h"
#include "tm_util.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <assert.h>




GWEN_LIST_FUNCTIONS(TYPEMAKER2_TYPE, Typemaker2_Type);




TYPEMAKER2_TYPE *Typemaker2_Type_new() {
  TYPEMAKER2_TYPE *ty;

  GWEN_NEW_OBJECT(TYPEMAKER2_TYPE, ty);
  ty->refCount=1;
  GWEN_LIST_INIT(TYPEMAKER2_TYPE, ty);

  ty->headers=Typemaker2_Header_List_new();

  ty->structIncludes=GWEN_StringList_new();
  ty->privateIncludes=GWEN_StringList_new();
  ty->libraryIncludes=GWEN_StringList_new();
  ty->publicIncludes=GWEN_StringList_new();
  ty->codeIncludes=GWEN_StringList_new();

  ty->members=Typemaker2_Member_List_new();

  return ty;
}



void Typemaker2_Type_free(TYPEMAKER2_TYPE *ty) {
  if (ty) {
    assert(ty->refCount);
    if (ty->refCount==1) {
      GWEN_LIST_FINI(TYPEMAKER2_TYPE, ty);
      free(ty->name);
      free(ty->extends);
      free(ty->baseType);
      free(ty->inherits);
      free(ty->identifier);
      free(ty->prefix);
      free(ty->code_construct);
      free(ty->code_destruct);
      free(ty->code_assign);
      free(ty->code_dup);
      free(ty->code_fromDb);
      free(ty->code_toDb);
      free(ty->code_fromXml);
      free(ty->code_toXml);
      free(ty->code_toObject);
      free(ty->code_fromObject);

      free(ty->defaultValue);
      free(ty->presetValue);

      Typemaker2_Header_List_free(ty->headers);

      GWEN_StringList_free(ty->structIncludes);
      GWEN_StringList_free(ty->privateIncludes);
      GWEN_StringList_free(ty->libraryIncludes);
      GWEN_StringList_free(ty->publicIncludes);
      GWEN_StringList_free(ty->codeIncludes);

      Typemaker2_Member_List_free(ty->members);

      free(ty->fieldCountId);

      ty->refCount=0;
      GWEN_FREE_OBJECT(ty);
    }
    else
      ty->refCount--;
  }
}



void Typemaker2_Type_Attach(TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);
  ty->refCount++;
}



const char *Typemaker2_Type_GetName(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);
  return ty->name;
}



void Typemaker2_Type_SetName(TYPEMAKER2_TYPE *ty, const char *s) {
  assert(ty);
  assert(ty->refCount);
  free(ty->name);
  if (s && *s) ty->name=strdup(s);
  else ty->name=NULL;
}



int Typemaker2_Type_GetType(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);

  /* nothing here? ask extended type */
  if (ty->type==TypeMaker2_Type_Unknown &&
      ty->extendsPtr)
    return Typemaker2_Type_GetType(ty->extendsPtr);

  return ty->type;
}



void Typemaker2_Type_SetType(TYPEMAKER2_TYPE *ty, int i) {
  assert(ty);
  assert(ty->refCount);
  ty->type=i;
}



const char *Typemaker2_Type_GetExtends(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);
  return ty->extends;
}



void Typemaker2_Type_SetExtends(TYPEMAKER2_TYPE *ty, const char *s) {
  assert(ty);
  assert(ty->refCount);
  free(ty->extends);
  if (s && *s) ty->extends=strdup(s);
  else ty->extends=NULL;
}



const char *Typemaker2_Type_GetBaseType(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);

  if (ty->baseType==NULL && ty->extendsPtr)
    return Typemaker2_Type_GetBaseType(ty->extendsPtr);

  return ty->baseType;
}



void Typemaker2_Type_SetBaseType(TYPEMAKER2_TYPE *ty, const char *s) {
  assert(ty);
  assert(ty->refCount);
  free(ty->baseType);
  if (s && *s) ty->baseType=strdup(s);
  else ty->baseType=NULL;
}



const char *Typemaker2_Type_GetInherits(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);

  if (ty->inherits==NULL && ty->extendsPtr)
    return Typemaker2_Type_GetInherits(ty->extendsPtr);

  return ty->inherits;
}



void Typemaker2_Type_SetInherits(TYPEMAKER2_TYPE *ty, const char *s) {
  assert(ty);
  assert(ty->refCount);
  free(ty->inherits);
  if (s && *s) ty->inherits=strdup(s);
  else ty->inherits=NULL;
}



uint32_t Typemaker2_Type_GetFlags(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);

  if (ty->flags==0 && ty->extendsPtr)
    return Typemaker2_Type_GetFlags(ty->extendsPtr);

  return ty->flags;
}



void Typemaker2_Type_SetFlags(TYPEMAKER2_TYPE *ty, uint32_t i) {
  assert(ty);
  assert(ty->refCount);
  ty->flags=i;
}



uint32_t Typemaker2_Type_GetSetFlags(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);

  if (ty->setFlags==0 && ty->extendsPtr)
    return Typemaker2_Type_GetSetFlags(ty->extendsPtr);

  return ty->setFlags;
}



void Typemaker2_Type_SetSetFlags(TYPEMAKER2_TYPE *ty, uint32_t i) {
  assert(ty);
  assert(ty->refCount);
  ty->setFlags=i;
}



uint32_t Typemaker2_Type_GetGetFlags(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);

  if (ty->getFlags==0 && ty->extendsPtr)
    return Typemaker2_Type_GetGetFlags(ty->extendsPtr);

  return ty->getFlags;
}



void Typemaker2_Type_SetGetFlags(TYPEMAKER2_TYPE *ty, uint32_t i) {
  assert(ty);
  assert(ty->refCount);
  ty->getFlags=i;
}



uint32_t Typemaker2_Type_GetDupFlags(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);

  if (ty->dupFlags==0 && ty->extendsPtr)
    return Typemaker2_Type_GetDupFlags(ty->extendsPtr);

  return ty->dupFlags;
}



void Typemaker2_Type_SetDupFlags(TYPEMAKER2_TYPE *ty, uint32_t i) {
  assert(ty);
  assert(ty->refCount);
  ty->dupFlags=i;
}



int Typemaker2_Type_GetAccess(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);

  if (ty->access==TypeMaker2_Access_Unknown && ty->extendsPtr)
    return Typemaker2_Type_GetAccess(ty->extendsPtr);

  return ty->access;
}



void Typemaker2_Type_SetAccess(TYPEMAKER2_TYPE *ty, int i) {
  assert(ty);
  assert(ty->refCount);
  ty->access=i;
}



const char *Typemaker2_Type_GetDefaultValue(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);

  if (ty->defaultValue==NULL && ty->extendsPtr)
    return Typemaker2_Type_GetDefaultValue(ty->extendsPtr);

  return ty->defaultValue;
}



void Typemaker2_Type_SetDefaultValue(TYPEMAKER2_TYPE *ty, const char *s) {
  assert(ty);
  assert(ty->refCount);
  free(ty->defaultValue);
  if (s && *s) ty->defaultValue=strdup(s);
  else ty->defaultValue=NULL;
}



const char *Typemaker2_Type_GetPresetValue(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);

  if (ty->presetValue==NULL && ty->extendsPtr)
    return Typemaker2_Type_GetPresetValue(ty->extendsPtr);

  return ty->presetValue;
}



void Typemaker2_Type_SetPresetValue(TYPEMAKER2_TYPE *ty, const char *s) {
  assert(ty);
  assert(ty->refCount);
  free(ty->presetValue);
  if (s && *s) ty->presetValue=strdup(s);
  else ty->presetValue=NULL;
}



const char *Typemaker2_Type_GetIdentifier(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);
  return ty->identifier;
}



void Typemaker2_Type_SetIdentifier(TYPEMAKER2_TYPE *ty, const char *s) {
  assert(ty);
  assert(ty->refCount);
  free(ty->identifier);
  if (s && *s) ty->identifier=strdup(s);
  else ty->identifier=NULL;
}



const char *Typemaker2_Type_GetPrefix(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);
  return ty->prefix;
}



void Typemaker2_Type_SetPrefix(TYPEMAKER2_TYPE *ty, const char *s) {
  assert(ty);
  assert(ty->refCount);
  free(ty->prefix);
  if (s && *s) ty->prefix=strdup(s);
  else ty->prefix=NULL;
}



const char *Typemaker2_Type_GetCodeConstruct(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);

  if (ty->code_construct==NULL && ty->extendsPtr)
    return Typemaker2_Type_GetCodeConstruct(ty->extendsPtr);

  return ty->code_construct;
}



void Typemaker2_Type_SetCodeConstruct(TYPEMAKER2_TYPE *ty, const char *s) {
  assert(ty);
  assert(ty->refCount);
  free(ty->code_construct);
  if (s && *s) ty->code_construct=strdup(s);
  else ty->code_construct=NULL;
}



const char *Typemaker2_Type_GetCodeDestruct(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);

  if (ty->code_destruct==NULL && ty->extendsPtr)
    return Typemaker2_Type_GetCodeDestruct(ty->extendsPtr);

  return ty->code_destruct;
}



void Typemaker2_Type_SetCodeDestruct(TYPEMAKER2_TYPE *ty, const char *s) {
  assert(ty);
  assert(ty->refCount);
  free(ty->code_destruct);
  if (s && *s) ty->code_destruct=strdup(s);
  else ty->code_destruct=NULL;
}



const char *Typemaker2_Type_GetCodeAssign(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);

  if (ty->code_assign==NULL && ty->extendsPtr)
    return Typemaker2_Type_GetCodeAssign(ty->extendsPtr);

  return ty->code_assign;
}



void Typemaker2_Type_SetCodeAssign(TYPEMAKER2_TYPE *ty, const char *s) {
  assert(ty);
  assert(ty->refCount);
  free(ty->code_assign);
  if (s && *s) ty->code_assign=strdup(s);
  else ty->code_assign=NULL;
}



const char *Typemaker2_Type_GetCodeDup(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);

  if (ty->code_dup==NULL && ty->extendsPtr)
    return Typemaker2_Type_GetCodeDup(ty->extendsPtr);

  return ty->code_dup;
}



void Typemaker2_Type_SetCodeDup(TYPEMAKER2_TYPE *ty, const char *s) {
  assert(ty);
  assert(ty->refCount);
  free(ty->code_dup);
  if (s && *s) ty->code_dup=strdup(s);
  else ty->code_dup=NULL;
}



const char *Typemaker2_Type_GetCodeFromDb(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);

  if (ty->code_fromDb==NULL && ty->extendsPtr)
    return Typemaker2_Type_GetCodeFromDb(ty->extendsPtr);

  return ty->code_fromDb;
}



void Typemaker2_Type_SetCodeFromDb(TYPEMAKER2_TYPE *ty, const char *s) {
  assert(ty);
  assert(ty->refCount);
  free(ty->code_fromDb);
  if (s && *s) ty->code_fromDb=strdup(s);
  else ty->code_fromDb=NULL;
}



const char *Typemaker2_Type_GetCodeToDb(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);

  if (ty->code_toDb==NULL && ty->extendsPtr)
    return Typemaker2_Type_GetCodeToDb(ty->extendsPtr);

  return ty->code_toDb;
}



void Typemaker2_Type_SetCodeToDb(TYPEMAKER2_TYPE *ty, const char *s) {
  assert(ty);
  assert(ty->refCount);
  free(ty->code_toDb);
  if (s && *s) ty->code_toDb=strdup(s);
  else ty->code_toDb=NULL;
}



const char *Typemaker2_Type_GetCodeFromXml(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);

  if (ty->code_fromXml==NULL && ty->extendsPtr)
    return Typemaker2_Type_GetCodeFromXml(ty->extendsPtr);

  return ty->code_fromXml;
}



void Typemaker2_Type_SetCodeFromXml(TYPEMAKER2_TYPE *ty, const char *s) {
  assert(ty);
  assert(ty->refCount);
  free(ty->code_fromXml);
  if (s && *s) ty->code_fromXml=strdup(s);
  else ty->code_fromXml=NULL;
}



const char *Typemaker2_Type_GetCodeToXml(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);

  if (ty->code_toXml==NULL && ty->extendsPtr)
    return Typemaker2_Type_GetCodeToXml(ty->extendsPtr);

  return ty->code_toXml;
}



void Typemaker2_Type_SetCodeToXml(TYPEMAKER2_TYPE *ty, const char *s) {
  assert(ty);
  assert(ty->refCount);
  free(ty->code_toXml);
  if (s && *s) ty->code_toXml=strdup(s);
  else ty->code_toXml=NULL;
}



const char *Typemaker2_Type_GetCodeFromObject(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);

  if (ty->code_fromObject==NULL && ty->extendsPtr)
    return Typemaker2_Type_GetCodeFromObject(ty->extendsPtr);

  return ty->code_fromObject;
}



void Typemaker2_Type_SetCodeFromObject(TYPEMAKER2_TYPE *ty, const char *s) {
  assert(ty);
  assert(ty->refCount);
  free(ty->code_fromObject);
  if (s && *s) ty->code_fromObject=strdup(s);
  else ty->code_fromObject=NULL;
}



const char *Typemaker2_Type_GetCodeToObject(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);

  if (ty->code_toObject==NULL && ty->extendsPtr)
    return Typemaker2_Type_GetCodeToObject(ty->extendsPtr);

  return ty->code_toObject;
}



void Typemaker2_Type_SetCodeToObject(TYPEMAKER2_TYPE *ty, const char *s) {
  assert(ty);
  assert(ty->refCount);
  free(ty->code_toObject);
  if (s && *s) ty->code_toObject=strdup(s);
  else ty->code_toObject=NULL;
}



TYPEMAKER2_TYPE *Typemaker2_Type_GetExtendsPtr(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);
  return ty->extendsPtr;
}



void Typemaker2_Type_SetExtendsPtr(TYPEMAKER2_TYPE *ty, TYPEMAKER2_TYPE *p) {
  assert(ty);
  assert(ty->refCount);
  ty->extendsPtr=p;
}



TYPEMAKER2_TYPE *Typemaker2_Type_GetInheritsPtr(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);
  return ty->inheritsPtr;
}



void Typemaker2_Type_SetInheritsPtr(TYPEMAKER2_TYPE *ty, TYPEMAKER2_TYPE *p) {
  assert(ty);
  assert(ty->refCount);
  ty->inheritsPtr=p;
}



TYPEMAKER2_TYPE *Typemaker2_Type_GetBaseTypePtr(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);
  return ty->baseTypePtr;
}



void Typemaker2_Type_SetBaseTypePtr(TYPEMAKER2_TYPE *ty, TYPEMAKER2_TYPE *p) {
  assert(ty);
  assert(ty->refCount);
  ty->baseTypePtr=p;
}



TYPEMAKER2_MEMBER_LIST *Typemaker2_Type_GetMembers(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);
  return ty->members;
}



int Typemaker2_Type_GetNonVolatileMemberCount(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);
  return ty->nonVolatileMemberCount;
}



void Typemaker2_Type_SetNonVolatileMemberCount(TYPEMAKER2_TYPE *ty, int i) {
  assert(ty);
  assert(ty->refCount);
  ty->nonVolatileMemberCount=i;
}



const char *Typemaker2_Type_GetFieldCountId(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);
  return ty->fieldCountId;
}



void Typemaker2_Type_SetFieldCountId(TYPEMAKER2_TYPE *ty, const char *s) {
  assert(ty);
  assert(ty->refCount);
  free(ty->fieldCountId);
  if (s) ty->fieldCountId=strdup(s);
  else ty->fieldCountId=NULL;
}



int Typemaker2_Type_readXml(TYPEMAKER2_TYPE *ty, GWEN_XMLNODE *node, const char *wantedLang) {
  GWEN_XMLNODE *langNode=NULL;
  GWEN_XMLNODE *n;
  const char *s;

  /* parse type */
  s=GWEN_XMLNode_GetProperty(node, "type", "opaque");
  if (strcasecmp(s, "opaque")==0)
    Typemaker2_Type_SetType(ty, TypeMaker2_Type_Opaque);
  else if (strcasecmp(s, "pointer")==0)
    Typemaker2_Type_SetType(ty, TypeMaker2_Type_Pointer);
  else if (strcasecmp(s, "array")==0)
    Typemaker2_Type_SetType(ty, TypeMaker2_Type_Array);
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Unknown type [%s]", s);
    return GWEN_ERROR_BAD_DATA;
  }

  s=GWEN_XMLNode_GetProperty(node, "id", NULL);
  Typemaker2_Type_SetName(ty, s);

  s=GWEN_XMLNode_GetProperty(node, "extends", NULL);
  Typemaker2_Type_SetExtends(ty, s);

  s=GWEN_XMLNode_GetProperty(node, "basetype", NULL);
  Typemaker2_Type_SetBaseType(ty, s);

  if (wantedLang)
    langNode=GWEN_XMLNode_FindFirstTag(node, "lang", "id", wantedLang);
  if (langNode==NULL)
    langNode=node;

  /* read identifier and prefix */
  s=GWEN_XMLNode_GetCharValue(langNode, "identifier", NULL);
  Typemaker2_Type_SetIdentifier(ty, s);

  s=GWEN_XMLNode_GetCharValue(langNode, "prefix", NULL);
  Typemaker2_Type_SetPrefix(ty, s);

  /* read headers */
  n=GWEN_XMLNode_FindFirstTag(langNode, "headers", NULL, NULL);
  if (n) {
    GWEN_XMLNODE *nn;

    nn=GWEN_XMLNode_FindFirstTag(n, "header", NULL, NULL);
    while(nn) {
      TYPEMAKER2_HEADER *th;
      int rv;

      th=Typemaker2_Header_new();
      rv=Typemaker2_Header_readXml(th, nn);
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        Typemaker2_Header_free(th);
        return rv;
      }
      Typemaker2_Header_List_Add(th, ty->headers);
      nn=GWEN_XMLNode_FindNextTag(nn, "header", NULL, NULL);
    }
  }

  /* read members */
  n=GWEN_XMLNode_FindFirstTag(node, "members", NULL, NULL);
  if (n) {
    GWEN_XMLNODE *nn;

    nn=GWEN_XMLNode_FindFirstTag(n, "member", NULL, NULL);
    while(nn) {
      TYPEMAKER2_MEMBER *tm;
      int rv;

      tm=Typemaker2_Member_new();
      rv=Typemaker2_Member_readXml(tm, nn);
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	Typemaker2_Member_free(tm);
        return rv;
      }
      /* TODO: only count members which are not volatile
       Typemaker2_Member_SetMemberPosition(tm, memberPos++);
       */
      Typemaker2_Member_List_Add(tm, ty->members);
      nn=GWEN_XMLNode_FindNextTag(nn, "member", NULL, NULL);
    }
  }

  /* read code */
  n=GWEN_XMLNode_FindFirstTag(langNode, "code", NULL, NULL);
  if (n) {
    s=GWEN_XMLNode_GetCharValue(n, "construct", NULL);
    Typemaker2_Type_SetCodeConstruct(ty, s);

    s=GWEN_XMLNode_GetCharValue(n, "destruct", NULL);
    Typemaker2_Type_SetCodeDestruct(ty, s);

    s=GWEN_XMLNode_GetCharValue(n, "assign", NULL);
    Typemaker2_Type_SetCodeAssign(ty, s);

    s=GWEN_XMLNode_GetCharValue(n, "dup", NULL);
    Typemaker2_Type_SetCodeDup(ty, s);

    s=GWEN_XMLNode_GetCharValue(n, "todb", NULL);
    Typemaker2_Type_SetCodeToDb(ty, s);

    s=GWEN_XMLNode_GetCharValue(n, "fromdb", NULL);
    Typemaker2_Type_SetCodeFromDb(ty, s);

    s=GWEN_XMLNode_GetCharValue(n, "toxml", NULL);
    Typemaker2_Type_SetCodeToXml(ty, s);

    s=GWEN_XMLNode_GetCharValue(n, "fromxml", NULL);
    Typemaker2_Type_SetCodeFromXml(ty, s);

    s=GWEN_XMLNode_GetCharValue(n, "toobject", NULL);
    Typemaker2_Type_SetCodeToObject(ty, s);

    s=GWEN_XMLNode_GetCharValue(n, "fromobject", NULL);
    Typemaker2_Type_SetCodeFromObject(ty, s);
  }

  /* read defaults */
  n=GWEN_XMLNode_FindFirstTag(langNode, "defaults", NULL, NULL);
  if (n) {
    s=GWEN_XMLNode_GetCharValue(n, "default", NULL);
    Typemaker2_Type_SetDefaultValue(ty, s);

    s=GWEN_XMLNode_GetCharValue(n, "preset", NULL);
    Typemaker2_Type_SetPresetValue(ty, s);

    /* read flags */
    s=GWEN_XMLNode_GetCharValue(n, "flags", NULL);
    if (s && *s)
      Typemaker2_Type_SetFlags(ty, Typemaker2_FlagsFromString(s));
  
    /* read setflags */
    s=GWEN_XMLNode_GetCharValue(n, "setflags", NULL);
    if (s && *s)
      Typemaker2_Type_SetSetFlags(ty, Typemaker2_FlagsFromString(s));
  
    /* read getflags */
    s=GWEN_XMLNode_GetCharValue(n, "getflags", NULL);
    if (s && *s)
      Typemaker2_Type_SetGetFlags(ty, Typemaker2_FlagsFromString(s));
  
    /* read dupflags */
    s=GWEN_XMLNode_GetCharValue(n, "dupflags", NULL);
    if (s && *s)
      Typemaker2_Type_SetDupFlags(ty, Typemaker2_FlagsFromString(s));
  
    /* read access */
    s=GWEN_XMLNode_GetCharValue(n, "access", NULL);
    if (s && *s) {
      int i=Typemaker2_AccessFromString(s);
      if (i==TypeMaker2_Access_Unknown) {
	DBG_ERROR(GWEN_LOGDOMAIN, "Unknown access type [%s]", s);
	return GWEN_ERROR_BAD_DATA;
      }
      Typemaker2_Type_SetAccess(ty, i);
    }
  }

  return 0;
}



void Typemaker2_Type_Dump(TYPEMAKER2_TYPE *ty, FILE *f, int indent) {
  if (ty) {
    int i;
    TYPEMAKER2_MEMBER *tm;
    const char *s1, *s2;

    for (i=0; i<indent; i++) fprintf(f, " ");
    fprintf(f, "Type\n");

    for (i=0; i<indent+2; i++) fprintf(f, " ");
    fprintf(f, "Name      : %s\n", (ty->name)?(ty->name):"<null>");

    for (i=0; i<indent+2; i++) fprintf(f, " ");
    fprintf(f, "Identifier: %s\n", (ty->identifier)?(ty->identifier):"<null>");

    for (i=0; i<indent+2; i++) fprintf(f, " ");
    fprintf(f, "Extends   : %s\n", (ty->extends)?(ty->extends):"<null>");

    for (i=0; i<indent+2; i++) fprintf(f, " ");
    s1=ty->inherits;
    s2=Typemaker2_Type_GetInherits(ty);
    fprintf(f, "Inherits  : %s [%s]\n", s1?s1:"<null>", s2?s2:"<null>");

    for (i=0; i<indent+2; i++) fprintf(f, " ");
    fprintf(f, "BaseType  : %s\n", (ty->baseType)?(ty->baseType):"<null>");

    for (i=0; i<indent+2; i++) fprintf(f, " ");
    fprintf(f, "Prefix    : %s\n", (ty->prefix)?(ty->prefix):"<null>");

    for (i=0; i<indent+2; i++) fprintf(f, " ");
    fprintf(f, "Access    : %d [%d]\n", ty->access, Typemaker2_Type_GetAccess(ty));

    for (i=0; i<indent+2; i++) fprintf(f, " ");
    fprintf(f, "Flags     : %08x [%08x]\n", ty->flags, Typemaker2_Type_GetFlags(ty));

    for (i=0; i<indent+2; i++) fprintf(f, " ");
    fprintf(f, "SetFlags  : %08x [%08x]\n", ty->setFlags, Typemaker2_Type_GetSetFlags(ty));

    for (i=0; i<indent+2; i++) fprintf(f, " ");
    fprintf(f, "GetFlags  : %08x [%08x]\n", ty->getFlags, Typemaker2_Type_GetGetFlags(ty));

    for (i=0; i<indent+2; i++) fprintf(f, " ");
    fprintf(f, "DupFlags  : %08x [%08x]\n", ty->dupFlags, Typemaker2_Type_GetDupFlags(ty));

    for (i=0; i<indent+2; i++) fprintf(f, " ");
    s1=ty->defaultValue;
    s2=Typemaker2_Type_GetDefaultValue(ty);
    fprintf(f, "Default   : %s [%s]\n", s1?s1:"<null>", s2?s2:"<null>");

    for (i=0; i<indent+2; i++) fprintf(f, " ");
    s1=ty->presetValue;
    s2=Typemaker2_Type_GetPresetValue(ty);
    fprintf(f, "Preset    : %s [%s]\n", s1?s1:"<null>", s2?s2:"<null>");

    for (i=0; i<indent+2; i++) fprintf(f, " ");
    fprintf(f, "Members\n");
    tm=Typemaker2_Member_List_First(ty->members);
    if (tm==NULL) {
      for (i=0; i<indent+4; i++) fprintf(f, " ");
      fprintf(f, "none\n");
    }
    else {
      while(tm) {
	Typemaker2_Member_Dump(tm, f, indent+4);
	tm=Typemaker2_Member_List_Next(tm);
      }
    }
    for (i=0; i<indent+2; i++) fprintf(f, " ");
    fprintf(f, "Field Count Id: %s\n", (ty->fieldCountId)?(ty->fieldCountId):"<null>");
  }
}








