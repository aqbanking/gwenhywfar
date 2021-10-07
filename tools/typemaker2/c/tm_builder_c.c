/***************************************************************************
    begin       : Thu Jul 02 2009
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



#include "tm_builder_c.h"
#include "tm2c_enums.h"
#include "tm2c_constructor.h"
#include "tm2c_destructor.h"
#include "tm2c_typedef.h"
#include "tm2c_struct.h"
#include "tm2c_getter.h"
#include "tm2c_setter.h"
#include "tm2c_readdb.h"
#include "tm2c_writedb.h"
#include "tm2c_readxml.h"
#include "tm2c_writexml.h"
#include "tm2c_readobject.h"
#include "tm2c_writeobject.h"
#include "tm2c_dup.h"
#include "tm2c_copy.h"
#include "tm2c_macros.h"
#include "tm2c_list1.h"
#include "tm2c_tree1.h"
#include "tm2c_tree2.h"
#include "tm2c_signal.h"
#include "tm2c_slot.h"
#include "tm2c_object.h"
#include "tm2c_virtualfn.h"


#include <gwenhywfar/debug.h>

#include <ctype.h>
#include <string.h>



/* DEBUG */
/*DBG_ERROR(0, "Member %s has flags %x", Typemaker2_Member_GetName(tm), Typemaker2_Member_GetFlags(tm));*/



static int _buildFieldIds(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  GWEN_BUFFER *tbuf;
  TYPEMAKER2_MEMBER_LIST *tml;

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  tml=Typemaker2_Type_GetMembers(ty);
  if (tml) {
    TYPEMAKER2_MEMBER *tm;

    tm=Typemaker2_Member_List_First(tml);
    while (tm) {
      if (!(Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_VOLATILE))
        GWEN_Buffer_AppendArgs(tbuf,
                               "#define %s %d\n",
                               Typemaker2_Member_GetFieldId(tm),
                               Typemaker2_Member_GetMemberPosition(tm));
      tm=Typemaker2_Member_List_Next(tm);
    }
  }

  GWEN_Buffer_AppendArgs(tbuf, "#define %s %d\n",
                         Typemaker2_Type_GetFieldCountId(ty),
                         Typemaker2_Type_GetNonVolatileMemberCount(ty));

  Typemaker2_Builder_AddPrivateDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}


