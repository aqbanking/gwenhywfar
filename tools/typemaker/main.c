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

/* Internationalization */
#ifdef ENABLE_NLS
# include <libintl.h>
# include <locale.h>
# define I18N(m) dgettext("gwenhywfar", m)
# define I18S(m) m
#else
# define I18N(m) m
# define I18S(m) m
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



int write_h_header(ARGUMENTS *args, GWEN_XMLNODE *node,
		   GWEN_BUFFEREDIO *bio) {
  int isSys;
  GWEN_ERRORCODE err;
  const char *d;
  GWEN_XMLNODE *dn;

  dn=GWEN_XMLNode_GetFirstData(node);
  if (!dn) {
    DBG_ERROR(0, "Empty <header>");
    return -1;
  }
  d=GWEN_XMLNode_GetData(dn);
  if (!d) {
    DBG_ERROR(0, "Empty <header>");
    return -1;
  }
  isSys=(strcasecmp(GWEN_XMLNode_GetProperty(node, "type", ""),
		    "sys")==0);

  err=GWEN_BufferedIO_Write(bio, "#include ");
  if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
  if (isSys) {
    err=GWEN_BufferedIO_Write(bio, "<");
    if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
  }
  else {
    err=GWEN_BufferedIO_Write(bio, "\"");
    if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
  }
  err=GWEN_BufferedIO_Write(bio, d);
  if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
  if (isSys) {
    err=GWEN_BufferedIO_WriteLine(bio, ">");
    if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
  }
  else {
    err=GWEN_BufferedIO_WriteLine(bio, "\"");
    if (!GWEN_Error_IsOk(err)) { DBG_ERROR_ERR(0, err); return -1;}
  }

  return 0;
}



