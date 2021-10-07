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


#include "tm2c_virtualfn.h"
#include "tm2c_misc.h"

#include <gwenhywfar/debug.h>

#include <ctype.h>



int TM2C_BuildDefineVirtualFns(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, const char *loc)
{
  TYPEMAKER2_VIRTUALFN_LIST *fns;

  fns=Typemaker2_Type_GetVirtualFns(ty);

  assert(fns);
  if (Typemaker2_VirtualFn_List_GetCount(fns)) {
    GWEN_BUFFER *tbuf;
    const char *s;
    TYPEMAKER2_VIRTUALFN *vf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);

    if (loc && *loc)
      GWEN_Buffer_AppendArgs(tbuf, "/* definitions for virtual functions (%s) */\n", loc);
    else
      GWEN_Buffer_AppendString(tbuf, "/* definitions for virtual functions */\n");

    vf=Typemaker2_VirtualFn_List_First(fns);
    while (vf) {
      s=Typemaker2_VirtualFn_GetLocation(vf);
      if (!(s && *s))
        s="pre";
      if (s && loc && strcasecmp(s, loc)==0) {
        int access=Typemaker2_VirtualFn_GetAccess(vf);
        TYPEMAKER2_VIRTUALFN_PARAM_LIST *plist;
        int i;

        GWEN_Buffer_AppendString(tbuf, "typedef ");
        s=Typemaker2_VirtualFn_GetReturnType(vf);
        if (!(s && *s)) {
          GWEN_Buffer_free(tbuf);
          return GWEN_ERROR_BAD_DATA;
        }
        GWEN_Buffer_AppendString(tbuf, s);

        if (Typemaker2_VirtualFn_GetFlags(vf) & TYPEMAKER2_FLAGS_GWEN_CB) {
          GWEN_Buffer_AppendString(tbuf, " GWENHYWFAR_CB");
        }

        GWEN_Buffer_AppendArgs(tbuf, "  (*%s_", Typemaker2_Type_GetIdentifier(ty));

        s=Typemaker2_VirtualFn_GetName(vf);
        if (!(s && *s)) {
          GWEN_Buffer_free(tbuf);
          return GWEN_ERROR_BAD_DATA;
        }
        while (*s) {
          GWEN_Buffer_AppendByte(tbuf, toupper(*s));
          s++;
        }
        GWEN_Buffer_AppendString(tbuf, "_FN)(");

        if (Typemaker2_VirtualFn_GetFlags(vf) & TYPEMAKER2_FLAGS_CONST)
          GWEN_Buffer_AppendString(tbuf, "const ");

        s=Typemaker2_Type_GetIdentifier(ty);
        GWEN_Buffer_AppendString(tbuf, s);
        /* created structs are always pointers */
        GWEN_Buffer_AppendString(tbuf, " *p_struct");

        plist=Typemaker2_VirtualFn_GetParamTypeList(vf);
        if (plist) {
          TYPEMAKER2_VIRTUALFN_PARAM *prm;

          i=1;
          prm=Typemaker2_VirtualFn_Param_List_First(plist);
          while (prm) {
            const char *paramName;
            const char *paramType;

            GWEN_Buffer_AppendString(tbuf, ", ");

            paramName=Typemaker2_VirtualFn_Param_GetName(prm);
            paramType=Typemaker2_VirtualFn_Param_GetType(prm);

            GWEN_Buffer_AppendString(tbuf, paramType);
            GWEN_Buffer_AppendString(tbuf, " ");

            if (paramName)
              GWEN_Buffer_AppendString(tbuf, paramName);
            else
              GWEN_Buffer_AppendArgs(tbuf, "param%d", i);

            prm=Typemaker2_VirtualFn_Param_List_Next(prm);
            i++;
          }
        }

        GWEN_Buffer_AppendString(tbuf, ");\n");
        Typemaker2_Builder_AddDeclaration(tb, access, GWEN_Buffer_GetStart(tbuf));
        GWEN_Buffer_Reset(tbuf);
      }

      vf=Typemaker2_VirtualFn_List_Next(vf);
    }
    GWEN_Buffer_free(tbuf);
  }

  return 0;
}



