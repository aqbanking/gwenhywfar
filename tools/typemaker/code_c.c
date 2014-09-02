/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
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
#include <gwenhywfar/syncio_file.h>

#include <stdlib.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>



int write_c_enums(ARGUMENTS *args, GWEN_XMLNODE *node,
		  GWEN_SYNCIO *sio) {
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

      GWEN_SyncIo_WriteString(sio, GWEN_Buffer_GetStart(tid));
      GWEN_SyncIo_WriteString(sio, " ");
      GWEN_SyncIo_WriteString(sio, GWEN_Buffer_GetStart(tprefix));
      GWEN_SyncIo_WriteLine(sio, "_fromString(const char *s) {");
      GWEN_SyncIo_WriteLine(sio, "  if (s) {");

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
            GWEN_SyncIo_WriteString(sio, "    if (strcasecmp(s, \"");
          else
            GWEN_SyncIo_WriteString(sio, "    else if (strcasecmp(s, \"");
          s=GWEN_XMLNode_GetData(nnn);
          assert(s);
          GWEN_SyncIo_WriteString(sio, s);
          GWEN_SyncIo_WriteLine(sio, "\")==0)");
          GWEN_SyncIo_WriteString(sio, "      return ");

          GWEN_Buffer_AppendByte(tprefix, toupper(*s));
          GWEN_Buffer_AppendString(tprefix, s+1);
          GWEN_SyncIo_WriteString(sio, GWEN_Buffer_GetStart(tprefix));
          GWEN_SyncIo_WriteLine(sio, ";");

          GWEN_Buffer_Crop(tprefix, 0, vpos);
          GWEN_Buffer_SetPos(tprefix, vpos);
          first=0;
          nn=GWEN_XMLNode_FindNextTag(nn, "value", 0, 0);
        }
      }
      GWEN_SyncIo_WriteLine(sio, "  }");
      GWEN_SyncIo_WriteString(sio, "  return ");
      GWEN_SyncIo_WriteString(sio, GWEN_Buffer_GetStart(tprefix));
      GWEN_SyncIo_WriteLine(sio, "Unknown;");
      GWEN_SyncIo_WriteLine(sio, "}");
      GWEN_SyncIo_WriteLine(sio, "");
      GWEN_SyncIo_WriteLine(sio, "");

      GWEN_SyncIo_WriteString(sio, "const char *");
      GWEN_SyncIo_WriteString(sio, GWEN_Buffer_GetStart(tprefix));
      GWEN_SyncIo_WriteString(sio, "_toString(");
      GWEN_SyncIo_WriteString(sio, GWEN_Buffer_GetStart(tid));
      GWEN_SyncIo_WriteLine(sio, " v) {");
      GWEN_SyncIo_WriteLine(sio, "  switch(v) {");

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

          GWEN_SyncIo_WriteString(sio, "    case ");
          GWEN_SyncIo_WriteString(sio, GWEN_Buffer_GetStart(tprefix));
          GWEN_SyncIo_WriteLine(sio, ":");
          GWEN_SyncIo_WriteString(sio, "      return \"");
          GWEN_SyncIo_WriteString(sio, s);
          GWEN_SyncIo_WriteLine(sio, "\";");
          GWEN_SyncIo_WriteLine(sio, "");

          GWEN_Buffer_Crop(tprefix, 0, vpos);
          GWEN_Buffer_SetPos(tprefix, vpos);
          nn=GWEN_XMLNode_FindNextTag(nn, "value", 0, 0);
        }
      }
      GWEN_SyncIo_WriteLine(sio, "    default:");
      GWEN_SyncIo_WriteLine(sio, "      return \"unknown\";");


      GWEN_SyncIo_WriteLine(sio, "  } /* switch */");
      GWEN_SyncIo_WriteLine(sio, "} ");
      GWEN_SyncIo_WriteLine(sio, "");
      GWEN_SyncIo_WriteLine(sio, "");

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
                          GWEN_SYNCIO *sio){
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

  err=GWEN_SyncIo_WriteString(sio, "  if (st->");
  if (err) { DBG_ERROR_ERR(0, err); return -1;}
  err=GWEN_SyncIo_WriteString(sio, name);
  if (err) { DBG_ERROR_ERR(0, err); return -1;}
  err=GWEN_SyncIo_WriteLine(sio, ")");
  if (err) { DBG_ERROR_ERR(0, err); return -1;}

  if (strcmp(typ, "char")==0) {
    /* we can handle chars */
    err=GWEN_SyncIo_WriteString(sio, "    free(st->");
    if (err) { DBG_ERROR_ERR(0, err); return -1;}
    err=GWEN_SyncIo_WriteString(sio, name);
    if (err) { DBG_ERROR_ERR(0, err); return -1;}
    err=GWEN_SyncIo_WriteLine(sio, ");");
    if (err) { DBG_ERROR_ERR(0, err); return -1;}
    return 0;
  }
  else {
    const char *fname;

    fname=get_function_name(node, "free");
    if (fname) {
      err=GWEN_SyncIo_WriteString(sio, "    ");
      if (err) { DBG_ERROR_ERR(0, err); return -1;}
      err=GWEN_SyncIo_WriteString(sio, fname);
      if (err) { DBG_ERROR_ERR(0, err); return -1;}
      err=GWEN_SyncIo_WriteString(sio, "(st->");
      if (err) { DBG_ERROR_ERR(0, err); return -1;}
      err=GWEN_SyncIo_WriteString(sio, name);
      if (err) { DBG_ERROR_ERR(0, err); return -1;}
      err=GWEN_SyncIo_WriteLine(sio, ");");
      if (err) { DBG_ERROR_ERR(0, err); return -1;}
      return 0;
    }
  }

  DBG_ERROR(0, "Unknown \"free\" function for type \"%s\"", typ);
  return -1;
}



