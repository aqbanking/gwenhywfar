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


#include "tm2c_typedef.h"

#include <gwenhywfar/debug.h>

#include <ctype.h>



static void _addFlagSpecificSystemHeaders(TYPEMAKER2_TYPE *ty, GWEN_BUFFER *tbuf);
static void _addPreHeaders(TYPEMAKER2_TYPE *ty, GWEN_BUFFER *tbuf);
static void _addTypeSpecificDefs(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, uint32_t flags, GWEN_BUFFER *tbuf);




/** writes typedef line into public header and also writes includes */
int TM2C_BuildTypedef(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  GWEN_BUFFER *tbuf;
  const char *s;
  uint32_t flags;

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  flags=Typemaker2_Type_GetFlags(ty);

  _addFlagSpecificSystemHeaders(ty, tbuf);
  _addPreHeaders(ty, tbuf);

  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_OBJECT)
    GWEN_Buffer_AppendString(tbuf, "#include <aqdatabase/aqdb.h>\n");

  GWEN_Buffer_AppendString(tbuf, "typedef struct ");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " ");
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, ";\n");

  _addTypeSpecificDefs(tb, ty, flags, tbuf);

  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_OBJECT) {
    GWEN_Buffer_AppendString(tbuf, "#include <aqdatabase/aqdb_db.h>\n");
  }
  GWEN_Buffer_AppendString(tbuf, "\n");

  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



void _addFlagSpecificSystemHeaders(TYPEMAKER2_TYPE *ty, GWEN_BUFFER *tbuf)
{
  uint32_t flags;

  flags=Typemaker2_Type_GetFlags(ty);

  /* add some system headers */
  GWEN_Buffer_AppendString(tbuf, "/* needed system headers */\n");
  GWEN_Buffer_AppendString(tbuf, "#include <gwenhywfar/types.h>\n");
  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_LIST1)
    GWEN_Buffer_AppendString(tbuf, "#include <gwenhywfar/list1.h>\n");
  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_LIST2)
    GWEN_Buffer_AppendString(tbuf, "#include <gwenhywfar/list2.h>\n");
  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_TREE)
    GWEN_Buffer_AppendString(tbuf, "#include <gwenhywfar/tree.h>\n");
  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_TREE2)
    GWEN_Buffer_AppendString(tbuf, "#include <gwenhywfar/tree2.h>\n");
  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_INHERIT)
    GWEN_Buffer_AppendString(tbuf, "#include <gwenhywfar/inherit.h>\n");
  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_IDMAP)
    GWEN_Buffer_AppendString(tbuf, "#include <gwenhywfar/idmap.h>\n");
  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_DB)
    GWEN_Buffer_AppendString(tbuf, "#include <gwenhywfar/db.h>\n");
  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_XML)
    GWEN_Buffer_AppendString(tbuf, "#include <gwenhywfar/xml.h>\n");
  if ((flags & TYPEMAKER2_TYPEFLAGS_WITH_SIGNALS) || (flags & TYPEMAKER2_TYPEFLAGS_WITH_SLOTS))
    GWEN_Buffer_AppendString(tbuf, "#include <gwenhywfar/gwensignal.h>\n");
  GWEN_Buffer_AppendString(tbuf, "\n");
}



void _addPreHeaders(TYPEMAKER2_TYPE *ty, GWEN_BUFFER *tbuf)
{
  TYPEMAKER2_HEADER_LIST *hl;

  /* handle pre-headers */
  hl=Typemaker2_Type_GetHeaders(ty);
  if (hl) {
    TYPEMAKER2_HEADER *h;

    h=Typemaker2_Header_List_First(hl);
    if (h) {
      GWEN_Buffer_AppendString(tbuf, "/* pre-headers */\n");
      while (h) {
        if (Typemaker2_Header_GetLocation(h)==Typemaker2_HeaderLocation_Pre) {
          GWEN_Buffer_AppendString(tbuf, "#include ");

          if (Typemaker2_Header_GetType(h)==Typemaker2_HeaderType_System) {
            GWEN_Buffer_AppendString(tbuf, "<");
            GWEN_Buffer_AppendString(tbuf, Typemaker2_Header_GetFileName(h));
            GWEN_Buffer_AppendString(tbuf, ">");
          }
          else {
            GWEN_Buffer_AppendString(tbuf, "\"");
            GWEN_Buffer_AppendString(tbuf, Typemaker2_Header_GetFileName(h));
            GWEN_Buffer_AppendString(tbuf, "\"");
          }
          GWEN_Buffer_AppendString(tbuf, "\n");
        }
        h=Typemaker2_Header_List_Next(h);
      }
      GWEN_Buffer_AppendString(tbuf, "\n");
    }
  }
}



