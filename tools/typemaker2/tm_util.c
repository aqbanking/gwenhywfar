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


#include "tm_util.h"
#include "tm_type.h"

#include <gwenhywfar/stringlist.h>
#include <gwenhywfar/debug.h>



uint32_t Typemaker2_FlagsFromString(const char *s) {
  GWEN_STRINGLIST *sl;
  uint32_t flags=0;

  assert(s && *s);

  sl=GWEN_StringList_fromString(s, " ,:/", 1);
  if (sl) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(sl);
    while(se) {
      s=GWEN_StringListEntry_Data(se);
      assert(s);

      if (strcasecmp(s, "own")==0)
	flags|=TYPEMAKER2_FLAGS_OWN;
      else if (strcasecmp(s, "volatile")==0)
	flags|=TYPEMAKER2_FLAGS_VOLATILE;
      else if (strcasecmp(s, "const")==0)
	flags|=TYPEMAKER2_FLAGS_CONST;
      else if (strcasecmp(s, "static")==0)
	flags|=TYPEMAKER2_FLAGS_STATIC;
      else if (strcasecmp(s, "dup")==0)
	flags|=TYPEMAKER2_FLAGS_DUP;
      else if (strcasecmp(s, "nodup")==0)
	flags|=TYPEMAKER2_FLAGS_NODUP;
      else if (strcasecmp(s, "attribute")==0)
	flags|=TYPEMAKER2_FLAGS_ATTRIBUTE;
      else if (strcasecmp(s, "enum")==0)
	flags|=TYPEMAKER2_FLAGS_ENUM;
      else if (strcasecmp(s, "define")==0)
	flags|=TYPEMAKER2_FLAGS_DEFINE;
      else if (strcasecmp(s, "with_db")==0)
	flags|=TYPEMAKER2_FLAGS_WITH_DB;
      else if (strcasecmp(s, "with_xml")==0)
	flags|=TYPEMAKER2_FLAGS_WITH_XML;
      else if (strcasecmp(s, "with_object")==0)
	flags|=TYPEMAKER2_FLAGS_WITH_OBJECT;
      else if (strcasecmp(s, "with_list1")==0)
	flags|=TYPEMAKER2_FLAGS_WITH_LIST1;
      else if (strcasecmp(s, "with_list2")==0)
	flags|=TYPEMAKER2_FLAGS_WITH_LIST2;
      else if (strcasecmp(s, "with_inherit")==0)
	flags|=TYPEMAKER2_FLAGS_WITH_INHERIT;
      else if (strcasecmp(s, "with_idmap")==0)
	flags|=TYPEMAKER2_FLAGS_WITH_IDMAP;
      else if (strcasecmp(s, "with_getbymember")==0)
	flags|=TYPEMAKER2_FLAGS_WITH_GETBYMEMBER;
      else if (strcasecmp(s, "with_tree")==0)
	flags|=TYPEMAKER2_FLAGS_WITH_TREE;
      else if (strcasecmp(s, "with_hash")==0)
	flags|=TYPEMAKER2_FLAGS_WITH_HASH;
      else if (strcasecmp(s, "with_refcount")==0)
	flags|=TYPEMAKER2_FLAGS_WITH_REFCOUNT;

      else if (strcasecmp(s, "none")==0)
	flags|=TYPEMAKER2_FLAGS_NONE;

      se=GWEN_StringListEntry_Next(se);
    }

    GWEN_StringList_free(sl);
  }

  return flags;
}



int Typemaker2_AccessFromString(const char *s) {
  assert(s);
  if (strcasecmp(s, "pub")==0 ||
      strcasecmp(s, "public")==0)
    return TypeMaker2_Access_Public;
  else if (strcasecmp(s, "library")==0 ||
	   strcasecmp(s, "lib")==0)
    return TypeMaker2_Access_Library;
  else if (strcasecmp(s, "protected")==0 ||
	   strcasecmp(s, "prot")==0)
    return TypeMaker2_Access_Protected;
  else if (strcasecmp(s, "private")==0 ||
	   strcasecmp(s, "priv")==0)
    return TypeMaker2_Access_Private;

  return TypeMaker2_Access_Unknown;
}




