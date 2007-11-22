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



int write_c_enums(ARGUMENTS *args, GWEN_XMLNODE *node,
		  GWEN_BUFFEREDIO *bio) {
  GWEN_XMLNODE *n;

  n=GWEN_XMLNode_FindFirstTag(node, "subtypes", 0, 0);
  if (n)
    n=GWEN_XMLNode_FindFirstTag(n, "type", "mode", "enum");
  if (n) {
    GWEN_BUFFER *tprefix;
    GWEN_BUFFER *tid;
    uint32_t ppos;
    uint32_t tpos;
    const char *s;

    tprefix=GWEN_Buffer_new(0, 64, 0, 1);
    tid=GWEN_Buffer_new(0, 64, 0, 1);

    s=get_struct_property(node, "prefix", 0);
    assert(s);
    GWEN_Buffer_AppendString(tprefix, s);
    GWEN_Buffer_AppendString(tprefix, "_");
    ppos=GWEN_Buffer_GetPos(tprefix);

    s=get_struct_property(node, "id", 0);
    assert(s);
    GWEN_Buffer_AppendString(tid, s);
    GWEN_Buffer_AppendString(tid, "_");
    tpos=GWEN_Buffer_GetPos(tid);

    while(n) {
      GWEN_XMLNODE *nn;
      s=GWEN_XMLNode_GetProperty(n, "access", "public");

      s=GWEN_XMLNode_GetProperty(n, "id", 0);
      assert(s);
      GWEN_Buffer_AppendString(tid, s);
      s=GWEN_XMLNode_GetProperty(n, "prefix", 0);
      assert(s);
      GWEN_Buffer_AppendString(tprefix, s);

      GWEN_BufferedIO_Write(bio, GWEN_Buffer_GetStart(tid));
      GWEN_BufferedIO_Write(bio, " ");
      GWEN_BufferedIO_Write(bio, GWEN_Buffer_GetStart(tprefix));
      GWEN_BufferedIO_WriteLine(bio, "_fromString(const char *s) {");
      GWEN_BufferedIO_WriteLine(bio, "  if (s) {");

      nn=GWEN_XMLNode_FindFirstTag(n, "values", 0, 0);
      if (nn)
        nn=GWEN_XMLNode_FindFirstTag(nn, "value", 0, 0);
      if (nn) {
        uint32_t vpos;
        int first=1;

        vpos=GWEN_Buffer_GetPos(tprefix);
        while(nn) {
          GWEN_XMLNODE *nnn;

          nnn=GWEN_XMLNode_GetFirstData(nn);
          if (!nnn) {
            DBG_ERROR(0, "No values in enum description for \"%s\"",
                      GWEN_Buffer_GetStart(tid));
            GWEN_Buffer_free(tid);
            GWEN_Buffer_free(tprefix);
            return -1;
          }
          if (first)
            GWEN_BufferedIO_Write(bio, "    if (strcasecmp(s, \"");
          else
            GWEN_BufferedIO_Write(bio, "    else if (strcasecmp(s, \"");
          s=GWEN_XMLNode_GetData(nnn);
          assert(s);
          GWEN_BufferedIO_Write(bio, s);
          GWEN_BufferedIO_WriteLine(bio, "\")==0)");
          GWEN_BufferedIO_Write(bio, "      return ");

          GWEN_Buffer_AppendByte(tprefix, toupper(*s));
          GWEN_Buffer_AppendString(tprefix, s+1);
          GWEN_BufferedIO_Write(bio, GWEN_Buffer_GetStart(tprefix));
          GWEN_BufferedIO_WriteLine(bio, ";");

          GWEN_Buffer_Crop(tprefix, 0, vpos);
          GWEN_Buffer_SetPos(tprefix, vpos);
          first=0;
          nn=GWEN_XMLNode_FindNextTag(nn, "value", 0, 0);
        }
      }
      GWEN_BufferedIO_WriteLine(bio, "  }");
      GWEN_BufferedIO_Write(bio, "  return ");
      GWEN_BufferedIO_Write(bio, GWEN_Buffer_GetStart(tprefix));
      GWEN_BufferedIO_WriteLine(bio, "Unknown;");
      GWEN_BufferedIO_WriteLine(bio, "}");
      GWEN_BufferedIO_WriteLine(bio, "");
      GWEN_BufferedIO_WriteLine(bio, "");

      GWEN_BufferedIO_Write(bio, "const char *");
      GWEN_BufferedIO_Write(bio, GWEN_Buffer_GetStart(tprefix));
      GWEN_BufferedIO_Write(bio, "_toString(");
      GWEN_BufferedIO_Write(bio, GWEN_Buffer_GetStart(tid));
      GWEN_BufferedIO_WriteLine(bio, " v) {");
      GWEN_BufferedIO_WriteLine(bio, "  switch(v) {");

      nn=GWEN_XMLNode_FindFirstTag(n, "values", 0, 0);
      if (nn)
        nn=GWEN_XMLNode_FindFirstTag(nn, "value", 0, 0);
      if (nn) {
        uint32_t vpos;

        vpos=GWEN_Buffer_GetPos(tprefix);
        while(nn) {
          GWEN_XMLNODE *nnn;

          nnn=GWEN_XMLNode_GetFirstData(nn);
          if (!nnn) {
            DBG_ERROR(0, "No values in enum description for \"%s\"",
                      GWEN_Buffer_GetStart(tid));
            GWEN_Buffer_free(tid);
            GWEN_Buffer_free(tprefix);
            return -1;
          }
          s=GWEN_XMLNode_GetData(nnn);
          assert(s);
          GWEN_Buffer_AppendByte(tprefix, toupper(*s));
          GWEN_Buffer_AppendString(tprefix, s+1);

          GWEN_BufferedIO_Write(bio, "    case ");
          GWEN_BufferedIO_Write(bio, GWEN_Buffer_GetStart(tprefix));
          GWEN_BufferedIO_WriteLine(bio, ":");
          GWEN_BufferedIO_Write(bio, "      return \"");
          GWEN_BufferedIO_Write(bio, s);
          GWEN_BufferedIO_WriteLine(bio, "\";");
          GWEN_BufferedIO_WriteLine(bio, "");

          GWEN_Buffer_Crop(tprefix, 0, vpos);
          GWEN_Buffer_SetPos(tprefix, vpos);
          nn=GWEN_XMLNode_FindNextTag(nn, "value", 0, 0);
        }
      }
      GWEN_BufferedIO_WriteLine(bio, "    default:");
      GWEN_BufferedIO_WriteLine(bio, "      return \"unknown\";");


      GWEN_BufferedIO_WriteLine(bio, "  } /* switch */");
      GWEN_BufferedIO_WriteLine(bio, "} ");
      GWEN_BufferedIO_WriteLine(bio, "");
      GWEN_BufferedIO_WriteLine(bio, "");

      GWEN_Buffer_Crop(tprefix, 0, ppos);
      GWEN_Buffer_Crop(tid, 0, tpos);
      n=GWEN_XMLNode_FindNextTag(n, "type", "mode", "enum");
    } /* while n */

    GWEN_Buffer_free(tid);
    GWEN_Buffer_free(tprefix);
  } /* if enum types found */

  return 0;
}