int write_h_setget_c(ARGUMENTS *args,
                     GWEN_XMLNODE *node,
                     GWEN_BUFFEREDIO *bio,
                     const char *acc) {
  GWEN_XMLNODE *n;
  int rv;
  const char *prefix;
  const char *styp;
  const char *sacc;

  sacc=get_struct_property(node, "access", "public");
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
        const char *name;
        int hasDoc;
        GWEN_XMLNODE *dn;

        hasDoc=0;
        name=GWEN_XMLNode_GetProperty(n, "name", 0);
        if (name) {
          hasDoc=1;
          GWEN_BufferedIO_Write(bio, "/** @name ");
          GWEN_BufferedIO_WriteLine(bio, name);
          dn=GWEN_XMLNode_FindFirstTag(n, "descr", 0, 0);
          if (dn) {
            GWEN_BufferedIO_WriteLine(bio, " *");
            if (GWEN_XMLNode_WriteToStream(dn, bio,
                                           GWEN_XML_FLAGS_SIMPLE |
                                           GWEN_XML_FLAGS_INDENT))
              return -1;
          }
          GWEN_BufferedIO_WriteLine(bio, "*/");
          GWEN_BufferedIO_WriteLine(bio, "/*@{*/");
        }
  
        rv=write_h_setget_c(args, n, bio, acc);
        if (rv)
          return rv;

        if (hasDoc) {
          GWEN_BufferedIO_WriteLine(bio, "/*@}*/");
        }

      }
      else if (strcasecmp(GWEN_XMLNode_GetData(n), "elem")==0) {
	if (strcasecmp(GWEN_XMLNode_GetProperty(n, "access", sacc),
		       acc)==0) {
	  int isPtr;
          const char *typ;
          const char *name;
          const char *mode;
          const char *tmode;
          int isConst;
          int doCopy;
          GWEN_XMLNODE *tnode;

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
          if (strcasecmp(mode, "single")!=0)
            isPtr=1;

          tnode=get_typedef(node, typ);
          if (tnode)
            tmode=GWEN_XMLNode_GetProperty(tnode, "mode", "single");
          else
            tmode=mode;

          /* getter */
          GWEN_BufferedIO_WriteLine(bio, "/**");
          GWEN_BufferedIO_Write(bio, "* Returns the property @ref ");
          GWEN_BufferedIO_Write(bio, styp);
          GWEN_BufferedIO_Write(bio, "_");
          GWEN_BufferedIO_WriteChar(bio, toupper(*name));
          GWEN_BufferedIO_WriteLine(bio, name+1);
          GWEN_BufferedIO_WriteLine(bio, "*/");
          if (args->domain) {
            GWEN_BufferedIO_Write(bio, args->domain);
            GWEN_BufferedIO_Write(bio, " ");
          }
          if (isPtr &&
              (strcasecmp(mode, "single")==0 || isConst)) {
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
          GWEN_BufferedIO_WriteChar(bio, toupper(*name));;
          GWEN_BufferedIO_Write(bio, name+1);
          GWEN_BufferedIO_Write(bio, "(const ");
          GWEN_BufferedIO_Write(bio, styp);
          GWEN_BufferedIO_WriteLine(bio, " *el);");

          /* setter */
          GWEN_BufferedIO_WriteLine(bio, "/**");
          GWEN_BufferedIO_Write(bio, "* Set the property @ref ");
          GWEN_BufferedIO_Write(bio, styp);
          GWEN_BufferedIO_Write(bio, "_");
          GWEN_BufferedIO_WriteChar(bio, toupper(*name));
          GWEN_BufferedIO_WriteLine(bio, name+1);
          GWEN_BufferedIO_WriteLine(bio, "*/");
          if (args->domain) {
            GWEN_BufferedIO_Write(bio, args->domain);
            GWEN_BufferedIO_Write(bio, " ");
          }
          GWEN_BufferedIO_Write(bio, "void ");
          GWEN_BufferedIO_Write(bio, prefix);
          GWEN_BufferedIO_Write(bio, "_Set");
          GWEN_BufferedIO_WriteChar(bio, toupper(*name));;
          GWEN_BufferedIO_Write(bio, name+1);
          GWEN_BufferedIO_Write(bio, "(");

          GWEN_BufferedIO_Write(bio, styp);
          GWEN_BufferedIO_Write(bio, " *el, ");
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
          GWEN_BufferedIO_WriteLine(bio, "d);");

          if (strcasecmp(typ, "GWEN_STRINGLIST")==0) {
            /* special functions for string lists */
            if (args->domain) {
              GWEN_BufferedIO_Write(bio, args->domain);
              GWEN_BufferedIO_Write(bio, " ");
            }
            GWEN_BufferedIO_Write(bio, "void ");
            GWEN_BufferedIO_Write(bio, prefix);
            GWEN_BufferedIO_Write(bio, "_Add");
            GWEN_BufferedIO_WriteChar(bio, toupper(*name));
            GWEN_BufferedIO_Write(bio, name+1);
            GWEN_BufferedIO_Write(bio, "(");
            GWEN_BufferedIO_Write(bio, styp);
            GWEN_BufferedIO_WriteLine(bio, " *st, const char *d, int chk);");

            if (args->domain) {
              GWEN_BufferedIO_Write(bio, args->domain);
              GWEN_BufferedIO_Write(bio, " ");
            }
            GWEN_BufferedIO_Write(bio, "void ");
            GWEN_BufferedIO_Write(bio, prefix);
            GWEN_BufferedIO_Write(bio, "_Remove");
            GWEN_BufferedIO_WriteChar(bio, toupper(*name));
            GWEN_BufferedIO_Write(bio, name+1);
            GWEN_BufferedIO_Write(bio, "(");
            GWEN_BufferedIO_Write(bio, styp);
            GWEN_BufferedIO_WriteLine(bio, " *st, const char *d);");

            if (args->domain) {
              GWEN_BufferedIO_Write(bio, args->domain);
              GWEN_BufferedIO_Write(bio, " ");
            }
            GWEN_BufferedIO_Write(bio, "void ");
            GWEN_BufferedIO_Write(bio, prefix);
            GWEN_BufferedIO_Write(bio, "_Clear");
            GWEN_BufferedIO_WriteChar(bio, toupper(*name));;
            GWEN_BufferedIO_Write(bio, name+1);
            GWEN_BufferedIO_Write(bio, "(");
            GWEN_BufferedIO_Write(bio, styp);
            GWEN_BufferedIO_WriteLine(bio, " *st);");

            if (args->domain) {
              GWEN_BufferedIO_Write(bio, args->domain);
              GWEN_BufferedIO_Write(bio, " ");
            }
            GWEN_BufferedIO_Write(bio, "int ");
            GWEN_BufferedIO_Write(bio, prefix);
            GWEN_BufferedIO_Write(bio, "_Has");
            GWEN_BufferedIO_WriteChar(bio, toupper(*name));;
            GWEN_BufferedIO_Write(bio, name+1);
            GWEN_BufferedIO_Write(bio, "(const ");
            GWEN_BufferedIO_Write(bio, styp);
            GWEN_BufferedIO_WriteLine(bio, " *st, const char *d);");
          }

        }
      }
    }
    GWEN_BufferedIO_WriteLine(bio, "");
    n=GWEN_XMLNode_GetNextTag(n);
  }
  return 0;
}



