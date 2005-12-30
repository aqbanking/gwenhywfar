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



int write_h_elem_c(ARGUMENTS *args, GWEN_XMLNODE *node,
		   GWEN_BUFFEREDIO *bio) {
  const char *name;
  const char *typ;
  const char *mode;
  GWEN_XMLNODE *tnode;
  const char *tmode;
  GWEN_ERRORCODE err;
  int isPtr;

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

  mode=GWEN_XMLNode_GetProperty(node, "mode", "single");

  tnode=get_typedef(node, typ);
  if (tnode)
    tmode=GWEN_XMLNode_GetProperty(tnode, "mode", "single");
  else
    tmode=mode;

  isPtr=atoi(get_property(node, "ptr", "0"));

  err=GWEN_BufferedIO_Write(bio, "  ");
  if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}

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

  GWEN_BufferedIO_Write(bio, " ");
  if (isPtr) {
    GWEN_BufferedIO_Write(bio, "*");
  }

  GWEN_BufferedIO_WriteChar(bio, tolower(*name));
  GWEN_BufferedIO_Write(bio, name+1);
  GWEN_BufferedIO_WriteLine(bio, ";");

  return 0;
}



int write_h_func_c(ARGUMENTS *args, GWEN_XMLNODE *node,
                   GWEN_BUFFEREDIO *bio) {
  const char *name;
  const char *typ;
  const char *styp;

  styp=get_struct_property(node, "id", 0);
  if (!styp) {
    DBG_ERROR(0, "No id in struct");
    return -1;
  }

  name=GWEN_XMLNode_GetProperty(node, "name", 0);
  if (!name) {
    DBG_ERROR(0, "No name for function");
    return -1;
  }

  typ=GWEN_XMLNode_GetProperty(node, "type", 0);
  if (!typ) {
    DBG_ERROR(0, "No type for function");
    return -1;
  }

  GWEN_BufferedIO_Write(bio, "  ");

  /* rettype name; */
  GWEN_BufferedIO_Write(bio, styp);
  GWEN_BufferedIO_Write(bio, "_");
  GWEN_BufferedIO_Write(bio, typ);
  GWEN_BufferedIO_Write(bio, " ");
  GWEN_BufferedIO_WriteChar(bio, tolower(*name));
  GWEN_BufferedIO_Write(bio, name+1);
  GWEN_BufferedIO_WriteLine(bio, ";");

  return 0;
}



int write_h_struct_c(ARGUMENTS *args, GWEN_XMLNODE *node,
		     GWEN_BUFFEREDIO *bio) {
  GWEN_XMLNODE *n;
  const char *id;
  const char *acc;
  const char *prefix;

  id=get_struct_property(node, "id", 0);
  if (!id) {
    DBG_ERROR(0, "No id for struct");
    return -1;
  }

  prefix=get_struct_property(node, "prefix", 0);
  if (!prefix) {
    DBG_ERROR(0, "No prefix for struct");
    return -1;
  }

  if (write_h_enums(args, node, bio, "private")) {
    DBG_ERROR(0, "Error writing enum types");
    return -1;
  }

  if (write_h_funcs(args, node, bio, "private")) {
    DBG_ERROR(0, "Error writing function types");
    return -1;
  }

  acc=get_struct_property(node, "access", "public");
  if (strcasecmp(acc, "private")==0) {
    GWEN_BufferedIO_Write(bio, "typedef struct ");
    GWEN_BufferedIO_Write(bio, id);
    GWEN_BufferedIO_Write(bio, " ");
    GWEN_BufferedIO_Write(bio, id);
    GWEN_BufferedIO_WriteLine(bio, ";");
  }
  GWEN_BufferedIO_Write(bio, "struct ");
  GWEN_BufferedIO_Write(bio, id);
  GWEN_BufferedIO_WriteLine(bio, " {");

  if (get_struct_property(node, "inherit", 0)) {
    GWEN_BufferedIO_Write(bio, "  GWEN_INHERIT_ELEMENT(");
    GWEN_BufferedIO_Write(bio, id);
    GWEN_BufferedIO_WriteLine(bio, ")");
  }

  if (get_struct_property(node, "list", 0)) {
    GWEN_BufferedIO_Write(bio, "  GWEN_LIST_ELEMENT(");
    GWEN_BufferedIO_Write(bio, id);
    GWEN_BufferedIO_WriteLine(bio, ")");
  }

  GWEN_BufferedIO_WriteLine(bio, "  int _usage;");
  GWEN_BufferedIO_WriteLine(bio, "  int _modified;");

  GWEN_BufferedIO_WriteLine(bio, "");

  n=GWEN_XMLNode_GetFirstTag(node);
  while(n) {
    int rv;

    rv=write_hp_group_c(args, n, bio);
    if (rv)
      return rv;
    n=GWEN_XMLNode_GetNextTag(n);
  } /* while */

  GWEN_BufferedIO_WriteLine(bio, "};");

  return 0;
}