int write_code_freeElem_c(ARGUMENTS *args,
                          GWEN_XMLNODE *node,
                          GWEN_BUFFEREDIO *bio){
  const char *typ;
  const char *name;
  int doCopy;
  int takeOver;
  int err;

  if (atoi(get_property(node, "ptr", "0"))==0)
    return 0;

  doCopy=atoi(GWEN_XMLNode_GetProperty(node, "copy", "1"));
  takeOver=atoi(GWEN_XMLNode_GetProperty(node, "takeOver", "0"));

  if (!doCopy && !takeOver)
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
  if (err) { DBG_ERROR_ERR(0, err); return -1;}
  err=GWEN_BufferedIO_Write(bio, name);
  if (err) { DBG_ERROR_ERR(0, err); return -1;}
  err=GWEN_BufferedIO_WriteLine(bio, ")");
  if (err) { DBG_ERROR_ERR(0, err); return -1;}

  if (strcmp(typ, "char")==0) {
    /* we can handle chars */
    err=GWEN_BufferedIO_Write(bio, "    free(st->");
    if (err) { DBG_ERROR_ERR(0, err); return -1;}
    err=GWEN_BufferedIO_Write(bio, name);
    if (err) { DBG_ERROR_ERR(0, err); return -1;}
    err=GWEN_BufferedIO_WriteLine(bio, ");");
    if (err) { DBG_ERROR_ERR(0, err); return -1;}
    return 0;
  }
  else {
    const char *fname;

    fname=get_function_name(node, "free");
    if (fname) {
      err=GWEN_BufferedIO_Write(bio, "    ");
      if (err) { DBG_ERROR_ERR(0, err); return -1;}
      err=GWEN_BufferedIO_Write(bio, fname);
      if (err) { DBG_ERROR_ERR(0, err); return -1;}
      err=GWEN_BufferedIO_Write(bio, "(st->");
      if (err) { DBG_ERROR_ERR(0, err); return -1;}
      err=GWEN_BufferedIO_Write(bio, name);
      if (err) { DBG_ERROR_ERR(0, err); return -1;}
      err=GWEN_BufferedIO_WriteLine(bio, ");");
      if (err) { DBG_ERROR_ERR(0, err); return -1;}
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
      else if (strcasecmp(GWEN_XMLNode_GetData(n), "elem")==0) {
	rv=write_code_freeElem_c(args, n, bio);
      }
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
  int err;

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
    if (err) { DBG_ERROR_ERR(0, err); return -1;}
    err=GWEN_BufferedIO_Write(bio, param);
    if (err) { DBG_ERROR_ERR(0, err); return -1;}
    err=GWEN_BufferedIO_WriteLine(bio, ");");
    if (err) { DBG_ERROR_ERR(0, err); return -1;}
    return 0;
  }
  else {
    const char *fname;

    fname=get_function_name(node, "dup");
    if (!fname) {
      DBG_ERROR(0, "No dup function set for type %s", typ);
      return -1;
    }
    err=GWEN_BufferedIO_Write(bio, fname);
    if (err) { DBG_ERROR_ERR(0, err); return -1;}
    err=GWEN_BufferedIO_Write(bio, "(");
    err=GWEN_BufferedIO_Write(bio, param);
    if (err) { DBG_ERROR_ERR(0, err); return -1;}
    err=GWEN_BufferedIO_WriteLine(bio, ");");
    if (err) { DBG_ERROR_ERR(0, err); return -1;}
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
  const char *mode;
  int isPtr;

  isPtr=atoi(get_property(node, "ptr", "0"));

  name=GWEN_XMLNode_GetProperty(node, "name", 0);
  if (!name) {
    DBG_ERROR(0, "No name for element");
    return -1;
  }

  /* "single" as opposed to "list" or "list2" */
  mode=GWEN_XMLNode_GetProperty(node, "mode", "single");
  if (strcasecmp(mode, "list")==0 ||
      strcasecmp(mode, "list2")==0)
    /* all list modes operate on pointers */
    isPtr=1;

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
      else if (strcasecmp(mode, "list")==0) {
	const char *elemType;
	const char *elemPrefix;
        GWEN_XMLNODE *elemNode;

	/* create list code */
	elemType=GWEN_XMLNode_GetProperty(node, "elemType", 0);
	if (!elemType) {
	  DBG_ERROR(0, "No \"type\" for list type \"%s\"", typ);
          return -1;
	}

	elemNode=get_typedef(node, elemType);
	if (!elemNode) {
	  DBG_ERROR(0, "Undefined type %s", elemType);
          return -1;
	}
	elemPrefix=GWEN_XMLNode_GetProperty(elemNode, "prefix", 0);
	if (!elemPrefix) {
	  DBG_ERROR(0, "No \"prefix\" for type \"%s\" (within %s)",
		    elemType, typ);
	  return -1;
	}

        /* actually generate the code */
	GWEN_BufferedIO_WriteLine(bio, "  if (1) {");
	GWEN_BufferedIO_WriteLine(bio, "    GWEN_DB_NODE *dbT;");

        GWEN_BufferedIO_Write(bio, "    ");
	GWEN_BufferedIO_Write(bio, elemType);
	GWEN_BufferedIO_WriteLine(bio, " *e;");
	GWEN_BufferedIO_WriteLine(bio, "");
	GWEN_BufferedIO_Write(bio,
			      "    dbT=GWEN_DB_GetGroup(db, "
			      "GWEN_PATH_FLAGS_CREATE_GROUP, \"");
	GWEN_BufferedIO_WriteChar(bio, tolower(*name));
	GWEN_BufferedIO_Write(bio, name+1);
	GWEN_BufferedIO_WriteLine(bio, "\");");
	GWEN_BufferedIO_WriteLine(bio, "    assert(dbT);");

	/* e=ElemType_List_First(st->name) */
	GWEN_BufferedIO_Write(bio, "    e=");
	GWEN_BufferedIO_Write(bio, elemPrefix);
	GWEN_BufferedIO_Write(bio, "_List_First(st->");
	GWEN_BufferedIO_WriteChar(bio, tolower(*name));
	GWEN_BufferedIO_Write(bio, name+1);
	GWEN_BufferedIO_WriteLine(bio, ");");

        /* while (e) */
	GWEN_BufferedIO_WriteLine(bio, "    while(e) {");

	/* handle element type */
	GWEN_BufferedIO_Write(bio, "      if (");
	GWEN_BufferedIO_Write(bio, elemPrefix);
	GWEN_BufferedIO_Write(bio, "_toDb(e, ");
	GWEN_BufferedIO_Write(bio,
			      "GWEN_DB_GetGroup(dbT, "
			      "GWEN_PATH_FLAGS_CREATE_GROUP, \"");
	GWEN_BufferedIO_Write(bio, "element");
	GWEN_BufferedIO_WriteLine(bio, "\")))");
	GWEN_BufferedIO_WriteLine(bio, "        return -1;");

	/* e=ElemType_List_Next(e) */
	GWEN_BufferedIO_Write(bio, "      e=");
	GWEN_BufferedIO_Write(bio, elemPrefix);
	GWEN_BufferedIO_WriteLine(bio, "_List_Next(e);");

	GWEN_BufferedIO_WriteLine(bio, "    } /* while */");

	GWEN_BufferedIO_WriteLine(bio, "  } /* if (1) */");

      }
      else if (strcasecmp(mode, "list2")==0) {
	const char *elemType;
	const char *elemPrefix;
        GWEN_XMLNODE *elemNode;

	/* create list2 code */
	elemType=GWEN_XMLNode_GetProperty(node, "elemType", 0);
	if (!elemType) {
	  DBG_ERROR(0, "No \"type\" for list type \"%s\"", typ);
          return -1;
	}

	elemNode=get_typedef(node, elemType);
	if (!elemNode) {
	  DBG_ERROR(0, "Undefined type %s", elemType);
          return -1;
	}
	elemPrefix=GWEN_XMLNode_GetProperty(elemNode, "prefix", 0);
	if (!elemPrefix) {
	  DBG_ERROR(0, "No \"prefix\" for type \"%s\" (within %s)",
		    elemType, typ);
	  return -1;
	}

        /* actually generate the code */
	GWEN_BufferedIO_WriteLine(bio, "  if (1) {");
	GWEN_BufferedIO_WriteLine(bio, "    GWEN_DB_NODE *dbT;");
	GWEN_BufferedIO_Write(bio, "    ");
	GWEN_BufferedIO_Write(bio, elemType);
	GWEN_BufferedIO_WriteLine(bio, "_LIST2_ITERATOR *it;");
	GWEN_BufferedIO_WriteLine(bio, "");
	GWEN_BufferedIO_Write(bio,
			      "    dbT=GWEN_DB_GetGroup(db, "
			      "GWEN_PATH_FLAGS_CREATE_GROUP, \"");
	GWEN_BufferedIO_WriteChar(bio, tolower(*name));
	GWEN_BufferedIO_Write(bio, name+1);
	GWEN_BufferedIO_WriteLine(bio, "\");");
	GWEN_BufferedIO_WriteLine(bio, "    assert(dbT);");

	/* it=ElemType_List2_First(st->name) */
	GWEN_BufferedIO_Write(bio, "    it=");
	GWEN_BufferedIO_Write(bio, elemPrefix);
	GWEN_BufferedIO_Write(bio, "_List2_First(st->");
	GWEN_BufferedIO_WriteChar(bio, tolower(*name));
	GWEN_BufferedIO_Write(bio, name+1);
	GWEN_BufferedIO_WriteLine(bio, ");");

        /* if (it) */
	GWEN_BufferedIO_WriteLine(bio, "    if (it) {");
	GWEN_BufferedIO_Write(bio, "      ");
	GWEN_BufferedIO_Write(bio, elemType);
	GWEN_BufferedIO_WriteLine(bio, " *e;");
	GWEN_BufferedIO_WriteLine(bio, "");

	/* e=ElemType_List2Iterator_Data(it) */
	GWEN_BufferedIO_Write(bio, "        e=");
	GWEN_BufferedIO_Write(bio, elemPrefix);
	GWEN_BufferedIO_WriteLine(bio, "_List2Iterator_Data(it);");
	GWEN_BufferedIO_Write(bio, "        assert(e);");

	/* while (e) */
	GWEN_BufferedIO_WriteLine(bio, "      while(e) {");

	/* handle element type */
	GWEN_BufferedIO_Write(bio, "        if (");
	GWEN_BufferedIO_Write(bio, elemPrefix);
	GWEN_BufferedIO_Write(bio, "_toDb(e, ");
	GWEN_BufferedIO_Write(bio,
			      "GWEN_DB_GetGroup(dbT, "
			      "GWEN_PATH_FLAGS_CREATE_GROUP, \"");
	GWEN_BufferedIO_Write(bio, "element");
	GWEN_BufferedIO_WriteLine(bio, "\")))");
	GWEN_BufferedIO_WriteLine(bio, "          return -1;");

	/* e=ElemType_List2Iterator_Next(it) */
	GWEN_BufferedIO_Write(bio, "        e=");
	GWEN_BufferedIO_Write(bio, elemPrefix);
	GWEN_BufferedIO_WriteLine(bio, "_List2Iterator_Next(it);");

	GWEN_BufferedIO_WriteLine(bio, "      } /* while */");

        /* free iterator */
	GWEN_BufferedIO_Write(bio, "      ");
	GWEN_BufferedIO_Write(bio, elemPrefix);
	GWEN_BufferedIO_Write(bio, "_List2Iterator_free(it);");

	GWEN_BufferedIO_WriteLine(bio, "    } /* if (it) */");

	GWEN_BufferedIO_WriteLine(bio, "  } /* if (1) */");

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
      else if (strcasecmp(typ, "uint32_t")==0)
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
      GWEN_XMLNODE *tnode;
      const char *tmode;

      tnode=get_typedef(node, typ);
      if (!tnode) {
        DBG_ERROR(0, "Undefined type %s", typ);
        return -1;
      }
      tmode=GWEN_XMLNode_GetProperty(tnode, "mode", "single");
      if (strcasecmp(tmode, "enum")==0) {
        GWEN_XMLNODE *tnode;
        const char *tmode;
  
        tnode=get_typedef(node, typ);
        if (!tnode) {
          DBG_ERROR(0, "Undefined type %s", typ);
          return -1;
        }
        tmode=GWEN_XMLNode_GetProperty(tnode, "mode", "single");
        if (strcasecmp(tmode, "enum")==0) {
          GWEN_BUFFER *tprefix;
          const char *s;
      
          tprefix=GWEN_Buffer_new(0, 64, 0, 1);
      
          s=get_struct_property(node, "prefix", 0);
          assert(s);
          GWEN_Buffer_AppendString(tprefix, s);
          GWEN_Buffer_AppendString(tprefix, "_");

          s=GWEN_XMLNode_GetProperty(tnode, "prefix", 0);
          assert(s);
          GWEN_BufferedIO_Write(bio, "  if (GWEN_DB_SetCharValue(db, ");
          GWEN_BufferedIO_Write(bio, "GWEN_DB_FLAGS_OVERWRITE_VARS, \"");
          GWEN_BufferedIO_WriteChar(bio, tolower(*name));
          GWEN_BufferedIO_Write(bio, name+1);
          GWEN_BufferedIO_Write(bio, "\", ");

          GWEN_Buffer_AppendString(tprefix, s);
          GWEN_BufferedIO_Write(bio, GWEN_Buffer_GetStart(tprefix));
          GWEN_BufferedIO_Write(bio, "_toString(st->");
          GWEN_BufferedIO_WriteChar(bio, tolower(*name));
          GWEN_BufferedIO_Write(bio, name+1);
          GWEN_BufferedIO_Write(bio, ")");
          GWEN_BufferedIO_Write(bio, ")");
          GWEN_BufferedIO_WriteLine(bio, ") ");
          GWEN_BufferedIO_WriteLine(bio, "    return -1;");

          GWEN_Buffer_free(tprefix);
          return 0;
        } /* if enum */
        else {
          DBG_ERROR(0, "Bad non-pointer type \"%s\" (not a base type)", typ);
          return -1;
        }

      }
      else {
        DBG_ERROR(0, "Bad non-pointer type \"%s\" (not a base type)", typ);
        return -1;
      }
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
  const char *mode;
  int isPtr;
  const char *defval;
  int isVolatile;

  isVolatile=atoi(GWEN_XMLNode_GetProperty(node, "volatile", "0"));
  if (isVolatile)
    /* don't save volatile data */
    return 0;

  isPtr=atoi(get_property(node, "ptr", "0"));

  name=GWEN_XMLNode_GetProperty(node, "name", 0);
  if (!name) {
    DBG_ERROR(0, "No name for element");
    return -1;
  }

  mode=GWEN_XMLNode_GetProperty(node, "mode", "single");
  if (strcasecmp(mode, "single")!=0)
    /* all list modes operate on pointers */
    isPtr=1;

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
        DBG_ERROR(0, "No fromDb function for type \"%s\"", typ);
        return -1;
      }
    }

  }
  else {
    btype=get_property(node, "basetype", 0);
    if (!btype) {
      if (strcasecmp(typ, "char")==0)
        btype="char";
      else if (strcasecmp(typ, "uint32_t")==0)
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
      GWEN_XMLNODE *tnode;
      const char *tmode;

      tnode=get_typedef(node, typ);
      if (!tnode) {
        DBG_ERROR(0, "Undefined type %s", typ);
        return -1;
      }
      tmode=GWEN_XMLNode_GetProperty(tnode, "mode", "single");
      if (strcasecmp(tmode, "enum")==0) {
        GWEN_BUFFER *tprefix;
        const char *s;
    
        tprefix=GWEN_Buffer_new(0, 64, 0, 1);
    
        s=get_struct_property(node, "prefix", 0);
        assert(s);
        GWEN_Buffer_AppendString(tprefix, s);
        GWEN_Buffer_AppendString(tprefix, "_");

        s=GWEN_XMLNode_GetProperty(tnode, "prefix", 0);
        assert(s);
        GWEN_Buffer_AppendString(tprefix, s);
        GWEN_BufferedIO_Write(bio, GWEN_Buffer_GetStart(tprefix));
        GWEN_BufferedIO_Write(bio, "_fromString(");
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
        GWEN_BufferedIO_Write(bio, ")");
        GWEN_Buffer_free(tprefix);
      } /* if enum */
      else {
        DBG_ERROR(0, "Bad non-pointer type \"%s\" (not a base type)", typ);
        return -1;
      }
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
        const char *mode;

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
        mode=GWEN_XMLNode_GetProperty(n, "mode", "single");

        if (strcasecmp(mode, "single")!=0)
          /* lists always use pointers */
          isPtr=1;

	if (isPtr) {
	  if (strcasecmp(typ, "GWEN_STRINGLIST")==0) {
	    GWEN_BufferedIO_Write(bio, "  st->");
	    GWEN_BufferedIO_WriteChar(bio, tolower(*name));
	    GWEN_BufferedIO_Write(bio, name+1);
	    GWEN_BufferedIO_WriteLine(bio, "=GWEN_StringList_new();");
          }
          else if (strcasecmp(mode, "single")!=0) {
            int initVar;

            initVar=atoi(get_property(n, "init", "0"));
            if (initVar) {
              const char *fname;

              fname=get_function_name(n, "new");
              if (!fname) {
                DBG_ERROR(0, "No new-function set for type %s", typ);
                return -1;
              }
              GWEN_BufferedIO_Write(bio, "  st->");
              GWEN_BufferedIO_WriteChar(bio, tolower(*name));
              GWEN_BufferedIO_Write(bio, name+1);
              GWEN_BufferedIO_Write(bio, "=");
              GWEN_BufferedIO_Write(bio, fname);
              GWEN_BufferedIO_WriteLine(bio, "();");
            } /* if init requested */
          } /* if !single */
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
	else {
          if (setval) {
            /* TODO: check for enum values */
	    GWEN_BufferedIO_Write(bio, "  st->");
	    GWEN_BufferedIO_WriteChar(bio, tolower(*name));
	    GWEN_BufferedIO_Write(bio, name+1);
	    GWEN_BufferedIO_Write(bio, "=");
	    GWEN_BufferedIO_Write(bio, setval);
	    GWEN_BufferedIO_WriteLine(bio, ";");
	  }
	}
      }
      else if (strcasecmp(GWEN_XMLNode_GetData(n), "func")==0) {
      }
    }
    n=GWEN_XMLNode_GetNextTag(n);
  }
  return 0;
}