int write_h_enums(ARGUMENTS *args, GWEN_XMLNODE *node,
		  GWEN_BUFFEREDIO *bio,
		  const char *acc) {
  GWEN_XMLNODE *n;

  n=GWEN_XMLNode_FindFirstTag(node, "subtypes", 0, 0);
  if (n)
    n=GWEN_XMLNode_FindFirstTag(n, "type", "mode", "enum");
  if (n) {
    GWEN_BUFFER *tprefix;
    GWEN_BUFFER *tid;
    GWEN_TYPE_UINT32 ppos;
    GWEN_TYPE_UINT32 tpos;
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
      s=GWEN_XMLNode_GetProperty(n, "access", "public");
      if (strcasecmp(s, acc)==0) {
	GWEN_XMLNODE *nn;

	s=GWEN_XMLNode_GetProperty(n, "id", 0);
	assert(s);
	GWEN_Buffer_AppendString(tid, s);
	s=GWEN_XMLNode_GetProperty(n, "prefix", 0);
	assert(s);
	GWEN_Buffer_AppendString(tprefix, s);
  
	GWEN_BufferedIO_WriteLine(bio, "typedef enum {");
	GWEN_BufferedIO_Write(bio, "  ");
	GWEN_BufferedIO_Write(bio, GWEN_Buffer_GetStart(tprefix));
	GWEN_BufferedIO_Write(bio, "Unknown=-1");

	nn=GWEN_XMLNode_FindFirstTag(n, "values", 0, 0);
	if (nn)
	  nn=GWEN_XMLNode_FindFirstTag(nn, "value", 0, 0);
	if (nn) {
	  GWEN_TYPE_UINT32 vpos;

	  vpos=GWEN_Buffer_GetPos(tprefix);
	  while(nn) {
	    GWEN_XMLNODE *nnn;
            GWEN_XMLNODE *dn;

	    nnn=GWEN_XMLNode_GetFirstData(nn);
	    if (!nnn) {
	      DBG_ERROR(0, "No values in enum description for \"%s\"",
			GWEN_Buffer_GetStart(tid));
	      GWEN_Buffer_free(tid);
	      GWEN_Buffer_free(tprefix);
	      return -1;
	    }
            GWEN_BufferedIO_WriteLine(bio, ",");

            dn=GWEN_XMLNode_FindFirstTag(nn, "descr", 0, 0);
            if (dn) {
              GWEN_BufferedIO_Write(bio, "  /** ");
              if (GWEN_XMLNode_WriteToStream(dn, bio,
                                             GWEN_XML_FLAGS_SIMPLE |
                                             GWEN_XML_FLAGS_INDENT))
                return -1;
              GWEN_BufferedIO_WriteLine(bio, "  */");
            }

            GWEN_BufferedIO_Write(bio, "  ");
	    s=GWEN_XMLNode_GetData(nnn);
	    assert(s);
	    GWEN_Buffer_AppendByte(tprefix, toupper(*s));
	    GWEN_Buffer_AppendString(tprefix, s+1);
	    GWEN_BufferedIO_Write(bio, GWEN_Buffer_GetStart(tprefix));
	    s=GWEN_XMLNode_GetProperty(nn, "value", 0);
	    if (s) {
	      GWEN_BufferedIO_Write(bio, "=");
	      GWEN_BufferedIO_Write(bio, s);
	    }
  
	    GWEN_Buffer_Crop(tprefix, 0, vpos);
	    nn=GWEN_XMLNode_FindNextTag(nn, "value", 0, 0);
	  }
	}
  
	GWEN_BufferedIO_WriteLine(bio, "");
	GWEN_BufferedIO_Write(bio, "} ");
	GWEN_BufferedIO_Write(bio, GWEN_Buffer_GetStart(tid));
	GWEN_BufferedIO_WriteLine(bio, ";");
	GWEN_BufferedIO_WriteLine(bio, "");

	GWEN_BufferedIO_Write(bio, GWEN_Buffer_GetStart(tid));
	GWEN_BufferedIO_Write(bio, " ");
	GWEN_BufferedIO_Write(bio, GWEN_Buffer_GetStart(tprefix));
	GWEN_BufferedIO_WriteLine(bio, "_fromString(const char *s);");

	GWEN_BufferedIO_Write(bio, "const char *");
	GWEN_BufferedIO_Write(bio, GWEN_Buffer_GetStart(tprefix));
	GWEN_BufferedIO_Write(bio, "_toString(");
	GWEN_BufferedIO_Write(bio, GWEN_Buffer_GetStart(tid));
	GWEN_BufferedIO_WriteLine(bio, " v);");

	GWEN_Buffer_Crop(tprefix, 0, ppos);
	GWEN_Buffer_Crop(tid, 0, tpos);
	GWEN_BufferedIO_WriteLine(bio, "");
      } /* if access type matches */
      n=GWEN_XMLNode_FindNextTag(n, "type", "mode", "enum");
    } /* while n */
    GWEN_Buffer_free(tid);
    GWEN_Buffer_free(tprefix);
  } /* if enum types found */

  return 0;
}