int TM2C_BuildProtoVirtualFns(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  TYPEMAKER2_VIRTUALFN_LIST *fns;
  TYPEMAKER2_TYPEMANAGER *tym;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  fns=Typemaker2_Type_GetVirtualFns(ty);

  assert(fns);
  if (Typemaker2_VirtualFn_List_GetCount(fns)) {
    GWEN_BUFFER *tbuf;
    const char *s;
    TYPEMAKER2_VIRTUALFN *vf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);

    GWEN_Buffer_AppendString(tbuf, "/* prototypes for virtual functions */\n");
    vf=Typemaker2_VirtualFn_List_First(fns);
    while (vf) {
      int access=Typemaker2_VirtualFn_GetAccess(vf);
      TYPEMAKER2_VIRTUALFN_PARAM_LIST *plist;
      int i;

      /* write APIDOC */
      GWEN_Buffer_AppendString(tbuf, "/**\n");
      s=Typemaker2_VirtualFn_GetDescr(vf);
      if (s && *s) {
        int len;

        GWEN_Buffer_AppendString(tbuf, " * ");
        GWEN_Buffer_AppendString(tbuf, s);
        len=strlen(s);
        if (s[len-1]!='\n')
          GWEN_Buffer_AppendString(tbuf, "\n");
      }

      s=Typemaker2_VirtualFn_GetReturnTypeDescr(vf);
      if (s && *s) {
        int len;

        GWEN_Buffer_AppendString(tbuf, " * @return ");
        GWEN_Buffer_AppendString(tbuf, s);
        len=strlen(s);
        if (s[len-1]!='\n')
          GWEN_Buffer_AppendString(tbuf, "\n");
      }


      i=1;
      plist=Typemaker2_VirtualFn_GetParamTypeList(vf);
      if (plist) {
        TYPEMAKER2_VIRTUALFN_PARAM *prm;

        i=1;
        prm=Typemaker2_VirtualFn_Param_List_First(plist);
        while (prm) {
          const char *paramDescr;

          paramDescr=Typemaker2_VirtualFn_Param_GetDescr(prm);
          if (paramDescr && *paramDescr) {
            const char *paramName;
            int len;

            GWEN_Buffer_AppendString(tbuf, " * @param ");
            paramName=Typemaker2_VirtualFn_Param_GetName(prm);
            if (paramName)
              GWEN_Buffer_AppendString(tbuf, paramName);
            else {
              char numbuf[64];

              snprintf(numbuf, sizeof(numbuf)-1, "param%d", i);
              numbuf[sizeof(numbuf)-1]=0;
              GWEN_Buffer_AppendString(tbuf, numbuf);
            }

            GWEN_Buffer_AppendString(tbuf, " ");
            GWEN_Buffer_AppendString(tbuf, paramDescr);
            len=strlen(paramDescr);
            if (paramDescr[len-1]!='\n')
              GWEN_Buffer_AppendString(tbuf, "\n");
          }

          i++;
          prm=Typemaker2_VirtualFn_Param_List_Next(prm);
        }
      }
      GWEN_Buffer_AppendString(tbuf, " */\n");


      s=Typemaker2_TypeManager_GetApiDeclaration(tym);
      if (s && *s) {
        GWEN_Buffer_AppendString(tbuf, s);
        GWEN_Buffer_AppendString(tbuf, " ");
      }

      s=Typemaker2_VirtualFn_GetReturnType(vf);
      if (!(s && *s)) {
        GWEN_Buffer_free(tbuf);
        return GWEN_ERROR_BAD_DATA;
      }
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, " ");

      s=Typemaker2_Type_GetPrefix(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "_");
      s=Typemaker2_VirtualFn_GetName(vf);
      if (!(s && *s)) {
        GWEN_Buffer_free(tbuf);
        return GWEN_ERROR_BAD_DATA;
      }
      GWEN_Buffer_AppendByte(tbuf, toupper(*s));
      GWEN_Buffer_AppendString(tbuf, s+1);
      GWEN_Buffer_AppendString(tbuf, "(");

      if (Typemaker2_VirtualFn_GetFlags(vf) & TYPEMAKER2_FLAGS_CONST)
        GWEN_Buffer_AppendString(tbuf, "const ");

      s=Typemaker2_Type_GetIdentifier(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      /* created structs are always pointers */
      GWEN_Buffer_AppendString(tbuf, " *p_struct");


      i=1;
      plist=Typemaker2_VirtualFn_GetParamTypeList(vf);
      if (plist) {
        TYPEMAKER2_VIRTUALFN_PARAM *prm;

        i=1;
        prm=Typemaker2_VirtualFn_Param_List_First(plist);
        while (prm) {
          const char *paramName;
          const char *paramType;

          GWEN_Buffer_AppendString(tbuf, ", ");

          paramName=Typemaker2_VirtualFn_Param_GetName(prm);
          paramType=Typemaker2_VirtualFn_Param_GetType(prm);

          GWEN_Buffer_AppendString(tbuf, paramType);
          GWEN_Buffer_AppendString(tbuf, " ");

          if (paramName)
            GWEN_Buffer_AppendString(tbuf, paramName);
          else {
            char numbuf[64];

            snprintf(numbuf, sizeof(numbuf)-1, "param%d", i);
            numbuf[sizeof(numbuf)-1]=0;
            GWEN_Buffer_AppendString(tbuf, numbuf);
          }

          prm=Typemaker2_VirtualFn_Param_List_Next(prm);
          i++;
        }
      }
      GWEN_Buffer_AppendString(tbuf, ");\n");
      Typemaker2_Builder_AddDeclaration(tb, access, GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_Reset(tbuf);

      vf=Typemaker2_VirtualFn_List_Next(vf);
    }
    GWEN_Buffer_free(tbuf);
  }

  return 0;
}



