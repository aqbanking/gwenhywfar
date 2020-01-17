/***************************************************************************
 copyright   : (C) 2020 by Martin Preuss
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



static int GWEN_XMLNode__WriteIndents(GWEN_FAST_BUFFER *fb, int ind)
{
  int i;
  int rv=0;

  for (i=0; i<ind; i++) {
    GWEN_FASTBUFFER_WRITEBYTE(fb, rv, ' ');
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }
  return 0;
}



static int GWEN_XMLNode__WriteNamespacesToStream(const GWEN_XMLNODE *n, GWEN_FAST_BUFFER *fb)
{
  GWEN_XMLNODE_NAMESPACE *ns;

  ns=GWEN_XMLNode_NameSpace_List_First(n->nameSpaces);
  while (ns) {
    const char *name;
    const char *url;
    int rv;

    name=GWEN_XMLNode_NameSpace_GetName(ns);
    url=GWEN_XMLNode_NameSpace_GetUrl(ns);
    GWEN_FASTBUFFER_WRITEBYTE(fb, rv, ' ');
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    GWEN_FASTBUFFER_WRITEFORCED(fb, rv, "xmlns", -1);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    if (name && *name) {
      GWEN_FASTBUFFER_WRITEFORCED(fb, rv, ":", -1);
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }
      GWEN_FASTBUFFER_WRITEFORCED(fb, rv, name, -1);
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }
    }
    GWEN_FASTBUFFER_WRITEFORCED(fb, rv, "=\"", -1);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    if (url) {
      GWEN_FASTBUFFER_WRITEFORCED(fb, rv, url, -1);
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }
    }
    GWEN_FASTBUFFER_WRITEFORCED(fb, rv, "\"", -1);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    ns=GWEN_XMLNode_NameSpace_List_Next(ns);
  }

  return 0;
}



static int GWEN_XMLNode__WritePropertiesToStream(const GWEN_XMLNODE *n, GWEN_FAST_BUFFER *fb, const char *encoding)
{
  GWEN_XMLPROPERTY *p;

  p=n->properties;
  if (p) {
    GWEN_BUFFER *buf;
    int rv=0;

    buf=GWEN_Buffer_new(0, 256, 0, 1);
    while (p) {
      GWEN_FASTBUFFER_WRITEBYTE(fb, rv, ' ');
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	GWEN_Buffer_free(buf);
	return rv;
      }
      GWEN_FASTBUFFER_WRITEFORCED(fb, rv, p->name, -1);
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	GWEN_Buffer_free(buf);
	return rv;
      }
      if (p->value) {
	GWEN_FASTBUFFER_WRITEFORCED(fb, rv, "=\"", -1);
	if (rv<0) {
	  DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	  GWEN_Buffer_free(buf);
	  return rv;
	}
	rv=GWEN_Text_ConvertCharset("UTF-8", encoding, p->value, strlen(p->value), buf);
	if (rv<0) {
	  DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	  GWEN_Buffer_free(buf);
	  return rv;
	}
	GWEN_FASTBUFFER_WRITEFORCED(fb, rv, GWEN_Buffer_GetStart(buf), -1);
	if (rv<0) {
	  DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	  GWEN_Buffer_free(buf);
	  return rv;
	}
	GWEN_Buffer_Reset(buf);
	GWEN_FASTBUFFER_WRITEFORCED(fb, rv, "\"", -1);
	if (rv<0) {
	  DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	  GWEN_Buffer_free(buf);
	  return rv;
	}
      }
      p=p->next;
    }
  }

  return 0;
}



static int GWEN_XMLNode__WriteTagToStream(const GWEN_XMLNODE *n,
					  GWEN_FAST_BUFFER *fb,
					  uint32_t flags,
					  const char *encoding,
					  unsigned int ind)
{
  int rv;
  int simpleTag=0;

  if (!(flags & GWEN_XML_FLAGS_SIMPLE)) {
    if (flags & GWEN_XML_FLAGS_INDENT) {
      rv=GWEN_XMLNode__WriteIndents(fb, ind);
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }
    }
  }

  /* write element opening ("<NAME") */
  if (n->data) {
    GWEN_FASTBUFFER_WRITEBYTE(fb, rv, '<');
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    GWEN_FASTBUFFER_WRITEFORCED(fb, rv, n->data, -1);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }
  else {
    GWEN_FASTBUFFER_WRITEFORCED(fb, rv, "<UNKNOWN", -1);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  /* write namespaces */
  if (flags & GWEN_XML_FLAGS_HANDLE_NAMESPACES) {
    rv=GWEN_XMLNode__WriteNamespacesToStream(n, fb);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  /* write properties */
  rv=GWEN_XMLNode__WritePropertiesToStream(n, fb, encoding);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }


  /* write element closing (">) */
  if (n->data) {
    if (n->data[0]=='?') {
      simpleTag=1;
      GWEN_FASTBUFFER_WRITEBYTE(fb, rv, '?');
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }
    }
    else if (n->data[0]=='!') {
      simpleTag=1;
    }
  }
  if (flags & GWEN_XML_FLAGS_SIMPLE) {
    GWEN_FASTBUFFER_WRITEFORCED(fb, rv, ">", -1);
  }
  else {
    GWEN_FASTBUFFER_WRITELINE(fb, rv, ">");
  }
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* probably write children */
  if (!simpleTag) {
    int hasSubTags;
    GWEN_XMLNODE *c;

    hasSubTags=(GWEN_XMLNode_GetFirstTag(n)!=NULL);
    if (hasSubTags) {
      GWEN_FASTBUFFER_WRITELINE(fb, rv, "");
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }
    }

    c=GWEN_XMLNode_GetChild(n);
    while (c) {
      rv=GWEN_XMLNode__WriteToStream(c, fb, flags, encoding, ind+2);
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }
      c=GWEN_XMLNode_Next(c);
    }

    if (!(flags & GWEN_XML_FLAGS_SIMPLE)) {
      if (flags & GWEN_XML_FLAGS_INDENT) {
	rv=GWEN_XMLNode__WriteIndents(fb, ind);
	if (rv<0) {
	  DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	  return rv;
	}
      }
    }

    /* write closing tag ("</NAME>") */
    if (n->data) {
      GWEN_FASTBUFFER_WRITEFORCED(fb, rv, "</", -1);
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }
      GWEN_FASTBUFFER_WRITEFORCED(fb, rv, n->data, -1);
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }
      if (flags & GWEN_XML_FLAGS_SIMPLE) {
#if 0
	if (!hasSubTags) {
	  GWEN_FASTBUFFER_WRITELINE(fb, rv, ">");
	}
	else {
	  GWEN_FASTBUFFER_WRITEFORCED(fb, rv, ">", -1);
	}
#else
	GWEN_FASTBUFFER_WRITELINE(fb, rv, ">");
#endif
      }
      else {
	GWEN_FASTBUFFER_WRITELINE(fb, rv, ">");
      }
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }
    }
    else {
      GWEN_FASTBUFFER_WRITELINE(fb, rv, "</UNKNOWN>");
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }
    }
  }

  return 0;
}



