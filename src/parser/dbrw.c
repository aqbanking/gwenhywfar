/***************************************************************************
 begin       : Tue Sep 09 2003
 copyright   : (C) 2003-2010 by Martin Preuss
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


/* This file is included from db.c */



int GWEN_DB_EscapeToBufferTolerant(const char *src, GWEN_BUFFER *buf) {
  while(*src) {
    unsigned char x;

    x=(unsigned char)*src;
    if (!(
          (x>='A' && x<='Z') ||
          (x>='a' && x<='z') ||
          (x>='0' && x<='9') ||
          x=='%' ||
          x=='.' ||
          x==',' ||
          x=='.' ||
          x=='_' ||
          x=='-' ||
          x=='*' ||
          x=='?'
         )) {
      unsigned char c;

      GWEN_Buffer_AppendByte(buf, '&');
      c=(((unsigned char)(*src))>>4)&0xf;
      if (c>9)
	c+=7;
      c+='0';
      GWEN_Buffer_AppendByte(buf, c);
      c=((unsigned char)(*src))&0xf;
      if (c>9)
	c+=7;
      c+='0';
      GWEN_Buffer_AppendByte(buf, c);
    }
    else
      GWEN_Buffer_AppendByte(buf, *src);

    src++;
  } /* while */

  return 0;
}



int GWEN_DB_UnescapeToBufferTolerant(const char *src, GWEN_BUFFER *buf) {
  while(*src) {
    const char *srcBak;
    int charHandled;

    srcBak=src;
    charHandled=0;
    if (*src=='&') {
      if (strlen(src)>2) {
        unsigned char d1, d2;
        unsigned char c;

        if (isxdigit((int)src[1]) && isxdigit((int)src[2])) {
          /* skip '%' */
          src++;
          /* read first digit */
          d1=(unsigned char)(toupper(*src));

          /* get second digit */
          src++;
          d2=(unsigned char)(toupper(*src));
          /* compute character */
          d1-='0';
          if (d1>9)
            d1-=7;
          c=(d1<<4)&0xf0;
          d2-='0';
          if (d2>9)
            d2-=7;
          c+=(d2&0xf);
          /* store character */
          GWEN_Buffer_AppendByte(buf, (char)c);
          charHandled=1;
        }
      }
    }
    if (!charHandled)
      GWEN_Buffer_AppendByte(buf, *src);
    src++;
  } /* while */

  return 0;
}






int GWEN_DB_ReadFileAs(GWEN_DB_NODE *db,
                       const char *fname,
                       const char *type,
                       GWEN_DB_NODE *params,
		       uint32_t dbflags,
		       uint32_t guiid,
		       int msecs){
  GWEN_IO_LAYER *io;
  GWEN_DBIO *dbio;
  int fd;
  int rv;

  dbio=GWEN_DBIO_GetPlugin(type);
  if (!dbio) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Plugin \"%s\" is not supported", type);
    return GWEN_ERROR_NOT_SUPPORTED;
  }

  fd=open(fname, O_RDONLY);
  if (fd==-1) {
    DBG_ERROR(GWEN_LOGDOMAIN, "open(%s, O_RDONLY): %s", fname, strerror(errno));
    return GWEN_ERROR_IO;
  }

  /* create io layer for this file (readonly) */
  io=GWEN_Io_LayerFile_new(fd, -1);
  assert(io);

  rv=GWEN_Io_Manager_RegisterLayer(io);
  if (rv) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Internal error: Could not register io layer (%d)", rv);
    GWEN_Io_Layer_DisconnectRecursively(io, NULL, GWEN_IO_REQUEST_FLAGS_FORCE, guiid, msecs);
    GWEN_Io_Layer_free(io);
    return rv;
  }

  rv=GWEN_DBIO_Import(dbio, io, db, params, dbflags, guiid, msecs);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
  }
  GWEN_Io_Layer_DisconnectRecursively(io, NULL, GWEN_IO_REQUEST_FLAGS_FORCE, guiid, msecs);
  GWEN_Io_Layer_free(io);

  return rv;
}