int write_apidocrec_c(ARGUMENTS *args,
                      GWEN_XMLNODE *node,
                      GWEN_BUFFEREDIO *bio,
                      const char *acc,
                      int level) {
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
        const char *name;
        GWEN_XMLNODE *dn;
        char numbuf[16];

        name=GWEN_XMLNode_GetProperty(n, "name", 0);
        if (!name) {
          DBG_ERROR(0, "No name for element");
          return -1;
        }

        snprintf(numbuf, sizeof(numbuf), "%d", level);
        GWEN_BufferedIO_Write(bio, "<h");
        GWEN_BufferedIO_Write(bio, numbuf);
        GWEN_BufferedIO_Write(bio, ">");
        GWEN_BufferedIO_WriteChar(bio, toupper(*name));
        GWEN_BufferedIO_Write(bio, name+1);
        GWEN_BufferedIO_Write(bio, "</h");
        GWEN_BufferedIO_Write(bio, numbuf);
        GWEN_BufferedIO_WriteLine(bio, ">");

        GWEN_BufferedIO_WriteLine(bio, "<p>");
        dn=GWEN_XMLNode_FindFirstTag(n, "descr", 0, 0);
        if (dn) {
          if (GWEN_XMLNode_WriteToStream(dn, bio,
                                         GWEN_XML_FLAGS_SIMPLE |
                                         GWEN_XML_FLAGS_INDENT))
            return -1;
        }
        GWEN_BufferedIO_WriteLine(bio, "</p>");

        rv=write_apidocrec_c(args, n, bio, acc, level+1);
        if (rv)
          return rv;
      }
      else if (strcasecmp(GWEN_XMLNode_GetData(n), "elem")==0) {
        if (strcasecmp(GWEN_XMLNode_GetProperty(n, "access", "public"),
                       acc)==0) {
          const char *typ;
          const char *name;
          GWEN_XMLNODE *dn;
          char numbuf[16];

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

          snprintf(numbuf, sizeof(numbuf), "%d", level);
          GWEN_BufferedIO_Write(bio, "@anchor ");
          GWEN_BufferedIO_Write(bio, styp);
          GWEN_BufferedIO_Write(bio, "_");
          GWEN_BufferedIO_WriteChar(bio, toupper(*name));
          GWEN_BufferedIO_WriteLine(bio, name+1);

          GWEN_BufferedIO_Write(bio, "<h");
          GWEN_BufferedIO_Write(bio, numbuf);
          GWEN_BufferedIO_Write(bio, ">");
          GWEN_BufferedIO_WriteChar(bio, toupper(*name));
          GWEN_BufferedIO_Write(bio, name+1);
          GWEN_BufferedIO_Write(bio, "</h");
          GWEN_BufferedIO_Write(bio, numbuf);
          GWEN_BufferedIO_WriteLine(bio, ">");

          dn=GWEN_XMLNode_FindFirstTag(n, "brief", 0, 0);
          if (dn) {
            GWEN_BufferedIO_Write(bio, "@short ");
            if (GWEN_XMLNode_WriteToStream(dn, bio,
                                           GWEN_XML_FLAGS_SIMPLE |
                                           GWEN_XML_FLAGS_INDENT))
              return -1;
            GWEN_BufferedIO_WriteLine(bio, "");
            GWEN_BufferedIO_WriteLine(bio, "");
          }

          GWEN_BufferedIO_WriteLine(bio, "<p>");
          dn=GWEN_XMLNode_FindFirstTag(n, "descr", 0, 0);
          if (dn) {
            if (GWEN_XMLNode_WriteToStream(dn, bio,
                                           GWEN_XML_FLAGS_SIMPLE |
                                           GWEN_XML_FLAGS_INDENT))
              return -1;
          }
          GWEN_BufferedIO_WriteLine(bio, "</p>");

          GWEN_BufferedIO_WriteLine(bio, "<p>");
          GWEN_BufferedIO_Write(bio, "Set this property with @ref ");
          GWEN_BufferedIO_Write(bio, prefix);
          GWEN_BufferedIO_Write(bio, "_Set");
          GWEN_BufferedIO_WriteChar(bio, toupper(*name));
          GWEN_BufferedIO_Write(bio, name+1);
          GWEN_BufferedIO_WriteLine(bio, ", ");
          GWEN_BufferedIO_Write(bio, "get it with @ref ");
          GWEN_BufferedIO_Write(bio, prefix);
          GWEN_BufferedIO_Write(bio, "_Get");
          GWEN_BufferedIO_WriteChar(bio, toupper(*name));
          GWEN_BufferedIO_WriteLine(bio, name+1);
          GWEN_BufferedIO_WriteLine(bio, "</p>");
          GWEN_BufferedIO_WriteLine(bio, "");
        }
      }
    }
    n=GWEN_XMLNode_GetNextTag(n);
  }
  return 0;
}