int TM2C_BuildCodeVirtualFns(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  TYPEMAKER2_VIRTUALFN_LIST *fns;

  fns=Typemaker2_Type_GetVirtualFns(ty);

  assert(fns);
  if (Typemaker2_VirtualFn_List_GetCount(fns)) {
    GWEN_BUFFER *tbuf;
    const char *s;
    TYPEMAKER2_VIRTUALFN *vf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);

    GWEN_Buffer_AppendString(tbuf, "/* code for virtual functions */\n");
    vf=Typemaker2_VirtualFn_List_First(fns);
    while (vf) {
      TYPEMAKER2_VIRTUALFN_PARAM_LIST *plist;
      int i;

      s=Typemaker2_VirtualFn_GetReturnType(vf);
      if (!(s && *s)) {
        GWEN_Buffer_free(tbuf);
        return GWEN_ERROR_BAD_DATA;
      }
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, " ");

      s=Typemaker2_Type_GetPrefix(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "_");
      s=Typemaker2_VirtualFn_GetName(vf);
      if (!(s && *s)) {
        GWEN_Buffer_free(tbuf);
        return GWEN_ERROR_BAD_DATA;
      }
      GWEN_Buffer_AppendByte(tbuf, toupper(*s));
      GWEN_Buffer_AppendString(tbuf, s+1);
      GWEN_Buffer_AppendString(tbuf, "(");

      if (Typemaker2_VirtualFn_GetFlags(vf) & TYPEMAKER2_FLAGS_CONST)
        GWEN_Buffer_AppendString(tbuf, "const ");

      s=Typemaker2_Type_GetIdentifier(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      /* created structs are always pointers */
      GWEN_Buffer_AppendString(tbuf, " *p_struct");


      i=1;
      plist=Typemaker2_VirtualFn_GetParamTypeList(vf);
      if (plist) {
        TYPEMAKER2_VIRTUALFN_PARAM *prm;

        i=1;
        prm=Typemaker2_VirtualFn_Param_List_First(plist);
        while (prm) {
          const char *paramName;
          const char *paramType;

          GWEN_Buffer_AppendString(tbuf, ", ");

          paramName=Typemaker2_VirtualFn_Param_GetName(prm);
          paramType=Typemaker2_VirtualFn_Param_GetType(prm);

          GWEN_Buffer_AppendString(tbuf, paramType);
          GWEN_Buffer_AppendString(tbuf, " ");

          if (paramName)
            GWEN_Buffer_AppendString(tbuf, paramName);
          else {
            char numbuf[64];

            snprintf(numbuf, sizeof(numbuf)-1, "param%d", i);
            numbuf[sizeof(numbuf)-1]=0;
            GWEN_Buffer_AppendString(tbuf, numbuf);
          }

          prm=Typemaker2_VirtualFn_Param_List_Next(prm);
          i++;
        }
      }
      GWEN_Buffer_AppendString(tbuf, ") {\n");

      GWEN_Buffer_AppendString(tbuf, "  assert(p_struct);\n");
      GWEN_Buffer_AppendString(tbuf, "  if (p_struct->");
      s=Typemaker2_VirtualFn_GetName(vf);
      if (!(s && *s)) {
        DBG_ERROR(0, "Virtual functions has no name");
        GWEN_Buffer_free(tbuf);
        return GWEN_ERROR_BAD_DATA;
      }
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "Fn)\n");
      GWEN_Buffer_AppendString(tbuf, "    return p_struct->");
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "Fn(p_struct");

      i=1;
      plist=Typemaker2_VirtualFn_GetParamTypeList(vf);
      if (plist) {
        TYPEMAKER2_VIRTUALFN_PARAM *prm;

        i=1;
        prm=Typemaker2_VirtualFn_Param_List_First(plist);
        while (prm) {
          const char *paramName;

          GWEN_Buffer_AppendString(tbuf, ", ");

          paramName=Typemaker2_VirtualFn_Param_GetName(prm);

          if (paramName)
            GWEN_Buffer_AppendString(tbuf, paramName);
          else {
            char numbuf[64];

            snprintf(numbuf, sizeof(numbuf)-1, "param%d", i);
            numbuf[sizeof(numbuf)-1]=0;
            GWEN_Buffer_AppendString(tbuf, numbuf);
          }

          prm=Typemaker2_VirtualFn_Param_List_Next(prm);
          i++;
        }
      }
      GWEN_Buffer_AppendString(tbuf, ");\n");

      GWEN_Buffer_AppendString(tbuf, "  else\n");
      GWEN_Buffer_AppendString(tbuf, "    return ");
      s=Typemaker2_VirtualFn_GetDefaultReturnValue(vf);
      if (!(s && *s)) {
        DBG_ERROR(0, "No default return value");
        GWEN_Buffer_free(tbuf);
        return GWEN_ERROR_BAD_DATA;
      }
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, ";\n");
      GWEN_Buffer_AppendString(tbuf, "}\n");

      Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_Reset(tbuf);

      vf=Typemaker2_VirtualFn_List_Next(vf);
    }
    GWEN_Buffer_free(tbuf);
  }

  return 0;
}