int write_code_freeElems_c(ARGUMENTS *args,
                           GWEN_XMLNODE *node,
                           GWEN_SYNCIO *sio){
  if (GWEN_XMLNode_GetType(node)==GWEN_XMLNodeTypeTag) {
    GWEN_XMLNODE *n;

    n=GWEN_XMLNode_GetFirstTag(node);
    while(n) {
      int rv;

      if (strcasecmp(GWEN_XMLNode_GetData(n), "group")==0)
        rv=write_code_freeElems_c(args, n, sio);
      else if (strcasecmp(GWEN_XMLNode_GetData(n), "elem")==0) {
	rv=write_code_freeElem_c(args, n, sio);
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
                        GWEN_SYNCIO *sio,
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
    err=GWEN_SyncIo_WriteString(sio, "strdup(");
    if (err) { DBG_ERROR_ERR(0, err); return -1;}
    err=GWEN_SyncIo_WriteString(sio, param);
    if (err) { DBG_ERROR_ERR(0, err); return -1;}
    err=GWEN_SyncIo_WriteLine(sio, ");");
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
    err=GWEN_SyncIo_WriteString(sio, fname);
    if (err) { DBG_ERROR_ERR(0, err); return -1;}
    err=GWEN_SyncIo_WriteString(sio, "(");
    err=GWEN_SyncIo_WriteString(sio, param);
    if (err) { DBG_ERROR_ERR(0, err); return -1;}
    err=GWEN_SyncIo_WriteLine(sio, ");");
    if (err) { DBG_ERROR_ERR(0, err); return -1;}
    return 0;
  }

  DBG_ERROR(0, "Unknown \"dup\" function for type \"%s\"", typ);
  return -1;
}



int write_code_todbArg_c(ARGUMENTS *args,
                         GWEN_XMLNODE *node,
			 GWEN_SYNCIO *sio) {
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
      GWEN_SyncIo_WriteString(sio, "    if (");
      GWEN_SyncIo_WriteString(sio, fname);
      GWEN_SyncIo_WriteString(sio, "(st->");
      GWEN_SyncIo_WriteChar(sio, tolower(*name));
      GWEN_SyncIo_WriteString(sio, name+1);
      GWEN_SyncIo_WriteString(sio,
                            ", GWEN_DB_GetGroup(db, "
                            "GWEN_DB_FLAGS_DEFAULT, \"");
      GWEN_SyncIo_WriteChar(sio, tolower(*name));
      GWEN_SyncIo_WriteString(sio, name+1);
      GWEN_SyncIo_WriteLine(sio, "\")))");
      GWEN_SyncIo_WriteLine(sio, "      return -1;");
    }
    else {
      if (strcasecmp(typ, "char")==0) {
        GWEN_SyncIo_WriteString(sio,
                                  "    if (GWEN_DB_SetCharValue(db, "
                                  "GWEN_DB_FLAGS_OVERWRITE_VARS, \"");
        GWEN_SyncIo_WriteChar(sio, tolower(*name));
        GWEN_SyncIo_WriteString(sio, name+1);
        GWEN_SyncIo_WriteString(sio, "\", st->");
        GWEN_SyncIo_WriteChar(sio, tolower(*name));
        GWEN_SyncIo_WriteString(sio, name+1);
        GWEN_SyncIo_WriteLine(sio, "))");
        GWEN_SyncIo_WriteLine(sio, "      return -1;");
      }
      else if (strcasecmp(typ, "GWEN_STRINGLIST")==0) {
        GWEN_SyncIo_WriteLine(sio, "    {");
        GWEN_SyncIo_WriteLine(sio, "      GWEN_STRINGLISTENTRY *se;");

        GWEN_SyncIo_WriteLine(sio, "");
        GWEN_SyncIo_WriteString(sio,"      GWEN_DB_DeleteVar(db, \"");
        GWEN_SyncIo_WriteChar(sio, tolower(*name));
        GWEN_SyncIo_WriteString(sio, name+1);
        GWEN_SyncIo_WriteLine(sio, "\");");

        GWEN_SyncIo_WriteString(sio, "      se=GWEN_StringList_FirstEntry(st->");
        GWEN_SyncIo_WriteChar(sio, tolower(*name));
        GWEN_SyncIo_WriteString(sio, name+1);
        GWEN_SyncIo_WriteLine(sio, ");");

        GWEN_SyncIo_WriteLine(sio, "      while(se) {");
        GWEN_SyncIo_WriteLine(sio, "        const char *s;");
        GWEN_SyncIo_WriteLine(sio, "");

        GWEN_SyncIo_WriteLine(sio, "        s=GWEN_StringListEntry_Data(se);");
        GWEN_SyncIo_WriteLine(sio, "        assert(s);");
        GWEN_SyncIo_WriteString(sio,
                              "        if (GWEN_DB_SetCharValue(db, "
                              "GWEN_DB_FLAGS_DEFAULT, \"");
        GWEN_SyncIo_WriteChar(sio, tolower(*name));
        GWEN_SyncIo_WriteString(sio, name+1);
        GWEN_SyncIo_WriteLine(sio, "\", s))");
        GWEN_SyncIo_WriteLine(sio, "          return -1;");

        GWEN_SyncIo_WriteLine(sio, "        se=GWEN_StringListEntry_Next(se);");
        GWEN_SyncIo_WriteLine(sio, "      } /* while */");
        GWEN_SyncIo_WriteLine(sio, "    }");
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
	GWEN_SyncIo_WriteLine(sio, "  if (1) {");
	GWEN_SyncIo_WriteLine(sio, "    GWEN_DB_NODE *dbT;");

        GWEN_SyncIo_WriteString(sio, "    ");
	GWEN_SyncIo_WriteString(sio, elemType);
	GWEN_SyncIo_WriteLine(sio, " *e;");
	GWEN_SyncIo_WriteLine(sio, "");
	GWEN_SyncIo_WriteString(sio,
			      "    dbT=GWEN_DB_GetGroup(db, "
			      "GWEN_PATH_FLAGS_CREATE_GROUP, \"");
	GWEN_SyncIo_WriteChar(sio, tolower(*name));
	GWEN_SyncIo_WriteString(sio, name+1);
	GWEN_SyncIo_WriteLine(sio, "\");");
	GWEN_SyncIo_WriteLine(sio, "    assert(dbT);");

	/* e=ElemType_List_First(st->name) */
	GWEN_SyncIo_WriteString(sio, "    e=");
	GWEN_SyncIo_WriteString(sio, elemPrefix);
	GWEN_SyncIo_WriteString(sio, "_List_First(st->");
	GWEN_SyncIo_WriteChar(sio, tolower(*name));
	GWEN_SyncIo_WriteString(sio, name+1);
	GWEN_SyncIo_WriteLine(sio, ");");

        /* while (e) */
	GWEN_SyncIo_WriteLine(sio, "    while(e) {");

	/* handle element type */
	GWEN_SyncIo_WriteString(sio, "      if (");
	GWEN_SyncIo_WriteString(sio, elemPrefix);
	GWEN_SyncIo_WriteString(sio, "_toDb(e, ");
	GWEN_SyncIo_WriteString(sio,
			      "GWEN_DB_GetGroup(dbT, "
			      "GWEN_PATH_FLAGS_CREATE_GROUP, \"");
	GWEN_SyncIo_WriteString(sio, "element");
	GWEN_SyncIo_WriteLine(sio, "\")))");
	GWEN_SyncIo_WriteLine(sio, "        return -1;");

	/* e=ElemType_List_Next(e) */
	GWEN_SyncIo_WriteString(sio, "      e=");
	GWEN_SyncIo_WriteString(sio, elemPrefix);
	GWEN_SyncIo_WriteLine(sio, "_List_Next(e);");

	GWEN_SyncIo_WriteLine(sio, "    } /* while */");

	GWEN_SyncIo_WriteLine(sio, "  } /* if (1) */");

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
	GWEN_SyncIo_WriteLine(sio, "  if (1) {");
	GWEN_SyncIo_WriteLine(sio, "    GWEN_DB_NODE *dbT;");
	GWEN_SyncIo_WriteString(sio, "    ");
	GWEN_SyncIo_WriteString(sio, elemType);
	GWEN_SyncIo_WriteLine(sio, "_LIST2_ITERATOR *it;");
	GWEN_SyncIo_WriteLine(sio, "");
	GWEN_SyncIo_WriteString(sio,
			      "    dbT=GWEN_DB_GetGroup(db, "
			      "GWEN_PATH_FLAGS_CREATE_GROUP, \"");
	GWEN_SyncIo_WriteChar(sio, tolower(*name));
	GWEN_SyncIo_WriteString(sio, name+1);
	GWEN_SyncIo_WriteLine(sio, "\");");
	GWEN_SyncIo_WriteLine(sio, "    assert(dbT);");

	/* it=ElemType_List2_First(st->name) */
	GWEN_SyncIo_WriteString(sio, "    it=");
	GWEN_SyncIo_WriteString(sio, elemPrefix);
	GWEN_SyncIo_WriteString(sio, "_List2_First(st->");
	GWEN_SyncIo_WriteChar(sio, tolower(*name));
	GWEN_SyncIo_WriteString(sio, name+1);
	GWEN_SyncIo_WriteLine(sio, ");");

        /* if (it) */
	GWEN_SyncIo_WriteLine(sio, "    if (it) {");
	GWEN_SyncIo_WriteString(sio, "      ");
	GWEN_SyncIo_WriteString(sio, elemType);
	GWEN_SyncIo_WriteLine(sio, " *e;");
	GWEN_SyncIo_WriteLine(sio, "");

	/* e=ElemType_List2Iterator_Data(it) */
	GWEN_SyncIo_WriteString(sio, "        e=");
	GWEN_SyncIo_WriteString(sio, elemPrefix);
	GWEN_SyncIo_WriteLine(sio, "_List2Iterator_Data(it);");
	GWEN_SyncIo_WriteString(sio, "        assert(e);");

	/* while (e) */
	GWEN_SyncIo_WriteLine(sio, "      while(e) {");

	/* handle element type */
	GWEN_SyncIo_WriteString(sio, "        if (");
	GWEN_SyncIo_WriteString(sio, elemPrefix);
	GWEN_SyncIo_WriteString(sio, "_toDb(e, ");
	GWEN_SyncIo_WriteString(sio,
			      "GWEN_DB_GetGroup(dbT, "
			      "GWEN_PATH_FLAGS_CREATE_GROUP, \"");
	GWEN_SyncIo_WriteString(sio, "element");
	GWEN_SyncIo_WriteLine(sio, "\")))");
	GWEN_SyncIo_WriteLine(sio, "          return -1;");

	/* e=ElemType_List2Iterator_Next(it) */
	GWEN_SyncIo_WriteString(sio, "        e=");
	GWEN_SyncIo_WriteString(sio, elemPrefix);
	GWEN_SyncIo_WriteLine(sio, "_List2Iterator_Next(it);");

	GWEN_SyncIo_WriteLine(sio, "      } /* while */");

        /* free iterator */
	GWEN_SyncIo_WriteString(sio, "      ");
	GWEN_SyncIo_WriteString(sio, elemPrefix);
	GWEN_SyncIo_WriteString(sio, "_List2Iterator_free(it);");

	GWEN_SyncIo_WriteLine(sio, "    } /* if (it) */");

	GWEN_SyncIo_WriteLine(sio, "  } /* if (1) */");

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
      GWEN_SyncIo_WriteString(sio,
                                "  if (GWEN_DB_SetIntValue(db, "
                                "GWEN_DB_FLAGS_OVERWRITE_VARS, \"");
    }
    else if (strcasecmp(btype, "char")==0) {
      GWEN_SyncIo_WriteString(sio,
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
          GWEN_SyncIo_WriteString(sio, "  if (GWEN_DB_SetCharValue(db, ");
          GWEN_SyncIo_WriteString(sio, "GWEN_DB_FLAGS_OVERWRITE_VARS, \"");
          GWEN_SyncIo_WriteChar(sio, tolower(*name));
          GWEN_SyncIo_WriteString(sio, name+1);
          GWEN_SyncIo_WriteString(sio, "\", ");

          GWEN_Buffer_AppendString(tprefix, s);
          GWEN_SyncIo_WriteString(sio, GWEN_Buffer_GetStart(tprefix));
          GWEN_SyncIo_WriteString(sio, "_toString(st->");
          GWEN_SyncIo_WriteChar(sio, tolower(*name));
          GWEN_SyncIo_WriteString(sio, name+1);
          GWEN_SyncIo_WriteString(sio, ")");
          GWEN_SyncIo_WriteString(sio, ")");
          GWEN_SyncIo_WriteLine(sio, ") ");
          GWEN_SyncIo_WriteLine(sio, "    return -1;");

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

    GWEN_SyncIo_WriteChar(sio, tolower(*name));
    GWEN_SyncIo_WriteString(sio, name+1);
    GWEN_SyncIo_WriteString(sio, "\", st->");
    GWEN_SyncIo_WriteChar(sio, tolower(*name));
    GWEN_SyncIo_WriteString(sio, name+1);
    GWEN_SyncIo_WriteLine(sio, "))");
    GWEN_SyncIo_WriteLine(sio, "    return -1;");
  }

  return 0;
}



int write_code_fromdbArg_c(ARGUMENTS *args,
                           GWEN_XMLNODE *node,
                           GWEN_SYNCIO *sio) {
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
      GWEN_SyncIo_WriteString(sio, fname);
      GWEN_SyncIo_WriteString(sio, "(dbT)");
    }
    else {
      if (strcasecmp(typ, "char")==0) {
        GWEN_SyncIo_WriteString(sio, "GWEN_DB_GetCharValue(db, \"");
	GWEN_SyncIo_WriteChar(sio, tolower(*name));
        GWEN_SyncIo_WriteString(sio, name+1);
        GWEN_SyncIo_WriteString(sio, "\", 0, ");
        if (defval) {
          GWEN_SyncIo_WriteString(sio, "\"");
          GWEN_SyncIo_WriteString(sio, defval);
          GWEN_SyncIo_WriteString(sio, "\"");
        }
        else {
          GWEN_SyncIo_WriteString(sio, "0");
        }
	GWEN_SyncIo_WriteString(sio, ")");
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
      GWEN_SyncIo_WriteString(sio, "GWEN_DB_GetIntValue(db, \"");
      GWEN_SyncIo_WriteChar(sio, tolower(*name));
      GWEN_SyncIo_WriteString(sio, name+1);
      GWEN_SyncIo_WriteString(sio, "\", 0, ");
      if (defval) {
        GWEN_SyncIo_WriteString(sio, defval);
      }
      else {
        GWEN_SyncIo_WriteString(sio, "0");
      }
      GWEN_SyncIo_WriteString(sio, ")");
    }
    else if (strcasecmp(btype, "char")==0) {
      GWEN_SyncIo_WriteString(sio, "GWEN_DB_GetCharValue(db, \"");
      GWEN_SyncIo_WriteChar(sio, tolower(*name));
      GWEN_SyncIo_WriteString(sio, name+1);
      GWEN_SyncIo_WriteString(sio, "\", 0, ");
      if (defval) {
        GWEN_SyncIo_WriteString(sio, "\"");
        GWEN_SyncIo_WriteString(sio, defval);
        GWEN_SyncIo_WriteString(sio, "\"");
      }
      else {
        GWEN_SyncIo_WriteString(sio, "0");
      }
      GWEN_SyncIo_WriteString(sio, ")");
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
        GWEN_SyncIo_WriteString(sio, GWEN_Buffer_GetStart(tprefix));
        GWEN_SyncIo_WriteString(sio, "_fromString(");
        GWEN_SyncIo_WriteString(sio, "GWEN_DB_GetCharValue(db, \"");
        GWEN_SyncIo_WriteChar(sio, tolower(*name));
        GWEN_SyncIo_WriteString(sio, name+1);
        GWEN_SyncIo_WriteString(sio, "\", 0, ");
        if (defval) {
          GWEN_SyncIo_WriteString(sio, "\"");
          GWEN_SyncIo_WriteString(sio, defval);
          GWEN_SyncIo_WriteString(sio, "\"");
        }
        else {
          GWEN_SyncIo_WriteString(sio, "0");
        }
        GWEN_SyncIo_WriteString(sio, ")");
        GWEN_SyncIo_WriteString(sio, ")");
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
			  GWEN_SYNCIO *sio){
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
	rv=write_code_constrec_c(args, n, sio);
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
	    GWEN_SyncIo_WriteString(sio, "  st->");
	    GWEN_SyncIo_WriteChar(sio, tolower(*name));
	    GWEN_SyncIo_WriteString(sio, name+1);
	    GWEN_SyncIo_WriteLine(sio, "=GWEN_StringList_new();");
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
              GWEN_SyncIo_WriteString(sio, "  st->");
              GWEN_SyncIo_WriteChar(sio, tolower(*name));
              GWEN_SyncIo_WriteString(sio, name+1);
              GWEN_SyncIo_WriteString(sio, "=");
              GWEN_SyncIo_WriteString(sio, fname);
              GWEN_SyncIo_WriteLine(sio, "();");
            } /* if init requested */
          } /* if !single */
          else {
            if (setval) {
              GWEN_SyncIo_WriteString(sio, "  st->");
              GWEN_SyncIo_WriteChar(sio, tolower(*name));
              GWEN_SyncIo_WriteString(sio, name+1);
              GWEN_SyncIo_WriteString(sio, "=");
              GWEN_SyncIo_WriteString(sio, setval);
              GWEN_SyncIo_WriteLine(sio, ";");
            }
          }
	}
	else {
          if (setval) {
            /* TODO: check for enum values */
	    GWEN_SyncIo_WriteString(sio, "  st->");
	    GWEN_SyncIo_WriteChar(sio, tolower(*name));
	    GWEN_SyncIo_WriteString(sio, name+1);
	    GWEN_SyncIo_WriteString(sio, "=");
	    GWEN_SyncIo_WriteString(sio, setval);
	    GWEN_SyncIo_WriteLine(sio, ";");
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
                       GWEN_SYNCIO *sio){
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

  GWEN_SyncIo_WriteString(sio, styp);
  GWEN_SyncIo_WriteString(sio, " *");
  GWEN_SyncIo_WriteString(sio, prefix);
  if (constName && *constName)
    GWEN_SyncIo_WriteString(sio, constName);
  else
    GWEN_SyncIo_WriteString(sio, "_new");
  GWEN_SyncIo_WriteLine(sio, "(void) {");

  GWEN_SyncIo_WriteString(sio, "  ");
  GWEN_SyncIo_WriteString(sio, styp);
  GWEN_SyncIo_WriteLine(sio, " *st;");
  GWEN_SyncIo_WriteLine(sio, "");

  GWEN_SyncIo_WriteString(sio, "  GWEN_NEW_OBJECT(");
  GWEN_SyncIo_WriteString(sio, styp);
  GWEN_SyncIo_WriteLine(sio, ", st)");
  GWEN_SyncIo_WriteLine(sio, "  st->_usage=1;");

  // add inherit functions
  if (get_struct_property(node, "inherit", 0)) {
    GWEN_SyncIo_WriteString(sio, "  GWEN_INHERIT_INIT(");
    GWEN_SyncIo_WriteString(sio, styp);
    GWEN_SyncIo_WriteLine(sio, ", st)");
  }

  // add list functions
  if (get_struct_property(node, "list", 0)) {
    GWEN_SyncIo_WriteString(sio, "  GWEN_LIST_INIT(");
    GWEN_SyncIo_WriteString(sio, styp);
    GWEN_SyncIo_WriteLine(sio, ", st)");
  }

  rv=write_code_constrec_c(args, node, sio);
  if (rv)
    return rv;

  GWEN_SyncIo_WriteLine(sio, "  return st;");
  GWEN_SyncIo_WriteLine(sio, "}");

  return 0;
}



int write_code_dest_c(ARGUMENTS *args,
                      GWEN_XMLNODE *node,
                      GWEN_SYNCIO *sio){
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

  GWEN_SyncIo_WriteString(sio, "void ");
  GWEN_SyncIo_WriteString(sio, prefix);
  GWEN_SyncIo_WriteString(sio, "_free(");
  GWEN_SyncIo_WriteString(sio, styp);
  GWEN_SyncIo_WriteLine(sio, " *st) {");

  GWEN_SyncIo_WriteLine(sio, "  if (st) {");

  GWEN_SyncIo_WriteLine(sio, "    assert(st->_usage);");
  GWEN_SyncIo_WriteLine(sio, "    if (--(st->_usage)==0) {");

  // add inherit functions
  if (get_struct_property(node, "inherit", 0)) {
    GWEN_SyncIo_WriteString(sio, "  GWEN_INHERIT_FINI(");
    GWEN_SyncIo_WriteString(sio, styp);
    GWEN_SyncIo_WriteLine(sio, ", st)");
  }

  rv=write_code_freeElems_c(args, node, sio);
  if (rv) {
    DBG_ERROR(0, "Error in freeElems");
    return rv;
  }

  // add list functions
  if (get_struct_property(node, "list", 0)) {
    GWEN_SyncIo_WriteString(sio, "  GWEN_LIST_FINI(");
    GWEN_SyncIo_WriteString(sio, styp);
    GWEN_SyncIo_WriteLine(sio, ", st)");
  }

  GWEN_SyncIo_WriteLine(sio, "  GWEN_FREE_OBJECT(st);");
  GWEN_SyncIo_WriteLine(sio, "    }");
  GWEN_SyncIo_WriteLine(sio, "  }");
  GWEN_SyncIo_WriteLine(sio, "");
  GWEN_SyncIo_WriteLine(sio, "}");

  return 0;
}



int write_code_setget_c(ARGUMENTS *args,
                        GWEN_XMLNODE *node,
                        GWEN_SYNCIO *sio){
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
        rv=write_code_setget_c(args, n, sio);
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

        GWEN_SyncIo_WriteLine(sio, "");
        GWEN_SyncIo_WriteLine(sio, "");

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
          GWEN_SyncIo_WriteString(sio, "const ");
        }
        if (strcasecmp(tmode, "enum")!=0)
          GWEN_SyncIo_WriteString(sio, typ);
        else {
          GWEN_BUFFER *tid;
          const char *s;

          tid=GWEN_Buffer_new(0, 64, 0, 1);
          s=get_struct_property(node, "id", 0);
          assert(s);
          GWEN_Buffer_AppendString(tid, s);
          GWEN_Buffer_AppendString(tid, "_");
          GWEN_Buffer_AppendString(tid, typ);
          GWEN_SyncIo_WriteString(sio, GWEN_Buffer_GetStart(tid));
          GWEN_Buffer_free(tid);
        }
        if (isPtr) {
          GWEN_SyncIo_WriteString(sio, " *");
        }
        else {
          GWEN_SyncIo_WriteString(sio, " ");
        }
        GWEN_SyncIo_WriteString(sio, prefix);
        GWEN_SyncIo_WriteString(sio, "_Get");
        GWEN_SyncIo_WriteChar(sio, toupper(*name));
        GWEN_SyncIo_WriteString(sio, name+1);
        GWEN_SyncIo_WriteString(sio, "(const ");
        GWEN_SyncIo_WriteString(sio, styp);
        GWEN_SyncIo_WriteLine(sio, " *st) {");

        GWEN_SyncIo_WriteLine(sio, "  assert(st);");

        GWEN_SyncIo_WriteString(sio, "  return st->");
        GWEN_SyncIo_WriteChar(sio, tolower(*name));
        GWEN_SyncIo_WriteString(sio, name+1);
        GWEN_SyncIo_WriteLine(sio, ";");
        GWEN_SyncIo_WriteLine(sio, "}");
        GWEN_SyncIo_WriteLine(sio, "");
        GWEN_SyncIo_WriteLine(sio, "");

        /* write setter */
        GWEN_SyncIo_WriteString(sio, "void ");
        GWEN_SyncIo_WriteString(sio, prefix);
        GWEN_SyncIo_WriteString(sio, "_Set");
        GWEN_SyncIo_WriteChar(sio, toupper(*name));;
        GWEN_SyncIo_WriteString(sio, name+1);
        GWEN_SyncIo_WriteString(sio, "(");
        GWEN_SyncIo_WriteString(sio, styp);
        GWEN_SyncIo_WriteString(sio, " *st, ");
        if (isPtr && isConst) {
          GWEN_SyncIo_WriteString(sio, "const ");
        }
        if (strcasecmp(tmode, "enum")!=0)
          GWEN_SyncIo_WriteString(sio, typ);
        else {
          GWEN_BUFFER *tid;
          const char *s;

          tid=GWEN_Buffer_new(0, 64, 0, 1);
          s=get_struct_property(node, "id", 0);
          assert(s);
          GWEN_Buffer_AppendString(tid, s);
          GWEN_Buffer_AppendString(tid, "_");
          GWEN_Buffer_AppendString(tid, typ);
          GWEN_SyncIo_WriteString(sio, GWEN_Buffer_GetStart(tid));
          GWEN_Buffer_free(tid);
        }
        if (isPtr) {
          GWEN_SyncIo_WriteString(sio, " *");
        }
        else {
          GWEN_SyncIo_WriteString(sio, " ");
        }
        GWEN_SyncIo_WriteLine(sio, "d) {");
        GWEN_SyncIo_WriteLine(sio, "  assert(st);");

        if (isPtr) {
          /* free old pointer if any */
          rv=write_code_freeElem_c(args, n, sio);
          if (rv)
            return rv;

          /* copy argument if any */
          if (strcasecmp(mode, "single")==0) {
            if (strcasecmp(typ, "char")==0)
              GWEN_SyncIo_WriteLine(sio, "  if (d && *d)");
            else
              GWEN_SyncIo_WriteLine(sio, "  if (d)");

            GWEN_SyncIo_WriteString(sio, "    st->");
            GWEN_SyncIo_WriteChar(sio, tolower(*name));
            GWEN_SyncIo_WriteString(sio, name+1);
            GWEN_SyncIo_WriteString(sio, "=");

            if (doCopy) {
              rv=write_code_dupArg_c(args, n, sio, "d");
              if (rv)
                return rv;
            }
            else {
              GWEN_SyncIo_WriteLine(sio, "d;");
            }
          }
          else {
            if (doCopy) {
              rv=write_code_dupList_c(args, n, sio, "d");
              if (rv)
                return rv;
            }
            else {
              if (isConst) {
                DBG_ERROR(0, "Properties: CONST but not COPY");
                return -1;
              }
              GWEN_SyncIo_WriteString(sio, "    st->");
              GWEN_SyncIo_WriteChar(sio, tolower(*name));
              GWEN_SyncIo_WriteString(sio, name+1);
              GWEN_SyncIo_WriteLine(sio, "=d;");
            }
          }
          GWEN_SyncIo_WriteLine(sio, "  else");
          GWEN_SyncIo_WriteString(sio, "    st->");
          GWEN_SyncIo_WriteChar(sio, tolower(*name));
          GWEN_SyncIo_WriteString(sio, name+1);
          GWEN_SyncIo_WriteLine(sio, "=0;");
        }
        else {
          GWEN_SyncIo_WriteString(sio, "  st->");
          GWEN_SyncIo_WriteChar(sio, tolower(*name));
          GWEN_SyncIo_WriteString(sio, name+1);
          GWEN_SyncIo_WriteLine(sio, "=d;");
        }
        GWEN_SyncIo_WriteLine(sio, "  st->_modified=1;");
        GWEN_SyncIo_WriteLine(sio, "}");
        GWEN_SyncIo_WriteLine(sio, "");
        GWEN_SyncIo_WriteLine(sio, "");

        if (strcasecmp(typ, "GWEN_STRINGLIST")==0) {
          /* special functions for string lists */
          GWEN_SyncIo_WriteLine(sio, "");
          GWEN_SyncIo_WriteLine(sio, "");
          GWEN_SyncIo_WriteString(sio, "void ");
          GWEN_SyncIo_WriteString(sio, prefix);
          GWEN_SyncIo_WriteString(sio, "_Add");
          GWEN_SyncIo_WriteChar(sio, toupper(*name));
          GWEN_SyncIo_WriteString(sio, name+1);
          GWEN_SyncIo_WriteString(sio, "(");
          GWEN_SyncIo_WriteString(sio, styp);
          GWEN_SyncIo_WriteLine(sio, " *st, const char *d, int chk){");
          GWEN_SyncIo_WriteLine(sio, "  assert(st);");
          GWEN_SyncIo_WriteLine(sio, "  assert(d);");
          GWEN_SyncIo_WriteString(sio, "  if (GWEN_StringList_AppendString(st->");
          GWEN_SyncIo_WriteChar(sio, tolower(*name));
          GWEN_SyncIo_WriteString(sio, name+1);
          GWEN_SyncIo_WriteLine(sio, ", d, 0, chk))");
          GWEN_SyncIo_WriteLine(sio, "    st->_modified=1;");
          GWEN_SyncIo_WriteLine(sio, "}");
          GWEN_SyncIo_WriteLine(sio, "");
          GWEN_SyncIo_WriteLine(sio, "");

          /* remove */
          GWEN_SyncIo_WriteString(sio, "void ");
          GWEN_SyncIo_WriteString(sio, prefix);
          GWEN_SyncIo_WriteString(sio, "_Remove");
          GWEN_SyncIo_WriteChar(sio, toupper(*name));
          GWEN_SyncIo_WriteString(sio, name+1);
          GWEN_SyncIo_WriteString(sio, "(");
          GWEN_SyncIo_WriteString(sio, styp);
          GWEN_SyncIo_WriteLine(sio, " *st, const char *d) {");
          GWEN_SyncIo_WriteString(sio, "  if (GWEN_StringList_RemoveString(st->");
          GWEN_SyncIo_WriteChar(sio, tolower(*name));
          GWEN_SyncIo_WriteString(sio, name+1);
          GWEN_SyncIo_WriteLine(sio, ", d))");
          GWEN_SyncIo_WriteLine(sio, "    st->_modified=1;");
          GWEN_SyncIo_WriteLine(sio, "}");
          GWEN_SyncIo_WriteLine(sio, "");
          GWEN_SyncIo_WriteLine(sio, "");

          /* clear */
          GWEN_SyncIo_WriteString(sio, "void ");
          GWEN_SyncIo_WriteString(sio, prefix);
          GWEN_SyncIo_WriteString(sio, "_Clear");
          GWEN_SyncIo_WriteChar(sio, toupper(*name));
          GWEN_SyncIo_WriteString(sio, name+1);
          GWEN_SyncIo_WriteString(sio, "(");
          GWEN_SyncIo_WriteString(sio, styp);
          GWEN_SyncIo_WriteLine(sio, " *st) {");
          GWEN_SyncIo_WriteString(sio, "  if (GWEN_StringList_Count(st->");
          GWEN_SyncIo_WriteChar(sio, tolower(*name));
          GWEN_SyncIo_WriteString(sio, name+1);
          GWEN_SyncIo_WriteLine(sio, ")) {");
          GWEN_SyncIo_WriteString(sio, "    GWEN_StringList_Clear(st->");
          GWEN_SyncIo_WriteChar(sio, tolower(*name));
          GWEN_SyncIo_WriteString(sio, name+1);
          GWEN_SyncIo_WriteLine(sio, ");");
          GWEN_SyncIo_WriteLine(sio, "    st->_modified=1;");
          GWEN_SyncIo_WriteLine(sio, "  }");
          GWEN_SyncIo_WriteLine(sio, "}");
          GWEN_SyncIo_WriteLine(sio, "");
          GWEN_SyncIo_WriteLine(sio, "");

          /* has */
          GWEN_SyncIo_WriteString(sio, "int ");
          GWEN_SyncIo_WriteString(sio, prefix);
          GWEN_SyncIo_WriteString(sio, "_Has");
          GWEN_SyncIo_WriteChar(sio, toupper(*name));
          GWEN_SyncIo_WriteString(sio, name+1);
          GWEN_SyncIo_WriteString(sio, "(const ");
          GWEN_SyncIo_WriteString(sio, styp);
          GWEN_SyncIo_WriteLine(sio, " *st, const char *d) {");
          GWEN_SyncIo_WriteString(sio, "  return GWEN_StringList_HasString(st->");
          GWEN_SyncIo_WriteChar(sio, tolower(*name));
          GWEN_SyncIo_WriteString(sio, name+1);
          GWEN_SyncIo_WriteLine(sio, ", d);");
          GWEN_SyncIo_WriteLine(sio, "}");
          GWEN_SyncIo_WriteLine(sio, "");
          GWEN_SyncIo_WriteLine(sio, "");
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
        GWEN_SyncIo_WriteString(sio, styp);
        GWEN_SyncIo_WriteString(sio, "_");
        GWEN_SyncIo_WriteString(sio, typ);
        GWEN_SyncIo_WriteString(sio, " ");
        GWEN_SyncIo_WriteString(sio, prefix);
        GWEN_SyncIo_WriteString(sio, "_Get");
        GWEN_SyncIo_WriteChar(sio, toupper(*name));
        GWEN_SyncIo_WriteString(sio, name+1);
        GWEN_SyncIo_WriteString(sio, "(const ");
        GWEN_SyncIo_WriteString(sio, styp);
        GWEN_SyncIo_WriteLine(sio, " *st) {");
        GWEN_SyncIo_WriteLine(sio, "  assert(st);");
        GWEN_SyncIo_WriteString(sio, "  return st->");
        GWEN_SyncIo_WriteChar(sio, tolower(*name));
        GWEN_SyncIo_WriteString(sio, name+1);
        GWEN_SyncIo_WriteLine(sio, ";");
        GWEN_SyncIo_WriteLine(sio, "}");
        GWEN_SyncIo_WriteLine(sio, "");
        GWEN_SyncIo_WriteLine(sio, "");

        /* setter */
        GWEN_SyncIo_WriteString(sio, "void ");
        GWEN_SyncIo_WriteString(sio, prefix);
        GWEN_SyncIo_WriteString(sio, "_Set");
        GWEN_SyncIo_WriteChar(sio, toupper(*name));
        GWEN_SyncIo_WriteString(sio, name+1);
        GWEN_SyncIo_WriteString(sio, "(");
        GWEN_SyncIo_WriteString(sio, styp);
        GWEN_SyncIo_WriteString(sio, " *st, ");
        GWEN_SyncIo_WriteString(sio, styp);
        GWEN_SyncIo_WriteString(sio, "_");
        GWEN_SyncIo_WriteString(sio, typ);
        GWEN_SyncIo_WriteLine(sio, " d) {");
        GWEN_SyncIo_WriteLine(sio, "  assert(st);");
        GWEN_SyncIo_WriteString(sio, "  st->");
        GWEN_SyncIo_WriteChar(sio, tolower(*name));
        GWEN_SyncIo_WriteString(sio, name+1);
        GWEN_SyncIo_WriteLine(sio, "=d;");
        GWEN_SyncIo_WriteLine(sio, "}");
        GWEN_SyncIo_WriteLine(sio, "");
        GWEN_SyncIo_WriteLine(sio, "");

        /* function call */
        GWEN_SyncIo_WriteString(sio, rettype);
        if (isPtr)
          GWEN_SyncIo_WriteString(sio, "*");
        GWEN_SyncIo_WriteString(sio, " ");
        GWEN_SyncIo_WriteString(sio, prefix);
        GWEN_SyncIo_WriteString(sio, "_");
        GWEN_SyncIo_WriteChar(sio, toupper(*name));
        GWEN_SyncIo_WriteString(sio, name+1);
        GWEN_SyncIo_WriteString(sio, "(");

        GWEN_SyncIo_WriteString(sio, styp);
        GWEN_SyncIo_WriteString(sio, " *st");

        anode=GWEN_XMLNode_FindFirstTag(n, "arg", 0, 0);
        idx=0;
        while(anode) {
          const char *aname;
          const char *atype;
          int aisPtr;
    
          GWEN_SyncIo_WriteString(sio, ", ");

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
    
          GWEN_SyncIo_WriteString(sio, atype);
          if (aisPtr)
            GWEN_SyncIo_WriteString(sio, "*");
          GWEN_SyncIo_WriteString(sio, " ");
          GWEN_SyncIo_WriteString(sio, aname);
    
          idx++;
          anode=GWEN_XMLNode_FindNextTag(anode, "arg", 0, 0);
        }

        GWEN_SyncIo_WriteLine(sio, ") {");
        GWEN_SyncIo_WriteLine(sio, "  assert(st);");
        GWEN_SyncIo_WriteString(sio, "  if (st->");
        GWEN_SyncIo_WriteChar(sio, tolower(*name));
        GWEN_SyncIo_WriteString(sio, name+1);
        GWEN_SyncIo_WriteLine(sio, ")");
        GWEN_SyncIo_WriteString(sio, "    ");
        if (!isVoid)
          GWEN_SyncIo_WriteString(sio, "return ");
        GWEN_SyncIo_WriteString(sio, "st->");
        GWEN_SyncIo_WriteChar(sio, tolower(*name));
        GWEN_SyncIo_WriteString(sio, name+1);
        GWEN_SyncIo_WriteString(sio, "(st");

        anode=GWEN_XMLNode_FindFirstTag(n, "arg", 0, 0);
        while(anode) {
          const char *aname;

          GWEN_SyncIo_WriteString(sio, ", ");
          aname=GWEN_XMLNode_GetProperty(anode, "name", 0);
          GWEN_SyncIo_WriteString(sio, aname);
          anode=GWEN_XMLNode_FindNextTag(anode, "arg", 0, 0);
        }
        GWEN_SyncIo_WriteLine(sio, ");");
        if (!isVoid) {
          GWEN_SyncIo_WriteString(sio, "return ");
          GWEN_SyncIo_WriteString(sio, defret);
          GWEN_SyncIo_WriteLine(sio, ";");
        }
        GWEN_SyncIo_WriteLine(sio, "}");
        GWEN_SyncIo_WriteLine(sio, "");
        GWEN_SyncIo_WriteLine(sio, "");
      }
    }
    n=GWEN_XMLNode_GetNextTag(n);
  }
  return 0;
}