static int GWEN_XMLNode__WriteDataToStream(const GWEN_XMLNODE *n,
					   GWEN_FAST_BUFFER *fb,
					   uint32_t flags,
					   const char *encoding,
					   unsigned int ind)
{

  if (n->data) {
    GWEN_BUFFER *buf;
    int rv=0;

    if (!(flags & GWEN_XML_FLAGS_SIMPLE)) {
      if (flags & GWEN_XML_FLAGS_INDENT) {
	rv=GWEN_XMLNode__WriteIndents(fb, ind);
	if (rv<0) {
	  DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	  return rv;
	}
      }
    }

    buf=GWEN_Buffer_new(0, 256, 0, 1);
    rv=GWEN_Text_ConvertCharset("UTF-8", encoding, n->data, strlen(n->data), buf);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(buf);
      return rv;
    }
    GWEN_FASTBUFFER_WRITEFORCED(fb, rv, GWEN_Buffer_GetStart(buf), -1);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(buf);
      return rv;
    }
    GWEN_Buffer_free(buf);
    if (!(flags & GWEN_XML_FLAGS_SIMPLE)) {
      GWEN_FASTBUFFER_WRITELINE(fb, rv, "");
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }
    }
  }

  return 0;
}



static int GWEN_XMLNode__WriteCommentToStream(const GWEN_XMLNODE *n,
					      GWEN_FAST_BUFFER *fb,
					      uint32_t flags,
					      const char *encoding,
					      unsigned int ind)
{

  if (n->data) {
    GWEN_BUFFER *buf;
    int rv=0;

    if (!(flags & GWEN_XML_FLAGS_SIMPLE)) {
      if (flags & GWEN_XML_FLAGS_INDENT) {
	rv=GWEN_XMLNode__WriteIndents(fb, ind);
	if (rv<0) {
	  DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	  return rv;
	}
      }
    }

    GWEN_FASTBUFFER_WRITEFORCED(fb, rv, "<!--", -1);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    buf=GWEN_Buffer_new(0, 256, 0, 1);
    rv=GWEN_Text_ConvertCharset("UTF-8", encoding, n->data, strlen(n->data), buf);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(buf);
      return rv;
    }
    GWEN_FASTBUFFER_WRITEFORCED(fb, rv, GWEN_Buffer_GetStart(buf), -1);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(buf);
      return rv;
    }
    GWEN_Buffer_free(buf);

    GWEN_FASTBUFFER_WRITEFORCED(fb, rv, "-->", -1);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    if (!(flags & GWEN_XML_FLAGS_SIMPLE)) {
      GWEN_FASTBUFFER_WRITELINE(fb, rv, "");
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }
    }
  }

  return 0;
}



