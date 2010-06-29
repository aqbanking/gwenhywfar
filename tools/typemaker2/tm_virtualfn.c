/***************************************************************************
    begin       : Mon Jun 28 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "tm_virtualfn_p.h"
#include "tm_util.h"
#include "tm_type.h"


#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <assert.h>


GWEN_LIST_FUNCTIONS(TYPEMAKER2_VIRTUALFN, Typemaker2_VirtualFn);



TYPEMAKER2_VIRTUALFN *Typemaker2_VirtualFn_new() {
  TYPEMAKER2_VIRTUALFN *vf;

  GWEN_NEW_OBJECT(TYPEMAKER2_VIRTUALFN, vf);
  vf->refCount=1;
  GWEN_LIST_INIT(TYPEMAKER2_VIRTUALFN, vf);
  vf->paramTypes=GWEN_StringList_new();
  vf->access=TypeMaker2_Access_Public;

  return vf;
}



void Typemaker2_VirtualFn_free(TYPEMAKER2_VIRTUALFN *vf) {
  if (vf) {
    if (vf->refCount==1) {
      GWEN_LIST_FINI(TYPEMAKER2_VIRTUALFN, vf);
      GWEN_StringList_free(vf->paramTypes);
      free(vf->name);
      free(vf->returnType);
      free(vf->defaultReturnValue);
      free(vf->location);
      vf->refCount=0;
      GWEN_FREE_OBJECT(vf);
    }
    else
      vf->refCount--;
  }
}



void Typemaker2_VirtualFn_Attach(TYPEMAKER2_VIRTUALFN *vf) {
  assert(vf);
  assert(vf->refCount);
  vf->refCount++;
}



int Typemaker2_VirtualFn_readXml(TYPEMAKER2_VIRTUALFN *vf, GWEN_XMLNODE *node) {
  const char *s;
  GWEN_XMLNODE *n;

  assert(vf);
  assert(vf->refCount);

  s=GWEN_XMLNode_GetProperty(node, "name", NULL);
  if (s==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Member has no id");
    return GWEN_ERROR_BAD_DATA;
  }
  Typemaker2_VirtualFn_SetName(vf, s);

  s=GWEN_XMLNode_GetProperty(node, "location", "pre");
  Typemaker2_VirtualFn_SetLocation(vf, s);

  /* read flags */
  s=GWEN_XMLNode_GetCharValue(node, "flags", NULL);
  if (s && *s)
    Typemaker2_VirtualFn_SetFlags(vf, Typemaker2_FlagsFromString(s));

  /* read access */
  s=GWEN_XMLNode_GetCharValue(node, "access", NULL);
  if (s && *s) {
    int i=Typemaker2_AccessFromString(s);
    if (i==TypeMaker2_Access_Unknown) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unknown access type [%s]", s);
      return GWEN_ERROR_BAD_DATA;
    }
    Typemaker2_VirtualFn_SetAccess(vf, i);
  }

  /* read default value */
  s=GWEN_XMLNode_GetCharValue(node, "defaultReturnValue", NULL);
  if (s && *s)
    Typemaker2_VirtualFn_SetDefaultReturnValue(vf, s);

  /* read return type */
  s=GWEN_XMLNode_GetCharValue(node, "returnType", NULL);
  if (s && *s)
    Typemaker2_VirtualFn_SetReturnType(vf, s);

  n=GWEN_XMLNode_FindFirstTag(node, "params", NULL, NULL);
  if (n) {
    n=GWEN_XMLNode_FindFirstTag(n, "param", NULL, NULL);
    while(n) {
      s=GWEN_XMLNode_GetProperty(n, "type", NULL);
      if (s==NULL) {
	DBG_ERROR(GWEN_LOGDOMAIN, "Parameter has no type");
	return GWEN_ERROR_BAD_DATA;
      }
      GWEN_StringList_AppendString(vf->paramTypes, s, 0, 0);

      n=GWEN_XMLNode_FindNextTag(n, "param", NULL, NULL);
    }
  }

  return 0;
}



const char *Typemaker2_VirtualFn_GetName(const TYPEMAKER2_VIRTUALFN *vf) {
  assert(vf);
  assert(vf->refCount);

  return vf->name;
}



void Typemaker2_VirtualFn_SetName(TYPEMAKER2_VIRTUALFN *vf, const char *s) {
  assert(vf);
  assert(vf->refCount);

  free(vf->name);
  if (s) vf->name=strdup(s);
  else vf->name=NULL;
}



const char *Typemaker2_VirtualFn_GetLocation(const TYPEMAKER2_VIRTUALFN *vf) {
  assert(vf);
  assert(vf->refCount);

  return vf->location;
}



void Typemaker2_VirtualFn_SetLocation(TYPEMAKER2_VIRTUALFN *vf, const char *s) {
  assert(vf);
  assert(vf->refCount);

  free(vf->location);
  if (s) vf->location=strdup(s);
  else vf->location=NULL;
}



uint32_t Typemaker2_VirtualFn_GetFlags(const TYPEMAKER2_VIRTUALFN *vf) {
  assert(vf);
  assert(vf->refCount);

  return vf->flags;
}



void Typemaker2_VirtualFn_SetFlags(TYPEMAKER2_VIRTUALFN *vf, uint32_t fl) {
  assert(vf);
  assert(vf->refCount);

  vf->flags=fl;
}



void Typemaker2_VirtualFn_AddFlags(TYPEMAKER2_VIRTUALFN *vf, uint32_t fl) {
  assert(vf);
  assert(vf->refCount);

  vf->flags|=fl;
}



void Typemaker2_VirtualFn_SubFlags(TYPEMAKER2_VIRTUALFN *vf, uint32_t fl) {
  assert(vf);
  assert(vf->refCount);

  vf->flags&=~fl;
}



int Typemaker2_VirtualFn_GetAccess(const TYPEMAKER2_VIRTUALFN *vf) {
  assert(vf);
  assert(vf->refCount);

  return vf->access;
}



void Typemaker2_VirtualFn_SetAccess(TYPEMAKER2_VIRTUALFN *vf, int i) {
  assert(vf);
  assert(vf->refCount);

  vf->access=i;
}



const char *Typemaker2_VirtualFn_GetReturnType(const TYPEMAKER2_VIRTUALFN *vf) {
  assert(vf);
  assert(vf->refCount);

  return vf->returnType;
}



void Typemaker2_VirtualFn_SetReturnType(TYPEMAKER2_VIRTUALFN *vf, const char *s) {
  assert(vf);
  assert(vf->refCount);

  free(vf->returnType);
  if (s) vf->returnType=strdup(s);
  else vf->returnType=NULL;
}



const char *Typemaker2_VirtualFn_GetDefaultReturnValue(const TYPEMAKER2_VIRTUALFN *vf) {
  assert(vf);
  assert(vf->refCount);

  return vf->defaultReturnValue;
}



void Typemaker2_VirtualFn_SetDefaultReturnValue(TYPEMAKER2_VIRTUALFN *vf, const char *s) {
  assert(vf);
  assert(vf->refCount);

  free(vf->defaultReturnValue);
  if (s) vf->defaultReturnValue=strdup(s);
  else vf->defaultReturnValue=NULL;
}



GWEN_STRINGLIST *Typemaker2_VirtualFn_GetParamTypes(const TYPEMAKER2_VIRTUALFN *vf) {
  assert(vf);
  assert(vf->refCount);

  return vf->paramTypes;
}











