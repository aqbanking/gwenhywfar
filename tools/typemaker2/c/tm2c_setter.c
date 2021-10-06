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
#include "tm2c_misc.h"

#include <gwenhywfar/debug.h>

#include <ctype.h>



static int _addSetterPrototype(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm);
static int _addSetterImplementation(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm);
static void _addSetterFnDeclaration(TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm, GWEN_BUFFER *tbuf);
static int _addDupNewValueCode(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm, GWEN_BUFFER *tbuf);
static int _addAssignNewValueCode(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm, GWEN_BUFFER *tbuf);
static int _addFlagFnsPrototype(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm);
static void _addFlagFnsImplementation(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm);




int TM2C_BuildSetter(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty)
{
  TYPEMAKER2_MEMBER_LIST *tml;

  tml=Typemaker2_Type_GetMembers(ty);
  if (tml) {
    TYPEMAKER2_MEMBER *tm;

    tm=Typemaker2_Member_List_First(tml);
    while (tm) {
      if (!(Typemaker2_Member_GetSetFlags(tm) & TYPEMAKER2_FLAGS_OMIT)) {
        TYPEMAKER2_TYPE *mty;
	int rv;

        mty=Typemaker2_Member_GetTypePtr(tm);
        assert(mty);

	rv=_addSetterPrototype(tb, ty, tm);
	if (rv<0) {
	  DBG_INFO(NULL, "here (%d)", rv);
	  return rv;
	}

        if (Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_WITH_FLAGS) {
	  /* add special functions for flags */
	  rv=_addFlagFnsPrototype(tb, ty, tm);
	  if (rv<0) {
	    DBG_INFO(NULL, "here (%d)", rv);
	    return rv;
	  }
	}

	rv=_addSetterImplementation(tb, ty, tm);
	if (rv<0) {
	  DBG_INFO(NULL, "here (%d)", rv);
	  return rv;
	}

	if (Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_WITH_FLAGS)
	  _addFlagFnsImplementation(tb, ty, tm);
      } /* if not OMIT */

      tm=Typemaker2_Member_List_Next(tm);
    }
  }

  return 0;
}