static int _buildPostHeaders(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  GWEN_BUFFER *tbuf;
  TYPEMAKER2_TYPEMANAGER *tym GWEN_UNUSED;
  TYPEMAKER2_HEADER_LIST *hl;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  /* handle post-headers */
  hl=Typemaker2_Type_GetHeaders(ty);
  if (hl) {
    TYPEMAKER2_HEADER *h;

    h=Typemaker2_Header_List_First(hl);
    if (h) {
      GWEN_Buffer_AppendString(tbuf, "/* post-headers */\n");
      while (h) {
        if (Typemaker2_Header_GetLocation(h)==Typemaker2_HeaderLocation_Post) {
          GWEN_Buffer_AppendString(tbuf, "#include ");

          if (Typemaker2_Header_GetType(h)==Typemaker2_HeaderType_System)
            GWEN_Buffer_AppendArgs(tbuf, "<%s>", Typemaker2_Header_GetFileName(h));
          else
            GWEN_Buffer_AppendArgs(tbuf, "\"%s\"", Typemaker2_Header_GetFileName(h));
          GWEN_Buffer_AppendString(tbuf, "\n");
        }
        h=Typemaker2_Header_List_Next(h);
      }
    }
    GWEN_Buffer_AppendString(tbuf, "\n");
  }

  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



static int _buildEndHeaders(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  GWEN_BUFFER *tbuf;
  TYPEMAKER2_TYPEMANAGER *tym GWEN_UNUSED;
  TYPEMAKER2_HEADER_LIST *hl;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  /* handle post-headers */
  hl=Typemaker2_Type_GetHeaders(ty);
  if (hl) {
    TYPEMAKER2_HEADER *h;

    h=Typemaker2_Header_List_First(hl);
    if (h) {
      GWEN_Buffer_AppendString(tbuf, "/* end-headers */\n");
      while (h) {
        if (Typemaker2_Header_GetLocation(h)==Typemaker2_HeaderLocation_HeaderEnd) {
          GWEN_Buffer_AppendString(tbuf, "#include ");

          if (Typemaker2_Header_GetType(h)==Typemaker2_HeaderType_System)
            GWEN_Buffer_AppendArgs(tbuf, "<%s>", Typemaker2_Header_GetFileName(h));
          else
            GWEN_Buffer_AppendArgs(tbuf, "\"%s\"", Typemaker2_Header_GetFileName(h));
          GWEN_Buffer_AppendString(tbuf, "\n");
        }
        h=Typemaker2_Header_List_Next(h);
      }
    }
    GWEN_Buffer_AppendString(tbuf, "\n");
  }

  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



static int _buildGetByMember(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  GWEN_BUFFER *tbuf;
  TYPEMAKER2_MEMBER_LIST *tml;
  uint32_t flags;
  /* TYPEMAKER2_TYPEMANAGER *tym; */

  /* tym=Typemaker2_Builder_GetTypeManager(tb); */
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  flags=Typemaker2_Type_GetFlags(ty);

  tml=Typemaker2_Type_GetMembers(ty);
  if (tml) {
    TYPEMAKER2_MEMBER *tm;

    tm=Typemaker2_Member_List_First(tml);
    while (tm) {
      TYPEMAKER2_TYPE *mty;
      int rv;

      mty=Typemaker2_Member_GetTypePtr(tm);
      assert(mty);

      if (Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_WITH_GETBYMEMBER) {
        if (flags & TYPEMAKER2_TYPEFLAGS_WITH_LIST1) {
          rv=TM2C_BuildList1GetByMember(tb, ty, tm);
          if (rv<0) {
            DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
            return rv;
          }
        }

        if (flags & TYPEMAKER2_TYPEFLAGS_WITH_TREE) {
          rv=TM2C_BuildTreeGetByMember(tb, ty, tm);
          if (rv<0) {
            DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
            return rv;
          }
        }

        if (flags & TYPEMAKER2_TYPEFLAGS_WITH_TREE2) {
          rv=TM2C_BuildTree2GetByMember(tb, ty, tm);
          if (rv<0) {
            DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
            return rv;
          }
        }

      }
      tm=Typemaker2_Member_List_Next(tm);
    }
    GWEN_Buffer_AppendString(tbuf, "  return p_cl;\n");
  }

  return 0;
}



static int _buildSortByMember(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  TYPEMAKER2_MEMBER_LIST *tml;
  uint32_t flags;

  flags=Typemaker2_Type_GetFlags(ty);

  tml=Typemaker2_Type_GetMembers(ty);
  if (tml) {
    TYPEMAKER2_MEMBER *tm;

    tm=Typemaker2_Member_List_First(tml);
    while (tm) {
      TYPEMAKER2_TYPE *mty;
      int rv;

      mty=Typemaker2_Member_GetTypePtr(tm);
      assert(mty);

      if (Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_SORTBYMEMBER) {
        if (flags & TYPEMAKER2_TYPEFLAGS_WITH_LIST1) {
          rv=TM2C_BuildList1SortByMember(tb, ty, tm);
          if (rv<0) {
            DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
            return rv;
          }
        }
      }
      tm=Typemaker2_Member_List_Next(tm);
    }
  }

  return 0;
}



static int _buildDefineDefines(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  TYPEMAKER2_DEFINE_LIST *defines;

  defines=Typemaker2_Type_GetDefines(ty);

  assert(defines);
  if (Typemaker2_Define_List_GetCount(defines)) {
    GWEN_BUFFER *tbuf;
    const char *s;
    TYPEMAKER2_DEFINE *td;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);

    td=Typemaker2_Define_List_First(defines);
    while (td) {
      const char *prefix;
      TYPEMAKER2_ITEM *ti;
      int access=TypeMaker2_Access_Public;
      int j=Typemaker2_Define_GetStartValue(td);

      GWEN_Buffer_AppendString(tbuf, "/* define ");
      s=Typemaker2_Define_GetId(td);
      if (s && *s)
        GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, " */\n");

      prefix=Typemaker2_Define_GetPrefix(td);
      if (prefix==NULL || *prefix==0) {
        DBG_ERROR(GWEN_LOGDOMAIN, "No prefix in define definition");
        GWEN_Buffer_free(tbuf);
        return GWEN_ERROR_BAD_DATA;
      }

      ti=Typemaker2_Item_List_First(Typemaker2_Define_GetItems(td));
      while (ti) {
        GWEN_Buffer_AppendString(tbuf, "#define ");
        if (prefix)
          GWEN_Buffer_AppendString(tbuf, prefix);
        s=Typemaker2_Item_GetName(ti);
        if (s && *s) {
          GWEN_Buffer_AppendString(tbuf, s);
        }
        s=Typemaker2_Item_GetValue(ti);
        if (s) {
          GWEN_Buffer_AppendString(tbuf, " ");
          GWEN_Buffer_AppendString(tbuf, s);
        }
        else {
          char numbuf[32];
          int k=0;

          switch (Typemaker2_Define_GetMode(td)) {
          case Typemaker2_Define_Mode_BitField:
            k=1<<j;
            break;
          case Typemaker2_Define_Mode_Sequence:
          default:
            k=j;
          }
          snprintf(numbuf, sizeof(numbuf)-1, "%d", k);
          numbuf[sizeof(numbuf)-1]=0;
          GWEN_Buffer_AppendString(tbuf, " ");
          GWEN_Buffer_AppendString(tbuf, numbuf);
          j++;
        }

        GWEN_Buffer_AppendString(tbuf, "\n");
        ti=Typemaker2_Item_List_Next(ti);
      }
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

      td=Typemaker2_Define_List_Next(td);
    }
  }
  return 0;
}



