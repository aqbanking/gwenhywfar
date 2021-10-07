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


#include "tm2c_enums.h"
#include "tm2c_misc.h"

#include <gwenhywfar/debug.h>

#include <ctype.h>




int TM2C_BuildDefineEnums(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  TYPEMAKER2_ENUM_LIST *enums;

  enums=Typemaker2_Type_GetEnums(ty);
  assert(enums);
  if (Typemaker2_Enum_List_GetCount(enums)) {
    GWEN_BUFFER *tbuf;
    const char *s;
    TYPEMAKER2_ENUM *te;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);

    te=Typemaker2_Enum_List_First(enums);
    while (te) {
      const char *prefix;
      const char *etype;
      TYPEMAKER2_ITEM *ti;
      int access=TypeMaker2_Access_Public;

      prefix=Typemaker2_Enum_GetPrefix(te);
      if (prefix==NULL || *prefix==0) {
        DBG_ERROR(GWEN_LOGDOMAIN, "No prefix in enum definition");
        GWEN_Buffer_free(tbuf);
        return GWEN_ERROR_BAD_DATA;
      }

      etype=Typemaker2_Enum_GetType(te);
      if (etype && *etype)
        GWEN_Buffer_AppendString(tbuf, "typedef enum {\n");
      else
        GWEN_Buffer_AppendString(tbuf, "enum {\n");

      GWEN_Buffer_AppendString(tbuf, "  ");
      if (prefix)
        GWEN_Buffer_AppendString(tbuf, prefix);
      GWEN_Buffer_AppendString(tbuf, "Unknown = -1");
      ti=Typemaker2_Item_List_First(Typemaker2_Enum_GetItems(te));
      if (ti)
        GWEN_Buffer_AppendString(tbuf, ",");
      GWEN_Buffer_AppendString(tbuf, "\n");
      while (ti) {
        GWEN_Buffer_AppendString(tbuf, "  ");
        if (prefix)
          GWEN_Buffer_AppendString(tbuf, prefix);
        s=Typemaker2_Item_GetName(ti);
        if (s && *s) {
          GWEN_Buffer_AppendByte(tbuf, toupper(*s));
          GWEN_Buffer_AppendString(tbuf, s+1);
        }
        s=Typemaker2_Item_GetValue(ti);
        if (s) {
          GWEN_Buffer_AppendString(tbuf, " = ");
          GWEN_Buffer_AppendString(tbuf, s);
        }

        ti=Typemaker2_Item_List_Next(ti);
        if (ti)
          GWEN_Buffer_AppendString(tbuf, ",");
        GWEN_Buffer_AppendString(tbuf, "\n");
      }
      GWEN_Buffer_AppendString(tbuf, "}");

      if (etype && *etype) {
        GWEN_Buffer_AppendString(tbuf, " ");
        GWEN_Buffer_AppendString(tbuf, etype);
      }

      GWEN_Buffer_AppendString(tbuf, ";\n");
      GWEN_Buffer_AppendString(tbuf, "\n");

      switch (access) {
      case TypeMaker2_Access_Public:
        Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
        break;
      case TypeMaker2_Access_Library:
        Typemaker2_Builder_AddLibraryDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
        break;
      case TypeMaker2_Access_Protected:
        Typemaker2_Builder_AddProtectedDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
        break;
      case TypeMaker2_Access_Private:
        Typemaker2_Builder_AddPrivateDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
        break;
      default:
        DBG_ERROR(GWEN_LOGDOMAIN, "Invalid access type");
        GWEN_Buffer_free(tbuf);
        return GWEN_ERROR_BAD_DATA;
      }
      GWEN_Buffer_Reset(tbuf);

      te=Typemaker2_Enum_List_Next(te);
    }
  }

  return 0;
}




