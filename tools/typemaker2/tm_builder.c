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
#include <errno.h>
#include <string.h>
#include <ctype.h>


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
    free(tb->destFolder);
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



const char *Typemaker2_Builder_GetSourceFileName(const TYPEMAKER2_BUILDER *tb) {
  assert(tb);
  return tb->sourceFileName;
}



void Typemaker2_Builder_SetSourceFileName(TYPEMAKER2_BUILDER *tb, const char *s) {
  assert(tb);
  free(tb->sourceFileName);
  if (s) tb->sourceFileName=strdup(s);
  else tb->sourceFileName=NULL;
}



const char *Typemaker2_Builder_GetDestFolderName(const TYPEMAKER2_BUILDER *tb) {
  assert(tb);
  return tb->destFolder;
}



void Typemaker2_Builder_SetDestFolderName(TYPEMAKER2_BUILDER *tb, const char *s) {
  assert(tb);
  free(tb->destFolder);
  if (s) tb->destFolder=strdup(s);
  else tb->destFolder=NULL;
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

  if (tb->typeManager) {
    s=Typemaker2_TypeManager_GetApiDeclaration(tb->typeManager);
    if (s && *s)
      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "api", s);
    else
      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "", s);
  }

  /* set some type vars */
  if (ty) {
    TYPEMAKER2_TYPE *bty;

    s=Typemaker2_Type_GetIdentifier(ty);
    if (s && *s)
      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "struct_type", s);

    s=Typemaker2_Type_GetPrefix(ty);
    if (s && *s)
      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "struct_prefix", s);

    bty=Typemaker2_Type_GetBaseTypePtr(ty);
    if (bty) {
      s=Typemaker2_Type_GetIdentifier(bty);
      if (s && *s)
	GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "basetype_type", s);

      s=Typemaker2_Type_GetPrefix(bty);
      if (s && *s)
	GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "basetype_prefix", s);
    }
  }

  /* set some member vars */
  if (tm) {
    char numbuf[32];
    TYPEMAKER2_TYPE *mty;

    s=Typemaker2_Member_GetName(tm);
    if (s && *s)
      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "name", s);

      tbuf=GWEN_Buffer_new(0, 256, 0, 1);
      GWEN_Buffer_AppendByte(tbuf, toupper(*s));
      GWEN_Buffer_AppendString(tbuf, s+1);
      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "nameWithCapital", GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_free(tbuf);
    }

    s=Typemaker2_Member_GetElementName(tm);
    if (s && *s)
      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "element_name", s);
    else
      /* default behaviour is to use the name "element" for list members in GWEN_DBs */
      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "element_name", "element");

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

    /* maxlen */
    snprintf(numbuf, sizeof(numbuf)-1, "%d", Typemaker2_Member_GetMaxLen(tm));
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "maxlen", numbuf);

    /* set basetype stuff */
    mty=Typemaker2_Member_GetTypePtr(tm);
    assert(mty);
    if (mty) {
      TYPEMAKER2_TYPE *bty;

      s=Typemaker2_Type_GetIdentifier(mty);
      if (s && *s)
	GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "member_type", s);

      s=Typemaker2_Type_GetPrefix(mty);
      if (s && *s)
	GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "member_prefix", s);

      bty=Typemaker2_Type_GetBaseTypePtr(mty);
      if (bty) {
	s=Typemaker2_Type_GetIdentifier(bty);
	if (s && *s)
	  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "member_basetype_type", s);

	s=Typemaker2_Type_GetPrefix(bty);
	if (s && *s)
	  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "member_basetype_prefix", s);
      }
    }

    /* set enum-specific stuff */
    if (Typemaker2_Member_GetFlags(tm) & TYPEMAKER2_FLAGS_ENUM) {
      TYPEMAKER2_ENUM *te;

      te=Typemaker2_Member_GetEnumPtr(tm);
      if (te) {
	s=Typemaker2_Enum_GetFromStringFn(te);
	if (s && *s)
	  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "enum_fromstring_fn", s);
	s=Typemaker2_Enum_GetToStringFn(te);
	if (s && *s)
	  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "enum_tostring_fn", s);
      }
    }
  }

  /* set src and dst */
  if (src && *src)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "src", src);
  if (dst && *dst)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "dst", dst);

  /* set some fixed vars */
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "retval", "p_rv");
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "db", "p_db");
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "buffer", "p_buffer");

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
	  memmove(name, pStart, len);
	  name[len]=0;
	  v=GWEN_DB_GetCharValue(db, name, 0, NULL);
	  if (v) {
	    free(name);
	    GWEN_Buffer_AppendString(dbuf, v);
          }
#if 0 /* just replace with empty value */
	  else {
	    GWEN_Buffer_AppendString(dbuf, " [__VALUE OF ");
            GWEN_Buffer_AppendString(dbuf, name);
	    GWEN_Buffer_AppendString(dbuf, " WAS NOT SET__] ");
	    free(name);
          }
