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
GWEN_LIST_FUNCTIONS(TYPEMAKER2_VIRTUALFN_PARAM, Typemaker2_VirtualFn_Param);



TYPEMAKER2_VIRTUALFN *Typemaker2_VirtualFn_new()
{
  TYPEMAKER2_VIRTUALFN *vf;

  GWEN_NEW_OBJECT(TYPEMAKER2_VIRTUALFN, vf);
  vf->refCount=1;
  GWEN_LIST_INIT(TYPEMAKER2_VIRTUALFN, vf);
  vf->access=TypeMaker2_Access_Public;

  vf->paramList=Typemaker2_VirtualFn_Param_List_new();
  return vf;
}



void Typemaker2_VirtualFn_free(TYPEMAKER2_VIRTUALFN *vf)
{
  if (vf) {
    if (vf->refCount==1) {
      GWEN_LIST_FINI(TYPEMAKER2_VIRTUALFN, vf);
      free(vf->preset);
      free(vf->name);
      free(vf->descr);
      free(vf->returnType);
      free(vf->defaultReturnValue);
      free(vf->location);
      Typemaker2_VirtualFn_Param_List_free(vf->paramList);
      vf->refCount=0;
      GWEN_FREE_OBJECT(vf);
    }
    else
      vf->refCount--;
  }
}



void Typemaker2_VirtualFn_Attach(TYPEMAKER2_VIRTUALFN *vf)
{
  assert(vf);
  assert(vf->refCount);
  vf->refCount++;
}



int Typemaker2_VirtualFn_readXml(TYPEMAKER2_VIRTUALFN *vf, GWEN_XMLNODE *node)
{
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

  s=GWEN_XMLNode_GetCharValue(node, "preset", NULL);
  if (s && *s)
    Typemaker2_VirtualFn_SetPreset(vf, s);

  /* read return type */
  s=GWEN_XMLNode_GetCharValue(node, "returnType", NULL);
  if (s && *s)
    Typemaker2_VirtualFn_SetReturnType(vf, s);

  /* read returnType description */
  n=GWEN_XMLNode_FindFirstTag(node, "returnType", NULL, NULL);
  if (n) {
    s=GWEN_XMLNode_GetCharValue(n, "descr", NULL);
    Typemaker2_VirtualFn_SetReturnTypeDescr(vf, s);
  }


  n=GWEN_XMLNode_FindFirstTag(node, "params", NULL, NULL);
  if (n) {
    n=GWEN_XMLNode_FindFirstTag(n, "param", NULL, NULL);
    while (n) {
      const char *paramName;
      const char *paramType;
      const char *paramDescr;
      TYPEMAKER2_VIRTUALFN_PARAM *prm;

      paramType=GWEN_XMLNode_GetProperty(n, "type", NULL);
      paramName=GWEN_XMLNode_GetProperty(n, "name", NULL);
      paramDescr=GWEN_XMLNode_GetCharValue(n, "descr", NULL);

      if (!(paramType && *paramType)) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Parameter has no type");
        return GWEN_ERROR_BAD_DATA;
      }

      prm=Typemaker2_VirtualFn_Param_new();
      Typemaker2_VirtualFn_Param_SetName(prm, paramName);
      Typemaker2_VirtualFn_Param_SetType(prm, paramType);
      Typemaker2_VirtualFn_Param_SetDescr(prm, paramDescr);
      Typemaker2_VirtualFn_Param_List_Add(prm, vf->paramList);

      n=GWEN_XMLNode_FindNextTag(n, "param", NULL, NULL);
    }
  }

  /* read descr */
  s=GWEN_XMLNode_GetCharValue(node, "descr", NULL);
  Typemaker2_VirtualFn_SetDescr(vf, s);


  return 0;
}



const char *Typemaker2_VirtualFn_GetName(const TYPEMAKER2_VIRTUALFN *vf)
{
  assert(vf);
  assert(vf->refCount);

  return vf->name;
}



void Typemaker2_VirtualFn_SetName(TYPEMAKER2_VIRTUALFN *vf, const char *s)
{
  assert(vf);
  assert(vf->refCount);

  free(vf->name);
  if (s)
    vf->name=strdup(s);
  else
    vf->name=NULL;
}



const char *Typemaker2_VirtualFn_GetLocation(const TYPEMAKER2_VIRTUALFN *vf)
{
  assert(vf);
  assert(vf->refCount);

  return vf->location;
}



void Typemaker2_VirtualFn_SetLocation(TYPEMAKER2_VIRTUALFN *vf, const char *s)
{
  assert(vf);
  assert(vf->refCount);

  free(vf->location);
  if (s)
    vf->location=strdup(s);
  else
    vf->location=NULL;
}



const char *Typemaker2_VirtualFn_GetDescr(const TYPEMAKER2_VIRTUALFN *vf)
{
  assert(vf);
  assert(vf->refCount);

  return vf->descr;
}



void Typemaker2_VirtualFn_SetDescr(TYPEMAKER2_VIRTUALFN *vf, const char *s)
{
  assert(vf);
  assert(vf->refCount);

  free(vf->descr);
  if (s)
    vf->descr=strdup(s);
  else
    vf->descr=NULL;
}



uint32_t Typemaker2_VirtualFn_GetFlags(const TYPEMAKER2_VIRTUALFN *vf)
{
  assert(vf);
  assert(vf->refCount);

  return vf->flags;
}