int TM2C_SetEnumStringFns(TYPEMAKER2_TYPE *ty)
{
  TYPEMAKER2_ENUM_LIST *enums;
  /* TYPEMAKER2_TYPEMANAGER *tym; */
  /* uint32_t flags; */

  /* tym=Typemaker2_Builder_GetTypeManager(tb); */
  enums=Typemaker2_Type_GetEnums(ty);
  /* flags=Typemaker2_Type_GetFlags(ty); */

  assert(enums);
  if (Typemaker2_Enum_List_GetCount(enums)) {
    TYPEMAKER2_ENUM *te;

    te=Typemaker2_Enum_List_First(enums);
    while (te) {
      const char *prefix;
      GWEN_BUFFER *tbuf;

      prefix=Typemaker2_Enum_GetPrefix(te);
      if (prefix==NULL || *prefix==0) {
        DBG_ERROR(GWEN_LOGDOMAIN, "No prefix in enum definition");
        return GWEN_ERROR_BAD_DATA;
      }

      tbuf=GWEN_Buffer_new(0, 256, 0, 1);
      if (prefix && *prefix) {
        GWEN_Buffer_AppendString(tbuf, prefix);
        if (prefix[strlen(prefix)-1]!='_')
          GWEN_Buffer_AppendString(tbuf, "_");
      }
      GWEN_Buffer_AppendString(tbuf, "fromString");
      Typemaker2_Enum_SetFromStringFn(te, GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_Reset(tbuf);

      if (prefix && *prefix) {
        GWEN_Buffer_AppendString(tbuf, prefix);
        if (prefix[strlen(prefix)-1]!='_')
          GWEN_Buffer_AppendString(tbuf, "_");
      }
      GWEN_Buffer_AppendString(tbuf, "toString");
      Typemaker2_Enum_SetToStringFn(te, GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_free(tbuf);

      te=Typemaker2_Enum_List_Next(te);
    }
  }

  return 0;
}



int TM2C_BuildEnumFromString(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  TYPEMAKER2_ENUM_LIST *enums;
  TYPEMAKER2_TYPEMANAGER *tym;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  enums=Typemaker2_Type_GetEnums(ty);

  assert(enums);
  if (Typemaker2_Enum_List_GetCount(enums)) {
    GWEN_BUFFER *tbuf;
    const char *s;
    TYPEMAKER2_ENUM *te;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);

    te=Typemaker2_Enum_List_First(enums);
    while (te) {
      const char *prefix;
      const char *etype;
      TYPEMAKER2_ITEM *ti;
      int acc=TypeMaker2_Access_Public;
      int i;

      prefix=Typemaker2_Enum_GetPrefix(te);
      if (prefix==NULL || *prefix==0) {
        DBG_ERROR(GWEN_LOGDOMAIN, "No prefix in enum definition");
        GWEN_Buffer_free(tbuf);
        return GWEN_ERROR_BAD_DATA;
      }

      etype=Typemaker2_Enum_GetType(te);

      /* prototype */
      s=Typemaker2_TypeManager_GetApiDeclaration(tym);
      if (s && *s)
        GWEN_Buffer_AppendArgs(tbuf, "%s ", s);

      GWEN_Buffer_AppendArgs(tbuf, "%s ", (etype && *etype)?etype:"int");
      if (prefix && *prefix) {
        GWEN_Buffer_AppendString(tbuf, prefix);
        if (prefix[strlen(prefix)-1]!='_')
          GWEN_Buffer_AppendString(tbuf, "_");
      }
      GWEN_Buffer_AppendString(tbuf, "fromString(const char *p_s);\n");
      switch (acc) {
      case TypeMaker2_Access_Public:
        Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
        break;
      case TypeMaker2_Access_Library:
        Typemaker2_Builder_AddLibraryDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
        break;
      case TypeMaker2_Access_Protected:
        Typemaker2_Builder_AddProtectedDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
        break;
      case TypeMaker2_Access_Private:
        Typemaker2_Builder_AddPrivateDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
        break;
      default:
        DBG_ERROR(GWEN_LOGDOMAIN, "Invalid access type");
        GWEN_Buffer_free(tbuf);
        return GWEN_ERROR_BAD_DATA;
      }
      GWEN_Buffer_Reset(tbuf);

      /* implementation */
      if (etype && *etype)
        GWEN_Buffer_AppendArgs(tbuf, "%s ", etype);
      else
        GWEN_Buffer_AppendString(tbuf, "int ");
      if (prefix && *prefix) {
        GWEN_Buffer_AppendString(tbuf, prefix);
        if (prefix[strlen(prefix)-1]!='_')
          GWEN_Buffer_AppendString(tbuf, "_");
      }
      GWEN_Buffer_AppendString(tbuf, "fromString(const char *p_s) {\n");

      GWEN_Buffer_AppendString(tbuf, "  if (p_s && *p_s) {\n");

      i=0;
      ti=Typemaker2_Item_List_First(Typemaker2_Enum_GetItems(te));
      while (ti) {
        const char *sItemName;

        sItemName=Typemaker2_Item_GetName(ti);
        GWEN_Buffer_AppendString(tbuf, "    ");
        if (i++)
          GWEN_Buffer_AppendString(tbuf, "else ");

        GWEN_Buffer_AppendArgs(tbuf, "if (strcasecmp(p_s, \"%s\")==0)\n", sItemName);
        GWEN_Buffer_AppendArgs(tbuf, "      return %s%c%s;\n", prefix, toupper(*sItemName), sItemName+1);

        ti=Typemaker2_Item_List_Next(ti);
      }
      GWEN_Buffer_AppendString(tbuf, "  }\n");
      GWEN_Buffer_AppendArgs(tbuf, "  return %sUnknown;\n", prefix);
      GWEN_Buffer_AppendString(tbuf, "}\n");

      Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_Reset(tbuf);
      te=Typemaker2_Enum_List_Next(te);
    }
    GWEN_Buffer_free(tbuf);
  } /* if enums */

  return 0;
}



int TM2C_BuildEnumToString(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  TYPEMAKER2_ENUM_LIST *enums;
  TYPEMAKER2_TYPEMANAGER *tym;
  /* uint32_t flags; */

  tym=Typemaker2_Builder_GetTypeManager(tb);
  enums=Typemaker2_Type_GetEnums(ty);
  /* flags=Typemaker2_Type_GetFlags(ty); */

  assert(enums);
  if (Typemaker2_Enum_List_GetCount(enums)) {
    GWEN_BUFFER *tbuf;
    const char *s;
    TYPEMAKER2_ENUM *te;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);

    te=Typemaker2_Enum_List_First(enums);
    while (te) {
      const char *prefix;
      const char *etype;
      TYPEMAKER2_ITEM *ti;
      int acc=TypeMaker2_Access_Public;

      prefix=Typemaker2_Enum_GetPrefix(te);
      if (prefix==NULL || *prefix==0) {
        DBG_ERROR(GWEN_LOGDOMAIN, "No prefix in enum definition");
        GWEN_Buffer_free(tbuf);
        return GWEN_ERROR_BAD_DATA;
      }

      etype=Typemaker2_Enum_GetType(te);

      /* prototype */
      s=Typemaker2_TypeManager_GetApiDeclaration(tym);
      if (s) {
        GWEN_Buffer_AppendString(tbuf, s);
        GWEN_Buffer_AppendString(tbuf, " ");
      }
      GWEN_Buffer_AppendString(tbuf, "const char *");
      if (prefix && *prefix) {
        GWEN_Buffer_AppendString(tbuf, prefix);
        if (prefix[strlen(prefix)-1]!='_')
          GWEN_Buffer_AppendString(tbuf, "_");
      }
      GWEN_Buffer_AppendString(tbuf, "toString(");
      if (etype && *etype)
        GWEN_Buffer_AppendString(tbuf, etype);
      else
        GWEN_Buffer_AppendString(tbuf, "int");
      GWEN_Buffer_AppendString(tbuf, " p_i);\n");

      switch (acc) {
      case TypeMaker2_Access_Public:
        Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
        break;
      case TypeMaker2_Access_Library:
        Typemaker2_Builder_AddLibraryDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
        break;
      case TypeMaker2_Access_Protected:
        Typemaker2_Builder_AddProtectedDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
        break;
      case TypeMaker2_Access_Private:
        Typemaker2_Builder_AddPrivateDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
        break;
      default:
        DBG_ERROR(GWEN_LOGDOMAIN, "Invalid access type");
        GWEN_Buffer_free(tbuf);
        return GWEN_ERROR_BAD_DATA;
      }
      GWEN_Buffer_Reset(tbuf);

      /* implementation */
      GWEN_Buffer_AppendString(tbuf, "const char *");
      if (prefix && *prefix) {
        GWEN_Buffer_AppendString(tbuf, prefix);
        if (prefix[strlen(prefix)-1]!='_')
          GWEN_Buffer_AppendString(tbuf, "_");
      }
      GWEN_Buffer_AppendString(tbuf, "toString(");
      if (etype && *etype)
        GWEN_Buffer_AppendString(tbuf, etype);
      else
        GWEN_Buffer_AppendString(tbuf, "int");
      GWEN_Buffer_AppendString(tbuf, " p_i) {\n");

      GWEN_Buffer_AppendString(tbuf, "  switch(p_i) {\n");
      ti=Typemaker2_Item_List_First(Typemaker2_Enum_GetItems(te));
      while (ti) {
        GWEN_Buffer_AppendString(tbuf, "    case ");
        if (prefix)
          GWEN_Buffer_AppendString(tbuf, prefix);
        s=Typemaker2_Item_GetName(ti);
        if (s && *s) {
          GWEN_Buffer_AppendByte(tbuf, toupper(*s));
          GWEN_Buffer_AppendString(tbuf, s+1);
        }
        GWEN_Buffer_AppendString(tbuf, ": return \"");
        s=Typemaker2_Item_GetName(ti);
        if (s && *s) {
          GWEN_Buffer_AppendByte(tbuf, tolower(*s));
          GWEN_Buffer_AppendString(tbuf, s+1);
        }
        GWEN_Buffer_AppendString(tbuf, "\";\n");

        ti=Typemaker2_Item_List_Next(ti);
      }

      /* handle "unknown" */
      GWEN_Buffer_AppendString(tbuf, "    case ");
      if (prefix)
        GWEN_Buffer_AppendString(tbuf, prefix);
      GWEN_Buffer_AppendString(tbuf, "Unknown:\n");

      /* handle default */
      GWEN_Buffer_AppendString(tbuf, "    default: return \"unknown\";\n");
      GWEN_Buffer_AppendString(tbuf, "  }\n");
      GWEN_Buffer_AppendString(tbuf, "}\n");

      Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_Reset(tbuf);
      te=Typemaker2_Enum_List_Next(te);
    }
    GWEN_Buffer_free(tbuf);
  } /* if enums */

  return 0;
}



