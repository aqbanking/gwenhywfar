/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



#include "args.h"
#include "typemaker_p.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/logger.h>
#include <gwenhywfar/xml.h>
#include <gwenhywfar/bufferedio.h>

#include <stdlib.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>



int write_code_freeElem_c(ARGUMENTS *args,
                          GWEN_XMLNODE *node,
                          GWEN_BUFFEREDIO *bio){
  const char *typ;
  const char *name;
  GWEN_ERRORCODE err;

  if (atoi(get_property(node, "ptr", "0"))==0)
    return 0;

  typ=GWEN_XMLNode_GetProperty(node, "type", 0);
  if (!typ) {
    DBG_ERROR(0, "No type for element");
    return -1;
  }

  name=GWEN_XMLNode_GetProperty(node, "name", 0);
  if (!name) {
    DBG_ERROR(0, "No type for element");
    return -1;
  }

  err=GWEN_BufferedIO_Write(bio, "  if (st->");
  if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
  err=GWEN_BufferedIO_Write(bio, name);
  if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
  err=GWEN_BufferedIO_WriteLine(bio, ")");
  if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}

  if (strcmp(typ, "char")==0) {
    /* we can handle chars */
    err=GWEN_BufferedIO_Write(bio, "    free(st->");
    if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
    err=GWEN_BufferedIO_Write(bio, name);
    if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
    err=GWEN_BufferedIO_WriteLine(bio, ");");
    if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
    return 0;
  }
  else {
    const char *fname;

    fname=get_function_name(node, "free");
    if (fname) {
      err=GWEN_BufferedIO_Write(bio, "    ");
      if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
      err=GWEN_BufferedIO_Write(bio, fname);
      if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
      err=GWEN_BufferedIO_Write(bio, "(st->");
      if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
      err=GWEN_BufferedIO_Write(bio, name);
      if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
      err=GWEN_BufferedIO_WriteLine(bio, ");");
      if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
      return 0;
    }
  }

  DBG_ERROR(0, "Unknown \"free\" function for type \"%s\"", typ);
  return -1;
}



int write_code_freeElems_c(ARGUMENTS *args,
                           GWEN_XMLNODE *node,
                           GWEN_BUFFEREDIO *bio){
  if (GWEN_XMLNode_GetType(node)==GWEN_XMLNodeTypeTag) {
    GWEN_XMLNODE *n;

    n=GWEN_XMLNode_GetFirstTag(node);
    while(n) {
      int rv;

      if (strcasecmp(GWEN_XMLNode_GetData(n), "group")==0)
        rv=write_code_freeElems_c(args, n, bio);
      else if (strcasecmp(GWEN_XMLNode_GetData(n), "elem")==0)
        rv=write_code_freeElem_c(args, n, bio);
      else {
        rv=0;
      }

      if (rv)
        return rv;
      n=GWEN_XMLNode_GetNextTag(n);
    } /* while */
  }

  return 0;
}



int write_code_dupArg_c(ARGUMENTS *args,
                        GWEN_XMLNODE *node,
                        GWEN_BUFFEREDIO *bio,
                        const char *param){
  const char *typ;
  const char *name;
  GWEN_ERRORCODE err;

  typ=GWEN_XMLNode_GetProperty(node, "type", 0);
  if (!typ) {
    DBG_ERROR(0, "No type for element");
    return -1;
  }

  name=GWEN_XMLNode_GetProperty(node, "name", 0);
  if (!name) {
    DBG_ERROR(0, "No type for element");
    return -1;
  }

  if (strcmp(typ, "char")==0) {
    /* we can handle chars */
    err=GWEN_BufferedIO_Write(bio, "strdup(");
    if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
    err=GWEN_BufferedIO_Write(bio, param);
    if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
    err=GWEN_BufferedIO_WriteLine(bio, ");");
    if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
    return 0;
  }
  else {
    const char *fname;

    fname=get_function_name(node, "dup");
    err=GWEN_BufferedIO_Write(bio, fname);
    if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
    err=GWEN_BufferedIO_Write(bio, "(");
    err=GWEN_BufferedIO_Write(bio, param);
    if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
    err=GWEN_BufferedIO_WriteLine(bio, ");");
    if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
    return 0;
  }

  DBG_ERROR(0, "Unknown \"dup\" function for type \"%s\"", typ);
  return -1;
}