int GWEN_DB_WriteFileAs(GWEN_DB_NODE *db,
                        const char *fname,
                        const char *type,
                        GWEN_DB_NODE *params,
			uint32_t dbflags,
			uint32_t guiid,
			int msecs){
  int rv;
  GWEN_DBIO *dbio;

  dbio=GWEN_DBIO_GetPlugin(type);
  if (!dbio) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Plugin \"%s\" is not supported", type);
    return GWEN_ERROR_NOT_SUPPORTED;
  }

  rv=GWEN_DBIO_ExportToFile(dbio, fname, db, params, dbflags, guiid, msecs);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int GWEN_DB_WriteGroupToIoLayer(GWEN_DB_NODE *node,
                                GWEN_FAST_BUFFER *fb,
				uint32_t dbflags,
				int insert) {
  GWEN_DB_NODE *n;
  GWEN_DB_NODE *cn;
  int i;
  int err;
  int lastWasVar;

  lastWasVar=0;

  n=GWEN_DB_Node_List_First(node->children);
  while(n) {
    if (!(n->nodeFlags & GWEN_DB_NODE_FLAGS_VOLATILE)) {
      DBG_VERBOUS(GWEN_LOGDOMAIN, "Writing node");
      switch(n->typ) {
      case GWEN_DB_NodeType_Group:
	if (dbflags & GWEN_DB_FLAGS_WRITE_SUBGROUPS) {
	  GWEN_BUFFER *tbuf;

          if (dbflags & GWEN_DB_FLAGS_ADD_GROUP_NEWLINES) {
            if (lastWasVar) {
              /* only insert newline if the last one before this group was a
	       * variable */
	      GWEN_FASTBUFFER_WRITELINE(fb, err, "");
	      if (err<0) {
		DBG_INFO(GWEN_LOGDOMAIN, "called from here");
		return err;
	      }
            }
	  }

          /* indend */
          if (dbflags & GWEN_DB_FLAGS_INDEND) {
	    for (i=0; i<insert; i++) {
	      GWEN_FASTBUFFER_WRITEBYTE(fb, err, ' ');
	      if (err<0) {
		DBG_INFO(GWEN_LOGDOMAIN, "called from here");
		return err;
	      }
            } /* for */
	  } /* if indend */

	  tbuf=GWEN_Buffer_new(0, 128, 0, 1);
	  err=GWEN_DB_EscapeToBufferTolerant(n->data.dataName, tbuf);
	  if (err<0) {
	    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", err);
            GWEN_Buffer_free(tbuf);
	    return err;
	  }

	  GWEN_FASTBUFFER_WRITEFORCED(fb, err,
				      GWEN_Buffer_GetStart(tbuf),
				      GWEN_Buffer_GetUsedBytes(tbuf));
	  GWEN_Buffer_free(tbuf);
	  if (err<0) {
	    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", err);
	    return err;
	  }
	  GWEN_FASTBUFFER_WRITELINE(fb, err, " {");
	  if (err<0) {
	    DBG_INFO(GWEN_LOGDOMAIN, "called from here");
	    return err;
	  }
	  err=GWEN_DB_WriteGroupToIoLayer(n, fb, dbflags, insert+2);
	  if (err<0)
	    return err;

          /* indend */
          if (dbflags & GWEN_DB_FLAGS_INDEND) {
            for (i=0; i<insert; i++) {
	      GWEN_FASTBUFFER_WRITEBYTE(fb, err, ' ');
	      if (err<0) {
		DBG_INFO(GWEN_LOGDOMAIN, "called from here");
		return err;
	      }
            } /* for */
          } /* if indend */

          if (dbflags & GWEN_DB_FLAGS_DETAILED_GROUPS) {
	    GWEN_FASTBUFFER_WRITEFORCED(fb, err, "} #", -1);
	    if (err<0) {
	      DBG_INFO(GWEN_LOGDOMAIN, "called from here");
	      return err;
	    }
	    GWEN_FASTBUFFER_WRITELINE(fb, err, n->data.dataName);
	    if (err<0) {
	      DBG_INFO(GWEN_LOGDOMAIN, "called from here");
	      return err;
	    }
          } /* if detailed groups */
          else {
	    GWEN_FASTBUFFER_WRITELINE(fb, err, "}");
	    if (err<0) {
	      DBG_INFO(GWEN_LOGDOMAIN, "called from here");
	      return err;
	    }
	  }
	  if (dbflags & GWEN_DB_FLAGS_ADD_GROUP_NEWLINES) {
	    if (GWEN_DB_Node_List_Next(n)) {
	      /* only insert newline if something
	       * is following on the same level */
	      GWEN_FASTBUFFER_WRITELINE(fb, err, "");
	      if (err<0) {
		DBG_INFO(GWEN_LOGDOMAIN, "called from here");
		return err;
	      }
	    }
	  }
	}
	lastWasVar=0;
	break;

      case GWEN_DB_NodeType_Var:
	cn=GWEN_DB_Node_List_First(n->children);
	if (cn) {
          char *typname;
          int namewritten;
          int values;

          typname=0;
          namewritten=0;
          values=0;
          while(cn) {
            char numbuffer[32];
	    char *binbuffer=NULL;
	    unsigned int bbsize;
	    const char *pvalue=NULL;
	    GWEN_BUFFER *vbuf=NULL;

	    switch(cn->typ) {
	    case GWEN_DB_NodeType_ValueChar:
	      typname="char ";
	      pvalue=cn->data.dataChar;
	      if (dbflags & GWEN_DB_FLAGS_ESCAPE_CHARVALUES) {
		vbuf=GWEN_Buffer_new(0, strlen(pvalue)+32, 0, 1);
		if (GWEN_Text_EscapeToBufferTolerant(pvalue, vbuf)) {
		  DBG_INFO(GWEN_LOGDOMAIN, "called from here");
		  GWEN_Buffer_free(vbuf);
		  return 1;
		}
		pvalue=GWEN_Buffer_GetStart(vbuf);
	      }
	      break;

	    case GWEN_DB_NodeType_ValueInt:
	      typname="int  ";
	      if (GWEN_Text_NumToString(cn->data.dataInt,
					numbuffer,
					sizeof(numbuffer)-1,
					0)<1) {
		DBG_ERROR(GWEN_LOGDOMAIN, "Error writing numeric value");
		return GWEN_ERROR_GENERIC;
	      }
	      pvalue=numbuffer;
	      break;

	    case GWEN_DB_NodeType_ValueBin:
	      bbsize=cn->dataSize*2+1;
	      binbuffer=(char*)GWEN_Memory_malloc(bbsize);
	      assert(binbuffer);
	      typname="bin  ";
	      if (!GWEN_Text_ToHex(cn->data.dataBin,
				   cn->dataSize,
				   binbuffer,
				   bbsize)) {
		DBG_ERROR(GWEN_LOGDOMAIN, "Error writing binary value");
		return GWEN_ERROR_GENERIC;
	      }
	      pvalue=binbuffer;
	      break;

	    case GWEN_DB_NodeType_ValuePtr:
	      DBG_DEBUG(GWEN_LOGDOMAIN, "Not writing ptr type");
	      break;

	    default:
	      DBG_DEBUG(GWEN_LOGDOMAIN, "Unhandled type [%d]", cn->typ);
	      break;
	    }

	    if (pvalue) {
	      if (!namewritten) {
		/* write name */
		/* indend */
		if (dbflags & GWEN_DB_FLAGS_INDEND) {
		  for (i=0; i<insert; i++) {
		    GWEN_FASTBUFFER_WRITEBYTE(fb, err, ' ');
		    if (err<0) {
		      DBG_INFO(GWEN_LOGDOMAIN, "called from here");
		      GWEN_Memory_dealloc(binbuffer);
		      GWEN_Buffer_free(vbuf);
		      return 1;
		    }
		  } /* for */
		} /* if indend */
		if (!(dbflags & GWEN_DB_FLAGS_OMIT_TYPES)) {
		  GWEN_FASTBUFFER_WRITEFORCED(fb, err, typname, -1);
		  if (err<0) {
		    DBG_INFO(GWEN_LOGDOMAIN, "called from here");
		    GWEN_Memory_dealloc(binbuffer);
		    GWEN_Buffer_free(vbuf);
		    return 1;
		  }
		}
		if (dbflags & GWEN_DB_FLAGS_QUOTE_VARNAMES) {
		  GWEN_FASTBUFFER_WRITEBYTE(fb, err, '\"');
		  if (err<0) {
		    DBG_INFO(GWEN_LOGDOMAIN, "called from here");
		    GWEN_Memory_dealloc(binbuffer);
		    GWEN_Buffer_free(vbuf);
		    return 1;
		  }
		}
		GWEN_FASTBUFFER_WRITEFORCED(fb, err, n->data.dataName, -1);
		if (err<0) {
		  DBG_INFO(GWEN_LOGDOMAIN, "called from here");
		  GWEN_Memory_dealloc(binbuffer);
		  GWEN_Buffer_free(vbuf);
		  return 1;
		}
		if (dbflags & GWEN_DB_FLAGS_QUOTE_VARNAMES) {
		  GWEN_FASTBUFFER_WRITEBYTE(fb, err, '\"');
		  if (err<0) {
		    DBG_INFO(GWEN_LOGDOMAIN, "called from here");
		    GWEN_Memory_dealloc(binbuffer);
		    GWEN_Buffer_free(vbuf);
		    return 1;
		  }
		}
		GWEN_FASTBUFFER_WRITEFORCED(fb, err, ((dbflags & GWEN_DB_FLAGS_USE_COLON)?": ":"="), -1);
		if (err<0) {
		  DBG_INFO(GWEN_LOGDOMAIN, "called from here");
		  GWEN_Memory_dealloc(binbuffer);
		  GWEN_Buffer_free(vbuf);
		  return 1;
		}
		namewritten=1;
	      } /* if !namewritten */

	      if (values) {
		GWEN_FASTBUFFER_WRITEFORCED(fb, err, ", ", -1);
		if (err<0) {
		  DBG_INFO(GWEN_LOGDOMAIN, "called from here");
		  GWEN_Memory_dealloc(binbuffer);
		  GWEN_Buffer_free(vbuf);
		  return 1;
		}
	      }
	      values++;
	      if (dbflags & GWEN_DB_FLAGS_QUOTE_VALUES) {
		GWEN_FASTBUFFER_WRITEBYTE(fb, err, '\"');
		if (err<0) {
		  DBG_INFO(GWEN_LOGDOMAIN, "called from here");
		  GWEN_Memory_dealloc(binbuffer);
		  GWEN_Buffer_free(vbuf);
		  return 1;
		}
	      }

	      GWEN_FASTBUFFER_WRITEFORCED(fb, err, pvalue, -1);
	      if (err<0) {
		DBG_INFO(GWEN_LOGDOMAIN, "called from here");
		GWEN_Memory_dealloc(binbuffer);
		GWEN_Buffer_free(vbuf);
		return 1;
	      }

	      if (dbflags & GWEN_DB_FLAGS_QUOTE_VALUES) {
		GWEN_FASTBUFFER_WRITEBYTE(fb, err, '\"');
		if (err<0) {
		  DBG_INFO(GWEN_LOGDOMAIN, "called from here");
		  GWEN_Memory_dealloc(binbuffer);
		  GWEN_Buffer_free(vbuf);
		  return 1;
		}
	      }
	    } /* if pvalue */

	    GWEN_Memory_dealloc(binbuffer);
            GWEN_Buffer_free(vbuf);
            cn=GWEN_DB_Node_List_Next(cn);
	  } /* while cn */

	  if (namewritten) {
	    GWEN_FASTBUFFER_WRITELINE(fb, err, "");
	    if (err<0) {
	      DBG_INFO(GWEN_LOGDOMAIN, "called from here");
	      return GWEN_ERROR_GENERIC;
	    }
	  }
	} /* if children */
	lastWasVar=1;
	break;

      default:
	DBG_WARN(GWEN_LOGDOMAIN, "[unhandled node type %d]", n->typ);
      } /* switch */
    } /* if not volatile */
    else {
      DBG_DEBUG(GWEN_LOGDOMAIN, "Node is volatile, not writing it");
    }
    n=GWEN_DB_Node_List_Next(n);
  } /* while */

  return 0;
}