int TM2C_BuildProtoSetterVirtualFns(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  TYPEMAKER2_VIRTUALFN_LIST *fns;
  TYPEMAKER2_TYPEMANAGER *tym;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  fns=Typemaker2_Type_GetVirtualFns(ty);

  assert(fns);
  if (Typemaker2_VirtualFn_List_GetCount(fns)) {
    GWEN_BUFFER *tbuf;
    const char *s;
    TYPEMAKER2_VIRTUALFN *vf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);

    GWEN_Buffer_AppendString(tbuf, "/* setters for virtual functions */\n");
    vf=Typemaker2_VirtualFn_List_First(fns);
    while (vf) {
      int access=Typemaker2_VirtualFn_GetAccess(vf);

      s=Typemaker2_TypeManager_GetApiDeclaration(tym);
      if (s && *s) {
        GWEN_Buffer_AppendString(tbuf, s);
        GWEN_Buffer_AppendString(tbuf, " ");
      }

      s=Typemaker2_Type_GetIdentifier(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "_");
      s=Typemaker2_VirtualFn_GetName(vf);
      if (!(s && *s)) {
        GWEN_Buffer_free(tbuf);
        return GWEN_ERROR_BAD_DATA;
      }
      while (*s)
        GWEN_Buffer_AppendByte(tbuf, toupper(*(s++)));
      GWEN_Buffer_AppendString(tbuf, "_FN ");

      s=Typemaker2_Type_GetPrefix(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "_Set");
      s=Typemaker2_VirtualFn_GetName(vf);
      if (!(s && *s)) {
        GWEN_Buffer_free(tbuf);
        return GWEN_ERROR_BAD_DATA;
      }
      GWEN_Buffer_AppendByte(tbuf, toupper(*s));
      GWEN_Buffer_AppendString(tbuf, s+1);
      GWEN_Buffer_AppendString(tbuf, "Fn(");

      s=Typemaker2_Type_GetIdentifier(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      /* created structs are always pointers */
      GWEN_Buffer_AppendString(tbuf, " *p_struct, ");

      s=Typemaker2_Type_GetIdentifier(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "_");
      s=Typemaker2_VirtualFn_GetName(vf);
      if (!(s && *s)) {
        GWEN_Buffer_free(tbuf);
        return GWEN_ERROR_BAD_DATA;
      }
      while (*s)
        GWEN_Buffer_AppendByte(tbuf, toupper(*(s++)));
      GWEN_Buffer_AppendString(tbuf, "_FN fn);\n");

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

      vf=Typemaker2_VirtualFn_List_Next(vf);
    }
    GWEN_Buffer_free(tbuf);
  }

  return 0;
}