int write_code_todbArg_c(ARGUMENTS *args,
                         GWEN_XMLNODE *node,
                         GWEN_BUFFEREDIO *bio) {
  const char *btype;
  const char *typ;
  const char *name;
  int isPtr;

  isPtr=atoi(get_property(node, "ptr", "0"));

  name=GWEN_XMLNode_GetProperty(node, "name", 0);
  if (!name) {
    DBG_ERROR(0, "No name for element");
    return -1;
  }

  typ=GWEN_XMLNode_GetProperty(node, "type", 0);
  if (!typ) {
    DBG_ERROR(0, "No type for element");
    return -1;
  }

  if (isPtr) {
    const char *fname;

    fname=get_function_name(node, "todb");
    if (fname) {
      GWEN_BufferedIO_Write(bio, "    if (");
      GWEN_BufferedIO_Write(bio, fname);
      GWEN_BufferedIO_Write(bio, "(st->");
      GWEN_BufferedIO_WriteChar(bio, tolower(*name));
      GWEN_BufferedIO_Write(bio, name+1);
      GWEN_BufferedIO_Write(bio,
                            ", GWEN_DB_GetGroup(db, "
                            "GWEN_DB_FLAGS_DEFAULT, \"");
      GWEN_BufferedIO_WriteChar(bio, tolower(*name));
      GWEN_BufferedIO_Write(bio, name+1);
      GWEN_BufferedIO_WriteLine(bio, "\")))");
      GWEN_BufferedIO_WriteLine(bio, "      return -1;");
    }
    else {
      if (strcasecmp(typ, "char")==0) {
        GWEN_BufferedIO_Write(bio,
                                  "    if (GWEN_DB_SetCharValue(db, "
                                  "GWEN_DB_FLAGS_OVERWRITE_VARS, \"");
        GWEN_BufferedIO_WriteChar(bio, tolower(*name));
        GWEN_BufferedIO_Write(bio, name+1);
        GWEN_BufferedIO_Write(bio, "\", st->");
        GWEN_BufferedIO_WriteChar(bio, tolower(*name));
        GWEN_BufferedIO_Write(bio, name+1);
        GWEN_BufferedIO_WriteLine(bio, "))");
        GWEN_BufferedIO_WriteLine(bio, "      return -1;");
      }
      else if (strcasecmp(typ, "GWEN_STRINGLIST")==0) {
        GWEN_BufferedIO_WriteLine(bio, "    {");
        GWEN_BufferedIO_WriteLine(bio, "      GWEN_STRINGLISTENTRY *se;");

        GWEN_BufferedIO_WriteLine(bio, "");
        GWEN_BufferedIO_Write(bio,"      GWEN_DB_DeleteVar(db, \"");
        GWEN_BufferedIO_WriteChar(bio, tolower(*name));
        GWEN_BufferedIO_Write(bio, name+1);
        GWEN_BufferedIO_WriteLine(bio, "\");");

        GWEN_BufferedIO_Write(bio, "      se=GWEN_StringList_FirstEntry(st->");
        GWEN_BufferedIO_WriteChar(bio, tolower(*name));
        GWEN_BufferedIO_Write(bio, name+1);
        GWEN_BufferedIO_WriteLine(bio, ");");

        GWEN_BufferedIO_WriteLine(bio, "      while(se) {");
        GWEN_BufferedIO_WriteLine(bio, "        const char *s;");
        GWEN_BufferedIO_WriteLine(bio, "");

        GWEN_BufferedIO_WriteLine(bio, "        s=GWEN_StringListEntry_Data(se);");
        GWEN_BufferedIO_WriteLine(bio, "        assert(s);");
        GWEN_BufferedIO_Write(bio,
                              "        if (GWEN_DB_SetCharValue(db, "
                              "GWEN_DB_FLAGS_DEFAULT, \"");
        GWEN_BufferedIO_WriteChar(bio, tolower(*name));
        GWEN_BufferedIO_Write(bio, name+1);
        GWEN_BufferedIO_WriteLine(bio, "\", s))");
        GWEN_BufferedIO_WriteLine(bio, "          return -1;");

        GWEN_BufferedIO_WriteLine(bio, "        se=GWEN_StringListEntry_Next(se);");
        GWEN_BufferedIO_WriteLine(bio, "      } /* while */");
        GWEN_BufferedIO_WriteLine(bio, "    }");
      }
      else {
        DBG_ERROR(0, "No toDb function for type \"%s\"", typ);
        return -1;
      }
    }

  }
  else {
    btype=get_property(node, "basetype", 0);
    if (!btype) {
      if (strcasecmp(typ, "char")==0)
        btype="char";
      else if (strcasecmp(typ, "GWEN_TYPE_UINT32")==0)
        btype="int";
      else if (strcasecmp(typ, "GWEN_TYPE_UINT64")==0)
        btype="int";
      else {
        btype=typ;
      }
    }
    if (strcasecmp(btype, "int")==0) {
      GWEN_BufferedIO_Write(bio,
                                "  if (GWEN_DB_SetIntValue(db, "
                                "GWEN_DB_FLAGS_OVERWRITE_VARS, \"");
    }
    else if (strcasecmp(btype, "char")==0) {
      GWEN_BufferedIO_Write(bio,
                                "  if (GWEN_DB_SetCharValue(db, "
                                "GWEN_DB_FLAGS_OVERWRITE_VARS, \"");
    }
    else {
      DBG_ERROR(0, "Unknown base type \"%s\"", btype);
      return -1;
    }

    GWEN_BufferedIO_WriteChar(bio, tolower(*name));
    GWEN_BufferedIO_Write(bio, name+1);
    GWEN_BufferedIO_Write(bio, "\", st->");
    GWEN_BufferedIO_WriteChar(bio, tolower(*name));
    GWEN_BufferedIO_Write(bio, name+1);
    GWEN_BufferedIO_WriteLine(bio, "))");
    GWEN_BufferedIO_WriteLine(bio, "    return -1;");
  }

  return 0;
}



int write_code_fromdbArg_c(ARGUMENTS *args,
                           GWEN_XMLNODE *node,
                           GWEN_BUFFEREDIO *bio) {
  const char *btype;
  const char *typ;
  const char *name;
  int isPtr;
  const char *defval;

  isPtr=atoi(get_property(node, "ptr", "0"));

  name=GWEN_XMLNode_GetProperty(node, "name", 0);
  if (!name) {
    DBG_ERROR(0, "No name for element");
    return -1;
  }

  typ=GWEN_XMLNode_GetProperty(node, "type", 0);
  if (!typ) {
    DBG_ERROR(0, "No type for element");
    return -1;
  }

  defval=get_property(node, "default", 0);

  if (isPtr) {
    const char *fname;

    fname=get_function_name(node, "fromdb");
    if (fname) {
      GWEN_BufferedIO_Write(bio, fname);
      GWEN_BufferedIO_Write(bio, "(dbT)");
    }
    else {
      if (strcasecmp(typ, "char")==0) {
        GWEN_BufferedIO_Write(bio, "GWEN_DB_GetCharValue(db, \"");
        GWEN_BufferedIO_WriteChar(bio, tolower(*name));
        GWEN_BufferedIO_Write(bio, name+1);
        GWEN_BufferedIO_Write(bio, "\", 0, ");
        if (defval) {
          GWEN_BufferedIO_Write(bio, "\"");
          GWEN_BufferedIO_Write(bio, defval);
          GWEN_BufferedIO_Write(bio, "\"");
        }
        else {
          GWEN_BufferedIO_Write(bio, "0");
        }
        GWEN_BufferedIO_Write(bio, ")");
      }
      else {
        DBG_ERROR(0, "No toDb function for type \"%s\"", typ);
        return -1;
      }
    }

  }
  else {
    btype=get_property(node, "basetype", 0);
    if (!btype) {
      if (strcasecmp(typ, "char")==0)
        btype="char";
      else if (strcasecmp(typ, "GWEN_TYPE_UINT32")==0)
        btype="int";
      else if (strcasecmp(typ, "GWEN_TYPE_UINT64")==0)
        btype="int";
      else {
        btype=typ;
      }
    }
    if (strcasecmp(btype, "int")==0) {
      GWEN_BufferedIO_Write(bio, "GWEN_DB_GetIntValue(db, \"");
      GWEN_BufferedIO_WriteChar(bio, tolower(*name));
      GWEN_BufferedIO_Write(bio, name+1);
      GWEN_BufferedIO_Write(bio, "\", 0, ");
      if (defval) {
        GWEN_BufferedIO_Write(bio, defval);
      }
      else {
        GWEN_BufferedIO_Write(bio, "0");
      }
      GWEN_BufferedIO_Write(bio, ")");
    }
    else if (strcasecmp(btype, "char")==0) {
      GWEN_BufferedIO_Write(bio, "GWEN_DB_GetCharValue(db, \"");
      GWEN_BufferedIO_WriteChar(bio, tolower(*name));
      GWEN_BufferedIO_Write(bio, name+1);
      GWEN_BufferedIO_Write(bio, "\", 0, ");
      if (defval) {
        GWEN_BufferedIO_Write(bio, "\"");
        GWEN_BufferedIO_Write(bio, defval);
        GWEN_BufferedIO_Write(bio, "\"");
      }
      else {
        GWEN_BufferedIO_Write(bio, "0");
      }
      GWEN_BufferedIO_Write(bio, ")");
    }
    else {
      DBG_ERROR(0, "Unknown base type \"%s\"", btype);
      return -1;
    }
  }

  return 0;
}