int GWEN_DB_WriteToFastBuffer(GWEN_DB_NODE *node,
			      GWEN_FAST_BUFFER *fb,
			      uint32_t dbflags) {
  int rv;

  rv=GWEN_DB_WriteGroupToIoLayer(node, fb, dbflags, 0);
  if (rv<0) {
    return rv;
  }
  GWEN_FASTBUFFER_FLUSH(fb, rv);
  return rv;
}



int GWEN_DB_WriteToIo(GWEN_DB_NODE *node,
		      GWEN_IO_LAYER *io,
		      uint32_t dbflags,
		      uint32_t guiid,
		      int msecs) {
  int rv;
  GWEN_FAST_BUFFER *fb;

  fb=GWEN_FastBuffer_new(512, io, guiid, msecs);
  if (dbflags & GWEN_DB_FLAGS_DOSMODE)
    GWEN_FastBuffer_AddFlags(fb, GWEN_FAST_BUFFER_FLAGS_DOSMODE);
  rv=GWEN_DB_WriteGroupToIoLayer(node, fb, dbflags, 0);
  if (rv<0) {
    GWEN_FastBuffer_free(fb);
    return rv;
  }
  GWEN_FASTBUFFER_FLUSH(fb, rv);
  GWEN_FastBuffer_free(fb);
  return rv;
}



