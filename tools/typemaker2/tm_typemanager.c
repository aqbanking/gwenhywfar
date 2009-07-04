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


#include "tm_typemanager_p.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/directory.h>

#include <assert.h>
#include <ctype.h>





TYPEMAKER2_TYPEMANAGER *Typemaker2_TypeManager_new() {
  TYPEMAKER2_TYPEMANAGER *tym;

  GWEN_NEW_OBJECT(TYPEMAKER2_TYPEMANAGER, tym);

  tym->typeList=Typemaker2_Type_List_new();
  tym->folders=GWEN_StringList_new();
  tym->lang=strdup("c");


  return tym;
}



void Typemaker2_TypeManager_free(TYPEMAKER2_TYPEMANAGER *tym) {
  if (tym) {
    Typemaker2_Type_List_free(tym->typeList);
    GWEN_StringList_free(tym->folders);
    GWEN_FREE_OBJECT(tym);
  }
}



const char *Typemaker2_TypeManager_GetLanguage(const TYPEMAKER2_TYPEMANAGER *tym) {
  assert(tym);
  return tym->lang;
}



void Typemaker2_TypeManager_SetLanguage(TYPEMAKER2_TYPEMANAGER *tym, const char *s) {
  assert(tym);
  free(tym->lang);
  if (s) tym->lang=strdup(s);
  else tym->lang=NULL;
}



const char *Typemaker2_TypeManager_GetApiDeclaration(const TYPEMAKER2_TYPEMANAGER *tym) {
  assert(tym);
  return tym->apiDeclaration;
}



void Typemaker2_TypeManager_SetApiDeclaration(TYPEMAKER2_TYPEMANAGER *tym, const char *s) {
  assert(tym);
  free(tym->apiDeclaration);
  if (s) tym->apiDeclaration=strdup(s);
  else tym->apiDeclaration=NULL;
}



void Typemaker2_TypeManager_AddFolder(TYPEMAKER2_TYPEMANAGER *tym, const char *s) {
  assert(tym);
  GWEN_StringList_AppendString(tym->folders, s, 0, 1);
}



void Typemaker2_TypeManager_AddType(TYPEMAKER2_TYPEMANAGER *tym, TYPEMAKER2_TYPE *ty) {
  assert(tym);
  Typemaker2_Type_List_Add(ty, tym->typeList);
}



TYPEMAKER2_TYPE *Typemaker2_TypeManager_FindType(TYPEMAKER2_TYPEMANAGER *tym, const char *s) {
  TYPEMAKER2_TYPE *ty;

  assert(tym);
  ty=Typemaker2_Type_List_First(tym->typeList);
  while(ty) {
    const char *n;

    n=Typemaker2_Type_GetName(ty);
    if (n && strcasecmp(s, n)==0)
      break;
    ty=Typemaker2_Type_List_Next(ty);
  }

  return ty;
}