int write_code_constrec_c(ARGUMENTS *args,
                          GWEN_XMLNODE *node,
			  GWEN_BUFFEREDIO *bio){
  GWEN_XMLNODE *n;
  int rv;
  const char *prefix;
  const char *styp;

  prefix=get_struct_property(node, "prefix", 0);
  if (!prefix) {
    DBG_ERROR(0, "No prefix in struct");
    return -1;
  }
  styp=get_struct_property(node, "id", 0);
  if (!styp) {
    DBG_ERROR(0, "No id in struct");
    return -1;
  }

  n=GWEN_XMLNode_GetFirstTag(node);
  while(n) {
    if (GWEN_XMLNode_GetType(n)==GWEN_XMLNodeTypeTag) {
      if (strcasecmp(GWEN_XMLNode_GetData(n), "group")==0) {
	rv=write_code_constrec_c(args, n, bio);
	if (rv) {
	  DBG_ERROR(0, "Error in dup");
	  return rv;
	}
      }
      else if (strcasecmp(GWEN_XMLNode_GetData(n), "elem")==0) {
        int isPtr;
        const char *typ;
        const char *name;
	const char *setval;

        name=GWEN_XMLNode_GetProperty(n, "name", 0);
        if (!name) {
          DBG_ERROR(0, "No name for element");
          return -1;
        }

        typ=GWEN_XMLNode_GetProperty(n, "type", 0);
        if (!typ) {
          DBG_ERROR(0, "No type for element");
          return -1;
        }

	setval=GWEN_XMLNode_GetProperty(n, "preset", 0);
        isPtr=atoi(get_property(n, "ptr", "0"));

	if (isPtr) {
	  if (strcasecmp(typ, "GWEN_STRINGLIST")==0) {
	    GWEN_BufferedIO_Write(bio, "  st->");
	    GWEN_BufferedIO_WriteChar(bio, tolower(*name));
	    GWEN_BufferedIO_Write(bio, name+1);
	    GWEN_BufferedIO_WriteLine(bio, "=GWEN_StringList_new();");
	  }
	}
	else {
	  if (setval) {
	    GWEN_BufferedIO_Write(bio, "  st->");
	    GWEN_BufferedIO_WriteChar(bio, tolower(*name));
	    GWEN_BufferedIO_Write(bio, name+1);
	    GWEN_BufferedIO_Write(bio, "=");
	    GWEN_BufferedIO_Write(bio, setval);
	    GWEN_BufferedIO_WriteLine(bio, ";");
	  }
	}
      }
    }
    n=GWEN_XMLNode_GetNextTag(n);
  }
  return 0;
}




int write_code_const_c(ARGUMENTS *args,
                       GWEN_XMLNODE *node,
                       GWEN_BUFFEREDIO *bio){
  GWEN_ERRORCODE err;
  const char *prefix;
  const char *styp;
  int rv;

  prefix=get_struct_property(node, "prefix", 0);
  if (!prefix) {
    DBG_ERROR(0, "No prefix in struct");
    return -1;
  }
  styp=get_struct_property(node, "id", 0);
  if (!styp) {
    DBG_ERROR(0, "No id in struct");
    return -1;
  }

  err=GWEN_BufferedIO_Write(bio, styp);
  if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
  err=GWEN_BufferedIO_Write(bio, " *");
  if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
  err=GWEN_BufferedIO_Write(bio, prefix);
  if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
  err=GWEN_BufferedIO_WriteLine(bio, "_new() {");
  if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}

  err=GWEN_BufferedIO_Write(bio, "  ");
  if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
  err=GWEN_BufferedIO_Write(bio, styp);
  if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
  err=GWEN_BufferedIO_WriteLine(bio, " *st;");
  if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
  err=GWEN_BufferedIO_WriteLine(bio, "");
  if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}

  err=GWEN_BufferedIO_Write(bio, "  GWEN_NEW_OBJECT(");
  if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
  err=GWEN_BufferedIO_Write(bio, styp);
  if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
  err=GWEN_BufferedIO_WriteLine(bio, ", st)");
  if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
  err=GWEN_BufferedIO_WriteLine(bio, "  st->_usage=1;");
  if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}

  // add inherit functions
  if (get_struct_property(node, "inherit", 0)) {
    err=GWEN_BufferedIO_Write(bio, "  GWEN_INHERIT_INIT(");
    if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
    err=GWEN_BufferedIO_Write(bio, styp);
    if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
    err=GWEN_BufferedIO_WriteLine(bio, ", st)");
    if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
  }

  // add list functions
  if (get_struct_property(node, "list", 0)) {
    err=GWEN_BufferedIO_Write(bio, "  GWEN_LIST_INIT(");
    if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
    err=GWEN_BufferedIO_Write(bio, styp);
    if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
    err=GWEN_BufferedIO_WriteLine(bio, ", st)");
    if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
  }

  rv=write_code_constrec_c(args, node, bio);
  if (rv)
    return rv;

  err=GWEN_BufferedIO_WriteLine(bio, "  return st;");
  if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
  err=GWEN_BufferedIO_WriteLine(bio, "}");
  if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}

  return 0;
}



int write_code_dest_c(ARGUMENTS *args,
                      GWEN_XMLNODE *node,
                      GWEN_BUFFEREDIO *bio){
  const char *prefix;
  const char *styp;
  int rv;

  prefix=get_struct_property(node, "prefix", 0);
  if (!prefix) {
    DBG_ERROR(0, "No prefix in struct");
    return -1;
  }
  styp=get_struct_property(node, "id", 0);
  if (!styp) {
    DBG_ERROR(0, "No id in struct");
    return -1;
  }

  GWEN_BufferedIO_Write(bio, "void ");
  GWEN_BufferedIO_Write(bio, prefix);
  GWEN_BufferedIO_Write(bio, "_free(");
  GWEN_BufferedIO_Write(bio, styp);
  GWEN_BufferedIO_WriteLine(bio, " *st) {");

  GWEN_BufferedIO_WriteLine(bio, "  if (st) {");

  GWEN_BufferedIO_WriteLine(bio, "    assert(st->_usage);");
  GWEN_BufferedIO_WriteLine(bio, "    if (--(st->_usage)==0) {");

  // add inherit functions
  if (get_struct_property(node, "inherit", 0)) {
    GWEN_BufferedIO_Write(bio, "  GWEN_INHERIT_FINI(");
    GWEN_BufferedIO_Write(bio, styp);
    GWEN_BufferedIO_WriteLine(bio, ", st)");
  }

  rv=write_code_freeElems_c(args, node, bio);
  if (rv) {
    DBG_ERROR(0, "Error in freeElems");
    return rv;
  }

  // add list functions
  if (get_struct_property(node, "list", 0)) {
    GWEN_BufferedIO_Write(bio, "  GWEN_LIST_FINI(");
    GWEN_BufferedIO_Write(bio, styp);
    GWEN_BufferedIO_WriteLine(bio, ", st)");
  }

  GWEN_BufferedIO_WriteLine(bio, "  GWEN_FREE_OBJECT(st);");
  GWEN_BufferedIO_WriteLine(bio, "    }");
  GWEN_BufferedIO_WriteLine(bio, "  }");
  GWEN_BufferedIO_WriteLine(bio, "");
  GWEN_BufferedIO_WriteLine(bio, "}");

  return 0;
}