int write_code_dupList_c(ARGUMENTS *args, GWEN_XMLNODE *n,
                         GWEN_SYNCIO *sio,
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
      GWEN_SyncIo_WriteString(sio, "  if (");
      GWEN_SyncIo_WriteString(sio, listName);
      GWEN_SyncIo_WriteLine(sio, ") {");
  
      /* ELEMTYPE *e; */
      GWEN_SyncIo_WriteString(sio, "    ");
      GWEN_SyncIo_WriteString(sio, elemType);
      GWEN_SyncIo_WriteLine(sio, " *e;");
      GWEN_SyncIo_WriteLine(sio, "");

      /* st->LIST=LIST_new() */
      GWEN_SyncIo_WriteString(sio, "  ");
      GWEN_SyncIo_WriteString(sio, "st->");
      GWEN_SyncIo_WriteChar(sio, tolower(*name));
      GWEN_SyncIo_WriteString(sio, name+1);
      GWEN_SyncIo_WriteString(sio, "=");
      GWEN_SyncIo_WriteString(sio, elemPrefix);
      GWEN_SyncIo_WriteLine(sio, "_List_new();");


      /* e=ElemType_List_First */
      GWEN_SyncIo_WriteString(sio, "    e=");
      GWEN_SyncIo_WriteString(sio, elemPrefix);
      GWEN_SyncIo_WriteString(sio, "_List_First(");
      GWEN_SyncIo_WriteString(sio, listName);
      GWEN_SyncIo_WriteLine(sio, ");");
  
      /* while (e) ; */
      GWEN_SyncIo_WriteLine(sio, "    while(e) {");
  
      /* ELEMTYPE *ne; */
      GWEN_SyncIo_WriteString(sio, "      ");
      GWEN_SyncIo_WriteString(sio, elemType);
      GWEN_SyncIo_WriteLine(sio, " *ne;");
      GWEN_SyncIo_WriteLine(sio, "");
  
      /* ne=ElemType_dup; assert(ne); */
      GWEN_SyncIo_WriteString(sio, "      ne=");
      GWEN_SyncIo_WriteString(sio, elemPrefix);
      GWEN_SyncIo_WriteLine(sio, "_dup(e);");
      GWEN_SyncIo_WriteLine(sio, "      assert(ne);");
  
      /* ElemType_List_Add(ne, st->NAME); */
      GWEN_SyncIo_WriteString(sio, "      ");
      GWEN_SyncIo_WriteString(sio, elemPrefix);
      GWEN_SyncIo_WriteString(sio, "_List_Add(ne, st->");
      GWEN_SyncIo_WriteChar(sio, tolower(*name));
      GWEN_SyncIo_WriteString(sio, name+1);
      GWEN_SyncIo_WriteLine(sio, ");");
  
      /* e=ElemType_List_Next */
      GWEN_SyncIo_WriteString(sio, "      e=");
      GWEN_SyncIo_WriteString(sio, elemPrefix);
      GWEN_SyncIo_WriteLine(sio, "_List_Next(e);");
  
      GWEN_SyncIo_WriteLine(sio, "    } /* while (e) */");
  
      GWEN_SyncIo_WriteLine(sio, "  } /* if LIST */");
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
      GWEN_SyncIo_WriteString(sio, "  st->");
      GWEN_SyncIo_WriteChar(sio, tolower(*name));
      GWEN_SyncIo_WriteString(sio, name+1);
      GWEN_SyncIo_WriteString(sio, "=");
      GWEN_SyncIo_WriteString(sio, elemPrefix);
      GWEN_SyncIo_WriteLine(sio, "_List2_new();");
  
      GWEN_SyncIo_WriteString(sio, "  if (");
      GWEN_SyncIo_WriteString(sio, listName);
      GWEN_SyncIo_WriteLine(sio, ") {");
  
      GWEN_SyncIo_WriteString(sio, "    ");
      GWEN_SyncIo_WriteString(sio, elemType);
      GWEN_SyncIo_WriteLine(sio, "_LIST2_ITERATOR *it;");
      GWEN_SyncIo_WriteLine(sio, "");
  
      /* it=ElemType_List2_First */
      GWEN_SyncIo_WriteString(sio, "    it=");
      GWEN_SyncIo_WriteString(sio, elemPrefix);
      GWEN_SyncIo_WriteString(sio, "_List2_First(");
      GWEN_SyncIo_WriteString(sio, listName);
      GWEN_SyncIo_WriteLine(sio, ");");
  
      /* if (it) */
      GWEN_SyncIo_WriteLine(sio, "    if (it) {");
  
      /* ELEMTYPE *e; */
      GWEN_SyncIo_WriteString(sio, "      ");
      GWEN_SyncIo_WriteString(sio, elemType);
      GWEN_SyncIo_WriteLine(sio, " *e;");
      GWEN_SyncIo_WriteLine(sio, "");
  
      /* e=ElemType_List2Iterator_Data */
      GWEN_SyncIo_WriteString(sio, "      e=");
      GWEN_SyncIo_WriteString(sio, elemPrefix);
      GWEN_SyncIo_WriteLine(sio, "_List2Iterator_Data(it);");
      GWEN_SyncIo_WriteLine(sio, "      assert(e);");
  
      /* while (e) ; */
      GWEN_SyncIo_WriteLine(sio, "      while(e) {");
  
      /* ELEMTYPE *ne; */
      GWEN_SyncIo_WriteString(sio, "        ");
      GWEN_SyncIo_WriteString(sio, elemType);
      GWEN_SyncIo_WriteLine(sio, " *ne;");
      GWEN_SyncIo_WriteLine(sio, "");
  
      /* ne=ElemType_dup; assert(ne); */
      GWEN_SyncIo_WriteString(sio, "        ne=");
      GWEN_SyncIo_WriteString(sio, elemPrefix);
      GWEN_SyncIo_WriteLine(sio, "_dup(e);");
      GWEN_SyncIo_WriteLine(sio, "        assert(ne);");
  
      /* ElemType_List2_PushBack(st->NAME, ne); */
      GWEN_SyncIo_WriteString(sio, "        ");
      GWEN_SyncIo_WriteString(sio, elemPrefix);
      GWEN_SyncIo_WriteString(sio, "_List2_PushBack(st->");
      GWEN_SyncIo_WriteChar(sio, tolower(*name));
      GWEN_SyncIo_WriteString(sio, name+1);
      GWEN_SyncIo_WriteLine(sio, ", ne);");
  
      /* e=ElemType_List2Iterator_Next */
      GWEN_SyncIo_WriteString(sio, "        e=");
      GWEN_SyncIo_WriteString(sio, elemPrefix);
      GWEN_SyncIo_WriteLine(sio, "_List2Iterator_Next(it);");
  
      GWEN_SyncIo_WriteLine(sio, "      } /* while (e) */");
  
      /* ElemType_List2Iterator_free */
      GWEN_SyncIo_WriteString(sio, "        ");
      GWEN_SyncIo_WriteString(sio, elemPrefix);
      GWEN_SyncIo_WriteLine(sio, "_List2Iterator_free(it);");
  
      GWEN_SyncIo_WriteLine(sio, "    } /* if (it) */");
  
      GWEN_SyncIo_WriteLine(sio, "  } /* LIST */");
    }
  }

  return 0;
}