int _addSetterPrototype(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm)
{
  TYPEMAKER2_TYPEMANAGER *tym;
  TYPEMAKER2_TYPE *mty;
  GWEN_BUFFER *tbuf;
  const char *s;

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  tym=Typemaker2_Builder_GetTypeManager(tb);
  mty=Typemaker2_Member_GetTypePtr(tm);
  assert(mty);
  
  /* prototype */
  GWEN_Buffer_AppendArgs(tbuf,
			 "/** Setter.\n"
			 " * Use this function to set the member \"%s\" (see @ref %s_%s)\n*/\n",
			 Typemaker2_Member_GetName(tm),
			 Typemaker2_Type_GetIdentifier(ty),
			 Typemaker2_Member_GetName(tm));

  s=Typemaker2_TypeManager_GetApiDeclaration(tym);
  if (s && Typemaker2_Member_GetAccess(tm)<=TypeMaker2_Access_Protected) {
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendString(tbuf, " ");
  }

  _addSetterFnDeclaration(ty, tm, tbuf);
  GWEN_Buffer_AppendString(tbuf, ";\n");

  switch (Typemaker2_Member_GetAccess(tm)) {
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
  GWEN_Buffer_free(tbuf);

  return 0;
}



void _addSetterFnDeclaration(TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm, GWEN_BUFFER *tbuf)
{
  TYPEMAKER2_TYPE *mty;
  const char *s;

  mty=Typemaker2_Member_GetTypePtr(tm);

  s=Typemaker2_Member_GetName(tm);
  GWEN_Buffer_AppendArgs(tbuf,
			 "void %s_Set%c%s(%s *p_struct, ",
			 Typemaker2_Type_GetPrefix(ty),
                         toupper(*s),
                         s+1,
                         Typemaker2_Type_GetIdentifier(ty));

  if (Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_ENUM) {
    TYPEMAKER2_ENUM *te;
    const char *s;

    te=Typemaker2_Member_GetEnumPtr(tm);
    assert(te);
    s=Typemaker2_Enum_GetType(te);
    if (!(s && *s))
      s=Typemaker2_Type_GetIdentifier(mty);
    GWEN_Buffer_AppendArgs(tbuf, "%s p_src", s);
  }
  else if (Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Pointer ||
	   Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Array) {
    if (Typemaker2_Member_GetSetFlags(tm) & TYPEMAKER2_FLAGS_CONST ||
	Typemaker2_Member_GetSetFlags(tm) & TYPEMAKER2_FLAGS_DUP)
      GWEN_Buffer_AppendString(tbuf, "const ");
    GWEN_Buffer_AppendArgs(tbuf, "%s *p_src", Typemaker2_Type_GetIdentifier(mty));
  }
  else {
    GWEN_Buffer_AppendArgs(tbuf, "%s p_src", Typemaker2_Type_GetIdentifier(mty));
  }
  GWEN_Buffer_AppendString(tbuf, ")");
}



int _addSetterImplementation(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm)
{
  TYPEMAKER2_TYPE *mty;
  GWEN_BUFFER *tbuf;
  int rv;

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  mty=Typemaker2_Member_GetTypePtr(tm);
  assert(mty);

  /* implementation */
  _addSetterFnDeclaration(ty, tm, tbuf);
  GWEN_Buffer_AppendString(tbuf, "{\n");
  
  GWEN_Buffer_AppendString(tbuf, "  assert(p_struct);\n");

  if (Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_OWN) {
    rv=TM2C_AddReleaseOldValueCode(tb, ty, tm, tbuf);
    if (rv<0) {
      DBG_INFO(NULL, "here (%d)", rv);
      GWEN_Buffer_free(tbuf);
      return rv;
    }
  } /* if own */
  

  if (Typemaker2_Member_GetSetFlags(tm) & TYPEMAKER2_FLAGS_DUP) {
    rv=_addDupNewValueCode(tb, ty, tm, tbuf);
    if (rv<0) {
      DBG_INFO(NULL, "here (%d)", rv);
      GWEN_Buffer_free(tbuf);
      return rv;
    }
  }
  else {
    rv=_addAssignNewValueCode(tb, ty, tm, tbuf);
    if (rv<0) {
      DBG_INFO(NULL, "here (%d)", rv);
      GWEN_Buffer_free(tbuf);
      return rv;
    }
  }

  GWEN_Buffer_AppendString(tbuf, "}\n");

  return 0;
}



int _addReleaseOldValueCode(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm, GWEN_BUFFER *tbuf)
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



int _addDupNewValueCode(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm, GWEN_BUFFER *tbuf)
{
  TYPEMAKER2_TYPE *mty;
  GWEN_BUFFER *dstbuf;
  int rv;

  mty=Typemaker2_Member_GetTypePtr(tm);
  assert(mty);

  if (Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Pointer)
    GWEN_Buffer_AppendString(tbuf, "  if (p_src) {\n");
  
  /* dup */
  dstbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendArgs(dstbuf, "p_struct->%s", Typemaker2_Member_GetName(tm));
  GWEN_Buffer_AppendString(tbuf, "    ");
  rv=Typemaker2_Builder_Invoke_DupFn(tb, ty, tm, "p_src", GWEN_Buffer_GetStart(dstbuf), tbuf);
  GWEN_Buffer_free(dstbuf);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  GWEN_Buffer_AppendString(tbuf, "\n");

  if (Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Pointer) {
    GWEN_Buffer_AppendString(tbuf, "  }\n");
    GWEN_Buffer_AppendString(tbuf, "  else {\n");
    GWEN_Buffer_AppendArgs(tbuf,   "    p_struct->%s=NULL;\n", Typemaker2_Member_GetName(tm));
    GWEN_Buffer_AppendString(tbuf, "  }\n");
  }
  else {
    rv=_addAssignNewValueCode(tb, ty, tm, tbuf);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }
  GWEN_Buffer_AppendString(tbuf, "\n");

  return 0;
}



int _addAssignNewValueCode(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm, GWEN_BUFFER *tbuf)
{
  TYPEMAKER2_TYPE *mty;
  GWEN_BUFFER *dstbuf;
  int rv;

  mty=Typemaker2_Member_GetTypePtr(tm);
  assert(mty);

  dstbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendArgs(dstbuf, "p_struct->%s", Typemaker2_Member_GetName(tm));

  GWEN_Buffer_AppendString(tbuf, "  ");
  rv=Typemaker2_Builder_Invoke_AssignFn(tb, ty, tm, "p_src", GWEN_Buffer_GetStart(dstbuf), tbuf);
  GWEN_Buffer_free(dstbuf);
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    return rv;
  }
  GWEN_Buffer_AppendString(tbuf, "\n");

  return 0;
}