static int GWEN_XMLNode__WriteToStream(const GWEN_XMLNODE *n,
				       GWEN_FAST_BUFFER *fb,
				       uint32_t flags,
				       const char *encoding,
				       unsigned int ind)
{
  int rv;

  assert(n);

  if (n->type==GWEN_XMLNodeTypeTag) {
    rv=GWEN_XMLNode__WriteTagToStream(n, fb, flags, encoding, ind);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }
  else if (n->type==GWEN_XMLNodeTypeData) {
    rv=GWEN_XMLNode__WriteDataToStream(n, fb, flags, encoding, ind);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }
  else if (n->type==GWEN_XMLNodeTypeComment) {
    if (flags & GWEN_XML_FLAGS_HANDLE_COMMENTS) {
      rv=GWEN_XMLNode__WriteCommentToStream(n, fb, flags, encoding, ind);
      if (rv<0) {
	DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }
    }
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Unknown tag type (%d)", n->type);
  }

  return 0;
}



int GWEN_XMLNode_WriteToStream(const GWEN_XMLNODE *n,
                               GWEN_XML_CONTEXT *ctx,
                               GWEN_SYNCIO *sio)
{
  const GWEN_XMLNODE *nn;
  const GWEN_XMLNODE *nchild;
  const GWEN_XMLNODE *nheader;
  uint32_t flags;
  GWEN_FAST_BUFFER *fb;
  int rv;

  flags=GWEN_XmlCtx_GetFlags(ctx);
  nchild=GWEN_XMLNode_GetChild(n);
  nheader=GWEN_XMLNode_GetHeader(n);

  fb=GWEN_FastBuffer_new(512, sio);

  if (nheader && (flags & GWEN_XML_FLAGS_HANDLE_HEADERS)) {

    nn=nheader;
    while (nn) {
      const GWEN_XMLNODE *next;

      rv=GWEN_XMLNode__WriteToStream(nn, fb, flags,
                                     GWEN_XmlCtx_GetEncoding(ctx), 0);
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
      if (strcmp(GWEN_XMLNode_GetData(nn), "?xml")==0) {
        const char *encoding;

        encoding=GWEN_XMLNode_GetProperty(nn, "encoding", NULL);
        if (encoding) {
          if (strcasecmp(encoding, "UTF-8")==0 ||
              strcasecmp(encoding, "UTF8")==0)
            encoding=NULL;
          GWEN_XmlCtx_SetEncoding(ctx, encoding);
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
  while (nn) {
    const GWEN_XMLNODE *next;

    if (GWEN_XMLNode__WriteToStream(nn, fb, flags,
                                    GWEN_XmlCtx_GetEncoding(ctx), 0))
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
                           uint32_t flags)
{
  GWEN_XML_CONTEXT *ctx;
  GWEN_SYNCIO *sio;
  int rv;

  sio=GWEN_SyncIo_File_new(fname, GWEN_SyncIo_File_CreationMode_CreateAlways);
  GWEN_SyncIo_AddFlags(sio,
                       GWEN_SYNCIO_FILE_FLAGS_READ | GWEN_SYNCIO_FILE_FLAGS_WRITE |
                       GWEN_SYNCIO_FILE_FLAGS_UREAD | GWEN_SYNCIO_FILE_FLAGS_UWRITE);
  rv=GWEN_SyncIo_Connect(sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_free(sio);
    return rv;
  }

  /* create context and io layers */
  ctx=GWEN_XmlCtxStore_new(NULL, flags);

  /* write data to stream */
  rv=GWEN_XMLNode_WriteToStream(n, ctx, sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    GWEN_XmlCtx_free(ctx);
    return rv;
  }

  /* close file */
  GWEN_SyncIo_Disconnect(sio);
  GWEN_SyncIo_free(sio);

  GWEN_XmlCtx_free(ctx);

  return 0;
}



int GWEN_XMLNode_toBuffer(const GWEN_XMLNODE *n, GWEN_BUFFER *buf, uint32_t flags)
{
  GWEN_XML_CONTEXT *ctx;
  GWEN_SYNCIO *sio;
  int rv;

  sio=GWEN_SyncIo_Memory_new(buf, 0);

  /* create context and io layers */
  ctx=GWEN_XmlCtxStore_new(NULL, flags);

  /* write data to stream */
  rv=GWEN_XMLNode_WriteToStream(n, ctx, sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_free(sio);
    GWEN_XmlCtx_free(ctx);
    return rv;
  }

  GWEN_SyncIo_free(sio);

  GWEN_XmlCtx_free(ctx);

  return 0;
}








int GWEN_XML__ReadData(GWEN_XML_CONTEXT *ctx,
                       GWEN_FAST_BUFFER *fb,
                       GWEN_UNUSED uint32_t flags)
{
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
    uint32_t len;
    char *s;

    len=GWEN_Buffer_GetUsedBytes(dbuf);
    s=strdup(GWEN_Buffer_GetStart(dbuf));
    assert(s);
    GWEN_Buffer_Reset(dbuf);
    rv=GWEN_Text_ConvertCharset(GWEN_XmlCtx_GetEncoding(ctx), "UTF-8",
                                s, len, dbuf);
    free(s);
    if (rv) {
      GWEN_Buffer_free(dbuf);
      return rv;
    }
    s=GWEN_Buffer_GetStart(dbuf);
    if (*s) {
      rv=GWEN_XmlCtx_AddData(ctx, s);
      if (rv) {
        GWEN_Buffer_free(dbuf);
        return rv;
      }
    }
  }
  GWEN_Buffer_free(dbuf);

  return 0;
}



int GWEN_XML__ReadTag(GWEN_XML_CONTEXT *ctx,
                      GWEN_FAST_BUFFER *fb,
                      GWEN_UNUSED uint32_t flags,
                      GWEN_BUFFER *dbuf)
{
  int chr;
  unsigned char uc=0;
  int rv;

  /* skip blanks */
  for (;;) {
    GWEN_FASTBUFFER_READBYTE(fb, chr);
    if (chr<0) {
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
        return chr;
      }
      uc=(unsigned char) chr;
      if (uc=='>' || uc<33)
        break;

      GWEN_Buffer_AppendByte(dbuf, uc);
    }

    rv=GWEN_XmlCtx_StartTag(ctx, GWEN_Buffer_GetStart(dbuf));
    if (rv) {
      return rv;
    }
    if (uc!='>') {
      for (;;) {
        /* skip blanks, expect '>' */
        GWEN_FASTBUFFER_READBYTE(fb, chr);
        if (chr<0) {
          return chr;
        }
        uc=(unsigned char) chr;
        if (uc>32)
          break;
      }
    }
    if (uc!='>') {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected character");
      DBG_ERROR(GWEN_LOGDOMAIN, "Data so far:");
      GWEN_Buffer_Dump(dbuf, 2);

      return GWEN_ERROR_BAD_DATA;
    }

    /* tag finished */
    rv=GWEN_XmlCtx_EndTag(ctx, 0);
    if (rv) {
      return rv;
    }
    return 0;
  }
  else if (uc=='!') {
    /* check for comment */
    GWEN_FASTBUFFER_PEEKBYTE(fb, chr);
    if (chr<0) {
      return chr;
    }
    uc=(unsigned char) chr;
    if (uc=='-') {
      fb->bufferReadPos++;
      GWEN_FASTBUFFER_PEEKBYTE(fb, chr);
      if (chr<0) {
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
            return chr;
          }
          uc=(unsigned char) chr;
          GWEN_Buffer_AppendByte(cbuf, uc);
          if (GWEN_Buffer_GetUsedBytes(cbuf)>2) {
            char *p;

            p=GWEN_Buffer_GetStart(cbuf);
            p+=GWEN_Buffer_GetUsedBytes(cbuf)-3;
            if (strcmp(p, "-->")==0) {
              uint32_t len;

              *p=0;
              len=GWEN_Buffer_GetUsedBytes(cbuf)-3;
              p=strdup(GWEN_Buffer_GetStart(cbuf));
              assert(p);
              GWEN_Buffer_Reset(cbuf);
              rv=GWEN_Text_ConvertCharset(GWEN_XmlCtx_GetEncoding(ctx), "UTF-8",
                                          p, len, cbuf);
              free(p);
              if (rv) {
                GWEN_Buffer_free(cbuf);
                return rv;
              }
              rv=GWEN_XmlCtx_AddComment(ctx, GWEN_Buffer_GetStart(cbuf));
              if (rv) {
                GWEN_Buffer_free(cbuf);
                return rv;
              }
              GWEN_Buffer_free(cbuf);
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
        return chr;
      }
      else {
        return chr;
      }
    }

    uc=(unsigned char) chr;
  }

  /* tag started */
  if (GWEN_Buffer_GetUsedBytes(dbuf)==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Element name missing");
    return GWEN_ERROR_BAD_DATA;
  }

  rv=GWEN_XmlCtx_StartTag(ctx, GWEN_Buffer_GetStart(dbuf));
  if (rv) {
    return rv;
  }

  if (uc=='/' || uc=='?' || uc=='!') {
    GWEN_FASTBUFFER_PEEKBYTE(fb, chr);
    if (chr<0) {
      return chr;
    }
    uc=(unsigned char) chr;
    if (uc=='>') {
      fb->bufferReadPos++;
      rv=GWEN_XmlCtx_EndTag(ctx, 1);
      if (rv) {
        return rv;
      }
      /* tag finished */
      return 0;
    }
  }

  if (uc=='>') {
    rv=GWEN_XmlCtx_EndTag(ctx, 0);
    if (rv) {
      return rv;
    }
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
        return chr;
      }
      uc=(unsigned char) chr;
    }

    if (GWEN_Buffer_GetUsedBytes(nbuf)) {
      if (uc=='=') {
        /* read attribute value if there is an equation mark */
        int inQuote=0;
        uint32_t len;

        vbuf=GWEN_Buffer_new(0, 256, 0, 1);
        for (;;) {
          GWEN_FASTBUFFER_READBYTE(fb, chr);
          if (chr<0) {
            GWEN_Buffer_free(nbuf);
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
                return GWEN_ERROR_BAD_DATA;
              }
              else if (GWEN_Buffer_GetUsedBytes(dbuf)) {
                if (uc=='/' || uc=='!' || uc=='?') {
                  unsigned char tc;

                  GWEN_FASTBUFFER_PEEKBYTE(fb, chr);
                  if (chr<0) {
                    GWEN_Buffer_free(vbuf);
                    GWEN_Buffer_free(nbuf);
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
          return GWEN_ERROR_BAD_DATA;
        }

        len=GWEN_Buffer_GetUsedBytes(vbuf);
        if (len==0) {
          GWEN_Buffer_free(vbuf);
          vbuf=NULL;
        }
        else {
          char *s;

          s=strdup(GWEN_Buffer_GetStart(vbuf));
          GWEN_Buffer_Reset(vbuf);
          rv=GWEN_Text_ConvertCharset(GWEN_XmlCtx_GetEncoding(ctx), "UTF-8",
                                      s, len, vbuf);
          free(s);
          if (rv) {
            GWEN_Buffer_free(vbuf);
            GWEN_Buffer_free(nbuf);
            return rv;
          }
        }
      }
      rv=GWEN_XmlCtx_AddAttr(ctx,
                             GWEN_Buffer_GetStart(nbuf),
                             vbuf?GWEN_Buffer_GetStart(vbuf):NULL);
      if (rv) {
        GWEN_Buffer_free(vbuf);
        GWEN_Buffer_free(nbuf);
        return rv;
      }
    }

    GWEN_Buffer_free(vbuf);
    GWEN_Buffer_free(nbuf);

    if (uc=='>' || uc=='?' || uc=='!' || uc=='/')
      break;
  }

  if (uc=='?' || uc=='!' || uc=='/') {
    unsigned char ucsave=uc;

    GWEN_FASTBUFFER_PEEKBYTE(fb, chr);
    if (chr<0) {
      return chr;
    }
    uc=(unsigned char) chr;
    if (uc=='>') {
      DBG_VERBOUS(GWEN_LOGDOMAIN, "Ending tag [%s]", GWEN_Buffer_GetStart(dbuf));
      fb->bufferReadPos++;
      rv=GWEN_XmlCtx_EndTag(ctx, 1);
      if (rv) {
        return rv;
      }
      /* tag finished */
      return 0;
    }
    else {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Got an unexpected character here (after %02x[%c]): %02x[%c], "
                "maybe the text contains unescaped XML characters?",
                ucsave, ucsave, uc, uc);
    }
  }
  else if (uc=='>') {
    rv=GWEN_XmlCtx_EndTag(ctx, 0);
    if (rv) {
      return rv;
    }
    /* tag finished */
    return 0;
  }

  DBG_ERROR(GWEN_LOGDOMAIN,
            "Internal error: Should never reach this point");
  return GWEN_ERROR_INTERNAL;
}




int GWEN_XML_ReadFromFastBuffer(GWEN_XML_CONTEXT *ctx, GWEN_FAST_BUFFER *fb)
{
  int oks=0;
  int startingDepth;
  GWEN_BUFFER *workBuf;

  startingDepth=GWEN_XmlCtx_GetDepth(ctx);

  workBuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_XmlCtx_ResetFinishedElement(ctx);
  for (;;) {
    int rv;

    GWEN_FASTBUFFER_PEEKBYTE(fb, rv);
    if (rv<0) {
      if (rv!=GWEN_ERROR_EOF || !oks) {
        DBG_DEBUG(GWEN_LOGDOMAIN, "here (%d), after reading %d bytes",
                  rv, (int) GWEN_FastBuffer_GetBytesRead(fb));
        GWEN_Buffer_free(workBuf);
        return rv;
      }
      GWEN_Buffer_free(workBuf);
      return 0;
    }

    rv=GWEN_XML__ReadData(ctx, fb, GWEN_XmlCtx_GetFlags(ctx));
    if (rv) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(workBuf);
      return rv;
    }
    oks=1;

    GWEN_FASTBUFFER_PEEKBYTE(fb, rv);
    if (rv<0) {
      if (rv!=GWEN_ERROR_EOF || !oks ||
          (GWEN_XmlCtx_GetDepth(ctx)!=startingDepth)) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (rv=%d, oks=%d, depth=%d, startingDepth=%d)",
                 rv, oks, GWEN_XmlCtx_GetDepth(ctx), startingDepth);
        GWEN_Buffer_free(workBuf);
        return rv;
      }
      GWEN_Buffer_free(workBuf);
      return 0;
    }
    else if (rv=='<') {
      fb->bufferReadPos++;
      rv=GWEN_XML__ReadTag(ctx, fb, GWEN_XmlCtx_GetFlags(ctx), workBuf);
      if (rv) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        GWEN_Buffer_free(workBuf);
        return rv;
      }
      GWEN_Buffer_Reset(workBuf);
      oks=1;
    }

    if (GWEN_XmlCtx_GetFinishedElement(ctx) &&
        GWEN_XmlCtx_GetDepth(ctx)==startingDepth) {
      DBG_INFO(GWEN_LOGDOMAIN, "Finished element at depth %d", GWEN_XmlCtx_GetDepth(ctx));
      break;
    }
  }

  if (GWEN_XmlCtx_GetDepth(ctx)!=startingDepth) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Not on same level where we started...(%d!=%d)",
              GWEN_XmlCtx_GetDepth(ctx), startingDepth);
  }
  GWEN_Buffer_free(workBuf);

  return 0;
}