int write_code_duprec_c(ARGUMENTS *args, GWEN_XMLNODE *node,
                        GWEN_SYNCIO *sio) {
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
        rv=write_code_duprec_c(args, n, sio);
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
        int takeOver;

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
	takeOver=atoi(get_property(n, "takeOver", "0"));
        mode=GWEN_XMLNode_GetProperty(n, "mode", "single");
        if (strcasecmp(mode, "single")!=0)
          /* lists are always pointers */
          isPtr=1;

        if (isPtr) {
          GWEN_BUFFER *pbuf;
          const char *fname;

          fname=get_function_name(n, "dup");

#if 0
          /* this doesn't work very well: it should only delete the new element if there is no
           * element in the source object... */
          /* free old pointer if any */
          rv=write_code_freeElem_c(args, n, sio);
	  if (rv)
	    return rv;
#endif

          pbuf=GWEN_Buffer_new(0, 256, 0, 1);
          GWEN_Buffer_AppendString(pbuf, "d->");
          GWEN_Buffer_AppendByte(pbuf, tolower(*name));
          GWEN_Buffer_AppendString(pbuf, name+1);

          if (strcasecmp(mode, "single")!=0 && !fname) {
            rv=write_code_dupList_c(args, n, sio, GWEN_Buffer_GetStart(pbuf));
            GWEN_Buffer_free(pbuf);
            if (rv)
              return rv;
          }
          else {
            /* copy argument if any */
            GWEN_SyncIo_WriteString(sio, "  if (d->");
            GWEN_SyncIo_WriteChar(sio, tolower(*name));
            GWEN_SyncIo_WriteString(sio, name+1);
            GWEN_SyncIo_WriteLine(sio, ")");

            GWEN_SyncIo_WriteString(sio, "    st->");
            GWEN_SyncIo_WriteChar(sio, tolower(*name));
            GWEN_SyncIo_WriteString(sio, name+1);
            GWEN_SyncIo_WriteString(sio, "=");
	    if (doCopy || takeOver) {
              rv=write_code_dupArg_c(args, n, sio, GWEN_Buffer_GetStart(pbuf));
              GWEN_Buffer_free(pbuf);
              if (rv)
                return rv;
            }
            else {
              GWEN_SyncIo_WriteLine(sio, GWEN_Buffer_GetStart(pbuf));
              GWEN_Buffer_free(pbuf);
              GWEN_SyncIo_WriteString(sio, ";");
            }
          }
        }
        else {
          GWEN_SyncIo_WriteString(sio, "  st->");
          GWEN_SyncIo_WriteChar(sio, tolower(*name));
          GWEN_SyncIo_WriteString(sio, name+1);
          GWEN_SyncIo_WriteString(sio, "=d->");
          GWEN_SyncIo_WriteChar(sio, tolower(*name));
          GWEN_SyncIo_WriteString(sio, name+1);
          GWEN_SyncIo_WriteLine(sio, ";");
        }
      }
    }
    n=GWEN_XMLNode_GetNextTag(n);
  }
  return 0;
}