int GWEN_DB_WriteToFd(GWEN_DB_NODE *n, int fd, uint32_t dbflags, uint32_t guiid, int msecs){
  GWEN_IO_LAYER *io;
  int rv;

  /* create io layer for this file */
  io=GWEN_Io_LayerFile_new(-1, fd);
  assert(io);
  GWEN_Io_Layer_AddFlags(io, GWEN_IO_LAYER_FLAGS_DONTCLOSE);

  rv=GWEN_Io_Manager_RegisterLayer(io);
  if (rv) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Internal error: Could not register io layer (%d)", rv);
    GWEN_Io_Layer_DisconnectRecursively(io, NULL, GWEN_IO_REQUEST_FLAGS_FORCE, guiid, msecs);
    GWEN_Io_Layer_free(io);
    return rv;
  }

  /* write to file */
  rv=GWEN_DB_WriteToIo(n, io, dbflags, guiid, msecs);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Io_Layer_Disconnect(io, GWEN_IO_REQUEST_FLAGS_FORCE, guiid, 1000);
    GWEN_Io_Layer_free(io);
    return rv;
  }

  /* close io layer */
  rv=GWEN_Io_Layer_DisconnectRecursively(io, NULL, 0, guiid, 30000);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Io_Layer_Disconnect(io, GWEN_IO_REQUEST_FLAGS_FORCE, guiid, 1000);
    GWEN_Io_Layer_free(io);
    return rv;
  }

  GWEN_Io_Layer_free(io);

  return 0;
}



int GWEN_DB_WriteFile(GWEN_DB_NODE *n, const char *fname, uint32_t dbflags, uint32_t guiid, int msecs){
  int fd;
  int rv;
  GWEN_FSLOCK *lck=0;

  /* if locking requested */
  if (dbflags & GWEN_DB_FLAGS_LOCKFILE) {
    GWEN_FSLOCK_RESULT res;

    lck=GWEN_FSLock_new(fname, GWEN_FSLock_TypeFile);
    assert(lck);
    res=GWEN_FSLock_Lock(lck, GWEN_DB_DEFAULT_LOCK_TIMEOUT, 0);
    if (res!=GWEN_FSLock_ResultOk) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Could not apply lock to file \"%s\" (%d)",
                fname, res);
      GWEN_FSLock_free(lck);
      return -1;
    }
  }

  /* open file */
  if (dbflags & GWEN_DB_FLAGS_APPEND_FILE)
    fd=open(fname, O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
  else
    fd=open(fname, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  if (fd==-1) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Error opening file \"%s\": %s",
              fname,
              strerror(errno));
    if (lck) {
      GWEN_FSLock_Unlock(lck);
      GWEN_FSLock_free(lck);
    }
    return GWEN_ERROR_IO;
  }

  rv=GWEN_DB_WriteToFd(n, fd, dbflags, guiid, msecs);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    close(fd);
    if (lck) {
      GWEN_FSLock_Unlock(lck);
      GWEN_FSLock_free(lck);
    }
    return rv;
  }

  if (close(fd)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Error closing file \"%s\": %s",
	      fname,
	      strerror(errno));
    if (lck) {
      GWEN_FSLock_Unlock(lck);
      GWEN_FSLock_free(lck);
    }
    return GWEN_ERROR_IO;
  }

  /* remove lock, if any */
  if (lck) {
    GWEN_FSLOCK_RESULT res;

    res=GWEN_FSLock_Unlock(lck);
    if (res!=GWEN_FSLock_ResultOk) {
      DBG_WARN(GWEN_LOGDOMAIN,
               "Could not remove lock on file \"%s\" (%d)",
               fname, res);
    }
    GWEN_FSLock_free(lck);
  }

  return 0;
}