TYPEMAKER2_TYPE *Typemaker2_TypeManager_LoadType(TYPEMAKER2_TYPEMANAGER *tym, const char *typeName) {
  GWEN_BUFFER *tbuf;
  GWEN_BUFFER *nbuf;
  char *p;
  int rv;
  TYPEMAKER2_TYPE *ty=NULL;
  GWEN_XMLNODE *root;
  GWEN_XMLNODE *node;

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(tbuf, typeName);
  p=GWEN_Buffer_GetStart(tbuf);
  while(*p) {
    *p=tolower(*p);
    p++;
  }
  GWEN_Buffer_AppendString(tbuf, ".tm2");

  nbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_Directory_FindFileInPaths(tym->folders, GWEN_Buffer_GetStart(tbuf), nbuf);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Typefile [%s] not found (%d)", GWEN_Buffer_GetStart(tbuf), rv);
    GWEN_Buffer_free(nbuf);
    GWEN_Buffer_free(tbuf);
    return NULL;
  }

  /* read XML file */
  root=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "xml");

  rv=GWEN_XML_ReadFile(root, GWEN_Buffer_GetStart(nbuf),
		       GWEN_XML_FLAGS_DEFAULT |
		       GWEN_XML_FLAGS_HANDLE_HEADERS |
		       GWEN_XML_FLAGS_HANDLE_OPEN_HTMLTAGS);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not load typefile [%s] (%d)", GWEN_Buffer_GetStart(nbuf), rv);
    GWEN_XMLNode_free(root);
    GWEN_Buffer_free(nbuf);
    GWEN_Buffer_free(tbuf);
    return NULL;
  }

  /* get <tm2> element */
  node=GWEN_XMLNode_FindFirstTag(root, "tm2", NULL, NULL);
  if (node==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN,
	      "File [%s] does not contain a tm2 element",
	      GWEN_Buffer_GetStart(nbuf));
    GWEN_XMLNode_free(root);
    GWEN_Buffer_free(nbuf);
    GWEN_Buffer_free(tbuf);
    return NULL;
  }

  /* get <typedef> element with id==typeName and wanted language */
  node=GWEN_XMLNode_FindFirstTag(node, "typedef", "id", typeName);
  while(node) {
    const char *s=GWEN_XMLNode_GetProperty(node, "lang", NULL);
    if (s && *s && strcasecmp(s, tym->lang)==0)
      break;
    node=GWEN_XMLNode_FindNextTag(node, "typedef", "id", typeName);
  }
  if (node==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN,
	      "File [%s] does not contain a typedef element for type [%s] and language [%s]",
	      GWEN_Buffer_GetStart(nbuf), typeName, tym->lang);
    GWEN_XMLNode_free(root);
    GWEN_Buffer_free(nbuf);
    GWEN_Buffer_free(tbuf);
    return NULL;
  }

  /* load typedef from XML element */
  ty=Typemaker2_Type_new();
  rv=Typemaker2_Type_readXml(ty, node, NULL);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error reading type [%s] from file [%s] (%d)",
	     typeName,
	     GWEN_Buffer_GetStart(nbuf),
	     rv);
    Typemaker2_Type_free(ty);
    GWEN_XMLNode_free(root);
    GWEN_Buffer_free(nbuf);
    GWEN_Buffer_free(tbuf);
    return NULL;
  }

  GWEN_XMLNode_free(root);
  GWEN_Buffer_free(nbuf);
  GWEN_Buffer_free(tbuf);

  return ty;
}



TYPEMAKER2_TYPE *Typemaker2_TypeManager_LoadTypeFile(TYPEMAKER2_TYPEMANAGER *tym, const char *fileName) {
  int rv;
  TYPEMAKER2_TYPE *ty=NULL;
  GWEN_XMLNODE *root;
  GWEN_XMLNODE *node;

  /* read XML file */
  root=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "xml");

  rv=GWEN_XML_ReadFile(root, fileName,
		       GWEN_XML_FLAGS_DEFAULT |
		       GWEN_XML_FLAGS_HANDLE_HEADERS |
		       GWEN_XML_FLAGS_HANDLE_OPEN_HTMLTAGS);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not load typefile [%s] (%d)", fileName, rv);
    GWEN_XMLNode_free(root);
    return NULL;
  }

  /* get <tm2> element */
  node=GWEN_XMLNode_FindFirstTag(root, "tm2", NULL, NULL);
  if (node==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN,
	      "File [%s] does not contain a tm2 element",
	      fileName);
    GWEN_XMLNode_free(root);
    return NULL;
  }

  /* get <type> element with id==typeName and wanted language */
  node=GWEN_XMLNode_FindFirstTag(node, "type", NULL, NULL);
  if (node==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN,
	      "File [%s] does not contain a type element",
              fileName);
    GWEN_XMLNode_free(root);
    return NULL;
  }

  /* load type from XML element */
  ty=Typemaker2_Type_new();
  rv=Typemaker2_Type_readXml(ty, node, tym->lang);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error reading type from file [%s] (%d)",
	     fileName,
	     rv);
    Typemaker2_Type_free(ty);
    GWEN_XMLNode_free(root);
    return NULL;
  }

  GWEN_XMLNode_free(root);

  /* add first, because other types might want to refer to this one */
  Typemaker2_Type_List_Add(ty, tym->typeList);

  /* set type pointers in this type structure */
  rv=Typemaker2_TypeManager_SetTypePtrs(tym, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    Typemaker2_Type_free(ty);
    return NULL;
  }

  /* set type pointers in the member structures */
  rv=Typemaker2_TypeManager_SetMemberTypePtrs(tym, ty);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    Typemaker2_Type_free(ty);
    return NULL;
  }


  return ty;
}