int write_code_dup_c(ARGUMENTS *args, GWEN_XMLNODE *node,
                     GWEN_SYNCIO *sio) {
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
    GWEN_SyncIo_WriteString(sio, styp);
    GWEN_SyncIo_WriteString(sio, " *");
    GWEN_SyncIo_WriteString(sio, prefix);
    if (dupName)
      GWEN_SyncIo_WriteString(sio, dupName);
    else
      GWEN_SyncIo_WriteString(sio, "_dup");
    GWEN_SyncIo_WriteString(sio, "(const ");
    GWEN_SyncIo_WriteString(sio, styp);
    GWEN_SyncIo_WriteLine(sio, " *d) {");

    GWEN_SyncIo_WriteString(sio, "  ");
    GWEN_SyncIo_WriteString(sio, styp);
    GWEN_SyncIo_WriteLine(sio, " *st;");
    GWEN_SyncIo_WriteLine(sio, "");

    GWEN_SyncIo_WriteLine(sio, "  assert(d);");

    GWEN_SyncIo_WriteString(sio, "  st=");
    GWEN_SyncIo_WriteString(sio, prefix);
    GWEN_SyncIo_WriteLine(sio, "_new();");

    rv=write_code_duprec_c(args, node, sio);
    if (rv) {
      DBG_ERROR(0, "Error in dup");
      return rv;
    }
    GWEN_SyncIo_WriteLine(sio, "  return st;");
    GWEN_SyncIo_WriteLine(sio, "}");
  }
  return 0;
}