int write_code_const_c(ARGUMENTS *args,
                       GWEN_XMLNODE *node,
                       GWEN_BUFFEREDIO *bio){
  const char *prefix;
  const char *styp;
  const char *constName;
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
  constName=get_struct_property(node, "constructor-name", 0);

  GWEN_BufferedIO_Write(bio, styp);
  GWEN_BufferedIO_Write(bio, " *");
  GWEN_BufferedIO_Write(bio, prefix);
  if (constName && *constName)
    GWEN_BufferedIO_Write(bio, constName);
  else
    GWEN_BufferedIO_Write(bio, "_new");
  GWEN_BufferedIO_WriteLine(bio, "() {");

  GWEN_BufferedIO_Write(bio, "  ");
  GWEN_BufferedIO_Write(bio, styp);
  GWEN_BufferedIO_WriteLine(bio, " *st;");
  GWEN_BufferedIO_WriteLine(bio, "");

  GWEN_BufferedIO_Write(bio, "  GWEN_NEW_OBJECT(");
  GWEN_BufferedIO_Write(bio, styp);
  GWEN_BufferedIO_WriteLine(bio, ", st)");
  GWEN_BufferedIO_WriteLine(bio, "  st->_usage=1;");

  // add inherit functions
  if (get_struct_property(node, "inherit", 0)) {
    GWEN_BufferedIO_Write(bio, "  GWEN_INHERIT_INIT(");
    GWEN_BufferedIO_Write(bio, styp);
    GWEN_BufferedIO_WriteLine(bio, ", st)");
  }

  // add list functions
  if (get_struct_property(node, "list", 0)) {
    GWEN_BufferedIO_Write(bio, "  GWEN_LIST_INIT(");
    GWEN_BufferedIO_Write(bio, styp);
    GWEN_BufferedIO_WriteLine(bio, ", st)");
  }

  rv=write_code_constrec_c(args, node, bio);
  if (rv)
    return rv;

  GWEN_BufferedIO_WriteLine(bio, "  return st;");
  GWEN_BufferedIO_WriteLine(bio, "}");

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
        const char *mode;
        GWEN_XMLNODE *tnode;
        const char *tmode;
        int isConst;
        int doCopy;

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
        isConst=atoi(get_property(n, "const", "1"));
        doCopy=atoi(get_property(n, "copy", "1"));
        mode=GWEN_XMLNode_GetProperty(n, "mode", "single");

        tnode=get_typedef(node, typ);
        if (tnode)
          tmode=GWEN_XMLNode_GetProperty(tnode, "mode", "single");
        else
          tmode=mode;

        if (isPtr &&
            (/*strcasecmp(mode, "single")==0 ||*/ isConst)) {
          GWEN_BufferedIO_Write(bio, "const ");
        }
        if (strcasecmp(tmode, "enum")!=0)
          GWEN_BufferedIO_Write(bio, typ);
        else {
          GWEN_BUFFER *tid;
          const char *s;

          tid=GWEN_Buffer_new(0, 64, 0, 1);
          s=get_struct_property(node, "id", 0);
          assert(s);
          GWEN_Buffer_AppendString(tid, s);
          GWEN_Buffer_AppendString(tid, "_");
          GWEN_Buffer_AppendString(tid, typ);
          GWEN_BufferedIO_Write(bio, GWEN_Buffer_GetStart(tid));
          GWEN_Buffer_free(tid);
        }
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
        GWEN_BufferedIO_WriteLine(bio, "");
        GWEN_BufferedIO_WriteLine(bio, "");

        /* write setter */
        GWEN_BufferedIO_Write(bio, "void ");
        GWEN_BufferedIO_Write(bio, prefix);
        GWEN_BufferedIO_Write(bio, "_Set");
        GWEN_BufferedIO_WriteChar(bio, toupper(*name));;
        GWEN_BufferedIO_Write(bio, name+1);
        GWEN_BufferedIO_Write(bio, "(");
        GWEN_BufferedIO_Write(bio, styp);
        GWEN_BufferedIO_Write(bio, " *st, ");
        if (isPtr && isConst) {
          GWEN_BufferedIO_Write(bio, "const ");
        }
        if (strcasecmp(tmode, "enum")!=0)
          GWEN_BufferedIO_Write(bio, typ);
        else {
          GWEN_BUFFER *tid;
          const char *s;

          tid=GWEN_Buffer_new(0, 64, 0, 1);
          s=get_struct_property(node, "id", 0);
          assert(s);
          GWEN_Buffer_AppendString(tid, s);
          GWEN_Buffer_AppendString(tid, "_");
          GWEN_Buffer_AppendString(tid, typ);
          GWEN_BufferedIO_Write(bio, GWEN_Buffer_GetStart(tid));
          GWEN_Buffer_free(tid);
        }
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
          if (strcasecmp(mode, "single")==0) {
            if (strcasecmp(typ, "char")==0)
              GWEN_BufferedIO_WriteLine(bio, "  if (d && *d)");
            else
              GWEN_BufferedIO_WriteLine(bio, "  if (d)");

            GWEN_BufferedIO_Write(bio, "    st->");
            GWEN_BufferedIO_WriteChar(bio, tolower(*name));
            GWEN_BufferedIO_Write(bio, name+1);
            GWEN_BufferedIO_Write(bio, "=");

            if (doCopy) {
              rv=write_code_dupArg_c(args, n, bio, "d");
              if (rv)
                return rv;
            }
            else {
              GWEN_BufferedIO_WriteLine(bio, "d;");
            }
          }
          else {
            if (doCopy) {
              rv=write_code_dupList_c(args, n, bio, "d");
              if (rv)
                return rv;
            }
            else {
              if (isConst) {
                DBG_ERROR(0, "Properties: CONST but not COPY");
                return -1;
              }
              GWEN_BufferedIO_Write(bio, "    st->");
              GWEN_BufferedIO_WriteChar(bio, tolower(*name));
              GWEN_BufferedIO_Write(bio, name+1);
              GWEN_BufferedIO_WriteLine(bio, "=d;");
            }
          }
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
        GWEN_BufferedIO_WriteLine(bio, "");
        GWEN_BufferedIO_WriteLine(bio, "");

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
          GWEN_BufferedIO_WriteLine(bio, "");
          GWEN_BufferedIO_WriteLine(bio, "");

          /* remove */
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
          GWEN_BufferedIO_WriteLine(bio, "");
          GWEN_BufferedIO_WriteLine(bio, "");

          /* clear */
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
          GWEN_BufferedIO_WriteLine(bio, "");
          GWEN_BufferedIO_WriteLine(bio, "");

          /* has */
          GWEN_BufferedIO_Write(bio, "int ");
          GWEN_BufferedIO_Write(bio, prefix);
          GWEN_BufferedIO_Write(bio, "_Has");
          GWEN_BufferedIO_WriteChar(bio, toupper(*name));
          GWEN_BufferedIO_Write(bio, name+1);
          GWEN_BufferedIO_Write(bio, "(const ");
          GWEN_BufferedIO_Write(bio, styp);
          GWEN_BufferedIO_WriteLine(bio, " *st, const char *d) {");
          GWEN_BufferedIO_Write(bio, "  return GWEN_StringList_HasString(st->");
          GWEN_BufferedIO_WriteChar(bio, tolower(*name));
          GWEN_BufferedIO_Write(bio, name+1);
          GWEN_BufferedIO_WriteLine(bio, ", d);");
          GWEN_BufferedIO_WriteLine(bio, "}");
          GWEN_BufferedIO_WriteLine(bio, "");
          GWEN_BufferedIO_WriteLine(bio, "");
        }
      }
      else if (strcasecmp(GWEN_XMLNode_GetData(n), "func")==0) {
        const char *typ;
        const char *name;
        const char *rettype;
        const char *defret;
        GWEN_XMLNODE *anode;
        int isPtr;
        int isVoid;
        int idx;

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

        rettype=GWEN_XMLNode_GetProperty(n, "return", 0);
        if (!rettype) {
          DBG_ERROR(0, "No return type for function");
          return -1;
        }

        isPtr=atoi(get_property(n, "ptr", "0"));
        isVoid=(!isPtr && strcasecmp(rettype, "void")==0);

        defret=GWEN_XMLNode_GetProperty(n, "default", 0);
        if (!defret && !isVoid) {
          DBG_ERROR(0, "No default return value for function %s", name);
          return -1;
        }

        /* getter */
        GWEN_BufferedIO_Write(bio, styp);
        GWEN_BufferedIO_Write(bio, "_");
        GWEN_BufferedIO_Write(bio, typ);
        GWEN_BufferedIO_Write(bio, " ");
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
        GWEN_BufferedIO_WriteLine(bio, "");
        GWEN_BufferedIO_WriteLine(bio, "");

        /* setter */
        GWEN_BufferedIO_Write(bio, "void ");
        GWEN_BufferedIO_Write(bio, prefix);
        GWEN_BufferedIO_Write(bio, "_Set");
        GWEN_BufferedIO_WriteChar(bio, toupper(*name));
        GWEN_BufferedIO_Write(bio, name+1);
        GWEN_BufferedIO_Write(bio, "(");
        GWEN_BufferedIO_Write(bio, styp);
        GWEN_BufferedIO_Write(bio, " *st, ");
        GWEN_BufferedIO_Write(bio, styp);
        GWEN_BufferedIO_Write(bio, "_");
        GWEN_BufferedIO_Write(bio, typ);
        GWEN_BufferedIO_WriteLine(bio, " d) {");
        GWEN_BufferedIO_WriteLine(bio, "  assert(st);");
        GWEN_BufferedIO_Write(bio, "  st->");
        GWEN_BufferedIO_WriteChar(bio, tolower(*name));
        GWEN_BufferedIO_Write(bio, name+1);
        GWEN_BufferedIO_WriteLine(bio, "=d;");
        GWEN_BufferedIO_WriteLine(bio, "}");
        GWEN_BufferedIO_WriteLine(bio, "");
        GWEN_BufferedIO_WriteLine(bio, "");

        /* function call */
        GWEN_BufferedIO_Write(bio, rettype);
        if (isPtr)
          GWEN_BufferedIO_Write(bio, "*");
        GWEN_BufferedIO_Write(bio, " ");
        GWEN_BufferedIO_Write(bio, prefix);
        GWEN_BufferedIO_Write(bio, "_");
        GWEN_BufferedIO_WriteChar(bio, toupper(*name));
        GWEN_BufferedIO_Write(bio, name+1);
        GWEN_BufferedIO_Write(bio, "(");

        GWEN_BufferedIO_Write(bio, styp);
        GWEN_BufferedIO_Write(bio, " *st");

        anode=GWEN_XMLNode_FindFirstTag(n, "arg", 0, 0);
        idx=0;
        while(anode) {
          const char *aname;
          const char *atype;
          int aisPtr;
    
          GWEN_BufferedIO_Write(bio, ", ");

          aisPtr=atoi(GWEN_XMLNode_GetProperty(anode, "ptr", "0"));
          aname=GWEN_XMLNode_GetProperty(anode, "name", 0);
          if (!aname || !*aname) {
            DBG_ERROR(0, "No name for argument %d in function %s", idx, name);
            return -1;
          }
          atype=GWEN_XMLNode_GetProperty(anode, "type", 0);
          if (!atype || !*atype) {
            DBG_ERROR(0, "No type for argument %d in function %s", idx, name);
            return -1;
          }
    
          GWEN_BufferedIO_Write(bio, atype);
          if (aisPtr)
            GWEN_BufferedIO_Write(bio, "*");
          GWEN_BufferedIO_Write(bio, " ");
          GWEN_BufferedIO_Write(bio, aname);
    
          idx++;
          anode=GWEN_XMLNode_FindNextTag(anode, "arg", 0, 0);
        }

        GWEN_BufferedIO_WriteLine(bio, ") {");
        GWEN_BufferedIO_WriteLine(bio, "  assert(st);");
        GWEN_BufferedIO_Write(bio, "  if (st->");
        GWEN_BufferedIO_WriteChar(bio, tolower(*name));
        GWEN_BufferedIO_Write(bio, name+1);
        GWEN_BufferedIO_WriteLine(bio, ")");
        GWEN_BufferedIO_Write(bio, "    ");
        if (!isVoid)
          GWEN_BufferedIO_Write(bio, "return ");
        GWEN_BufferedIO_Write(bio, "st->");
        GWEN_BufferedIO_WriteChar(bio, tolower(*name));
        GWEN_BufferedIO_Write(bio, name+1);
        GWEN_BufferedIO_Write(bio, "(st");

        anode=GWEN_XMLNode_FindFirstTag(n, "arg", 0, 0);
        while(anode) {
          const char *aname;

          GWEN_BufferedIO_Write(bio, ", ");
          aname=GWEN_XMLNode_GetProperty(anode, "name", 0);
          GWEN_BufferedIO_Write(bio, aname);
          anode=GWEN_XMLNode_FindNextTag(anode, "arg", 0, 0);
        }
        GWEN_BufferedIO_WriteLine(bio, ");");
        if (!isVoid) {
          GWEN_BufferedIO_Write(bio, "return ");
          GWEN_BufferedIO_Write(bio, defret);
          GWEN_BufferedIO_WriteLine(bio, ";");
        }
        GWEN_BufferedIO_WriteLine(bio, "}");
        GWEN_BufferedIO_WriteLine(bio, "");
        GWEN_BufferedIO_WriteLine(bio, "");
      }
    }
    n=GWEN_XMLNode_GetNextTag(n);
  }
  return 0;
}






