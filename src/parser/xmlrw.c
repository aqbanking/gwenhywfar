/***************************************************************************
 copyright   : (C) 2007 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston,                 *
 *   MA  02111-1307  USA                                                   *
 *                                                                         *
 ***************************************************************************/


/* this file is included from xml.c */



int GWEN_XMLNode__WriteToStream(const GWEN_XMLNODE *n,
				GWEN_FAST_BUFFER *fb,
				uint32_t flags,
				unsigned int ind) {
  GWEN_XMLPROPERTY *p;
  GWEN_XMLNODE *c;
  int i;
  int simpleTag;
  int rv;

#define CHECK_ERROR(rv) \
  if (rv<0) {\
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);\
    return rv;\
  }

  assert(n);

  if (flags & GWEN_XML_FLAGS_INDENT) {
    for(i=0; i<ind; i++) {
      GWEN_FASTBUFFER_WRITEBYTE(fb, rv, ' ');
      CHECK_ERROR(rv);
    }
  }

  simpleTag=0;
  if (n->type==GWEN_XMLNodeTypeTag) {
    if (n->data) {
      GWEN_FASTBUFFER_WRITEBYTE(fb, rv, '<');
      CHECK_ERROR(rv);
      GWEN_FASTBUFFER_WRITEFORCED(fb, rv, n->data, -1);
      CHECK_ERROR(rv);
    }
    else {
      GWEN_FASTBUFFER_WRITEFORCED(fb, rv, "<UNKNOWN", -1);
      CHECK_ERROR(rv);
    }

    if (flags & GWEN_XML_FLAGS_HANDLE_NAMESPACES) {
      GWEN_XMLNODE_NAMESPACE *ns;

      ns=GWEN_XMLNode_NameSpace_List_First(n->nameSpaces);
      while(ns) {
	const char *name;
	const char *url;

	name=GWEN_XMLNode_NameSpace_GetName(ns);
	url=GWEN_XMLNode_NameSpace_GetUrl(ns);
	GWEN_FASTBUFFER_WRITEBYTE(fb, rv, ' ');
	CHECK_ERROR(rv);
	GWEN_FASTBUFFER_WRITEFORCED(fb, rv, "xmlns", -1);
	CHECK_ERROR(rv);
	if (name) {
	  GWEN_FASTBUFFER_WRITEFORCED(fb, rv, ":", -1);
	  CHECK_ERROR(rv);
	  GWEN_FASTBUFFER_WRITEFORCED(fb, rv, name, -1);
	  CHECK_ERROR(rv);
	}
	GWEN_FASTBUFFER_WRITEFORCED(fb, rv, "=\"", -1);
	CHECK_ERROR(rv);
	if (url) {
	  GWEN_FASTBUFFER_WRITEFORCED(fb, rv, url, -1);
	  CHECK_ERROR(rv);
	}
	GWEN_FASTBUFFER_WRITEFORCED(fb, rv, "\"", -1);
	CHECK_ERROR(rv);

	ns=GWEN_XMLNode_NameSpace_List_Next(ns);
      }
    }

    p=n->properties;
    while (p) {
      GWEN_FASTBUFFER_WRITEBYTE(fb, rv, ' ');
      CHECK_ERROR(rv);
      GWEN_FASTBUFFER_WRITEFORCED(fb, rv, p->name, -1);
      CHECK_ERROR(rv);
      if (p->value) {
	GWEN_FASTBUFFER_WRITEFORCED(fb, rv, "=\"", -1);
	CHECK_ERROR(rv);
	GWEN_FASTBUFFER_WRITEFORCED(fb, rv, p->value, -1);
	CHECK_ERROR(rv);
	GWEN_FASTBUFFER_WRITEFORCED(fb, rv, "\"", -1);
	CHECK_ERROR(rv);
      }
      p=p->next;
    }

    if (n->data) {
      if (n->data[0]=='?') {
        simpleTag=1;
	GWEN_FASTBUFFER_WRITEBYTE(fb, rv, '?');
	CHECK_ERROR(rv);
      }
      else if (n->data[0]=='!') {
        simpleTag=1;
      }
    }

    GWEN_FASTBUFFER_WRITELINE(fb, rv, ">");
    CHECK_ERROR(rv);
    if (!simpleTag) {
      c=GWEN_XMLNode_GetChild(n);
      while(c) {
	rv=GWEN_XMLNode__WriteToStream(c, fb, flags, ind+2);
	CHECK_ERROR(rv);
        c=GWEN_XMLNode_Next(c);
      }
      if (flags & GWEN_XML_FLAGS_INDENT) {
	for(i=0; i<ind; i++) {
	  GWEN_FASTBUFFER_WRITEBYTE(fb, rv, ' ');
	  CHECK_ERROR(rv);
	}
      }
      if (n->data) {
	GWEN_FASTBUFFER_WRITEFORCED(fb, rv, "</", -1);
	CHECK_ERROR(rv);
	GWEN_FASTBUFFER_WRITEFORCED(fb, rv, n->data, -1);
	CHECK_ERROR(rv);
	GWEN_FASTBUFFER_WRITELINE(fb, rv, ">");
	CHECK_ERROR(rv);
      }
      else {
	GWEN_FASTBUFFER_WRITELINE(fb, rv, "</UNKNOWN>");
	CHECK_ERROR(rv);
      }
    }
  }
  else if (n->type==GWEN_XMLNodeTypeData) {
    if (n->data) {
      GWEN_FASTBUFFER_WRITEFORCED(fb, rv, n->data, -1);
      CHECK_ERROR(rv);
      GWEN_FASTBUFFER_WRITELINE(fb, rv, "");
      CHECK_ERROR(rv);
    }
  }
  else if (n->type==GWEN_XMLNodeTypeComment) {
    if (flags & GWEN_XML_FLAGS_HANDLE_COMMENTS) {
      GWEN_FASTBUFFER_WRITEFORCED(fb, rv, "<!--", -1);
      CHECK_ERROR(rv);
      if (n->data) {
	GWEN_FASTBUFFER_WRITEFORCED(fb, rv, n->data, -1);
	CHECK_ERROR(rv);
      }
      GWEN_FASTBUFFER_WRITELINE(fb, rv, "-->");
      CHECK_ERROR(rv);
    }
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Unknown tag type (%d)", n->type);
  }

  return 0;