#endif
	}
      }
      else {
	DBG_ERROR(GWEN_LOGDOMAIN, "Bad variable string in code");
        return GWEN_ERROR_BAD_DATA;
      }
      p++;
    }
    else {
      if (*p=='#') {
	/* let # lines begin on a new line */
	GWEN_Buffer_AppendByte(dbuf, '\n');
	GWEN_Buffer_AppendByte(dbuf, *p);

	/* skip introducing cross and copy all stuff until the next cross
	 * upon which wa inject a newline (to make the preprocessor happy)
	 */
	p++;
	while(*p && *p!='#') {
	  GWEN_Buffer_AppendByte(dbuf, *p);
	  p++;
	}
	if (*p=='#') {
	  GWEN_Buffer_AppendByte(dbuf, '\n');
	  p++;
	}
      }
      else {
	GWEN_Buffer_AppendByte(dbuf, *p);
	p++;
      }
    }
  }

  return 0;
}



#define INVOKE_FN(macro_var1) \
  int Typemaker2_Builder_Invoke_##macro_var1##Fn(TYPEMAKER2_BUILDER *tb,  \
                                                 TYPEMAKER2_TYPE *ty,     \
					         TYPEMAKER2_MEMBER *tm,   \
                                                 const char *src,         \
					         const char *dst,         \
					         GWEN_BUFFER *dbuf) {     \
    if (tm) {                                                             \
      TYPEMAKER2_TYPE *mty;                                               \
      TYPEMAKER2_CODE *tc;                                                \
      const char *s=NULL;                                                 \
									  \
      mty=Typemaker2_Member_GetTypePtr(tm);                               \
      if (mty==NULL) {                                                    \
        DBG_ERROR(GWEN_LOGDOMAIN, "No type pointer for member");          \
	return GWEN_ERROR_NO_DATA;                                        \
      }                                                                   \
      tc=Typemaker2_Type_FindCodeForMember(mty, tm, __STRING(macro_var1));\
      if (tc)                                                             \
        s=Typemaker2_Code_GetCode(tc);                                    \
      if (s && *s) {                                                      \
	GWEN_DB_NODE *db;                                                 \
	int rv;                                                           \
									  \
	db=Typemaker2_Builder_CreateDbForCall(tb, ty, tm, src, dst);      \
	if (db==NULL) {                                                   \
	  DBG_INFO(GWEN_LOGDOMAIN, "here");                               \
	  return GWEN_ERROR_BAD_DATA;                                     \
	}                                                                 \
                                                                          \
        if (0) {                                                          \
	  GWEN_Buffer_AppendString(dbuf, "/* function \"");               \
	  GWEN_Buffer_AppendString(dbuf, __STRING(macro_var1));           \
	  GWEN_Buffer_AppendString(dbuf, "\" of type \"");                \
	  GWEN_Buffer_AppendString(dbuf, Typemaker2_Type_GetName(mty));   \
	  GWEN_Buffer_AppendString(dbuf, "\" */\n");                      \
        }                                                                 \
	rv=Typemaker2_Builder_ReplaceVars(s, db, dbuf);                   \
	GWEN_DB_Group_free(db);                                           \
	if (rv<0) {                                                       \
	  DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);                      \
	  return rv;                                                      \
	}                                                                 \
	return 0;                                                         \
      }                                                                   \
      else {                                                              \
	DBG_INFO(GWEN_LOGDOMAIN, "No code for type [%s]",                 \
		 Typemaker2_Type_GetName(mty));                           \
	return 0;                                                         \
      }                                                                   \
    }                                                                     \
    else {                                                                \
      DBG_ERROR(GWEN_LOGDOMAIN, "No member information");                 \
      return GWEN_ERROR_NO_DATA;                                          \
   }                                                                      \
  }


INVOKE_FN(Construct);
INVOKE_FN(Destruct);
INVOKE_FN(Assign);
INVOKE_FN(Dup);
INVOKE_FN(Compare);
INVOKE_FN(ToDb);
INVOKE_FN(FromDb);
INVOKE_FN(ToXml);
INVOKE_FN(FromXml);
INVOKE_FN(ToObject);
INVOKE_FN(FromObject);
INVOKE_FN(ToHashString);