int write_code_todbrec_c(ARGUMENTS *args, GWEN_XMLNODE *node,
                         GWEN_SYNCIO *sio) {
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
        rv=write_code_todbrec_c(args, n, sio);
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
	    GWEN_SyncIo_WriteString(sio, "  if (st->");
	    GWEN_SyncIo_WriteChar(sio, tolower(*name));
	    GWEN_SyncIo_WriteString(sio, name+1);
	    GWEN_SyncIo_WriteLine(sio, ")");
	  }
  
	  rv=write_code_todbArg_c(args, n, sio);
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
                      GWEN_SYNCIO *sio) {
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

  GWEN_SyncIo_WriteString(sio, "int ");
  GWEN_SyncIo_WriteString(sio, prefix);
  GWEN_SyncIo_WriteString(sio, "_toDb(const ");
  GWEN_SyncIo_WriteString(sio, styp);
  GWEN_SyncIo_WriteLine(sio, " *st, GWEN_DB_NODE *db) {");


  GWEN_SyncIo_WriteLine(sio, "  assert(st);");
  GWEN_SyncIo_WriteLine(sio, "  assert(db);");

  rv=write_code_todbrec_c(args, node, sio);
  if (rv) {
    DBG_ERROR(0, "Error in todb");
    return rv;
  }
  GWEN_SyncIo_WriteLine(sio, "  return 0;");
  GWEN_SyncIo_WriteLine(sio, "}");
  return 0;
}



int write_code_fromdbrec_c(ARGUMENTS *args, GWEN_XMLNODE *node,
                           GWEN_SYNCIO *sio) {
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
        rv=write_code_fromdbrec_c(args, n, sio);
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
	    GWEN_SyncIo_WriteString(sio, "  st->");
	    GWEN_SyncIo_WriteChar(sio, tolower(*name));
	    GWEN_SyncIo_WriteString(sio, name+1);
	    GWEN_SyncIo_WriteString(sio, "=");
	    GWEN_SyncIo_WriteString(sio, elemPrefix);
	    GWEN_SyncIo_WriteLine(sio, "_List_new();");
  
            GWEN_SyncIo_WriteLine(sio,
                                      "  if (1) {/* just for local vars */");
	    GWEN_SyncIo_WriteLine(sio, "    GWEN_DB_NODE *dbT;");
	    GWEN_SyncIo_WriteString(sio, "    ");
	    GWEN_SyncIo_WriteString(sio, elemType);
	    GWEN_SyncIo_WriteLine(sio, " *e;");
	    GWEN_SyncIo_WriteLine(sio, "");
	    GWEN_SyncIo_WriteString(sio,
				  "    dbT=GWEN_DB_GetGroup(db, "
				  "GWEN_PATH_FLAGS_NAMEMUSTEXIST, \"");
	    GWEN_SyncIo_WriteChar(sio, tolower(*name));
	    GWEN_SyncIo_WriteString(sio, name+1);
	    GWEN_SyncIo_WriteLine(sio, "\");");
	    GWEN_SyncIo_WriteLine(sio, "    if (dbT) {");
	    GWEN_SyncIo_WriteLine(sio, "      GWEN_DB_NODE *dbT2;");
	    GWEN_SyncIo_WriteLine(sio, "");
    
	    GWEN_SyncIo_WriteString(sio,
				  "      dbT2=GWEN_DB_FindFirstGroup(dbT, \"");
	    GWEN_SyncIo_WriteString(sio, "element");
	    GWEN_SyncIo_WriteLine(sio, "\");");
    
	    /* while (e) */
	    GWEN_SyncIo_WriteLine(sio, "      while(dbT2) {");
    
	    /* e=ElemType_fromDb(e) */
	    GWEN_SyncIo_WriteString(sio, "        e=");
	    GWEN_SyncIo_WriteString(sio, elemPrefix);
	    GWEN_SyncIo_WriteLine(sio, "_fromDb(dbT2);");
    
	    /* if (!e) */
	    GWEN_SyncIo_WriteLine(sio, "        if (!e) {");
	    GWEN_SyncIo_WriteString(sio, "          "
				  "DBG_ERROR(0, \"Bad element for type \\\"");
	    GWEN_SyncIo_WriteString(sio, elemType);
	    GWEN_SyncIo_WriteLine(sio, "\\\"\");");
	    GWEN_SyncIo_WriteLine(sio, "          "
				      "if (GWEN_Logger_GetLevel(0)>="
				      "GWEN_LoggerLevel_Debug)");
	    GWEN_SyncIo_WriteLine(sio, "            "
				      "GWEN_DB_Dump(dbT2, 2);");
	    GWEN_SyncIo_WriteString(sio, "          ");
	    GWEN_SyncIo_WriteString(sio, prefix);
	    GWEN_SyncIo_WriteLine(sio, "_free(st);");
	    GWEN_SyncIo_WriteLine(sio, "          return 0;");
	    GWEN_SyncIo_WriteLine(sio, "        }");
    
	    /* ElemType_List_Add(e, st->NAME); */
	    GWEN_SyncIo_WriteString(sio, "        ");
	    GWEN_SyncIo_WriteString(sio, elemPrefix);
	    GWEN_SyncIo_WriteString(sio, "_List_Add(e, st->");
	    GWEN_SyncIo_WriteChar(sio, tolower(*name));
	    GWEN_SyncIo_WriteString(sio, name+1);
	    GWEN_SyncIo_WriteString(sio, ");");
    
	    GWEN_SyncIo_WriteString(sio,
				  "    dbT2=GWEN_DB_FindNextGroup(dbT2, \"");
	    GWEN_SyncIo_WriteString(sio, "element");
	    GWEN_SyncIo_WriteLine(sio, "\");");
    
	    GWEN_SyncIo_WriteLine(sio, "      } /* while */");
    
	    GWEN_SyncIo_WriteLine(sio, "    } /* if (dbT) */");
    
	    GWEN_SyncIo_WriteLine(sio, "  } /* if (1) */");
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
	    GWEN_SyncIo_WriteString(sio, "  st->");
	    GWEN_SyncIo_WriteChar(sio, tolower(*name));
	    GWEN_SyncIo_WriteString(sio, name+1);
	    GWEN_SyncIo_WriteString(sio, "=");
	    GWEN_SyncIo_WriteString(sio, elemPrefix);
	    GWEN_SyncIo_WriteLine(sio, "_List2_new();");
  
	    GWEN_SyncIo_WriteLine(sio, "  if (1) {");
	    GWEN_SyncIo_WriteLine(sio, "    GWEN_DB_NODE *dbT;");
	    GWEN_SyncIo_WriteString(sio, "    ");
	    GWEN_SyncIo_WriteString(sio, elemType);
	    GWEN_SyncIo_WriteLine(sio, " *e;");
	    GWEN_SyncIo_WriteLine(sio, "");
	    GWEN_SyncIo_WriteString(sio,
				  "    dbT=GWEN_DB_GetGroup(db, "
				  "GWEN_PATH_FLAGS_NAMEMUSTEXIST, \"");
	    GWEN_SyncIo_WriteChar(sio, tolower(*name));
	    GWEN_SyncIo_WriteString(sio, name+1);
	    GWEN_SyncIo_WriteLine(sio, "\");");
	    GWEN_SyncIo_WriteLine(sio, "    if (dbT) {");
	    GWEN_SyncIo_WriteLine(sio, "      GWEN_DB_NODE *dbT2;");
	    GWEN_SyncIo_WriteLine(sio, "");
    
	    GWEN_SyncIo_WriteString(sio,
				  "    dbT2=GWEN_DB_FindFirstGroup(dbT, \"");
	    GWEN_SyncIo_WriteString(sio, "element");
	    GWEN_SyncIo_WriteLine(sio, "\");");
    
	    /* while (e) */
	    GWEN_SyncIo_WriteLine(sio, "      while(dbT2) {");
    
	    /* e=ElemType_fromDb(e) */
	    GWEN_SyncIo_WriteString(sio, "        e=");
	    GWEN_SyncIo_WriteString(sio, elemPrefix);
	    GWEN_SyncIo_WriteLine(sio, "_fromDb(dbT2);");
    
	    /* if (!e) */
	    GWEN_SyncIo_WriteLine(sio, "        if (!e) {");
	    GWEN_SyncIo_WriteString(sio, "          "
				  "DBG_ERROR(0, \"Bad element for type \\\"");
	    GWEN_SyncIo_WriteString(sio, elemType);
	    GWEN_SyncIo_WriteLine(sio, "\\\"\");");
	    GWEN_SyncIo_WriteLine(sio, "          "
				      "if (GWEN_Logger_GetLevel(0)>="
				      "GWEN_LoggerLevel_Debug)");
	    GWEN_SyncIo_WriteLine(sio, "            "
				      "GWEN_DB_Dump(dbT2, 2);");
	    GWEN_SyncIo_WriteString(sio, "          ");
	    GWEN_SyncIo_WriteString(sio, prefix);
	    GWEN_SyncIo_WriteLine(sio, "_free(st);");
	    GWEN_SyncIo_WriteLine(sio, "          return 0;");
	    GWEN_SyncIo_WriteLine(sio, "        } /* if !e */");
    
	    /* ElemType_List_Add(e, st->NAME); */
	    GWEN_SyncIo_WriteString(sio, "        ");
	    GWEN_SyncIo_WriteString(sio, elemPrefix);
	    GWEN_SyncIo_WriteString(sio, "_List2_PushBack(st->");
	    GWEN_SyncIo_WriteChar(sio, tolower(*name));
	    GWEN_SyncIo_WriteString(sio, name+1);
	    GWEN_SyncIo_WriteLine(sio, ", e);");
  
	    GWEN_SyncIo_WriteString(sio,"        "
				  "dbT2=GWEN_DB_FindNextGroup(dbT2, \"");
	    GWEN_SyncIo_WriteString(sio, "element");
	    GWEN_SyncIo_WriteLine(sio, "\");");
    
	    GWEN_SyncIo_WriteLine(sio, "      } /* while */");
    
	    GWEN_SyncIo_WriteLine(sio, "    } /* if (dbT) */");
    
	    GWEN_SyncIo_WriteLine(sio, "  } /* if (1) */");
	  }
	  else if (strcasecmp(typ, "GWEN_STRINGLIST")==0) {
	    GWEN_SyncIo_WriteLine(sio, "  if (1) {");
	    GWEN_SyncIo_WriteLine(sio, "    int i;");
	    GWEN_SyncIo_WriteLine(sio, "");
	    GWEN_SyncIo_WriteLine(sio, "    for (i=0; ; i++) {");
	    GWEN_SyncIo_WriteLine(sio, "      const char *s;");
	    GWEN_SyncIo_WriteLine(sio, "");
	    GWEN_SyncIo_WriteString(sio, "      s=GWEN_DB_GetCharValue(db, \"");
	    GWEN_SyncIo_WriteChar(sio, tolower(*name));
	    GWEN_SyncIo_WriteString(sio, name+1);
	    GWEN_SyncIo_WriteLine(sio, "\", i, 0);");
	    GWEN_SyncIo_WriteLine(sio, "      if (!s)");
	    GWEN_SyncIo_WriteLine(sio, "        break;");
	    GWEN_SyncIo_WriteString(sio, "      ");
	    GWEN_SyncIo_WriteString(sio, prefix);
	    GWEN_SyncIo_WriteString(sio, "_Add");
	    GWEN_SyncIo_WriteChar(sio, toupper(*name));
	    GWEN_SyncIo_WriteString(sio, name+1);
	    GWEN_SyncIo_WriteLine(sio, "(st, s, 0);");
	    GWEN_SyncIo_WriteLine(sio, "    } /* for */");
	    GWEN_SyncIo_WriteLine(sio, "  }");
	  }
	  else {
	    isPtr=atoi(get_property(n, "ptr", "0"));
  
	    if (isPtr) {
              if (strcasecmp(typ, "char")!=0) {
                GWEN_SyncIo_WriteLine(sio,
                                          "  if (1) { /* for local vars */");
		GWEN_SyncIo_WriteLine(sio, "    GWEN_DB_NODE *dbT;");
		GWEN_SyncIo_WriteLine(sio, "");
		GWEN_SyncIo_WriteString(sio,
				      "    dbT=GWEN_DB_GetGroup(db, "
				      "GWEN_PATH_FLAGS_NAMEMUSTEXIST, \"");
		GWEN_SyncIo_WriteChar(sio, tolower(*name));
		GWEN_SyncIo_WriteString(sio, name+1);
		GWEN_SyncIo_WriteLine(sio, "\");");
		GWEN_SyncIo_WriteString(sio, "    if (dbT)");
	      }
	    }
            if (isPtr && strcasecmp(typ, "char")!=0) {
              GWEN_SyncIo_WriteLine(sio, " {");
              rv=write_code_freeElem_c(args, n, sio);
              if (rv)
                return rv;
              GWEN_SyncIo_WriteString(sio, "  st->");
	      GWEN_SyncIo_WriteString(sio, name);
	      GWEN_SyncIo_WriteString(sio, "=");
	      rv=write_code_fromdbArg_c(args, n, sio);
	      if (rv)
		return rv;
	      GWEN_SyncIo_WriteLine(sio, ";");
              GWEN_SyncIo_WriteLine(sio, "}");
	    }
	    else {
	      GWEN_SyncIo_WriteString(sio, "  ");
	      GWEN_SyncIo_WriteString(sio, prefix);
	      GWEN_SyncIo_WriteString(sio, "_Set");
	      GWEN_SyncIo_WriteChar(sio, toupper(*name));
	      GWEN_SyncIo_WriteString(sio, name+1);
	      GWEN_SyncIo_WriteString(sio, "(st, ");
  
	      rv=write_code_fromdbArg_c(args, n, sio);
	      if (rv)
		return rv;
	      GWEN_SyncIo_WriteLine(sio, ");");
	    }
  
	    if (isPtr && strcasecmp(typ, "char")!=0) {
	      GWEN_SyncIo_WriteLine(sio, "  }");
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
                        GWEN_SYNCIO *sio) {
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

  GWEN_SyncIo_WriteString(sio, "int ");
  GWEN_SyncIo_WriteString(sio, prefix);
  GWEN_SyncIo_WriteString(sio, "_ReadDb(");
  GWEN_SyncIo_WriteString(sio, styp);
  GWEN_SyncIo_WriteLine(sio, " *st, GWEN_DB_NODE *db) {");

  GWEN_SyncIo_WriteLine(sio, "  assert(st);");
  GWEN_SyncIo_WriteLine(sio, "  assert(db);");

  rv=write_code_fromdbrec_c(args, node, sio);
  if (rv) {
    DBG_ERROR(0, "Error in fromdb");
    return rv;
  }

  GWEN_SyncIo_WriteLine(sio, "  return 0;");
  GWEN_SyncIo_WriteLine(sio, "}");
  return 0;
}



int write_code_fromdb_c(ARGUMENTS *args, GWEN_XMLNODE *node,
                        GWEN_SYNCIO *sio) {
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
    GWEN_SyncIo_WriteString(sio, styp);
    GWEN_SyncIo_WriteString(sio, " *");
    GWEN_SyncIo_WriteString(sio, prefix);
    if (fromDbName)
      GWEN_SyncIo_WriteString(sio, fromDbName);
    else
      GWEN_SyncIo_WriteString(sio, "_fromDb");
    GWEN_SyncIo_WriteLine(sio, "(GWEN_DB_NODE *db) {");
  
    GWEN_SyncIo_WriteString(sio, "  ");
    GWEN_SyncIo_WriteString(sio, styp);
    GWEN_SyncIo_WriteLine(sio, " *st;");
    GWEN_SyncIo_WriteLine(sio, "");
    GWEN_SyncIo_WriteLine(sio, "  assert(db);");
    GWEN_SyncIo_WriteString(sio, "  st=");
    GWEN_SyncIo_WriteString(sio, prefix);
    GWEN_SyncIo_WriteLine(sio, "_new();");
  
    GWEN_SyncIo_WriteString(sio, "  ");
    GWEN_SyncIo_WriteString(sio, prefix);
    GWEN_SyncIo_WriteLine(sio, "_ReadDb(st, db);");
  
    GWEN_SyncIo_WriteLine(sio, "  st->_modified=0;");
    GWEN_SyncIo_WriteLine(sio, "  return st;");
    GWEN_SyncIo_WriteLine(sio, "}");
  } /* if fromDb wanted */
  return 0;
}