int TM2C_BuildSetterVirtualFns(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  TYPEMAKER2_VIRTUALFN_LIST *fns;

  fns=Typemaker2_Type_GetVirtualFns(ty);

  assert(fns);
  if (Typemaker2_VirtualFn_List_GetCount(fns)) {
    GWEN_BUFFER *tbuf;
    const char *s;
    TYPEMAKER2_VIRTUALFN *vf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);

    GWEN_Buffer_AppendString(tbuf, "/* setters for virtual functions */\n");
    vf=Typemaker2_VirtualFn_List_First(fns);
    while (vf) {

      s=Typemaker2_Type_GetIdentifier(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "_");

      s=Typemaker2_VirtualFn_GetName(vf);
      if (!(s && *s)) {
        GWEN_Buffer_free(tbuf);
        return GWEN_ERROR_BAD_DATA;
      }
      while (*s)
        GWEN_Buffer_AppendByte(tbuf, toupper(*(s++)));
      GWEN_Buffer_AppendString(tbuf, "_FN ");

      s=Typemaker2_Type_GetPrefix(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "_Set");
      s=Typemaker2_VirtualFn_GetName(vf);
      if (!(s && *s)) {
        GWEN_Buffer_free(tbuf);
        return GWEN_ERROR_BAD_DATA;
      }
      GWEN_Buffer_AppendByte(tbuf, toupper(*s));
      GWEN_Buffer_AppendString(tbuf, s+1);
      GWEN_Buffer_AppendString(tbuf, "Fn(");

      s=Typemaker2_Type_GetIdentifier(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      /* created structs are always pointers */
      GWEN_Buffer_AppendString(tbuf, " *p_struct, ");

      s=Typemaker2_Type_GetIdentifier(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "_");
      s=Typemaker2_VirtualFn_GetName(vf);
      if (!(s && *s)) {
        GWEN_Buffer_free(tbuf);
        return GWEN_ERROR_BAD_DATA;
      }
      while (*s)
        GWEN_Buffer_AppendByte(tbuf, toupper(*(s++)));
      GWEN_Buffer_AppendString(tbuf, "_FN fn){\n");

      GWEN_Buffer_AppendString(tbuf, "  ");
      s=Typemaker2_Type_GetIdentifier(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "_");
      s=Typemaker2_VirtualFn_GetName(vf);
      if (!(s && *s)) {
        GWEN_Buffer_free(tbuf);
        return GWEN_ERROR_BAD_DATA;
      }
      while (*s)
        GWEN_Buffer_AppendByte(tbuf, toupper(*(s++)));
      GWEN_Buffer_AppendString(tbuf, "_FN oldFn;\n\n");

      GWEN_Buffer_AppendString(tbuf, "  assert(p_struct);\n");
      GWEN_Buffer_AppendString(tbuf, "  oldFn=p_struct->");
      s=Typemaker2_VirtualFn_GetName(vf);
      if (!(s && *s)) {
        GWEN_Buffer_free(tbuf);
        return GWEN_ERROR_BAD_DATA;
      }
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "Fn;\n");

      GWEN_Buffer_AppendString(tbuf, "  p_struct->");
      s=Typemaker2_VirtualFn_GetName(vf);
      if (!(s && *s)) {
        GWEN_Buffer_free(tbuf);
        return GWEN_ERROR_BAD_DATA;
      }
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "Fn=fn;\n");
      GWEN_Buffer_AppendString(tbuf, "  return oldFn;\n");
      GWEN_Buffer_AppendString(tbuf, "}\n");

      Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_Reset(tbuf);

      vf=Typemaker2_VirtualFn_List_Next(vf);
    }
    GWEN_Buffer_free(tbuf);
  }

  return 0;
}