int Typemaker2_TypeManager_SetTypePtrs(TYPEMAKER2_TYPEMANAGER *tym, TYPEMAKER2_TYPE *ty) {
  if (Typemaker2_Type_GetExtendsPtr(ty)==NULL) {
    const char *s;

    s=Typemaker2_Type_GetExtends(ty);
    if (s && *s) {
      TYPEMAKER2_TYPE *tt;

      tt=Typemaker2_TypeManager_GetType(tym, s);
      if (tt==NULL) {
	DBG_INFO(GWEN_LOGDOMAIN, "Type for \"extends\" not found [%s]", s);
        return GWEN_ERROR_NOT_FOUND;
      }
      Typemaker2_Type_SetExtendsPtr(ty, tt);
    }
  }

  if (Typemaker2_Type_GetBaseTypePtr(ty)==NULL) {
    const char *s;

    s=Typemaker2_Type_GetBaseType(ty);
    if (s && *s) {
      TYPEMAKER2_TYPE *tt;

      tt=Typemaker2_TypeManager_GetType(tym, s);
      if (tt==NULL) {
	DBG_INFO(GWEN_LOGDOMAIN, "Type for \"basetype\" not found [%s]", s);
        return GWEN_ERROR_NOT_FOUND;
      }
      Typemaker2_Type_SetBaseTypePtr(ty, tt);
    }
  }

  if (Typemaker2_Type_GetInheritsPtr(ty)==NULL) {
    const char *s;

    s=Typemaker2_Type_GetInherits(ty);
    if (s && *s) {
      TYPEMAKER2_TYPE *tt;

      tt=Typemaker2_TypeManager_GetType(tym, s);
      if (tt==NULL) {
	DBG_INFO(GWEN_LOGDOMAIN, "Type for \"inherits\" not found [%s]", s);
        return GWEN_ERROR_NOT_FOUND;
      }
      Typemaker2_Type_SetInheritsPtr(ty, tt);
    }
  }

  return 0;
}