int write_code_builtin_c(ARGUMENTS *args, GWEN_XMLNODE *node,
                         GWEN_SYNCIO *sio) {
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
  GWEN_SyncIo_WriteString(sio, "int ");
  GWEN_SyncIo_WriteString(sio, prefix);
  GWEN_SyncIo_WriteString(sio, "_IsModified(const ");
  GWEN_SyncIo_WriteString(sio, styp);
  GWEN_SyncIo_WriteLine(sio, " *st) {");
  GWEN_SyncIo_WriteLine(sio, "  assert(st);");
  GWEN_SyncIo_WriteLine(sio, "  return st->_modified;");
  GWEN_SyncIo_WriteLine(sio, "}");

  GWEN_SyncIo_WriteLine(sio, "");
  GWEN_SyncIo_WriteLine(sio, "");

  /* SetModified */
  GWEN_SyncIo_WriteString(sio, "void ");
  GWEN_SyncIo_WriteString(sio, prefix);
  GWEN_SyncIo_WriteString(sio, "_SetModified(");
  GWEN_SyncIo_WriteString(sio, styp);
  GWEN_SyncIo_WriteLine(sio, " *st, int i) {");
  GWEN_SyncIo_WriteLine(sio, "  assert(st);");
  GWEN_SyncIo_WriteLine(sio, "  st->_modified=i;");
  GWEN_SyncIo_WriteLine(sio, "}");

  GWEN_SyncIo_WriteLine(sio, "");
  GWEN_SyncIo_WriteLine(sio, "");

  /* Attach */
  GWEN_SyncIo_WriteString(sio, "void ");
  GWEN_SyncIo_WriteString(sio, prefix);
  GWEN_SyncIo_WriteString(sio, "_Attach(");
  GWEN_SyncIo_WriteString(sio, styp);
  GWEN_SyncIo_WriteLine(sio, " *st) {");
  GWEN_SyncIo_WriteLine(sio, "  assert(st);");
  GWEN_SyncIo_WriteLine(sio, "  st->_usage++;");
  GWEN_SyncIo_WriteLine(sio, "}");

  /* list2 functions */
  if (get_struct_property(node, "list2", 0)) {
    /* List2_freeAll */
    GWEN_SyncIo_WriteString(sio, styp);
    GWEN_SyncIo_WriteString(sio, " *");
    GWEN_SyncIo_WriteString(sio, prefix);
    GWEN_SyncIo_WriteString(sio, "_List2__freeAll_cb(");
    GWEN_SyncIo_WriteString(sio, styp);
    GWEN_SyncIo_WriteString(sio, " *");
    GWEN_SyncIo_WriteLine(sio, "st, void *user_data) {");

    GWEN_SyncIo_WriteString(sio, "  ");
    GWEN_SyncIo_WriteString(sio, prefix);
    GWEN_SyncIo_WriteLine(sio, "_free(st);");
    GWEN_SyncIo_WriteLine(sio, "return 0;");

    GWEN_SyncIo_WriteLine(sio, "}");
    GWEN_SyncIo_WriteLine(sio, "");
    GWEN_SyncIo_WriteLine(sio, "");

    GWEN_SyncIo_WriteString(sio, "void ");
    GWEN_SyncIo_WriteString(sio, prefix);
    GWEN_SyncIo_WriteString(sio, "_List2_freeAll(");
    GWEN_SyncIo_WriteString(sio, styp);
    GWEN_SyncIo_WriteLine(sio, "_LIST2 *stl) {");

    GWEN_SyncIo_WriteLine(sio, "  if (stl) {");

    GWEN_SyncIo_WriteString(sio, "    ");
    GWEN_SyncIo_WriteString(sio, prefix);
    GWEN_SyncIo_WriteString(sio, "_List2_ForEach(stl, ");
    GWEN_SyncIo_WriteString(sio, prefix);
    GWEN_SyncIo_WriteLine(sio, "_List2__freeAll_cb, 0);");

    GWEN_SyncIo_WriteString(sio, "    ");
    GWEN_SyncIo_WriteString(sio, prefix);
    GWEN_SyncIo_WriteLine(sio, "_List2_free(stl); ");

    GWEN_SyncIo_WriteLine(sio, "  }");
    GWEN_SyncIo_WriteLine(sio, "}");
    GWEN_SyncIo_WriteLine(sio, "");
    GWEN_SyncIo_WriteLine(sio, "");
  }
  /* list functions */
  if (get_struct_property(node, "list", 0)) {
    /* LIST_dup functions */
    if (dupAcc && strcasecmp(dupAcc, "none")!=0) {
      const char *dupName;

      dupName=get_struct_property(node, "dup-name", 0);
      GWEN_SyncIo_WriteString(sio, styp);
      GWEN_SyncIo_WriteString(sio, "_LIST *");

      GWEN_SyncIo_WriteString(sio, prefix);
      GWEN_SyncIo_WriteString(sio, "_List_dup(const ");
      GWEN_SyncIo_WriteString(sio, styp);
      GWEN_SyncIo_WriteLine(sio, "_LIST *stl) {");

      GWEN_SyncIo_WriteLine(sio, "  if (stl) {");

      /* ELEMTYPE_LIST *nl; */
      GWEN_SyncIo_WriteString(sio, "    ");
      GWEN_SyncIo_WriteString(sio, styp);
      GWEN_SyncIo_WriteLine(sio, "_LIST *nl;");

      /* ELEMTYPE *e; */
      GWEN_SyncIo_WriteString(sio, "    ");
      GWEN_SyncIo_WriteString(sio, styp);
      GWEN_SyncIo_WriteLine(sio, " *e;");
      GWEN_SyncIo_WriteLine(sio, "");

      /* nl=ElemType_List */
      GWEN_SyncIo_WriteString(sio, "    nl=");
      GWEN_SyncIo_WriteString(sio, prefix);
      GWEN_SyncIo_WriteLine(sio, "_List_new();");

      /* e=ElemType_List_First */
      GWEN_SyncIo_WriteString(sio, "    e=");
      GWEN_SyncIo_WriteString(sio, prefix);
      GWEN_SyncIo_WriteLine(sio, "_List_First(stl);");

      /* while (e) ; */
      GWEN_SyncIo_WriteLine(sio, "    while(e) {");

      /* ELEMTYPE *ne; */
      GWEN_SyncIo_WriteString(sio, "      ");
      GWEN_SyncIo_WriteString(sio, styp);
      GWEN_SyncIo_WriteLine(sio, " *ne;");
      GWEN_SyncIo_WriteLine(sio, "");

      /* ne=ElemType_dup; assert(ne); */
      GWEN_SyncIo_WriteString(sio, "      ne=");
      GWEN_SyncIo_WriteString(sio, prefix);
      if (dupName)
        GWEN_SyncIo_WriteString(sio, dupName);
      else
        GWEN_SyncIo_WriteString(sio, "_dup");
      GWEN_SyncIo_WriteLine(sio, "(e);");
      GWEN_SyncIo_WriteLine(sio, "      assert(ne);");

      /* ElemType_List_Add(ne, st->NAME); */
      GWEN_SyncIo_WriteString(sio, "      ");
      GWEN_SyncIo_WriteString(sio, prefix);
      GWEN_SyncIo_WriteLine(sio, "_List_Add(ne, nl);");

      /* e=ElemType_List_Next */
      GWEN_SyncIo_WriteString(sio, "      e=");
      GWEN_SyncIo_WriteString(sio, prefix);
      GWEN_SyncIo_WriteLine(sio, "_List_Next(e);");

      GWEN_SyncIo_WriteLine(sio, "    } /* while (e) */");


      GWEN_SyncIo_WriteLine(sio, "    return nl;");

      GWEN_SyncIo_WriteLine(sio, "  }");
      GWEN_SyncIo_WriteLine(sio, "  else");
      GWEN_SyncIo_WriteLine(sio, "    return 0;");
      GWEN_SyncIo_WriteLine(sio, "}");
      GWEN_SyncIo_WriteLine(sio, "");
      GWEN_SyncIo_WriteLine(sio, "");
    } /* if we have a dup function */

  }



  return 0;
}