static int _buildToHashString(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  GWEN_BUFFER *tbuf;
  const char *s;
  TYPEMAKER2_TYPEMANAGER *tym;
  TYPEMAKER2_MEMBER_LIST *tml;
  const char *sTypeId;
  const char *sTypePrefix;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  sTypeId=Typemaker2_Type_GetIdentifier(ty);
  sTypePrefix=Typemaker2_Type_GetPrefix(ty);

  /* prototype */
  s=Typemaker2_TypeManager_GetApiDeclaration(tym);
  if (s && *s)
    GWEN_Buffer_AppendArgs(tbuf, "%s ", s);
  GWEN_Buffer_AppendArgs(tbuf, "void %s_toHashString(const %s *p_struct, GWEN_BUFFER *p_buffer);\n",
                         sTypePrefix, sTypeId);
  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_Reset(tbuf);

  /* implementation */
  GWEN_Buffer_AppendArgs(tbuf, "void %s_toHashString(const %s *p_struct, GWEN_BUFFER *p_buffer) {\n",
                         sTypePrefix, sTypeId);
  GWEN_Buffer_AppendString(tbuf, "  assert(p_struct);\n");

  tml=Typemaker2_Type_GetMembers(ty);
  if (tml) {
    TYPEMAKER2_MEMBER *tm;

    tm=Typemaker2_Member_List_First(tml);
    while (tm) {
      TYPEMAKER2_TYPE *mty;
      const char *sMemberName;

      mty=Typemaker2_Member_GetTypePtr(tm);
      assert(mty);
      sMemberName=Typemaker2_Member_GetName(tm);

      if (Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_WITH_HASH) {
        GWEN_Buffer_AppendArgs(tbuf, "  /* member \"%s\"*/\n", sMemberName);
        /* toHashString */
        if (1) {
          GWEN_BUFFER *srcbuf;
          int rv;

          srcbuf=GWEN_Buffer_new(0, 256, 0, 1);
          GWEN_Buffer_AppendArgs(srcbuf, "p_struct->%s", sMemberName);

          GWEN_Buffer_AppendString(tbuf, "  ");
          rv=Typemaker2_Builder_Invoke_ToHashStringFn(tb, ty, tm,
                                                      GWEN_Buffer_GetStart(srcbuf),
                                                      NULL, /* no dest */
                                                      tbuf);
          GWEN_Buffer_free(srcbuf);
          if (rv<0) {
            DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
            GWEN_Buffer_free(tbuf);
            return rv;
          }
          GWEN_Buffer_AppendString(tbuf, "\n");
        }

        GWEN_Buffer_AppendString(tbuf, "  GWEN_Buffer_AppendByte(p_buffer, \':\');\n");
      }
      else
        GWEN_Buffer_AppendArgs(tbuf, "  /* member \"%s\" doesn't have with_hash flag, ignoring */\n", sMemberName);

      tm=Typemaker2_Member_List_Next(tm);
    } /* while tm */
  }

  GWEN_Buffer_AppendString(tbuf, "}\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



static int _buildInlines(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  GWEN_BUFFER *tbuf;
  uint32_t flags;

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  flags=Typemaker2_Type_GetFlags(ty);

  while (ty) {
    TYPEMAKER2_INLINE *ti;

    ti=Typemaker2_Inline_List_First(Typemaker2_Type_GetInlines(ty));
    while (ti) {
      if ((flags & Typemaker2_Inline_GetTypeFlagsMask(ti))==Typemaker2_Inline_GetTypeFlagsValue(ti)) {
        const char *content;

        content=Typemaker2_Inline_GetContent(ti);
        if (content && *content) {
          GWEN_DB_NODE *db;
          int rv;

          db=Typemaker2_Builder_CreateDbForCall(tb, ty, NULL, NULL, NULL);
          assert(db);
          rv=Typemaker2_Builder_ReplaceVars(content, db, tbuf);
          if (rv<0) {
            DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
            GWEN_Buffer_free(tbuf);
            return rv;
          }

          /* include code */
          switch (Typemaker2_Inline_GetLocation(ti)) {
          case Typemaker2_InlineLocation_Header:
            Typemaker2_Builder_AddDeclaration(tb, Typemaker2_Inline_GetAccess(ti), GWEN_Buffer_GetStart(tbuf));
            break;

          case Typemaker2_InlineLocation_Code:
            Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
            break;
          }
        }
        GWEN_Buffer_Reset(tbuf);
      }

      ti=Typemaker2_Inline_List_Next(ti);
    }
    ty=Typemaker2_Type_GetExtendsPtr(ty);
  }

  return 0;
}



static int _buildMemberInlines(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  GWEN_BUFFER *tbuf;
  uint32_t flags;
  /* TYPEMAKER2_TYPEMANAGER *tym; */

  /* tym=Typemaker2_Builder_GetTypeManager(tb); */
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  flags=Typemaker2_Type_GetFlags(ty);

  while (ty) {
    TYPEMAKER2_MEMBER_LIST *tml;

    tml=Typemaker2_Type_GetMembers(ty);
    if (tml) {
      TYPEMAKER2_MEMBER *tm;

      tm=Typemaker2_Member_List_First(tml);
      while (tm) {
        TYPEMAKER2_TYPE *mty;

        mty=Typemaker2_Member_GetTypePtr(tm);
        if (mty) {
          TYPEMAKER2_INLINE *ti;

          /* get inlines from member type */
          ti=Typemaker2_Inline_List_First(Typemaker2_Type_GetInlines(mty));
          while (ti) {
            if ((flags & Typemaker2_Inline_GetTypeFlagsMask(ti))==Typemaker2_Inline_GetTypeFlagsValue(ti)) {
              const char *content;

              content=Typemaker2_Inline_GetContent(ti);
              if (content && *content) {
                GWEN_DB_NODE *db;
                int rv;

                /* replace vars in context of the struct type, not the particular member type */
                db=Typemaker2_Builder_CreateDbForCall(tb, ty, tm, NULL, NULL);
                assert(db);
                rv=Typemaker2_Builder_ReplaceVars(content, db, tbuf);
                if (rv<0) {
                  DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
                  GWEN_Buffer_free(tbuf);
                  return rv;
                }

                /* include code */
                switch (Typemaker2_Inline_GetLocation(ti)) {
                case Typemaker2_InlineLocation_Header:
                  Typemaker2_Builder_AddDeclaration(tb, Typemaker2_Inline_GetAccess(ti), GWEN_Buffer_GetStart(tbuf));
                  break;

                case Typemaker2_InlineLocation_Code:
                  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
                  break;
                }
              }
              GWEN_Buffer_Reset(tbuf);
            }

            ti=Typemaker2_Inline_List_Next(ti);
          }
        } /* if mty */

        tm=Typemaker2_Member_List_Next(tm);
      } /* while tm */
    } /* if tml */

    ty=Typemaker2_Type_GetExtendsPtr(ty);
  }

  return 0;
}



static int _buildAttach(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  GWEN_BUFFER *tbuf;
  const char *s;
  TYPEMAKER2_TYPEMANAGER *tym;
  const char *sTypeId;
  const char *sTypePrefix;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  sTypeId=Typemaker2_Type_GetIdentifier(ty);
  sTypePrefix=Typemaker2_Type_GetPrefix(ty);

  /* prototype */
  s=Typemaker2_TypeManager_GetApiDeclaration(tym);
  if (s && *s)
    GWEN_Buffer_AppendArgs(tbuf, "%s ", s);

  GWEN_Buffer_AppendArgs(tbuf, "void %s_Attach(%s *p_struct);\n", sTypePrefix, sTypeId);

  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_Reset(tbuf);

  /* implementation */
  GWEN_Buffer_AppendArgs(tbuf,   "void %s_Attach(%s *p_struct) {\n", sTypePrefix, sTypeId);
  GWEN_Buffer_AppendString(tbuf, "  assert(p_struct);\n");
  GWEN_Buffer_AppendString(tbuf, "  assert(p_struct->_refCount);\n");
  GWEN_Buffer_AppendString(tbuf, "  p_struct->_refCount++;\n");
  GWEN_Buffer_AppendString(tbuf, "}\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



static int _buildCacheFunctions(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  GWEN_BUFFER *tbuf;
  TYPEMAKER2_TYPEMANAGER *tym;
  const char *sApi;
  const char *sTypePrefix;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  sApi=Typemaker2_TypeManager_GetApiDeclaration(tym);
  sTypePrefix=Typemaker2_Type_GetPrefix(ty);

  /* public prototypes */
  GWEN_Buffer_AppendString(tbuf, "/* cache functions */\n");

  if (sApi && *sApi)
    GWEN_Buffer_AppendArgs(tbuf, "%s ", sApi);
  GWEN_Buffer_AppendArgs(tbuf,   "int GWENHYWFAR_CB %s_CacheFn_Attach(void *ptr);\n", sTypePrefix);
  GWEN_Buffer_AppendString(tbuf, "\n");

  if (sApi && *sApi)
    GWEN_Buffer_AppendArgs(tbuf, "%s ", sApi);
  GWEN_Buffer_AppendArgs(tbuf,   "int GWENHYWFAR_CB %s_CacheFn_Free(void *ptr);\n", sTypePrefix);
  GWEN_Buffer_AppendString(tbuf, "\n");

  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_Reset(tbuf);

  /* implementations */
  GWEN_Buffer_AppendString(tbuf, "/* cache functions */\n");
  GWEN_Buffer_AppendArgs(tbuf,   "int %s_CacheFn_Attach(void *ptr) {\n", sTypePrefix);
  GWEN_Buffer_AppendString(tbuf, "  return 0;\n");
  GWEN_Buffer_AppendString(tbuf, "}\n");
  GWEN_Buffer_AppendString(tbuf, "\n");

  GWEN_Buffer_AppendArgs(tbuf,   "int %s_CacheFn_Free(void *ptr) {\n", sTypePrefix);
  GWEN_Buffer_AppendString(tbuf, "  return 0;\n");
  GWEN_Buffer_AppendString(tbuf, "}\n");
  GWEN_Buffer_AppendString(tbuf, "\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



static int _buildGroupApiDoc(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty,
                             TYPEMAKER2_GROUP *grp, int depth, GWEN_BUFFER *buf)
{
  const char *s;
  TYPEMAKER2_GROUP *childGrp;
  TYPEMAKER2_MEMBER_LIST *members;
  const char *sTypeIdentifier;
  const char *sTypePrefix;

  sTypeIdentifier=Typemaker2_Type_GetIdentifier(ty);
  sTypePrefix=Typemaker2_Type_GetPrefix(ty);

  /* write title */
  s=Typemaker2_Group_GetTitle(grp);
  GWEN_Buffer_AppendString(buf, "\n\n");
  GWEN_Buffer_AppendArgs(buf, "<h%d>%s</h%d>\n", depth, (s && *s)?s:"Unnamed Group", depth);
  GWEN_Buffer_AppendString(buf, "\n");

  s=Typemaker2_Group_GetDescription(grp);
  if (s && *s) {
    GWEN_DB_NODE *dbCall;
    int rv;

    dbCall=Typemaker2_Builder_CreateDbForCall(tb, ty, NULL, NULL, NULL);
    assert(dbCall);
    rv=Typemaker2_Builder_ReplaceVars(s, dbCall, buf);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_DB_Group_free(dbCall);
      GWEN_Buffer_free(buf);
      return rv;
    }
    GWEN_DB_Group_free(dbCall);
    GWEN_Buffer_AppendString(buf, "\n");
  }

  /* write member docs */
  members=Typemaker2_Type_GetMembers(ty);
  if (members) {
    TYPEMAKER2_MEMBER *tm;

    tm=Typemaker2_Member_List_First(members);
    while (tm) {
      if (Typemaker2_Member_GetGroupPtr(tm)==grp) {
        const char *sMemberName;

        sMemberName=Typemaker2_Member_GetName(tm);
        GWEN_Buffer_AppendArgs(buf, "\n\n@anchor %s_%s\n", sTypeIdentifier, sMemberName);
        GWEN_Buffer_AppendArgs(buf, "<h%d>%s</h%d>\n\n", depth+1, sMemberName, depth+1);

        /* add description, if any */
        s=Typemaker2_Member_GetDescription(tm);
        if (s && *s) {
          GWEN_DB_NODE *dbCall;
          int rv;

          dbCall=Typemaker2_Builder_CreateDbForCall(tb, ty, tm, NULL, NULL);
          assert(dbCall);
          rv=Typemaker2_Builder_ReplaceVars(s, dbCall, buf);
          if (rv<0) {
            DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
            GWEN_DB_Group_free(dbCall);
            GWEN_Buffer_free(buf);
            return rv;
          }
          GWEN_DB_Group_free(dbCall);
          GWEN_Buffer_AppendString(buf, "\n");
        }

        /* add setter/getter info */
        GWEN_Buffer_AppendArgs(buf,
                               "<p>Set this property with @ref %s_Set%c%s(), ",
                               sTypePrefix, toupper(*sMemberName), sMemberName+1);
        GWEN_Buffer_AppendArgs(buf,
                               "get it with @ref %s_Get%c%s().</p>\n",
                               sTypePrefix, toupper(*sMemberName), sMemberName+1);
      }
      tm=Typemaker2_Member_List_Next(tm);
    }
  }

  /* write children groups */
  childGrp=Typemaker2_Group_Tree_GetFirstChild(grp);
  while (childGrp) {
    int rv;

    rv=_buildGroupApiDoc(tb, ty, childGrp, depth+1, buf);
    if (rv<0) {
      DBG_ERROR(0, "here (%d)", rv);
      return rv;
    }
    childGrp=Typemaker2_Group_Tree_GetNext(childGrp);
  }

  return 0;
}




static int _buildApiDoc(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  TYPEMAKER2_GROUP_TREE *groupTree;
  TYPEMAKER2_GROUP *grp;
  GWEN_BUFFER *tbuf;
  const char *sTypeIdentifier;

  sTypeIdentifier=Typemaker2_Type_GetIdentifier(ty);

  groupTree=Typemaker2_Type_GetGroupTree(ty);

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  GWEN_Buffer_AppendArgs(tbuf, "/** @page P_%s Structure %s\n", sTypeIdentifier, sTypeIdentifier);
  GWEN_Buffer_AppendArgs(tbuf, "<p>This page describes the properties of %s.</p>\n", sTypeIdentifier);
  GWEN_Buffer_AppendString(tbuf, "\n");

  grp=Typemaker2_Group_Tree_GetFirst(groupTree);
  while (grp) {
    _buildGroupApiDoc(tb, ty, grp, 1, tbuf);
    grp=Typemaker2_Group_Tree_GetNext(grp);
  }
  GWEN_Buffer_AppendString(tbuf, "\n");
  GWEN_Buffer_AppendString(tbuf, "*/\n");

  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));

  GWEN_Buffer_free(tbuf);

  return 0;
}








static int Typemaker2_Builder_C_Build(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  int rv;
  uint32_t flags;

  flags=Typemaker2_Type_GetFlags(ty);

  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_SLOTS) {
    TM2C_DelVirtualFnsFromSlots(ty);
    rv=TM2C_AddVirtualFnsFromSlots(ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }


  rv=TM2C_SetEnumStringFns(ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_OBJECT) {
    rv=_buildFieldIds(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  rv=_buildApiDoc(tb, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=_buildDefineDefines(tb, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=TM2C_BuildTypedef(tb, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=TM2C_BuildDefineEnums(tb, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=TM2C_BuildDefineVirtualFns(tb, ty, "pre");
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=_buildPostHeaders(tb, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=TM2C_BuildDefineVirtualFns(tb, ty, "post");
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=TM2C_BuildStruct(tb, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=TM2C_BuildMacroFunctions(tb, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=TM2C_BuildEnumFromString(tb, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=TM2C_BuildEnumToString(tb, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=TM2C_BuildConstructor(tb, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=TM2C_BuildDestructor(tb, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_REFCOUNT) {
    rv=_buildAttach(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  if (!(flags & TYPEMAKER2_TYPEFLAGS_NODUP)) {
    rv=TM2C_BuildDup(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  if (!(flags & TYPEMAKER2_TYPEFLAGS_NOCOPY)) {
    rv=TM2C_BuildCopy(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  rv=TM2C_BuildGetter(tb, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=TM2C_BuildSetter(tb, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=TM2C_BuildProtoVirtualFns(tb, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=TM2C_BuildCodeVirtualFns(tb, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=TM2C_BuildProtoSetterVirtualFns(tb, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=TM2C_BuildSetterVirtualFns(tb, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=TM2C_BuildProtoGetterVirtualFns(tb, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=TM2C_BuildGetterVirtualFns(tb, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_LIST1) {
    if (!(flags & TYPEMAKER2_TYPEFLAGS_NODUP)) {
      rv=TM2C_BuildList1Dup(tb, ty);
      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
    }
  }

  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_DB) {
    rv=TM2C_BuildReadDb(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    rv=TM2C_BuildWriteDb(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    rv=TM2C_BuildFromDb(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    rv=TM2C_BuildToDb(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

  }

  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_XML) {
    rv=TM2C_BuildReadXml(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    rv=TM2C_BuildWriteXml(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    rv=TM2C_BuildToXml(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    rv=TM2C_BuildFromXml(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_OBJECT) {
    rv=TM2C_BuildReadObject(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    rv=TM2C_BuildWriteObject(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    rv=TM2C_BuildToObject(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    rv=TM2C_BuildFromObject(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    rv=TM2C_BuildCreateColumnList(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    rv=TM2C_BuildCreateTable(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    rv=TM2C_BuildCreateObject(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  rv=_buildGetByMember(tb, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_HASH) {
    rv=_buildToHashString(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  rv=_buildMemberInlines(tb, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }


  rv=_buildSortByMember(tb, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }


  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_SIGNALS) {
    rv=TM2C_BuildSignalFunctions(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_SLOTS) {
    rv=TM2C_BuildSlotFunctions(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  if (flags & TYPEMAKER2_TYPEFLAGS_WITH_CACHEFNS) {
    rv=_buildCacheFunctions(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  rv=_buildInlines(tb, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=_buildEndHeaders(tb, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



TYPEMAKER2_BUILDER *Typemaker2_Builder_C_new()
{
  TYPEMAKER2_BUILDER *tb;

  tb=Typemaker2_Builder_new();
  Typemaker2_Builder_SetBuildFn(tb, Typemaker2_Builder_C_Build);

  return tb;
}





