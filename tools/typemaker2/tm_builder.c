/***************************************************************************
    begin       : Thu Jul 02 2009
    copyright   : (C) 2009 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "tm_builder_p.h"
#include "tm_member.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <assert.h>



GWEN_INHERIT_FUNCTIONS(TYPEMAKER2_BUILDER)


TYPEMAKER2_BUILDER *Typemaker2_Builder_new() {
  TYPEMAKER2_BUILDER *tb;

  GWEN_NEW_OBJECT(TYPEMAKER2_BUILDER, tb);
  GWEN_INHERIT_INIT(TYPEMAKER2_BUILDER, tb);

  tb->declarationsPublic=GWEN_StringList_new();
  tb->declarationsLibrary=GWEN_StringList_new();
  tb->declarationsProtected=GWEN_StringList_new();
  tb->declarationsPrivate=GWEN_StringList_new();
  tb->code=GWEN_StringList_new();

  return tb;
}



void Typemaker2_Builder_free(TYPEMAKER2_BUILDER *tb) {
  if (tb) {
    GWEN_INHERIT_FINI(TYPEMAKER2_BUILDER, tb);

    GWEN_StringList_free(tb->declarationsPublic);
    GWEN_StringList_free(tb->declarationsLibrary);
    GWEN_StringList_free(tb->declarationsProtected);
    GWEN_StringList_free(tb->declarationsPrivate);
    GWEN_StringList_free(tb->code);
    free(tb->fileNamePublic);
    free(tb->fileNameLibrary);
    free(tb->fileNameProtected);
    free(tb->fileNamePrivate);
    free(tb->fileNameCode);
    GWEN_FREE_OBJECT(tb);
  }
}



GWEN_STRINGLIST *Typemaker2_Builder_GetPublicDeclarations(const TYPEMAKER2_BUILDER *tb) {
  assert(tb);
  return tb->declarationsPublic;
}



GWEN_STRINGLIST *Typemaker2_Builder_GetLibraryDeclarations(const TYPEMAKER2_BUILDER *tb) {
  assert(tb);
  return tb->declarationsLibrary;
}



GWEN_STRINGLIST *Typemaker2_Builder_GetProtectedDeclarations(const TYPEMAKER2_BUILDER *tb) {
  assert(tb);
  return tb->declarationsProtected;
}



GWEN_STRINGLIST *Typemaker2_Builder_GetPrivateDeclarations(const TYPEMAKER2_BUILDER *tb) {
  assert(tb);
  return tb->declarationsPrivate;
}



GWEN_STRINGLIST *Typemaker2_Builder_GetCode(const TYPEMAKER2_BUILDER *tb) {
  assert(tb);
  return tb->code;
}



TYPEMAKER2_TYPEMANAGER *Typemaker2_Builder_GetTypeManager(const TYPEMAKER2_BUILDER *tb) {
  assert(tb);
  return tb->typeManager;
}



void Typemaker2_Builder_SetTypeManager(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPEMANAGER *tm) {
  assert(tb);
  tb->typeManager=tm;
}



void Typemaker2_Builder_AddPublicDeclaration(TYPEMAKER2_BUILDER *tb, const char *s) {
  assert(tb);
  GWEN_StringList_AppendString(tb->declarationsPublic, s, 0, 0);
}



void Typemaker2_Builder_AddLibraryDeclaration(TYPEMAKER2_BUILDER *tb, const char *s) {
  assert(tb);
  GWEN_StringList_AppendString(tb->declarationsLibrary, s, 0, 0);
}



void Typemaker2_Builder_AddProtectedDeclaration(TYPEMAKER2_BUILDER *tb, const char *s) {
  assert(tb);
  GWEN_StringList_AppendString(tb->declarationsProtected, s, 0, 0);
}



void Typemaker2_Builder_AddPrivateDeclaration(TYPEMAKER2_BUILDER *tb, const char *s) {
  assert(tb);
  GWEN_StringList_AppendString(tb->declarationsPrivate, s, 0, 0);
}



void Typemaker2_Builder_AddCode(TYPEMAKER2_BUILDER *tb, const char *s) {
  assert(tb);
  GWEN_StringList_AppendString(tb->code, s, 0, 0);
}



const char *Typemaker2_Builder_GetFileNamePublic(const TYPEMAKER2_BUILDER *tb) {
  assert(tb);
  return tb->fileNamePublic;
}



void Typemaker2_Builder_SetFileNamePublic(TYPEMAKER2_BUILDER *tb, const char *s) {
  assert(tb);
  free(tb->fileNamePublic);
  if (s) tb->fileNamePublic=strdup(s);
  else tb->fileNamePublic=NULL;
}



const char *Typemaker2_Builder_GetFileNameLibrary(const TYPEMAKER2_BUILDER *tb) {
  assert(tb);
  return tb->fileNameLibrary;
}



void Typemaker2_Builder_SetFileNameLibrary(TYPEMAKER2_BUILDER *tb, const char *s) {
  assert(tb);
  free(tb->fileNameLibrary);
  if (s) tb->fileNameLibrary=strdup(s);
  else tb->fileNameLibrary=NULL;
}



const char *Typemaker2_Builder_GetFileNameProtected(const TYPEMAKER2_BUILDER *tb) {
  assert(tb);
  return tb->fileNameProtected;
}



void Typemaker2_Builder_SetFileNameProtected(TYPEMAKER2_BUILDER *tb, const char *s) {
  assert(tb);
  free(tb->fileNameProtected);
  if (s) tb->fileNameProtected=strdup(s);
  else tb->fileNameProtected=NULL;
}



const char *Typemaker2_Builder_GetFileNamePrivate(const TYPEMAKER2_BUILDER *tb) {
  assert(tb);
  return tb->fileNamePrivate;
}



void Typemaker2_Builder_SetFileNamePrivate(TYPEMAKER2_BUILDER *tb, const char *s) {
  assert(tb);
  free(tb->fileNamePrivate);
  if (s) tb->fileNamePrivate=strdup(s);
  else tb->fileNamePrivate=NULL;
}



const char *Typemaker2_Builder_GetFileNameCode(const TYPEMAKER2_BUILDER *tb) {
  assert(tb);
  return tb->fileNameCode;
}



void Typemaker2_Builder_SetFileNameCode(TYPEMAKER2_BUILDER *tb, const char *s) {
  assert(tb);
  free(tb->fileNameCode);
  if (s) tb->fileNameCode=strdup(s);
  else tb->fileNameCode=NULL;
}



void Typemaker2_Builder_SetBuildFn(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_BUILDER_BUILD_FN fn) {
  assert(tb);
  tb->buildFn=fn;
}



int Typemaker2_Builder_Build(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty) {
  assert(tb);
  if (tb->buildFn)
    return tb->buildFn(tb, ty);
  else
    return GWEN_ERROR_NOT_SUPPORTED;
}



GWEN_DB_NODE *Typemaker2_Builder_CreateDbForCall(TYPEMAKER2_BUILDER *tb,
						 TYPEMAKER2_TYPE *ty,
						 TYPEMAKER2_MEMBER *tm,
						 const char *src,
						 const char *dst) {
  GWEN_DB_NODE *db;
  const char *s;

  db=GWEN_DB_Group_new("vars");

  /* set some type vars */
  if (ty) {
    s=Typemaker2_Type_GetIdentifier(ty);
    if (s && *s)
      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "type", s);

    s=Typemaker2_Type_GetPrefix(ty);
    if (s && *s)
      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "prefix", s);
  }

  /* set some member vars */
  if (tm) {
    s=Typemaker2_Member_GetName(tm);
    if (s && *s)
      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "name", s);

    s=Typemaker2_Member_GetDefaultValue(tm);
    if (s && *s)
      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "default", s);

    s=Typemaker2_Member_GetPresetValue(tm);
    if (s && *s)
      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "preset", s);

    if (!(Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_VOLATILE)) {
      /* set field number for toObject/fromObject */
      s=Typemaker2_Member_GetFieldId(tm);
      if (s && *s)
	GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "fieldid", s);
    }
  }

  /* set src and dst */
  if (src && *src)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "src", src);
  if (dst && *dst)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "dst", dst);

  /* set some fixed vars */
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "retval", "rv");
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "db", "db");

  return db;
}