int GWEN_DB__ReadValues(GWEN_DB_NODE *n,
			uint32_t dbflags,
			const char *typeName,
			const char *varName,
			uint8_t *p) {
  GWEN_DB_NODE_TYPE nodeType=GWEN_DB_NodeType_ValueChar;
  GWEN_DB_NODE *dbVar;
  GWEN_BUFFER *wbuf;
  uint8_t *pDebug;

  pDebug=p;

  if (typeName==NULL)
    typeName="char";
  if (strcasecmp(typeName, "int")==0)
    nodeType=GWEN_DB_NodeType_ValueInt;
  else if (strcasecmp(typeName, "char")==0)
    nodeType=GWEN_DB_NodeType_ValueChar;
  else if (strcasecmp(typeName, "bin")==0)
    nodeType=GWEN_DB_NodeType_ValueBin;
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Unknown type \"%s\"", typeName);
    return GWEN_ERROR_BAD_DATA;
  }

  dbVar=GWEN_DB_GetNode(n, varName, dbflags | GWEN_PATH_FLAGS_VARIABLE);
  if (dbVar==NULL) {
    DBG_INFO(GWEN_LOGDOMAIN, "Variable [%s] is not available", varName);
    return GWEN_ERROR_GENERIC;
  }

  wbuf=GWEN_Buffer_new(0, 32, 0, 1);
  for (;;) {
    int quotes=0;
    GWEN_DB_NODE *dbVal=NULL;
    const char *v;

    while(*p && isspace(*p))
      p++;
    if (!*p) {
      DBG_INFO(GWEN_LOGDOMAIN, "Missing value");
      GWEN_Buffer_free(wbuf);
      return GWEN_ERROR_BAD_DATA;
    }

    if (*p=='"') {
      quotes=1;
      p++;
    }

    while(*p) {
      if (*p=='%') {
	uint8_t c;
	uint8_t cHex;

	/* get first nibble */
	p++;
	if (!*p) {
	  DBG_INFO(GWEN_LOGDOMAIN, "Incomplete escape sequence");
	  GWEN_Buffer_free(wbuf);
	  return GWEN_ERROR_BAD_DATA;
	}
	c=toupper(*p)-'0';
	if (c>9) c-=7;
	cHex=c<<4;

	p++;
	if (!*p) {
	  DBG_INFO(GWEN_LOGDOMAIN, "Incomplete escape sequence");
	  GWEN_Buffer_free(wbuf);
	  return GWEN_ERROR_BAD_DATA;
	}
	c=toupper(*p)-'0';
	if (c>9) c-=7;
	cHex|=c;
	GWEN_Buffer_AppendByte(wbuf, cHex);
      }
      else
	if (quotes) {
	  if (*p=='"') {
	    p++;
	    break;
	  }
	  else
	    GWEN_Buffer_AppendByte(wbuf, *p);
	}
	else {
	  if (*p==',' || *p==';' || *p=='#')
	    break;
	  else if (*p=='"') {
	    DBG_INFO(GWEN_LOGDOMAIN, "Unexpected quotation mark (Line: [%s], parsed: [%s]",
		     pDebug, GWEN_Buffer_GetStart(wbuf));
	    GWEN_Buffer_free(wbuf);
	    return GWEN_ERROR_BAD_DATA;
	  }
	  else
	    GWEN_Buffer_AppendByte(wbuf, *p);
	}
      p++;
    }

    v=GWEN_Buffer_GetStart(wbuf);
    if (nodeType==GWEN_DB_NodeType_ValueInt) {
      int i;

      if (1!=sscanf(v, "%d", &i)) {
	DBG_INFO(GWEN_LOGDOMAIN, "Not an integer value [%s]", v);
	GWEN_Buffer_free(wbuf);
	return GWEN_ERROR_BAD_DATA;
      }
      dbVal=GWEN_DB_ValueInt_new(i);
    }
    else if (nodeType==GWEN_DB_NodeType_ValueChar)
      dbVal=GWEN_DB_ValueChar_new(v);
    else if (nodeType==GWEN_DB_NodeType_ValueBin) {
      GWEN_BUFFER *bbuf;
      int rv;

      bbuf=GWEN_Buffer_new(0, (GWEN_Buffer_GetUsedBytes(wbuf)/2)+1, 0, 1);
      rv=GWEN_Text_FromHexBuffer(v, bbuf);
      if (rv) {
	DBG_INFO(GWEN_LOGDOMAIN, "Bad bin value [%s]", v);
	GWEN_Buffer_free(bbuf);
	GWEN_Buffer_free(wbuf);
	return GWEN_ERROR_BAD_DATA;
      }
      dbVal=GWEN_DB_ValueBin_new(GWEN_Buffer_GetStart(bbuf),
				 GWEN_Buffer_GetUsedBytes(bbuf));
      GWEN_Buffer_free(bbuf);
    }
    else {
      /* should never reach this point */
      assert(0);
    }
    GWEN_DB_Node_Append(dbVar, dbVal);

    /* skip blanks if any */
    while(*p && isspace(*p))
      p++;
    if (!*p || *p==';' || *p=='#')
      break;
    else if (*p!=',') {
      DBG_INFO(GWEN_LOGDOMAIN, "Unexpected character [%s]", p);
      GWEN_Buffer_free(wbuf);
      return GWEN_ERROR_BAD_DATA;
    }
    p++;
    GWEN_Buffer_Reset(wbuf);
  }

  GWEN_Buffer_free(wbuf);
  return 0;
}



