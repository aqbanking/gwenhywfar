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

  isPtr=atoi(get_property(node, "ptr", "0"));

  err=GWEN_BufferedIO_Write(bio, "  ");
  if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}

  err=GWEN_BufferedIO_Write(bio, typ);
  if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
  err=GWEN_BufferedIO_Write(bio, " ");
  if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
  if (isPtr) {
    err=GWEN_BufferedIO_Write(bio, "*");
    if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
  }

  err=GWEN_BufferedIO_WriteChar(bio, tolower(*name));
  if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
  err=GWEN_BufferedIO_Write(bio, name+1);
  if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
  err=GWEN_BufferedIO_WriteLine(bio, ";");

  return 0;
}


int write_h_struct_c(ARGUMENTS *args, GWEN_XMLNODE *node,
		     GWEN_BUFFEREDIO *bio) {
  GWEN_XMLNODE *n;
  const char *id;
  GWEN_ERRORCODE err;
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

  acc=get_struct_property(node, "access", "public");

  if (strcasecmp(acc, "private")==0) {
    err=GWEN_BufferedIO_Write(bio, "typedef struct ");
    if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
    err=GWEN_BufferedIO_Write(bio, id);
    if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
    err=GWEN_BufferedIO_Write(bio, " ");
    if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
    err=GWEN_BufferedIO_Write(bio, id);
    if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
    err=GWEN_BufferedIO_WriteLine(bio, ";");
    if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
  }
  err=GWEN_BufferedIO_Write(bio, "struct ");
  if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
  err=GWEN_BufferedIO_Write(bio, id);
  if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
  err=GWEN_BufferedIO_WriteLine(bio, " {");
  if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}

  if (get_struct_property(node, "inherit", 0)) {
    err=GWEN_BufferedIO_Write(bio, "  GWEN_INHERIT_ELEMENT(");
    if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
    err=GWEN_BufferedIO_Write(bio, id);
    if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
    err=GWEN_BufferedIO_WriteLine(bio, ")");
    if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
  }

  if (get_struct_property(node, "list", 0)) {
    err=GWEN_BufferedIO_Write(bio, "  GWEN_LIST_ELEMENT(");
    if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
    err=GWEN_BufferedIO_Write(bio, id);
    if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
    err=GWEN_BufferedIO_WriteLine(bio, ")");
    if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
  }

  err=GWEN_BufferedIO_WriteLine(bio, "  int _usage;");
  if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
  err=GWEN_BufferedIO_WriteLine(bio, "  int _modified;");
  if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}

  err=GWEN_BufferedIO_WriteLine(bio, "");
  if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}

  n=GWEN_XMLNode_GetFirstTag(node);
  while(n) {
    int rv;

    rv=write_hp_group_c(args, n, bio);
    if (rv)
      return rv;
    n=GWEN_XMLNode_GetNextTag(n);
  } /* while */

  err=GWEN_BufferedIO_WriteLine(bio, "};");
  if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}

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
    else if (strcasecmp(GWEN_XMLNode_GetData(node), "header")==0) {
      return write_h_header(args, node, bio);
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

  f=get_struct_property(node, "filename", 0);
  if (!f) {
    DBG_ERROR(0, "No filename given");
    return -1;
  }

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

  GWEN_BufferedIO_WriteLine(bio, "/* This is a generated file. Please do not edit. */");

  hbuf=GWEN_Buffer_new(0, 256, 0, 1);
  s=f;
  while(*s) {
    GWEN_Buffer_AppendByte(hbuf, toupper(*s));
    s++;
  }
  GWEN_Buffer_AppendString(hbuf, "_P_H");

  err=GWEN_BufferedIO_Write(bio, "#ifndef ");
  if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
  err=GWEN_BufferedIO_WriteLine(bio, GWEN_Buffer_GetStart(hbuf));
  if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
  err=GWEN_BufferedIO_Write(bio, "#define ");
  if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
  err=GWEN_BufferedIO_WriteLine(bio, GWEN_Buffer_GetStart(hbuf));
  if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
  err=GWEN_BufferedIO_WriteLine(bio, "");
  if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}

  if (strcasecmp(get_struct_property(node, "inherit", ""),
                 "private")==0) {
    err=GWEN_BufferedIO_WriteLine(bio, "#include <gwenhywfar/inherit.h>");
    if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
  }

  if (strcasecmp(get_struct_property(node, "list", ""),
                 "private")==0) {
    err=GWEN_BufferedIO_WriteLine(bio, "#include <gwenhywfar/misc.h>");
    if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
  }

  if (strcasecmp(get_struct_property(node, "list2", ""),
                 "private")==0) {
    err=GWEN_BufferedIO_WriteLine(bio, "#include <gwenhywfar/list2.h>");
    if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
  }

  fname=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(fname, f);
  GWEN_Buffer_AppendString(fname, "_l.h");
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    GWEN_Buffer_free(fname);
    GWEN_Buffer_free(hbuf);
    return -1;
  }
  err=GWEN_BufferedIO_Write(bio, "#include \"");
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    GWEN_Buffer_free(fname);
    GWEN_Buffer_free(hbuf);
    return -1;
  }
  err=GWEN_BufferedIO_Write(bio, GWEN_Buffer_GetStart(fname));
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    GWEN_Buffer_free(fname);
    GWEN_Buffer_free(hbuf);
    return -1;
  }
  err=GWEN_BufferedIO_WriteLine(bio, "\"");
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    GWEN_Buffer_free(fname);
    GWEN_Buffer_free(hbuf);
    return -1;
  }
  GWEN_Buffer_free(fname);

  err=GWEN_BufferedIO_WriteLine(bio, "");
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    GWEN_Buffer_free(hbuf);
    return -1;
  }

  rv=write_h_struct_c(args, node, bio);
  if (rv) {
    GWEN_Buffer_free(hbuf);
    GWEN_BufferedIO_Abandon(bio);
    GWEN_BufferedIO_free(bio);
    return rv;
  }

  err=GWEN_BufferedIO_WriteLine(bio, "");
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    GWEN_Buffer_free(hbuf);
    return -1;
  }

  /* include private functions */
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