int Typemaker2_Builder_WriteFile(TYPEMAKER2_BUILDER *tb,
                                 TYPEMAKER2_TYPE *ty,
				 const char *fileName,
				 GWEN_STRINGLIST *sl,
				 int acc) {
  GWEN_STRINGLISTENTRY *se;
  FILE *f;

  f=fopen(fileName, "w");
  if (f==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "fopen(%s): %s (%d)",
	      fileName,
	      strerror(errno),
	      errno);
    return GWEN_ERROR_IO;
  }

  fprintf(f,
	  "/**********************************************************\n"
	  " * This file has been automatically created by \"typemaker2\"\n"
	  " * from the file \"%s\".\n"
	  " * Please do not edit this file, all changes will be lost.\n"
	  " * Better edit the mentioned source file instead.\n"
	  " **********************************************************/\n"
	  "\n",
	  Typemaker2_Builder_GetSourceFileName(tb));

  if (acc==TypeMaker2_Access_Unknown) {
    const char *s;
    TYPEMAKER2_HEADER_LIST *hl;

    fprintf(f, "#ifdef HAVE_CONFIG_H\n# include <config.h>\n#endif\n\n");

    s=tb->fileNamePrivate;
    if (s==NULL)
      s=tb->fileNameLibrary;
    if (s==NULL)
      s=tb->fileNameProtected;
    if (s==NULL)
      s=tb->fileNamePublic;
    if (s) {
      fprintf(f, "#include \"%s\"\n\n", s);
    }

    /* add some needed headers */
    fprintf(f, "#include <gwenhywfar/misc.h>\n");
    fprintf(f, "#include <gwenhywfar/debug.h>\n");

    /* write code headers */
    fprintf(f, "\n");
    fprintf(f, "/* code headers */\n");
    hl=Typemaker2_Type_GetHeaders(ty);
    if (hl) {
      TYPEMAKER2_HEADER *h;

      h=Typemaker2_Header_List_First(hl);
      while(h) {
	if (Typemaker2_Header_GetLocation(h)==Typemaker2_HeaderLocation_Code) {
          if (Typemaker2_Header_GetType(h)==Typemaker2_HeaderType_System)
	    fprintf(f, "#include <%s>\n", Typemaker2_Header_GetFileName(h));
          else
	    fprintf(f, "#include \"%s\"\n", Typemaker2_Header_GetFileName(h));
	}
	h=Typemaker2_Header_List_Next(h);
      }
    }
    fprintf(f, "\n");
  }
  else {
    GWEN_BUFFER *xbuf;
    char *p;
    const char *s;

    xbuf=GWEN_Buffer_new(0, 256, 0, 1);
    s=Typemaker2_Type_GetName(ty);
    GWEN_Buffer_AppendString(xbuf, s);
    GWEN_Buffer_AppendString(xbuf, "_");
    s=strrchr(fileName, GWEN_DIR_SEPARATOR);
    if (s && *s)
        GWEN_Buffer_AppendString(xbuf, s+1);
    else
        GWEN_Buffer_AppendString(xbuf, fileName);
    p=GWEN_Buffer_GetStart(xbuf);
    while (*p) {
      if (!isalnum(*p))
	*p='_';
      else
	*p=toupper(*p);
      p++;
    }
    fprintf(f, "#ifndef %s\n", GWEN_Buffer_GetStart(xbuf));
    fprintf(f, "#define %s\n\n", GWEN_Buffer_GetStart(xbuf));

    s=NULL;
    switch(acc) {
    case TypeMaker2_Access_Unknown:
      s=tb->fileNamePrivate;
      if (s)
	break;
    case TypeMaker2_Access_Private:
      s=tb->fileNameLibrary;
      if (s)
	break;
    case TypeMaker2_Access_Library:
      s=tb->fileNameProtected;
      if (s)
	break;
    case TypeMaker2_Access_Protected:
      s=tb->fileNamePublic;
      if (s)
	break;
    default:
      break;
    }

    if (s) {
      fprintf(f, "#include \"%s\"\n\n", s);
    }

    fprintf(f, "\n");
    fprintf(f, "#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n");
    GWEN_Buffer_free(xbuf);
  }

  se=GWEN_StringList_FirstEntry(sl);
  while(se) {
    const char *s;

    s=GWEN_StringListEntry_Data(se);
    if (s)
      fprintf(f, "%s\n", s);

    se=GWEN_StringListEntry_Next(se);
  }

  if (acc==TypeMaker2_Access_Unknown) {
    TYPEMAKER2_HEADER_LIST *hl;

    /* write codeEnd headers */
    fprintf(f, "\n");
    fprintf(f, "/* code headers */\n");
    hl=Typemaker2_Type_GetHeaders(ty);
    if (hl) {
      TYPEMAKER2_HEADER *h;

      h=Typemaker2_Header_List_First(hl);
      while(h) {
	if (Typemaker2_Header_GetLocation(h)==Typemaker2_HeaderLocation_CodeEnd) {
          if (Typemaker2_Header_GetType(h)==Typemaker2_HeaderType_System)
	    fprintf(f, "#include <%s>\n", Typemaker2_Header_GetFileName(h));
          else
	    fprintf(f, "#include \"%s\"\n", Typemaker2_Header_GetFileName(h));
	}
	h=Typemaker2_Header_List_Next(h);
      }
    }
    fprintf(f, "\n");
  }

  if (acc!=TypeMaker2_Access_Unknown) {
    fprintf(f, "#ifdef __cplusplus\n}\n#endif\n\n");
    fprintf(f, "#endif\n\n");
  }

  if (fclose(f)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "fclose(%s): %s (%d)",
	      fileName,
	      strerror(errno),
	      errno);
    return GWEN_ERROR_IO;
  }

  return 0;
}