int write_apidoc_c(ARGUMENTS *args,
                   GWEN_XMLNODE *node,
                   GWEN_BUFFEREDIO *bio,
                   const char *acc) {
  const char *prefix;
  const char *styp;
  GWEN_XMLNODE *dn;
  const char *brief;
  const char *s;
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

  GWEN_BufferedIO_Write(bio, "/** @page P_");
  GWEN_BufferedIO_Write(bio, styp);
  GWEN_BufferedIO_Write(bio, "_");
  s=acc;
  while(*s) {
    GWEN_BufferedIO_WriteChar(bio, toupper(*s));
    s++;
  }
  brief=GWEN_XMLNode_GetProperty(node, "name", prefix);
  GWEN_BufferedIO_Write(bio, " ");
  GWEN_BufferedIO_Write(bio, brief);
  GWEN_BufferedIO_Write(bio, " (");
  GWEN_BufferedIO_Write(bio, acc);
  GWEN_BufferedIO_WriteLine(bio, ")");

  GWEN_BufferedIO_Write(bio, "This page describes the properties of ");
  GWEN_BufferedIO_WriteLine(bio, styp);

  dn=GWEN_XMLNode_FindFirstTag(node, "descr", 0, 0);
  if (dn) {
    if (GWEN_XMLNode_WriteToStream(dn, bio,
                                   GWEN_XML_FLAGS_SIMPLE |
                                   GWEN_XML_FLAGS_INDENT))
      return -1;
  }

  rv=write_apidocrec_c(args, node, bio, acc, 3);
  if (rv)
    return rv;

  GWEN_BufferedIO_WriteLine(bio, "*/");

  return 0;
}




