/***************************************************************************
    begin       : Thu Jul 02 2009
    copyright   : (C) 2009 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



#include "tm_builder_c.h"







static int Typemaker2_Builder_C_Build(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty) {
  return 0;
}



TYPEMAKER2_BUILDER *Typemaker2_Builder_C_new() {
  TYPEMAKER2_BUILDER *tb;

  tb=Typemaker2_Builder_new();
  Typemaker2_Builder_SetBuildFn(tb, Typemaker2_Builder_C_Build);

  return tb;
}