int Typemaker2_Builder_WriteTypedefFile(TYPEMAKER2_BUILDER *tb,
					TYPEMAKER2_TYPE *ty,
					const char *fileName) {
  FILE *f;

  f=fopen(fileName, "w");
  if (f==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "fopen(%s): %s (%d)",
	      fileName,
	      strerror(errno),
	      errno);
    return GWEN_ERROR_IO;
  }

  fprintf(f, "<?xml?>\n");
  fprintf(f, "\n");
  fprintf(f, "<tm2>\n");

  fprintf(f, "  <typedef id=\"%s\" type=\"pointer\" lang=\"c\" extends=\"struct_base\">\n",
	  Typemaker2_Type_GetName(ty));

  fprintf(f, "    <identifier>%s</identifier>\n", Typemaker2_Type_GetName(ty));
  fprintf(f, "    <prefix>%s</prefix>\n", Typemaker2_Type_GetPrefix(ty));

  fprintf(f, "  </typedef>\n");
  fprintf(f, "</tm2>\n");

  if (fclose(f)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "fclose(%s): %s (%d)",
	      fileName,
	      strerror(errno),
	      errno);
    return GWEN_ERROR_IO;
  }

  return 0;
}



int Typemaker2_Builder_WriteTypedefFile_List1(TYPEMAKER2_BUILDER *tb,
					      TYPEMAKER2_TYPE *ty,
					      const char *fileName) {
  FILE *f;

  f=fopen(fileName, "w");
  if (f==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "fopen(%s): %s (%d)",
	      fileName,
	      strerror(errno),
	      errno);
    return GWEN_ERROR_IO;
  }

  fprintf(f, "<?xml?>\n");
  fprintf(f, "\n");
  fprintf(f, "<tm2>\n");

  fprintf(f, "  <typedef id=\"%s_LIST\" type=\"pointer\" lang=\"c\" extends=\"list1_base\" "
	  "basetype=\"%s\">\n",
	  Typemaker2_Type_GetName(ty),
	  Typemaker2_Type_GetName(ty));

  fprintf(f, "    <identifier>%s_LIST</identifier>\n", Typemaker2_Type_GetName(ty));
  fprintf(f, "    <prefix>%s_List</prefix>\n", Typemaker2_Type_GetPrefix(ty));

  fprintf(f, "  </typedef>\n");
  fprintf(f, "</tm2>\n");

  if (fclose(f)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "fclose(%s): %s (%d)",
	      fileName,
	      strerror(errno),
	      errno);
    return GWEN_ERROR_IO;
  }

  return 0;
}



int Typemaker2_Builder_WriteTypedefFile_List2(TYPEMAKER2_BUILDER *tb,
					      TYPEMAKER2_TYPE *ty,
					      const char *fileName) {
  FILE *f;

  f=fopen(fileName, "w");
  if (f==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "fopen(%s): %s (%d)",
	      fileName,
	      strerror(errno),
	      errno);
    return GWEN_ERROR_IO;
  }

  fprintf(f, "<?xml?>\n");
  fprintf(f, "\n");
  fprintf(f, "<tm2>\n");

  fprintf(f, "  <typedef id=\"%s_LIST2\" type=\"pointer\" lang=\"c\" extends=\"list2_base\" "
	  "basetype=\"%s\">\n",
	  Typemaker2_Type_GetName(ty),
	  Typemaker2_Type_GetName(ty));

  fprintf(f, "    <identifier>%s_LIST2</identifier>\n", Typemaker2_Type_GetName(ty));
  fprintf(f, "    <prefix>%s_List2</prefix>\n", Typemaker2_Type_GetPrefix(ty));

  fprintf(f, "  </typedef>\n");
  fprintf(f, "</tm2>\n");

  if (fclose(f)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "fclose(%s): %s (%d)",
	      fileName,
	      strerror(errno),
	      errno);
    return GWEN_ERROR_IO;
  }

  return 0;
}



int Typemaker2_Builder_WriteTypedefFile_Tree(TYPEMAKER2_BUILDER *tb,
					     TYPEMAKER2_TYPE *ty,
					     const char *fileName) {
  FILE *f;

  f=fopen(fileName, "w");
  if (f==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "fopen(%s): %s (%d)",
	      fileName,
	      strerror(errno),
	      errno);
    return GWEN_ERROR_IO;
  }

  fprintf(f, "<?xml?>\n");
  fprintf(f, "\n");
  fprintf(f, "<tm2>\n");

  fprintf(f, "  <typedef id=\"%s_TREE\" type=\"pointer\" lang=\"c\" extends=\"tree_base\" "
	  "basetype=\"%s\">\n",
	  Typemaker2_Type_GetName(ty),
	  Typemaker2_Type_GetName(ty));

  fprintf(f, "    <identifier>%s_TREE</identifier>\n", Typemaker2_Type_GetName(ty));
  fprintf(f, "    <prefix>%s_Tree</prefix>\n", Typemaker2_Type_GetPrefix(ty));

  fprintf(f, "  </typedef>\n");
  fprintf(f, "</tm2>\n");

  if (fclose(f)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "fclose(%s): %s (%d)",
	      fileName,
	      strerror(errno),
	      errno);
    return GWEN_ERROR_IO;
  }

  return 0;
}