int write_files(ARGUMENTS *args, GWEN_XMLNODE *node) {
  int rv;
  GWEN_XMLNODE *n;

  n=GWEN_XMLNode_FindFirstTag(node, "types", 0, 0);
  if (!n) {
    DBG_WARN(0, "Empty file");
    return 0;
  }
  rv=write_hp_files_c(args, n);
  if (rv)
    return rv;

  rv=write_hl_files_c(args, n);
  if (rv)
    return rv;

  rv=write_ha_files_c(args, n);
  if (rv)
    return rv;

  rv=write_code_files_c(args, n);
  if (rv)
    return rv;

  return 0;
}



const char *get_function_name2(GWEN_XMLNODE *node, const char *ftype,
                               const char *name) {
  GWEN_XMLNODE *n;

  /* find typedef for this type */
  n=node;
  /* get root */
  while(GWEN_XMLNode_GetParent(n))
    n=GWEN_XMLNode_GetParent(n);

  n=GWEN_XMLNode_FindFirstTag(n, "typedefs", 0, 0);
  if (!n)
    return 0;
  n=GWEN_XMLNode_FindFirstTag(n, "type", "id", name);
  if (!n)
    return 0;

  n=GWEN_XMLNode_FindFirstTag(n, "functions", 0, 0);
  if (n) {
    n=GWEN_XMLNode_FindFirstTag(n, "function", "type", ftype);
    if (n)
      return GWEN_XMLNode_GetProperty(n, "name", 0);
  }

  return 0;
}



const char *get_function_name(GWEN_XMLNODE *node, const char *ftype) {
  GWEN_XMLNODE *n;
  const char *name;

  n=GWEN_XMLNode_FindFirstTag(node, "functions", 0, 0);
  if (n) {
    n=GWEN_XMLNode_FindFirstTag(n, "function", "type", ftype);
    if (n)
      return GWEN_XMLNode_GetProperty(n, "name", 0);
  }

  name=GWEN_XMLNode_GetProperty(node, "type", 0);
  if (!name) {
    DBG_ERROR(0, "no type for element");
    return 0;
  }

  /* find typedef for this type */
  n=node;
  /* get root */
  while(GWEN_XMLNode_GetParent(n))
    n=GWEN_XMLNode_GetParent(n);

  n=GWEN_XMLNode_FindFirstTag(n, "typedefs", 0, 0);
  if (!n)
    return 0;
  n=GWEN_XMLNode_FindFirstTag(n, "type", "id", name);
  if (!n)
    return 0;

  n=GWEN_XMLNode_FindFirstTag(n, "functions", 0, 0);
  if (n) {
    n=GWEN_XMLNode_FindFirstTag(n, "function", "type", ftype);
    if (n)
      return GWEN_XMLNode_GetProperty(n, "name", 0);
  }

  return 0;
}