int write_code_file_c(ARGUMENTS *args, GWEN_XMLNODE *node) {
  int rv;
  const char *f;
  GWEN_BUFFER *fname;
  const char *nacc;
  GWEN_SYNCIO *sio;
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

  sio=GWEN_SyncIo_File_new(GWEN_Buffer_GetStart(fname),
			   GWEN_SyncIo_File_CreationMode_CreateAlways);
  GWEN_SyncIo_AddFlags(sio,
		       GWEN_SYNCIO_FILE_FLAGS_READ |
		       GWEN_SYNCIO_FILE_FLAGS_WRITE |
		       GWEN_SYNCIO_FILE_FLAGS_UREAD |
		       GWEN_SYNCIO_FILE_FLAGS_UWRITE |
		       GWEN_SYNCIO_FILE_FLAGS_GREAD |
		       GWEN_SYNCIO_FILE_FLAGS_GWRITE);
  rv=GWEN_SyncIo_Connect(sio);
  if (rv<0) {
    DBG_ERROR(0, "open(%s): %s",
	      GWEN_Buffer_GetStart(fname),
	      strerror(errno));
    GWEN_Buffer_free(fname);
    GWEN_SyncIo_free(sio);
    return -1;
  }
  GWEN_Buffer_free(fname);

  /* Insert the auto-generation warning */
  GWEN_SyncIo_WriteString(sio, "/* This file is auto-generated from \"");
  GWEN_SyncIo_WriteString(sio, f);
  GWEN_SyncIo_WriteLine(sio, ".xml\" by the typemaker");
  GWEN_SyncIo_WriteLine(sio, "   tool of Gwenhywfar. ");
  GWEN_SyncIo_WriteLine(sio, "   Do not edit this file -- all changes will be lost! */");

  GWEN_SyncIo_WriteLine(sio, "#ifdef HAVE_CONFIG_H");
  GWEN_SyncIo_WriteLine(sio, "# include \"config.h\"");
  GWEN_SyncIo_WriteLine(sio, "#endif");
  GWEN_SyncIo_WriteLine(sio, "");

  fname=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(fname, f);
  GWEN_Buffer_AppendString(fname, "_p.h");

  GWEN_SyncIo_WriteString(sio, "#include \"");
  GWEN_SyncIo_WriteString(sio, GWEN_Buffer_GetStart(fname));
  GWEN_SyncIo_WriteLine(sio, "\"");
  GWEN_Buffer_free(fname);

  GWEN_SyncIo_WriteLine(sio, "#include <gwenhywfar/misc.h>");
  GWEN_SyncIo_WriteLine(sio, "#include <gwenhywfar/db.h>");
  GWEN_SyncIo_WriteLine(sio, "#include <gwenhywfar/debug.h>");
  GWEN_SyncIo_WriteLine(sio, "#include <assert.h>");
  GWEN_SyncIo_WriteLine(sio, "#include <stdlib.h>");
  GWEN_SyncIo_WriteLine(sio, "#include <strings.h>");
  GWEN_SyncIo_WriteLine(sio, "");

  /* write headers */
  n=GWEN_XMLNode_FindFirstTag(node, "headers", 0, 0);
  if (n) {
    n=GWEN_XMLNode_FindFirstTag(n, "header", 0, 0);
    while(n) {
      write_h_header(args, n, sio, "source");
      n=GWEN_XMLNode_FindNextTag(n, "header", 0, 0);
    }
    GWEN_SyncIo_WriteLine(sio, "");
    GWEN_SyncIo_WriteLine(sio, "");
  }

  /* write c-headers */
  n=GWEN_XMLNode_FindFirstTag(node, "c-headers", 0, 0);
  if (n) {
    n=GWEN_XMLNode_FindFirstTag(n, "header", 0, 0);
    while(n) {
      write_h_header(args, n, sio, "source");
      n=GWEN_XMLNode_FindNextTag(n, "header", 0, 0);
    }
    GWEN_SyncIo_WriteLine(sio, "");
    GWEN_SyncIo_WriteLine(sio, "");
  }

  if (get_struct_property(node, "inherit", 0)) {
    GWEN_SyncIo_WriteString(sio, "GWEN_INHERIT_FUNCTIONS(");
    GWEN_SyncIo_WriteString(sio, id);
    GWEN_SyncIo_WriteLine(sio, ")");
  }

  if (get_struct_property(node, "list", 0)) {
    GWEN_SyncIo_WriteString(sio, "GWEN_LIST_FUNCTIONS(");
    GWEN_SyncIo_WriteString(sio, id);
    GWEN_SyncIo_WriteString(sio, ", ");
    GWEN_SyncIo_WriteString(sio, prefix);
    GWEN_SyncIo_WriteLine(sio, ")");
  }

  if (get_struct_property(node, "list2", 0)) {
    GWEN_SyncIo_WriteString(sio, "GWEN_LIST2_FUNCTIONS(");
    GWEN_SyncIo_WriteString(sio, id);
    GWEN_SyncIo_WriteString(sio, ", ");
    GWEN_SyncIo_WriteString(sio, prefix);
    GWEN_SyncIo_WriteLine(sio, ")");
  }
  GWEN_SyncIo_WriteLine(sio, "");
  GWEN_SyncIo_WriteLine(sio, "");

  rv=write_c_enums(args, node, sio);
  if (rv) {
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    return rv;
  }
  GWEN_SyncIo_WriteLine(sio, "");
  GWEN_SyncIo_WriteLine(sio, "");

  rv=write_code_const_c(args, node, sio);
  if (rv) {
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    return rv;
  }
  GWEN_SyncIo_WriteLine(sio, "");
  GWEN_SyncIo_WriteLine(sio, "");

  rv=write_code_dest_c(args, node, sio);
  if (rv) {
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    return rv;
  }
  GWEN_SyncIo_WriteLine(sio, "");
  GWEN_SyncIo_WriteLine(sio, "");

  rv=write_code_dup_c(args, node, sio);
  if (rv) {
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    return rv;
  }
  GWEN_SyncIo_WriteLine(sio, "");
  GWEN_SyncIo_WriteLine(sio, "");

  rv=write_code_todb_c(args, node, sio);
  if (rv) {
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    return rv;
  }
  GWEN_SyncIo_WriteLine(sio, "");
  GWEN_SyncIo_WriteLine(sio, "");

  rv=write_code_readdb_c(args, node, sio);
  if (rv) {
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    return rv;
  }
  GWEN_SyncIo_WriteLine(sio, "");
  GWEN_SyncIo_WriteLine(sio, "");

  rv=write_code_fromdb_c(args, node, sio);
  if (rv) {
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    return rv;
  }
  GWEN_SyncIo_WriteLine(sio, "");
  GWEN_SyncIo_WriteLine(sio, "");

  rv=write_code_setget_c(args, node, sio);
  if (rv) {
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    return rv;
  }
  GWEN_SyncIo_WriteLine(sio, "");
  GWEN_SyncIo_WriteLine(sio, "");

  rv=write_code_builtin_c(args, node, sio);
  if (rv) {
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    return rv;
  }
  GWEN_SyncIo_WriteLine(sio, "");
  GWEN_SyncIo_WriteLine(sio, "");

  /* close stream */
  err=GWEN_SyncIo_Disconnect(sio);
  if (err) {
    DBG_ERROR_ERR(0, err);
    GWEN_SyncIo_free(sio);
    return -1;
  }

  GWEN_SyncIo_free(sio);
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