int Typemaker2_Builder_WriteTypedefFile_Tree2(TYPEMAKER2_BUILDER *tb,
                                              TYPEMAKER2_TYPE *ty,
                                              const char *fileName)
{
  FILE *f;

  f=fopen(fileName, "w");
  if (f==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "fopen(%s): %s (%d)",
              fileName,
              strerror(errno),
              errno);
    return GWEN_ERROR_IO;
  }

  fprintf(f, "<?xml?>\n");
  fprintf(f, "\n");
  fprintf(f, "<tm2>\n");

  fprintf(f, "  <typedef id=\"%s_TREE2\" type=\"pointer\" lang=\"c\" extends=\"tree_base\" "
          "basetype=\"%s\">\n",
          Typemaker2_Type_GetName(ty),
          Typemaker2_Type_GetName(ty));

  fprintf(f, "    <identifier>%s_TREE2</identifier>\n", Typemaker2_Type_GetName(ty));
  fprintf(f, "    <prefix>%s_Tree2</prefix>\n", Typemaker2_Type_GetPrefix(ty));

  fprintf(f, "  </typedef>\n");
  fprintf(f, "</tm2>\n");

  if (fclose(f)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "fclose(%s): %s (%d)",
              fileName,
              strerror(errno),
              errno);
    return GWEN_ERROR_IO;
  }

  return 0;
}



int Typemaker2_Builder_WriteTypedefFile_IdMap(TYPEMAKER2_BUILDER *tb,
					      TYPEMAKER2_TYPE *ty,
					      const char *fileName) {
  FILE *f;

  f=fopen(fileName, "w");
  if (f==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "fopen(%s): %s (%d)",
	      fileName,
	      strerror(errno),
	      errno);
    return GWEN_ERROR_IO;
  }

  fprintf(f, "<?xml?>\n");
  fprintf(f, "\n");
  fprintf(f, "<tm2>\n");

  fprintf(f, "  <typedef id=\"%s_IDMAP\" type=\"pointer\" lang=\"c\" extends=\"idmap_base\" "
	  "basetype=\"%s\">\n",
	  Typemaker2_Type_GetName(ty),
	  Typemaker2_Type_GetName(ty));

  fprintf(f, "    <identifier>%s_IDMAP</identifier>\n", Typemaker2_Type_GetName(ty));
  fprintf(f, "    <prefix>%s_IdMap</prefix>\n", Typemaker2_Type_GetPrefix(ty));

  fprintf(f, "  </typedef>\n");
  fprintf(f, "</tm2>\n");

  if (fclose(f)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "fclose(%s): %s (%d)",
	      fileName,
	      strerror(errno),
	      errno);
    return GWEN_ERROR_IO;
  }

  return 0;
}





