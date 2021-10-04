/***************************************************************************
    begin       : Thu Jul 02 2009
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "tm2c_setter.h"

#include <gwenhywfar/debug.h>

#include <ctype.h>



int TM2C_AddReleaseOldValueCode(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm, GWEN_BUFFER *tbuf)
{
  TYPEMAKER2_TYPE *mty;
  GWEN_BUFFER *srcbuf;
  int rv;

  mty=Typemaker2_Member_GetTypePtr(tm);
  assert(mty);

  if (Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Pointer)
    GWEN_Buffer_AppendArgs(tbuf, "  if (p_struct->%s) {\n", Typemaker2_Member_GetName(tm));

  /* invoke free */
  srcbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendArgs(srcbuf, "p_struct->%s", Typemaker2_Member_GetName(tm));

  GWEN_Buffer_AppendString(tbuf, "    ");
  rv=Typemaker2_Builder_Invoke_DestructFn(tb, ty, tm,
					  GWEN_Buffer_GetStart(srcbuf),
					  NULL, /* no dest */
					  tbuf);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(srcbuf);
    return rv;
  }
  GWEN_Buffer_AppendString(tbuf, "\n");
  GWEN_Buffer_free(srcbuf);


  if (Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Pointer) {
    GWEN_Buffer_AppendString(tbuf, "  }\n");
  }

  return 0;
}


