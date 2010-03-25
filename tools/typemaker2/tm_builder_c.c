/***************************************************************************
    begin       : Thu Jul 02 2009
    copyright   : (C) 2009 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



#include "tm_builder_c.h"

#include <gwenhywfar/debug.h>

#include <ctype.h>



static int _buildFieldIds(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty) {
  GWEN_BUFFER *tbuf;
  const char *s;
  TYPEMAKER2_MEMBER_LIST *tml;
  char numbuf[16];

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  tml=Typemaker2_Type_GetMembers(ty);
  if (tml) {
    TYPEMAKER2_MEMBER *tm;

    tm=Typemaker2_Member_List_First(tml);
    while(tm) {
      if (!(Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_VOLATILE)) {
	GWEN_Buffer_AppendString(tbuf, "#define ");
	s=Typemaker2_Member_GetFieldId(tm);
	GWEN_Buffer_AppendString(tbuf, s);
	GWEN_Buffer_AppendString(tbuf, " ");

	snprintf(numbuf, sizeof(numbuf)-1, "%d",
                 Typemaker2_Member_GetMemberPosition(tm));
	GWEN_Buffer_AppendString(tbuf, numbuf);
	GWEN_Buffer_AppendString(tbuf, "\n");
      }

      tm=Typemaker2_Member_List_Next(tm);
    }
  }

  GWEN_Buffer_AppendString(tbuf, "#define ");
  s=Typemaker2_Type_GetFieldCountId(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " ");

  snprintf(numbuf, sizeof(numbuf)-1, "%d",
	   Typemaker2_Type_GetNonVolatileMemberCount(ty));
  GWEN_Buffer_AppendString(tbuf, numbuf);
  GWEN_Buffer_AppendString(tbuf, "\n");

  Typemaker2_Builder_AddPrivateDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



/** writes typedef line into public header and also writes includes */
static int _buildTypedef(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty) {
  GWEN_BUFFER *tbuf;
  const char *s;
  uint32_t flags;
  TYPEMAKER2_TYPEMANAGER *tym;
  TYPEMAKER2_HEADER_LIST *hl;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  flags=Typemaker2_Type_GetFlags(ty);

  /* add some system headers */
  GWEN_Buffer_AppendString(tbuf, "/* needed system headers */\n");
  GWEN_Buffer_AppendString(tbuf, "#include <gwenhywfar/types.h>\n");
  if (flags & TYPEMAKER2_FLAGS_WITH_LIST1)
    GWEN_Buffer_AppendString(tbuf, "#include <gwenhywfar/list1.h>\n");
  if (flags & TYPEMAKER2_FLAGS_WITH_LIST2)
    GWEN_Buffer_AppendString(tbuf, "#include <gwenhywfar/list2.h>\n");
  if (flags & TYPEMAKER2_FLAGS_WITH_TREE)
    GWEN_Buffer_AppendString(tbuf, "#include <gwenhywfar/tree.h>\n");
  if (flags & TYPEMAKER2_FLAGS_WITH_INHERIT)
    GWEN_Buffer_AppendString(tbuf, "#include <gwenhywfar/inherit.h>\n");
  if (flags & TYPEMAKER2_FLAGS_WITH_IDMAP)
    GWEN_Buffer_AppendString(tbuf, "#include <gwenhywfar/idmap.h>\n");
  if (flags & TYPEMAKER2_FLAGS_WITH_DB)
    GWEN_Buffer_AppendString(tbuf, "#include <gwenhywfar/db.h>\n");
  if (flags & TYPEMAKER2_FLAGS_WITH_XML)
    GWEN_Buffer_AppendString(tbuf, "#include <gwenhywfar/xml.h>\n");
  if (flags & TYPEMAKER2_FLAGS_WITH_OBJECT) {
    GWEN_Buffer_AppendString(tbuf, "#include <aqfinance/engine/db/aedb.h>\n");
    GWEN_Buffer_AppendString(tbuf, "#include <aqfinance/engine/db/aedb_db.h>\n");
  }
  GWEN_Buffer_AppendString(tbuf, "\n");

  /* handle pre-headers */
  hl=Typemaker2_Type_GetHeaders(ty);
  if (hl) {
    TYPEMAKER2_HEADER *h;

    h=Typemaker2_Header_List_First(hl);
    if (h) {
      GWEN_Buffer_AppendString(tbuf, "/* pre-headers */\n");
      while(h) {
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

  GWEN_Buffer_AppendString(tbuf, "typedef struct ");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " ");
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, ";\n");

  if (flags & TYPEMAKER2_FLAGS_WITH_LIST1) {
    if (Typemaker2_TypeManager_GetApiDeclaration(tym)) {
      GWEN_Buffer_AppendString(tbuf, "GWEN_LIST_FUNCTION_LIB_DEFS(");
      s=Typemaker2_Type_GetIdentifier(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, ", ");
      s=Typemaker2_Type_GetPrefix(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, ", ");
      s=Typemaker2_TypeManager_GetApiDeclaration(tym);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, ")\n");
    }
    else {
      GWEN_Buffer_AppendString(tbuf, "GWEN_LIST_FUNCTION_DEFS(");
      s=Typemaker2_Type_GetIdentifier(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, ", ");
      s=Typemaker2_Type_GetPrefix(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, ")\n");
    }
  }

  if (flags & TYPEMAKER2_FLAGS_WITH_LIST2) {
    if (Typemaker2_TypeManager_GetApiDeclaration(tym)) {
      GWEN_Buffer_AppendString(tbuf, "GWEN_LIST2_FUNCTION_LIB_DEFS(");
      s=Typemaker2_Type_GetIdentifier(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, ", ");
      s=Typemaker2_Type_GetPrefix(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, ", ");
      s=Typemaker2_TypeManager_GetApiDeclaration(tym);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, ")\n");
    }
    else {
      GWEN_Buffer_AppendString(tbuf, "GWEN_LIST2_FUNCTION_DEFS(");
      s=Typemaker2_Type_GetIdentifier(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, ", ");
      s=Typemaker2_Type_GetPrefix(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, ")\n");
    }
  }

  if (flags & TYPEMAKER2_FLAGS_WITH_TREE) {
    if (Typemaker2_TypeManager_GetApiDeclaration(tym)) {
      GWEN_Buffer_AppendString(tbuf, "GWEN_TREE_FUNCTION_LIB_DEFS(");
      s=Typemaker2_Type_GetIdentifier(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, ", ");
      s=Typemaker2_Type_GetPrefix(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, ", ");
      s=Typemaker2_TypeManager_GetApiDeclaration(tym);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, ")\n");
    }
    else {
      GWEN_Buffer_AppendString(tbuf, "GWEN_TREE_FUNCTION_DEFS(");
      s=Typemaker2_Type_GetIdentifier(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, ", ");
      s=Typemaker2_Type_GetPrefix(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, ")\n");
    }
  }

  if (flags & TYPEMAKER2_FLAGS_WITH_INHERIT) {
    if (Typemaker2_TypeManager_GetApiDeclaration(tym)) {
      GWEN_Buffer_AppendString(tbuf, "GWEN_INHERIT_FUNCTION_LIB_DEFS(");
      s=Typemaker2_Type_GetIdentifier(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, ", ");
      s=Typemaker2_TypeManager_GetApiDeclaration(tym);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, ")\n");
    }
    else {
      GWEN_Buffer_AppendString(tbuf, "GWEN_INHERIT_FUNCTION_DEFS(");
      s=Typemaker2_Type_GetIdentifier(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, ")\n");
    }
  }

  if (flags & TYPEMAKER2_FLAGS_WITH_IDMAP) {
    if (Typemaker2_TypeManager_GetApiDeclaration(tym)) {
      GWEN_Buffer_AppendString(tbuf, "GWEN_IDMAP_FUNCTION_LIB_DEFS(");
      s=Typemaker2_Type_GetIdentifier(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, ", ");
      s=Typemaker2_Type_GetPrefix(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, ", ");
      s=Typemaker2_TypeManager_GetApiDeclaration(tym);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, ")\n");
    }
    else {
      GWEN_Buffer_AppendString(tbuf, "GWEN_IDMAP_FUNCTION_DEFS(");
      s=Typemaker2_Type_GetIdentifier(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, ", ");
      s=Typemaker2_Type_GetPrefix(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, ")\n");
    }
  }
  GWEN_Buffer_AppendString(tbuf, "\n");

  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



static int _buildPostHeaders(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty) {
  GWEN_BUFFER *tbuf;
  TYPEMAKER2_TYPEMANAGER *tym;
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
      while(h) {
	if (Typemaker2_Header_GetLocation(h)==Typemaker2_HeaderLocation_Post) {
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
    }
    GWEN_Buffer_AppendString(tbuf, "\n");
  }

  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



static int _buildStruct(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty) {
  GWEN_BUFFER *tbuf;
  const char *s;
  TYPEMAKER2_MEMBER_LIST *tml;
  uint32_t flags;

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  flags=Typemaker2_Type_GetFlags(ty);

  GWEN_Buffer_AppendString(tbuf, "struct ");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " {\n");

  if (flags & TYPEMAKER2_FLAGS_WITH_INHERIT) {
    GWEN_Buffer_AppendString(tbuf, "  GWEN_INHERIT_ELEMENT(");
    s=Typemaker2_Type_GetIdentifier(ty);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, ")\n");
  }

  if (flags & TYPEMAKER2_FLAGS_WITH_LIST1) {
    GWEN_Buffer_AppendString(tbuf, "  GWEN_LIST_ELEMENT(");
    s=Typemaker2_Type_GetIdentifier(ty);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, ")\n");
  }

  if (flags & TYPEMAKER2_FLAGS_WITH_TREE) {
    GWEN_Buffer_AppendString(tbuf, "  GWEN_TREE_ELEMENT(");
    s=Typemaker2_Type_GetIdentifier(ty);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, ")\n");
  }

  if (flags & TYPEMAKER2_FLAGS_WITH_REFCOUNT)
    GWEN_Buffer_AppendString(tbuf, "  int _refCount;\n");


  tml=Typemaker2_Type_GetMembers(ty);
  if (tml) {
    TYPEMAKER2_MEMBER *tm;

    tm=Typemaker2_Member_List_First(tml);
    while(tm) {
      TYPEMAKER2_TYPE *mty;
      int typeIsWritten=0;

      mty=Typemaker2_Member_GetTypePtr(tm);
      assert(mty);

      GWEN_Buffer_AppendString(tbuf, "  ");

      if (Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_ENUM) {
	TYPEMAKER2_ENUM *te=Typemaker2_Member_GetEnumPtr(tm);
	const char *s;

	assert(te);
	s=Typemaker2_Enum_GetType(te);
	if (s && *s) {
	  GWEN_Buffer_AppendString(tbuf, s);
	  GWEN_Buffer_AppendString(tbuf, " ");
	  typeIsWritten=1;
	}
      }

      if (!typeIsWritten) {
	if ((Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Pointer) &&
	    (Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_CONST))
	  GWEN_Buffer_AppendString(tbuf, "const ");

	s=Typemaker2_Type_GetIdentifier(mty);
	GWEN_Buffer_AppendString(tbuf, s);
	GWEN_Buffer_AppendString(tbuf, " ");

	if (Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Pointer)
	  GWEN_Buffer_AppendString(tbuf, "*");
      }

      s=Typemaker2_Member_GetName(tm);
      GWEN_Buffer_AppendString(tbuf, s);
      if (Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Array) {
	char numbuf[32];

	snprintf(numbuf, sizeof(numbuf)-1, "[%d]",
		 Typemaker2_Member_GetMaxLen(tm));
	GWEN_Buffer_AppendString(tbuf, numbuf);
      }

      GWEN_Buffer_AppendString(tbuf, ";");
      if (Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_VOLATILE)
	GWEN_Buffer_AppendString(tbuf, " /* volatile */");
      GWEN_Buffer_AppendString(tbuf, "\n");

      tm=Typemaker2_Member_List_Next(tm);
    }
  }

  GWEN_Buffer_AppendString(tbuf, "};\n");

  Typemaker2_Builder_AddPrivateDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