void _addTypeSpecificDefs(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, uint32_t flags, GWEN_BUFFER *tbuf)
{
  TYPEMAKER2_TYPEMANAGER *tym;
  const char *sTypeId;
  const char *sTypePrefix;
  const char *sApi;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  sTypeId=Typemaker2_Type_GetIdentifier(ty);
  sTypePrefix=Typemaker2_Type_GetPrefix(ty);
  sApi=Typemaker2_TypeManager_GetApiDeclaration(tym);

  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_LIST1) {
    if (Typemaker2_TypeManager_GetApiDeclaration(tym))
      GWEN_Buffer_AppendArgs(tbuf, "GWEN_LIST_FUNCTION_LIB_DEFS(%s, %s, %s)\n", sTypeId, sTypePrefix, sApi);
    else
      GWEN_Buffer_AppendArgs(tbuf, "GWEN_LIST_FUNCTION_DEFS(%s, %s)\n", sTypeId, sTypePrefix);
  }

  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_LIST2) {
    if (Typemaker2_TypeManager_GetApiDeclaration(tym))
      GWEN_Buffer_AppendArgs(tbuf, "GWEN_LIST2_FUNCTION_LIB_DEFS(%s, %s, %s)\n", sTypeId, sTypePrefix, sApi);
    else
      GWEN_Buffer_AppendArgs(tbuf, "GWEN_LIST2_FUNCTION_DEFS(%s, %s)\n", sTypeId, sTypePrefix);
  }

  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_CONSTLIST2) {
    if (Typemaker2_TypeManager_GetApiDeclaration(tym))
      GWEN_Buffer_AppendArgs(tbuf, "GWEN_CONSTLIST2_FUNCTION_LIB_DEFS(%s, %s, %s)\n", sTypeId, sTypePrefix, sApi);
    else
      GWEN_Buffer_AppendArgs(tbuf, "GWEN_CONSTLIST2_FUNCTION_DEFS(%s, %s)\n", sTypeId, sTypePrefix);
  }

  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_TREE) {
    if (Typemaker2_TypeManager_GetApiDeclaration(tym))
      GWEN_Buffer_AppendArgs(tbuf, "GWEN_TREE_FUNCTION_LIB_DEFS(%s, %s, %s)\n", sTypeId, sTypePrefix, sApi);
    else
      GWEN_Buffer_AppendArgs(tbuf, "GWEN_TREE_FUNCTION_DEFS(%s, %s)\n", sTypeId, sTypePrefix);
  }

  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_TREE2) {
    if (Typemaker2_TypeManager_GetApiDeclaration(tym))
      GWEN_Buffer_AppendArgs(tbuf, "GWEN_TREE2_FUNCTION_LIB_DEFS(%s, %s, %s)\n", sTypeId, sTypePrefix, sApi);
    else
      GWEN_Buffer_AppendArgs(tbuf, "GWEN_TREE2_FUNCTION_DEFS(%s, %s)\n", sTypeId, sTypePrefix);
  }

  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_INHERIT) {
    if (Typemaker2_TypeManager_GetApiDeclaration(tym))
      GWEN_Buffer_AppendArgs(tbuf, "GWEN_INHERIT_FUNCTION_LIB_DEFS(%s, %s)\n", sTypeId, sApi);
    else
      GWEN_Buffer_AppendArgs(tbuf, "GWEN_INHERIT_FUNCTION_DEFS(%s)\n", sTypeId);
  }

  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_IDMAP) {
    if (Typemaker2_TypeManager_GetApiDeclaration(tym))
      GWEN_Buffer_AppendArgs(tbuf, "GWEN_IDMAP_FUNCTION_LIB_DEFS(%s, %s, %s)\n", sTypeId, sTypePrefix, sApi);
    else
      GWEN_Buffer_AppendArgs(tbuf, "GWEN_IDMAP_FUNCTION_DEFS(%s, %s)\n", sTypeId, sTypePrefix);
  }
  GWEN_Buffer_AppendString(tbuf, "\n");
}