int Typemaker2_Builder_DetermineOutFileNames(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty) {
  const char *fname;

  if (GWEN_StringList_Count(tb->declarationsPublic)) {
    fname=tb->fileNamePublic;
    if (fname==NULL || *fname==0) {
      const char *s;
      char *t;
      GWEN_BUFFER *tbuf;

      s=Typemaker2_Type_GetBaseFileName(ty);
      if (s==NULL || *s==0) {
	s=Typemaker2_Type_GetName(ty);
	if (s==NULL || *s==0) {
	  DBG_ERROR(GWEN_LOGDOMAIN, "Type has no name");
	      return GWEN_ERROR_BAD_DATA;
	}
      }
      tbuf=GWEN_Buffer_new(0, 256, 0, 1);
      if (tb->destFolder) {
	GWEN_Buffer_AppendString(tbuf, tb->destFolder);
        GWEN_Buffer_AppendString(tbuf, GWEN_DIR_SEPARATOR_S);
      }
      GWEN_Buffer_AppendString(tbuf, s);
      t=GWEN_Buffer_GetStart(tbuf);
      while(*t) {
	*t=tolower(*t);
        t++;
      }
      GWEN_Buffer_AppendString(tbuf, ".h");
      fname=GWEN_Buffer_GetStart(tbuf);
      Typemaker2_Builder_SetFileNamePublic(tb, fname);
      GWEN_Buffer_free(tbuf);
    }
  }

  if (GWEN_StringList_Count(tb->declarationsLibrary)) {
    fname=tb->fileNameLibrary;
    if (fname==NULL || *fname==0) {
      const char *s;
      char *t;
      GWEN_BUFFER *tbuf;

      s=Typemaker2_Type_GetBaseFileName(ty);
      if (s==NULL || *s==0) {
	s=Typemaker2_Type_GetName(ty);
	if (s==NULL || *s==0) {
	  DBG_ERROR(GWEN_LOGDOMAIN, "Type has no name");
	      return GWEN_ERROR_BAD_DATA;
	}
      }
      tbuf=GWEN_Buffer_new(0, 256, 0, 1);
      if (tb->destFolder) {
	GWEN_Buffer_AppendString(tbuf, tb->destFolder);
        GWEN_Buffer_AppendString(tbuf, GWEN_DIR_SEPARATOR_S);
      }
      GWEN_Buffer_AppendString(tbuf, s);
      t=GWEN_Buffer_GetStart(tbuf);
      while(*t) {
	*t=tolower(*t);
        t++;
      }
      GWEN_Buffer_AppendString(tbuf, "_l.h");
      fname=GWEN_Buffer_GetStart(tbuf);
      Typemaker2_Builder_SetFileNameLibrary(tb, fname);
      GWEN_Buffer_free(tbuf);
    }
  }

  if (GWEN_StringList_Count(tb->declarationsProtected)) {
    fname=tb->fileNameProtected;
    if (fname==NULL || *fname==0) {
      const char *s;
      char *t;
      GWEN_BUFFER *tbuf;

      s=Typemaker2_Type_GetBaseFileName(ty);
      if (s==NULL || *s==0) {
	s=Typemaker2_Type_GetName(ty);
	if (s==NULL || *s==0) {
	  DBG_ERROR(GWEN_LOGDOMAIN, "Type has no name");
	      return GWEN_ERROR_BAD_DATA;
	}
      }
      tbuf=GWEN_Buffer_new(0, 256, 0, 1);
      if (tb->destFolder) {
	GWEN_Buffer_AppendString(tbuf, tb->destFolder);
        GWEN_Buffer_AppendString(tbuf, GWEN_DIR_SEPARATOR_S);
      }
      GWEN_Buffer_AppendString(tbuf, s);
      t=GWEN_Buffer_GetStart(tbuf);
      while(*t) {
	*t=tolower(*t);
        t++;
      }
      GWEN_Buffer_AppendString(tbuf, "_be.h");
      fname=GWEN_Buffer_GetStart(tbuf);
      Typemaker2_Builder_SetFileNameProtected(tb, fname);
      GWEN_Buffer_free(tbuf);
    }
  }

  if (GWEN_StringList_Count(tb->declarationsPrivate)) {
    fname=tb->fileNamePrivate;
    if (fname==NULL || *fname==0) {
      const char *s;
      char *t;
      GWEN_BUFFER *tbuf;

      s=Typemaker2_Type_GetBaseFileName(ty);
      if (s==NULL || *s==0) {
	s=Typemaker2_Type_GetName(ty);
	if (s==NULL || *s==0) {
	  DBG_ERROR(GWEN_LOGDOMAIN, "Type has no name");
	      return GWEN_ERROR_BAD_DATA;
	}
      }
      tbuf=GWEN_Buffer_new(0, 256, 0, 1);
      if (tb->destFolder) {
	GWEN_Buffer_AppendString(tbuf, tb->destFolder);
        GWEN_Buffer_AppendString(tbuf, GWEN_DIR_SEPARATOR_S);
      }
      GWEN_Buffer_AppendString(tbuf, s);
      t=GWEN_Buffer_GetStart(tbuf);
      while(*t) {
	*t=tolower(*t);
        t++;
      }
      GWEN_Buffer_AppendString(tbuf, "_p.h");
      fname=GWEN_Buffer_GetStart(tbuf);
      Typemaker2_Builder_SetFileNamePrivate(tb, fname);
      GWEN_Buffer_free(tbuf);
    }
  }

  if (GWEN_StringList_Count(tb->code)) {
    fname=tb->fileNameCode;
    if (fname==NULL || *fname==0) {
      const char *s;
      char *t;
      GWEN_BUFFER *tbuf;

      s=Typemaker2_Type_GetBaseFileName(ty);
      if (s==NULL || *s==0) {
	s=Typemaker2_Type_GetName(ty);
	if (s==NULL || *s==0) {
	  DBG_ERROR(GWEN_LOGDOMAIN, "Type has no name");
	      return GWEN_ERROR_BAD_DATA;
	}
      }
      tbuf=GWEN_Buffer_new(0, 256, 0, 1);
      if (tb->destFolder) {
	GWEN_Buffer_AppendString(tbuf, tb->destFolder);
        GWEN_Buffer_AppendString(tbuf, GWEN_DIR_SEPARATOR_S);
      }
      GWEN_Buffer_AppendString(tbuf, s);
      t=GWEN_Buffer_GetStart(tbuf);
      while(*t) {
	*t=tolower(*t);
        t++;
      }
      GWEN_Buffer_AppendString(tbuf, ".c");
      fname=GWEN_Buffer_GetStart(tbuf);
      Typemaker2_Builder_SetFileNameCode(tb, fname);
      GWEN_Buffer_free(tbuf);
    }
  }

  return 0;
}