int write_code_setget_c(ARGUMENTS *args,
                        GWEN_XMLNODE *node,
                        GWEN_BUFFEREDIO *bio){
  GWEN_XMLNODE *n;
  int rv;
  const char *prefix;
  const char *styp;

  prefix=get_struct_property(node, "prefix", 0);
  if (!prefix) {
    DBG_ERROR(0, "No prefix in struct");
    return -1;
  }
  styp=get_struct_property(node, "id", 0);
  if (!styp) {
    DBG_ERROR(0, "No id in struct");
    return -1;
  }
  n=GWEN_XMLNode_GetFirstTag(node);
  while(n) {
    if (GWEN_XMLNode_GetType(n)==GWEN_XMLNodeTypeTag) {
      if (strcasecmp(GWEN_XMLNode_GetData(n), "group")==0) {
        rv=write_code_setget_c(args, n, bio);
        if (rv) {
          DBG_ERROR(0, "Error in setget");
          return rv;
        }
      }
      else if (strcasecmp(GWEN_XMLNode_GetData(n), "elem")==0) {
        int isPtr;
        const char *typ;
        const char *name;

        GWEN_BufferedIO_WriteLine(bio, "");
        GWEN_BufferedIO_WriteLine(bio, "");

        name=GWEN_XMLNode_GetProperty(n, "name", 0);
        if (!name) {
          DBG_ERROR(0, "No name for element");
          return -1;
        }

        typ=GWEN_XMLNode_GetProperty(n, "type", 0);
        if (!typ) {
          DBG_ERROR(0, "No type for element");
          return -1;
        }

        isPtr=atoi(get_property(n, "ptr", "0"));

        if (isPtr) {
          GWEN_BufferedIO_Write(bio, "const ");
        }
        GWEN_BufferedIO_Write(bio, typ);
        if (isPtr) {
          GWEN_BufferedIO_Write(bio, " *");
        }
        else {
          GWEN_BufferedIO_Write(bio, " ");
        }
        GWEN_BufferedIO_Write(bio, prefix);
        GWEN_BufferedIO_Write(bio, "_Get");
        GWEN_BufferedIO_WriteChar(bio, toupper(*name));
        GWEN_BufferedIO_Write(bio, name+1);
        GWEN_BufferedIO_Write(bio, "(const ");
        GWEN_BufferedIO_Write(bio, styp);
        GWEN_BufferedIO_WriteLine(bio, " *st) {");

        GWEN_BufferedIO_WriteLine(bio, "  assert(st);");

        GWEN_BufferedIO_Write(bio, "  return st->");
        GWEN_BufferedIO_WriteChar(bio, tolower(*name));
        GWEN_BufferedIO_Write(bio, name+1);
        GWEN_BufferedIO_WriteLine(bio, ";");
        GWEN_BufferedIO_WriteLine(bio, "}");

        /* write setter */
        GWEN_BufferedIO_WriteLine(bio, "");
        GWEN_BufferedIO_WriteLine(bio, "");

        GWEN_BufferedIO_Write(bio, "void ");
        GWEN_BufferedIO_Write(bio, prefix);
        GWEN_BufferedIO_Write(bio, "_Set");
        GWEN_BufferedIO_WriteChar(bio, toupper(*name));;
        GWEN_BufferedIO_Write(bio, name+1);
        GWEN_BufferedIO_Write(bio, "(");
        GWEN_BufferedIO_Write(bio, styp);
        GWEN_BufferedIO_Write(bio, " *st, ");
        if (isPtr) {
          GWEN_BufferedIO_Write(bio, "const ");
        }
        GWEN_BufferedIO_Write(bio, typ);
        if (isPtr) {
          GWEN_BufferedIO_Write(bio, " *");
        }
        else {
          GWEN_BufferedIO_Write(bio, " ");
        }
        GWEN_BufferedIO_WriteLine(bio, "d) {");
        GWEN_BufferedIO_WriteLine(bio, "  assert(st);");

        if (isPtr) {
          /* free old pointer if any */
          rv=write_code_freeElem_c(args, n, bio);
          if (rv)
            return rv;

          /* copy argument if any */
          GWEN_BufferedIO_WriteLine(bio, "  if (d)");

          GWEN_BufferedIO_Write(bio, "    st->");
          GWEN_BufferedIO_WriteChar(bio, tolower(*name));
          GWEN_BufferedIO_Write(bio, name+1);
          GWEN_BufferedIO_Write(bio, "=");
          rv=write_code_dupArg_c(args, n, bio, "d");
          if (rv)
            return rv;

          GWEN_BufferedIO_WriteLine(bio, "  else");
          GWEN_BufferedIO_Write(bio, "    st->");
          GWEN_BufferedIO_WriteChar(bio, tolower(*name));
          GWEN_BufferedIO_Write(bio, name+1);
          GWEN_BufferedIO_WriteLine(bio, "=0;");
        }
        else {
          GWEN_BufferedIO_Write(bio, "  st->");
          GWEN_BufferedIO_WriteChar(bio, tolower(*name));
          GWEN_BufferedIO_Write(bio, name+1);
          GWEN_BufferedIO_WriteLine(bio, "=d;");
        }
        GWEN_BufferedIO_WriteLine(bio, "  st->_modified=1;");
        GWEN_BufferedIO_WriteLine(bio, "}");

        if (strcasecmp(typ, "GWEN_STRINGLIST")==0) {
          /* special functions for string lists */
          GWEN_BufferedIO_WriteLine(bio, "");
          GWEN_BufferedIO_WriteLine(bio, "");
          GWEN_BufferedIO_Write(bio, "void ");
          GWEN_BufferedIO_Write(bio, prefix);
          GWEN_BufferedIO_Write(bio, "_Add");
          GWEN_BufferedIO_WriteChar(bio, toupper(*name));
          GWEN_BufferedIO_Write(bio, name+1);
          GWEN_BufferedIO_Write(bio, "(");
          GWEN_BufferedIO_Write(bio, styp);
          GWEN_BufferedIO_WriteLine(bio, " *st, const char *d, int chk){");
          GWEN_BufferedIO_WriteLine(bio, "  assert(st);");
          GWEN_BufferedIO_WriteLine(bio, "  assert(d);");
          GWEN_BufferedIO_Write(bio, "  if (GWEN_StringList_AppendString(st->");
          GWEN_BufferedIO_WriteChar(bio, tolower(*name));
          GWEN_BufferedIO_Write(bio, name+1);
          GWEN_BufferedIO_WriteLine(bio, ", d, 0, chk))");
          GWEN_BufferedIO_WriteLine(bio, "    st->_modified=1;");
          GWEN_BufferedIO_WriteLine(bio, "}");

          /* remove */
          GWEN_BufferedIO_WriteLine(bio, "");
          GWEN_BufferedIO_WriteLine(bio, "");
          GWEN_BufferedIO_Write(bio, "void ");
          GWEN_BufferedIO_Write(bio, prefix);
          GWEN_BufferedIO_Write(bio, "_Remove");
          GWEN_BufferedIO_WriteChar(bio, toupper(*name));
          GWEN_BufferedIO_Write(bio, name+1);
          GWEN_BufferedIO_Write(bio, "(");
          GWEN_BufferedIO_Write(bio, styp);
          GWEN_BufferedIO_WriteLine(bio, " *st, const char *d) {");
          GWEN_BufferedIO_Write(bio, "  if (GWEN_StringList_RemoveString(st->");
          GWEN_BufferedIO_WriteChar(bio, tolower(*name));
          GWEN_BufferedIO_Write(bio, name+1);
          GWEN_BufferedIO_WriteLine(bio, ", d))");
          GWEN_BufferedIO_WriteLine(bio, "    st->_modified=1;");
          GWEN_BufferedIO_WriteLine(bio, "}");

          /* clear */
          GWEN_BufferedIO_WriteLine(bio, "");
          GWEN_BufferedIO_WriteLine(bio, "");
          GWEN_BufferedIO_Write(bio, "void ");
          GWEN_BufferedIO_Write(bio, prefix);
          GWEN_BufferedIO_Write(bio, "_Clear");
          GWEN_BufferedIO_WriteChar(bio, toupper(*name));
          GWEN_BufferedIO_Write(bio, name+1);
          GWEN_BufferedIO_Write(bio, "(");
          GWEN_BufferedIO_Write(bio, styp);
          GWEN_BufferedIO_WriteLine(bio, " *st) {");
          GWEN_BufferedIO_Write(bio, "  if (GWEN_StringList_Count(st->");
          GWEN_BufferedIO_WriteChar(bio, tolower(*name));
          GWEN_BufferedIO_Write(bio, name+1);
          GWEN_BufferedIO_WriteLine(bio, ")) {");
          GWEN_BufferedIO_Write(bio, "    GWEN_StringList_Clear(st->");
          GWEN_BufferedIO_WriteChar(bio, tolower(*name));
          GWEN_BufferedIO_Write(bio, name+1);
          GWEN_BufferedIO_WriteLine(bio, ");");
          GWEN_BufferedIO_WriteLine(bio, "    st->_modified=1;");
          GWEN_BufferedIO_WriteLine(bio, "  }");
          GWEN_BufferedIO_WriteLine(bio, "}");

          /* has */
          GWEN_BufferedIO_WriteLine(bio, "");
          GWEN_BufferedIO_WriteLine(bio, "");
          GWEN_BufferedIO_Write(bio, "int ");
          GWEN_BufferedIO_Write(bio, prefix);
          GWEN_BufferedIO_Write(bio, "_Has");
          GWEN_BufferedIO_WriteChar(bio, toupper(*name));
          GWEN_BufferedIO_Write(bio, name+1);
          GWEN_BufferedIO_Write(bio, "(");
          GWEN_BufferedIO_Write(bio, styp);
          GWEN_BufferedIO_WriteLine(bio, " *st, const char *d) {");
          GWEN_BufferedIO_Write(bio, "  return GWEN_StringList_HasString(st->");
          GWEN_BufferedIO_WriteChar(bio, tolower(*name));
          GWEN_BufferedIO_Write(bio, name+1);
          GWEN_BufferedIO_WriteLine(bio, ", d);");
          GWEN_BufferedIO_WriteLine(bio, "}");
        }

      }
    }
    n=GWEN_XMLNode_GetNextTag(n);
  }
  return 0;
}



