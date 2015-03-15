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
GWEN_LIST2_FUNCTIONS(TYPEMAKER2_TYPE, Typemaker2_Type);




TYPEMAKER2_TYPE *Typemaker2_Type_new() {
  TYPEMAKER2_TYPE *ty;

  GWEN_NEW_OBJECT(TYPEMAKER2_TYPE, ty);
  ty->refCount=1;
  GWEN_LIST_INIT(TYPEMAKER2_TYPE, ty);

  ty->headers=Typemaker2_Header_List_new();
  ty->enums=Typemaker2_Enum_List_new();
  ty->defines=Typemaker2_Define_List_new();
  ty->codeDefs=Typemaker2_Code_List_new();
  ty->inlines=Typemaker2_Inline_List_new();
  ty->virtualFns=Typemaker2_VirtualFn_List_new();
  ty->signals=Typemaker2_Signal_List_new();
  ty->slots=Typemaker2_Slot_List_new();

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

      free(ty->defaultValue);
      free(ty->presetValue);

      free(ty->aqdb_type);
      free(ty->baseFileName);

      free(ty->freeHook);

      free(ty->descr);

      Typemaker2_Header_List_free(ty->headers);
      Typemaker2_Enum_List_free(ty->enums);
      Typemaker2_Define_List_free(ty->defines);
      Typemaker2_Code_List_free(ty->codeDefs);
      Typemaker2_Inline_List_free(ty->inlines);
      Typemaker2_VirtualFn_List_free(ty->virtualFns);
      Typemaker2_Signal_List_free(ty->signals);
      Typemaker2_Slot_List_free(ty->slots);

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



const char *Typemaker2_Type_GetDescription(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);
  return ty->descr;
}



void Typemaker2_Type_SetDescription(TYPEMAKER2_TYPE *ty, const char *s) {
  assert(ty);
  assert(ty->refCount);
  free(ty->descr);
  if (s && *s) ty->descr=strdup(s);
  else ty->descr=NULL;
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



const char *Typemaker2_Type_GetAqDbType(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);

  if (ty->aqdb_type==NULL && ty->extendsPtr)
    return Typemaker2_Type_GetAqDbType(ty->extendsPtr);

  return ty->aqdb_type;
}



void Typemaker2_Type_SetAqDbType(TYPEMAKER2_TYPE *ty, const char *s) {
  assert(ty);
  assert(ty->refCount);
  free(ty->aqdb_type);
  if (s && *s) ty->aqdb_type=strdup(s);
  else ty->aqdb_type=NULL;
}



const char *Typemaker2_Type_GetBaseFileName(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);

  return ty->baseFileName;
}



void Typemaker2_Type_SetBaseFileName(TYPEMAKER2_TYPE *ty, const char *s) {
  assert(ty);
  assert(ty->refCount);
  free(ty->baseFileName);
  if (s && *s) ty->baseFileName=strdup(s);
  else ty->baseFileName=NULL;
}



const char *Typemaker2_Type_GetFreeHook(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);

  return ty->freeHook;
}



void Typemaker2_Type_SetFreeHook(TYPEMAKER2_TYPE *ty, const char *s) {
  assert(ty);
  assert(ty->refCount);
  free(ty->freeHook);
  if (s && *s) ty->freeHook=strdup(s);
  else ty->freeHook=NULL;
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



uint32_t Typemaker2_Type_GetCopyFlags(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);

  if (ty->copyFlags==0 && ty->extendsPtr)
    return Typemaker2_Type_GetCopyFlags(ty->extendsPtr);

  return ty->copyFlags;
}



void Typemaker2_Type_SetCopyFlags(TYPEMAKER2_TYPE *ty, uint32_t i) {
  assert(ty);
  assert(ty->refCount);
  ty->copyFlags=i;
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



int Typemaker2_Type_GetUsePrivateConstructor(const TYPEMAKER2_TYPE *ty){
  assert(ty);
  assert(ty->refCount);
  return ty->usePrivateConstructor;
}



void Typemaker2_Type_SetUsePrivateConstructor(TYPEMAKER2_TYPE *ty, int i){
  assert(ty);
  assert(ty->refCount);
  ty->usePrivateConstructor=i;
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



TYPEMAKER2_HEADER_LIST *Typemaker2_Type_GetHeaders(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);
  return ty->headers;
}



TYPEMAKER2_ENUM_LIST *Typemaker2_Type_GetEnums(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);
  return ty->enums;
}



TYPEMAKER2_DEFINE_LIST *Typemaker2_Type_GetDefines(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);
  return ty->defines;
}