static int _buildMacroFunctions(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty) {
  GWEN_BUFFER *tbuf;
  const char *s;
  uint32_t flags;
  TYPEMAKER2_TYPEMANAGER *tym;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  GWEN_Buffer_AppendString(tbuf, "/* macro functions */\n");
 
  flags=Typemaker2_Type_GetFlags(ty);
  if (flags & TYPEMAKER2_FLAGS_WITH_LIST1) {
    GWEN_Buffer_AppendString(tbuf, "GWEN_LIST_FUNCTIONS(");
    s=Typemaker2_Type_GetIdentifier(ty);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, ", ");
    s=Typemaker2_Type_GetPrefix(ty);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, ")\n");
  }

  if (flags & TYPEMAKER2_FLAGS_WITH_LIST2) {
    GWEN_Buffer_AppendString(tbuf, "GWEN_LIST2_FUNCTIONS(");
    s=Typemaker2_Type_GetIdentifier(ty);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, ", ");
    s=Typemaker2_Type_GetPrefix(ty);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, ")\n");
  }

  if (flags & TYPEMAKER2_FLAGS_WITH_TREE) {
    GWEN_Buffer_AppendString(tbuf, "GWEN_TREE_FUNCTIONS(");
    s=Typemaker2_Type_GetIdentifier(ty);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, ", ");
    s=Typemaker2_Type_GetPrefix(ty);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, ")\n");
  }

  if (flags & TYPEMAKER2_FLAGS_WITH_INHERIT) {
    GWEN_Buffer_AppendString(tbuf, "GWEN_INHERIT_FUNCTIONS(");
    s=Typemaker2_Type_GetIdentifier(ty);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, ")\n");
  }

  if (flags & TYPEMAKER2_FLAGS_WITH_IDMAP) {
    GWEN_Buffer_AppendString(tbuf, "GWEN_IDMAP_FUNCTIONS(");
    s=Typemaker2_Type_GetIdentifier(ty);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, ", ");
    s=Typemaker2_Type_GetPrefix(ty);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, ")\n");
  }
  GWEN_Buffer_AppendString(tbuf, "\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



static int _buildConstructor(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty) {
  GWEN_BUFFER *tbuf;
  const char *s;
  TYPEMAKER2_MEMBER_LIST *tml;
  uint32_t flags;
  TYPEMAKER2_TYPEMANAGER *tym;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  flags=Typemaker2_Type_GetFlags(ty);

  /* prototype */
  GWEN_Buffer_AppendString(tbuf, "/** Constructor. */\n");
  s=Typemaker2_TypeManager_GetApiDeclaration(tym);
  if (s) {
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, " ");
  }
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  /* created structs are always pointers */
  GWEN_Buffer_AppendString(tbuf, " *");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_new()");
  GWEN_Buffer_AppendString(tbuf, ";\n");
  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_Reset(tbuf);

  /* implementation */
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_new()");
  GWEN_Buffer_AppendString(tbuf, " {\n");

  GWEN_Buffer_AppendString(tbuf, "  ");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *p_struct;\n");
  GWEN_Buffer_AppendString(tbuf, "\n");

  GWEN_Buffer_AppendString(tbuf, "  GWEN_NEW_OBJECT(");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, ", p_struct)\n");

  if (flags & TYPEMAKER2_FLAGS_WITH_REFCOUNT)
    GWEN_Buffer_AppendString(tbuf, "  p_struct->_refCount=1;\n");

  if (flags & TYPEMAKER2_FLAGS_WITH_INHERIT) {
    GWEN_Buffer_AppendString(tbuf, "  GWEN_INHERIT_INIT(");
    s=Typemaker2_Type_GetIdentifier(ty);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, ", p_struct)\n");
  }

  if (flags & TYPEMAKER2_FLAGS_WITH_LIST1) {
    GWEN_Buffer_AppendString(tbuf, "  GWEN_LIST_INIT(");
    s=Typemaker2_Type_GetIdentifier(ty);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, ", p_struct)\n");
  }

  if (flags & TYPEMAKER2_FLAGS_WITH_TREE) {
    GWEN_Buffer_AppendString(tbuf, "  GWEN_TREE_INIT(");
    s=Typemaker2_Type_GetIdentifier(ty);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, ", p_struct)\n");
  }

  GWEN_Buffer_AppendString(tbuf, "  /* members */\n");

  tml=Typemaker2_Type_GetMembers(ty);
  if (tml) {
    TYPEMAKER2_MEMBER *tm;
    int rv;

    tm=Typemaker2_Member_List_First(tml);
    while(tm) {
      TYPEMAKER2_TYPE *mty;
      GWEN_BUFFER *dstbuf;

      dstbuf=GWEN_Buffer_new(0, 256, 0, 1);
      GWEN_Buffer_AppendString(dstbuf, "p_struct->");
      s=Typemaker2_Member_GetName(tm);
      GWEN_Buffer_AppendString(dstbuf, s);

      mty=Typemaker2_Member_GetTypePtr(tm);
      assert(mty);

      GWEN_Buffer_AppendString(tbuf, "  ");

      rv=Typemaker2_Builder_Invoke_ConstructFn(tb, ty, tm,
					       NULL, /* no source */
					       GWEN_Buffer_GetStart(dstbuf),
					       tbuf);
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	GWEN_Buffer_free(dstbuf);
	GWEN_Buffer_free(tbuf);
	return rv;
      }
      GWEN_Buffer_free(dstbuf);

      GWEN_Buffer_AppendString(tbuf, "\n");

      tm=Typemaker2_Member_List_Next(tm);
    }
  }

  GWEN_Buffer_AppendString(tbuf, "\n");
  GWEN_Buffer_AppendString(tbuf, "  return p_struct;\n");
  GWEN_Buffer_AppendString(tbuf, "}\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



static int _buildDestructor(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty) {
  GWEN_BUFFER *tbuf;
  const char *s;
  TYPEMAKER2_MEMBER_LIST *tml;
  uint32_t flags;
  TYPEMAKER2_TYPEMANAGER *tym;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  flags=Typemaker2_Type_GetFlags(ty);

  /* prototype */
  GWEN_Buffer_AppendString(tbuf, "/** Destructor. */\n");
  s=Typemaker2_TypeManager_GetApiDeclaration(tym);
  if (s) {
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, " ");
  }
  GWEN_Buffer_AppendString(tbuf, "void ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_free(");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *p_struct);\n");
  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_Reset(tbuf);

  /* implementation */
  GWEN_Buffer_AppendString(tbuf, "void ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_free(");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *p_struct) {\n");

  GWEN_Buffer_AppendString(tbuf, "  if (p_struct) {\n");

  if (flags & TYPEMAKER2_FLAGS_WITH_REFCOUNT) {
    GWEN_Buffer_AppendString(tbuf, "  assert(p_struct->_refCount);\n");
    GWEN_Buffer_AppendString(tbuf, "  if (p_struct->_refCount==1) {\n");
  }

  if (flags & TYPEMAKER2_FLAGS_WITH_INHERIT) {
    GWEN_Buffer_AppendString(tbuf, "    GWEN_INHERIT_FINI(");
    s=Typemaker2_Type_GetIdentifier(ty);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, ", p_struct)\n");
  }

  if (flags & TYPEMAKER2_FLAGS_WITH_LIST1) {
    GWEN_Buffer_AppendString(tbuf, "    GWEN_LIST_FINI(");
    s=Typemaker2_Type_GetIdentifier(ty);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, ", p_struct)\n");
  }

  if (flags & TYPEMAKER2_FLAGS_WITH_TREE) {
    GWEN_Buffer_AppendString(tbuf, "    GWEN_TREE_FINI(");
    s=Typemaker2_Type_GetIdentifier(ty);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, ", p_struct)\n");
  }

  GWEN_Buffer_AppendString(tbuf, "  /* members */\n");
  tml=Typemaker2_Type_GetMembers(ty);
  if (tml) {
    TYPEMAKER2_MEMBER *tm;

    tm=Typemaker2_Member_List_First(tml);
    while(tm) {
      /* destruct */
      if (1) {
	GWEN_BUFFER *dstbuf;
	int rv;

	dstbuf=GWEN_Buffer_new(0, 256, 0, 1);
	GWEN_Buffer_AppendString(dstbuf, "p_struct->");
	s=Typemaker2_Member_GetName(tm);
	GWEN_Buffer_AppendString(dstbuf, s);

	GWEN_Buffer_AppendString(tbuf, "    ");
	rv=Typemaker2_Builder_Invoke_DestructFn(tb, ty, tm,
						GWEN_Buffer_GetStart(dstbuf),
						NULL, /* no dest */
						tbuf);
	if (rv<0) {
	  DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	  GWEN_Buffer_free(dstbuf);
	  GWEN_Buffer_free(tbuf);
	  return rv;
	}
	GWEN_Buffer_free(dstbuf);
	GWEN_Buffer_AppendString(tbuf, "\n");
      }

      tm=Typemaker2_Member_List_Next(tm);
    }
  }

  if (flags & TYPEMAKER2_FLAGS_WITH_REFCOUNT) {
    GWEN_Buffer_AppendString(tbuf, "    p_struct->_refCount=0;\n");
  }

  GWEN_Buffer_AppendString(tbuf, "    GWEN_FREE_OBJECT(p_struct);\n");
  if (flags & TYPEMAKER2_FLAGS_WITH_REFCOUNT) {
    GWEN_Buffer_AppendString(tbuf, "  }\n");
    GWEN_Buffer_AppendString(tbuf, "  else\n");
    GWEN_Buffer_AppendString(tbuf, "    p_struct->_refCount--;\n");
  }
  GWEN_Buffer_AppendString(tbuf, "  }\n");
  GWEN_Buffer_AppendString(tbuf, "}\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



static int _buildGetter(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty) {
  GWEN_BUFFER *tbuf;
  const char *s;
  TYPEMAKER2_MEMBER_LIST *tml;
  uint32_t flags;
  TYPEMAKER2_TYPEMANAGER *tym;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  flags=Typemaker2_Type_GetFlags(ty);

  tml=Typemaker2_Type_GetMembers(ty);
  if (tml) {
    TYPEMAKER2_MEMBER *tm;

    tm=Typemaker2_Member_List_First(tml);
    while(tm) {
      TYPEMAKER2_TYPE *mty;

      mty=Typemaker2_Member_GetTypePtr(tm);
      assert(mty);

      /* prototype */
      GWEN_Buffer_AppendString(tbuf,
			       "/** Getter.\n"
			       " * Use this function to get the member \"");
      s=Typemaker2_Member_GetName(tm);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "\"\n*/\n");
      s=Typemaker2_TypeManager_GetApiDeclaration(tym);
      if (s && Typemaker2_Member_GetAccess(tm)<=TypeMaker2_Access_Protected) {
	GWEN_Buffer_AppendString(tbuf, s);
	GWEN_Buffer_AppendString(tbuf, " ");
      }
      if ((Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Pointer ||
	   Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Array) &&
	  (Typemaker2_Member_GetGetFlags(tm) & TYPEMAKER2_FLAGS_CONST))
	GWEN_Buffer_AppendString(tbuf, "const ");
      if (Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_ENUM) {
	TYPEMAKER2_ENUM *te=Typemaker2_Member_GetEnumPtr(tm);
	const char *s;

	assert(te);
	s=Typemaker2_Enum_GetType(te);
	if (s && *s) {
	  GWEN_Buffer_AppendString(tbuf, s);
	}
	else {
	  s=Typemaker2_Type_GetIdentifier(mty);
	  GWEN_Buffer_AppendString(tbuf, s);
	}
      }
      else {
	s=Typemaker2_Type_GetIdentifier(mty);
	GWEN_Buffer_AppendString(tbuf, s);
      }
      GWEN_Buffer_AppendString(tbuf, " ");
      if (Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Pointer ||
	  Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Array)
	GWEN_Buffer_AppendString(tbuf, "*");

      s=Typemaker2_Type_GetPrefix(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "_Get");
      s=Typemaker2_Member_GetName(tm);
      GWEN_Buffer_AppendByte(tbuf, toupper(*s));
      GWEN_Buffer_AppendString(tbuf, s+1);

      GWEN_Buffer_AppendString(tbuf, "(const ");
      s=Typemaker2_Type_GetIdentifier(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, " *p_struct);\n");

      switch(Typemaker2_Member_GetAccess(tm)) {
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
      if ((Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Pointer ||
	   Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Array) &&
	  (Typemaker2_Member_GetGetFlags(tm) & TYPEMAKER2_FLAGS_CONST))
	GWEN_Buffer_AppendString(tbuf, "const ");
      if (Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_ENUM) {
	TYPEMAKER2_ENUM *te=Typemaker2_Member_GetEnumPtr(tm);
	const char *s;

	assert(te);
	s=Typemaker2_Enum_GetType(te);
	if (s && *s) {
	  GWEN_Buffer_AppendString(tbuf, s);
	}
	else {
	  s=Typemaker2_Type_GetIdentifier(mty);
	  GWEN_Buffer_AppendString(tbuf, s);
	}
      }
      else {
	s=Typemaker2_Type_GetIdentifier(mty);
	GWEN_Buffer_AppendString(tbuf, s);
      }
      GWEN_Buffer_AppendString(tbuf, " ");
      if (Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Pointer ||
	  Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Array)
	GWEN_Buffer_AppendString(tbuf, "*");

      s=Typemaker2_Type_GetPrefix(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "_Get");
      s=Typemaker2_Member_GetName(tm);
      GWEN_Buffer_AppendByte(tbuf, toupper(*s));
      GWEN_Buffer_AppendString(tbuf, s+1);

      GWEN_Buffer_AppendString(tbuf, "(const ");
      s=Typemaker2_Type_GetIdentifier(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, " *p_struct) {\n");

      GWEN_Buffer_AppendString(tbuf, "  assert(p_struct);\n");
      GWEN_Buffer_AppendString(tbuf, "  return p_struct->");
      s=Typemaker2_Member_GetName(tm);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, ";\n");

      GWEN_Buffer_AppendString(tbuf, "}\n");

      Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_Reset(tbuf);

      tm=Typemaker2_Member_List_Next(tm);
    }
  }

  return 0;
}