int write_code_duprec_c(ARGUMENTS *args, GWEN_XMLNODE *node,
                        GWEN_BUFFEREDIO *bio) {
  GWEN_XMLNODE *n;
  int rv;
  const char *prefix;
  const char *styp;

  prefix=get_struct_property(node, "prefix", 0);
  if (!prefix) {
    DBG_ERROR(0, "No prefix in struct");
    return -1;
  }
  styp=get_struct_property(node, "id", 0);
  if (!styp) {
    DBG_ERROR(0, "No id in struct");
    return -1;
  }

  n=GWEN_XMLNode_GetFirstTag(node);
  while(n) {
    if (GWEN_XMLNode_GetType(n)==GWEN_XMLNodeTypeTag) {
      if (strcasecmp(GWEN_XMLNode_GetData(n), "group")==0) {
        rv=write_code_duprec_c(args, n, bio);
        if (rv) {
          DBG_ERROR(0, "Error in dup");
          return rv;
        }
      }
      else if (strcasecmp(GWEN_XMLNode_GetData(n), "elem")==0) {
        int isPtr;
        const char *typ;
        const char *name;

        name=GWEN_XMLNode_GetProperty(n, "name", 0);
        if (!name) {
          DBG_ERROR(0, "No name for element");
          return -1;
        }

        typ=GWEN_XMLNode_GetProperty(n, "type", 0);
        if (!typ) {
          DBG_ERROR(0, "No type for element");
          return -1;
        }

        isPtr=atoi(get_property(n, "ptr", "0"));

        if (isPtr) {
          GWEN_BUFFER *pbuf;

          pbuf=GWEN_Buffer_new(0, 256, 0, 1);
          GWEN_Buffer_AppendString(pbuf, "d->");
          GWEN_Buffer_AppendByte(pbuf, tolower(*name));
          GWEN_Buffer_AppendString(pbuf, name+1);

          /* copy argument if any */
          GWEN_BufferedIO_Write(bio, "  if (d->");
          GWEN_BufferedIO_WriteChar(bio, tolower(*name));
          GWEN_BufferedIO_Write(bio, name+1);
          GWEN_BufferedIO_WriteLine(bio, ")");

          GWEN_BufferedIO_Write(bio, "    st->");
          GWEN_BufferedIO_WriteChar(bio, tolower(*name));
          GWEN_BufferedIO_Write(bio, name+1);
          GWEN_BufferedIO_Write(bio, "=");
          rv=write_code_dupArg_c(args, n, bio, GWEN_Buffer_GetStart(pbuf));
          GWEN_Buffer_free(pbuf);
          if (rv)
            return rv;
        }
        else {
          GWEN_BufferedIO_Write(bio, "  st->");
          GWEN_BufferedIO_WriteChar(bio, tolower(*name));
          GWEN_BufferedIO_Write(bio, name+1);
          GWEN_BufferedIO_Write(bio, "=d->");
          GWEN_BufferedIO_WriteChar(bio, tolower(*name));
          GWEN_BufferedIO_Write(bio, name+1);
          GWEN_BufferedIO_WriteLine(bio, ";");
        }
      }
    }
    n=GWEN_XMLNode_GetNextTag(n);
  }
  return 0;
}



int write_code_dup_c(ARGUMENTS *args, GWEN_XMLNODE *node,
                     GWEN_BUFFEREDIO *bio) {
  int rv;
  const char *prefix;
  const char *styp;

  prefix=get_struct_property(node, "prefix", 0);
  if (!prefix) {
    DBG_ERROR(0, "No prefix in struct");
    return -1;
  }
  styp=get_struct_property(node, "id", 0);
  if (!styp) {
    DBG_ERROR(0, "No id in struct");
    return -1;
  }

  GWEN_BufferedIO_Write(bio, styp);
  GWEN_BufferedIO_Write(bio, " *");
  GWEN_BufferedIO_Write(bio, prefix);
  GWEN_BufferedIO_Write(bio, "_dup(const ");
  GWEN_BufferedIO_Write(bio, styp);
  GWEN_BufferedIO_WriteLine(bio, " *d) {");

  GWEN_BufferedIO_Write(bio, "  ");
  GWEN_BufferedIO_Write(bio, styp);
  GWEN_BufferedIO_WriteLine(bio, " *st;");
  GWEN_BufferedIO_WriteLine(bio, "");

  GWEN_BufferedIO_WriteLine(bio, "  assert(d);");

  GWEN_BufferedIO_Write(bio, "  st=");
  GWEN_BufferedIO_Write(bio, prefix);
  GWEN_BufferedIO_WriteLine(bio, "_new();");

  rv=write_code_duprec_c(args, node, bio);
  if (rv) {
    DBG_ERROR(0, "Error in dup");
    return rv;
  }
  GWEN_BufferedIO_WriteLine(bio, "  return st;");
  GWEN_BufferedIO_WriteLine(bio, "}");
  return 0;
}