int write_code_dupList_c(ARGUMENTS *args, GWEN_XMLNODE *n,
                         GWEN_BUFFEREDIO *bio,
                         const char *listName) {
  int isPtr;
  const char *typ;
  const char *name;
  const char *mode;

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
  mode=GWEN_XMLNode_GetProperty(n, "mode", "single");
  if (strcasecmp(mode, "single")!=0)
    /* lists are always pointers */
    isPtr=1;
  
  if (isPtr) {
    if (strcasecmp(mode, "list")==0) {
      const char *prefix;
      const char *elemType;
      const char *elemPrefix;
      GWEN_XMLNODE *elemNode;
  
      prefix=get_struct_property(n, "prefix", 0);
      assert(prefix);
  
      /* create list code */
      elemType=GWEN_XMLNode_GetProperty(n, "elemType", 0);
      if (!elemType) {
        DBG_ERROR(0, "No \"type\" for list type \"%s\"", typ);
        return -1;
      }
  
      elemNode=get_typedef(n, elemType);
      if (!elemNode) {
        DBG_ERROR(0, "Undefined type %s", elemType);
        return -1;
      }
      elemPrefix=GWEN_XMLNode_GetProperty(elemNode, "prefix", 0);
      if (!elemPrefix) {
        DBG_ERROR(0, "No \"prefix\" for type \"%s\" (within %s)",
                  elemType, typ);
        return -1;
      }
  
      /* actually generate the code */
      GWEN_BufferedIO_Write(bio, "  if (");
      GWEN_BufferedIO_Write(bio, listName);
      GWEN_BufferedIO_WriteLine(bio, ") {");
  
      /* ELEMTYPE *e; */
      GWEN_BufferedIO_Write(bio, "    ");
      GWEN_BufferedIO_Write(bio, elemType);
      GWEN_BufferedIO_WriteLine(bio, " *e;");
      GWEN_BufferedIO_WriteLine(bio, "");

      /* st->LIST=LIST_new() */
      GWEN_BufferedIO_Write(bio, "  ");
      GWEN_BufferedIO_Write(bio, "st->");
      GWEN_BufferedIO_WriteChar(bio, tolower(*name));
      GWEN_BufferedIO_Write(bio, name+1);
      GWEN_BufferedIO_Write(bio, "=");
      GWEN_BufferedIO_Write(bio, elemPrefix);
      GWEN_BufferedIO_WriteLine(bio, "_List_new();");


      /* e=ElemType_List_First */
      GWEN_BufferedIO_Write(bio, "    e=");
      GWEN_BufferedIO_Write(bio, elemPrefix);
      GWEN_BufferedIO_Write(bio, "_List_First(");
      GWEN_BufferedIO_Write(bio, listName);
      GWEN_BufferedIO_WriteLine(bio, ");");
  
      /* while (e) ; */
      GWEN_BufferedIO_WriteLine(bio, "    while(e) {");
  
      /* ELEMTYPE *ne; */
      GWEN_BufferedIO_Write(bio, "      ");
      GWEN_BufferedIO_Write(bio, elemType);
      GWEN_BufferedIO_WriteLine(bio, " *ne;");
      GWEN_BufferedIO_WriteLine(bio, "");
  
      /* ne=ElemType_dup; assert(ne); */
      GWEN_BufferedIO_Write(bio, "      ne=");
      GWEN_BufferedIO_Write(bio, elemPrefix);
      GWEN_BufferedIO_WriteLine(bio, "_dup(e);");
      GWEN_BufferedIO_WriteLine(bio, "      assert(ne);");
  
      /* ElemType_List_Add(ne, st->NAME); */
      GWEN_BufferedIO_Write(bio, "      ");
      GWEN_BufferedIO_Write(bio, elemPrefix);
      GWEN_BufferedIO_Write(bio, "_List_Add(ne, st->");
      GWEN_BufferedIO_WriteChar(bio, tolower(*name));
      GWEN_BufferedIO_Write(bio, name+1);
      GWEN_BufferedIO_WriteLine(bio, ");");
  
      /* e=ElemType_List_Next */
      GWEN_BufferedIO_Write(bio, "      e=");
      GWEN_BufferedIO_Write(bio, elemPrefix);
      GWEN_BufferedIO_WriteLine(bio, "_List_Next(e);");
  
      GWEN_BufferedIO_WriteLine(bio, "    } /* while (e) */");
  
      GWEN_BufferedIO_WriteLine(bio, "  } /* if LIST */");
    }
    else if (strcasecmp(mode, "list2")==0) {
      const char *prefix;
      const char *elemType;
      const char *elemPrefix;
      GWEN_XMLNODE *elemNode;
  
      prefix=get_struct_property(n, "prefix", 0);
      assert(prefix);
  
      /* create list code */
      elemType=GWEN_XMLNode_GetProperty(n, "elemType", 0);
      if (!elemType) {
        DBG_ERROR(0, "No \"type\" for list type \"%s\"", typ);
        return -1;
      }
  
      elemNode=get_typedef(n, elemType);
      if (!elemNode) {
        DBG_ERROR(0, "Undefined type %s", elemType);
        return -1;
      }
      elemPrefix=GWEN_XMLNode_GetProperty(elemNode, "prefix", 0);
      if (!elemPrefix) {
        DBG_ERROR(0, "No \"prefix\" for type \"%s\" (within %s)",
                  elemType, typ);
        return -1;
      }
  
      /* actually generate the code */
      GWEN_BufferedIO_Write(bio, "  st->");
      GWEN_BufferedIO_WriteChar(bio, tolower(*name));
      GWEN_BufferedIO_Write(bio, name+1);
      GWEN_BufferedIO_Write(bio, "=");
      GWEN_BufferedIO_Write(bio, elemPrefix);
      GWEN_BufferedIO_WriteLine(bio, "_List2_new();");
  
      GWEN_BufferedIO_Write(bio, "  if (");
      GWEN_BufferedIO_Write(bio, listName);
      GWEN_BufferedIO_WriteLine(bio, ") {");
  
      GWEN_BufferedIO_Write(bio, "    ");
      GWEN_BufferedIO_Write(bio, elemType);
      GWEN_BufferedIO_WriteLine(bio, "_LIST2_ITERATOR *it;");
      GWEN_BufferedIO_WriteLine(bio, "");
  
      /* it=ElemType_List2_First */
      GWEN_BufferedIO_Write(bio, "    it=");
      GWEN_BufferedIO_Write(bio, elemPrefix);
      GWEN_BufferedIO_Write(bio, "_List2_First(");
      GWEN_BufferedIO_Write(bio, listName);
      GWEN_BufferedIO_WriteLine(bio, ");");
  
      /* if (it) */
      GWEN_BufferedIO_WriteLine(bio, "    if (it) {");
  
      /* ELEMTYPE *e; */
      GWEN_BufferedIO_Write(bio, "      ");
      GWEN_BufferedIO_Write(bio, elemType);
      GWEN_BufferedIO_WriteLine(bio, " *e;");
      GWEN_BufferedIO_WriteLine(bio, "");
  
      /* e=ElemType_List2Iterator_Data */
      GWEN_BufferedIO_Write(bio, "      e=");
      GWEN_BufferedIO_Write(bio, elemPrefix);
      GWEN_BufferedIO_WriteLine(bio, "_List2Iterator_Data(it);");
      GWEN_BufferedIO_WriteLine(bio, "      assert(e);");
  
      /* while (e) ; */
      GWEN_BufferedIO_WriteLine(bio, "      while(e) {");
  
      /* ELEMTYPE *ne; */
      GWEN_BufferedIO_Write(bio, "        ");
      GWEN_BufferedIO_Write(bio, elemType);
      GWEN_BufferedIO_WriteLine(bio, " *ne;");
      GWEN_BufferedIO_WriteLine(bio, "");
  
      /* ne=ElemType_dup; assert(ne); */
      GWEN_BufferedIO_Write(bio, "        ne=");
      GWEN_BufferedIO_Write(bio, elemPrefix);
      GWEN_BufferedIO_WriteLine(bio, "_dup(e);");
      GWEN_BufferedIO_WriteLine(bio, "        assert(ne);");
  
      /* ElemType_List2_PushBack(st->NAME, ne); */
      GWEN_BufferedIO_Write(bio, "        ");
      GWEN_BufferedIO_Write(bio, elemPrefix);
      GWEN_BufferedIO_Write(bio, "_List2_PushBack(st->");
      GWEN_BufferedIO_WriteChar(bio, tolower(*name));
      GWEN_BufferedIO_Write(bio, name+1);
      GWEN_BufferedIO_WriteLine(bio, ", ne);");
  
      /* e=ElemType_List2Iterator_Next */
      GWEN_BufferedIO_Write(bio, "        e=");
      GWEN_BufferedIO_Write(bio, elemPrefix);
      GWEN_BufferedIO_WriteLine(bio, "_List2Iterator_Next(it);");
  
      GWEN_BufferedIO_WriteLine(bio, "      } /* while (e) */");
  
      /* ElemType_List2Iterator_free */
      GWEN_BufferedIO_Write(bio, "        ");
      GWEN_BufferedIO_Write(bio, elemPrefix);
      GWEN_BufferedIO_WriteLine(bio, "_List2Iterator_free(it);");
  
      GWEN_BufferedIO_WriteLine(bio, "    } /* if (it) */");
  
      GWEN_BufferedIO_WriteLine(bio, "  } /* LIST */");
    }
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
        const char *mode;
        int doCopy;

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
        doCopy=atoi(get_property(n, "copy", "1"));
        mode=GWEN_XMLNode_GetProperty(n, "mode", "single");
        if (strcasecmp(mode, "single")!=0)
          /* lists are always pointers */
          isPtr=1;

        if (isPtr) {
          GWEN_BUFFER *pbuf;
          const char *fname;

          fname=get_function_name(n, "dup");

          pbuf=GWEN_Buffer_new(0, 256, 0, 1);
          GWEN_Buffer_AppendString(pbuf, "d->");
          GWEN_Buffer_AppendByte(pbuf, tolower(*name));
          GWEN_Buffer_AppendString(pbuf, name+1);

          if (strcasecmp(mode, "single")!=0 && !fname) {
            rv=write_code_dupList_c(args, n, bio, GWEN_Buffer_GetStart(pbuf));
            GWEN_Buffer_free(pbuf);
            if (rv)
              return rv;
          }
          else {
            /* copy argument if any */
            GWEN_BufferedIO_Write(bio, "  if (d->");
            GWEN_BufferedIO_WriteChar(bio, tolower(*name));
            GWEN_BufferedIO_Write(bio, name+1);
            GWEN_BufferedIO_WriteLine(bio, ")");

            GWEN_BufferedIO_Write(bio, "    st->");
            GWEN_BufferedIO_WriteChar(bio, tolower(*name));
            GWEN_BufferedIO_Write(bio, name+1);
            GWEN_BufferedIO_Write(bio, "=");
            if (doCopy) {
              rv=write_code_dupArg_c(args, n, bio, GWEN_Buffer_GetStart(pbuf));
              GWEN_Buffer_free(pbuf);
              if (rv)
                return rv;
            }
            else {
              GWEN_BufferedIO_WriteLine(bio, GWEN_Buffer_GetStart(pbuf));
              GWEN_Buffer_free(pbuf);
              GWEN_BufferedIO_Write(bio, ";");
            }
          }
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
  const char *dupAcc;
  const char *dupName;

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
  dupAcc=get_struct_property(node, "dup-access",
                             get_struct_property(node, "access", 0));
  dupName=get_struct_property(node, "dup-name", 0);

  if (dupAcc && strcasecmp(dupAcc, "none")!=0) {
    GWEN_BufferedIO_Write(bio, styp);
    GWEN_BufferedIO_Write(bio, " *");
    GWEN_BufferedIO_Write(bio, prefix);
    if (dupName)
      GWEN_BufferedIO_Write(bio, dupName);
    else
      GWEN_BufferedIO_Write(bio, "_dup");
    GWEN_BufferedIO_Write(bio, "(const ");
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
  }
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
        int isVolatile;
      
        isVolatile=atoi(GWEN_XMLNode_GetProperty(n, "volatile", "0"));
        if (isVolatile==0) {
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
	  if (rv) {
	    DBG_ERROR(0, "Error in toDb function");
	    return rv;
	  }
	}
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
  int isVolatile;

  isVolatile=atoi(GWEN_XMLNode_GetProperty(node, "volatile", "0"));
  if (isVolatile)
    /* don't save volatile data */
    return 0;

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
        int isVolatile;
      
        isVolatile=atoi(GWEN_XMLNode_GetProperty(n, "volatile", "0"));
        if (isVolatile==0) {
	  int isPtr;
	  const char *typ;
	  const char *name;
	  const char *mode;

	  name=GWEN_XMLNode_GetProperty(n, "name", 0);
	  if (!name) {
	    DBG_ERROR(0, "No name for element");
	    return -1;
	  }
  
	  mode=GWEN_XMLNode_GetProperty(n, "mode", "single");
  
	  typ=GWEN_XMLNode_GetProperty(n, "type", 0);
	  if (!typ) {
	    DBG_ERROR(0, "No type for element");
	    return -1;
	  }
  
	  if (strcasecmp(mode, "list")==0) {
	    const char *prefix;
	    const char *elemType;
	    const char *elemPrefix;
	    GWEN_XMLNODE *elemNode;
  
	    prefix=get_struct_property(node, "prefix", 0);
	    assert(prefix);
    
	    /* create list code */
	    elemType=GWEN_XMLNode_GetProperty(n, "elemType", 0);
	    if (!elemType) {
	      DBG_ERROR(0, "No \"type\" for list type \"%s\"", typ);
	      return -1;
	    }
    
	    elemNode=get_typedef(n, elemType);
	    if (!elemNode) {
	      DBG_ERROR(0, "Undefined type %s", elemType);
	      return -1;
	    }
	    elemPrefix=GWEN_XMLNode_GetProperty(elemNode, "prefix", 0);
	    if (!elemPrefix) {
	      DBG_ERROR(0, "No \"prefix\" for type \"%s\" (within %s)",
			elemType, typ);
	      return -1;
	    }
    
	    /* actually generate the code */
	    GWEN_BufferedIO_Write(bio, "  st->");
	    GWEN_BufferedIO_WriteChar(bio, tolower(*name));
	    GWEN_BufferedIO_Write(bio, name+1);
	    GWEN_BufferedIO_Write(bio, "=");
	    GWEN_BufferedIO_Write(bio, elemPrefix);
	    GWEN_BufferedIO_WriteLine(bio, "_List_new();");
  
            GWEN_BufferedIO_WriteLine(bio,
                                      "  if (1) {/* just for local vars */");
	    GWEN_BufferedIO_WriteLine(bio, "    GWEN_DB_NODE *dbT;");
	    GWEN_BufferedIO_Write(bio, "    ");
	    GWEN_BufferedIO_Write(bio, elemType);
	    GWEN_BufferedIO_WriteLine(bio, " *e;");
	    GWEN_BufferedIO_WriteLine(bio, "");
	    GWEN_BufferedIO_Write(bio,
				  "    dbT=GWEN_DB_GetGroup(db, "
				  "GWEN_PATH_FLAGS_NAMEMUSTEXIST, \"");
	    GWEN_BufferedIO_WriteChar(bio, tolower(*name));
	    GWEN_BufferedIO_Write(bio, name+1);
	    GWEN_BufferedIO_WriteLine(bio, "\");");
	    GWEN_BufferedIO_WriteLine(bio, "    if (dbT) {");
	    GWEN_BufferedIO_WriteLine(bio, "      GWEN_DB_NODE *dbT2;");
	    GWEN_BufferedIO_WriteLine(bio, "");
    
	    GWEN_BufferedIO_Write(bio,
				  "      dbT2=GWEN_DB_FindFirstGroup(dbT, \"");
	    GWEN_BufferedIO_Write(bio, "element");
	    GWEN_BufferedIO_WriteLine(bio, "\");");
    
	    /* while (e) */
	    GWEN_BufferedIO_WriteLine(bio, "      while(dbT2) {");
    
	    /* e=ElemType_fromDb(e) */
	    GWEN_BufferedIO_Write(bio, "        e=");
	    GWEN_BufferedIO_Write(bio, elemPrefix);
	    GWEN_BufferedIO_WriteLine(bio, "_fromDb(dbT2);");
    
	    /* if (!e) */
	    GWEN_BufferedIO_WriteLine(bio, "        if (!e) {");
	    GWEN_BufferedIO_Write(bio, "          "
				  "DBG_ERROR(0, \"Bad element for type \\\"");
	    GWEN_BufferedIO_Write(bio, elemType);
	    GWEN_BufferedIO_WriteLine(bio, "\\\"\");");
	    GWEN_BufferedIO_WriteLine(bio, "          "
				      "if (GWEN_Logger_GetLevel(0)>="
				      "GWEN_LoggerLevel_Debug)");
	    GWEN_BufferedIO_WriteLine(bio, "            "
				      "GWEN_DB_Dump(dbT2, stderr, 2);");
	    GWEN_BufferedIO_Write(bio, "          ");
	    GWEN_BufferedIO_Write(bio, prefix);
	    GWEN_BufferedIO_WriteLine(bio, "_free(st);");
	    GWEN_BufferedIO_WriteLine(bio, "          return 0;");
	    GWEN_BufferedIO_WriteLine(bio, "        }");
    
	    /* ElemType_List_Add(e, st->NAME); */
	    GWEN_BufferedIO_Write(bio, "        ");
	    GWEN_BufferedIO_Write(bio, elemPrefix);
	    GWEN_BufferedIO_Write(bio, "_List_Add(e, st->");
	    GWEN_BufferedIO_WriteChar(bio, tolower(*name));
	    GWEN_BufferedIO_Write(bio, name+1);
	    GWEN_BufferedIO_Write(bio, ");");
    
	    GWEN_BufferedIO_Write(bio,
				  "    dbT2=GWEN_DB_FindNextGroup(dbT2, \"");
	    GWEN_BufferedIO_Write(bio, "element");
	    GWEN_BufferedIO_WriteLine(bio, "\");");
    
	    GWEN_BufferedIO_WriteLine(bio, "      } /* while */");
    
	    GWEN_BufferedIO_WriteLine(bio, "    } /* if (dbT) */");
    
	    GWEN_BufferedIO_WriteLine(bio, "  } /* if (1) */");
	  }
	  else if (strcasecmp(mode, "list2")==0) {
	    const char *prefix;
	    const char *elemType;
	    const char *elemPrefix;
	    GWEN_XMLNODE *elemNode;
  
	    prefix=get_struct_property(node, "prefix", 0);
	    assert(prefix);
    
	    /* create list code */
	    elemType=GWEN_XMLNode_GetProperty(n, "elemType", 0);
	    if (!elemType) {
	      DBG_ERROR(0, "No \"type\" for list type \"%s\"", typ);
	      return -1;
	    }
    
	    elemNode=get_typedef(node, elemType);
	    if (!elemNode) {
	      DBG_ERROR(0, "Undefined type %s", elemType);
	      return -1;
	    }
	    elemPrefix=GWEN_XMLNode_GetProperty(elemNode, "prefix", 0);
	    if (!elemPrefix) {
	      DBG_ERROR(0, "No \"prefix\" for type \"%s\" (within %s)",
			elemType, typ);
	      return -1;
	    }
    
	    /* actually generate the code */
	    GWEN_BufferedIO_Write(bio, "  st->");
	    GWEN_BufferedIO_WriteChar(bio, tolower(*name));
	    GWEN_BufferedIO_Write(bio, name+1);
	    GWEN_BufferedIO_Write(bio, "=");
	    GWEN_BufferedIO_Write(bio, elemPrefix);
	    GWEN_BufferedIO_WriteLine(bio, "_List2_new();");
  
	    GWEN_BufferedIO_WriteLine(bio, "  if (1) {");
	    GWEN_BufferedIO_WriteLine(bio, "    GWEN_DB_NODE *dbT;");
	    GWEN_BufferedIO_Write(bio, "    ");
	    GWEN_BufferedIO_Write(bio, elemType);
	    GWEN_BufferedIO_WriteLine(bio, " *e;");
	    GWEN_BufferedIO_WriteLine(bio, "");
	    GWEN_BufferedIO_Write(bio,
				  "    dbT=GWEN_DB_GetGroup(db, "
				  "GWEN_PATH_FLAGS_NAMEMUSTEXIST, \"");
	    GWEN_BufferedIO_WriteChar(bio, tolower(*name));
	    GWEN_BufferedIO_Write(bio, name+1);
	    GWEN_BufferedIO_WriteLine(bio, "\");");
	    GWEN_BufferedIO_WriteLine(bio, "    if (dbT) {");
	    GWEN_BufferedIO_WriteLine(bio, "      GWEN_DB_NODE *dbT2;");
	    GWEN_BufferedIO_WriteLine(bio, "");
    
	    GWEN_BufferedIO_Write(bio,
				  "    dbT2=GWEN_DB_FindFirstGroup(dbT, \"");
	    GWEN_BufferedIO_Write(bio, "element");
	    GWEN_BufferedIO_WriteLine(bio, "\");");
    
	    /* while (e) */
	    GWEN_BufferedIO_WriteLine(bio, "      while(dbT2) {");
    
	    /* e=ElemType_fromDb(e) */
	    GWEN_BufferedIO_Write(bio, "        e=");
	    GWEN_BufferedIO_Write(bio, elemPrefix);
	    GWEN_BufferedIO_WriteLine(bio, "_fromDb(dbT2);");
    
	    /* if (!e) */
	    GWEN_BufferedIO_WriteLine(bio, "        if (!e) {");
	    GWEN_BufferedIO_Write(bio, "          "
				  "DBG_ERROR(0, \"Bad element for type \\\"");
	    GWEN_BufferedIO_Write(bio, elemType);
	    GWEN_BufferedIO_WriteLine(bio, "\\\"\");");
	    GWEN_BufferedIO_WriteLine(bio, "          "
				      "if (GWEN_Logger_GetLevel(0)>="
				      "GWEN_LoggerLevel_Debug)");
	    GWEN_BufferedIO_WriteLine(bio, "            "
				      "GWEN_DB_Dump(dbT2, stderr, 2);");
	    GWEN_BufferedIO_Write(bio, "          ");
	    GWEN_BufferedIO_Write(bio, prefix);
	    GWEN_BufferedIO_WriteLine(bio, "_free(st);");
	    GWEN_BufferedIO_WriteLine(bio, "          return 0;");
	    GWEN_BufferedIO_WriteLine(bio, "        } /* if !e */");
    
	    /* ElemType_List_Add(e, st->NAME); */
	    GWEN_BufferedIO_Write(bio, "        ");
	    GWEN_BufferedIO_Write(bio, elemPrefix);
	    GWEN_BufferedIO_Write(bio, "_List2_PushBack(st->");
	    GWEN_BufferedIO_WriteChar(bio, tolower(*name));
	    GWEN_BufferedIO_Write(bio, name+1);
	    GWEN_BufferedIO_WriteLine(bio, ", e);");
  
	    GWEN_BufferedIO_Write(bio,"        "
				  "dbT2=GWEN_DB_FindNextGroup(dbT2, \"");
	    GWEN_BufferedIO_Write(bio, "element");
	    GWEN_BufferedIO_WriteLine(bio, "\");");
    
	    GWEN_BufferedIO_WriteLine(bio, "      } /* while */");
    
	    GWEN_BufferedIO_WriteLine(bio, "    } /* if (dbT) */");
    
	    GWEN_BufferedIO_WriteLine(bio, "  } /* if (1) */");
	  }
	  else if (strcasecmp(typ, "GWEN_STRINGLIST")==0) {
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
              if (strcasecmp(typ, "char")!=0) {
                GWEN_BufferedIO_WriteLine(bio,
                                          "  if (1) { /* for local vars */");
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
	    }
            if (isPtr && strcasecmp(typ, "char")!=0) {
              GWEN_BufferedIO_WriteLine(bio, " {");
              rv=write_code_freeElem_c(args, n, bio);
              if (rv)
                return rv;
              GWEN_BufferedIO_Write(bio, "  st->");
	      GWEN_BufferedIO_Write(bio, name);
	      GWEN_BufferedIO_Write(bio, "=");
	      rv=write_code_fromdbArg_c(args, n, bio);
	      if (rv)
		return rv;
	      GWEN_BufferedIO_WriteLine(bio, ";");
              GWEN_BufferedIO_WriteLine(bio, "}");
	    }
	    else {
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
	    }
  
	    if (isPtr && strcasecmp(typ, "char")!=0) {
	      GWEN_BufferedIO_WriteLine(bio, "  }");
	    }
	  }
	}
      }
    }
    n=GWEN_XMLNode_GetNextTag(n);
  }
  return 0;
}