static int _buildSetter(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty) {
  GWEN_BUFFER *tbuf;
  const char *s;
  TYPEMAKER2_MEMBER_LIST *tml;
  uint32_t flags;
  TYPEMAKER2_TYPEMANAGER *tym;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  flags=Typemaker2_Type_GetFlags(ty);

  tml=Typemaker2_Type_GetMembers(ty);
  if (tml) {
    TYPEMAKER2_MEMBER *tm;

    tm=Typemaker2_Member_List_First(tml);
    while(tm) {
      TYPEMAKER2_TYPE *mty;
      int typeIsWritten=0;

      mty=Typemaker2_Member_GetTypePtr(tm);
      assert(mty);

      /* prototype */
      GWEN_Buffer_AppendString(tbuf,
			       "/** Setter.\n"
			       " * Use this function to set the member \"");
      s=Typemaker2_Member_GetName(tm);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "\"\n*/\n");
      s=Typemaker2_TypeManager_GetApiDeclaration(tym);
      if (s && Typemaker2_Member_GetAccess(tm)<=TypeMaker2_Access_Protected) {
	GWEN_Buffer_AppendString(tbuf, s);
	GWEN_Buffer_AppendString(tbuf, " ");
      }
      GWEN_Buffer_AppendString(tbuf, "void ");
      s=Typemaker2_Type_GetPrefix(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "_Set");
      s=Typemaker2_Member_GetName(tm);
      GWEN_Buffer_AppendByte(tbuf, toupper(*s));
      GWEN_Buffer_AppendString(tbuf, s+1);

      GWEN_Buffer_AppendString(tbuf, "(");
      s=Typemaker2_Type_GetIdentifier(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, " *p_struct, ");

      typeIsWritten=0;
      if (Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_ENUM) {
	TYPEMAKER2_ENUM *te=Typemaker2_Member_GetEnumPtr(tm);
	const char *s;

	assert(te);
	s=Typemaker2_Enum_GetType(te);
	if (s && *s) {
	  GWEN_Buffer_AppendString(tbuf, s);
	  GWEN_Buffer_AppendString(tbuf, " ");
	  GWEN_Buffer_AppendString(tbuf, " p_src");
	  typeIsWritten=1;
	}
      }

      if (!typeIsWritten) {
	if (Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Pointer ||
	    Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Array) {
	  if (Typemaker2_Member_GetSetFlags(tm) & TYPEMAKER2_FLAGS_CONST ||
	      Typemaker2_Member_GetSetFlags(tm) & TYPEMAKER2_FLAGS_DUP)
	    GWEN_Buffer_AppendString(tbuf, "const ");
	  s=Typemaker2_Type_GetIdentifier(mty);
	  GWEN_Buffer_AppendString(tbuf, s);
	  GWEN_Buffer_AppendString(tbuf, " *p_src");
	}
	else {
	  s=Typemaker2_Type_GetIdentifier(mty);
	  GWEN_Buffer_AppendString(tbuf, s);
	  GWEN_Buffer_AppendString(tbuf, " p_src");
	}
      }
      GWEN_Buffer_AppendString(tbuf, ");\n");

      switch(Typemaker2_Member_GetAccess(tm)) {
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

      /* add special functions for flags */
      if (Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_WITH_FLAGS) {
	GWEN_Buffer_AppendString(tbuf,
				 "/** Add flags.\n"
				 " * Use this function to add flags to the member \"");
	s=Typemaker2_Member_GetName(tm);
	GWEN_Buffer_AppendString(tbuf, s);
	GWEN_Buffer_AppendString(tbuf, "\"\n*/\n");
	s=Typemaker2_TypeManager_GetApiDeclaration(tym);
	if (s && Typemaker2_Member_GetAccess(tm)<=TypeMaker2_Access_Protected) {
	  GWEN_Buffer_AppendString(tbuf, s);
	  GWEN_Buffer_AppendString(tbuf, " ");
	}
	GWEN_Buffer_AppendString(tbuf, "void ");
	s=Typemaker2_Type_GetPrefix(ty);
	GWEN_Buffer_AppendString(tbuf, s);
	GWEN_Buffer_AppendString(tbuf, "_Add");
	s=Typemaker2_Member_GetName(tm);
	GWEN_Buffer_AppendByte(tbuf, toupper(*s));
	GWEN_Buffer_AppendString(tbuf, s+1);
  
	GWEN_Buffer_AppendString(tbuf, "(");
	s=Typemaker2_Type_GetIdentifier(ty);
	GWEN_Buffer_AppendString(tbuf, s);
	GWEN_Buffer_AppendString(tbuf, " *p_struct, ");
  
	typeIsWritten=0;
	if (Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_ENUM) {
	  TYPEMAKER2_ENUM *te=Typemaker2_Member_GetEnumPtr(tm);
	  const char *s;
  
	  assert(te);
	  s=Typemaker2_Enum_GetType(te);
	  if (s && *s) {
	    GWEN_Buffer_AppendString(tbuf, s);
	    GWEN_Buffer_AppendString(tbuf, " ");
	    GWEN_Buffer_AppendString(tbuf, " p_src");
	    typeIsWritten=1;
	  }
	}
  
	if (!typeIsWritten) {
	  if (Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Pointer ||
	      Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Array) {
	    if (Typemaker2_Member_GetSetFlags(tm) & TYPEMAKER2_FLAGS_CONST ||
		Typemaker2_Member_GetSetFlags(tm) & TYPEMAKER2_FLAGS_DUP)
	      GWEN_Buffer_AppendString(tbuf, "const ");
	    s=Typemaker2_Type_GetIdentifier(mty);
	    GWEN_Buffer_AppendString(tbuf, s);
	    GWEN_Buffer_AppendString(tbuf, " *p_src");
	  }
	  else {
	    s=Typemaker2_Type_GetIdentifier(mty);
	    GWEN_Buffer_AppendString(tbuf, s);
	    GWEN_Buffer_AppendString(tbuf, " p_src");
	  }
	}
	GWEN_Buffer_AppendString(tbuf, ");\n");
  
	switch(Typemaker2_Member_GetAccess(tm)) {
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

	GWEN_Buffer_AppendString(tbuf,
				 "/** Sub flags.\n"
				 " * Use this function to sub flags to the member \"");
	s=Typemaker2_Member_GetName(tm);
	GWEN_Buffer_AppendString(tbuf, s);
	GWEN_Buffer_AppendString(tbuf, "\"\n*/\n");
	s=Typemaker2_TypeManager_GetApiDeclaration(tym);
	if (s && Typemaker2_Member_GetAccess(tm)<=TypeMaker2_Access_Protected) {
	  GWEN_Buffer_AppendString(tbuf, s);
	  GWEN_Buffer_AppendString(tbuf, " ");
	}
	GWEN_Buffer_AppendString(tbuf, "void ");
	s=Typemaker2_Type_GetPrefix(ty);
	GWEN_Buffer_AppendString(tbuf, s);
	GWEN_Buffer_AppendString(tbuf, "_Sub");
	s=Typemaker2_Member_GetName(tm);
	GWEN_Buffer_AppendByte(tbuf, toupper(*s));
	GWEN_Buffer_AppendString(tbuf, s+1);
  
	GWEN_Buffer_AppendString(tbuf, "(");
	s=Typemaker2_Type_GetIdentifier(ty);
	GWEN_Buffer_AppendString(tbuf, s);
	GWEN_Buffer_AppendString(tbuf, " *p_struct, ");
  
	typeIsWritten=0;
	if (Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_ENUM) {
	  TYPEMAKER2_ENUM *te=Typemaker2_Member_GetEnumPtr(tm);
	  const char *s;
  
	  assert(te);
	  s=Typemaker2_Enum_GetType(te);
	  if (s && *s) {
	    GWEN_Buffer_AppendString(tbuf, s);
	    GWEN_Buffer_AppendString(tbuf, " ");
	    GWEN_Buffer_AppendString(tbuf, " p_src");
	    typeIsWritten=1;
	  }
	}
  
	if (!typeIsWritten) {
	  if (Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Pointer ||
	      Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Array) {
	    if (Typemaker2_Member_GetSetFlags(tm) & TYPEMAKER2_FLAGS_CONST ||
		Typemaker2_Member_GetSetFlags(tm) & TYPEMAKER2_FLAGS_DUP)
	      GWEN_Buffer_AppendString(tbuf, "const ");
	    s=Typemaker2_Type_GetIdentifier(mty);
	    GWEN_Buffer_AppendString(tbuf, s);
	    GWEN_Buffer_AppendString(tbuf, " *p_src");
	  }
	  else {
	    s=Typemaker2_Type_GetIdentifier(mty);
	    GWEN_Buffer_AppendString(tbuf, s);
	    GWEN_Buffer_AppendString(tbuf, " p_src");
	  }
	}
	GWEN_Buffer_AppendString(tbuf, ");\n");
  
	switch(Typemaker2_Member_GetAccess(tm)) {
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
      }


      /* implementation */
      GWEN_Buffer_AppendString(tbuf, "void ");
      s=Typemaker2_Type_GetPrefix(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "_Set");
      s=Typemaker2_Member_GetName(tm);
      GWEN_Buffer_AppendByte(tbuf, toupper(*s));
      GWEN_Buffer_AppendString(tbuf, s+1);

      GWEN_Buffer_AppendString(tbuf, "(");
      s=Typemaker2_Type_GetIdentifier(ty);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, " *p_struct, ");

      typeIsWritten=0;
      if (Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_ENUM) {
	TYPEMAKER2_ENUM *te=Typemaker2_Member_GetEnumPtr(tm);
	const char *s;

	assert(te);
	s=Typemaker2_Enum_GetType(te);
	if (s && *s) {
	  GWEN_Buffer_AppendString(tbuf, s);
	  GWEN_Buffer_AppendString(tbuf, " ");
	  GWEN_Buffer_AppendString(tbuf, " p_src");
	  typeIsWritten=1;
	}
      }

      if (!typeIsWritten) {
	if (Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Pointer ||
	    Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Array) {
	  if (Typemaker2_Member_GetSetFlags(tm) & TYPEMAKER2_FLAGS_CONST ||
	      Typemaker2_Member_GetSetFlags(tm) & TYPEMAKER2_FLAGS_DUP)
	    GWEN_Buffer_AppendString(tbuf, "const ");
	  s=Typemaker2_Type_GetIdentifier(mty);
	  GWEN_Buffer_AppendString(tbuf, s);
	  GWEN_Buffer_AppendString(tbuf, " *p_src");
	}
	else {
	  s=Typemaker2_Type_GetIdentifier(mty);
	  GWEN_Buffer_AppendString(tbuf, s);
	  GWEN_Buffer_AppendString(tbuf, " p_src");
	}
      }
      GWEN_Buffer_AppendString(tbuf, ") {\n");

      GWEN_Buffer_AppendString(tbuf, "  assert(p_struct);\n");
      if (Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Pointer &&
	  (Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_OWN)) {
	GWEN_Buffer_AppendString(tbuf, "  if (p_struct->");
	s=Typemaker2_Member_GetName(tm);
	GWEN_Buffer_AppendString(tbuf, s);
	GWEN_Buffer_AppendString(tbuf, ") {\n");

	/* free */
	if (1) {
	  GWEN_BUFFER *srcbuf;
          int rv;
    
	  srcbuf=GWEN_Buffer_new(0, 256, 0, 1);
	  GWEN_Buffer_AppendString(srcbuf, "p_struct->");
	  s=Typemaker2_Member_GetName(tm);
	  GWEN_Buffer_AppendString(srcbuf, s);

	  GWEN_Buffer_AppendString(tbuf, "    ");
	  rv=Typemaker2_Builder_Invoke_DestructFn(tb, ty, tm,
						  GWEN_Buffer_GetStart(srcbuf),
						  NULL, /* no dest */
						  tbuf);
	  if (rv<0) {
	    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	    GWEN_Buffer_free(srcbuf);
	    GWEN_Buffer_free(tbuf);
	    return rv;
	  }
	  GWEN_Buffer_AppendString(tbuf, "\n");
	  GWEN_Buffer_free(srcbuf);
	}

	GWEN_Buffer_AppendString(tbuf, "  }\n");
      }

      if (Typemaker2_Member_GetSetFlags(tm) & TYPEMAKER2_FLAGS_DUP) {
	if (Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Pointer)
	  GWEN_Buffer_AppendString(tbuf, "  if (p_src) {\n");

        /* dup */
	if (1) {
	  GWEN_BUFFER *dstbuf;
	  int rv;

	  dstbuf=GWEN_Buffer_new(0, 256, 0, 1);
	  GWEN_Buffer_AppendString(dstbuf, "p_struct->");
	  s=Typemaker2_Member_GetName(tm);
	  GWEN_Buffer_AppendString(dstbuf, s);

	  GWEN_Buffer_AppendString(tbuf, "    ");
	  rv=Typemaker2_Builder_Invoke_DupFn(tb, ty, tm,
					     "p_src",
					     GWEN_Buffer_GetStart(dstbuf),
					     tbuf);
	  if (rv<0) {
	    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	    GWEN_Buffer_free(dstbuf);
	    GWEN_Buffer_free(tbuf);
	    return rv;
	  }
	  GWEN_Buffer_AppendString(tbuf, "\n");
	  GWEN_Buffer_free(dstbuf);
	}

	if (Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Pointer) {
	  GWEN_Buffer_AppendString(tbuf, "  }\n");
	  GWEN_Buffer_AppendString(tbuf, "  else {\n");

          /* construct */
	  if (1) {
	    GWEN_BUFFER *dstbuf;
	    int rv;

	    dstbuf=GWEN_Buffer_new(0, 256, 0, 1);
	    GWEN_Buffer_AppendString(dstbuf, "p_struct->");
	    s=Typemaker2_Member_GetName(tm);
	    GWEN_Buffer_AppendString(dstbuf, s);
    
	    GWEN_Buffer_AppendString(tbuf, "    ");
	    rv=Typemaker2_Builder_Invoke_ConstructFn(tb, ty, tm,
						     NULL,
						     GWEN_Buffer_GetStart(dstbuf),
						     tbuf);
	    if (rv<0) {
	      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	      GWEN_Buffer_free(dstbuf);
	      GWEN_Buffer_free(tbuf);
	      return rv;
	    }
	    GWEN_Buffer_AppendString(tbuf, "\n");
	    GWEN_Buffer_free(dstbuf);
	  }
	  else {
	    GWEN_Buffer_AppendString(tbuf, "    p_struct->");
	    s=Typemaker2_Member_GetName(tm);
	    GWEN_Buffer_AppendString(tbuf, s);
	    GWEN_Buffer_AppendString(tbuf, " = ");
	    s=Typemaker2_Member_GetPresetValue(tm);
            if (s && *s)
	      GWEN_Buffer_AppendString(tbuf, s);
            else
	      GWEN_Buffer_AppendString(tbuf, "NULL");
	    GWEN_Buffer_AppendString(tbuf, ";\n");
	  }

	  GWEN_Buffer_AppendString(tbuf, "  }\n");
	}
      }
      else {
	if (1) {
	  GWEN_BUFFER *dstbuf;
          int rv;

	  dstbuf=GWEN_Buffer_new(0, 256, 0, 1);
	  GWEN_Buffer_AppendString(dstbuf, "p_struct->");
	  s=Typemaker2_Member_GetName(tm);
	  GWEN_Buffer_AppendString(dstbuf, s);

	  GWEN_Buffer_AppendString(tbuf, "  ");
	  rv=Typemaker2_Builder_Invoke_AssignFn(tb, ty, tm,
						"p_src",
						GWEN_Buffer_GetStart(dstbuf),
						tbuf);
	  if (rv<0) {
	    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	    GWEN_Buffer_free(dstbuf);
	    GWEN_Buffer_free(tbuf);
	    return rv;
	  }
	  GWEN_Buffer_AppendString(tbuf, "\n");
	  GWEN_Buffer_free(dstbuf);
	}
      }

      GWEN_Buffer_AppendString(tbuf, "}\n");

      Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_Reset(tbuf);

      if (Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_WITH_FLAGS) {
        /* add flags */
	GWEN_Buffer_AppendString(tbuf, "void ");
	s=Typemaker2_Type_GetPrefix(ty);
	GWEN_Buffer_AppendString(tbuf, s);
	GWEN_Buffer_AppendString(tbuf, "_Add");
	s=Typemaker2_Member_GetName(tm);
	GWEN_Buffer_AppendByte(tbuf, toupper(*s));
	GWEN_Buffer_AppendString(tbuf, s+1);
  
	GWEN_Buffer_AppendString(tbuf, "(");
	s=Typemaker2_Type_GetIdentifier(ty);
	GWEN_Buffer_AppendString(tbuf, s);
	GWEN_Buffer_AppendString(tbuf, " *p_struct, ");
  
	typeIsWritten=0;
	if (Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_ENUM) {
	  TYPEMAKER2_ENUM *te=Typemaker2_Member_GetEnumPtr(tm);
	  const char *s;
  
	  assert(te);
	  s=Typemaker2_Enum_GetType(te);
	  if (s && *s) {
	    GWEN_Buffer_AppendString(tbuf, s);
	    GWEN_Buffer_AppendString(tbuf, " ");
	    GWEN_Buffer_AppendString(tbuf, " p_src");
	    typeIsWritten=1;
	  }
	}
  
	if (!typeIsWritten) {
	  s=Typemaker2_Type_GetIdentifier(mty);
	  GWEN_Buffer_AppendString(tbuf, s);
	  GWEN_Buffer_AppendString(tbuf, " p_src");
	}
	GWEN_Buffer_AppendString(tbuf, ") {\n");
  
	GWEN_Buffer_AppendString(tbuf, "  assert(p_struct);\n");
	GWEN_Buffer_AppendString(tbuf, "  p_struct->");
	s=Typemaker2_Member_GetName(tm);
	GWEN_Buffer_AppendString(tbuf, s);
	GWEN_Buffer_AppendString(tbuf, "|=p_src;\n");
	GWEN_Buffer_AppendString(tbuf, "}\n");

	Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
	GWEN_Buffer_Reset(tbuf);

        /* sub flags */
	GWEN_Buffer_AppendString(tbuf, "void ");
	s=Typemaker2_Type_GetPrefix(ty);
	GWEN_Buffer_AppendString(tbuf, s);
	GWEN_Buffer_AppendString(tbuf, "_Sub");
	s=Typemaker2_Member_GetName(tm);
	GWEN_Buffer_AppendByte(tbuf, toupper(*s));
	GWEN_Buffer_AppendString(tbuf, s+1);
  
	GWEN_Buffer_AppendString(tbuf, "(");
	s=Typemaker2_Type_GetIdentifier(ty);
	GWEN_Buffer_AppendString(tbuf, s);
	GWEN_Buffer_AppendString(tbuf, " *p_struct, ");
  
	typeIsWritten=0;
	if (Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_ENUM) {
	  TYPEMAKER2_ENUM *te=Typemaker2_Member_GetEnumPtr(tm);
	  const char *s;
  
	  assert(te);
	  s=Typemaker2_Enum_GetType(te);
	  if (s && *s) {
	    GWEN_Buffer_AppendString(tbuf, s);
	    GWEN_Buffer_AppendString(tbuf, " ");
	    GWEN_Buffer_AppendString(tbuf, " p_src");
	    typeIsWritten=1;
	  }
	}
  
	if (!typeIsWritten) {
	  s=Typemaker2_Type_GetIdentifier(mty);
	  GWEN_Buffer_AppendString(tbuf, s);
	  GWEN_Buffer_AppendString(tbuf, " p_src");
	}
	GWEN_Buffer_AppendString(tbuf, ") {\n");
  
	GWEN_Buffer_AppendString(tbuf, "  assert(p_struct);\n");
	GWEN_Buffer_AppendString(tbuf, "  p_struct->");
	s=Typemaker2_Member_GetName(tm);
	GWEN_Buffer_AppendString(tbuf, s);
	GWEN_Buffer_AppendString(tbuf, "&=~p_src;\n");
	GWEN_Buffer_AppendString(tbuf, "}\n");

	Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
	GWEN_Buffer_Reset(tbuf);
      }



      tm=Typemaker2_Member_List_Next(tm);
    }
  }

  return 0;
}



static int _buildReadDb(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty) {
  GWEN_BUFFER *tbuf;
  const char *s;
  TYPEMAKER2_MEMBER_LIST *tml;
  uint32_t flags;
  TYPEMAKER2_TYPEMANAGER *tym;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  flags=Typemaker2_Type_GetFlags(ty);

  /* prototype */
  s=Typemaker2_TypeManager_GetApiDeclaration(tym);
  if (s) {
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, " ");
  }
  GWEN_Buffer_AppendString(tbuf, "void ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_ReadDb(");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *p_struct, GWEN_DB_NODE *p_db);\n");
  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_Reset(tbuf);

  /* implementation */
  GWEN_Buffer_AppendString(tbuf, "void ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_ReadDb(");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *p_struct, GWEN_DB_NODE *p_db) {\n");

  GWEN_Buffer_AppendString(tbuf, "  assert(p_struct);\n");

  tml=Typemaker2_Type_GetMembers(ty);
  if (tml) {
    TYPEMAKER2_MEMBER *tm;

    tm=Typemaker2_Member_List_First(tml);
    while(tm) {
      TYPEMAKER2_TYPE *mty;

      mty=Typemaker2_Member_GetTypePtr(tm);
      assert(mty);

      GWEN_Buffer_AppendString(tbuf, "  /* member \"");
      s=Typemaker2_Member_GetName(tm);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "\" */\n");

      if (Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Pointer &&
	  (Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_OWN)) {
	GWEN_Buffer_AppendString(tbuf, "  if (p_struct->");
	s=Typemaker2_Member_GetName(tm);
	GWEN_Buffer_AppendString(tbuf, s);
	GWEN_Buffer_AppendString(tbuf, ") {\n");

	/* free */
	if (1) {
	  GWEN_BUFFER *srcbuf;
	  int rv;

	  srcbuf=GWEN_Buffer_new(0, 256, 0, 1);
	  GWEN_Buffer_AppendString(srcbuf, "p_struct->");
	  s=Typemaker2_Member_GetName(tm);
	  GWEN_Buffer_AppendString(srcbuf, s);

	  GWEN_Buffer_AppendString(tbuf, "    ");
	  rv=Typemaker2_Builder_Invoke_DestructFn(tb, ty, tm,
						  GWEN_Buffer_GetStart(srcbuf),
						  NULL, /* no dest */
						  tbuf);
	  if (rv<0) {
	    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	    GWEN_Buffer_free(srcbuf);
	    GWEN_Buffer_free(tbuf);
	    return rv;
	  }
	  GWEN_Buffer_AppendString(tbuf, "\n");
	  GWEN_Buffer_free(srcbuf);
	}
	GWEN_Buffer_AppendString(tbuf, "  }\n");
      }

      /* preset */
      if (1) {
	GWEN_BUFFER *dstbuf;
	int rv;

	dstbuf=GWEN_Buffer_new(0, 256, 0, 1);
	GWEN_Buffer_AppendString(dstbuf, "p_struct->");
	s=Typemaker2_Member_GetName(tm);
	GWEN_Buffer_AppendString(dstbuf, s);

	GWEN_Buffer_AppendString(tbuf, "  ");
	rv=Typemaker2_Builder_Invoke_ConstructFn(tb, ty, tm,
						 NULL, /* no source */
						 GWEN_Buffer_GetStart(dstbuf),
						 tbuf);
	if (rv<0) {
	  DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	  GWEN_Buffer_free(dstbuf);
	  GWEN_Buffer_free(tbuf);
	  return rv;
	}
	GWEN_Buffer_AppendString(tbuf, "\n");
	GWEN_Buffer_free(dstbuf);
      }

      if (!(Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_VOLATILE)) {
	/* from db */
	if (1) {
	  GWEN_BUFFER *dstbuf;
          int rv;

	  dstbuf=GWEN_Buffer_new(0, 256, 0, 1);
	  GWEN_Buffer_AppendString(dstbuf, "p_struct->");
	  s=Typemaker2_Member_GetName(tm);
	  GWEN_Buffer_AppendString(dstbuf, s);

	  GWEN_Buffer_AppendString(tbuf, "  ");
	  rv=Typemaker2_Builder_Invoke_FromDbFn(tb, ty, tm,
						NULL, /* no source */
						GWEN_Buffer_GetStart(dstbuf),
						tbuf);
	  if (rv<0) {
	    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	    GWEN_Buffer_free(dstbuf);
	    GWEN_Buffer_free(tbuf);
	    return rv;
	  }
	  GWEN_Buffer_free(dstbuf);
	  GWEN_Buffer_AppendString(tbuf, "\n");
	}
      }
      else {
	GWEN_Buffer_AppendString(tbuf, "  /* member \"");
	s=Typemaker2_Member_GetName(tm);
	GWEN_Buffer_AppendString(tbuf, s);
	GWEN_Buffer_AppendString(tbuf, "\" is volatile, not reading from db */\n");
      }
      GWEN_Buffer_AppendString(tbuf, "\n");

      tm=Typemaker2_Member_List_Next(tm);
    }
  }

  GWEN_Buffer_AppendString(tbuf, "}\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



static int _buildWriteDb(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty) {
  GWEN_BUFFER *tbuf;
  const char *s;
  TYPEMAKER2_MEMBER_LIST *tml;
  uint32_t flags;
  TYPEMAKER2_TYPEMANAGER *tym;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  flags=Typemaker2_Type_GetFlags(ty);

  /* prototype */
  s=Typemaker2_TypeManager_GetApiDeclaration(tym);
  if (s) {
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, " ");
  }
  GWEN_Buffer_AppendString(tbuf, "int ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_WriteDb(const ");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *p_struct, GWEN_DB_NODE *p_db);\n");
  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_Reset(tbuf);

  /* implementation */
  GWEN_Buffer_AppendString(tbuf, "int ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_WriteDb(const ");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *p_struct, GWEN_DB_NODE *p_db) {\n");

  if (Typemaker2_Type_GetNonVolatileMemberCount(ty)) {
    GWEN_Buffer_AppendString(tbuf, "  int p_rv;\n");
    GWEN_Buffer_AppendString(tbuf, "\n");
  }
  GWEN_Buffer_AppendString(tbuf, "  assert(p_struct);\n");

  tml=Typemaker2_Type_GetMembers(ty);
  if (tml) {
    TYPEMAKER2_MEMBER *tm;

    tm=Typemaker2_Member_List_First(tml);
    while(tm) {
      TYPEMAKER2_TYPE *mty;

      mty=Typemaker2_Member_GetTypePtr(tm);
      assert(mty);

      if (!(Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_VOLATILE)) {
	GWEN_Buffer_AppendString(tbuf, "  /* member \"");
	s=Typemaker2_Member_GetName(tm);
	GWEN_Buffer_AppendString(tbuf, s);
	GWEN_Buffer_AppendString(tbuf, "\" */\n");

	/* toDb */
	if (1) {
	  GWEN_BUFFER *srcbuf;
	  int rv;
  
	  srcbuf=GWEN_Buffer_new(0, 256, 0, 1);
	  GWEN_Buffer_AppendString(srcbuf, "p_struct->");
	  s=Typemaker2_Member_GetName(tm);
	  GWEN_Buffer_AppendString(srcbuf, s);
  
	  GWEN_Buffer_AppendString(tbuf, "  ");
	  rv=Typemaker2_Builder_Invoke_ToDbFn(tb, ty, tm,
					      GWEN_Buffer_GetStart(srcbuf),
                                              NULL, /* no dest */
					      tbuf);
	  if (rv<0) {
	    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	    GWEN_Buffer_free(srcbuf);
	    GWEN_Buffer_free(tbuf);
	    return rv;
	  }
	  GWEN_Buffer_AppendString(tbuf, "\n");
	  GWEN_Buffer_free(srcbuf);
	}

	GWEN_Buffer_AppendString(tbuf, "  if (p_rv<0) {\n");
	GWEN_Buffer_AppendString(tbuf, "    DBG_INFO(GWEN_LOGDOMAIN, \"here (%d)\\n\", p_rv);\n");
	GWEN_Buffer_AppendString(tbuf, "    return p_rv;\n");
	GWEN_Buffer_AppendString(tbuf, "  }\n");

      }
      else {
	GWEN_Buffer_AppendString(tbuf, "  /* member \"");
	s=Typemaker2_Member_GetName(tm);
	GWEN_Buffer_AppendString(tbuf, s);
	GWEN_Buffer_AppendString(tbuf, "\" is volatile, not writing to db */\n");
      }
      GWEN_Buffer_AppendString(tbuf, "\n");

      tm=Typemaker2_Member_List_Next(tm);
    }
  }

  GWEN_Buffer_AppendString(tbuf, "  return 0;\n");
  GWEN_Buffer_AppendString(tbuf, "}\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



static int _buildToDb(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty) {
  GWEN_BUFFER *tbuf;
  const char *s;
  uint32_t flags;
  TYPEMAKER2_TYPEMANAGER *tym;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  flags=Typemaker2_Type_GetFlags(ty);

  /* prototype */
  s=Typemaker2_TypeManager_GetApiDeclaration(tym);
  if (s) {
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, " ");
  }
  GWEN_Buffer_AppendString(tbuf, "int ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_toDb(const ");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *p_struct, GWEN_DB_NODE *p_db);\n");
  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_Reset(tbuf);

  /* implementation */
  GWEN_Buffer_AppendString(tbuf, "int ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_toDb(const ");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *p_struct, GWEN_DB_NODE *p_db) {\n");

  GWEN_Buffer_AppendString(tbuf, "  return ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_WriteDb(p_struct, p_db);\n");
  GWEN_Buffer_AppendString(tbuf, "}\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



static int _buildFromDb(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty) {
  GWEN_BUFFER *tbuf;
  const char *s;
  uint32_t flags;
  TYPEMAKER2_TYPEMANAGER *tym;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  flags=Typemaker2_Type_GetFlags(ty);

  /* prototype */
  s=Typemaker2_TypeManager_GetApiDeclaration(tym);
  if (s) {
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, " ");
  }
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_fromDb(GWEN_DB_NODE *p_db);\n");
  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_Reset(tbuf);

  /* implementation */
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_fromDb(GWEN_DB_NODE *p_db) {\n");

  GWEN_Buffer_AppendString(tbuf, "  ");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *p_struct;\n");

  GWEN_Buffer_AppendString(tbuf, "  p_struct=");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_new();\n");

  GWEN_Buffer_AppendString(tbuf, "  ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_ReadDb(p_struct, p_db);\n");

  GWEN_Buffer_AppendString(tbuf, "  return p_struct;\n");
  GWEN_Buffer_AppendString(tbuf, "}\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



static int _buildReadXml(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty) {
  GWEN_BUFFER *tbuf;
  const char *s;
  TYPEMAKER2_MEMBER_LIST *tml;
  uint32_t flags;
  TYPEMAKER2_TYPEMANAGER *tym;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  flags=Typemaker2_Type_GetFlags(ty);

  /* prototype */
  s=Typemaker2_TypeManager_GetApiDeclaration(tym);
  if (s) {
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, " ");
  }
  GWEN_Buffer_AppendString(tbuf, "void ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_ReadXml(");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *p_struct, GWEN_XMLNODE *p_db);\n");
  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_Reset(tbuf);

  /* implementation */
  GWEN_Buffer_AppendString(tbuf, "void ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_ReadXml(");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *p_struct, GWEN_XMLNODE *p_db) {\n");

  GWEN_Buffer_AppendString(tbuf, "  assert(p_struct);\n");

  tml=Typemaker2_Type_GetMembers(ty);
  if (tml) {
    TYPEMAKER2_MEMBER *tm;

    tm=Typemaker2_Member_List_First(tml);
    while(tm) {
      TYPEMAKER2_TYPE *mty;

      mty=Typemaker2_Member_GetTypePtr(tm);
      assert(mty);

      GWEN_Buffer_AppendString(tbuf, "  /* member \"");
      s=Typemaker2_Member_GetName(tm);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "\" */\n");

      if (Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Pointer &&
	  (Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_OWN)) {
	GWEN_Buffer_AppendString(tbuf, "  if (p_struct->");
	s=Typemaker2_Member_GetName(tm);
	GWEN_Buffer_AppendString(tbuf, s);
	GWEN_Buffer_AppendString(tbuf, ") {\n");

	/* free */
	if (1) {
	  GWEN_BUFFER *srcbuf;
	  int rv;

	  srcbuf=GWEN_Buffer_new(0, 256, 0, 1);
	  GWEN_Buffer_AppendString(srcbuf, "p_struct->");
	  s=Typemaker2_Member_GetName(tm);
	  GWEN_Buffer_AppendString(srcbuf, s);

	  GWEN_Buffer_AppendString(tbuf, "    ");
	  rv=Typemaker2_Builder_Invoke_DestructFn(tb, ty, tm,
						  GWEN_Buffer_GetStart(srcbuf),
						  NULL, /* no dest */
						  tbuf);
	  if (rv<0) {
	    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	    GWEN_Buffer_free(srcbuf);
	    GWEN_Buffer_free(tbuf);
	    return rv;
	  }
	  GWEN_Buffer_AppendString(tbuf, "\n");
	  GWEN_Buffer_free(srcbuf);
	}
	GWEN_Buffer_AppendString(tbuf, "  }\n");
      }

      /* preset */
      if (1) {
	GWEN_BUFFER *dstbuf;
	int rv;

	dstbuf=GWEN_Buffer_new(0, 256, 0, 1);
	GWEN_Buffer_AppendString(dstbuf, "p_struct->");
	s=Typemaker2_Member_GetName(tm);
	GWEN_Buffer_AppendString(dstbuf, s);

	GWEN_Buffer_AppendString(tbuf, "  ");
	rv=Typemaker2_Builder_Invoke_ConstructFn(tb, ty, tm,
						 NULL, /* no source */
						 GWEN_Buffer_GetStart(dstbuf),
						 tbuf);
	if (rv<0) {
	  DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	  GWEN_Buffer_free(dstbuf);
	  GWEN_Buffer_free(tbuf);
	  return rv;
	}
	GWEN_Buffer_AppendString(tbuf, "\n");
	GWEN_Buffer_free(dstbuf);
      }

      if (!(Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_VOLATILE)) {
	/* from xml */
	if (1) {
	  GWEN_BUFFER *dstbuf;
          int rv;

	  dstbuf=GWEN_Buffer_new(0, 256, 0, 1);
	  GWEN_Buffer_AppendString(dstbuf, "p_struct->");
	  s=Typemaker2_Member_GetName(tm);
	  GWEN_Buffer_AppendString(dstbuf, s);

	  GWEN_Buffer_AppendString(tbuf, "  ");
	  rv=Typemaker2_Builder_Invoke_FromXmlFn(tb, ty, tm,
						 NULL, /* no source */
						 GWEN_Buffer_GetStart(dstbuf),
						 tbuf);
	  if (rv<0) {
	    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	    GWEN_Buffer_free(dstbuf);
	    GWEN_Buffer_free(tbuf);
	    return rv;
	  }
	  GWEN_Buffer_free(dstbuf);
	  GWEN_Buffer_AppendString(tbuf, "\n");
	}
      }
      else {
	GWEN_Buffer_AppendString(tbuf, "  /* member \"");
	s=Typemaker2_Member_GetName(tm);
	GWEN_Buffer_AppendString(tbuf, s);
	GWEN_Buffer_AppendString(tbuf, "\" is volatile, not reading from xml */\n");
      }
      GWEN_Buffer_AppendString(tbuf, "\n");

      tm=Typemaker2_Member_List_Next(tm);
    }
  }

  GWEN_Buffer_AppendString(tbuf, "}\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



static int _buildWriteXml(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty) {
  GWEN_BUFFER *tbuf;
  const char *s;
  TYPEMAKER2_MEMBER_LIST *tml;
  uint32_t flags;
  TYPEMAKER2_TYPEMANAGER *tym;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  flags=Typemaker2_Type_GetFlags(ty);

  /* prototype */
  s=Typemaker2_TypeManager_GetApiDeclaration(tym);
  if (s) {
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, " ");
  }
  GWEN_Buffer_AppendString(tbuf, "void ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_WriteXml(const ");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *p_struct, GWEN_XMLNODE *p_db);\n");
  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_Reset(tbuf);

  /* implementation */
  GWEN_Buffer_AppendString(tbuf, "void ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_WriteXml(const ");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *p_struct, GWEN_XMLNODE *p_db) {\n");

  GWEN_Buffer_AppendString(tbuf, "  assert(p_struct);\n");

  tml=Typemaker2_Type_GetMembers(ty);
  if (tml) {
    TYPEMAKER2_MEMBER *tm;

    tm=Typemaker2_Member_List_First(tml);
    while(tm) {
      TYPEMAKER2_TYPE *mty;

      mty=Typemaker2_Member_GetTypePtr(tm);
      assert(mty);

      if (!(Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_VOLATILE)) {
	GWEN_Buffer_AppendString(tbuf, "  /* member \"");
	s=Typemaker2_Member_GetName(tm);
	GWEN_Buffer_AppendString(tbuf, s);
	GWEN_Buffer_AppendString(tbuf, "\" */\n");

	/* toXml */
	if (1) {
	  GWEN_BUFFER *srcbuf;
	  int rv;
  
	  srcbuf=GWEN_Buffer_new(0, 256, 0, 1);
	  GWEN_Buffer_AppendString(srcbuf, "p_struct->");
	  s=Typemaker2_Member_GetName(tm);
	  GWEN_Buffer_AppendString(srcbuf, s);
  
	  GWEN_Buffer_AppendString(tbuf, "  ");
	  rv=Typemaker2_Builder_Invoke_ToXmlFn(tb, ty, tm,
					       GWEN_Buffer_GetStart(srcbuf),
					       NULL, /* no dest */
					       tbuf);
	  if (rv<0) {
	    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	    GWEN_Buffer_free(srcbuf);
	    GWEN_Buffer_free(tbuf);
	    return rv;
	  }
	  GWEN_Buffer_AppendString(tbuf, "\n");
	  GWEN_Buffer_free(srcbuf);
	}

      }
      else {
	GWEN_Buffer_AppendString(tbuf, "  /* member \"");
	s=Typemaker2_Member_GetName(tm);
	GWEN_Buffer_AppendString(tbuf, s);
	GWEN_Buffer_AppendString(tbuf, "\" is volatile, not writing to xml */\n");
      }
      GWEN_Buffer_AppendString(tbuf, "\n");

      tm=Typemaker2_Member_List_Next(tm);
    }
  }

  GWEN_Buffer_AppendString(tbuf, "}\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



static int _buildToXml(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty) {
  GWEN_BUFFER *tbuf;
  const char *s;
  uint32_t flags;
  TYPEMAKER2_TYPEMANAGER *tym;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  flags=Typemaker2_Type_GetFlags(ty);

  /* prototype */
  s=Typemaker2_TypeManager_GetApiDeclaration(tym);
  if (s) {
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, " ");
  }
  GWEN_Buffer_AppendString(tbuf, "void ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_toXml(const ");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *p_struct, GWEN_XMLNODE *p_db);\n");
  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_Reset(tbuf);

  /* implementation */
  GWEN_Buffer_AppendString(tbuf, "void ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_toXml(const ");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *p_struct, GWEN_XMLNODE *p_db) {\n");

  GWEN_Buffer_AppendString(tbuf, "  ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_WriteXml(p_struct, p_db);\n");
  GWEN_Buffer_AppendString(tbuf, "}\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



static int _buildFromXml(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty) {
  GWEN_BUFFER *tbuf;
  const char *s;
  uint32_t flags;
  TYPEMAKER2_TYPEMANAGER *tym;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  flags=Typemaker2_Type_GetFlags(ty);

  /* prototype */
  s=Typemaker2_TypeManager_GetApiDeclaration(tym);
  if (s) {
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, " ");
  }
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_fromXml(GWEN_XMLNODE *p_db);\n");
  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_Reset(tbuf);

  /* implementation */
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_fromXml(GWEN_XMLNODE *p_db) {\n");

  GWEN_Buffer_AppendString(tbuf, "  ");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *p_struct;\n");

  GWEN_Buffer_AppendString(tbuf, "  p_struct=");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_new();\n");

  GWEN_Buffer_AppendString(tbuf, "  ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_ReadXml(p_struct, p_db);\n");

  GWEN_Buffer_AppendString(tbuf, "  return p_struct;\n");
  GWEN_Buffer_AppendString(tbuf, "}\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



static int _buildReadObject(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty) {
  GWEN_BUFFER *tbuf;
  const char *s;
  TYPEMAKER2_MEMBER_LIST *tml;
  uint32_t flags;
  TYPEMAKER2_TYPEMANAGER *tym;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  flags=Typemaker2_Type_GetFlags(ty);

  /* prototype */
  s=Typemaker2_TypeManager_GetApiDeclaration(tym);
  if (s) {
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, " ");
  }
  GWEN_Buffer_AppendString(tbuf, "int ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_ReadObject(");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *p_struct, const AEDB_OBJECT *p_db);\n");
  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_Reset(tbuf);

  /* implementation */
  GWEN_Buffer_AppendString(tbuf, "int ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_ReadObject(");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *p_struct, const AEDB_OBJECT *p_db) {\n");

  if (Typemaker2_Type_GetNonVolatileMemberCount(ty)) {
    GWEN_Buffer_AppendString(tbuf, "  int p_rv;\n");
    GWEN_Buffer_AppendString(tbuf, "\n");
  }
  GWEN_Buffer_AppendString(tbuf, "  assert(p_struct);\n");

  tml=Typemaker2_Type_GetMembers(ty);
  if (tml) {
    TYPEMAKER2_MEMBER *tm;

    tm=Typemaker2_Member_List_First(tml);
    while(tm) {
      TYPEMAKER2_TYPE *mty;

      mty=Typemaker2_Member_GetTypePtr(tm);
      assert(mty);

      GWEN_Buffer_AppendString(tbuf, "  /* member \"");
      s=Typemaker2_Member_GetName(tm);
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "\" */\n");

      if (Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Pointer &&
	  (Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_OWN)) {
	GWEN_Buffer_AppendString(tbuf, "  if (p_struct->");
	s=Typemaker2_Member_GetName(tm);
	GWEN_Buffer_AppendString(tbuf, s);
	GWEN_Buffer_AppendString(tbuf, ") {\n");

	/* free */
	if (1) {
	  GWEN_BUFFER *srcbuf;
	  int rv;

	  srcbuf=GWEN_Buffer_new(0, 256, 0, 1);
	  GWEN_Buffer_AppendString(srcbuf, "p_struct->");
	  s=Typemaker2_Member_GetName(tm);
	  GWEN_Buffer_AppendString(srcbuf, s);

	  GWEN_Buffer_AppendString(tbuf, "    ");
	  rv=Typemaker2_Builder_Invoke_DestructFn(tb, ty, tm,
						  GWEN_Buffer_GetStart(srcbuf),
						  NULL, /* no dest */
						  tbuf);
	  if (rv<0) {
	    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	    GWEN_Buffer_free(srcbuf);
	    GWEN_Buffer_free(tbuf);
	    return rv;
	  }
	  GWEN_Buffer_AppendString(tbuf, "\n");
	  GWEN_Buffer_free(srcbuf);
	}
	GWEN_Buffer_AppendString(tbuf, "  }\n");
      }

      /* preset */
      if (1) {
	GWEN_BUFFER *dstbuf;
	int rv;

	dstbuf=GWEN_Buffer_new(0, 256, 0, 1);
	GWEN_Buffer_AppendString(dstbuf, "p_struct->");
	s=Typemaker2_Member_GetName(tm);
	GWEN_Buffer_AppendString(dstbuf, s);

	GWEN_Buffer_AppendString(tbuf, "  ");
	rv=Typemaker2_Builder_Invoke_ConstructFn(tb, ty, tm,
						 NULL, /* no source */
						 GWEN_Buffer_GetStart(dstbuf),
						 tbuf);
	if (rv<0) {
	  DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	  GWEN_Buffer_free(dstbuf);
	  GWEN_Buffer_free(tbuf);
	  return rv;
	}
	GWEN_Buffer_AppendString(tbuf, "\n");
	GWEN_Buffer_free(dstbuf);
      }

      if (!(Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_VOLATILE)) {
	/* from object */
	if (1) {
	  GWEN_BUFFER *dstbuf;
          int rv;

	  dstbuf=GWEN_Buffer_new(0, 256, 0, 1);
	  GWEN_Buffer_AppendString(dstbuf, "p_struct->");
	  s=Typemaker2_Member_GetName(tm);
	  GWEN_Buffer_AppendString(dstbuf, s);

	  GWEN_Buffer_AppendString(tbuf, "  ");
	  rv=Typemaker2_Builder_Invoke_FromObjectFn(tb, ty, tm,
						    NULL, /* no source */
						    GWEN_Buffer_GetStart(dstbuf),
						    tbuf);
	  if (rv<0) {
	    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	    GWEN_Buffer_free(dstbuf);
	    GWEN_Buffer_free(tbuf);
	    return rv;
	  }
	  GWEN_Buffer_free(dstbuf);
	  GWEN_Buffer_AppendString(tbuf, "\n");
	}

	GWEN_Buffer_AppendString(tbuf, "  if (p_rv<0) {\n");
	GWEN_Buffer_AppendString(tbuf, "    DBG_INFO(GWEN_LOGDOMAIN, \"here (%d)\\n\", p_rv);\n");
	GWEN_Buffer_AppendString(tbuf, "    return p_rv;\n");
	GWEN_Buffer_AppendString(tbuf, "  }\n");
      }
      else {
	GWEN_Buffer_AppendString(tbuf, "  /* member \"");
	s=Typemaker2_Member_GetName(tm);
	GWEN_Buffer_AppendString(tbuf, s);
	GWEN_Buffer_AppendString(tbuf, "\" is volatile, not reading from object */\n");
      }
      GWEN_Buffer_AppendString(tbuf, "\n");

      tm=Typemaker2_Member_List_Next(tm);
    }
  }

  GWEN_Buffer_AppendString(tbuf, "  return 0;\n");
  GWEN_Buffer_AppendString(tbuf, "}\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



static int _buildWriteObject(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty) {
  GWEN_BUFFER *tbuf;
  const char *s;
  TYPEMAKER2_MEMBER_LIST *tml;
  uint32_t flags;
  TYPEMAKER2_TYPEMANAGER *tym;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  flags=Typemaker2_Type_GetFlags(ty);

  /* prototype */
  s=Typemaker2_TypeManager_GetApiDeclaration(tym);
  if (s) {
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, " ");
  }
  GWEN_Buffer_AppendString(tbuf, "int ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_WriteObject(const ");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *p_struct, AEDB_OBJECT *p_db);\n");
  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_Reset(tbuf);

  /* implementation */
  GWEN_Buffer_AppendString(tbuf, "int ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_WriteObject(const ");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *p_struct, AEDB_OBJECT *p_db) {\n");

  if (Typemaker2_Type_GetNonVolatileMemberCount(ty)) {
    GWEN_Buffer_AppendString(tbuf, "  int p_rv;\n");
    GWEN_Buffer_AppendString(tbuf, "\n");
  }
  GWEN_Buffer_AppendString(tbuf, "  assert(p_struct);\n");

  tml=Typemaker2_Type_GetMembers(ty);
  if (tml) {
    TYPEMAKER2_MEMBER *tm;

    tm=Typemaker2_Member_List_First(tml);
    while(tm) {
      TYPEMAKER2_TYPE *mty;

      mty=Typemaker2_Member_GetTypePtr(tm);
      assert(mty);

      if (!(Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_VOLATILE)) {
	GWEN_Buffer_AppendString(tbuf, "  /* member \"");
	s=Typemaker2_Member_GetName(tm);
	GWEN_Buffer_AppendString(tbuf, s);
	GWEN_Buffer_AppendString(tbuf, "\" */\n");

	/* toObject */
	if (1) {
	  GWEN_BUFFER *srcbuf;
	  int rv;
  
	  srcbuf=GWEN_Buffer_new(0, 256, 0, 1);
	  GWEN_Buffer_AppendString(srcbuf, "p_struct->");
	  s=Typemaker2_Member_GetName(tm);
	  GWEN_Buffer_AppendString(srcbuf, s);
  
	  GWEN_Buffer_AppendString(tbuf, "  ");
	  rv=Typemaker2_Builder_Invoke_ToObjectFn(tb, ty, tm,
						  GWEN_Buffer_GetStart(srcbuf),
						  NULL, /* no dest */
						  tbuf);
	  if (rv<0) {
	    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	    GWEN_Buffer_free(srcbuf);
	    GWEN_Buffer_free(tbuf);
	    return rv;
	  }
	  GWEN_Buffer_AppendString(tbuf, "\n");
	  GWEN_Buffer_free(srcbuf);
	}

	GWEN_Buffer_AppendString(tbuf, "  if (p_rv<0) {\n");
	GWEN_Buffer_AppendString(tbuf, "    DBG_INFO(GWEN_LOGDOMAIN, \"here (%d)\\n\", p_rv);\n");
	GWEN_Buffer_AppendString(tbuf, "    return p_rv;\n");
	GWEN_Buffer_AppendString(tbuf, "  }\n");

      }
      else {
	GWEN_Buffer_AppendString(tbuf, "  /* member \"");
	s=Typemaker2_Member_GetName(tm);
	GWEN_Buffer_AppendString(tbuf, s);
	GWEN_Buffer_AppendString(tbuf, "\" is volatile, not writing to object */\n");
      }
      GWEN_Buffer_AppendString(tbuf, "\n");

      tm=Typemaker2_Member_List_Next(tm);
    }
  }

  GWEN_Buffer_AppendString(tbuf, "  return 0;\n");

  GWEN_Buffer_AppendString(tbuf, "}\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



static int _buildToObject(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty) {
  GWEN_BUFFER *tbuf;
  const char *s;
  uint32_t flags;
  TYPEMAKER2_TYPEMANAGER *tym;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  flags=Typemaker2_Type_GetFlags(ty);

  /* prototype */
  s=Typemaker2_TypeManager_GetApiDeclaration(tym);
  if (s) {
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, " ");
  }
  GWEN_Buffer_AppendString(tbuf, "int ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_toObject(const ");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *p_struct, AEDB_OBJECT *p_db);\n");
  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_Reset(tbuf);

  /* implementation */
  GWEN_Buffer_AppendString(tbuf, "int ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_toObject(const ");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *p_struct, AEDB_OBJECT *p_db) {\n");

  GWEN_Buffer_AppendString(tbuf, "  return ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_WriteObject(p_struct, p_db);\n");
  GWEN_Buffer_AppendString(tbuf, "}\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



static int _buildFromObject(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty) {
  GWEN_BUFFER *tbuf;
  const char *s;
  uint32_t flags;
  TYPEMAKER2_TYPEMANAGER *tym;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  flags=Typemaker2_Type_GetFlags(ty);

  /* prototype */
  s=Typemaker2_TypeManager_GetApiDeclaration(tym);
  if (s) {
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, " ");
  }
  GWEN_Buffer_AppendString(tbuf, "int ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_fromObject(const AEDB_OBJECT *p_db, ");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " **pp_struct);\n");
  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_Reset(tbuf);

  /* implementation */
  GWEN_Buffer_AppendString(tbuf, "int ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_fromObject(const AEDB_OBJECT *p_db, ");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " **pp_struct) {\n");

  GWEN_Buffer_AppendString(tbuf, "  ");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *p_struct;\n");
  GWEN_Buffer_AppendString(tbuf, "  int p_rv;\n");

  GWEN_Buffer_AppendString(tbuf, "  p_struct=");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_new();\n");

  GWEN_Buffer_AppendString(tbuf, "  p_rv=");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_ReadObject(p_struct, p_db);\n");

  GWEN_Buffer_AppendString(tbuf, "  if (p_rv<0) {\n");
  GWEN_Buffer_AppendString(tbuf, "    DBG_INFO(GWEN_LOGDOMAIN, \"here (%d)\", p_rv);\n");
  GWEN_Buffer_AppendString(tbuf, "    ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_free(p_struct);\n");
  GWEN_Buffer_AppendString(tbuf, "    return p_rv;\n");
  GWEN_Buffer_AppendString(tbuf, "  }\n");

  GWEN_Buffer_AppendString(tbuf, "  *pp_struct=p_struct;\n");
  GWEN_Buffer_AppendString(tbuf, "  return 0;\n");
  GWEN_Buffer_AppendString(tbuf, "}\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



static int _buildCreateColumnList(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty) {
  GWEN_BUFFER *tbuf;
  const char *s;
  TYPEMAKER2_MEMBER_LIST *tml;
  uint32_t flags;
  TYPEMAKER2_TYPEMANAGER *tym;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  flags=Typemaker2_Type_GetFlags(ty);

  /* prototype */
  s=Typemaker2_TypeManager_GetApiDeclaration(tym);
  if (s) {
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, " ");
  }
  GWEN_Buffer_AppendString(tbuf, "AEDB_COLUMN_LIST *");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_CreateColumnList();\n");
  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_Reset(tbuf);

  /* implementation */
  GWEN_Buffer_AppendString(tbuf, "AEDB_COLUMN_LIST *");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_CreateColumnList() {\n");

  if (Typemaker2_Type_GetNonVolatileMemberCount(ty)==0) {
    GWEN_Buffer_AppendString(tbuf, "  return NULL;\n");
  }
  else {
    GWEN_Buffer_AppendString(tbuf, "  AEDB_COLUMN_LIST *p_cl;\n");
    GWEN_Buffer_AppendString(tbuf, "  AEDB_COLUMN *p_c;\n");
    GWEN_Buffer_AppendString(tbuf, "\n");

    GWEN_Buffer_AppendString(tbuf, "  p_cl=AEDB_Column_List_new();\n");
    GWEN_Buffer_AppendString(tbuf, "\n");

    tml=Typemaker2_Type_GetMembers(ty);
    if (tml) {
      TYPEMAKER2_MEMBER *tm;

      tm=Typemaker2_Member_List_First(tml);
      while(tm) {
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

	  GWEN_Buffer_AppendString(tbuf, "  p_c=AEDB_Column_new(");
	  s=Typemaker2_Type_GetAeDbType(mty);
	  if (s && *s) {
	    GWEN_Buffer_AppendString(tbuf, s);
	  }
	  else {
	    DBG_ERROR(GWEN_LOGDOMAIN, "Type has no AEDB type element");
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
	  GWEN_Buffer_AppendString(tbuf, "  AEDB_Column_List_Add(p_c, p_cl);\n");
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



static int _buildDup(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty) {
  GWEN_BUFFER *tbuf;
  const char *s;
  TYPEMAKER2_MEMBER_LIST *tml;
  uint32_t flags;
  TYPEMAKER2_TYPEMANAGER *tym;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  flags=Typemaker2_Type_GetFlags(ty);

  /* prototype */
  s=Typemaker2_TypeManager_GetApiDeclaration(tym);
  if (s) {
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, " ");
  }
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_dup(const ");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *p_struct);\n");
  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_Reset(tbuf);

  /* implementation */
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_dup(const ");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *p_src) {\n");

  GWEN_Buffer_AppendString(tbuf, "  ");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *p_struct;\n");
  GWEN_Buffer_AppendString(tbuf, "\n");

  GWEN_Buffer_AppendString(tbuf, "  assert(p_src);\n");
  GWEN_Buffer_AppendString(tbuf, "  p_struct=");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_new();\n");

  tml=Typemaker2_Type_GetMembers(ty);
  if (tml) {
    TYPEMAKER2_MEMBER *tm;

    tm=Typemaker2_Member_List_First(tml);
    while(tm) {
      TYPEMAKER2_TYPE *mty;

      mty=Typemaker2_Member_GetTypePtr(tm);
      assert(mty);

      if (!(Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_NODUP)) {
	GWEN_Buffer_AppendString(tbuf, "  /* member \"");
	s=Typemaker2_Member_GetName(tm);
	GWEN_Buffer_AppendString(tbuf, s);
	GWEN_Buffer_AppendString(tbuf, "\" */\n");

	if (Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Pointer) {
	  if (Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_OWN) {
	    GWEN_Buffer_AppendString(tbuf, "  if (p_struct->");
	    s=Typemaker2_Member_GetName(tm);
	    GWEN_Buffer_AppendString(tbuf, s);
	    GWEN_Buffer_AppendString(tbuf, ") {\n");

	    /* free */
	    if (1) {
	      GWEN_BUFFER *srcbuf;
	      int rv;

	      srcbuf=GWEN_Buffer_new(0, 256, 0, 1);
	      GWEN_Buffer_AppendString(srcbuf, "p_struct->");
	      s=Typemaker2_Member_GetName(tm);
	      GWEN_Buffer_AppendString(srcbuf, s);

	      GWEN_Buffer_AppendString(tbuf, "    ");
	      rv=Typemaker2_Builder_Invoke_DestructFn(tb, ty, tm,
						      GWEN_Buffer_GetStart(srcbuf),
						      NULL, /* no dest */
						      tbuf);
	      if (rv<0) {
		DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
		GWEN_Buffer_free(srcbuf);
		GWEN_Buffer_free(tbuf);
		return rv;
	      }
	      GWEN_Buffer_AppendString(tbuf, "\n");
	      GWEN_Buffer_free(srcbuf);
	    }

	    /* preset with NULL */
	    GWEN_Buffer_AppendString(tbuf, "    p_struct->");
	    s=Typemaker2_Member_GetName(tm);
	    GWEN_Buffer_AppendString(tbuf, s);
	    GWEN_Buffer_AppendString(tbuf, "=NULL;\n");

	    GWEN_Buffer_AppendString(tbuf, "  }\n");
	  }
	  else {
	    /* preset with NULL */
	    GWEN_Buffer_AppendString(tbuf, "  p_struct->");
	    s=Typemaker2_Member_GetName(tm);
	    GWEN_Buffer_AppendString(tbuf, s);
	    GWEN_Buffer_AppendString(tbuf, "=NULL;\n");
	  }

	  GWEN_Buffer_AppendString(tbuf, "  if (p_src->");
	  s=Typemaker2_Member_GetName(tm);
	  GWEN_Buffer_AppendString(tbuf, s);
	  GWEN_Buffer_AppendString(tbuf, ") {\n");

	  /* dup */
	  if (1) {
	    GWEN_BUFFER *dstbuf;
	    GWEN_BUFFER *srcbuf;
	    int rv;
  
	    dstbuf=GWEN_Buffer_new(0, 256, 0, 1);
	    GWEN_Buffer_AppendString(dstbuf, "p_struct->");
	    s=Typemaker2_Member_GetName(tm);
	    GWEN_Buffer_AppendString(dstbuf, s);
  
	    srcbuf=GWEN_Buffer_new(0, 256, 0, 1);
	    GWEN_Buffer_AppendString(srcbuf, "p_src->");
	    s=Typemaker2_Member_GetName(tm);
	    GWEN_Buffer_AppendString(srcbuf, s);
  
	    GWEN_Buffer_AppendString(tbuf, "    ");
	    rv=Typemaker2_Builder_Invoke_DupFn(tb, ty, tm,
					       GWEN_Buffer_GetStart(srcbuf),
					       GWEN_Buffer_GetStart(dstbuf),
					       tbuf);
	    if (rv<0) {
	      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	      GWEN_Buffer_free(srcbuf);
	      GWEN_Buffer_free(dstbuf);
	      GWEN_Buffer_free(tbuf);
	      return rv;
	    }
	    GWEN_Buffer_AppendString(tbuf, "\n");
	    GWEN_Buffer_free(srcbuf);
	    GWEN_Buffer_free(dstbuf);
	  }
	  GWEN_Buffer_AppendString(tbuf, "  }\n");
	} /* if pointer */
	else {
	  /* dup */
	  if (1) {
	    GWEN_BUFFER *dstbuf;
	    GWEN_BUFFER *srcbuf;
	    int rv;

	    dstbuf=GWEN_Buffer_new(0, 256, 0, 1);
	    GWEN_Buffer_AppendString(dstbuf, "p_struct->");
	    s=Typemaker2_Member_GetName(tm);
	    GWEN_Buffer_AppendString(dstbuf, s);

	    srcbuf=GWEN_Buffer_new(0, 256, 0, 1);
	    GWEN_Buffer_AppendString(srcbuf, "p_src->");
	    s=Typemaker2_Member_GetName(tm);
	    GWEN_Buffer_AppendString(srcbuf, s);

	    GWEN_Buffer_AppendString(tbuf, "  ");
	    rv=Typemaker2_Builder_Invoke_DupFn(tb, ty, tm,
					       GWEN_Buffer_GetStart(srcbuf),
					       GWEN_Buffer_GetStart(dstbuf),
					       tbuf);
	    if (rv<0) {
	      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	      GWEN_Buffer_free(srcbuf);
	      GWEN_Buffer_free(dstbuf);
	      GWEN_Buffer_free(tbuf);
	      return rv;
	    }
	    GWEN_Buffer_AppendString(tbuf, "\n");
	    GWEN_Buffer_free(srcbuf);
	    GWEN_Buffer_free(dstbuf);
	  }
	}

      } /* if !nodup */
      else {
	GWEN_Buffer_AppendString(tbuf, "  /* member \"");
	s=Typemaker2_Member_GetName(tm);
	GWEN_Buffer_AppendString(tbuf, s);
	GWEN_Buffer_AppendString(tbuf, "\" has nodup flag, not copying */\n");
      }

      GWEN_Buffer_AppendString(tbuf, "\n");
      tm=Typemaker2_Member_List_Next(tm);
    }
  }

  GWEN_Buffer_AppendString(tbuf, "  return p_struct;\n");

  GWEN_Buffer_AppendString(tbuf, "}\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



static int _buildList1Dup(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty) {
  GWEN_BUFFER *tbuf;
  const char *s;
  uint32_t flags;
  TYPEMAKER2_TYPEMANAGER *tym;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  flags=Typemaker2_Type_GetFlags(ty);

  /* prototype */
  s=Typemaker2_TypeManager_GetApiDeclaration(tym);
  if (s) {
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, " ");
  }
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_LIST *");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_List_dup(const ");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_LIST *p_src);\n");
  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_Reset(tbuf);

  /* implementation */
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_LIST *");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_List_dup(const ");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_LIST *p_src) {\n");

  GWEN_Buffer_AppendString(tbuf, "  ");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_LIST *p_dest;\n");

  GWEN_Buffer_AppendString(tbuf, "  ");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *p_elem;\n");
  GWEN_Buffer_AppendString(tbuf, "\n");

  GWEN_Buffer_AppendString(tbuf, "  assert(p_src);\n");
  GWEN_Buffer_AppendString(tbuf, "  p_dest=");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_List_new();\n");

  GWEN_Buffer_AppendString(tbuf, "  p_elem=");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_List_First(p_src);\n");

  GWEN_Buffer_AppendString(tbuf, "  while(p_elem) {\n");
  GWEN_Buffer_AppendString(tbuf, "    ");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *p_cpy;\n");
  GWEN_Buffer_AppendString(tbuf, "\n");

  GWEN_Buffer_AppendString(tbuf, "    p_cpy=");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_dup(p_elem);\n");

  GWEN_Buffer_AppendString(tbuf, "    ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_List_Add(p_cpy, p_dest);\n");

  GWEN_Buffer_AppendString(tbuf, "    p_elem=");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_List_Next(p_elem);\n");
  GWEN_Buffer_AppendString(tbuf, "  }\n");
  GWEN_Buffer_AppendString(tbuf, "\n");

  GWEN_Buffer_AppendString(tbuf, "  return p_dest;\n");
  GWEN_Buffer_AppendString(tbuf, "}\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



static int _buildCreateTable(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty) {
  GWEN_BUFFER *tbuf;
  const char *s;
  uint32_t flags;
  TYPEMAKER2_TYPEMANAGER *tym;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  flags=Typemaker2_Type_GetFlags(ty);

  /* prototype */
  s=Typemaker2_TypeManager_GetApiDeclaration(tym);
  if (s) {
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, " ");
  }
  GWEN_Buffer_AppendString(tbuf, "int ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_CreateTable(AEDB_DB *p_db, const char *p_name, uint32_t p_flags);");
  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_Reset(tbuf);

  /* implementation */
  GWEN_Buffer_AppendString(tbuf, "int ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_CreateTable(AEDB_DB *p_db, const char *p_name, uint32_t p_flags) {\n");

  GWEN_Buffer_AppendString(tbuf, "  AEDB_COLUMN_LIST *p_cl;\n");
  GWEN_Buffer_AppendString(tbuf, "  int p_rv;\n");
  GWEN_Buffer_AppendString(tbuf, "\n");

  GWEN_Buffer_AppendString(tbuf, "  p_cl=");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_CreateColumnList();\n");

  GWEN_Buffer_AppendString(tbuf, "  p_rv=AEDB_DB_CreateTable(p_db, p_name, p_flags, p_cl);\n");
  GWEN_Buffer_AppendString(tbuf, "  AEDB_Column_List_free(p_cl);\n");
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



static int _buildCreateObject(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty) {
  GWEN_BUFFER *tbuf;
  const char *s;
  uint32_t flags;
  TYPEMAKER2_TYPEMANAGER *tym;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  flags=Typemaker2_Type_GetFlags(ty);

  /* prototype */
  s=Typemaker2_TypeManager_GetApiDeclaration(tym);
  if (s) {
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, " ");
  }
  GWEN_Buffer_AppendString(tbuf, "int ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_CreateObject(AEDB_DB *p_db, AEDB_ID p_tableId, AEDB_ID p_id, AEDB_OBJECT **pp_o);\n");
  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_Reset(tbuf);

  /* implementation */
  GWEN_Buffer_AppendString(tbuf, "int ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_CreateObject(AEDB_DB *p_db, AEDB_ID p_tableId, AEDB_ID p_id, AEDB_OBJECT **pp_o) {\n");

  GWEN_Buffer_AppendString(tbuf, "  AEDB_OBJECT *p_o;\n");
  GWEN_Buffer_AppendString(tbuf, "\n");

  GWEN_Buffer_AppendString(tbuf, "  p_o=AEDB_Object_new(p_tableId, p_id, ");
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



static int _buildDefineEnums(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty) {
  TYPEMAKER2_ENUM_LIST *enums;
  TYPEMAKER2_TYPEMANAGER *tym;
  uint32_t flags;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  enums=Typemaker2_Type_GetEnums(ty);
  flags=Typemaker2_Type_GetFlags(ty);

  assert(enums);
  if (Typemaker2_Enum_List_GetCount(enums)) {
    GWEN_BUFFER *tbuf;
    const char *s;
    TYPEMAKER2_ENUM *te;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);

    te=Typemaker2_Enum_List_First(enums);
    while(te) {
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
      while(ti) {
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

      switch(access) {
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



static int _setEnumStringFns(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty) {
  TYPEMAKER2_ENUM_LIST *enums;
  TYPEMAKER2_TYPEMANAGER *tym;
  uint32_t flags;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  enums=Typemaker2_Type_GetEnums(ty);
  flags=Typemaker2_Type_GetFlags(ty);

  assert(enums);
  if (Typemaker2_Enum_List_GetCount(enums)) {
    TYPEMAKER2_ENUM *te;

    te=Typemaker2_Enum_List_First(enums);
    while(te) {
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



static int _buildEnumFromString(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty) {
  TYPEMAKER2_ENUM_LIST *enums;
  TYPEMAKER2_TYPEMANAGER *tym;
  uint32_t flags;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  enums=Typemaker2_Type_GetEnums(ty);
  flags=Typemaker2_Type_GetFlags(ty);

  assert(enums);
  if (Typemaker2_Enum_List_GetCount(enums)) {
    GWEN_BUFFER *tbuf;
    const char *s;
    TYPEMAKER2_ENUM *te;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);

    te=Typemaker2_Enum_List_First(enums);
    while(te) {
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
      if (s) {
	GWEN_Buffer_AppendString(tbuf, s);
	GWEN_Buffer_AppendString(tbuf, " ");
      }
      if (etype && *etype) {
	GWEN_Buffer_AppendString(tbuf, etype);
	GWEN_Buffer_AppendString(tbuf, " ");
      }
      else
	GWEN_Buffer_AppendString(tbuf, "int ");
      if (prefix && *prefix) {
	GWEN_Buffer_AppendString(tbuf, prefix);
	if (prefix[strlen(prefix)-1]!='_')
	  GWEN_Buffer_AppendString(tbuf, "_");
      }
      GWEN_Buffer_AppendString(tbuf, "fromString(const char *p_s);\n");
      switch(acc) {
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
      if (etype && *etype) {
	GWEN_Buffer_AppendString(tbuf, etype);
	GWEN_Buffer_AppendString(tbuf, " ");
      }
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
      while(ti) {
	GWEN_Buffer_AppendString(tbuf, "    ");
	if (i++)
	  GWEN_Buffer_AppendString(tbuf, "else ");
	GWEN_Buffer_AppendString(tbuf, "if (strcasecmp(p_s, \"");
	s=Typemaker2_Item_GetName(ti);
	if (s && *s) {
	  GWEN_Buffer_AppendByte(tbuf, tolower(*s));
	  GWEN_Buffer_AppendString(tbuf, s+1);
	}
	GWEN_Buffer_AppendString(tbuf, "\")==0)\n");
	GWEN_Buffer_AppendString(tbuf, "      return ");
	if (prefix)
	  GWEN_Buffer_AppendString(tbuf, prefix);
	s=Typemaker2_Item_GetName(ti);
	if (s && *s) {
	  GWEN_Buffer_AppendByte(tbuf, toupper(*s));
	  GWEN_Buffer_AppendString(tbuf, s+1);
	}
	GWEN_Buffer_AppendString(tbuf, ";\n");

	ti=Typemaker2_Item_List_Next(ti);
      }
      GWEN_Buffer_AppendString(tbuf, "  }\n");
      GWEN_Buffer_AppendString(tbuf, "  return ");
      if (prefix)
	GWEN_Buffer_AppendString(tbuf, prefix);
      GWEN_Buffer_AppendString(tbuf, "Unknown;\n");
      GWEN_Buffer_AppendString(tbuf, "}\n");

      Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_Reset(tbuf);
      te=Typemaker2_Enum_List_Next(te);
    }
    GWEN_Buffer_free(tbuf);
  } /* if enums */

  return 0;
}



static int _buildEnumToString(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty) {
  TYPEMAKER2_ENUM_LIST *enums;
  TYPEMAKER2_TYPEMANAGER *tym;
  uint32_t flags;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  enums=Typemaker2_Type_GetEnums(ty);
  flags=Typemaker2_Type_GetFlags(ty);

  assert(enums);
  if (Typemaker2_Enum_List_GetCount(enums)) {
    GWEN_BUFFER *tbuf;
    const char *s;
    TYPEMAKER2_ENUM *te;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);

    te=Typemaker2_Enum_List_First(enums);
    while(te) {
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

      switch(acc) {
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
      while(ti) {
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



static int _buildList1GetByMember(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm) {
  GWEN_BUFFER *tbuf;
  const char *s;
  uint32_t flags;
  TYPEMAKER2_TYPEMANAGER *tym;
  TYPEMAKER2_TYPE *mty;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  flags=Typemaker2_Type_GetFlags(ty);

  mty=Typemaker2_Member_GetTypePtr(tm);
  assert(mty);

  /* prototype */
  s=Typemaker2_TypeManager_GetApiDeclaration(tym);
  if (s) {
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, " ");
  }

  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *");

  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_List_Get");
  GWEN_Buffer_AppendString(tbuf, "By");
  s=Typemaker2_Member_GetName(tm);
  GWEN_Buffer_AppendByte(tbuf, toupper(*s));
  GWEN_Buffer_AppendString(tbuf, s+1);
  GWEN_Buffer_AppendString(tbuf, "(const ");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_LIST *p_list, ");
  if (Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Pointer ||
      Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Array) {
    GWEN_Buffer_AppendString(tbuf, " const ");
    s=Typemaker2_Type_GetIdentifier(mty);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, " *");
  }
  else {
    s=Typemaker2_Type_GetIdentifier(mty);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, " ");
  }
  GWEN_Buffer_AppendString(tbuf, "p_cmp);\n");

  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_Reset(tbuf);

  /* implementation */
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *");

  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_List_Get");
  GWEN_Buffer_AppendString(tbuf, "By");
  s=Typemaker2_Member_GetName(tm);
  GWEN_Buffer_AppendByte(tbuf, toupper(*s));
  GWEN_Buffer_AppendString(tbuf, s+1);
  GWEN_Buffer_AppendString(tbuf, "(const ");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_LIST *p_list, ");
  if (Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Pointer ||
      Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Array) {
    GWEN_Buffer_AppendString(tbuf, " const ");
    s=Typemaker2_Type_GetIdentifier(mty);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, " *");
  }
  else {
    s=Typemaker2_Type_GetIdentifier(mty);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, " ");
  }
  GWEN_Buffer_AppendString(tbuf, "p_cmp) {\n");

  GWEN_Buffer_AppendString(tbuf, "  ");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *p_struct;\n");
  GWEN_Buffer_AppendString(tbuf, "\n");

  GWEN_Buffer_AppendString(tbuf, "  assert(p_list);\n");
  GWEN_Buffer_AppendString(tbuf, "  p_struct = ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_List_First(p_list);\n");
  GWEN_Buffer_AppendString(tbuf, "  while(p_struct) {\n");
  GWEN_Buffer_AppendString(tbuf, "    int p_rv;\n");
  GWEN_Buffer_AppendString(tbuf, "\n");

  GWEN_Buffer_AppendString(tbuf, "    ");
  if (1) {
    GWEN_BUFFER *dstbuf;
    GWEN_BUFFER *srcbuf;
    int rv;

    srcbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(srcbuf, "p_cmp");

    dstbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(dstbuf, "p_struct->");
    s=Typemaker2_Member_GetName(tm);
    GWEN_Buffer_AppendString(dstbuf, s);

    rv=Typemaker2_Builder_Invoke_CompareFn(tb, ty, tm,
					   GWEN_Buffer_GetStart(srcbuf),
					   GWEN_Buffer_GetStart(dstbuf),
					   tbuf);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(srcbuf);
      GWEN_Buffer_free(dstbuf);
      GWEN_Buffer_free(tbuf);
      return rv;
    }
    GWEN_Buffer_AppendString(tbuf, "\n");
    GWEN_Buffer_free(srcbuf);
    GWEN_Buffer_free(dstbuf);
  }


  GWEN_Buffer_AppendString(tbuf, "    if (p_rv == 0)\n");
  GWEN_Buffer_AppendString(tbuf, "      return p_struct;\n");

  GWEN_Buffer_AppendString(tbuf, "    p_struct = ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_List_Next(p_struct);\n");

  GWEN_Buffer_AppendString(tbuf, "  }\n");

  GWEN_Buffer_AppendString(tbuf, "  return NULL;\n");
  GWEN_Buffer_AppendString(tbuf, "}\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



static int _buildTreeGetByMember(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm) {
  GWEN_BUFFER *tbuf;
  const char *s;
  uint32_t flags;
  TYPEMAKER2_TYPEMANAGER *tym;
  TYPEMAKER2_TYPE *mty;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  flags=Typemaker2_Type_GetFlags(ty);

  mty=Typemaker2_Member_GetTypePtr(tm);
  assert(mty);

  /* prototype */
  s=Typemaker2_TypeManager_GetApiDeclaration(tym);
  if (s) {
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, " ");
  }

  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *");

  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_Tree_Get");
  GWEN_Buffer_AppendString(tbuf, "By");
  s=Typemaker2_Member_GetName(tm);
  GWEN_Buffer_AppendByte(tbuf, toupper(*s));
  GWEN_Buffer_AppendString(tbuf, s+1);
  GWEN_Buffer_AppendString(tbuf, "(const ");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_TREE *p_tree, ");
  if (Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Pointer ||
      Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Array) {
    GWEN_Buffer_AppendString(tbuf, " const ");
    s=Typemaker2_Type_GetIdentifier(mty);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, " *");
  }
  else {
    s=Typemaker2_Type_GetIdentifier(mty);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, " ");
  }
  GWEN_Buffer_AppendString(tbuf, "p_cmp);\n");

  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_Reset(tbuf);

  /* implementation */
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *");

  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_Tree_Get");
  GWEN_Buffer_AppendString(tbuf, "By");
  s=Typemaker2_Member_GetName(tm);
  GWEN_Buffer_AppendByte(tbuf, toupper(*s));
  GWEN_Buffer_AppendString(tbuf, s+1);
  GWEN_Buffer_AppendString(tbuf, "(const ");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_TREE *p_tree, ");
  if (Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Pointer ||
      Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Array) {
    GWEN_Buffer_AppendString(tbuf, " const ");
    s=Typemaker2_Type_GetIdentifier(mty);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, " *");
  }
  else {
    s=Typemaker2_Type_GetIdentifier(mty);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, " ");
  }
  GWEN_Buffer_AppendString(tbuf, "p_cmp) {\n");

  GWEN_Buffer_AppendString(tbuf, "  ");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *p_struct;\n");
  GWEN_Buffer_AppendString(tbuf, "\n");

  GWEN_Buffer_AppendString(tbuf, "  assert(p_tree);\n");
  GWEN_Buffer_AppendString(tbuf, "  p_struct = ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_Tree_GetFirst(p_tree);\n");
  GWEN_Buffer_AppendString(tbuf, "  while(p_struct) {\n");
  GWEN_Buffer_AppendString(tbuf, "    int p_rv;\n");
  GWEN_Buffer_AppendString(tbuf, "\n");

  GWEN_Buffer_AppendString(tbuf, "    ");
  if (1) {
    GWEN_BUFFER *dstbuf;
    GWEN_BUFFER *srcbuf;
    int rv;

    srcbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(srcbuf, "p_cmp");

    dstbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(dstbuf, "p_struct->");
    s=Typemaker2_Member_GetName(tm);
    GWEN_Buffer_AppendString(dstbuf, s);

    rv=Typemaker2_Builder_Invoke_CompareFn(tb, ty, tm,
					   GWEN_Buffer_GetStart(srcbuf),
					   GWEN_Buffer_GetStart(dstbuf),
					   tbuf);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(srcbuf);
      GWEN_Buffer_free(dstbuf);
      GWEN_Buffer_free(tbuf);
      return rv;
    }
    GWEN_Buffer_AppendString(tbuf, "\n");
    GWEN_Buffer_free(srcbuf);
    GWEN_Buffer_free(dstbuf);
  }


  GWEN_Buffer_AppendString(tbuf, "    if (p_rv == 0)\n");
  GWEN_Buffer_AppendString(tbuf, "      return p_struct;\n");

  GWEN_Buffer_AppendString(tbuf, "    p_struct = ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_Tree_GetBelow(p_struct);\n");

  GWEN_Buffer_AppendString(tbuf, "  }\n");

  GWEN_Buffer_AppendString(tbuf, "  return NULL;\n");
  GWEN_Buffer_AppendString(tbuf, "}\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



static int _buildGetByMember(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty) {
  GWEN_BUFFER *tbuf;
  TYPEMAKER2_MEMBER_LIST *tml;
  uint32_t flags;
  TYPEMAKER2_TYPEMANAGER *tym;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  flags=Typemaker2_Type_GetFlags(ty);

  tml=Typemaker2_Type_GetMembers(ty);
  if (tml) {
    TYPEMAKER2_MEMBER *tm;

    tm=Typemaker2_Member_List_First(tml);
    while(tm) {
      TYPEMAKER2_TYPE *mty;
      int rv;

      mty=Typemaker2_Member_GetTypePtr(tm);
      assert(mty);

      if (Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_WITH_GETBYMEMBER) {
	if (flags & TYPEMAKER2_FLAGS_WITH_LIST1) {
	  rv=_buildList1GetByMember(tb, ty, tm);
	  if (rv<0) {
	    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	    return rv;
	  }
	}

	if (flags & TYPEMAKER2_FLAGS_WITH_TREE) {
	  rv=_buildTreeGetByMember(tb, ty, tm);
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



static int _buildDefineDefines(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty) {
  TYPEMAKER2_DEFINE_LIST *defines;
  TYPEMAKER2_TYPEMANAGER *tym;
  uint32_t flags;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  defines=Typemaker2_Type_GetDefines(ty);
  flags=Typemaker2_Type_GetFlags(ty);

  assert(defines);
  if (Typemaker2_Define_List_GetCount(defines)) {
    GWEN_BUFFER *tbuf;
    const char *s;
    TYPEMAKER2_DEFINE *td;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);

    td=Typemaker2_Define_List_First(defines);
    while(td) {
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
      while(ti) {
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

	  switch(Typemaker2_Define_GetMode(td)) {
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

      switch(access) {
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



static int _buildToHashString(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty) {
  GWEN_BUFFER *tbuf;
  const char *s;
  uint32_t flags;
  TYPEMAKER2_TYPEMANAGER *tym;
  TYPEMAKER2_MEMBER_LIST *tml;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  flags=Typemaker2_Type_GetFlags(ty);

  /* prototype */
  s=Typemaker2_TypeManager_GetApiDeclaration(tym);
  if (s) {
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, " ");
  }

  GWEN_Buffer_AppendString(tbuf, "void ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_toHashString");
  GWEN_Buffer_AppendString(tbuf, "(const ");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *p_struct, GWEN_BUFFER *p_buffer);\n");

  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_Reset(tbuf);

  /* implementation */
  GWEN_Buffer_AppendString(tbuf, "void ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_toHashString");
  GWEN_Buffer_AppendString(tbuf, "(const ");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *p_struct, GWEN_BUFFER *p_buffer) {\n");

  GWEN_Buffer_AppendString(tbuf, "  assert(p_struct);\n");


  tml=Typemaker2_Type_GetMembers(ty);
  if (tml) {
    TYPEMAKER2_MEMBER *tm;

    tm=Typemaker2_Member_List_First(tml);
    while(tm) {
      TYPEMAKER2_TYPE *mty;

      mty=Typemaker2_Member_GetTypePtr(tm);
      assert(mty);

      if (Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_WITH_HASH) {
	GWEN_Buffer_AppendString(tbuf, "  /* member \"");
	s=Typemaker2_Member_GetName(tm);
	GWEN_Buffer_AppendString(tbuf, s);
	GWEN_Buffer_AppendString(tbuf, "\" */\n");

        /* toHashString */
	if (1) {
	  GWEN_BUFFER *srcbuf;
	  int rv;
    
	  srcbuf=GWEN_Buffer_new(0, 256, 0, 1);
	  GWEN_Buffer_AppendString(srcbuf, "p_struct->");
	  s=Typemaker2_Member_GetName(tm);
	  GWEN_Buffer_AppendString(srcbuf, s);
  
	  GWEN_Buffer_AppendString(tbuf, "  ");
	  rv=Typemaker2_Builder_Invoke_ToHashStringFn(tb, ty, tm,
						      GWEN_Buffer_GetStart(srcbuf),
						      NULL, /* no dest */
						      tbuf);
	  if (rv<0) {
	    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	    GWEN_Buffer_free(srcbuf);
	    GWEN_Buffer_free(tbuf);
	    return rv;
	  }
	  GWEN_Buffer_AppendString(tbuf, "\n");
	  GWEN_Buffer_free(srcbuf);
	}

	GWEN_Buffer_AppendString(tbuf, "  GWEN_Buffer_AppendByte(p_buffer, \':\');\n");
      }
      else {
	GWEN_Buffer_AppendString(tbuf, "  /* member \"");
	s=Typemaker2_Member_GetName(tm);
	GWEN_Buffer_AppendString(tbuf, s);
	GWEN_Buffer_AppendString(tbuf, "\" does not have the with_hash flag, ignoring */\n");
      }

      tm=Typemaker2_Member_List_Next(tm);
    }
  }


  GWEN_Buffer_AppendString(tbuf, "}\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



static int _buildInlines(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty) {
  GWEN_BUFFER *tbuf;
  uint32_t flags;
  TYPEMAKER2_TYPEMANAGER *tym;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  flags=Typemaker2_Type_GetFlags(ty);

  while(ty) {
    TYPEMAKER2_INLINE *ti;

    ti=Typemaker2_Inline_List_First(Typemaker2_Type_GetInlines(ty));
    while(ti) {
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
	  switch(Typemaker2_Inline_GetLocation(ti)) {
	  case Typemaker2_InlineLocation_Header:
	    switch(Typemaker2_Inline_GetAccess(ti)) {
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



static int _buildAttach(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty) {
  GWEN_BUFFER *tbuf;
  const char *s;
  uint32_t flags;
  TYPEMAKER2_TYPEMANAGER *tym;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  flags=Typemaker2_Type_GetFlags(ty);

  /* prototype */
  s=Typemaker2_TypeManager_GetApiDeclaration(tym);
  if (s) {
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, " ");
  }
  GWEN_Buffer_AppendString(tbuf, "void ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_Attach(");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *p_struct);\n");

  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_Reset(tbuf);

  /* implementation */
  GWEN_Buffer_AppendString(tbuf, "void ");
  s=Typemaker2_Type_GetPrefix(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, "_Attach(");
  s=Typemaker2_Type_GetIdentifier(ty);
  GWEN_Buffer_AppendString(tbuf, s);
  GWEN_Buffer_AppendString(tbuf, " *p_struct) {\n");

  GWEN_Buffer_AppendString(tbuf, "  assert(p_struct);\n");
  GWEN_Buffer_AppendString(tbuf, "  assert(p_struct->_refCount);\n");
  GWEN_Buffer_AppendString(tbuf, "  p_struct->_refCount++;\n");
  GWEN_Buffer_AppendString(tbuf, "}\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



static int Typemaker2_Builder_C_Build(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty) {
  int rv;
  uint32_t flags;

  flags=Typemaker2_Type_GetFlags(ty);

  rv=_setEnumStringFns(tb, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  if (flags & TYPEMAKER2_FLAGS_WITH_OBJECT) {
    rv=_buildFieldIds(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  rv=_buildDefineDefines(tb, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=_buildTypedef(tb, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=_buildDefineEnums(tb, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=_buildPostHeaders(tb, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=_buildStruct(tb, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=_buildMacroFunctions(tb, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=_buildEnumFromString(tb, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=_buildEnumToString(tb, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=_buildConstructor(tb, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=_buildDestructor(tb, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  if (flags & TYPEMAKER2_FLAGS_WITH_REFCOUNT) {
    rv=_buildAttach(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  rv=_buildDup(tb, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=_buildGetter(tb, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=_buildSetter(tb, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  if (flags & TYPEMAKER2_FLAGS_WITH_LIST1) {
    rv=_buildList1Dup(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  if (flags & TYPEMAKER2_FLAGS_WITH_DB) {
    rv=_buildReadDb(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    rv=_buildWriteDb(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    rv=_buildFromDb(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    rv=_buildToDb(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

  }

  if (flags & TYPEMAKER2_FLAGS_WITH_XML) {
    rv=_buildReadXml(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    rv=_buildWriteXml(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    rv=_buildToXml(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    rv=_buildFromXml(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  if (flags & TYPEMAKER2_FLAGS_WITH_OBJECT) {
    rv=_buildReadObject(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    rv=_buildWriteObject(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    rv=_buildToObject(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    rv=_buildFromObject(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    rv=_buildCreateColumnList(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    rv=_buildCreateTable(tb, ty);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    rv=_buildCreateObject(tb, ty);
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

  if (flags & TYPEMAKER2_FLAGS_WITH_HASH) {
    rv=_buildToHashString(tb, ty);
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


  return 0;
}



TYPEMAKER2_BUILDER *Typemaker2_Builder_C_new() {
  TYPEMAKER2_BUILDER *tb;

  tb=Typemaker2_Builder_new();
  Typemaker2_Builder_SetBuildFn(tb, Typemaker2_Builder_C_Build);

  return tb;
}