#undef CHECK_ERROR
}



int GWEN_XMLNode_WriteToStream(const GWEN_XMLNODE *n,
                               GWEN_XML_CONTEXT *ctx,
			       GWEN_IO_LAYER *io){
  const GWEN_XMLNODE *nn;
  const GWEN_XMLNODE *nchild;
  const GWEN_XMLNODE *nheader;
  uint32_t flags;
  GWEN_FAST_BUFFER *fb;
  int rv;

  flags=GWEN_XmlCtx_GetFlags(ctx);
  nchild=GWEN_XMLNode_GetChild(n);
  nheader=GWEN_XMLNode_GetHeader(n);

  fb=GWEN_FastBuffer_new(512, io, GWEN_XmlCtx_GetGuiId(ctx), GWEN_XmlCtx_GetTimeout(ctx));

  if (nheader && (flags & GWEN_XML_FLAGS_HANDLE_HEADERS)) {
    uint32_t lflags;

    lflags=flags & ~GWEN_XML_FLAGS_HANDLE_HEADERS;
    nn=nheader;
    while(nn) {
      const GWEN_XMLNODE *next;
      int rv;

      rv=GWEN_XMLNode__WriteToStream(nn, fb, GWEN_XmlCtx_GetFlags(ctx), 0);
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	GWEN_FastBuffer_free(fb);
	return rv;
      }
      next=GWEN_XMLNode_Next(nn);
      if (next) {
        int err;

	GWEN_FASTBUFFER_WRITELINE(fb, err, "");
	if (err<0) {
	  DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", err);
	  GWEN_FastBuffer_free(fb);
	  return err;
	}
      }

      nn=next;
    }

    if (nchild) {
      int err;

      GWEN_FASTBUFFER_WRITELINE(fb, err, "");
      if (err<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", err);
	GWEN_FastBuffer_free(fb);
	return err;
      }
    }
  }

  nn=nchild;
  while(nn) {
    const GWEN_XMLNODE *next;

    if (GWEN_XMLNode__WriteToStream(nn, fb, GWEN_XmlCtx_GetFlags(ctx), 0))
      return -1;
    next=GWEN_XMLNode_Next(nn);
    if (next) {
      int err;

      GWEN_FASTBUFFER_WRITELINE(fb, err, "");
      if (err<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", err);
	GWEN_FastBuffer_free(fb);
	return err;
      }
    }

    nn=next;
  } /* while */

  GWEN_FASTBUFFER_FLUSH(fb, rv);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_FastBuffer_free(fb);
    return rv;
  }
  GWEN_FastBuffer_free(fb);

  return 0;
}