int GWEN_XML__ReadAllFromIo(GWEN_XML_CONTEXT *ctx, GWEN_SYNCIO *sio)
{
  GWEN_FAST_BUFFER *fb;
  int oks=0;

  fb=GWEN_FastBuffer_new(GWEN_XML_BUFFERSIZE, sio);
  assert(fb);
  for (;;) {
    int rv;

    rv=GWEN_XML_ReadFromFastBuffer(ctx, fb);
    if (rv<0) {
      if (rv==GWEN_ERROR_EOF && oks)
        break;
      else {
        DBG_INFO(GWEN_LOGDOMAIN, "here (rv=%d, oks=%d)", rv, oks);
        GWEN_FastBuffer_free(fb);
        return rv;
      }
    }
    oks=1;
  }

  GWEN_FastBuffer_free(fb);
  return 0;
}



int GWEN_XMLContext_ReadFromIo(GWEN_XML_CONTEXT *ctx, GWEN_SYNCIO *sio)
{
#if 0
  GWEN_FAST_BUFFER *fb;
  int rv;

  fb=GWEN_FastBuffer_new(GWEN_XML_BUFFERSIZE, sio);
  assert(fb);
  rv=GWEN_XML_ReadFromFastBuffer(ctx, fb);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    GWEN_FastBuffer_free(fb);
    return rv;
  }

  GWEN_FastBuffer_free(fb);
  return 0;
