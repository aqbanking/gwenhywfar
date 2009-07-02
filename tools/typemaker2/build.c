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

#include "typemaker2.h"
#include "tm_typemanager.h"
#include "tm_builder_c.h"

#include <gwenhywfar/debug.h>



int buildFile(GWEN_DB_NODE *dbArgs, const char *fname) {
  TYPEMAKER2_TYPEMANAGER *tym;
  TYPEMAKER2_BUILDER *tb=NULL;
  TYPEMAKER2_TYPE *ty;
  const char *s;
  int i;
  int rv;

  tym=Typemaker2_TypeManager_new();

  s=GWEN_DB_GetCharValue(dbArgs, "api", 0, NULL);
  if (s && *s)
    Typemaker2_TypeManager_SetApiDeclaration(tym, s);

  s=GWEN_DB_GetCharValue(dbArgs, "language", 0, "c");
  if (s && *s) {
    Typemaker2_TypeManager_SetLanguage(tym, s);
    if (strcasecmp(s, "c")==0) {
      tb=Typemaker2_Builder_C_new();
      Typemaker2_Builder_SetTypeManager(tb, tym);
    }
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unsupported language [%s]", s);
      return 1;
    }
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Missing language specification");
    return 1;
  }

  for (i=0; i<99; i++) {
    s=GWEN_DB_GetCharValue(dbArgs, "include", 0, NULL);
    if (s && *s)
      Typemaker2_TypeManager_AddFolder(tym, s);
  }

  ty=Typemaker2_TypeManager_LoadTypeFile(tym, fname);
  if (ty==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Unable to load file [%s]", fname);
    return 2;
  }

  /* DEBUG */
  Typemaker2_TypeManager_Dump(tym, stderr, 2);

  /* build */
  rv=Typemaker2_Builder_Build(tb, ty);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "here (%d)", rv);
    return 2;
  }

  return 0;
}



int build(GWEN_DB_NODE *dbArgs) {
  const char *fileName;
  int rv=0;

  fileName=GWEN_DB_GetCharValue(dbArgs, "params", 0, NULL);
  if (fileName==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No input");
    return 1;
  }
  else {
    rv=buildFile(dbArgs, fileName);
  }

  return rv;
}