int GWEN_XMLNode_WriteFile(const GWEN_XMLNODE *n,
                           const char *fname,
                           uint32_t flags){
  GWEN_XML_CONTEXT *ctx;
  GWEN_IO_LAYER *io;
  int fd;
  int rv;

  /* open file for writing */
  fd=open(fname, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  if (fd==-1) {
    DBG_ERROR(0, "open(%s): %s", fname, strerror(errno));
    return GWEN_ERROR_IO;
  }

  /* create context and io layers */
  ctx=GWEN_XmlCtxStore_new(NULL, flags, 0, 10000);
  io=GWEN_Io_LayerFile_new(-1, fd);
  GWEN_Io_Manager_RegisterLayer(io);

  /* write data to stream */
  rv=GWEN_XMLNode_WriteToStream(n, ctx, io);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Io_Layer_Disconnect(io, GWEN_IO_REQUEST_FLAGS_FORCE, 0, 1000);
    GWEN_Io_Layer_free(io);
    GWEN_XmlCtx_free(ctx);
    return rv;
  }

  /* close file */
  rv=GWEN_Io_Layer_DisconnectRecursively(io, NULL, 0, GWEN_XmlCtx_GetGuiId(ctx), 30000);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Io_Layer_Disconnect(io, GWEN_IO_REQUEST_FLAGS_FORCE, GWEN_XmlCtx_GetGuiId(ctx), 1000);
    GWEN_Io_Layer_free(io);
    GWEN_XmlCtx_free(ctx);
    return rv;
  }

  GWEN_Io_Layer_free(io);
  GWEN_XmlCtx_free(ctx);

  return 0;
}



int GWEN_XMLNode_toBuffer(const GWEN_XMLNODE *n, GWEN_BUFFER *buf, uint32_t flags){
  GWEN_XML_CONTEXT *ctx;
  GWEN_IO_LAYER *io;
  int rv;

  /* create context and io layers */
  ctx=GWEN_XmlCtxStore_new(NULL, flags, 0, 10000);
  io=GWEN_Io_LayerMemory_new(buf);
  GWEN_Io_Manager_RegisterLayer(io);

  /* write data to stream */
  rv=GWEN_XMLNode_WriteToStream(n, ctx, io);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Io_Layer_Disconnect(io, GWEN_IO_REQUEST_FLAGS_FORCE, 0, 1000);
    GWEN_Io_Layer_free(io);
    GWEN_XmlCtx_free(ctx);
    return rv;
  }

  /* close file */
  rv=GWEN_Io_Layer_DisconnectRecursively(io, NULL, 0, 0, 30000);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Io_Layer_Disconnect(io, GWEN_IO_REQUEST_FLAGS_FORCE, 0, 1000);
    GWEN_Io_Layer_free(io);
    GWEN_XmlCtx_free(ctx);
    return rv;
  }

  GWEN_Io_Layer_free(io);
  GWEN_XmlCtx_free(ctx);

  return 0;
}








int GWEN_XML__ReadData(GWEN_XML_CONTEXT *ctx,
		       GWEN_FAST_BUFFER *fb,
		       uint32_t flags){
  int chr;
  unsigned char uc;
  GWEN_BUFFER *dbuf;

  dbuf=GWEN_Buffer_new(0, 256, 0, 1);

  for (;;) {
    GWEN_FASTBUFFER_PEEKBYTE(fb, chr);
    if (chr<0) {
      if (chr==GWEN_ERROR_EOF)
	break;
      else {
	GWEN_Buffer_free(dbuf);
	return chr;
      }
    }

    uc=(unsigned char) chr;
    if (uc=='<')
      break;
    fb->bufferReadPos++;
    GWEN_Buffer_AppendByte(dbuf, uc);
  }

  if (GWEN_Buffer_GetUsedBytes(dbuf)) {
    int rv;

    rv=GWEN_XmlCtx_AddData(ctx, GWEN_Buffer_GetStart(dbuf));
    if (rv) {
      GWEN_Buffer_free(dbuf);
      return rv;
    }
  }
  GWEN_Buffer_free(dbuf);

  return 0;
}