int write_code_todbrec_c(ARGUMENTS *args, GWEN_XMLNODE *node,
                         GWEN_BUFFEREDIO *bio) {
  GWEN_XMLNODE *n;
  int rv;
  const char *prefix;
  const char *styp;

  prefix=get_struct_property(node, "prefix", 0);
  if (!prefix) {
    DBG_ERROR(0, "No prefix in struct");
    return -1;
  }
  styp=get_struct_property(node, "id", 0);
  if (!styp) {
    DBG_ERROR(0, "No id in struct");
    return -1;
  }

  n=GWEN_XMLNode_GetFirstTag(node);
  while(n) {
    if (GWEN_XMLNode_GetType(n)==GWEN_XMLNodeTypeTag) {
      if (strcasecmp(GWEN_XMLNode_GetData(n), "group")==0) {
        rv=write_code_todbrec_c(args, n, bio);
        if (rv) {
          DBG_ERROR(0, "Error in todb");
          return rv;
        }
      }
      else if (strcasecmp(GWEN_XMLNode_GetData(n), "elem")==0) {
        int isPtr;
        const char *typ;
        const char *name;

        name=GWEN_XMLNode_GetProperty(n, "name", 0);
        if (!name) {
          DBG_ERROR(0, "No name for element");
          return -1;
        }

        typ=GWEN_XMLNode_GetProperty(n, "type", 0);
        if (!typ) {
          DBG_ERROR(0, "No type for element");
          return -1;
        }

        isPtr=atoi(get_property(n, "ptr", "0"));
        if (isPtr) {
          GWEN_BufferedIO_Write(bio, "  if (st->");
          GWEN_BufferedIO_WriteChar(bio, tolower(*name));
          GWEN_BufferedIO_Write(bio, name+1);
          GWEN_BufferedIO_WriteLine(bio, ")");
        }
        rv=write_code_todbArg_c(args, n, bio);
        if (rv)
          return rv;
      }
    }
    n=GWEN_XMLNode_GetNextTag(n);
  }
  return 0;
}



int write_code_todb_c(ARGUMENTS *args, GWEN_XMLNODE *node,
                      GWEN_BUFFEREDIO *bio) {
  int rv;
  const char *prefix;
  const char *styp;

  prefix=get_struct_property(node, "prefix", 0);
  if (!prefix) {
    DBG_ERROR(0, "No prefix in struct");
    return -1;
  }
  styp=get_struct_property(node, "id", 0);
  if (!styp) {
    DBG_ERROR(0, "No id in struct");
    return -1;
  }

  GWEN_BufferedIO_Write(bio, "int ");
  GWEN_BufferedIO_Write(bio, prefix);
  GWEN_BufferedIO_Write(bio, "_toDb(const ");
  GWEN_BufferedIO_Write(bio, styp);
  GWEN_BufferedIO_WriteLine(bio, " *st, GWEN_DB_NODE *db) {");


  GWEN_BufferedIO_WriteLine(bio, "  assert(st);");
  GWEN_BufferedIO_WriteLine(bio, "  assert(db);");

  rv=write_code_todbrec_c(args, node, bio);
  if (rv) {
    DBG_ERROR(0, "Error in todb");
    return rv;
  }
  GWEN_BufferedIO_WriteLine(bio, "  return 0;");
  GWEN_BufferedIO_WriteLine(bio, "}");
  return 0;
}



int write_code_fromdbrec_c(ARGUMENTS *args, GWEN_XMLNODE *node,
                           GWEN_BUFFEREDIO *bio) {
  GWEN_XMLNODE *n;
  int rv;
  const char *prefix;
  const char *styp;

  prefix=get_struct_property(node, "prefix", 0);
  if (!prefix) {
    DBG_ERROR(0, "No prefix in struct");
    return -1;
  }
  styp=get_struct_property(node, "id", 0);
  if (!styp) {
    DBG_ERROR(0, "No id in struct");
    return -1;
  }

  n=GWEN_XMLNode_GetFirstTag(node);
  while(n) {
    if (GWEN_XMLNode_GetType(n)==GWEN_XMLNodeTypeTag) {
      if (strcasecmp(GWEN_XMLNode_GetData(n), "group")==0) {
        rv=write_code_fromdbrec_c(args, n, bio);
        if (rv) {
          DBG_ERROR(0, "Error in fromdb");
          return rv;
        }
      }
      else if (strcasecmp(GWEN_XMLNode_GetData(n), "elem")==0) {
        int isPtr;
        const char *typ;
        const char *name;

        name=GWEN_XMLNode_GetProperty(n, "name", 0);
        if (!name) {
          DBG_ERROR(0, "No name for element");
          return -1;
        }

        typ=GWEN_XMLNode_GetProperty(n, "type", 0);
        if (!typ) {
          DBG_ERROR(0, "No type for element");
          return -1;
        }

        if (strcasecmp(typ, "GWEN_STRINGLIST")==0) {
          GWEN_BufferedIO_WriteLine(bio, "  if (1) {");
          GWEN_BufferedIO_WriteLine(bio, "    int i;");
          GWEN_BufferedIO_WriteLine(bio, "");
          GWEN_BufferedIO_WriteLine(bio, "    for (i=0; ; i++) {");
          GWEN_BufferedIO_WriteLine(bio, "      const char *s;");
          GWEN_BufferedIO_WriteLine(bio, "");
          GWEN_BufferedIO_Write(bio, "      s=GWEN_DB_GetCharValue(db, \"");
          GWEN_BufferedIO_WriteChar(bio, tolower(*name));
          GWEN_BufferedIO_Write(bio, name+1);
          GWEN_BufferedIO_WriteLine(bio, "\", i, 0);");
          GWEN_BufferedIO_WriteLine(bio, "      if (!s)");
          GWEN_BufferedIO_WriteLine(bio, "        break;");
          GWEN_BufferedIO_Write(bio, "      ");
          GWEN_BufferedIO_Write(bio, prefix);
          GWEN_BufferedIO_Write(bio, "_Add");
          GWEN_BufferedIO_WriteChar(bio, toupper(*name));
          GWEN_BufferedIO_Write(bio, name+1);
          GWEN_BufferedIO_WriteLine(bio, "(st, s, 0);");
          GWEN_BufferedIO_WriteLine(bio, "    } /* for */");
          GWEN_BufferedIO_WriteLine(bio, "  }");
        }
        else {
          isPtr=atoi(get_property(n, "ptr", "0"));
          if (isPtr) {
            GWEN_BufferedIO_WriteLine(bio, "  if (1) {");
            GWEN_BufferedIO_WriteLine(bio, "    GWEN_DB_NODE *dbT;");
            GWEN_BufferedIO_WriteLine(bio, "");
            GWEN_BufferedIO_Write(bio,
                                  "    dbT=GWEN_DB_GetGroup(db, "
                                  "GWEN_PATH_FLAGS_NAMEMUSTEXIST, \"");
            GWEN_BufferedIO_WriteChar(bio, tolower(*name));
            GWEN_BufferedIO_Write(bio, name+1);
            GWEN_BufferedIO_WriteLine(bio, "\");");
            GWEN_BufferedIO_Write(bio, "    if (dbT)");
          }
          GWEN_BufferedIO_Write(bio, "  ");
          GWEN_BufferedIO_Write(bio, prefix);
          GWEN_BufferedIO_Write(bio, "_Set");
          GWEN_BufferedIO_WriteChar(bio, toupper(*name));
          GWEN_BufferedIO_Write(bio, name+1);
          GWEN_BufferedIO_Write(bio, "(st, ");

          rv=write_code_fromdbArg_c(args, n, bio);
          if (rv)
            return rv;
          GWEN_BufferedIO_WriteLine(bio, ");");
          if (isPtr) {
            GWEN_BufferedIO_WriteLine(bio, "  }");
          }
        }
      }
    }
    n=GWEN_XMLNode_GetNextTag(n);
  }
  return 0;
}