TYPEMAKER2_CODE_LIST *Typemaker2_Type_GetCodeDefs(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);
  return ty->codeDefs;
}



TYPEMAKER2_INLINE_LIST *Typemaker2_Type_GetInlines(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);
  return ty->inlines;
}



TYPEMAKER2_VIRTUALFN_LIST *Typemaker2_Type_GetVirtualFns(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);
  return ty->virtualFns;
}



TYPEMAKER2_SIGNAL_LIST *Typemaker2_Type_GetSignals(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);
  return ty->signals;
}



TYPEMAKER2_SLOT_LIST *Typemaker2_Type_GetSlots(const TYPEMAKER2_TYPE *ty) {
  assert(ty);
  assert(ty->refCount);
  return ty->slots;
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

  s=GWEN_XMLNode_GetCharValue(langNode, "aqdb_type", NULL);
  Typemaker2_Type_SetAqDbType(ty, s);

  /* read base file name (used to derive other output filenames) */
  s=GWEN_XMLNode_GetCharValue(langNode, "basefilename", NULL);
  Typemaker2_Type_SetBaseFileName(ty, s);

  s=GWEN_XMLNode_GetCharValue(langNode, "freeHook", NULL);
  Typemaker2_Type_SetFreeHook(ty, s);

  /* read flags. this element exists for <type> elements.
   * For <typedef> elements the flags are stored in the <defaults> group. */
  s=GWEN_XMLNode_GetCharValue(langNode, "flags", NULL);
  if (s && *s)
    Typemaker2_Type_SetFlags(ty, Typemaker2_FlagsFromString(s));

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
      Typemaker2_Member_List_Add(tm, ty->members);
      nn=GWEN_XMLNode_FindNextTag(nn, "member", NULL, NULL);
    }
  }

  /* read codedefs */
  n=GWEN_XMLNode_FindFirstTag(langNode, "codedefs", NULL, NULL);
  if (n) {
    GWEN_XMLNODE *nn;

    nn=GWEN_XMLNode_FindFirstTag(n, "codedef", NULL, NULL);
    while(nn) {
      TYPEMAKER2_CODE *tc;

      tc=Typemaker2_Code_fromXml(nn);
      if (tc) {
        const char *s;

	s=Typemaker2_Code_GetMemberFlagsMask(tc);
	if (s && *s)
	  Typemaker2_Code_SetMemberFlagsMaskInt(tc, Typemaker2_FlagsFromString(s));

	s=Typemaker2_Code_GetMemberFlagsValue(tc);
	if (s && *s)
	  Typemaker2_Code_SetMemberFlagsValueInt(tc, Typemaker2_FlagsFromString(s));
      }

      Typemaker2_Code_List_Add(tc, ty->codeDefs);
      nn=GWEN_XMLNode_FindNextTag(nn, "codedef", NULL, NULL);
    }
  }

  /* read inlines */
  n=GWEN_XMLNode_FindFirstTag(langNode, "inlines", NULL, NULL);
  if (n) {
    GWEN_XMLNODE *nn;

    nn=GWEN_XMLNode_FindFirstTag(n, "inline", NULL, NULL);
    while(nn) {
      TYPEMAKER2_INLINE *ti;

      ti=Typemaker2_Inline_fromXml(nn);
      Typemaker2_Inline_List_Add(ti, ty->inlines);
      nn=GWEN_XMLNode_FindNextTag(nn, "inline", NULL, NULL);
    }
  }

  /* read enums */
  n=GWEN_XMLNode_FindFirstTag(node, "enums", NULL, NULL);
  if (n) {
    GWEN_XMLNODE *nn;

    nn=GWEN_XMLNode_FindFirstTag(n, "enum", NULL, NULL);
    while(nn) {
      TYPEMAKER2_ENUM *te;
      GWEN_XMLNODE *nnn;

      te=Typemaker2_Enum_fromXml(nn);

      /* read items */
      nnn=GWEN_XMLNode_FindFirstTag(nn, "item", NULL, NULL);
      while(nnn) {
	TYPEMAKER2_ITEM *ti;

	ti=Typemaker2_Item_fromXml(nnn);
	Typemaker2_Item_List_Add(ti, Typemaker2_Enum_GetItems(te));
	nnn=GWEN_XMLNode_FindNextTag(nnn, "item", NULL, NULL);
      }

      Typemaker2_Enum_List_Add(te, ty->enums);
      nn=GWEN_XMLNode_FindNextTag(nn, "enum", NULL, NULL);
    }
  }

  /* read defines */
  n=GWEN_XMLNode_FindFirstTag(node, "defines", NULL, NULL);
  if (n) {
    GWEN_XMLNODE *nn;

    nn=GWEN_XMLNode_FindFirstTag(n, "define", NULL, NULL);
    while(nn) {
      TYPEMAKER2_DEFINE *td;
      GWEN_XMLNODE *nnn;

      td=Typemaker2_Define_fromXml(nn);

      /* read items */
      nnn=GWEN_XMLNode_FindFirstTag(nn, "item", NULL, NULL);
      while(nnn) {
	TYPEMAKER2_ITEM *ti;

	ti=Typemaker2_Item_fromXml(nnn);
	Typemaker2_Item_List_Add(ti, Typemaker2_Define_GetItems(td));
	nnn=GWEN_XMLNode_FindNextTag(nnn, "item", NULL, NULL);
      }

      Typemaker2_Define_List_Add(td, ty->defines);
      nn=GWEN_XMLNode_FindNextTag(nn, "define", NULL, NULL);
    }
  }

  /* read virtual functions */
  n=GWEN_XMLNode_FindFirstTag(node, "virtualFns", NULL, NULL);
  if (n) {
    GWEN_XMLNODE *nn;

    nn=GWEN_XMLNode_FindFirstTag(n, "fn", NULL, NULL);
    while(nn) {
      TYPEMAKER2_VIRTUALFN *vf;
      int rv;

      vf=Typemaker2_VirtualFn_new();
      rv=Typemaker2_VirtualFn_readXml(vf, nn);
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	Typemaker2_VirtualFn_free(vf);
        return rv;
      }
      Typemaker2_VirtualFn_List_Add(vf, ty->virtualFns);
      nn=GWEN_XMLNode_FindNextTag(nn, "fn", NULL, NULL);
    }
  }

  /* read signals */
  n=GWEN_XMLNode_FindFirstTag(node, "signals", NULL, NULL);
  if (n) {
    GWEN_XMLNODE *nn;

    nn=GWEN_XMLNode_FindFirstTag(n, "signal", NULL, NULL);
    while(nn) {
      TYPEMAKER2_SIGNAL *sig;
      GWEN_XMLNODE *nnn;
      int i;

      s=GWEN_XMLNode_GetProperty(nn, "name", NULL);
      if (!(s && *s)) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Unnamed signal");
        return GWEN_ERROR_INVALID;
      }
      sig=Typemaker2_Signal_new();
      Typemaker2_Signal_SetName(sig, s);
      Typemaker2_Signal_SetParamType1(sig, "none");
      Typemaker2_Signal_SetParamType2(sig, "none");

      i=GWEN_XMLNode_GetIntProperty(nn, "useParam3", 0);
      if (i)
        Typemaker2_Signal_AddFlags(sig, TYPEMAKER2_SIGNAL_FLAGS_USE_PARAM3);
      i=GWEN_XMLNode_GetIntProperty(nn, "useParam4", 0);
      if (i)
        Typemaker2_Signal_AddFlags(sig, TYPEMAKER2_SIGNAL_FLAGS_USE_PARAM4);

      nnn=GWEN_XMLNode_FindFirstTag(nn, "params", NULL, NULL);
      if (nnn) {
        GWEN_XMLNODE *nnnn;
        int i;

        nnnn=GWEN_XMLNode_FindFirstTag(nnn, "param", NULL, NULL);
        i=1;
        while(nnnn && i<3) {
          const char *s;

          s=GWEN_XMLNode_GetProperty(nnnn, "type", "none");
          if (i==1)
            Typemaker2_Signal_SetParamType1(sig, s);
          else
            Typemaker2_Signal_SetParamType2(sig, s);
          i++;
          nnnn=GWEN_XMLNode_FindNextTag(nnnn, "param", NULL, NULL);
        }
      }
      Typemaker2_Signal_List_Add(sig, ty->signals);
      nn=GWEN_XMLNode_FindNextTag(nn, "signal", NULL, NULL);
    }
  }


  /* read slots */
  n=GWEN_XMLNode_FindFirstTag(node, "slots", NULL, NULL);
  if (n) {
    GWEN_XMLNODE *nn;

    nn=GWEN_XMLNode_FindFirstTag(n, "slot", NULL, NULL);
    while(nn) {
      TYPEMAKER2_SLOT *slot;
      GWEN_XMLNODE *nnn;
      int i;

      s=GWEN_XMLNode_GetProperty(nn, "name", NULL);
      if (!(s && *s)) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Unnamed slot");
        return GWEN_ERROR_INVALID;
      }
      slot=Typemaker2_Slot_new();
      Typemaker2_Slot_SetName(slot, s);
      Typemaker2_Slot_SetParamType1(slot, "none");
      Typemaker2_Slot_SetParamType2(slot, "none");

      i=GWEN_XMLNode_GetIntProperty(nn, "useParam3", 0);
      if (i)
        Typemaker2_Slot_AddFlags(slot, TYPEMAKER2_SLOT_FLAGS_USE_PARAM3);
      i=GWEN_XMLNode_GetIntProperty(nn, "useParam4", 0);
      if (i)
        Typemaker2_Slot_AddFlags(slot, TYPEMAKER2_SLOT_FLAGS_USE_PARAM4);


      nnn=GWEN_XMLNode_FindFirstTag(nn, "params", NULL, NULL);
      if (nnn) {
        GWEN_XMLNODE *nnnn;
        int i;

        nnnn=GWEN_XMLNode_FindFirstTag(nnn, "param", NULL, NULL);
        i=1;
        while(nnnn && i<3) {
          const char *s;

          s=GWEN_XMLNode_GetProperty(nnnn, "type", "none");
          if (i==1)
            Typemaker2_Slot_SetParamType1(slot, s);
          else
            Typemaker2_Slot_SetParamType2(slot, s);
          i++;
          nnnn=GWEN_XMLNode_FindNextTag(nnnn, "param", NULL, NULL);
        }
      }
      Typemaker2_Slot_List_Add(slot, ty->slots);
      nn=GWEN_XMLNode_FindNextTag(nn, "slot", NULL, NULL);
    }
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

    /* read copyflags */
    s=GWEN_XMLNode_GetCharValue(n, "copyflags", NULL);
    if (s && *s)
      Typemaker2_Type_SetCopyFlags(ty, Typemaker2_FlagsFromString(s));

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

  s=GWEN_XMLNode_GetProperty(langNode, "usePrivateConstructor", "0");
  if (s && *s)
    Typemaker2_Type_SetUsePrivateConstructor(ty, atoi(s));


  /* read description */
  n=GWEN_XMLNode_FindFirstTag(node, "descr", NULL, NULL);
  if (n) {
    GWEN_BUFFER *tbuf;
    int rv;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    rv=GWEN_XMLNode_toBuffer(n, tbuf, GWEN_XML_FLAGS_SIMPLE | GWEN_XML_FLAGS_HANDLE_COMMENTS);
    if (rv<0) {
      DBG_ERROR(0, "here (%d)", rv);
    }
    else {
      Typemaker2_Type_SetDescription(ty, GWEN_Buffer_GetStart(tbuf));
    }
    GWEN_Buffer_free(tbuf);
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
    fprintf(f, "AEDB Type : %s\n", (ty->aqdb_type)?(ty->aqdb_type):"<null>");

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
    fprintf(f, "CopyFlags : %08x [%08x]\n", ty->copyFlags, Typemaker2_Type_GetCopyFlags(ty));

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

    for (i=0; i<indent+2; i++) fprintf(f, " ");
    fprintf(f, "Descript. : %s\n", (ty->descr)?(ty->descr):"<null>");

  }
}