int write_hp_group_c(ARGUMENTS *args, GWEN_XMLNODE *node,
                     GWEN_BUFFEREDIO *bio) {
  int rv;

  if (GWEN_XMLNode_GetType(node)==GWEN_XMLNodeTypeTag) {
    if (strcasecmp(GWEN_XMLNode_GetData(node), "type")==0)
      return write_h_struct_c(args, node, bio);
    else if (strcasecmp(GWEN_XMLNode_GetData(node), "elem")==0) {
      return write_h_elem_c(args, node, bio);
    }
    else if (strcasecmp(GWEN_XMLNode_GetData(node), "func")==0) {
      return write_h_func_c(args, node, bio);
    }
    else if (strcasecmp(GWEN_XMLNode_GetData(node), "header")==0) {
      return write_h_header(args, node, bio, "header");
    }
    else if (strcasecmp(GWEN_XMLNode_GetData(node), "group")==0) {
      GWEN_XMLNODE *n;

      n=GWEN_XMLNode_GetFirstTag(node);
      while(n) {
        rv=write_hp_group_c(args, n, bio);
        if (rv)
          return rv;
        n=GWEN_XMLNode_GetNextTag(n);
      } /* while */
    } /* if group */

  }
  return 0;
}



int write_hp_file_c(ARGUMENTS *args, GWEN_XMLNODE *node) {
  int rv;
  const char *f;
  GWEN_BUFFER *fname;
  GWEN_BUFFER *hbuf;
  const char *s;
  int fd;
  GWEN_BUFFEREDIO *bio;
  GWEN_ERRORCODE err;
  const char *nacc;
  const char *constAcc;
  const char *constName;
  const char *fromDbAcc;
  const char *fromDbName;
  const char *dupAcc;
  const char *dupName;
  const char *id;
  const char *prefix;

  id=get_struct_property(node, "id", 0);
  if (!id) {
    DBG_ERROR(0, "No id for struct");
    return -1;
  }

  prefix=get_struct_property(node, "prefix", 0);
  if (!prefix) {
    DBG_ERROR(0, "No prefix for struct");
    return -1;
  }

  f=get_struct_property(node, "filename", 0);
  if (!f) {
    DBG_ERROR(0, "No filename given");
    return -1;
  }

  nacc=get_struct_property(node, "access", "public");
  constAcc=get_struct_property(node, "constructor-access", nacc);
  constName=get_struct_property(node, "constructor-name", 0);
  fromDbAcc=get_struct_property(node, "fromdb-access", nacc);
  fromDbName=get_struct_property(node, "fromdb-name", 0);
  dupAcc=get_struct_property(node, "dup-access", nacc);
  dupName=get_struct_property(node, "fromdb-name", 0);

  fname=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(fname, f);
  GWEN_Buffer_AppendString(fname, "_p.h");
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

  hbuf=GWEN_Buffer_new(0, 256, 0, 1);
  s=f;
  while(*s) {
    GWEN_Buffer_AppendByte(hbuf, toupper(*s));
    s++;
  }
  GWEN_Buffer_AppendString(hbuf, "_P_H");

  GWEN_BufferedIO_Write(bio, "#ifndef ");
  GWEN_BufferedIO_WriteLine(bio, GWEN_Buffer_GetStart(hbuf));
  GWEN_BufferedIO_Write(bio, "#define ");
  GWEN_BufferedIO_WriteLine(bio, GWEN_Buffer_GetStart(hbuf));
  GWEN_BufferedIO_WriteLine(bio, "");

  if (strcasecmp(get_struct_property(node, "inherit", ""),
                 "private")==0) {
    GWEN_BufferedIO_WriteLine(bio, "#include <gwenhywfar/inherit.h>");
  }

  if (strcasecmp(get_struct_property(node, "list", ""),
                 "private")==0) {
    GWEN_BufferedIO_WriteLine(bio, "#include <gwenhywfar/misc.h>");
  }

  if (strcasecmp(get_struct_property(node, "list2", ""),
                 "private")==0) {
    GWEN_BufferedIO_WriteLine(bio, "#include <gwenhywfar/list2.h>");
  }

  fname=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(fname, f);
  GWEN_Buffer_AppendString(fname, "_l.h");
  GWEN_BufferedIO_Write(bio, "#include \"");
  GWEN_BufferedIO_Write(bio, GWEN_Buffer_GetStart(fname));
  GWEN_BufferedIO_WriteLine(bio, "\"");
  GWEN_Buffer_free(fname);

  GWEN_BufferedIO_WriteLine(bio, "");

  rv=write_h_struct_c(args, node, bio);
  if (rv) {
    GWEN_Buffer_free(hbuf);
    GWEN_BufferedIO_Abandon(bio);
    GWEN_BufferedIO_free(bio);
    return rv;
  }

  GWEN_BufferedIO_WriteLine(bio, "");

  /* include private functions */
  if (strcasecmp(constAcc, "private")==0) {
    GWEN_BufferedIO_Write(bio, id);
    GWEN_BufferedIO_Write(bio, " *");
    GWEN_BufferedIO_Write(bio, prefix);
    if (constName)
      GWEN_BufferedIO_Write(bio, constName);
    else
      GWEN_BufferedIO_Write(bio, "_new");
    GWEN_BufferedIO_WriteLine(bio, "();");
  }

  /* FromDb */
  if (strcasecmp(fromDbAcc, "private")==0) {
    GWEN_BufferedIO_Write(bio, id);
    GWEN_BufferedIO_Write(bio, " *");
    GWEN_BufferedIO_Write(bio, prefix);
    if (fromDbName)
      GWEN_BufferedIO_Write(bio, fromDbName);
    else
      GWEN_BufferedIO_Write(bio, "_fromDb");
    GWEN_BufferedIO_WriteLine(bio, "(GWEN_DB_NODE *db);");
  }

  /* dup */
  if (strcasecmp(dupAcc, "private")==0) {
    GWEN_BufferedIO_Write(bio, id);
    GWEN_BufferedIO_Write(bio, " *");
    GWEN_BufferedIO_Write(bio, prefix);
    if (dupName)
      GWEN_BufferedIO_Write(bio, dupName);
    else
      GWEN_BufferedIO_Write(bio, "_dup");
    GWEN_BufferedIO_Write(bio, "(const ");
    GWEN_BufferedIO_Write(bio, id);
    GWEN_BufferedIO_WriteLine(bio, "*st);");
  }

  if (strcasecmp(nacc, "private")==0) {
    GWEN_BufferedIO_Write(bio, "void ");
    GWEN_BufferedIO_Write(bio, prefix);
    GWEN_BufferedIO_Write(bio, "_free(");
    GWEN_BufferedIO_Write(bio, id);
    GWEN_BufferedIO_WriteLine(bio, " *st);");

    GWEN_BufferedIO_Write(bio, "void ");
    GWEN_BufferedIO_Write(bio, prefix);
    GWEN_BufferedIO_Write(bio, "_Attach(");
    GWEN_BufferedIO_Write(bio, id);
    GWEN_BufferedIO_WriteLine(bio, " *st);");

    /* ReadDb */
    GWEN_BufferedIO_Write(bio, "int ");
    GWEN_BufferedIO_Write(bio, prefix);
    GWEN_BufferedIO_Write(bio, "_ReadDb(");
    GWEN_BufferedIO_Write(bio, id);
    GWEN_BufferedIO_WriteLine(bio, " *st, GWEN_DB_NODE *db);");

    /* ToDb */
    GWEN_BufferedIO_Write(bio, "int ");
    GWEN_BufferedIO_Write(bio, prefix);
    GWEN_BufferedIO_Write(bio, "_toDb(const ");
    GWEN_BufferedIO_Write(bio, id);
    GWEN_BufferedIO_Write(bio, "*st, GWEN_DB_NODE *db);");

    GWEN_BufferedIO_Write(bio, "int ");
    GWEN_BufferedIO_Write(bio, prefix);
    GWEN_BufferedIO_Write(bio, "_IsModified(const ");
    GWEN_BufferedIO_Write(bio, id);
    GWEN_BufferedIO_WriteLine(bio, " *st);");

    GWEN_BufferedIO_Write(bio, "void ");
    GWEN_BufferedIO_Write(bio, prefix);
    GWEN_BufferedIO_Write(bio, "_SetModified(");
    GWEN_BufferedIO_Write(bio, id);
    GWEN_BufferedIO_WriteLine(bio, " *st, int i);");
  }

  rv=write_h_setget_c(args, node, bio, "private");
  if (rv) {
    GWEN_Buffer_free(hbuf);
    GWEN_BufferedIO_Abandon(bio);
    GWEN_BufferedIO_free(bio);
    return rv;
  }

  /* write trailing endif */
  err=GWEN_BufferedIO_WriteLine(bio, "");
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    GWEN_Buffer_free(hbuf);
    return -1;
  }

  err=GWEN_BufferedIO_Write(bio, "#endif /* ");
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    GWEN_Buffer_free(hbuf);
    return -1;
  }
  err=GWEN_BufferedIO_Write(bio, GWEN_Buffer_GetStart(hbuf));
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    GWEN_Buffer_free(hbuf);
    return -1;
  }
  err=GWEN_BufferedIO_WriteLine(bio, " */");
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    GWEN_Buffer_free(hbuf);
    return -1;
  }

  err=GWEN_BufferedIO_Close(bio);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    GWEN_BufferedIO_free(bio);
    GWEN_Buffer_free(hbuf);
    return -1;
  }

  GWEN_Buffer_free(hbuf);
  return 0;
}



int write_hp_files_c(ARGUMENTS *args, GWEN_XMLNODE *node) {
  GWEN_XMLNODE *n;
  int rv;

  n=GWEN_XMLNode_FindFirstTag(node, "type", 0, 0);
  while (n) {
    rv=write_hp_file_c(args, n);
    if (rv)
      return rv;
    n=GWEN_XMLNode_FindNextTag(n, "type", 0, 0);
  }
  return 0;
}