int write_code_readdb_c(ARGUMENTS *args, GWEN_XMLNODE *node,
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
  GWEN_BufferedIO_Write(bio, "_ReadDb(");
  GWEN_BufferedIO_Write(bio, styp);
  GWEN_BufferedIO_WriteLine(bio, " *st, GWEN_DB_NODE *db) {");

  GWEN_BufferedIO_WriteLine(bio, "  assert(st);");
  GWEN_BufferedIO_WriteLine(bio, "  assert(db);");

  rv=write_code_fromdbrec_c(args, node, bio);
  if (rv) {
    DBG_ERROR(0, "Error in fromdb");
    return rv;
  }

  GWEN_BufferedIO_WriteLine(bio, "  return 0;");
  GWEN_BufferedIO_WriteLine(bio, "}");
  return 0;
}



int write_code_fromdb_c(ARGUMENTS *args, GWEN_XMLNODE *node,
                        GWEN_BUFFEREDIO *bio) {
  const char *prefix;
  const char *styp;
  const char *fromDbName;
  const char *fromDbAcc;

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
  fromDbAcc=get_struct_property(node, "dup-access",
                                get_struct_property(node, "access", 0));
  fromDbName=get_struct_property(node, "fromdb-name", 0);
  if (fromDbAcc && strcasecmp(fromDbAcc, "none")!=0) {
    GWEN_BufferedIO_Write(bio, styp);
    GWEN_BufferedIO_Write(bio, " *");
    GWEN_BufferedIO_Write(bio, prefix);
    if (fromDbName)
      GWEN_BufferedIO_Write(bio, fromDbName);
    else
      GWEN_BufferedIO_Write(bio, "_fromDb");
    GWEN_BufferedIO_WriteLine(bio, "(GWEN_DB_NODE *db) {");
  
    GWEN_BufferedIO_Write(bio, "  ");
    GWEN_BufferedIO_Write(bio, styp);
    GWEN_BufferedIO_WriteLine(bio, " *st;");
    GWEN_BufferedIO_WriteLine(bio, "");
    GWEN_BufferedIO_WriteLine(bio, "  assert(db);");
    GWEN_BufferedIO_Write(bio, "  st=");
    GWEN_BufferedIO_Write(bio, prefix);
    GWEN_BufferedIO_WriteLine(bio, "_new();");
  
    GWEN_BufferedIO_Write(bio, "  ");
    GWEN_BufferedIO_Write(bio, prefix);
    GWEN_BufferedIO_WriteLine(bio, "_ReadDb(st, db);");
  
    GWEN_BufferedIO_WriteLine(bio, "  st->_modified=0;");
    GWEN_BufferedIO_WriteLine(bio, "  return st;");
    GWEN_BufferedIO_WriteLine(bio, "}");
  } /* if fromDb wanted */
  return 0;
}