int GWEN_XML__ReadTag(GWEN_XML_CONTEXT *ctx,
		      GWEN_FAST_BUFFER *fb,
		      uint32_t flags){
  int chr;
  unsigned char uc=0;
  GWEN_BUFFER *dbuf;
  int rv;

  dbuf=GWEN_Buffer_new(0, 256, 0, 1);

  /* skip blanks */
  for (;;) {
    GWEN_FASTBUFFER_READBYTE(fb, chr);
    if (chr<0) {
      GWEN_Buffer_free(dbuf);
      return chr;
    }
    uc=(unsigned char) chr;
    if (uc>32)
      break;
  }

  if (uc=='/') {
    /* read end tag */
    GWEN_Buffer_AppendByte(dbuf, uc);
    for (;;) {
      GWEN_FASTBUFFER_READBYTE(fb, chr);
      if (chr<0) {
	GWEN_Buffer_free(dbuf);
	return chr;
      }
      uc=(unsigned char) chr;
      if (uc=='>' || uc<33)
	break;

      GWEN_Buffer_AppendByte(dbuf, uc);
    }

    rv=GWEN_XmlCtx_StartTag(ctx, GWEN_Buffer_GetStart(dbuf));
    if (rv) {
      GWEN_Buffer_free(dbuf);
      return rv;
    }
    if (uc!='>') {
      for (;;) {
	/* skip blanks, expect '>' */
	GWEN_FASTBUFFER_READBYTE(fb, chr);
	if (chr<0) {
	  GWEN_Buffer_free(dbuf);
	  return chr;
	}
	uc=(unsigned char) chr;
	if (uc>32)
	  break;
      }
    }
    if (uc!='>') {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected character");
      GWEN_Buffer_free(dbuf);
      return GWEN_ERROR_BAD_DATA;
    }

    /* tag finished */
    rv=GWEN_XmlCtx_EndTag(ctx, 0);
    if (rv) {
      GWEN_Buffer_free(dbuf);
      return rv;
    }
    GWEN_Buffer_free(dbuf);
    return 0;
  }
  else if (uc=='!') {
    /* check for comment */
    GWEN_FASTBUFFER_PEEKBYTE(fb, chr);
    if (chr<0) {
      GWEN_Buffer_free(dbuf);
      return chr;
    }
    uc=(unsigned char) chr;
    if (uc=='-') {
      fb->bufferReadPos++;
      GWEN_FASTBUFFER_PEEKBYTE(fb, chr);
      if (chr<0) {
	GWEN_Buffer_free(dbuf);
	return chr;
      }
      uc=(unsigned char) chr;
      if (uc=='-') {
	GWEN_BUFFER *cbuf;

	/* found comment */
	fb->bufferReadPos++;
	cbuf=GWEN_Buffer_new(0, 256, 0, 1);
	for (;;) {
	  GWEN_FASTBUFFER_READBYTE(fb, chr);
	  if (chr<0) {
	    GWEN_Buffer_free(cbuf);
	    GWEN_Buffer_free(dbuf);
	    return chr;
	  }
	  uc=(unsigned char) chr;
	  GWEN_Buffer_AppendByte(cbuf, uc);
	  if (GWEN_Buffer_GetUsedBytes(cbuf)>2) {
	    char *p;

	    p=GWEN_Buffer_GetStart(cbuf);
	    p+=GWEN_Buffer_GetUsedBytes(cbuf)-3;
	    if (strcmp(p, "-->")==0) {
	      *p=0;
	      rv=GWEN_XmlCtx_AddComment(ctx, GWEN_Buffer_GetStart(cbuf));
	      if (rv) {
		GWEN_Buffer_free(cbuf);
		GWEN_Buffer_free(dbuf);
                return rv;
	      }
	      GWEN_Buffer_free(cbuf);
	      GWEN_Buffer_free(dbuf);
	      return 0;
	    }
	  }
	}
      }
      else {
	GWEN_Buffer_AppendString(dbuf, "!-");
      }
    }
    else
      uc='!';
  }

  /* read name */
  for (;;) {
    if (uc==' ' || uc=='>' || uc=='/')
      break;
    else if (GWEN_Buffer_GetUsedBytes(dbuf)) {
      unsigned char fc;

      fc=*GWEN_Buffer_GetStart(dbuf);
      if ((fc=='!' && uc=='!') || (fc=='?' && uc=='?')) {
	GWEN_FASTBUFFER_PEEKBYTE(fb, chr);
	if (chr<0) {
	  GWEN_Buffer_free(dbuf);
	  return chr;
	}
	uc=(unsigned char) chr;
	if (uc=='>') {
	  fb->bufferReadPos++;
	  break;
	}
      }
    }

    GWEN_Buffer_AppendByte(dbuf, uc);

    GWEN_FASTBUFFER_READBYTE(fb, chr);
    if (chr<0) {
      if (chr==GWEN_ERROR_EOF) {
	GWEN_Buffer_free(dbuf);
	return chr;
      }
      else {
	GWEN_Buffer_free(dbuf);
	return chr;
      }
    }

    uc=(unsigned char) chr;
  }

  /* tag started */
  if (GWEN_Buffer_GetUsedBytes(dbuf)==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Element name missing");
    GWEN_Buffer_free(dbuf);
    return GWEN_ERROR_BAD_DATA;
  }

  rv=GWEN_XmlCtx_StartTag(ctx, GWEN_Buffer_GetStart(dbuf));
  if (rv) {
    GWEN_Buffer_free(dbuf);
    return rv;
  }

  if (uc=='/' || uc=='?' || uc=='!') {
    GWEN_FASTBUFFER_PEEKBYTE(fb, chr);
    if (chr<0) {
      GWEN_Buffer_free(dbuf);
      return chr;
    }
    uc=(unsigned char) chr;
    if (uc=='>') {
      fb->bufferReadPos++;
      rv=GWEN_XmlCtx_EndTag(ctx, 1);
      if (rv) {
	GWEN_Buffer_free(dbuf);
	return rv;
      }
      GWEN_Buffer_free(dbuf);
      /* tag finished */
      return 0;
    }
  }

  if (uc=='>') {
    rv=GWEN_XmlCtx_EndTag(ctx, 0);
    if (rv) {
      GWEN_Buffer_free(dbuf);
      return rv;
    }
    GWEN_Buffer_free(dbuf);
    /* tag finished */
    return 0;
  }

  /* read attributes */
  for (;;) {
    GWEN_BUFFER *nbuf;
    GWEN_BUFFER *vbuf=NULL;

    nbuf=GWEN_Buffer_new(0, 256, 0, 1);

    /* skip blanks */
    for (;;) {
      GWEN_FASTBUFFER_READBYTE(fb, chr);
      if (chr<0) {
	GWEN_Buffer_free(nbuf);
	GWEN_Buffer_free(dbuf);
	return chr;
      }
      uc=(unsigned char) chr;
      if (uc>32)
	break;
    }

    /* read attribute name */
    for (;;) {
      if (uc=='/' || uc=='!' || uc=='?' || uc=='=' || uc=='>')
	break;
      GWEN_Buffer_AppendByte(nbuf, uc);

      GWEN_FASTBUFFER_READBYTE(fb, chr);
      if (chr<0) {
	GWEN_Buffer_free(nbuf);
	GWEN_Buffer_free(dbuf);
	return chr;
      }
      uc=(unsigned char) chr;
    }

    if (GWEN_Buffer_GetUsedBytes(nbuf)) {
      if (uc=='=') {
	/* read attribute value if there is an equation mark */
	int inQuote=0;
  
	vbuf=GWEN_Buffer_new(0, 256, 0, 1);
	for (;;) {
	  GWEN_FASTBUFFER_READBYTE(fb, chr);
	  if (chr<0) {
	    GWEN_Buffer_free(nbuf);
	    GWEN_Buffer_free(dbuf);
	    return chr;
	  }
	  uc=(unsigned char) chr;
	  if (uc=='"') {
	    if (inQuote) {
	      inQuote=0;
	      break;
	    }
	    else
	      inQuote=1;
	  }
	  else {
	    if (!inQuote) {
	      if (uc=='>' || uc<33)
		break;
	      else if (uc=='<') {
		DBG_ERROR(GWEN_LOGDOMAIN,
			  "Nested element definitions");
		GWEN_Buffer_free(vbuf);
		GWEN_Buffer_free(nbuf);
		GWEN_Buffer_free(dbuf);
		return GWEN_ERROR_BAD_DATA;
	      }
	      else if (GWEN_Buffer_GetUsedBytes(dbuf)) {
		if (uc=='/' || uc=='!' || uc=='?') {
		  unsigned char tc;

		  GWEN_FASTBUFFER_PEEKBYTE(fb, chr);
		  if (chr<0) {
		    GWEN_Buffer_free(vbuf);
		    GWEN_Buffer_free(nbuf);
		    GWEN_Buffer_free(dbuf);
		    return chr;
		  }
		  tc=(unsigned char) chr;
		  if (tc=='>') {
		    break;
		  }
		}
	      }
	    }
	    GWEN_Buffer_AppendByte(vbuf, uc);
	  }
	}
	if (inQuote) {
	  DBG_ERROR(GWEN_LOGDOMAIN, "No matching number of quote chars");
	  GWEN_Buffer_free(vbuf);
	  GWEN_Buffer_free(nbuf);
	  GWEN_Buffer_free(dbuf);
	  return GWEN_ERROR_BAD_DATA;
	}

	if (GWEN_Buffer_GetUsedBytes(vbuf)==0) {
	  GWEN_Buffer_free(vbuf);
	  vbuf=NULL;
	}
      }
      rv=GWEN_XmlCtx_AddAttr(ctx,
			     GWEN_Buffer_GetStart(nbuf),
			     vbuf?GWEN_Buffer_GetStart(vbuf):NULL);
      if (rv) {
	GWEN_Buffer_free(vbuf);
	GWEN_Buffer_free(nbuf);
	GWEN_Buffer_free(dbuf);
	return rv;
      }
    }

    GWEN_Buffer_free(vbuf);
    GWEN_Buffer_free(nbuf);

    if (uc=='>' || uc=='?' || uc=='!' || uc=='/')
      break;
  }

  if (uc=='?' || uc=='!' || uc=='/') {
    GWEN_FASTBUFFER_PEEKBYTE(fb, chr);
    if (chr<0) {
      GWEN_Buffer_free(dbuf);
      return chr;
    }
    uc=(unsigned char) chr;
    if (uc=='>') {
      DBG_VERBOUS(GWEN_LOGDOMAIN, "Ending tag [%s]", GWEN_Buffer_GetStart(dbuf));
      fb->bufferReadPos++;
      rv=GWEN_XmlCtx_EndTag(ctx, 1);
      if (rv) {
	GWEN_Buffer_free(dbuf);
	return rv;
      }
      GWEN_Buffer_free(dbuf);
      /* tag finished */
      return 0;
    }
  }
  else if (uc=='>') {
    rv=GWEN_XmlCtx_EndTag(ctx, 0);
    if (rv) {
      GWEN_Buffer_free(dbuf);
      return rv;
    }
    GWEN_Buffer_free(dbuf);
    /* tag finished */
    return 0;
  }

  DBG_ERROR(GWEN_LOGDOMAIN,
	    "Internal error: Should never reach this point");
  GWEN_Buffer_free(dbuf);
  return GWEN_ERROR_INTERNAL;
}