int _addFlagFnsPrototype(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm)
{
  TYPEMAKER2_TYPEMANAGER *tym;
  TYPEMAKER2_TYPE *mty;
  GWEN_BUFFER *tbuf;
  const char *sApi;
  const char *s;

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  tym=Typemaker2_Builder_GetTypeManager(tb);
  mty=Typemaker2_Member_GetTypePtr(tm);
  assert(mty);
  
  sApi=Typemaker2_TypeManager_GetApiDeclaration(tym);

  s=Typemaker2_Member_GetName(tm);

  GWEN_Buffer_AppendArgs(tbuf,
			 "/** Add flags.\n"
			 " * Use this function to add flags to member \"%s\" (see @ref %s_%s)\n*/\n",
			 Typemaker2_Member_GetName(tm),
			 Typemaker2_Type_GetIdentifier(ty),
			 Typemaker2_Member_GetName(tm));
  if (sApi && Typemaker2_Member_GetAccess(tm)<=TypeMaker2_Access_Protected)
    GWEN_Buffer_AppendArgs(tbuf, "%s ", sApi);
  GWEN_Buffer_AppendArgs(tbuf,
			 "void %s_Add%c%s(%s *p_struct, %s p_src);\n",
			 Typemaker2_Type_GetPrefix(ty),
			 toupper(*s),
			 s+1,
                         Typemaker2_Type_GetIdentifier(ty),
			 Typemaker2_Type_GetIdentifier(mty));

  GWEN_Buffer_AppendArgs(tbuf,
			 "/** Add flags.\n"
			 " * Use this function to add flags to member \"%s\" (see @ref %s_%s)\n*/\n",
			 Typemaker2_Member_GetName(tm),
			 Typemaker2_Type_GetIdentifier(ty),
			 Typemaker2_Member_GetName(tm));
  if (sApi && Typemaker2_Member_GetAccess(tm)<=TypeMaker2_Access_Protected)
    GWEN_Buffer_AppendArgs(tbuf, "%s ", sApi);
  GWEN_Buffer_AppendArgs(tbuf,
			 "void %s_Sub%c%s(%s *p_struct, %s p_src);\n",
			 Typemaker2_Type_GetPrefix(ty),
			 toupper(*s),
                         s+1,
                         Typemaker2_Type_GetIdentifier(ty),
			 Typemaker2_Type_GetIdentifier(mty));

  switch (Typemaker2_Member_GetAccess(tm)) {
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
  GWEN_Buffer_free(tbuf);

  return 0;
}



void _addFlagFnsImplementation(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm)
{
  TYPEMAKER2_TYPE *mty;
  GWEN_BUFFER *tbuf;
  const char *s;

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  mty=Typemaker2_Member_GetTypePtr(tm);
  assert(mty);
  
  s=Typemaker2_Member_GetName(tm);

  GWEN_Buffer_AppendArgs(tbuf,
			 "void %s_Add%c%s(%s *p_struct, %s p_src) {\n"
			 "  assert(p_struct);\n"
			 "  p_struct->%s|=p_src;\n"
			 "}\n",
			 Typemaker2_Type_GetPrefix(ty),
			 toupper(*s),
			 s+1,
                         Typemaker2_Type_GetIdentifier(ty),
			 Typemaker2_Type_GetIdentifier(mty),
			 s);
  GWEN_Buffer_AppendString(tbuf, "\n\n");

  GWEN_Buffer_AppendArgs(tbuf,
			 "void %s_Sub%c%s(%s *p_struct, %s p_src) {\n"
			 "  assert(p_struct);\n"
			 "  p_struct->%s&=~p_src;\n"
			 "}\n",
			 Typemaker2_Type_GetPrefix(ty),
			 toupper(*s),
			 s+1,
                         Typemaker2_Type_GetIdentifier(ty),
			 Typemaker2_Type_GetIdentifier(mty),
			 s);
  GWEN_Buffer_AppendString(tbuf, "\n\n");


  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);
}




