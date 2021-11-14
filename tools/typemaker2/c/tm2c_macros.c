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


#include "tm2c_macros.h"
#include "tm2c_misc.h"

#include <gwenhywfar/debug.h>

#include <ctype.h>



int TM2C_BuildMacroFunctions(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  GWEN_BUFFER *tbuf;
  uint32_t flags;
  const char *sTypeId;
  const char *sTypePrefix;

  sTypeId=Typemaker2_Type_GetIdentifier(ty);
  sTypePrefix=Typemaker2_Type_GetPrefix(ty);

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  GWEN_Buffer_AppendString(tbuf, "/* macro functions */\n");

  flags=Typemaker2_Type_GetFlags(ty);
  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_LIST1)
    GWEN_Buffer_AppendArgs(tbuf, "GWEN_LIST_FUNCTIONS(%s, %s)\n", sTypeId, sTypePrefix);

  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_LIST2)
    GWEN_Buffer_AppendArgs(tbuf, "GWEN_LIST2_FUNCTIONS(%s, %s)\n", sTypeId, sTypePrefix);

  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_CONSTLIST2)
    GWEN_Buffer_AppendArgs(tbuf, "GWEN_CONSTLIST2_FUNCTIONS(%s, %s)\n", sTypeId, sTypePrefix);

  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_TREE)
    GWEN_Buffer_AppendArgs(tbuf, "GWEN_TREE_FUNCTIONS(%s, %s)\n", sTypeId, sTypePrefix);

  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_TREE2)
    GWEN_Buffer_AppendArgs(tbuf, "GWEN_TREE2_FUNCTIONS(%s, %s)\n", sTypeId, sTypePrefix);

  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_INHERIT)
    GWEN_Buffer_AppendArgs(tbuf, "GWEN_INHERIT_FUNCTIONS(%s)\n", sTypeId);

  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_IDMAP)
    GWEN_Buffer_AppendArgs(tbuf, "GWEN_IDMAP_FUNCTIONS(%s, %s)\n", sTypeId, sTypePrefix);

  GWEN_Buffer_AppendString(tbuf, "\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