int GWEN_XML_ReadFromFastBuffer(GWEN_XML_CONTEXT *ctx, GWEN_FAST_BUFFER *fb){
  int oks=0;
  int startingDepth;

  startingDepth=GWEN_XmlCtx_GetDepth(ctx);

  GWEN_XmlCtx_ResetFinishedElement(ctx);
  for (;;) {
    int rv;

    GWEN_FASTBUFFER_PEEKBYTE(fb, rv);
    if (rv<0) {
      if (rv!=GWEN_ERROR_EOF || !oks) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }
      return 0;
    }

    rv=GWEN_XML__ReadData(ctx, fb, GWEN_XmlCtx_GetFlags(ctx));
    if (rv) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    oks=1;

    GWEN_FASTBUFFER_PEEKBYTE(fb, rv);
    if (rv<0) {
      if (rv!=GWEN_ERROR_EOF || !oks ||
	  (GWEN_XmlCtx_GetDepth(ctx)!=startingDepth)) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }
      return 0;
    }
    else if (rv=='<') {
      fb->bufferReadPos++;
      rv=GWEN_XML__ReadTag(ctx, fb, GWEN_XmlCtx_GetFlags(ctx));
      if (rv) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }
      oks=1;
    }

    if (GWEN_XmlCtx_GetFinishedElement(ctx) &&
	GWEN_XmlCtx_GetDepth(ctx)==startingDepth)
      break;
  }

  if (GWEN_XmlCtx_GetDepth(ctx)!=startingDepth) {
    DBG_ERROR(GWEN_LOGDOMAIN,
	      "Not on same level where we started...(%d!=%d)",
	      GWEN_XmlCtx_GetDepth(ctx), startingDepth);
  }

  return 0;
}