int GWEN_DB_ReadFromFastBuffer(GWEN_DB_NODE *n,
			       GWEN_FAST_BUFFER *fb,
			       uint32_t dbflags) {
  GWEN_BUFFER *lbuf;
  GWEN_BUFFER *tbuf;
  int level=0;
  int someLinesRead=0;

  lbuf=GWEN_Buffer_new(0, 128, 0, 1);
  tbuf=GWEN_Buffer_new(0, 128, 0, 1);

  for (;;) {
    int rv;
    uint8_t *p;

    rv=GWEN_FastBuffer_ReadLineToBuffer(fb, lbuf);
    if (rv<0) {
      if (rv==GWEN_ERROR_EOF) {
	if (!someLinesRead && !(dbflags & GWEN_DB_FLAGS_ALLOW_EMPTY_STREAM)){
	  DBG_INFO(GWEN_LOGDOMAIN, "Unexpected EOF (%d)", rv);
	  GWEN_Buffer_free(tbuf);
	  GWEN_Buffer_free(lbuf);
	  return rv;
	}
	break;
      }
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(tbuf);
      GWEN_Buffer_free(lbuf);
      return rv;
    }

    if (GWEN_Buffer_GetUsedBytes(lbuf)==0) {
      if (dbflags & GWEN_DB_FLAGS_UNTIL_EMPTY_LINE) {
	break;
      }
    }
    else {
      someLinesRead=1;
      p=(uint8_t*)GWEN_Buffer_GetStart(lbuf);
      while(*p && isspace(*p))
	p++;
      if (*p) {
	uint8_t *p1begin=NULL, *p1end=NULL;
	uint8_t *p2begin=NULL, *p2end=NULL;
  
	/* non-empty line */
	if (*p=='}') {
	  /* found end of current group */
	  if (level<1) {
	    DBG_INFO(GWEN_LOGDOMAIN, "Unbalanced number of curly bracket");
	    GWEN_Buffer_free(tbuf);
	    GWEN_Buffer_free(lbuf);
	    return GWEN_ERROR_BAD_DATA;
	  }
	  n=n->parent;
	  assert(n); /* internal error if parent not found */
	  assert(n->typ==GWEN_DB_NodeType_Group); /* internal error if parent is not a group */
          level--;
	}
	else if (*p=='#') {
	  /* comment only line */
	}
	else {
	  p1begin=p;
	  /* read first token */
	  while(*p && !isspace(*p) &&
		*p!='{' &&
		*p!=((dbflags & GWEN_DB_FLAGS_USE_COLON)?':':'=') &&
		*p!='}' &&
		*p!=',' &&
		*p!=';')
	    p++;
	  if (!*p) {
	    DBG_INFO(GWEN_LOGDOMAIN, "Missing 2nd token");
            GWEN_Buffer_Dump(lbuf, stderr, 2);
	    GWEN_Buffer_free(tbuf);
	    GWEN_Buffer_free(lbuf);
	    return GWEN_ERROR_BAD_DATA;
	  }
	  p1end=p;

	  /* get to start of 2nd token */
	  while(*p && isspace(*p))
	    p++;
	  if (!*p) {
	    DBG_INFO(GWEN_LOGDOMAIN, "Missing 2nd token");
	    GWEN_Buffer_free(tbuf);
	    GWEN_Buffer_free(lbuf);
	    return GWEN_ERROR_BAD_DATA;
	  }
  
	  if (*p=='{') {
	    GWEN_DB_NODE *newGr;
  
	    /* found start of group */
	    *p1end=0;
	    rv=GWEN_DB_UnescapeToBufferTolerant((const char*)p1begin, tbuf);
	    if (rv<0) {
	      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	      GWEN_Buffer_free(tbuf);
	      GWEN_Buffer_free(lbuf);
	      return rv;
	    }
	    newGr=GWEN_DB_GetGroup(n, dbflags, GWEN_Buffer_GetStart(tbuf));
	    if (newGr==NULL) {
	      DBG_INFO(GWEN_LOGDOMAIN, "Could not create group [%s]", GWEN_Buffer_GetStart(tbuf));
	      GWEN_Buffer_free(tbuf);
	      GWEN_Buffer_free(lbuf);
	      return GWEN_ERROR_GENERIC;
	    }
	    GWEN_Buffer_Reset(tbuf);
	    n=newGr;
	    level++;
	  }
	  else if (*p=='=' || *p==':') {
	    /* found short variable definition */
	    *p1end=0;
	    p++;
	    rv=GWEN_DB__ReadValues(n, dbflags, NULL, (const char*)p1begin, p);
	    if (rv) {
	      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	      GWEN_Buffer_free(tbuf);
	      GWEN_Buffer_free(lbuf);
	      return rv;
	    }
	  }
	  else if (*p==',' || *p==';') {
	    DBG_INFO(GWEN_LOGDOMAIN, "Unexpected delimiter found");
	    GWEN_Buffer_free(tbuf);
	    GWEN_Buffer_free(lbuf);
	    return GWEN_ERROR_BAD_DATA;
	  }
	  else {
	    /* 2nd token, so this should be a standard variable definition */
	    p2begin=p;
	    while(*p &&
		  !isspace(*p) &&
		  *p!='{' &&
		  *p!=((dbflags & GWEN_DB_FLAGS_USE_COLON)?':':'=') &&
		  *p!='}' &&
		  *p!=',' &&
		  *p!=';')
	      p++;
	    if (!*p) {
	      DBG_INFO(GWEN_LOGDOMAIN, "Missing 2nd token [%s], [%s]", p1begin, p2begin);
	      GWEN_Buffer_free(tbuf);
	      GWEN_Buffer_free(lbuf);
	      return GWEN_ERROR_BAD_DATA;
	    }
	    p2end=p;
	    if (isspace(*p)) {
	      while(*p && isspace(*p))
		p++;
	      if (!*p) {
		DBG_INFO(GWEN_LOGDOMAIN, "Missing 2nd token");
		GWEN_Buffer_free(tbuf);
		GWEN_Buffer_free(lbuf);
		return GWEN_ERROR_BAD_DATA;
	      }
	    }
	    if (*p!='=' && *p!=':') {
	      DBG_INFO(GWEN_LOGDOMAIN, "Equation mark expected");
	      GWEN_Buffer_free(tbuf);
	      GWEN_Buffer_free(lbuf);
	      return GWEN_ERROR_BAD_DATA;
	    }
	    p++;
  
	    *p1end=0;
	    *p2end=0;
	    rv=GWEN_DB__ReadValues(n, dbflags, (const char*)p1begin, (const char*)p2begin, p);
	    if (rv) {
	      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	      GWEN_Buffer_free(tbuf);
	      GWEN_Buffer_free(lbuf);
	      return rv;
	    }
	  }
	}
      }
    }
    GWEN_Buffer_Reset(lbuf);
  }

  if (level) {
    DBG_INFO(GWEN_LOGDOMAIN, "Unbalanced number of curly bracket (too few)");
    GWEN_Buffer_free(tbuf);
    GWEN_Buffer_free(lbuf);
    return GWEN_ERROR_BAD_DATA;
  }

  GWEN_Buffer_free(tbuf);
  GWEN_Buffer_free(lbuf);

  return 0;
}



int GWEN_DB_ReadFromIo(GWEN_DB_NODE *n,
		       GWEN_IO_LAYER *io,
		       uint32_t dbflags,
		       uint32_t guiid,
		       int msecs) {
  GWEN_FAST_BUFFER *fb;
  int rv;

  /* prepare fast buffer */
  fb=GWEN_FastBuffer_new(1024, io, guiid, msecs);
  if (dbflags & GWEN_DB_FLAGS_DOSMODE)
    GWEN_FastBuffer_AddFlags(fb, GWEN_FAST_BUFFER_FLAGS_DOSMODE);

  /* read from it */
  rv=GWEN_DB_ReadFromFastBuffer(n, fb, dbflags);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_FastBuffer_free(fb);
    return rv;
  }

  GWEN_FastBuffer_free(fb);
  return 0;
}



