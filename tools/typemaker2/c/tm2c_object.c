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


#include "tm2c_object.h"
#include "tm2c_misc.h"

#include <gwenhywfar/debug.h>

#include <ctype.h>



int TM2C_BuildCreateColumnList(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  GWEN_BUFFER *tbuf;
  const char *s;
  TYPEMAKER2_MEMBER_LIST *tml;
  /* uint32_t flags; */
  TYPEMAKER2_TYPEMANAGER *tym;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  /* flags=Typemaker2_Type_GetFlags(ty); */

  /* prototype */
  s=Typemaker2_TypeManager_GetApiDeclaration(tym);
  if (s) {
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, " ");
  }
  GWEN_Buffer_AppendString(tbuf, "AQDB_COLUMN_LIST *");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_CreateColumnList();\n");
  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_Reset(tbuf);

  /* implementation */
  GWEN_Buffer_AppendString(tbuf, "AQDB_COLUMN_LIST *");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_CreateColumnList() {\n");

  if (Typemaker2_Type_GetNonVolatileMemberCount(ty)==0) {
    GWEN_Buffer_AppendString(tbuf, "  return NULL;\n");
  }
  else {
    GWEN_Buffer_AppendString(tbuf, "  AQDB_COLUMN_LIST *p_cl;\n");
    GWEN_Buffer_AppendString(tbuf, "  AQDB_COLUMN *p_c;\n");
    GWEN_Buffer_AppendString(tbuf, "\n");

    GWEN_Buffer_AppendString(tbuf, "  p_cl=AQDB_Column_List_new();\n");
    GWEN_Buffer_AppendString(tbuf, "\n");

    tml=Typemaker2_Type_GetMembers(ty);
    if (tml) {
      TYPEMAKER2_MEMBER *tm;

      tm=Typemaker2_Member_List_First(tml);
      while (tm) {
        TYPEMAKER2_TYPE *mty;

        mty=Typemaker2_Member_GetTypePtr(tm);
        assert(mty);

        if (!(Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_VOLATILE)) {
          char numbuf[32];
          int l;

          GWEN_Buffer_AppendString(tbuf, "  /* member \"");
          s=Typemaker2_Member_GetName(tm);
          GWEN_Buffer_AppendString(tbuf, s);
          GWEN_Buffer_AppendString(tbuf, "\" */\n");

          GWEN_Buffer_AppendString(tbuf, "  p_c=AQDB_Column_new(");
          s=Typemaker2_Type_GetAqDbType(mty);
          if (s && *s) {
            GWEN_Buffer_AppendString(tbuf, s);
          }
          else {
            DBG_ERROR(GWEN_LOGDOMAIN, "Type has no AQDB type element");
            GWEN_Buffer_free(tbuf);
            return GWEN_ERROR_BAD_DATA;
          }

          GWEN_Buffer_AppendString(tbuf, ", \"");
          s=Typemaker2_Member_GetName(tm);
          GWEN_Buffer_AppendString(tbuf, s);
          GWEN_Buffer_AppendString(tbuf, "\", ");

          l=Typemaker2_Member_GetMaxLen(tm);
          if (l==0) {
            DBG_ERROR(GWEN_LOGDOMAIN, "Member [%s] has no maxlen attribute",
                      Typemaker2_Member_GetName(tm));
            GWEN_Buffer_free(tbuf);
            return GWEN_ERROR_BAD_DATA;
          }
          snprintf(numbuf, sizeof(numbuf)-1, "%d", l);
          numbuf[sizeof(numbuf)-1]=0;
          GWEN_Buffer_AppendString(tbuf, numbuf);
          GWEN_Buffer_AppendString(tbuf, ");\n");
          GWEN_Buffer_AppendString(tbuf, "  AQDB_Column_List_Add(p_c, p_cl);\n");
        }
        else {
          GWEN_Buffer_AppendString(tbuf, "  /* member \"");
          s=Typemaker2_Member_GetName(tm);
          GWEN_Buffer_AppendString(tbuf, s);
          GWEN_Buffer_AppendString(tbuf, "\" is volatile, not adding to column list */\n");
        }
        GWEN_Buffer_AppendString(tbuf, "\n");

        tm=Typemaker2_Member_List_Next(tm);
      }
    }
    GWEN_Buffer_AppendString(tbuf, "  return p_cl;\n");
  }

  GWEN_Buffer_AppendString(tbuf, "}\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



int TM2C_BuildCreateTable(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  GWEN_BUFFER *tbuf;
  const char *s;
  /* uint32_t flags; */
  TYPEMAKER2_TYPEMANAGER *tym;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  /* flags=Typemaker2_Type_GetFlags(ty); */

  /* prototype */
  s=Typemaker2_TypeManager_GetApiDeclaration(tym);
  if (s) {
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, " ");
  }
  GWEN_Buffer_AppendString(tbuf, "int ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_CreateTable(AQDB_DB *p_db, const char *p_name, uint32_t p_flags);");
  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_Reset(tbuf);

  /* implementation */
  GWEN_Buffer_AppendString(tbuf, "int ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_CreateTable(AQDB_DB *p_db, const char *p_name, uint32_t p_flags) {\n");

  GWEN_Buffer_AppendString(tbuf, "  AQDB_COLUMN_LIST *p_cl;\n");
  GWEN_Buffer_AppendString(tbuf, "  int p_rv;\n");
  GWEN_Buffer_AppendString(tbuf, "\n");

  GWEN_Buffer_AppendString(tbuf, "  p_cl=");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_CreateColumnList();\n");

  GWEN_Buffer_AppendString(tbuf, "  p_rv=AQDB_DB_CreateTable(p_db, p_name, p_flags, p_cl);\n");
  GWEN_Buffer_AppendString(tbuf, "  AQDB_Column_List_free(p_cl);\n");
  GWEN_Buffer_AppendString(tbuf, "  if (p_rv<0) {\n");
  GWEN_Buffer_AppendString(tbuf, "    DBG_INFO(GWEN_LOGDOMAIN, \"here (%d)\", p_rv);\n");
  GWEN_Buffer_AppendString(tbuf, "    return p_rv;\n");
  GWEN_Buffer_AppendString(tbuf, "  }\n");

  GWEN_Buffer_AppendString(tbuf, "  return 0;\n");
  GWEN_Buffer_AppendString(tbuf, "}\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



int TM2C_BuildCreateObject(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  GWEN_BUFFER *tbuf;
  const char *s;
  /* uint32_t flags; */
  TYPEMAKER2_TYPEMANAGER *tym;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  /* flags=Typemaker2_Type_GetFlags(ty); */

  /* prototype */
  s=Typemaker2_TypeManager_GetApiDeclaration(tym);
  if (s) {
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, " ");
  }
  GWEN_Buffer_AppendString(tbuf, "int ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_CreateObject(AQDB_DB *p_db, AQDB_ID p_tableId, AQDB_ID p_id, AQDB_OBJECT **pp_o);\n");
  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_Reset(tbuf);

  /* implementation */
  GWEN_Buffer_AppendString(tbuf, "int ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_CreateObject(AQDB_DB *p_db, AQDB_ID p_tableId, AQDB_ID p_id, AQDB_OBJECT **pp_o) {\n");

  GWEN_Buffer_AppendString(tbuf, "  AQDB_OBJECT *p_o;\n");
  GWEN_Buffer_AppendString(tbuf, "\n");

  GWEN_Buffer_AppendString(tbuf, "  p_o=AQDB_Object_new(p_tableId, p_id, ");
  s=Typemaker2_Type_GetFieldCountId(ty);
  if (s && *s) {
  }
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, ");\n");
  GWEN_Buffer_AppendString(tbuf, "  *pp_o=p_o;\n");
  GWEN_Buffer_AppendString(tbuf, "  return 0;\n");
  GWEN_Buffer_AppendString(tbuf, "}\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



