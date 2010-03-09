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


#include "tm_header_p.h"


#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <assert.h>



GWEN_LIST_FUNCTIONS(TYPEMAKER2_HEADER, Typemaker2_Header)



TYPEMAKER2_HEADER *Typemaker2_Header_new() {
  TYPEMAKER2_HEADER *th;

  GWEN_NEW_OBJECT(TYPEMAKER2_HEADER, th);
  th->refCount=1;
  GWEN_LIST_INIT(TYPEMAKER2_HEADER, th);

  return th;
}



void Typemaker2_Header_free(TYPEMAKER2_HEADER *th) {
  if (th) {
    assert(th->refCount);
    if (th->refCount==1) {
      GWEN_LIST_FINI(TYPEMAKER2_HEADER, th);
      free(th->fileName);
      th->refCount=0;
      GWEN_FREE_OBJECT(th);
    }
    else
      th->refCount++;
  }
}



void Typemaker2_Header_Attach(TYPEMAKER2_HEADER *th) {
  assert(th);
  assert(th->refCount);
  th->refCount++;
}



const char *Typemaker2_Header_GetFileName(const TYPEMAKER2_HEADER *th) {
  assert(th);
  assert(th->refCount);
  return th->fileName;
}



void Typemaker2_Header_SetFileName(TYPEMAKER2_HEADER *th, const char *s) {
  assert(th);
  assert(th->refCount);
  free(th->fileName);
  if (s) th->fileName=strdup(s);
  else th->fileName=NULL;
}



int Typemaker2_Header_GetType(const TYPEMAKER2_HEADER *th) {
  assert(th);
  assert(th->refCount);
  return th->type;
}



void Typemaker2_Header_SetType(TYPEMAKER2_HEADER *th, int i) {
  assert(th);
  assert(th->refCount);
  th->type=i;
}



int Typemaker2_Header_GetLocation(const TYPEMAKER2_HEADER *th) {
  assert(th);
  assert(th->refCount);
  return th->location;
}



void Typemaker2_Header_SetLocation(TYPEMAKER2_HEADER *th, int i) {
  assert(th);
  assert(th->refCount);
  th->location=i;
}



int Typemaker2_Header_readXml(TYPEMAKER2_HEADER *th, GWEN_XMLNODE *node) {
  const char *s;
  GWEN_XMLNODE *n;

  assert(th);
  assert(th->refCount);

  n=GWEN_XMLNode_GetFirstData(node);
  if (n) {
    s=GWEN_XMLNode_GetData(n);
    Typemaker2_Header_SetFileName(th, s);
  }

  /* read header type */
  s=GWEN_XMLNode_GetProperty(node, "type", "sys");
  if (strcasecmp(s, "sys")==0 ||
      strcasecmp(s, "system")==0)
    Typemaker2_Header_SetType(th, Typemaker2_HeaderType_System);
  else if (strcasecmp(s, "local")==0)
    Typemaker2_Header_SetType(th, Typemaker2_HeaderType_Local);

  /* read header location */
  s=GWEN_XMLNode_GetProperty(node, "loc", "post");
  if (strcasecmp(s, "pre")==0)
    Typemaker2_Header_SetLocation(th, Typemaker2_HeaderLocation_Pre);
  else if (strcasecmp(s, "post")==0)
    Typemaker2_Header_SetLocation(th, Typemaker2_HeaderLocation_Post);
  else if (strcasecmp(s, "code")==0)
    Typemaker2_Header_SetLocation(th, Typemaker2_HeaderLocation_Code);
  else if (strcasecmp(s, "codeEnd")==0)
    Typemaker2_Header_SetLocation(th, Typemaker2_HeaderLocation_CodeEnd);

  return 0;
}