int GWEN_XML__ReadAllFromIo(GWEN_XML_CONTEXT *ctx, GWEN_IO_LAYER *io){
  GWEN_FAST_BUFFER *fb;
  int oks=0;

  fb=GWEN_FastBuffer_new(GWEN_XML_BUFFERSIZE,
			 io,
			 GWEN_XmlCtx_GetGuiId(ctx),
			 GWEN_XmlCtx_GetTimeout(ctx));
  assert(fb);
  for (;;) {
    int rv;

    rv=GWEN_XML_ReadFromFastBuffer(ctx, fb);
    if (rv<0) {
      if (rv==GWEN_ERROR_EOF && oks)
	break;
      else {
	DBG_INFO(GWEN_LOGDOMAIN, "here");
	GWEN_FastBuffer_free(fb);
	return rv;
      }
    }
    oks=1;
  }

  GWEN_FastBuffer_free(fb);
  return 0;
}



int GWEN_XML_ReadFromIo(GWEN_XML_CONTEXT *ctx, GWEN_IO_LAYER *io){
  GWEN_FAST_BUFFER *fb;
  int rv;

  fb=GWEN_FastBuffer_new(GWEN_XML_BUFFERSIZE,
			 io,
			 GWEN_XmlCtx_GetGuiId(ctx),
			 GWEN_XmlCtx_GetTimeout(ctx));
  assert(fb);
  rv=GWEN_XML_ReadFromFastBuffer(ctx, fb);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    GWEN_FastBuffer_free(fb);
    return rv;
  }

  GWEN_FastBuffer_free(fb);
  return 0;
}