int GWEN_DB_ReadFromFd(GWEN_DB_NODE *n,
                       int fd,
		       uint32_t dbflags,
		       uint32_t guiid,
		       int msecs) {
  GWEN_IO_LAYER *io;
  int rv;

  /* create io layer for this file (readonly) */
  io=GWEN_Io_LayerFile_new(fd, -1);
  assert(io);
  GWEN_Io_Layer_AddFlags(io, GWEN_IO_LAYER_FLAGS_DONTCLOSE);

  rv=GWEN_Io_Manager_RegisterLayer(io);
  if (rv) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Internal error: Could not register io layer (%d)", rv);
    GWEN_Io_Layer_free(io);
    return rv;
  }

  rv=GWEN_DB_ReadFromIo(n, io, dbflags, guiid, msecs);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
  }
  GWEN_Io_Layer_DisconnectRecursively(io, NULL, GWEN_IO_REQUEST_FLAGS_FORCE, guiid, msecs);
  GWEN_Io_Layer_free(io);

  return rv;
}



int GWEN_DB_ReadFromFastBuffer2(GWEN_DB_NODE *n,
				GWEN_FAST_BUFFER2 *fb,
				uint32_t dbflags) {
  GWEN_BUFFER *lbuf;
  GWEN_BUFFER *tbuf;
  int level=0;
  int someLinesRead=0;

  lbuf=GWEN_Buffer_new(0, 128, 0, 1);
  tbuf=GWEN_Buffer_new(0, 128, 0, 1);

  for (;;) {
    int rv;
    uint8_t *p;

    rv=GWEN_FastBuffer2_ReadLineToBuffer(fb, lbuf);
    if (rv<0) {
      if (rv==GWEN_ERROR_EOF) {
	if (!someLinesRead && !(dbflags & GWEN_DB_FLAGS_ALLOW_EMPTY_STREAM)){
	  DBG_INFO(GWEN_LOGDOMAIN, "Unexpected EOF (%d)", rv);
	  GWEN_Buffer_free(tbuf);
	  GWEN_Buffer_free(lbuf);
	  return rv;
	}
	break;
      }
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(tbuf);
      GWEN_Buffer_free(lbuf);
      return rv;
    }

    if (GWEN_Buffer_GetUsedBytes(lbuf)==0) {
      if (dbflags & GWEN_DB_FLAGS_UNTIL_EMPTY_LINE) {
	break;
      }
    }
    else {
      someLinesRead=1;
      p=(uint8_t*)GWEN_Buffer_GetStart(lbuf);
      while(*p && isspace(*p))
	p++;
      if (*p) {
	uint8_t *p1begin=NULL, *p1end=NULL;
	uint8_t *p2begin=NULL, *p2end=NULL;
  
	/* non-empty line */
	if (*p=='}') {
	  /* found end of current group */
	  if (level<1) {
	    DBG_INFO(GWEN_LOGDOMAIN, "Unbalanced number of curly bracket");
	    GWEN_Buffer_free(tbuf);
	    GWEN_Buffer_free(lbuf);
	    return GWEN_ERROR_BAD_DATA;
	  }
	  n=n->parent;
	  assert(n); /* internal error if parent not found */
	  assert(n->typ==GWEN_DB_NodeType_Group); /* internal error if parent is not a group */
          level--;
	}
	else if (*p=='#') {
	  /* comment only line */
	}
	else {
	  p1begin=p;
	  /* read first token */
	  while(*p && !isspace(*p) &&
		*p!='{' &&
		*p!=((dbflags & GWEN_DB_FLAGS_USE_COLON)?':':'=') &&
		*p!='}' &&
		*p!=',' &&
		*p!=';')
	    p++;
	  if (!*p) {
	    DBG_INFO(GWEN_LOGDOMAIN, "Missing 2nd token");
            GWEN_Buffer_Dump(lbuf, stderr, 2);
	    GWEN_Buffer_free(tbuf);
	    GWEN_Buffer_free(lbuf);
	    return GWEN_ERROR_BAD_DATA;
	  }
	  p1end=p;

	  /* get to start of 2nd token */
	  while(*p && isspace(*p))
	    p++;
	  if (!*p) {
	    DBG_INFO(GWEN_LOGDOMAIN, "Missing 2nd token");
	    GWEN_Buffer_free(tbuf);
	    GWEN_Buffer_free(lbuf);
	    return GWEN_ERROR_BAD_DATA;
	  }
  
	  if (*p=='{') {
	    GWEN_DB_NODE *newGr;
  
	    /* found start of group */
	    *p1end=0;
	    rv=GWEN_DB_UnescapeToBufferTolerant((const char*)p1begin, tbuf);
	    if (rv<0) {
	      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	      GWEN_Buffer_free(tbuf);
	      GWEN_Buffer_free(lbuf);
	      return rv;
	    }
	    newGr=GWEN_DB_GetGroup(n, dbflags, GWEN_Buffer_GetStart(tbuf));
	    if (newGr==NULL) {
	      DBG_INFO(GWEN_LOGDOMAIN, "Could not create group [%s]", GWEN_Buffer_GetStart(tbuf));
	      GWEN_Buffer_free(tbuf);
	      GWEN_Buffer_free(lbuf);
	      return GWEN_ERROR_GENERIC;
	    }
	    GWEN_Buffer_Reset(tbuf);
	    n=newGr;
	    level++;
	  }
	  else if (*p=='=' || *p==':') {
	    /* found short variable definition */
	    *p1end=0;
	    p++;
	    rv=GWEN_DB__ReadValues(n, dbflags, NULL, (const char*)p1begin, p);
	    if (rv) {
	      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	      GWEN_Buffer_free(tbuf);
	      GWEN_Buffer_free(lbuf);
	      return rv;
	    }
	  }
	  else if (*p==',' || *p==';') {
	    DBG_INFO(GWEN_LOGDOMAIN, "Unexpected delimiter found");
	    GWEN_Buffer_free(tbuf);
	    GWEN_Buffer_free(lbuf);
	    return GWEN_ERROR_BAD_DATA;
	  }
	  else {
	    /* 2nd token, so this should be a standard variable definition */
	    p2begin=p;
	    while(*p &&
		  !isspace(*p) &&
		  *p!='{' &&
		  *p!=((dbflags & GWEN_DB_FLAGS_USE_COLON)?':':'=') &&
		  *p!='}' &&
		  *p!=',' &&
		  *p!=';')
	      p++;
	    if (!*p) {
	      DBG_INFO(GWEN_LOGDOMAIN, "Missing 2nd token [%s], [%s]", p1begin, p2begin);
	      GWEN_Buffer_free(tbuf);
	      GWEN_Buffer_free(lbuf);
	      return GWEN_ERROR_BAD_DATA;
	    }
	    p2end=p;
	    if (isspace(*p)) {
	      while(*p && isspace(*p))
		p++;
	      if (!*p) {
		DBG_INFO(GWEN_LOGDOMAIN, "Missing 2nd token");
		GWEN_Buffer_free(tbuf);
		GWEN_Buffer_free(lbuf);
		return GWEN_ERROR_BAD_DATA;
	      }
	    }
	    if (*p!='=' && *p!=':') {
	      DBG_INFO(GWEN_LOGDOMAIN, "Equation mark expected");
	      GWEN_Buffer_free(tbuf);
	      GWEN_Buffer_free(lbuf);
	      return GWEN_ERROR_BAD_DATA;
	    }
	    p++;
  
	    *p1end=0;
	    *p2end=0;
	    rv=GWEN_DB__ReadValues(n, dbflags, (const char*)p1begin, (const char*)p2begin, p);
	    if (rv) {
	      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
	      GWEN_Buffer_free(tbuf);
	      GWEN_Buffer_free(lbuf);
	      return rv;
	    }
	  }
	}
      }
    }
    GWEN_Buffer_Reset(lbuf);
  }

  if (level) {
    DBG_INFO(GWEN_LOGDOMAIN, "Unbalanced number of curly bracket (too few)");
    GWEN_Buffer_free(tbuf);
    GWEN_Buffer_free(lbuf);
    return GWEN_ERROR_BAD_DATA;
  }

  GWEN_Buffer_free(tbuf);
  GWEN_Buffer_free(lbuf);

  return 0;
}



