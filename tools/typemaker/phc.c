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
#include <gwenhywfar/syncio_buffered.h>

#include <stdlib.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>



int write_h_elem_c(ARGUMENTS *args, GWEN_XMLNODE *node,
                   GWEN_SYNCIO *sio)
{
  const char *name;
  const char *typ;
  const char *mode;
  GWEN_XMLNODE *tnode;
  const char *tmode;
  int err;
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

  err=GWEN_SyncIo_WriteString(sio, "  ");
  if (err) {
    DBG_ERROR_ERR(0, err);
    return -1;
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

  GWEN_SyncIo_WriteString(sio, " ");
  if (isPtr) {
    GWEN_SyncIo_WriteString(sio, "*");
  }

  GWEN_SyncIo_WriteChar(sio, tolower(*name));
  GWEN_SyncIo_WriteString(sio, name+1);
  GWEN_SyncIo_WriteLine(sio, ";");

  return 0;
}



int write_h_func_c(ARGUMENTS *args, GWEN_XMLNODE *node,
                   GWEN_SYNCIO *sio)
{
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

  GWEN_SyncIo_WriteString(sio, "  ");

  /* rettype name; */
  GWEN_SyncIo_WriteString(sio, styp);
  GWEN_SyncIo_WriteString(sio, "_");
  GWEN_SyncIo_WriteString(sio, typ);
  GWEN_SyncIo_WriteString(sio, " ");
  GWEN_SyncIo_WriteChar(sio, tolower(*name));
  GWEN_SyncIo_WriteString(sio, name+1);
  GWEN_SyncIo_WriteLine(sio, ";");

  return 0;
}



int write_h_struct_c(ARGUMENTS *args, GWEN_XMLNODE *node,
                     GWEN_SYNCIO *sio)
{
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

  if (write_h_enums(args, node, sio, "private")) {
    DBG_ERROR(0, "Error writing enum types");
    return -1;
  }

  if (write_h_funcs(args, node, sio, "private")) {
    DBG_ERROR(0, "Error writing function types");
    return -1;
  }

  acc=get_struct_property(node, "access", "public");
  if (strcasecmp(acc, "private")==0) {
    GWEN_SyncIo_WriteString(sio, "typedef struct ");
    GWEN_SyncIo_WriteString(sio, id);
    GWEN_SyncIo_WriteString(sio, " ");
    GWEN_SyncIo_WriteString(sio, id);
    GWEN_SyncIo_WriteLine(sio, ";");
  }
  GWEN_SyncIo_WriteString(sio, "struct ");
  GWEN_SyncIo_WriteString(sio, id);
  GWEN_SyncIo_WriteLine(sio, " {");

  if (get_struct_property(node, "inherit", 0)) {
    GWEN_SyncIo_WriteString(sio, "  GWEN_INHERIT_ELEMENT(");
    GWEN_SyncIo_WriteString(sio, id);
    GWEN_SyncIo_WriteLine(sio, ")");
  }

  if (get_struct_property(node, "list", 0)) {
    GWEN_SyncIo_WriteString(sio, "  GWEN_LIST_ELEMENT(");
    GWEN_SyncIo_WriteString(sio, id);
    GWEN_SyncIo_WriteLine(sio, ")");
  }

  GWEN_SyncIo_WriteLine(sio, "  int _usage;");
  GWEN_SyncIo_WriteLine(sio, "  int _modified;");

  GWEN_SyncIo_WriteLine(sio, "");

  n=GWEN_XMLNode_GetFirstTag(node);
  while (n) {
    int rv;

    rv=write_hp_group_c(args, n, sio);
    if (rv)
      return rv;
    n=GWEN_XMLNode_GetNextTag(n);
  } /* while */

  GWEN_SyncIo_WriteLine(sio, "};");

  return 0;
}



int write_hp_group_c(ARGUMENTS *args, GWEN_XMLNODE *node,
                     GWEN_SYNCIO *sio)
{
  int rv;

  if (GWEN_XMLNode_GetType(node)==GWEN_XMLNodeTypeTag) {
    if (strcasecmp(GWEN_XMLNode_GetData(node), "type")==0)
      return write_h_struct_c(args, node, sio);
    else if (strcasecmp(GWEN_XMLNode_GetData(node), "elem")==0) {
      return write_h_elem_c(args, node, sio);
    }
    else if (strcasecmp(GWEN_XMLNode_GetData(node), "func")==0) {
      return write_h_func_c(args, node, sio);
    }
    else if (strcasecmp(GWEN_XMLNode_GetData(node), "header")==0) {
      return write_h_header(args, node, sio, "header");
    }
    else if (strcasecmp(GWEN_XMLNode_GetData(node), "group")==0) {
      GWEN_XMLNODE *n;

      n=GWEN_XMLNode_GetFirstTag(node);
      while (n) {
        rv=write_hp_group_c(args, n, sio);
        if (rv)
          return rv;
        n=GWEN_XMLNode_GetNextTag(n);
      } /* while */
    } /* if group */

  }
  return 0;
}



int write_hp_file_c(ARGUMENTS *args, GWEN_XMLNODE *node)
{
  int rv;
  const char *f;
  GWEN_BUFFER *fname;
  GWEN_BUFFER *hbuf;
  const char *s;
  GWEN_SYNCIO *sio;

  int err;
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

  hbuf=GWEN_Buffer_new(0, 256, 0, 1);
  s=f;
  while (*s) {
    GWEN_Buffer_AppendByte(hbuf, toupper(*s));
    s++;
  }
  GWEN_Buffer_AppendString(hbuf, "_P_H");

  GWEN_SyncIo_WriteString(sio, "#ifndef ");
  GWEN_SyncIo_WriteLine(sio, GWEN_Buffer_GetStart(hbuf));
  GWEN_SyncIo_WriteString(sio, "#define ");
  GWEN_SyncIo_WriteLine(sio, GWEN_Buffer_GetStart(hbuf));
  GWEN_SyncIo_WriteLine(sio, "");

  if (strcasecmp(get_struct_property(node, "inherit", ""),
                 "private")==0) {
    GWEN_SyncIo_WriteLine(sio, "#include <gwenhywfar/inherit.h>");
  }

  if (strcasecmp(get_struct_property(node, "list", ""),
                 "private")==0) {
    GWEN_SyncIo_WriteLine(sio, "#include <gwenhywfar/misc.h>");
  }

  if (strcasecmp(get_struct_property(node, "list2", ""),
                 "private")==0) {
    GWEN_SyncIo_WriteLine(sio, "#include <gwenhywfar/list2.h>");
  }

  fname=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(fname, f);
  GWEN_Buffer_AppendString(fname, "_l.h");
  GWEN_SyncIo_WriteString(sio, "#include \"");
  GWEN_SyncIo_WriteString(sio, GWEN_Buffer_GetStart(fname));
  GWEN_SyncIo_WriteLine(sio, "\"");
  GWEN_Buffer_free(fname);

  GWEN_SyncIo_WriteLine(sio, "");

  rv=write_h_struct_c(args, node, sio);
  if (rv) {
    GWEN_Buffer_free(hbuf);
    GWEN_SyncIo_free(sio);
    return rv;
  }

  GWEN_SyncIo_WriteLine(sio, "");

  /* include private functions */
  if (strcasecmp(constAcc, "private")==0) {
    GWEN_SyncIo_WriteString(sio, id);
    GWEN_SyncIo_WriteString(sio, " *");
    GWEN_SyncIo_WriteString(sio, prefix);
    if (constName)
      GWEN_SyncIo_WriteString(sio, constName);
    else
      GWEN_SyncIo_WriteString(sio, "_new");
    GWEN_SyncIo_WriteLine(sio, "(void);");
  }

  /* FromDb */
  if (strcasecmp(fromDbAcc, "private")==0) {
    GWEN_SyncIo_WriteString(sio, id);
    GWEN_SyncIo_WriteString(sio, " *");
    GWEN_SyncIo_WriteString(sio, prefix);
    if (fromDbName)
      GWEN_SyncIo_WriteString(sio, fromDbName);
    else
      GWEN_SyncIo_WriteString(sio, "_fromDb");
    GWEN_SyncIo_WriteLine(sio, "(GWEN_DB_NODE *db);");
  }

  /* dup */
  if (strcasecmp(dupAcc, "private")==0) {
    GWEN_SyncIo_WriteString(sio, id);
    GWEN_SyncIo_WriteString(sio, " *");
    GWEN_SyncIo_WriteString(sio, prefix);
    if (dupName)
      GWEN_SyncIo_WriteString(sio, dupName);
    else
      GWEN_SyncIo_WriteString(sio, "_dup");
    GWEN_SyncIo_WriteString(sio, "(const ");
    GWEN_SyncIo_WriteString(sio, id);
    GWEN_SyncIo_WriteLine(sio, "*st);");
  }

  if (strcasecmp(nacc, "private")==0) {
    GWEN_SyncIo_WriteString(sio, "void ");
    GWEN_SyncIo_WriteString(sio, prefix);
    GWEN_SyncIo_WriteString(sio, "_free(");
    GWEN_SyncIo_WriteString(sio, id);
    GWEN_SyncIo_WriteLine(sio, " *st);");

    GWEN_SyncIo_WriteString(sio, "void ");
    GWEN_SyncIo_WriteString(sio, prefix);
    GWEN_SyncIo_WriteString(sio, "_Attach(");
    GWEN_SyncIo_WriteString(sio, id);
    GWEN_SyncIo_WriteLine(sio, " *st);");

    /* ReadDb */
    GWEN_SyncIo_WriteString(sio, "int ");
    GWEN_SyncIo_WriteString(sio, prefix);
    GWEN_SyncIo_WriteString(sio, "_ReadDb(");
    GWEN_SyncIo_WriteString(sio, id);
    GWEN_SyncIo_WriteLine(sio, " *st, GWEN_DB_NODE *db);");

    /* ToDb */
    GWEN_SyncIo_WriteString(sio, "int ");
    GWEN_SyncIo_WriteString(sio, prefix);
    GWEN_SyncIo_WriteString(sio, "_toDb(const ");
    GWEN_SyncIo_WriteString(sio, id);
    GWEN_SyncIo_WriteString(sio, "*st, GWEN_DB_NODE *db);");

    GWEN_SyncIo_WriteString(sio, "int ");
    GWEN_SyncIo_WriteString(sio, prefix);
    GWEN_SyncIo_WriteString(sio, "_IsModified(const ");
    GWEN_SyncIo_WriteString(sio, id);
    GWEN_SyncIo_WriteLine(sio, " *st);");

    GWEN_SyncIo_WriteString(sio, "void ");
    GWEN_SyncIo_WriteString(sio, prefix);
    GWEN_SyncIo_WriteString(sio, "_SetModified(");
    GWEN_SyncIo_WriteString(sio, id);
    GWEN_SyncIo_WriteLine(sio, " *st, int i);");
  }

  rv=write_h_setget_c(args, node, sio, "private");
  if (rv) {
    GWEN_Buffer_free(hbuf);
    GWEN_SyncIo_free(sio);
    return rv;
  }

  /* write trailing endif */
  err=GWEN_SyncIo_WriteLine(sio, "");
  if (err) {
    DBG_ERROR_ERR(0, err);
    GWEN_Buffer_free(hbuf);
    return -1;
  }

  err=GWEN_SyncIo_WriteString(sio, "#endif /* ");
  if (err) {
    DBG_ERROR_ERR(0, err);
    GWEN_Buffer_free(hbuf);
    return -1;
  }
  err=GWEN_SyncIo_WriteString(sio, GWEN_Buffer_GetStart(hbuf));
  if (err) {
    DBG_ERROR_ERR(0, err);
    GWEN_Buffer_free(hbuf);
    return -1;
  }
  err=GWEN_SyncIo_WriteLine(sio, " */");
  if (err) {
    DBG_ERROR_ERR(0, err);
    GWEN_Buffer_free(hbuf);
    return -1;
  }

  err=GWEN_SyncIo_Disconnect(sio);
  if (err) {
    DBG_ERROR_ERR(0, err);
    GWEN_SyncIo_free(sio);
    GWEN_Buffer_free(hbuf);
    return -1;
  }

  GWEN_Buffer_free(hbuf);
  return 0;
}



int write_hp_files_c(ARGUMENTS *args, GWEN_XMLNODE *node)
{
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