GWEN_XMLNODE *get_typedef(GWEN_XMLNODE *node, const char *name) {
  GWEN_XMLNODE *n;
  GWEN_XMLNODE *n2;

  assert(name);
  /* find typedef for this type */
  n=node;
  /* get root */
  while(GWEN_XMLNode_GetParent(n))
    n=GWEN_XMLNode_GetParent(n);

  n2=GWEN_XMLNode_FindFirstTag(n, "typedefs", 0, 0);
  if (n2)
    n2=GWEN_XMLNode_FindFirstTag(n2, "type", "id", name);
  if (n2)
    return n2;

  n2=GWEN_XMLNode_FindFirstTag(n, "types", 0, 0);
  if (n2)
    n2=GWEN_XMLNode_FindFirstTag(n2, "type", "id", name);
  if (n2)
    return n2;

  n2=get_struct_node(node);
  assert(n2);

  n2=GWEN_XMLNode_FindFirstTag(n2, "subtypes", 0, 0);
  if (n2)
    n2=GWEN_XMLNode_FindFirstTag(n2, "type", "id", name);
  if (n2)
    return n2;

  return 0;
}



const char *get_property(GWEN_XMLNODE *node,
                         const char *pname,
                         const char *defval) {
  GWEN_XMLNODE *n;
  const char *name;
  const char *r;

  r=GWEN_XMLNode_GetProperty(node, pname, 0);
  if (r)
    return r;

  name=GWEN_XMLNode_GetProperty(node, "type", 0);
  if (!name) {
    DBG_ERROR(0, "no type for element");
    return defval;
  }

  /* find typedef for this type */
  n=node;
  /* get root */
  while(GWEN_XMLNode_GetParent(n))
    n=GWEN_XMLNode_GetParent(n);

  n=GWEN_XMLNode_FindFirstTag(n, "typedefs", 0, 0);
  if (!n)
    return defval;
  n=GWEN_XMLNode_FindFirstTag(n, "type", "id", name);
  if (!n)
    return defval;

  return GWEN_XMLNode_GetProperty(n, pname, defval);
}



const char *get_struct_property(GWEN_XMLNODE *node,
                                const char *pname,
                                const char *defval) {
  GWEN_XMLNODE *n;

  if (GWEN_XMLNode_GetType(node)==GWEN_XMLNodeTypeTag)
    if (strcasecmp(GWEN_XMLNode_GetData(node), "type")==0) {
      return GWEN_XMLNode_GetProperty(node, pname, defval);
    }

  /* find typedef for this type */
  n=GWEN_XMLNode_GetParent(node);
  if (n)
    return get_struct_property(n, pname, defval);

  return defval;
}



GWEN_XMLNODE *get_struct_node(GWEN_XMLNODE *node) {
  while(node) {
    if (GWEN_XMLNode_GetType(node)==GWEN_XMLNodeTypeTag)
      if (strcasecmp(GWEN_XMLNode_GetData(node), "type")==0) {
	return node;
      }
    node=GWEN_XMLNode_GetParent(node);
  }

  return 0;
}



int main(int argc, char **argv) {
  ARGUMENTS *args;
  int rv;
  FREEPARAM *inFile;

  args=Arguments_new();
  rv=checkArgs(args, argc, argv);
  if (rv==-1) {
    fprintf(stderr, "Parameter error\n");
    return rv;
  }
  else if (rv==-2) {
    return 0;
  }

  GWEN_Logger_Open(0, "typemaker",
                   args->logFile,
                   args->logType,
                   GWEN_LoggerFacilityUser);
  GWEN_Logger_SetLevel(0, args->logLevel);

  inFile=args->params;
  if (!inFile) {
    fprintf(stderr, "No input file given.\n");
    Arguments_free(args);
    return 1;
  }

  /* read all files */
  while(inFile) {
    GWEN_XMLNODE *n;

    n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "inFile");
    if (GWEN_XML_ReadFile(n, inFile->param, GWEN_XML_FLAGS_DEFAULT)) {
      fprintf(stderr, "ERROR: Error reading file \"%s\"\n", inFile->param);
      GWEN_XMLNode_free(n);
      return 2;
    }

    /* write file(s) */
    rv=write_files(args, n);
    if (rv) {
      DBG_ERROR(0, "Error in file \"%s\"", inFile->param);
      GWEN_XMLNode_free(n);
      return 2;
    }

    GWEN_XMLNode_free(n);
    inFile=inFile->next;
  } /* while */


  Arguments_free(args);

  return 0;
}