int GWEN_DB_ReadFile(GWEN_DB_NODE *n,
		     const char *fname,
		     uint32_t dbflags,
		     uint32_t guiid,
		     int msecs) {
  GWEN_SYNCIO *sio;
  GWEN_FAST_BUFFER2 *fb;
  int rv;

  sio=GWEN_SyncIo_File_new(fname, GWEN_SyncIo_File_CreationMode_OpenExisting);
  rv=GWEN_SyncIo_Connect(sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_free(sio);
    return rv;
  }

  /* prepare fast buffer */
  fb=GWEN_FastBuffer2_new(1024, sio);
  if (dbflags & GWEN_DB_FLAGS_DOSMODE)
    GWEN_FastBuffer2_AddFlags(fb, GWEN_FAST_BUFFER2_FLAGS_DOSMODE);

  /* read from it */
  rv=GWEN_DB_ReadFromFastBuffer2(n, fb, dbflags);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_FastBuffer2_free(fb);
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    return rv;
  }

  GWEN_FastBuffer2_free(fb);
  GWEN_SyncIo_Disconnect(sio);
  return 0;
}



int GWEN_DB_ReadFromString(GWEN_DB_NODE *n,
			   const char *str,
                           int len,
			   uint32_t dbflags,
			   uint32_t guiid,
			   int msecs) {
  GWEN_IO_LAYER *io;
  int rv;

  if (len==0)
    len=strlen(str);

  /* create io layer */
  io=GWEN_Io_LayerMemory_fromString((const uint8_t*)str, len);
  assert(io);

  rv=GWEN_Io_Manager_RegisterLayer(io);
  if (rv) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Internal error: Could not register io layer (%d)", rv);
    GWEN_Io_Layer_DisconnectRecursively(io, NULL, GWEN_IO_REQUEST_FLAGS_FORCE, guiid, msecs);
    GWEN_Io_Layer_free(io);
    return rv;
  }

  rv=GWEN_DB_ReadFromIo(n, io, dbflags, guiid, msecs);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
  }
  GWEN_Io_Layer_DisconnectRecursively(io, NULL, GWEN_IO_REQUEST_FLAGS_FORCE, guiid, msecs);
  GWEN_Io_Layer_free(io);

  return rv;
}



int GWEN_DB_WriteToBuffer(GWEN_DB_NODE *n,
			  GWEN_BUFFER *buf,
			  uint32_t dbflags,
			  uint32_t guiid,
			  int msecs) {
  GWEN_IO_LAYER *io;
  int rv;

  /* create io layer */
  io=GWEN_Io_LayerMemory_new(buf);
  assert(io);

  rv=GWEN_Io_Manager_RegisterLayer(io);
  if (rv) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Internal error: Could not register io layer (%d)", rv);
    GWEN_Io_Layer_DisconnectRecursively(io, NULL, GWEN_IO_REQUEST_FLAGS_FORCE, guiid, msecs);
    GWEN_Io_Layer_free(io);
    return rv;
  }

  rv=GWEN_DB_WriteToIo(n, io, dbflags, guiid, msecs);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Io_Layer_Disconnect(io, GWEN_IO_REQUEST_FLAGS_FORCE, 0, 1000);
    GWEN_Io_Layer_free(io);
    return rv;
  }

  /* flush data */
  rv=GWEN_Io_Layer_WriteString(io, "",
			       GWEN_IO_REQUEST_FLAGS_FLUSH,
			       guiid,
			       30000);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Io_Layer_Disconnect(io, GWEN_IO_REQUEST_FLAGS_FORCE, 0, 1000);
    GWEN_Io_Layer_free(io);
    return rv;
  }

  /* close io layer */
  rv=GWEN_Io_Layer_DisconnectRecursively(io, NULL, 0, guiid, 30000);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Io_Layer_Disconnect(io, GWEN_IO_REQUEST_FLAGS_FORCE, guiid, 1000);
    GWEN_Io_Layer_free(io);
    return rv;
  }

  GWEN_Io_Layer_free(io);

  return 0;
}