int Typemaker2_Builder_ReplaceVars(const char *s,
				   GWEN_DB_NODE *db,
				   GWEN_BUFFER *dbuf) {
  const char *p;

  p=s;
  while(*p) {
    if (*p=='$') {
      p++;
      if (*p=='$')
	GWEN_Buffer_AppendByte(dbuf, '$');
      else if (*p=='(') {
	const char *pStart;

	p++;
	pStart=p;
	while(*p && *p!=')')
	  p++;
	if (*p!=')') {
	  DBG_ERROR(GWEN_LOGDOMAIN, "Unterminated variable name in code");
	  return GWEN_ERROR_BAD_DATA;
	}
	else {
          int len;
	  char *name;
          const char *v;

	  len=p-pStart;
	  if (len<1) {
	    DBG_ERROR(GWEN_LOGDOMAIN, "Empty variable name in code");
	    return GWEN_ERROR_BAD_DATA;
	  }
	  name=(char*) malloc(len+1);
	  assert(name);
	  memmove(name, p, len);
	  name[len]=0;
	  v=GWEN_DB_GetCharValue(db, name, 0, NULL);
	  if (v==NULL) {
            DBG_ERROR(GWEN_LOGDOMAIN, "No value for variable [%s]", name);
	    free(name);
	    return GWEN_ERROR_NO_DATA;
	  }
	  free(name);
	  GWEN_Buffer_AppendString(dbuf, v);
	}
      }
      else {
	DBG_ERROR(GWEN_LOGDOMAIN, "Bad variable string in code");
        return GWEN_ERROR_BAD_DATA;
      }
    }
    else
      GWEN_Buffer_AppendByte(dbuf, *p);
    p++;
  }

  return 0;
}