void Typemaker2_VirtualFn_SetFlags(TYPEMAKER2_VIRTUALFN *vf, uint32_t fl)
{
  assert(vf);
  assert(vf->refCount);

  vf->flags=fl;
}



void Typemaker2_VirtualFn_AddFlags(TYPEMAKER2_VIRTUALFN *vf, uint32_t fl)
{
  assert(vf);
  assert(vf->refCount);

  vf->flags|=fl;
}



void Typemaker2_VirtualFn_SubFlags(TYPEMAKER2_VIRTUALFN *vf, uint32_t fl)
{
  assert(vf);
  assert(vf->refCount);

  vf->flags&=~fl;
}



int Typemaker2_VirtualFn_GetAccess(const TYPEMAKER2_VIRTUALFN *vf)
{
  assert(vf);
  assert(vf->refCount);

  return vf->access;
}



void Typemaker2_VirtualFn_SetAccess(TYPEMAKER2_VIRTUALFN *vf, int i)
{
  assert(vf);
  assert(vf->refCount);

  vf->access=i;
}



const char *Typemaker2_VirtualFn_GetReturnType(const TYPEMAKER2_VIRTUALFN *vf)
{
  assert(vf);
  assert(vf->refCount);

  return vf->returnType;
}



void Typemaker2_VirtualFn_SetReturnType(TYPEMAKER2_VIRTUALFN *vf, const char *s)
{
  assert(vf);
  assert(vf->refCount);

  free(vf->returnType);
  if (s)
    vf->returnType=strdup(s);
  else
    vf->returnType=NULL;
}



const char *Typemaker2_VirtualFn_GetReturnTypeDescr(const TYPEMAKER2_VIRTUALFN *vf)
{
  assert(vf);
  return vf->returnTypeDescr;
}



void Typemaker2_VirtualFn_SetReturnTypeDescr(TYPEMAKER2_VIRTUALFN *vf, const char *s)
{
  assert(vf);
  free(vf->returnTypeDescr);
  if (s)
    vf->returnTypeDescr=strdup(s);
  else
    vf->returnTypeDescr=NULL;
}



const char *Typemaker2_VirtualFn_GetDefaultReturnValue(const TYPEMAKER2_VIRTUALFN *vf)
{
  assert(vf);
  assert(vf->refCount);

  return vf->defaultReturnValue;
}



void Typemaker2_VirtualFn_SetDefaultReturnValue(TYPEMAKER2_VIRTUALFN *vf, const char *s)
{
  assert(vf);
  assert(vf->refCount);

  free(vf->defaultReturnValue);
  if (s)
    vf->defaultReturnValue=strdup(s);
  else
    vf->defaultReturnValue=NULL;
}



TYPEMAKER2_VIRTUALFN_PARAM_LIST *Typemaker2_VirtualFn_GetParamTypeList(const TYPEMAKER2_VIRTUALFN *vf)
{
  assert(vf);
  assert(vf->refCount);

  return vf->paramList;
}



const char *Typemaker2_VirtualFn_GetPreset(const TYPEMAKER2_VIRTUALFN *vf)
{
  assert(vf);
  assert(vf->refCount);

  return vf->preset;
}



void Typemaker2_VirtualFn_SetPreset(TYPEMAKER2_VIRTUALFN *vf, const char *s)
{
  assert(vf);
  assert(vf->refCount);

  free(vf->preset);
  if (s)
    vf->preset=strdup(s);
  else
    vf->preset=NULL;
}












TYPEMAKER2_VIRTUALFN_PARAM *Typemaker2_VirtualFn_Param_new()
{
  TYPEMAKER2_VIRTUALFN_PARAM *prm;

  GWEN_NEW_OBJECT(TYPEMAKER2_VIRTUALFN_PARAM, prm);
  GWEN_LIST_INIT(TYPEMAKER2_VIRTUALFN_PARAM, prm);

  return prm;
}



void Typemaker2_VirtualFn_Param_free(TYPEMAKER2_VIRTUALFN_PARAM *prm)
{
  if (prm) {
    free(prm->name);
    free(prm->type);
    free(prm->descr);

    GWEN_FREE_OBJECT(prm);
  }
}



const char *Typemaker2_VirtualFn_Param_GetName(const TYPEMAKER2_VIRTUALFN_PARAM *prm)
{
  assert(prm);
  return prm->name;
}



void Typemaker2_VirtualFn_Param_SetName(TYPEMAKER2_VIRTUALFN_PARAM *prm, const char *s)
{
  assert(prm);
  free(prm->name);
  if (s)
    prm->name=strdup(s);
  else
    prm->name=NULL;
}



const char *Typemaker2_VirtualFn_Param_GetType(const TYPEMAKER2_VIRTUALFN_PARAM *prm)
{
  assert(prm);
  return prm->type;
}



void Typemaker2_VirtualFn_Param_SetType(TYPEMAKER2_VIRTUALFN_PARAM *prm, const char *s)
{
  assert(prm);
  free(prm->type);
  if (s)
    prm->type=strdup(s);
  else
    prm->type=NULL;
}



const char *Typemaker2_VirtualFn_Param_GetDescr(const TYPEMAKER2_VIRTUALFN_PARAM *prm)
{
  assert(prm);
  return prm->descr;
}



void Typemaker2_VirtualFn_Param_SetDescr(TYPEMAKER2_VIRTUALFN_PARAM *prm, const char *s)
{
  assert(prm);
  free(prm->descr);
  if (s)
    prm->descr=strdup(s);
  else
    prm->descr=NULL;
}