#else
  int rv;

  rv=GWEN_XML__ReadAllFromIo(ctx, sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return rv;
#endif
}



int GWEN_XMLContext_ReadFromFile(GWEN_XML_CONTEXT *ctx, const char *fname)
{
  GWEN_SYNCIO *sio;
  int rv;

  sio=GWEN_SyncIo_File_new(fname, GWEN_SyncIo_File_CreationMode_OpenExisting);
  GWEN_SyncIo_AddFlags(sio, GWEN_SYNCIO_FILE_FLAGS_READ);
  rv=GWEN_SyncIo_Connect(sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_free(sio);
    return rv;
  }

  rv=GWEN_XML__ReadAllFromIo(ctx, sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    return rv;
  }

  GWEN_SyncIo_Disconnect(sio);
  GWEN_SyncIo_free(sio);

  return 0;
}



int GWEN_XMLContext_ReadFromString(GWEN_XML_CONTEXT *ctx, const char *text)
{
  if (text && *text) {
    GWEN_SYNCIO *sio;
    int rv;
    GWEN_BUFFER *tbuf;
    int i;

    i=strlen(text)+1;
    tbuf=GWEN_Buffer_new((char *)text, i, i, 0);
    /* static buffer, don't resize */
    GWEN_Buffer_SubMode(tbuf, GWEN_BUFFER_MODE_DYNAMIC);
    GWEN_Buffer_AddMode(tbuf, GWEN_BUFFER_MODE_READONLY);
    sio=GWEN_SyncIo_Memory_new(tbuf, 0);

    rv=GWEN_XML__ReadAllFromIo(ctx, sio);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_SyncIo_free(sio);
      GWEN_Buffer_free(tbuf);
      return rv;
    }

    GWEN_SyncIo_free(sio);
    GWEN_Buffer_free(tbuf);
  }
  return 0;
}




