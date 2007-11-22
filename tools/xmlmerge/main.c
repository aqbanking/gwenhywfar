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
#include <gwenhywfar/debug.h>
#include <gwenhywfar/logger.h>
#include <gwenhywfar/xml.h>
#include <gwenhywfar/io_file.h>
#include <gwenhywfar/io_buffered.h>
#include <gwenhywfar/iomanager.h>

#include <stdlib.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>


int addDefinitions(GWEN_XMLNODE *top,
                   GWEN_XMLNODE *node) {
  GWEN_XMLNODE *nsrc, *ndst;

  assert(top);
  assert(node);

  nsrc=GWEN_XMLNode_GetChild(node);
  while(nsrc) {
    if (GWEN_XMLNode_GetType(nsrc)==GWEN_XMLNodeTypeTag) {
      ndst=GWEN_XMLNode_FindNode(top, GWEN_XMLNodeTypeTag,
                                 GWEN_XMLNode_GetData(nsrc));
      if (ndst) {
	GWEN_XMLNODE *n;

        n=GWEN_XMLNode_GetChild(nsrc);
	while (n) {
	  GWEN_XMLNODE *newNode;

          DBG_DEBUG(0, "Adding node \"%s\"", GWEN_XMLNode_GetData(n));
          newNode=GWEN_XMLNode_dup(n);
          GWEN_XMLNode_AddChild(ndst, newNode);
	  n=GWEN_XMLNode_Next(n);
	} /* while n */
      }
      else {
	GWEN_XMLNODE *newNode;

        DBG_DEBUG(0, "Adding branch \"%s\"", GWEN_XMLNode_GetData(nsrc));
	newNode=GWEN_XMLNode_dup(nsrc);
        GWEN_XMLNode_AddChild(top, newNode);
      }
    } /* if TAG */
    nsrc=GWEN_XMLNode_Next(nsrc);
  } /* while */

  return 0;
}



int main(int argc, char **argv) {
  ARGUMENTS *args;
  int rv;
  FREEPARAM *inFile;
  GWEN_XMLNODE *top;
  GWEN_XMLNODE *comment;
  GWEN_IO_LAYER *io;
  GWEN_XML_CONTEXT *ctx;
  int fd;
  uint32_t flags;

  args=Arguments_new();
  rv=checkArgs(args, argc, argv);
  if (rv==-1) {
    fprintf(stderr, "Parameter error\n");
    return rv;
  }
  else if (rv==-2) {
    return 0;
  }

  GWEN_Logger_Open(0, "xmlmerge",
                   args->logFile,
                   args->logType,
                   GWEN_LoggerFacility_User);
  GWEN_Logger_SetLevel(0, args->logLevel);

  inFile=args->params;
  if (!inFile) {
    fprintf(stderr, "No input file given.\n");
    Arguments_free(args);
    return 1;
  }

  top=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
  /* add comment */
  comment=GWEN_XMLNode_new(GWEN_XMLNodeTypeComment,
                           "This is an automatically generated file, "
                           "do not edit");
  GWEN_XMLNode_AddChild(top, comment);

  /* add header */
  if (args->header) {
    GWEN_XMLNODE *header;

    header=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag,
                            "?xml");
    GWEN_XMLNode_SetProperty(header, "version", "1.0");
    GWEN_XMLNode_SetProperty(header, "encoding", "utf8");
    GWEN_XMLNode_AddHeader(top, header);
  }

  /* read all files */
  while(inFile) {
    GWEN_XMLNODE *n;

    n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "inFile");
    if (GWEN_XML_ReadFile(n, inFile->param,
                          GWEN_XML_FLAGS_DEFAULT |
                          GWEN_XML_FLAGS_HANDLE_HEADERS)) {
      fprintf(stderr, "ERROR: Error reading file \"%s\"\n", inFile->param);
      GWEN_XMLNode_free(n);
      GWEN_XMLNode_free(top);
      return 2;
    }
    if (addDefinitions(top, n)) {
      fprintf(stderr, "ERROR: Error merging file \"%s\"\n",
              inFile->param);
      GWEN_XMLNode_free(n);
      GWEN_XMLNode_free(top);
      return 3;
    }
    GWEN_XMLNode_free(n);
    inFile=inFile->next;
  } /* while */

  /* write file */
  if (args->outputFile) {
    fd=open(args->outputFile, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd==-1) {
      fprintf(stderr, "Error opening file \"%s\": %s\n",
              args->outputFile,
              strerror(errno));
      GWEN_XMLNode_free(top);
      return 4;
    }
  }
  else {
    fd=1;
  }

  if (args->compact) {
    flags=GWEN_XML_FLAGS_SIMPLE;
  }
  else {
    flags=GWEN_XML_FLAGS_INDENT;
  }
  flags|=GWEN_XML_FLAGS_HANDLE_HEADERS;

  ctx=GWEN_XmlCtxStore_new(NULL, flags, 0, 10000);

  io=GWEN_Io_LayerFile_new(-1, fd);
  GWEN_Io_Manager_RegisterLayer(io);

  rv=GWEN_XMLNode_WriteToStream(top, ctx, io);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Io_Layer_Disconnect(io, GWEN_IO_REQUEST_FLAGS_FORCE, 0, 1000);
    GWEN_Io_Layer_free(io);
    GWEN_XmlCtx_free(ctx);
    return 5;
  }

  /* close file */
  rv=GWEN_Io_Layer_DisconnectRecursively(io, NULL, 0, GWEN_XmlCtx_GetGuiId(ctx), 30000);
  if (rv<0) {
    fprintf(stderr, "Error closing output stream (%d)\n", rv);
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Io_Layer_Disconnect(io, GWEN_IO_REQUEST_FLAGS_FORCE, GWEN_XmlCtx_GetGuiId(ctx), 1000);
    GWEN_Io_Layer_free(io);
    GWEN_XmlCtx_free(ctx);
    return 5;
  }

  GWEN_Io_Layer_free(io);
  GWEN_XmlCtx_free(ctx);

  GWEN_XMLNode_free(top);
  Arguments_free(args);

  return 0;
}




