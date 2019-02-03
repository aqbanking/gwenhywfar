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
#include <gwenhywfar/gwenhywfar.h>
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


int addDefinitions(GWEN_XMLNODE *top,
                   GWEN_XMLNODE *node)
{
  GWEN_XMLNODE *nsrc, *ndst;

  assert(top);
  assert(node);

  nsrc=GWEN_XMLNode_GetChild(node);
  while (nsrc) {
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



int main(int argc, char **argv)
{
  ARGUMENTS *args;
  int rv;
  FREEPARAM *inFile;
  GWEN_XMLNODE *top;
  GWEN_XMLNODE *comment;
  GWEN_SYNCIO *sio;
  GWEN_XML_CONTEXT *ctx;
  uint32_t flags;

  rv=GWEN_Init();
  if (rv) {
    fprintf(stderr, "Could not initialize Gwenhywfar.\n");
    return 2;
  }

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
  while (inFile) {
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
  if (args->compact) {
    flags=GWEN_XML_FLAGS_SIMPLE;
  }
  else {
    flags=GWEN_XML_FLAGS_INDENT;
  }
  flags|=GWEN_XML_FLAGS_HANDLE_HEADERS;

  ctx=GWEN_XmlCtxStore_new(NULL, flags);

  sio=GWEN_SyncIo_File_new(args->outputFile, GWEN_SyncIo_File_CreationMode_CreateAlways);
  GWEN_SyncIo_AddFlags(sio,
                       GWEN_SYNCIO_FILE_FLAGS_READ |
                       GWEN_SYNCIO_FILE_FLAGS_WRITE |
                       GWEN_SYNCIO_FILE_FLAGS_UREAD |
                       GWEN_SYNCIO_FILE_FLAGS_UWRITE |
                       GWEN_SYNCIO_FILE_FLAGS_GREAD |
                       GWEN_SYNCIO_FILE_FLAGS_GWRITE);
  rv=GWEN_SyncIo_Connect(sio);
  if (rv<0) {
    fprintf(stderr, "Error opening file \"%s\": %s\n",
            args->outputFile,
            strerror(errno));
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    GWEN_XmlCtx_free(ctx);
    return 5;
  }


  rv=GWEN_XMLNode_WriteToStream(top, ctx, sio);
  if (rv<0) {
    fprintf(stderr, "Error writing to file \"%s\": %s\n",
            args->outputFile,
            strerror(errno));
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    GWEN_XmlCtx_free(ctx);
    return 5;
  }

  /* close file */
  rv=GWEN_SyncIo_Disconnect(sio);
  if (rv<0) {
    fprintf(stderr, "Error closing output stream (%d)\n", rv);
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_free(sio);
    GWEN_XmlCtx_free(ctx);
    return 5;
  }

  GWEN_SyncIo_free(sio);
  GWEN_XmlCtx_free(ctx);

  GWEN_XMLNode_free(top);
  Arguments_free(args);

  return 0;
}