TYPEMAKER2_ENUM *Typemaker2_Type_FindEnum(TYPEMAKER2_TYPE *ty, const char *s) {
  TYPEMAKER2_ENUM *te;

  assert(ty);
  te=Typemaker2_Enum_List_First(ty->enums);
  while(te) {
    const char *n;

    n=Typemaker2_Enum_GetId(te);
    if (n && strcasecmp(s, n)==0)
      break;
    te=Typemaker2_Enum_List_Next(te);
  }

  return te;
}



TYPEMAKER2_CODE *Typemaker2_Type_FindCodeForMember(const TYPEMAKER2_TYPE *ty,
                                                   const TYPEMAKER2_MEMBER *tm,
						   const char *id) {
  TYPEMAKER2_CODE *tc=NULL;
  uint32_t flags=0;

  if (tm)
    flags=Typemaker2_Member_GetFlags(tm);

  while(ty) {
    tc=Typemaker2_Code_List_First(ty->codeDefs);
    while(tc) {
      const char *s;

      s=Typemaker2_Code_GetId(tc);
      if (s && strcasecmp(s, id)==0) {
	if ((flags & Typemaker2_Code_GetMemberFlagsMaskInt(tc))==Typemaker2_Code_GetMemberFlagsValueInt(tc))
          return tc;
      }
      tc=Typemaker2_Code_List_Next(tc);
    }

    ty=ty->extendsPtr;
  }

  return NULL;
}





