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


#include "tm_group_p.h"


#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <assert.h>



GWEN_TREE_FUNCTIONS(TYPEMAKER2_GROUP, Typemaker2_Group)



TYPEMAKER2_GROUP *Typemaker2_Group_new() {
  TYPEMAKER2_GROUP *gr;

  GWEN_NEW_OBJECT(TYPEMAKER2_GROUP, gr);
  gr->refCount=1;
  GWEN_TREE_INIT(TYPEMAKER2_GROUP, gr);

  return gr;
}



void Typemaker2_Group_free(TYPEMAKER2_GROUP *gr) {
  if (gr) {
    assert(gr->refCount);
    if (gr->refCount==1) {
      GWEN_TREE_FINI(TYPEMAKER2_GROUP, gr);
      free(gr->title);
      free(gr->description);
      gr->refCount=0;
      GWEN_FREE_OBJECT(gr);
    }
    else
      gr->refCount++;
  }
}



void Typemaker2_Group_Attach(TYPEMAKER2_GROUP *gr) {
  assert(gr);
  assert(gr->refCount);
  gr->refCount++;
}



const char *Typemaker2_Group_GetTitle(TYPEMAKER2_GROUP *gr) {
  assert(gr);
  assert(gr->refCount);

  return gr->title;
}



void Typemaker2_Group_SetTitle(TYPEMAKER2_GROUP *gr, const char *s) {
  assert(gr);
  assert(gr->refCount);

  free(gr->title);
  if (s && *s) gr->title=strdup(s);
  else gr->title=NULL;
}



const char *Typemaker2_Group_GetDescription(TYPEMAKER2_GROUP *gr) {
  assert(gr);
  assert(gr->refCount);

  return gr->description;
}



void Typemaker2_Group_SetDescription(TYPEMAKER2_GROUP *gr, const char *s) {
  assert(gr);
  assert(gr->refCount);

  free(gr->description);
  if (s && *s) gr->description=strdup(s);
  else gr->description=NULL;
}



int Typemaker2_Group_readXml(TYPEMAKER2_GROUP *gr, GWEN_XMLNODE *node) {
  const char *s;
  GWEN_XMLNODE *n;

  assert(gr);
  assert(gr->refCount);

  /* read title */
  s=GWEN_XMLNode_GetProperty(node, "title", NULL);
  if (s && *s)
    Typemaker2_Group_SetTitle(gr, s);

  /* read descr */
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
      Typemaker2_Group_SetDescription(gr, GWEN_Buffer_GetStart(tbuf));
    }
    GWEN_Buffer_free(tbuf);
  }

  return 0;
}