int Typemaker2_TypeManager_SetMemberTypePtrs(TYPEMAKER2_TYPEMANAGER *tym, TYPEMAKER2_TYPE *ty) {
  TYPEMAKER2_MEMBER_LIST *ml;

  ml=Typemaker2_Type_GetMembers(ty);
  if (ml) {
    TYPEMAKER2_MEMBER *m;
    int pos=0;

    /* set pointers */
    m=Typemaker2_Member_List_First(ml);
    while(m) {
      if (Typemaker2_Member_GetTypePtr(m)==NULL) {
	const char *s;

        /* set type pointer */
	s=Typemaker2_Member_GetTypeName(m);
	if (s && *s) {
	  TYPEMAKER2_TYPE *tt;
    
	  tt=Typemaker2_TypeManager_GetType(tym, s);
	  if (tt==NULL) {
	    DBG_INFO(GWEN_LOGDOMAIN, "Type for \"type\" not found [%s]", s);
	    return GWEN_ERROR_NOT_FOUND;
	  }
	  Typemaker2_Member_SetTypePtr(m, tt);
	}

        /* set enum pointer (if any) */
	if ((Typemaker2_Member_GetFlags(m) & TYPEMAKER2_FLAGS_ENUM) &&
	    Typemaker2_Member_GetEnumPtr(m)==NULL) {
	  s=Typemaker2_Member_GetEnumId(m);
	  if (s && *s) {
	    TYPEMAKER2_ENUM *te=Typemaker2_Type_FindEnum(ty, s);
	    if (te)
	      Typemaker2_Member_SetEnumPtr(m, te);
	    else {
	      DBG_ERROR(GWEN_LOGDOMAIN, "Enum [%s] not found", s);
	      return GWEN_ERROR_NOT_FOUND;
	    }
	  }
	}
      }

      m=Typemaker2_Member_List_Next(m);
    }

    /* update member positions (needed for toObject/fromObject functions) */
    m=Typemaker2_Member_List_First(ml);
    while(m) {
      if (!(Typemaker2_Member_GetFlags(m) & TYPEMAKER2_FLAGS_VOLATILE)) {
	const char *s;

	Typemaker2_Member_SetMemberPosition(m, pos++);

	/* create field id */
	s=Typemaker2_Type_GetName(ty);
	if (s && *s) {
	  GWEN_BUFFER *tbuf;
	  char *p;
    
	  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
	  GWEN_Buffer_AppendString(tbuf, s);
	  GWEN_Buffer_AppendString(tbuf, "_FIELD_");
	  s=Typemaker2_Member_GetName(m);
	  GWEN_Buffer_AppendString(tbuf, s);
	  /* all in capitals */
	  p=GWEN_Buffer_GetStart(tbuf);
	  while(*p) {
	    *p=toupper(*p);
	    p++;
	  }

	  Typemaker2_Member_SetFieldId(m, GWEN_Buffer_GetStart(tbuf));
	  GWEN_Buffer_free(tbuf);
	}
      }

      m=Typemaker2_Member_List_Next(m);
    }
    Typemaker2_Type_SetNonVolatileMemberCount(ty, pos);
    if (pos) {
      const char *s;
  
      /* create field id */
      s=Typemaker2_Type_GetName(ty);
      if (s && *s) {
	GWEN_BUFFER *tbuf;
	char *p;
  
	tbuf=GWEN_Buffer_new(0, 256, 0, 1);
	GWEN_Buffer_AppendString(tbuf, s);
	GWEN_Buffer_AppendString(tbuf, "_FIELD_COUNT");
	/* all in capitals */
	p=GWEN_Buffer_GetStart(tbuf);
	while(*p) {
	  *p=toupper(*p);
	  p++;
	}

	Typemaker2_Type_SetFieldCountId(ty, GWEN_Buffer_GetStart(tbuf));
	GWEN_Buffer_free(tbuf);
      }

    }
  }

  return 0;
}



TYPEMAKER2_TYPE *Typemaker2_TypeManager_GetType(TYPEMAKER2_TYPEMANAGER *tym, const char *s) {
  TYPEMAKER2_TYPE *ty;

  ty=Typemaker2_TypeManager_FindType(tym, s);
  if (ty==NULL) {
    ty=Typemaker2_TypeManager_LoadType(tym, s);
    if (ty) {
      int rv;

      /* add first, because other types might want to refer to this one */
      Typemaker2_Type_List_Add(ty, tym->typeList);

      /* set type pointers in this type structure */
      rv=Typemaker2_TypeManager_SetTypePtrs(tym, ty);
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        return NULL;
      }

      /* set type pointers in the member structures */
      rv=Typemaker2_TypeManager_SetMemberTypePtrs(tym, ty);
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	return NULL;
      }
    }
    else {
      DBG_INFO(GWEN_LOGDOMAIN, "here");
    }
  }

  if (ty==NULL) {
    DBG_INFO(GWEN_LOGDOMAIN, "Type [%s] not found", s);
  }

  return ty;
}



void Typemaker2_TypeManager_Dump(TYPEMAKER2_TYPEMANAGER *tym, FILE *f, int indent) {
  TYPEMAKER2_TYPE *ty;
  int i;

  for (i=0; i<indent; i++) fprintf(f, " ");
  fprintf(f, "TypeManager\n");

  for (i=0; i<indent; i++) fprintf(f, " ");
  fprintf(f, "Types\n");

  ty=Typemaker2_Type_List_First(tym->typeList);
  while(ty) {
    Typemaker2_Type_Dump(ty, f, indent+2);
    ty=Typemaker2_Type_List_Next(ty);
  }
}





