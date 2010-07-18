/***************************************************************************
 begin       : Sat Feb 20 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "htmlobject_p.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>

#include <assert.h>
#include <string.h>


GWEN_TREE_FUNCTIONS(HTML_OBJECT, HtmlObject)
GWEN_INHERIT_FUNCTIONS(HTML_OBJECT)



HTML_OBJECT *HtmlObject_new(GWEN_XML_CONTEXT *ctx, HTML_OBJECT_TYPE t) {
  HTML_OBJECT *o;

  GWEN_NEW_OBJECT(HTML_OBJECT, o);
  o->refCount=1;
  o->objectType=t;
  o->xmlCtx=ctx;
  GWEN_TREE_INIT(HTML_OBJECT, o);
  GWEN_INHERIT_INIT(HTML_OBJECT, o);

  return o;
}



void HtmlObject_free(HTML_OBJECT *o) {
  if (o) {
    assert(o->refCount);
    if (o->refCount>1)
      o->refCount--;
    else {
      GWEN_TREE_FINI(HTML_OBJECT, o);
      GWEN_INHERIT_FINI(HTML_OBJECT, o);

      free(o->text);
      HtmlProps_free(o->properties);

      o->refCount=0;
      GWEN_FREE_OBJECT(o);
    }
  }
}



void HtmlObject_Attach(HTML_OBJECT *o) {
  assert(o);
  assert(o->refCount);
  o->refCount++;
}



GWEN_XML_CONTEXT *HtmlObject_GetXmlCtx(const HTML_OBJECT *o) {
  assert(o);
  assert(o->refCount);
  return o->xmlCtx;
}



HTML_OBJECT_TYPE HtmlObject_GetObjectType(const HTML_OBJECT *o) {
  assert(o);
  assert(o->refCount);
  return o->objectType;
}



void HtmlObject_SetObjectType(HTML_OBJECT *o, HTML_OBJECT_TYPE t) {
  assert(o);
  assert(o->refCount);
  o->objectType=t;
}



HTML_PROPS *HtmlObject_GetProperties(const HTML_OBJECT *o) {
  assert(o);
  assert(o->refCount);
  return o->properties;
}



void HtmlObject_SetProperties(HTML_OBJECT *o, HTML_PROPS *pr) {
  assert(o);
  assert(o->refCount);

  HtmlProps_Attach(pr);
  HtmlProps_free(o->properties);
  o->properties=pr;
}



int HtmlObject_GetX(const HTML_OBJECT *o) {
  assert(o);
  assert(o->refCount);
  return o->x;
}



void HtmlObject_SetX(HTML_OBJECT *o, int i) {
  assert(o);
  assert(o->refCount);
  o->x=i;
}



int HtmlObject_GetY(const HTML_OBJECT *o) {
  assert(o);
  assert(o->refCount);
  return o->y;
}



void HtmlObject_SetY(HTML_OBJECT *o, int i) {
  assert(o);
  assert(o->refCount);
  o->y=i;
}



int HtmlObject_GetWidth(const HTML_OBJECT *o) {
  assert(o);
  assert(o->refCount);
  return o->width;
}



void HtmlObject_SetWidth(HTML_OBJECT *o, int i) {
  assert(o);
  assert(o->refCount);
  o->width=i;
}



int HtmlObject_GetHeight(const HTML_OBJECT *o) {
  assert(o);
  assert(o->refCount);
  return o->height;
}



void HtmlObject_SetHeight(HTML_OBJECT *o, int i) {
  assert(o);
  assert(o->refCount);
  o->height=i;
}



int HtmlObject_GetConfiguredWidth(const HTML_OBJECT *o) {
  assert(o);
  assert(o->refCount);
  return o->configuredWidth;
}



void HtmlObject_SetConfiguredWidth(HTML_OBJECT *o, int i) {
  assert(o);
  assert(o->refCount);
  o->configuredWidth=i;
}



int HtmlObject_GetConfiguredHeight(const HTML_OBJECT *o) {
  assert(o);
  assert(o->refCount);
  return o->configuredHeight;
}



void HtmlObject_SetConfiguredHeight(HTML_OBJECT *o, int i) {
  assert(o);
  assert(o->refCount);
  o->configuredHeight=i;
}



const char *HtmlObject_GetText(const HTML_OBJECT *o) {
  assert(o);
  assert(o->refCount);
  return o->text;
}



void HtmlObject_SetText(HTML_OBJECT *o, const char *s) {
  assert(o);
  assert(o->refCount);
  free(o->text);
  if (s) o->text=strdup(s);
  else o->text=NULL;
}



uint32_t HtmlObject_GetFlags(const HTML_OBJECT *o) {
  assert(o);
  assert(o->refCount);

  return o->flags;
}



void HtmlObject_SetFlags(HTML_OBJECT *o, uint32_t fl) {
  assert(o);
  assert(o->refCount);

  o->flags=fl;
}



void HtmlObject_AddFlags(HTML_OBJECT *o, uint32_t fl) {
  assert(o);
  assert(o->refCount);

  o->flags|=fl;
}



void HtmlObject_SubFlags(HTML_OBJECT *o, uint32_t fl) {
  assert(o);
  assert(o->refCount);

  o->flags&=~fl;
}



int HtmlObject_Layout(HTML_OBJECT *o) {
  assert(o);
  assert(o->refCount);
  if (o->layoutFn)
    return o->layoutFn(o);
  else {
    o->width=0;
    o->height=0;
    return 0;
  }
}



HTML_OBJECT_LAYOUT_FN HtmlObject_SetLayoutFn(HTML_OBJECT *o,
					     HTML_OBJECT_LAYOUT_FN fn) {
  HTML_OBJECT_LAYOUT_FN of;

  of=o->layoutFn;
  o->layoutFn=fn;
  return of;
}