int GWEN_XML_ReadFile(GWEN_XMLNODE *n, const char *filepath, uint32_t flags)
{
  GWEN_XML_CONTEXT *ctx;
  GWEN_SYNCIO *sio;
  int rv;

  sio=GWEN_SyncIo_File_new(filepath, GWEN_SyncIo_File_CreationMode_OpenExisting);
  GWEN_SyncIo_AddFlags(sio, GWEN_SYNCIO_FILE_FLAGS_READ);
  rv=GWEN_SyncIo_Connect(sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_free(sio);
    return rv;
  }

  ctx=GWEN_XmlCtxStore_new(n, flags);
  rv=GWEN_XML__ReadAllFromIo(ctx, sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    GWEN_XmlCtx_free(ctx);
    return rv;
  }

  GWEN_SyncIo_Disconnect(sio);
  GWEN_SyncIo_free(sio);

  GWEN_XmlCtx_free(ctx);

  return 0;
}



GWEN_XMLNODE *GWEN_XMLNode_fromString(const char *s, int len, uint32_t flags)
{
#if 0
  GWEN_XML_CONTEXT *ctx;
  GWEN_SYNCIO *sio;
  GWEN_XMLNODE *n;
  int rv;

  if (len==0)
    len=strlen(s);
  sio=GWEN_SyncIo_Memory_fromBuffer((const uint8_t *)s, len);

  n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "doc");
  ctx=GWEN_XmlCtxStore_new(n, flags);
  rv=GWEN_XML__ReadAllFromIo(ctx, sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_free(sio);
    GWEN_XmlCtx_free(ctx);
    GWEN_XMLNode_free(n);
    return NULL;
  }

  GWEN_SyncIo_free(sio);

  GWEN_XmlCtx_free(ctx);

  return n;
#else
  GWEN_XML_CONTEXT *ctx;
  GWEN_SYNCIO *sio;
  GWEN_XMLNODE *n;
  int rv;
  GWEN_BUFFER *tbuf;

  tbuf=GWEN_Buffer_new((char *)s, len, len, 0);
  /* static buffer, don't resize */
  GWEN_Buffer_SubMode(tbuf, GWEN_BUFFER_MODE_DYNAMIC);
  GWEN_Buffer_AddMode(tbuf, GWEN_BUFFER_MODE_READONLY);
  sio=GWEN_SyncIo_Memory_new(tbuf, 0);

  n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "doc");
  ctx=GWEN_XmlCtxStore_new(n, flags);
  rv=GWEN_XML__ReadAllFromIo(ctx, sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_XmlCtx_free(ctx);
    GWEN_XMLNode_free(n);
    GWEN_SyncIo_free(sio);
    GWEN_Buffer_free(tbuf);
    return NULL;
  }

  GWEN_XmlCtx_free(ctx);
  GWEN_SyncIo_free(sio);
  GWEN_Buffer_free(tbuf);

  return n;
#endif
}




