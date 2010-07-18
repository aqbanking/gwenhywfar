


#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/xml.h>
#include <gwenhywfar/syncio_file.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>


#include "htmlctx_l.h"
#include "htmlprops_be.h"
#include "htmlfont_be.h"



void dumpObject(HTML_OBJECT *o, FILE *f, int indent) {
  HTML_OBJECT *c;
  int i;
  const char *s;
  HTML_PROPS *pr;
  HTML_FONT *fnt;

  s=HtmlObject_GetText(o);
  for (i=0; i<indent; i++) fprintf(f, " ");
  fprintf(stderr, "Object type: %d [%s] flags: %08x\n",
	  HtmlObject_GetObjectType(o),
	  s?s:"(empty)",
	  HtmlObject_GetFlags(o));

  pr=HtmlObject_GetProperties(o);
  fnt=HtmlProps_GetFont(pr);

  for (i=0; i<indent+2; i++) fprintf(f, " ");
  fprintf(stderr, "fgcol=%06x, bgcol=%06x, fontsize=%d, fontflags=%08x, fontname=[%s]\n",
	  HtmlProps_GetForegroundColor(pr),
	  HtmlProps_GetBackgroundColor(pr),
	  HtmlFont_GetFontSize(fnt),
	  HtmlFont_GetFontFlags(fnt),
	  HtmlFont_GetFontName(fnt));

  c=HtmlObject_Tree_GetFirstChild(o);
  while(c) {
    dumpObject(c, f, indent+2);
    c=HtmlObject_Tree_GetNext(c);
  }
}



int test1(int argc, char **argv) {
  GWEN_XML_CONTEXT *xmlCtx;
  GWEN_SYNCIO *sio;
  int rv;
  HTML_PROPS *pr;
  HTML_FONT *fnt;
  HTML_OBJECT_TREE *ot;

  if (argc<2) {
    fprintf(stderr, "Name of testfile needed.\n");
    return 1;
  }

  sio=GWEN_SyncIo_File_new(argv[1], GWEN_SyncIo_File_CreationMode_OpenExisting);
  GWEN_SyncIo_AddFlags(sio, GWEN_SYNCIO_FILE_FLAGS_READ);
  rv=GWEN_SyncIo_Connect(sio);
  if (rv<0) {
    DBG_ERROR(0, "here (%d)", rv);
    GWEN_SyncIo_free(sio);
    return 2;
  }

  xmlCtx=HtmlCtx_new(0);
  assert(xmlCtx);

  pr=HtmlProps_new();
  fnt=HtmlCtx_GetFont(xmlCtx, "times new roman", 12, 0);
  HtmlProps_SetFont(pr, fnt);
  HtmlFont_free(fnt);
  HtmlCtx_SetStandardProps(xmlCtx, pr);
  HtmlProps_free(pr);

  /* read OFX file into context */
  rv=GWEN_XMLContext_ReadFromIo(xmlCtx, sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    return rv;
  }

  ot=HtmlCtx_GetObjects(xmlCtx);
  if (ot) {
    HTML_OBJECT *o;

    o=HtmlObject_Tree_GetFirst(ot);
    dumpObject(o, stderr, 2);
  }

  GWEN_SyncIo_Disconnect(sio);
  GWEN_SyncIo_free(sio);
  GWEN_XmlCtx_free(xmlCtx);

  return 0;
}




int main(int argc, char **argv) {
  return test1(argc, argv);
}