int write_code_fromdb_c(ARGUMENTS *args, GWEN_XMLNODE *node,
                        GWEN_BUFFEREDIO *bio) {
  int rv;
  const char *prefix;
  const char *styp;

  prefix=get_struct_property(node, "prefix", 0);
  if (!prefix) {
    DBG_ERROR(0, "No prefix in struct");
    return -1;
  }
  styp=get_struct_property(node, "id", 0);
  if (!styp) {
    DBG_ERROR(0, "No id in struct");
    return -1;
  }

  GWEN_BufferedIO_Write(bio, styp);
  GWEN_BufferedIO_Write(bio, " *");
  GWEN_BufferedIO_Write(bio, prefix);
  GWEN_BufferedIO_WriteLine(bio, "_fromDb(GWEN_DB_NODE *db) {");

  GWEN_BufferedIO_Write(bio, styp);
  GWEN_BufferedIO_WriteLine(bio, " *st;");
  GWEN_BufferedIO_WriteLine(bio, "");
  GWEN_BufferedIO_WriteLine(bio, "  assert(db);");
  GWEN_BufferedIO_Write(bio, "  st=");
  GWEN_BufferedIO_Write(bio, prefix);
  GWEN_BufferedIO_WriteLine(bio, "_new();");

  rv=write_code_fromdbrec_c(args, node, bio);
  if (rv) {
    DBG_ERROR(0, "Error in fromdb");
    return rv;
  }

  GWEN_BufferedIO_WriteLine(bio, "  st->_modified=0;");
  GWEN_BufferedIO_WriteLine(bio, "  return st;");
  GWEN_BufferedIO_WriteLine(bio, "}");
  return 0;
}



int write_code_builtin_c(ARGUMENTS *args, GWEN_XMLNODE *node,
                         GWEN_BUFFEREDIO *bio) {
  const char *prefix;
  const char *styp;

  prefix=get_struct_property(node, "prefix", 0);
  if (!prefix) {
    DBG_ERROR(0, "No prefix in struct");
    return -1;
  }
  styp=get_struct_property(node, "id", 0);
  if (!styp) {
    DBG_ERROR(0, "No id in struct");
    return -1;
  }

  /* IsModified */
  GWEN_BufferedIO_Write(bio, "int ");
  GWEN_BufferedIO_Write(bio, prefix);
  GWEN_BufferedIO_Write(bio, "_IsModified(const ");
  GWEN_BufferedIO_Write(bio, styp);
  GWEN_BufferedIO_WriteLine(bio, " *st) {");
  GWEN_BufferedIO_WriteLine(bio, "  assert(st);");
  GWEN_BufferedIO_WriteLine(bio, "  return st->_modified;");
  GWEN_BufferedIO_WriteLine(bio, "}");

  GWEN_BufferedIO_WriteLine(bio, "");
  GWEN_BufferedIO_WriteLine(bio, "");

  /* SetModified */
  GWEN_BufferedIO_Write(bio, "void ");
  GWEN_BufferedIO_Write(bio, prefix);
  GWEN_BufferedIO_Write(bio, "_SetModified(");
  GWEN_BufferedIO_Write(bio, styp);
  GWEN_BufferedIO_WriteLine(bio, " *st, int i) {");
  GWEN_BufferedIO_WriteLine(bio, "  assert(st);");
  GWEN_BufferedIO_WriteLine(bio, "  st->_modified=i;");
  GWEN_BufferedIO_WriteLine(bio, "}");

  GWEN_BufferedIO_WriteLine(bio, "");
  GWEN_BufferedIO_WriteLine(bio, "");

  /* Attach */
  GWEN_BufferedIO_Write(bio, "void ");
  GWEN_BufferedIO_Write(bio, prefix);
  GWEN_BufferedIO_Write(bio, "_Attach(");
  GWEN_BufferedIO_Write(bio, styp);
  GWEN_BufferedIO_WriteLine(bio, " *st) {");
  GWEN_BufferedIO_WriteLine(bio, "  assert(st);");
  GWEN_BufferedIO_WriteLine(bio, "  st->_usage++;");
  GWEN_BufferedIO_WriteLine(bio, "}");

  if (get_struct_property(node, "list2", 0)) {
    GWEN_BufferedIO_Write(bio, styp);
    GWEN_BufferedIO_Write(bio, " *");
    GWEN_BufferedIO_Write(bio, prefix);
    GWEN_BufferedIO_Write(bio, "_List2__freeAll_cb(");
    GWEN_BufferedIO_Write(bio, styp);
    GWEN_BufferedIO_Write(bio, " *");
    GWEN_BufferedIO_WriteLine(bio, "st, void *user_data) {");

    GWEN_BufferedIO_Write(bio, "  ");
    GWEN_BufferedIO_Write(bio, prefix);
    GWEN_BufferedIO_WriteLine(bio, "_free(st);");
    GWEN_BufferedIO_WriteLine(bio, "return 0;");

    GWEN_BufferedIO_WriteLine(bio, "}");
    GWEN_BufferedIO_WriteLine(bio, "");
    GWEN_BufferedIO_WriteLine(bio, "");

    GWEN_BufferedIO_Write(bio, "void ");
    GWEN_BufferedIO_Write(bio, prefix);
    GWEN_BufferedIO_Write(bio, "_List2_freeAll(");
    GWEN_BufferedIO_Write(bio, styp);
    GWEN_BufferedIO_WriteLine(bio, "_LIST2 *stl) {");

    GWEN_BufferedIO_WriteLine(bio, "  if (stl) {");

    GWEN_BufferedIO_Write(bio, "    ");
    GWEN_BufferedIO_Write(bio, prefix);
    GWEN_BufferedIO_Write(bio, "_List2_ForEach(stl, ");
    GWEN_BufferedIO_Write(bio, prefix);
    GWEN_BufferedIO_WriteLine(bio, "_List2__freeAll_cb, 0);");

    GWEN_BufferedIO_Write(bio, "    ");
    GWEN_BufferedIO_Write(bio, prefix);
    GWEN_BufferedIO_WriteLine(bio, "_List2_free(stl); ");

    GWEN_BufferedIO_WriteLine(bio, "  }");
    GWEN_BufferedIO_WriteLine(bio, "}");
    GWEN_BufferedIO_WriteLine(bio, "");
    GWEN_BufferedIO_WriteLine(bio, "");

  }

  return 0;
}