int GWEN_XML_ReadFile(GWEN_XMLNODE *n, const char *filepath, uint32_t flags) {
  GWEN_XML_CONTEXT *ctx;
  GWEN_IO_LAYER *io;
  int fd;
  int rv;

  fd=open(filepath, O_RDONLY);
  if (fd==-1) {
    DBG_ERROR(0, "open(%s): %s", filepath, strerror(errno));
    return GWEN_ERROR_IO;
  }

  ctx=GWEN_XmlCtxStore_new(n, flags, 0, 10000);
  io=GWEN_Io_LayerFile_new(fd, -1);
  GWEN_Io_Manager_RegisterLayer(io);

  rv=GWEN_XML__ReadAllFromIo(ctx, io);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Io_Layer_Disconnect(io, GWEN_IO_REQUEST_FLAGS_FORCE, 0, 1000);
    GWEN_Io_Layer_free(io);
    GWEN_XmlCtx_free(ctx);
    return rv;
  }

  GWEN_Io_Layer_Disconnect(io, GWEN_IO_REQUEST_FLAGS_FORCE, 0, 1000);

  GWEN_Io_Layer_free(io);
  GWEN_XmlCtx_free(ctx);

  return 0;
}



GWEN_XMLNODE *GWEN_XMLNode_fromString(const char *s, int len, uint32_t flags) {
  GWEN_XML_CONTEXT *ctx;
  GWEN_IO_LAYER *io;
  GWEN_XMLNODE *n;
  int rv;

  n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "doc");
  ctx=GWEN_XmlCtxStore_new(n, flags, 0, 10000);
  io=GWEN_Io_LayerMemory_fromString((const uint8_t*)s, len);
  GWEN_Io_Manager_RegisterLayer(io);

  rv=GWEN_XML__ReadAllFromIo(ctx, io);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Io_Layer_Disconnect(io, GWEN_IO_REQUEST_FLAGS_FORCE, 0, 1000);
    GWEN_Io_Layer_free(io);
    GWEN_XmlCtx_free(ctx);
    GWEN_XMLNode_free(n);
    return NULL;
  }

  GWEN_Io_Layer_Disconnect(io, GWEN_IO_REQUEST_FLAGS_FORCE, 0, 1000);

  GWEN_Io_Layer_free(io);
  GWEN_XmlCtx_free(ctx);

  return n;
}