int write_code_builtin_c(ARGUMENTS *args, GWEN_XMLNODE *node,
                         GWEN_BUFFEREDIO *bio) {
  const char *prefix;
  const char *styp;
  const char *dupAcc;

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
  dupAcc=get_struct_property(node, "dup-access",
                             get_struct_property(node, "access", 0));

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

  /* list2 functions */
  if (get_struct_property(node, "list2", 0)) {
    /* List2_freeAll */
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
  /* list functions */
  if (get_struct_property(node, "list", 0)) {
    /* LIST_dup functions */
    if (dupAcc && strcasecmp(dupAcc, "none")!=0) {
      const char *dupName;

      dupName=get_struct_property(node, "dup-name", 0);
      GWEN_BufferedIO_Write(bio, styp);
      GWEN_BufferedIO_Write(bio, "_LIST *");

      GWEN_BufferedIO_Write(bio, prefix);
      GWEN_BufferedIO_Write(bio, "_List_dup(const ");
      GWEN_BufferedIO_Write(bio, styp);
      GWEN_BufferedIO_WriteLine(bio, "_LIST *stl) {");

      GWEN_BufferedIO_WriteLine(bio, "  if (stl) {");

      /* ELEMTYPE_LIST *nl; */
      GWEN_BufferedIO_Write(bio, "    ");
      GWEN_BufferedIO_Write(bio, styp);
      GWEN_BufferedIO_WriteLine(bio, "_LIST *nl;");

      /* ELEMTYPE *e; */
      GWEN_BufferedIO_Write(bio, "    ");
      GWEN_BufferedIO_Write(bio, styp);
      GWEN_BufferedIO_WriteLine(bio, " *e;");
      GWEN_BufferedIO_WriteLine(bio, "");

      /* nl=ElemType_List */
      GWEN_BufferedIO_Write(bio, "    nl=");
      GWEN_BufferedIO_Write(bio, prefix);
      GWEN_BufferedIO_WriteLine(bio, "_List_new();");

      /* e=ElemType_List_First */
      GWEN_BufferedIO_Write(bio, "    e=");
      GWEN_BufferedIO_Write(bio, prefix);
      GWEN_BufferedIO_WriteLine(bio, "_List_First(stl);");

      /* while (e) ; */
      GWEN_BufferedIO_WriteLine(bio, "    while(e) {");

      /* ELEMTYPE *ne; */
      GWEN_BufferedIO_Write(bio, "      ");
      GWEN_BufferedIO_Write(bio, styp);
      GWEN_BufferedIO_WriteLine(bio, " *ne;");
      GWEN_BufferedIO_WriteLine(bio, "");

      /* ne=ElemType_dup; assert(ne); */
      GWEN_BufferedIO_Write(bio, "      ne=");
      GWEN_BufferedIO_Write(bio, prefix);
      if (dupName)
        GWEN_BufferedIO_Write(bio, dupName);
      else
        GWEN_BufferedIO_Write(bio, "_dup");
      GWEN_BufferedIO_WriteLine(bio, "(e);");
      GWEN_BufferedIO_WriteLine(bio, "      assert(ne);");

      /* ElemType_List_Add(ne, st->NAME); */
      GWEN_BufferedIO_Write(bio, "      ");
      GWEN_BufferedIO_Write(bio, prefix);
      GWEN_BufferedIO_WriteLine(bio, "_List_Add(ne, nl);");

      /* e=ElemType_List_Next */
      GWEN_BufferedIO_Write(bio, "      e=");
      GWEN_BufferedIO_Write(bio, prefix);
      GWEN_BufferedIO_WriteLine(bio, "_List_Next(e);");

      GWEN_BufferedIO_WriteLine(bio, "    } /* while (e) */");


      GWEN_BufferedIO_WriteLine(bio, "    return nl;");

      GWEN_BufferedIO_WriteLine(bio, "  }");
      GWEN_BufferedIO_WriteLine(bio, "  else");
      GWEN_BufferedIO_WriteLine(bio, "    return 0;");
      GWEN_BufferedIO_WriteLine(bio, "}");
      GWEN_BufferedIO_WriteLine(bio, "");
      GWEN_BufferedIO_WriteLine(bio, "");
    } /* if we have a dup function */

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
  int err;
  const char *id;
  const char *prefix;
  GWEN_XMLNODE *n;

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

  GWEN_BufferedIO_WriteLine(bio, "#ifdef HAVE_CONFIG_H");
  GWEN_BufferedIO_WriteLine(bio, "# include \"config.h\"");
  GWEN_BufferedIO_WriteLine(bio, "#endif");
  GWEN_BufferedIO_WriteLine(bio, "");

  fname=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(fname, f);
  GWEN_Buffer_AppendString(fname, "_p.h");

  GWEN_BufferedIO_Write(bio, "#include \"");
  GWEN_BufferedIO_Write(bio, GWEN_Buffer_GetStart(fname));
  GWEN_BufferedIO_WriteLine(bio, "\"");
  GWEN_Buffer_free(fname);

  GWEN_BufferedIO_WriteLine(bio, "#include <gwenhywfar/misc.h>");
  GWEN_BufferedIO_WriteLine(bio, "#include <gwenhywfar/db.h>");
  GWEN_BufferedIO_WriteLine(bio, "#include <gwenhywfar/debug.h>");
  GWEN_BufferedIO_WriteLine(bio, "#include <assert.h>");
  GWEN_BufferedIO_WriteLine(bio, "#include <stdlib.h>");
  GWEN_BufferedIO_WriteLine(bio, "#include <strings.h>");
  GWEN_BufferedIO_WriteLine(bio, "");

  /* write headers */
  n=GWEN_XMLNode_FindFirstTag(node, "headers", 0, 0);
  if (n) {
    n=GWEN_XMLNode_FindFirstTag(n, "header", 0, 0);
    while(n) {
      write_h_header(args, n, bio, "source");
      n=GWEN_XMLNode_FindNextTag(n, "header", 0, 0);
    }
    GWEN_BufferedIO_WriteLine(bio, "");
    GWEN_BufferedIO_WriteLine(bio, "");
  }

  /* write c-headers */
  n=GWEN_XMLNode_FindFirstTag(node, "c-headers", 0, 0);
  if (n) {
    n=GWEN_XMLNode_FindFirstTag(n, "header", 0, 0);
    while(n) {
      write_h_header(args, n, bio, "source");
      n=GWEN_XMLNode_FindNextTag(n, "header", 0, 0);
    }
    GWEN_BufferedIO_WriteLine(bio, "");
    GWEN_BufferedIO_WriteLine(bio, "");
  }

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

  rv=write_c_enums(args, node, bio);
  if (rv) {
    GWEN_BufferedIO_Abandon(bio);
    GWEN_BufferedIO_free(bio);
    return rv;
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

  rv=write_code_readdb_c(args, node, bio);
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
  GWEN_BufferedIO_WriteLine(bio, "");
  GWEN_BufferedIO_WriteLine(bio, "");

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
  GWEN_BufferedIO_WriteLine(bio, "");

  /* close stream */
  err=GWEN_BufferedIO_Close(bio);
  if (err) {
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








