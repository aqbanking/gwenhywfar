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


#include "tm_inline_p.h"
#include "tm_util.h"


#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <assert.h>



GWEN_LIST_FUNCTIONS(TYPEMAKER2_INLINE, Typemaker2_Inline)



TYPEMAKER2_INLINE *Typemaker2_Inline_new() {
  TYPEMAKER2_INLINE *th;

  GWEN_NEW_OBJECT(TYPEMAKER2_INLINE, th);
  th->refCount=1;
  GWEN_LIST_INIT(TYPEMAKER2_INLINE, th);

  return th;
}



void Typemaker2_Inline_free(TYPEMAKER2_INLINE *th) {
  if (th) {
    assert(th->refCount);
    if (th->refCount==1) {
      GWEN_LIST_FINI(TYPEMAKER2_INLINE, th);
      free(th->content);
      th->refCount=0;
      GWEN_FREE_OBJECT(th);
    }
    else
      th->refCount++;
  }
}



void Typemaker2_Inline_Attach(TYPEMAKER2_INLINE *th) {
  assert(th);
  assert(th->refCount);
  th->refCount++;
}



const char *Typemaker2_Inline_GetContent(const TYPEMAKER2_INLINE *th) {
  assert(th);
  assert(th->refCount);
  return th->content;
}



void Typemaker2_Inline_SetContent(TYPEMAKER2_INLINE *th, const char *s) {
  assert(th);
  assert(th->refCount);
  free(th->content);
  if (s) th->content=strdup(s);
  else th->content=NULL;
}



int Typemaker2_Inline_GetLocation(const TYPEMAKER2_INLINE *th) {
  assert(th);
  assert(th->refCount);
  return th->location;
}



void Typemaker2_Inline_SetLocation(TYPEMAKER2_INLINE *th, int i) {
  assert(th);
  assert(th->refCount);
  th->location=i;
}



int Typemaker2_Inline_GetAccess(const TYPEMAKER2_INLINE *th) {
  assert(th);
  assert(th->refCount);
  return th->acc;
}



void Typemaker2_Inline_SetAccess(TYPEMAKER2_INLINE *th, int i) {
  assert(th);
  assert(th->refCount);
  th->acc=i;
}



int Typemaker2_Inline_GetTypeFlagsMask(const TYPEMAKER2_INLINE *th) {
  assert(th);
  assert(th->refCount);
  return th->typeFlagsMask;
}



int Typemaker2_Inline_GetTypeFlagsValue(const TYPEMAKER2_INLINE *th) {
  assert(th);
  assert(th->refCount);
  return th->typeFlagsValue;
}



TYPEMAKER2_INLINE *Typemaker2_Inline_fromXml(GWEN_XMLNODE *node) {
  TYPEMAKER2_INLINE *th;
  const char *s;

  th=Typemaker2_Inline_new();

  s=GWEN_XMLNode_GetCharValue(node, "content", NULL);
  Typemaker2_Inline_SetContent(th, s);

  s=GWEN_XMLNode_GetCharValue(node, "typeFlagsMask", NULL);
  if (s && *s)
    th->typeFlagsMask=Typemaker2_FlagsFromString(s);

  s=GWEN_XMLNode_GetCharValue(node, "typeFlagsValue", NULL);
  if (s && *s)
    th->typeFlagsValue=Typemaker2_FlagsFromString(s);

  /* read header location */
  s=GWEN_XMLNode_GetProperty(node, "loc", "post");
  if (strcasecmp(s, "header")==0)
    Typemaker2_Inline_SetLocation(th, Typemaker2_InlineLocation_Header);
  else if (strcasecmp(s, "code")==0)
    Typemaker2_Inline_SetLocation(th, Typemaker2_InlineLocation_Code);

  /* read access */
  s=GWEN_XMLNode_GetProperty(node, "access", "public");
  Typemaker2_Inline_SetAccess(th, Typemaker2_AccessFromString(s));

  return th;
}