int write_code_file_c(ARGUMENTS *args, GWEN_XMLNODE *node) {
  int rv;
  const char *f;
  GWEN_BUFFER *fname;
  int fd;
  const char *nacc;
  GWEN_BUFFEREDIO *bio;
  GWEN_ERRORCODE err;
  const char *id;
  const char *prefix;

  id=get_struct_property(node, "id", 0);
  if (!id) {
    DBG_ERROR(0, "No id for struct");
    return -1;
  }

  f=get_struct_property(node, "filename", 0);
  if (!f) {
    DBG_ERROR(0, "No filename given");
    return -1;
  }

  prefix=get_struct_property(node, "prefix", 0);
  if (!prefix) {
    DBG_ERROR(0, "No prefix in struct");
    return -1;
  }

  nacc=get_struct_property(node, "access", "public");

  fname=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(fname, f);
  GWEN_Buffer_AppendString(fname, ".c");
  fd=open(GWEN_Buffer_GetStart(fname),
	  O_RDWR|O_CREAT|O_TRUNC,
	  S_IRUSR|S_IWUSR);
  if (fd==-1) {
    DBG_ERROR(0, "open(%s): %s",
	      GWEN_Buffer_GetStart(fname),
	      strerror(errno));
    GWEN_Buffer_free(fname);
    return -1;
  }
  GWEN_Buffer_free(fname);

  bio=GWEN_BufferedIO_File_new(fd);
  GWEN_BufferedIO_SetWriteBuffer(bio, 0, 1024);

  /* Insert the auto-generation warning */
  GWEN_BufferedIO_Write(bio, "/* This file is auto-generated from \"");
  GWEN_BufferedIO_Write(bio, f);
  GWEN_BufferedIO_WriteLine(bio, ".xml\" by the typemaker");
  GWEN_BufferedIO_WriteLine(bio, "   tool of Gwenhywfar. ");
  GWEN_BufferedIO_WriteLine(bio, "   Do not edit this file -- all changes will be lost! */");

  fname=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(fname, f);
  GWEN_Buffer_AppendString(fname, "_p.h");

  GWEN_BufferedIO_Write(bio, "#include \"");
  GWEN_BufferedIO_Write(bio, GWEN_Buffer_GetStart(fname));
  GWEN_BufferedIO_WriteLine(bio, "\"");
  GWEN_Buffer_free(fname);

  GWEN_BufferedIO_WriteLine(bio, "#include <gwenhywfar/misc.h>");
  GWEN_BufferedIO_WriteLine(bio, "#include <gwenhywfar/db.h>");
  GWEN_BufferedIO_WriteLine(bio, "#include <assert.h>");
  GWEN_BufferedIO_WriteLine(bio, "#include <stdlib.h>");
  GWEN_BufferedIO_WriteLine(bio, "");
  GWEN_BufferedIO_WriteLine(bio, "#ifdef HAVE_CONFIG_H");
  GWEN_BufferedIO_WriteLine(bio, "# include \"config.h\"");
  GWEN_BufferedIO_WriteLine(bio, "#endif");
  GWEN_BufferedIO_WriteLine(bio, "");
  GWEN_BufferedIO_WriteLine(bio, "");

  if (get_struct_property(node, "inherit", 0)) {
    GWEN_BufferedIO_Write(bio, "GWEN_INHERIT_FUNCTIONS(");
    GWEN_BufferedIO_Write(bio, id);
    GWEN_BufferedIO_WriteLine(bio, ")");
  }

  if (get_struct_property(node, "list", 0)) {
    GWEN_BufferedIO_Write(bio, "GWEN_LIST_FUNCTIONS(");
    GWEN_BufferedIO_Write(bio, id);
    GWEN_BufferedIO_Write(bio, ", ");
    GWEN_BufferedIO_Write(bio, prefix);
    GWEN_BufferedIO_WriteLine(bio, ")");
  }

  if (get_struct_property(node, "list2", 0)) {
    GWEN_BufferedIO_Write(bio, "GWEN_LIST2_FUNCTIONS(");
    GWEN_BufferedIO_Write(bio, id);
    GWEN_BufferedIO_Write(bio, ", ");
    GWEN_BufferedIO_Write(bio, prefix);
    GWEN_BufferedIO_WriteLine(bio, ")");
  }

  GWEN_BufferedIO_WriteLine(bio, "");
  GWEN_BufferedIO_WriteLine(bio, "");

  rv=write_code_const_c(args, node, bio);
  if (rv) {
    GWEN_BufferedIO_Abandon(bio);
    GWEN_BufferedIO_free(bio);
    return rv;
  }

  GWEN_BufferedIO_WriteLine(bio, "");
  GWEN_BufferedIO_WriteLine(bio, "");

  rv=write_code_dest_c(args, node, bio);
  if (rv) {
    GWEN_BufferedIO_Abandon(bio);
    GWEN_BufferedIO_free(bio);
    return rv;
  }

  GWEN_BufferedIO_WriteLine(bio, "");
  GWEN_BufferedIO_WriteLine(bio, "");

  rv=write_code_dup_c(args, node, bio);
  if (rv) {
    GWEN_BufferedIO_Abandon(bio);
    GWEN_BufferedIO_free(bio);
    return rv;
  }

  GWEN_BufferedIO_WriteLine(bio, "");
  GWEN_BufferedIO_WriteLine(bio, "");

  rv=write_code_todb_c(args, node, bio);
  if (rv) {
    GWEN_BufferedIO_Abandon(bio);
    GWEN_BufferedIO_free(bio);
    return rv;
  }

  GWEN_BufferedIO_WriteLine(bio, "");
  GWEN_BufferedIO_WriteLine(bio, "");

  rv=write_code_fromdb_c(args, node, bio);
  if (rv) {
    GWEN_BufferedIO_Abandon(bio);
    GWEN_BufferedIO_free(bio);
    return rv;
  }

  rv=write_code_setget_c(args, node, bio);
  if (rv) {
    GWEN_BufferedIO_Abandon(bio);
    GWEN_BufferedIO_free(bio);
    return rv;
  }

  GWEN_BufferedIO_WriteLine(bio, "");
  GWEN_BufferedIO_WriteLine(bio, "");

  rv=write_code_builtin_c(args, node, bio);
  if (rv) {
    GWEN_BufferedIO_Abandon(bio);
    GWEN_BufferedIO_free(bio);
    return rv;
  }

  GWEN_BufferedIO_WriteLine(bio, "");


  err=GWEN_BufferedIO_Close(bio);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    GWEN_BufferedIO_free(bio);
    return -1;
  }

  GWEN_BufferedIO_free(bio);
  return 0;
}



int write_code_files_c(ARGUMENTS *args, GWEN_XMLNODE *node) {
  GWEN_XMLNODE *n;
  int rv;

  n=GWEN_XMLNode_FindFirstTag(node, "type", 0, 0);
  while (n) {
    rv=write_code_file_c(args, n);
    if (rv)
      return rv;
    n=GWEN_XMLNode_FindNextTag(n, "type", 0, 0);
  }
  return 0;
}