int Typemaker2_Builder_WriteFiles(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, uint32_t writeFlags)
{
  const char *fname;
  int rv;

  rv=Typemaker2_Builder_DetermineOutFileNames(tb, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  if (writeFlags & TYPEMAKER2_BUILDER_WRITEFILE_FLAGS_WRITE_TYPE) {
    if (GWEN_StringList_Count(tb->declarationsPublic)) {
      fname=tb->fileNamePublic;
      assert(fname);
  
      rv=Typemaker2_Builder_WriteFile(tb, ty, fname, tb->declarationsPublic,
                                      TypeMaker2_Access_Public);
      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
    }
  
    if (GWEN_StringList_Count(tb->declarationsLibrary)) {
      fname=tb->fileNameLibrary;
      assert(fname);
  
      rv=Typemaker2_Builder_WriteFile(tb, ty, fname, tb->declarationsLibrary,
                                      TypeMaker2_Access_Library);
      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
    }
  
    if (GWEN_StringList_Count(tb->declarationsProtected)) {
      fname=tb->fileNameProtected;
      assert(fname);
  
      rv=Typemaker2_Builder_WriteFile(tb, ty, fname, tb->declarationsProtected,
                                      TypeMaker2_Access_Protected);
      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
    }
  
    if (GWEN_StringList_Count(tb->declarationsPrivate)) {
      fname=tb->fileNamePrivate;
      assert(fname);
  
      rv=Typemaker2_Builder_WriteFile(tb, ty, fname, tb->declarationsPrivate,
                                      TypeMaker2_Access_Private);
      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
    }
  
    if (GWEN_StringList_Count(tb->code)) {
      fname=tb->fileNameCode;
      assert(fname);
  
      rv=Typemaker2_Builder_WriteFile(tb, ty, fname, tb->code,
                                      TypeMaker2_Access_Unknown);
      if (rv<0) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
    }
  }

  /* write typedef file */
  if (writeFlags & TYPEMAKER2_BUILDER_WRITEFILE_FLAGS_WRITE_DEFS) {
    if (1) {
      const char *s;
      char *t;
      GWEN_BUFFER *tbuf;

      s=Typemaker2_Type_GetName(ty);
      if (s==NULL || *s==0) {
	DBG_ERROR(GWEN_LOGDOMAIN, "Type has no name");
	return GWEN_ERROR_BAD_DATA;
      }
      tbuf=GWEN_Buffer_new(0, 256, 0, 1);
      if (tb->destFolder) {
	GWEN_Buffer_AppendString(tbuf, tb->destFolder);
	GWEN_Buffer_AppendString(tbuf, GWEN_DIR_SEPARATOR_S);
      }
      GWEN_Buffer_AppendString(tbuf, s);
      t=GWEN_Buffer_GetStart(tbuf);
      while (*t) {
	*t=tolower(*t);
	t++;
      }
      GWEN_Buffer_AppendString(tbuf, ".tm2");
      fname=GWEN_Buffer_GetStart(tbuf);
      rv=Typemaker2_Builder_WriteTypedefFile(tb, ty, fname);
      GWEN_Buffer_free(tbuf);
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }
    }

    /* write typedef file for list1 */
    if (Typemaker2_Type_GetFlags(ty) & TYPEMAKER2_TYPEFLAGS_WITH_LIST1) {
      const char *s;
      char *t;
      GWEN_BUFFER *tbuf;

      s=Typemaker2_Type_GetName(ty);
      if (s==NULL || *s==0) {
	DBG_ERROR(GWEN_LOGDOMAIN, "Type has no name");
	return GWEN_ERROR_BAD_DATA;
      }
      tbuf=GWEN_Buffer_new(0, 256, 0, 1);
      if (tb->destFolder) {
	GWEN_Buffer_AppendString(tbuf, tb->destFolder);
	GWEN_Buffer_AppendString(tbuf, GWEN_DIR_SEPARATOR_S);
      }
      GWEN_Buffer_AppendString(tbuf, s);
      t=GWEN_Buffer_GetStart(tbuf);
      while (*t) {
	*t=tolower(*t);
	t++;
      }
      GWEN_Buffer_AppendString(tbuf, "_list.tm2");
      fname=GWEN_Buffer_GetStart(tbuf);
      rv=Typemaker2_Builder_WriteTypedefFile_List1(tb, ty, fname);
      GWEN_Buffer_free(tbuf);
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }
    }

    /* write typedef file for list2 */
    if (Typemaker2_Type_GetFlags(ty) & TYPEMAKER2_TYPEFLAGS_WITH_LIST2) {
      const char *s;
      char *t;
      GWEN_BUFFER *tbuf;

      s=Typemaker2_Type_GetName(ty);
      if (s==NULL || *s==0) {
	DBG_ERROR(GWEN_LOGDOMAIN, "Type has no name");
	return GWEN_ERROR_BAD_DATA;
      }
      tbuf=GWEN_Buffer_new(0, 256, 0, 1);
      if (tb->destFolder) {
	GWEN_Buffer_AppendString(tbuf, tb->destFolder);
	GWEN_Buffer_AppendString(tbuf, GWEN_DIR_SEPARATOR_S);
      }
      GWEN_Buffer_AppendString(tbuf, s);
      t=GWEN_Buffer_GetStart(tbuf);
      while (*t) {
	*t=tolower(*t);
	t++;
      }
      GWEN_Buffer_AppendString(tbuf, "_list2.tm2");
      fname=GWEN_Buffer_GetStart(tbuf);
      rv=Typemaker2_Builder_WriteTypedefFile_List2(tb, ty, fname);
      GWEN_Buffer_free(tbuf);
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }
    }

    /* write typedef file for tree */
    if (Typemaker2_Type_GetFlags(ty) & TYPEMAKER2_TYPEFLAGS_WITH_TREE) {
      const char *s;
      char *t;
      GWEN_BUFFER *tbuf;

      s=Typemaker2_Type_GetName(ty);
      if (s==NULL || *s==0) {
	DBG_ERROR(GWEN_LOGDOMAIN, "Type has no name");
	return GWEN_ERROR_BAD_DATA;
      }
      tbuf=GWEN_Buffer_new(0, 256, 0, 1);
      if (tb->destFolder) {
	GWEN_Buffer_AppendString(tbuf, tb->destFolder);
	GWEN_Buffer_AppendString(tbuf, GWEN_DIR_SEPARATOR_S);
      }
      GWEN_Buffer_AppendString(tbuf, s);
      t=GWEN_Buffer_GetStart(tbuf);
      while (*t) {
	*t=tolower(*t);
	t++;
      }
      GWEN_Buffer_AppendString(tbuf, "_tree.tm2");
      fname=GWEN_Buffer_GetStart(tbuf);
      rv=Typemaker2_Builder_WriteTypedefFile_Tree(tb, ty, fname);
      GWEN_Buffer_free(tbuf);
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }
    }

    /* write typedef file for tree2 */
    if (Typemaker2_Type_GetFlags(ty) & TYPEMAKER2_TYPEFLAGS_WITH_TREE2) {
      const char *s;
      char *t;
      GWEN_BUFFER *tbuf;

      s=Typemaker2_Type_GetName(ty);
      if (s==NULL || *s==0) {
	DBG_ERROR(GWEN_LOGDOMAIN, "Type has no name");
	return GWEN_ERROR_BAD_DATA;
      }
      tbuf=GWEN_Buffer_new(0, 256, 0, 1);
      if (tb->destFolder) {
	GWEN_Buffer_AppendString(tbuf, tb->destFolder);
	GWEN_Buffer_AppendString(tbuf, GWEN_DIR_SEPARATOR_S);
      }
      GWEN_Buffer_AppendString(tbuf, s);
      t=GWEN_Buffer_GetStart(tbuf);
      while (*t) {
	*t=tolower(*t);
	t++;
      }
      GWEN_Buffer_AppendString(tbuf, "_tree2.tm2");
      fname=GWEN_Buffer_GetStart(tbuf);
      rv=Typemaker2_Builder_WriteTypedefFile_Tree2(tb, ty, fname);
      GWEN_Buffer_free(tbuf);
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }
    }

    /* write typedef file for idmap */
    if (Typemaker2_Type_GetFlags(ty) & TYPEMAKER2_TYPEFLAGS_WITH_IDMAP) {
      const char *s;
      char *t;
      GWEN_BUFFER *tbuf;

      s=Typemaker2_Type_GetName(ty);
      if (s==NULL || *s==0) {
	DBG_ERROR(GWEN_LOGDOMAIN, "Type has no name");
	return GWEN_ERROR_BAD_DATA;
      }
      tbuf=GWEN_Buffer_new(0, 256, 0, 1);
      if (tb->destFolder) {
	GWEN_Buffer_AppendString(tbuf, tb->destFolder);
	GWEN_Buffer_AppendString(tbuf, GWEN_DIR_SEPARATOR_S);
      }
      GWEN_Buffer_AppendString(tbuf, s);
      t=GWEN_Buffer_GetStart(tbuf);
      while (*t) {
	*t=tolower(*t);
	t++;
      }
      GWEN_Buffer_AppendString(tbuf, "_idmap.tm2");
      fname=GWEN_Buffer_GetStart(tbuf);
      rv=Typemaker2_Builder_WriteTypedefFile_IdMap(tb, ty, fname);
      GWEN_Buffer_free(tbuf);
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }
    }
  }

  return 0;
}