int TM2C_BuildProtoGetterVirtualFns(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  TYPEMAKER2_VIRTUALFN_LIST *fns;
  TYPEMAKER2_TYPEMANAGER *tym;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  fns=Typemaker2_Type_GetVirtualFns(ty);

  assert(fns);
  if (Typemaker2_VirtualFn_List_GetCount(fns)) {
    GWEN_BUFFER *tbuf;
    const char *s;
    TYPEMAKER2_VIRTUALFN *vf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);

    GWEN_Buffer_AppendString(tbuf, "/* getters for virtual functions */\n");
    vf=Typemaker2_VirtualFn_List_First(fns);
    while (vf) {
      int access=Typemaker2_VirtualFn_GetAccess(vf);

      s=Typemaker2_TypeManager_GetApiDeclaration(tym);
      if (s && *s) {
        GWEN_Buffer_AppendString(tbuf, s);
        GWEN_Buffer_AppendString(tbuf, " ");
      }

      s=Typemaker2_Type_GetIdentifier(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "_");
      s=Typemaker2_VirtualFn_GetName(vf);
      if (!(s && *s)) {
        GWEN_Buffer_free(tbuf);
        return GWEN_ERROR_BAD_DATA;
      }
      while (*s)
        GWEN_Buffer_AppendByte(tbuf, toupper(*(s++)));
      GWEN_Buffer_AppendString(tbuf, "_FN ");

      s=Typemaker2_Type_GetPrefix(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "_Get");
      s=Typemaker2_VirtualFn_GetName(vf);
      if (!(s && *s)) {
        GWEN_Buffer_free(tbuf);
        return GWEN_ERROR_BAD_DATA;
      }
      GWEN_Buffer_AppendByte(tbuf, toupper(*s));
      GWEN_Buffer_AppendString(tbuf, s+1);
      GWEN_Buffer_AppendString(tbuf, "Fn(const ");

      s=Typemaker2_Type_GetIdentifier(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      /* created structs are always pointers */
      GWEN_Buffer_AppendString(tbuf, " *p_struct);\n");

      Typemaker2_Builder_AddDeclaration(tb, access, GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_Reset(tbuf);

      vf=Typemaker2_VirtualFn_List_Next(vf);
    }
    GWEN_Buffer_free(tbuf);
  }

  return 0;
}



int TM2C_BuildGetterVirtualFns(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  TYPEMAKER2_VIRTUALFN_LIST *fns;

  fns=Typemaker2_Type_GetVirtualFns(ty);

  assert(fns);
  if (Typemaker2_VirtualFn_List_GetCount(fns)) {
    GWEN_BUFFER *tbuf;
    const char *s;
    TYPEMAKER2_VIRTUALFN *vf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);

    GWEN_Buffer_AppendString(tbuf, "/* getters for virtual functions */\n");
    vf=Typemaker2_VirtualFn_List_First(fns);
    while (vf) {

      s=Typemaker2_Type_GetIdentifier(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "_");

      s=Typemaker2_VirtualFn_GetName(vf);
      if (!(s && *s)) {
        GWEN_Buffer_free(tbuf);
        return GWEN_ERROR_BAD_DATA;
      }
      while (*s)
        GWEN_Buffer_AppendByte(tbuf, toupper(*(s++)));
      GWEN_Buffer_AppendString(tbuf, "_FN ");

      s=Typemaker2_Type_GetPrefix(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "_Get");
      s=Typemaker2_VirtualFn_GetName(vf);
      if (!(s && *s)) {
        GWEN_Buffer_free(tbuf);
        return GWEN_ERROR_BAD_DATA;
      }
      GWEN_Buffer_AppendByte(tbuf, toupper(*s));
      GWEN_Buffer_AppendString(tbuf, s+1);
      GWEN_Buffer_AppendString(tbuf, "Fn(const ");

      s=Typemaker2_Type_GetIdentifier(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      /* created structs are always pointers */
      GWEN_Buffer_AppendString(tbuf, " *p_struct){\n");

      GWEN_Buffer_AppendString(tbuf, "  assert(p_struct);\n");
      GWEN_Buffer_AppendString(tbuf, "  return p_struct->");
      s=Typemaker2_VirtualFn_GetName(vf);
      if (!(s && *s)) {
        GWEN_Buffer_free(tbuf);
        return GWEN_ERROR_BAD_DATA;
      }
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "Fn;\n");

      GWEN_Buffer_AppendString(tbuf, "}\n");

      Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_Reset(tbuf);

      vf=Typemaker2_VirtualFn_List_Next(vf);
    }
  }

  return 0;
}