#define INVOKE_FN(macro_var1) \
  int Typemaker2_Builder_Invoke_##macro_var1##Fn(TYPEMAKER2_BUILDER *tb, \
                                                 TYPEMAKER2_TYPE *ty,    \
					         TYPEMAKER2_MEMBER *tm,  \
                                                 const char *src,        \
					         const char *dst,        \
					         GWEN_BUFFER *dbuf) {    \
    const char *s;                                                       \
                                                                         \
    s=Typemaker2_Type_GetCode##macro_var1(ty);                           \
    if (s && *s) {                                                       \
      GWEN_DB_NODE *db;                                                  \
      int rv;                                                            \
                                                                         \
      db=Typemaker2_Builder_CreateDbForCall(tb, ty, tm, src, dst);       \
      if (db==NULL) {                                                    \
	DBG_INFO(GWEN_LOGDOMAIN, "here");                                \
	return GWEN_ERROR_BAD_DATA;                                      \
      }                                                                  \
      rv=Typemaker2_Builder_ReplaceVars(s, db, dbuf);                    \
      GWEN_DB_Group_free(db);                                            \
      if (rv<0) {                                                        \
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);                       \
	return rv;                                                       \
      }                                                                  \
      return 0;                                                          \
    }                                                                    \
    else {                                                               \
      DBG_ERROR(GWEN_LOGDOMAIN, "No code");                              \
      return GWEN_ERROR_NO_DATA;                                         \
    }                                                                    \
  }


INVOKE_FN(Construct);
INVOKE_FN(Destruct);
INVOKE_FN(Assign);
INVOKE_FN(Dup);
INVOKE_FN(ToDb);
INVOKE_FN(FromDb);
INVOKE_FN(ToXml);
INVOKE_FN(FromXml);
INVOKE_FN(ToObject);
INVOKE_FN(FromObject);















