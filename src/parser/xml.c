/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Sat Jun 28 2003
 copyright   : (C) 2003 by Martin Preuss
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define DISABLE_DEBUGLOG

#include "xml_p.h"
#include "gwenhywfar/debug.h"
#include "gwenhywfar/misc.h"
#include "gwenhywfar/text.h"
#include "gwenhywfar/path.h"
#include "i18n_l.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif
#include <errno.h>
#include <unistd.h>



GWEN_LIST2_FUNCTIONS(GWEN_XMLNODE, GWEN_XMLNode)



GWEN_XMLPROPERTY *GWEN_XMLProperty_new(const char *name, const char *value){
  GWEN_XMLPROPERTY *p;

  GWEN_NEW_OBJECT(GWEN_XMLPROPERTY, p);
  if (name)
    p->name=strdup(name);
  if (value)
    p->value=strdup(value);
  return p;
}



void GWEN_XMLProperty_free(GWEN_XMLPROPERTY *p){
  if (p) {
    free(p->name);
    free(p->value);
    free(p);
  }
}



GWEN_XMLPROPERTY *GWEN_XMLProperty_dup(const GWEN_XMLPROPERTY *p){
  return GWEN_XMLProperty_new(p->name, p->value);
}



void GWEN_XMLProperty_add(GWEN_XMLPROPERTY *p, GWEN_XMLPROPERTY **head){
  GWEN_LIST_ADD(GWEN_XMLPROPERTY, p, head);
}



void GWEN_XMLProperty_insert(GWEN_XMLPROPERTY *p, GWEN_XMLPROPERTY **head){
  GWEN_LIST_INSERT(GWEN_XMLPROPERTY, p, head);
}


void GWEN_XMLProperty_del(GWEN_XMLPROPERTY *p, GWEN_XMLPROPERTY **head){
  GWEN_LIST_DEL(GWEN_XMLPROPERTY, p, head);
}


void GWEN_XMLProperty_freeAll(GWEN_XMLPROPERTY *p) {
  while(p) {
    GWEN_XMLPROPERTY *next;

    next=p->next;
    GWEN_XMLProperty_free(p);
    p=next;
  } /* while */
}




GWEN_XMLNODE *GWEN_XMLNode_new(GWEN_XMLNODE_TYPE t, const char *data){
  GWEN_XMLNODE *n;

  n=(GWEN_XMLNODE *)malloc(sizeof(GWEN_XMLNODE));
  assert(n);
  memset(n,0,sizeof(GWEN_XMLNODE));
  n->type=t;
  if (data)
    n->data=strdup(data);
  return n;
}


void GWEN_XMLNode_free(GWEN_XMLNODE *n){
  if (n) {
    GWEN_XMLProperty_freeAll(n->properties);
    free(n->data);
    GWEN_XMLNode_freeAll(n->child);
    GWEN_XMLNode_freeAll(n->header);
    free(n);
  }
}


void GWEN_XMLNode_freeAll(GWEN_XMLNODE *n){
  while(n) {
    GWEN_XMLNODE *next;

    next=n->next;
    GWEN_XMLNode_free(n);
    n=next;
  } /* while */
}


GWEN_XMLNODE *GWEN_XMLNode_dup(const GWEN_XMLNODE *n){
  GWEN_XMLNODE *nn, *cn, *ncn;
  const GWEN_XMLPROPERTY *p;

  /* duplicate node itself */
  nn=GWEN_XMLNode_new(n->type, n->data);

  /* duplicate properties */
  p=n->properties;
  while(p) {
    GWEN_XMLPROPERTY *np;

    np=GWEN_XMLProperty_dup(p);
    GWEN_XMLProperty_add(np, &(nn->properties));
    p=p->next;
  } /* while */

  /* duplicate children */
  cn=n->child;
  while(cn) {
    ncn=GWEN_XMLNode_dup(cn);
    GWEN_XMLNode_add(ncn, &(nn->child));
    ncn->parent=nn;
    cn=cn->next;
  } /* while */

  return nn;
}


void GWEN_XMLNode_add(GWEN_XMLNODE *n, GWEN_XMLNODE **head){
  GWEN_LIST_ADD(GWEN_XMLNODE, n, head);
}


void GWEN_XMLNode_del(GWEN_XMLNODE *n, GWEN_XMLNODE **head){
  GWEN_LIST_DEL(GWEN_XMLNODE, n, head);
  n->parent=0;
}



GWEN_XMLNODE *GWEN_XMLNode_fromString(const char *s,
                                      int len,
                                      GWEN_TYPE_UINT32 flags){
  GWEN_BUFFEREDIO *bio;
  GWEN_XMLNODE *tNode;

  bio=GWEN_BufferedIO_Buffer_fromString(s, len);
  GWEN_BufferedIO_SetReadBuffer(bio, 0, (len && len<1024)?len:1024);

  tNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
  while(!GWEN_BufferedIO_CheckEOF(bio)) {
    if (GWEN_XML_Parse(tNode, bio, flags)) {
      GWEN_XMLNode_free(tNode);
      GWEN_BufferedIO_Abandon(bio);
      GWEN_BufferedIO_free(bio);
      return 0;
    }
  } /* while */
  GWEN_BufferedIO_Close(bio);
  GWEN_BufferedIO_free(bio);

  return tNode;
}




const char *GWEN_XMLNode_GetProperty(const GWEN_XMLNODE *n, const char *name,
                                     const char *defaultValue){
  GWEN_XMLPROPERTY *p;

  assert(n);
  assert(name);
  p=n->properties;
  while(p) {
    assert(p->name);
    if (strcasecmp(p->name, name)==0)
      break;
    p=p->next;
  } /* while */

  if (p) {
    if (p->value)
      return p->value;
  }
  return defaultValue;
}


void GWEN_XMLNode__SetProperty(GWEN_XMLNODE *n,
                               const char *name, const char *value,
                               int doInsert){
  GWEN_XMLPROPERTY *p;

  p=n->properties;
  while(p) {
    assert(p->name);
    if (strcasecmp(p->name, name)==0)
      break;
    p=p->next;
  } /* while */

  if (p) {
    free(p->value);
    if (value)
      p->value=strdup(value);
    else
      p->value=0;
  }
  else {
    p=GWEN_XMLProperty_new(name, value);
    if (doInsert)
      GWEN_XMLProperty_insert(p, &(n->properties));
    else
      GWEN_XMLProperty_add(p, &(n->properties));
  }
}



void GWEN_XMLNode_SetProperty(GWEN_XMLNODE *n,
                              const char *name, const char *value){
  return GWEN_XMLNode__SetProperty(n, name, value, 0);
}



void GWEN_XMLNode_IncUsage(GWEN_XMLNODE *n){
  assert(n);
  n->usage++;
}



void GWEN_XMLNode_DecUsage(GWEN_XMLNODE *n){
  assert(n);
  if (n->usage==0) {
    DBG_WARN(GWEN_LOGDOMAIN, "Node usage already is zero");
  }
  else
    n->usage--;
}



GWEN_TYPE_UINT32 GWEN_XMLNode_GetUsage(const GWEN_XMLNODE *n){
  assert(n);
  return n->usage;
}



const char *GWEN_XMLNode_GetData(const GWEN_XMLNODE *n){
  assert(n);
  return n->data;
}


void GWEN_XMLNode_SetData(GWEN_XMLNODE *n, const char *data){
  assert(n);
  free(n->data);
  if (data)
    n->data=strdup(data);
  else
    n->data=0;
}


GWEN_XMLNODE *GWEN_XMLNode_GetChild(const GWEN_XMLNODE *n){
  assert(n);
  return n->child;
}


GWEN_XMLNODE *GWEN_XMLNode_GetParent(const GWEN_XMLNODE *n){
  assert(n);
  return n->parent;
}


void GWEN_XMLNode_AddChild(GWEN_XMLNODE *n, GWEN_XMLNODE *child){
  assert(n);
  GWEN_XMLNode_add(child, &(n->child));
  child->parent=n;
}


int GWEN_XML__ReadWordBuf(GWEN_BUFFEREDIO *bio,
                          GWEN_TYPE_UINT32 flags,
                          int chr,
                          const char *delims,
                          GWEN_BUFFER *buf) {
  int inQuote;
  int lastQuoteChar = '\0';
  int lastWasSpace;
  char *pdst;
  GWEN_TYPE_UINT32 roomLeft;
  GWEN_TYPE_UINT32 bytesAdded;

#define GWEN_XML__APPENDCHAR(chr)                     \
  if (roomLeft<2) {                                   \
    if (bytesAdded) {                                 \
      GWEN_Buffer_IncrementPos(buf, bytesAdded);      \
      GWEN_Buffer_AdjustUsedBytes(buf);               \
    }                                                 \
    GWEN_Buffer_AllocRoom(buf, 2);                    \
    pdst=GWEN_Buffer_GetPosPointer(buf);              \
    roomLeft=GWEN_Buffer_GetMaxUnsegmentedWrite(buf); \
    bytesAdded=0;                                     \
   }                                                  \
   *(pdst++)=(unsigned char)chr;                      \
   *pdst=0;                                           \
   bytesAdded++;                                      \
  roomLeft--

  inQuote=0;
  lastWasSpace=0;

  pdst=GWEN_Buffer_GetPosPointer(buf);
  roomLeft=GWEN_Buffer_GetMaxUnsegmentedWrite(buf);
  bytesAdded=0;
  while(1) {
    /* get character, if needed */
    if (chr==0) {
      chr=GWEN_BufferedIO_ReadChar(bio);
      if (chr<0) {
        if (chr==GWEN_BUFFEREDIO_CHAR_EOF)
          break;
        DBG_ERROR(GWEN_LOGDOMAIN, "Error on ReadChar");
        if (bytesAdded) {
          GWEN_Buffer_IncrementPos(buf, bytesAdded);
          GWEN_Buffer_AdjustUsedBytes(buf);
        }
        return -1;
      }
    }

    if ((chr<32 || chr==127) && !(flags & GWEN_XML_FLAGS_KEEP_CNTRL)) {
      chr=' ';
    }

    if (inQuote) {
      if (chr==lastQuoteChar) {
	inQuote=0;
        break;
      }
      else {
        lastWasSpace=0;

        GWEN_XML__APPENDCHAR(chr);
      }
    }
    else {
      if (!(flags & GWEN_XML_FLAGS__DATA) &&
	  (chr=='"' || chr=='\'' || chr=='[')) {
        lastWasSpace=0;
        inQuote=1;
        if (chr=='[')
          lastQuoteChar=']';
        else
          lastQuoteChar=chr;
      }
      else if (strchr(delims, chr)) {
	break;
      }
      else if (chr=='<') {
	DBG_ERROR(GWEN_LOGDOMAIN, "No tags inside a tag definition allowed");
        if (bytesAdded) {
          GWEN_Buffer_IncrementPos(buf, bytesAdded);
          GWEN_Buffer_AdjustUsedBytes(buf);
        }
        return -1;
      }
      else {
        if (!inQuote) {
	  if (isspace(chr)) {
	    if (!lastWasSpace) {
	      if ((flags & GWEN_XML_FLAGS_NO_CONDENSE))
		lastWasSpace=0;
	      else
		lastWasSpace=1;
              GWEN_XML__APPENDCHAR(chr);
	    }
          }
          else {
	    lastWasSpace=0;
            GWEN_XML__APPENDCHAR(chr);
          }
        }
        else {
          lastWasSpace=0;
          GWEN_XML__APPENDCHAR(chr);
        }
      }
    }
    chr=0;
  } /* while */

  if (bytesAdded) {
    GWEN_Buffer_IncrementPos(buf, bytesAdded);
    GWEN_Buffer_AdjustUsedBytes(buf);
  }

  if (((chr==']' || chr=='"' || chr=='\'') &&
       !(flags & GWEN_XML_FLAGS__DATA)))
    return 0;
  if (lastWasSpace) {
    GWEN_TYPE_UINT32 s;

    s=GWEN_Buffer_GetUsedBytes(buf);
    GWEN_Buffer_Crop(buf, 0, s-1);
    GWEN_Buffer_SetPos(buf, s);
  }

  return chr;
#undef GWEN_XML__APPENDCHAR
}



int GWEN_XML_ReadFile(GWEN_XMLNODE *n, const char *filepath,
                      GWEN_TYPE_UINT32 flags){
  return GWEN_XML_ReadFileInt(n, 0, filepath, 0, flags);
}


int GWEN_XML_ReadFileSearch(GWEN_XMLNODE *n, const char *filepath,
                            GWEN_TYPE_UINT32 flags,
                            GWEN_STRINGLIST *searchPath){
  return GWEN_XML_ReadFileInt(n, 0, filepath, searchPath, flags);
}



void GWEN_XMLNode_AddChildrenOnly(GWEN_XMLNODE *n, GWEN_XMLNODE *nn,
                                  int copythem){
  GWEN_XMLNODE *ch;

  assert(n);
  assert(nn);

  ch=nn->child;
  while(ch) {
    GWEN_XMLNODE *nc;

    nc=ch->next;
    if (!copythem) {
      GWEN_XMLNode_UnlinkChild(nn, ch);
      GWEN_XMLNode_AddChild(n, ch);
    }
    else {
      GWEN_XMLNode_AddChild(n, GWEN_XMLNode_dup(ch));
    }
    ch=nc;
  } /* while */
}



int GWEN_XML_Parse(GWEN_XMLNODE *n,
                   GWEN_BUFFEREDIO *bio,
                   GWEN_TYPE_UINT32 flags) {
  return GWEN_XML_ReadBIO(n, bio, flags | GWEN_XML_FLAGS_IGNORE_INCLUDE,
                          0, 0, 0);
}



int GWEN_XML_ReadBIO(GWEN_XMLNODE *n,
                     GWEN_BUFFEREDIO *bio,
                     GWEN_TYPE_UINT32 flags,
                     const char *fpath,
                     GWEN_STRINGLIST *sl,
                     GWEN_XML_INCLUDE_FN fn){
  GWEN_XMLNODE *path[GWEN_XML_MAX_DEPTH];
  int currDepth;
  int chr;
  int isEndTag;
  int eofMet;
  int isComment;

  currDepth=0;
  chr=0;

  flags&=~GWEN_XML_FLAGS__INTERNAL;
  while (1) {
    /* read char (if none set) but skip blanks */
    if (chr==0) {
      chr=GWEN_BufferedIO_ReadChar(bio);
      if (chr<0) {
        if (chr==GWEN_BUFFEREDIO_CHAR_EOF) {
          break;
        }
        DBG_ERROR(GWEN_LOGDOMAIN, "Error on BufferedIO_ReadChar");
        return -1;
      }
    }
    eofMet=0;
    if (!(flags & GWEN_XML_FLAGS_KEEP_BLANKS)) {
      while(isspace(chr)) {
        chr=GWEN_BufferedIO_ReadChar(bio);
        if (chr<0) {
          if (chr==GWEN_BUFFEREDIO_CHAR_EOF) {
            eofMet=1;
            break;
          }
          DBG_ERROR(GWEN_LOGDOMAIN, "Error on BufferedIO_ReadChar");
          return -1;
        }
      }
    }
    if (eofMet)
      break;

    if (chr=='<') {
      GWEN_BUFFER *bufTagName;
      char *p;

      isEndTag=0;

      /* we have a tag */
      bufTagName=GWEN_Buffer_new(0, 32, 0, 1);
      chr=GWEN_XML__ReadWordBuf(bio,
				flags &
				~GWEN_XML_FLAGS_NO_CONDENSE &
				~GWEN_XML_FLAGS_KEEP_CNTRL,
				0, " ><", bufTagName);
      if (chr<0) {
	GWEN_Buffer_free(bufTagName);
	DBG_ERROR(GWEN_LOGDOMAIN, "Eror reading word");
	return -1;
      }
      GWEN_Buffer_AppendByte(bufTagName, 0);
      p=GWEN_Buffer_GetStart(bufTagName);
      if (*p=='/') {
	isEndTag=1;
	p++;
      }
      DBG_DEBUG(GWEN_LOGDOMAIN,
                "Found tag \"%s\"", GWEN_Buffer_GetStart(bufTagName));

      isComment=0;
      if (strlen(p)>=3)
	if (strncmp(p,"!--",3)==0)
	  isComment=1;
      if (isComment) {
        GWEN_BUFFER *bufComment;
        GWEN_XMLNODE *newNode;
        int comlen;

        comlen=0;
        bufComment=GWEN_Buffer_new(0, 128, 0, 1);
        DBG_DEBUG(GWEN_LOGDOMAIN, "Found comment");
	/* we have a remark tag, so read it over */
	while(1) {
	  if (chr==0) {
	    chr=GWEN_BufferedIO_ReadChar(bio);
            if (chr<0) {
              if (chr==GWEN_BUFFEREDIO_CHAR_EOF) {
                DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected EOF within comment");
              }
              GWEN_Buffer_free(bufComment);
              GWEN_Buffer_free(bufTagName);
	      DBG_ERROR(GWEN_LOGDOMAIN, "Eror reading char");
	      return -1;
	    }
          }

          GWEN_Buffer_AppendByte(bufComment, (unsigned char)chr);
          comlen++;

          if (comlen>=3) {
            char *bstart;

            bstart=GWEN_Buffer_GetStart(bufComment);
            if (strncmp(bstart+comlen-3,"-->", 3)==0){
              /* comment finished */
              GWEN_Buffer_Crop(bufComment, 0, comlen-3);
              GWEN_Buffer_SetPos(bufComment, comlen-3);
              GWEN_Buffer_AppendByte(bufComment, 0);
              comlen-=3;
              break;
            }
            else {
              if (!(flags & GWEN_XML_FLAGS_READ_COMMENTS)) {
                DBG_VERBOUS(GWEN_LOGDOMAIN, "Clipping comment to 2 bytes");
                memmove(bstart, bstart+comlen-2, 2);
                GWEN_Buffer_Crop(bufComment, 0, 2);
                GWEN_Buffer_SetPos(bufComment, 2);
                comlen=2;
              }
            }
          }
          chr=0;
	} /* while */

        /* create new node */
        DBG_VERBOUS(GWEN_LOGDOMAIN, "Comment finished");
        if (flags & GWEN_XML_FLAGS_READ_COMMENTS) {
          GWEN_BUFFER *tbuf;

          tbuf=GWEN_Buffer_new(0, GWEN_Buffer_GetUsedBytes(bufComment),
                               0, 1);
          if (GWEN_Text_UnescapeXmlToBuffer(GWEN_Buffer_GetStart(bufComment),
                                            tbuf)) {
            GWEN_Buffer_free(tbuf);
            DBG_INFO(GWEN_LOGDOMAIN, "here");
            return -1;
          }

          newNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeComment,
                                   GWEN_Buffer_GetStart(tbuf));
          GWEN_Buffer_free(tbuf);
          GWEN_XMLNode_add(newNode, &(n->child));
          newNode->parent=n;
          DBG_DEBUG(GWEN_LOGDOMAIN, "Added comment: \"%s\"",
                    GWEN_Buffer_GetStart(bufComment));
        }
        else {
          DBG_DEBUG(GWEN_LOGDOMAIN, "Skip comment");
        }
        GWEN_Buffer_free(bufComment);
      } /* if remark */
      else {
	/* handle tagname */
        if (isEndTag) {
          GWEN_BUFFER *tbuf;

	  /* handle endtag */
	  if (currDepth<1) {
            DBG_ERROR(GWEN_LOGDOMAIN,
                      "Line %d: More endtags than start tags !",
                      GWEN_BufferedIO_GetLines(bio));
            GWEN_Buffer_free(bufTagName);
	    return -1;
          }
          tbuf=GWEN_Buffer_new(0, 32, 0, 1);
          if (GWEN_Text_UnescapeXmlToBuffer(p,
                                            tbuf)) {
            GWEN_Buffer_free(tbuf);
            DBG_INFO(GWEN_LOGDOMAIN, "here");
            return -1;
          }

          if (strcasecmp(n->data, GWEN_Buffer_GetStart(tbuf))!=0) {
            DBG_ERROR(GWEN_LOGDOMAIN,
                      "Line %d: endtag \"%s\" does not match "
                      "last start tag (\"%s\")",
                      GWEN_BufferedIO_GetLines(bio),
                      GWEN_Buffer_GetStart(bufTagName), n->data);
            GWEN_Buffer_free(tbuf);
            GWEN_Buffer_free(bufTagName);
            return -1;
          }
          GWEN_Buffer_free(tbuf);

          /* check whether the tag is "descr" */
          if ((flags & GWEN_XML_FLAGS_IGNORE_DESCR) &&
              strcasecmp(n->data, "descr")==0) {
            GWEN_XMLNODE *nparent;

            nparent=n->parent;
            assert(nparent);
            /* unlink <DESCR> tag and free it */
            GWEN_XMLNode_UnlinkChild(nparent, n);
            GWEN_XMLNode_free(n);
            n=0;
          }
          /* check whether the tag is "include" */
          else if (!(flags & GWEN_XML_FLAGS_IGNORE_INCLUDE) &&
              strcasecmp(n->data, "include")==0 &&
              fn) {
            /* it is, we have to include something ;-) */
            GWEN_XMLNODE *inametag;
            const char *iname;

            iname=0;
            inametag=n->child;
            if (inametag) {
              /* found a child */
              if (inametag->type==GWEN_XMLNodeTypeData) {
                /* child is a data tag, so use it as a name */
                iname=inametag->data;
              }
              else {
                DBG_WARN(GWEN_LOGDOMAIN,
                         "Line %d: <INCLUDE> tag with unexpected children",
                         GWEN_BufferedIO_GetLines(bio));
              }
            }
            else {
              DBG_WARN(GWEN_LOGDOMAIN,
                       "Line %d: <INCLUDE> tag without children",
                       GWEN_BufferedIO_GetLines(bio));
            }

            if (iname) {
              int irv;
              GWEN_XMLNODE *newRoot;
              GWEN_XMLNODE *nparent;
              GWEN_XMLNODE *itag;

              newRoot=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "tmproot");
              DBG_INFO(GWEN_LOGDOMAIN, "Line %d: Including file \"%s\" / \"%s\"",
                       GWEN_BufferedIO_GetLines(bio),
                       fpath, iname);
              irv=fn(newRoot, fpath, iname, sl, flags);
              if (irv) {
                DBG_INFO(GWEN_LOGDOMAIN, "here");
                GWEN_XMLNode_free(newRoot);
                GWEN_Buffer_free(bufTagName);
                return irv;
              }

              /* we have read the file, now just move all children */
              nparent=n->parent;
              assert(nparent);
              /* unlink <INCLUDE> tag */
              GWEN_XMLNode_UnlinkChild(nparent, n);
              /* move all children of newRoot to nparent */
              itag=newRoot->child;
              while(itag) {
                GWEN_XMLNODE *nextc;

                nextc=itag->next;
                if (flags & GWEN_XML_FLAGS_INCLUDE_TO_TOPLEVEL) {
                  GWEN_XMLNODE *tl;

                  DBG_INFO(GWEN_LOGDOMAIN, "Line %d: Importing node to toplevel",
                           GWEN_BufferedIO_GetLines(bio));
                  tl=nparent;
                  while(tl->parent)
                    tl=tl->parent;

                  if ((flags & GWEN_XML_FLAGS_SHARE_TOPLEVEL) &&
                      (itag->type==GWEN_XMLNodeTypeTag)) {
                    GWEN_XMLNODE *oldNode;

                    /* share toplevel, so find old tag and copy newtag in */
                    oldNode=GWEN_XMLNode_FindFirstTag(tl,
                                                      itag->data, 0, 0);
                    if (oldNode) {
                      /* use old node, copy properties */
                      DBG_INFO(GWEN_LOGDOMAIN,
                               "Line %d: Using old toplevel node for \"%s\"",
                               GWEN_BufferedIO_GetLines(bio),
                               itag->data);
                      GWEN_XMLNode_CopyProperties(oldNode, itag, 0);
                      /* append children only (move them) */
                      GWEN_XMLNode_AddChildrenOnly(oldNode, itag, 0);
                      GWEN_XMLNode_UnlinkChild(newRoot, itag);
                      GWEN_XMLNode_free(itag);
                    }
                    else {
                      /* simply add to toplevel */
                      GWEN_XMLNode_UnlinkChild(newRoot, itag);
                      GWEN_XMLNode_AddChild(tl, itag);
                    }
                  }
                  else {
                    /* simply add to toplevel */
                    GWEN_XMLNode_UnlinkChild(newRoot, itag);
                    GWEN_XMLNode_AddChild(tl, itag);
                  }
                }
                else {
                  /* simply add to current position */
                  GWEN_XMLNode_UnlinkChild(newRoot, itag);
                  GWEN_XMLNode_AddChild(nparent, itag);
                }
                itag=nextc;
              } /* while */
              /* free <INCLUDE> tag and new root */
              GWEN_XMLNode_free(newRoot);
              GWEN_XMLNode_free(n);
              /* just to be sure the freed tag is no longer used */
              n=0;
            } /* if include filename given */
          } /* if include tag */

          /* surface */
          n=path[currDepth-1];
          currDepth--;
          if (currDepth==0) {
            DBG_DEBUG(GWEN_LOGDOMAIN, "One node done");
            GWEN_Buffer_free(bufTagName);
            return 0;
          }
        }
        else {
	  /* this is a start tag */
          GWEN_XMLNODE *newNode;
          GWEN_BUFFER *bufVarName;
          GWEN_BUFFER *bufValue;
          GWEN_BUFFER *tbuf;
          const char *varname;
          const char *value;
          int simpleTag;

          simpleTag=0;
	  if (*p) {
	    int j;

            if (*p=='!' || *p=='?')
              simpleTag=1;

            /* This XML parser is very simple, but I need it to be able to
             * read tags such as "<br>" without the need of a trailing
             * slash (like "<br/>" instead of "<br>").
             * So therefore I just hardcoded the few known simple HTML tags
             * here.
             */
	    if ((flags & GWEN_XML_FLAGS_HANDLE_OPEN_HTMLTAGS) &&
		(strcasecmp(p, "BR")==0 ||
		 strcasecmp(p, "HR")==0))
	      simpleTag=1;

	    j=strlen(p)-1;
	    if (p[j]=='/') {
	      if (chr!='>') {
                DBG_ERROR(GWEN_LOGDOMAIN,
                          "Line %d: \"/\" only allowed just before \">\"",
                          GWEN_BufferedIO_GetLines(bio));
                GWEN_Buffer_free(bufTagName);
		return -1;
	      }
	      p[j]=0;
	      isEndTag=1;
	    }
	  }

          bufVarName=GWEN_Buffer_new(0, 128, 0, 1);
          bufValue=GWEN_Buffer_new(0, 128, 0, 1);

          tbuf=GWEN_Buffer_new(0, 32, 0, 1);
          if (GWEN_Text_UnescapeXmlToBuffer(p, tbuf)) {
            GWEN_Buffer_free(tbuf);
            DBG_INFO(GWEN_LOGDOMAIN, "here");
            return -1;
          }

          newNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag,
                                   GWEN_Buffer_GetStart(tbuf));
          GWEN_Buffer_free(tbuf);
          while (chr!='>' && chr!='/') {
            GWEN_Buffer_Reset(bufVarName);
            GWEN_Buffer_Reset(bufValue);
            varname=0;
            value=0;

            /* skip blanks */
            /*chr=0; */
            DBG_DEBUG(GWEN_LOGDOMAIN, "Current char is: \"%c\"", chr);
            if (chr==0 || isspace(chr)) {
              while(1) {
                chr=GWEN_BufferedIO_ReadChar(bio);
                if (chr<0) {
                  if (chr==GWEN_BUFFEREDIO_CHAR_EOF)
                    break;
                  GWEN_XMLNode_free(newNode);
                  GWEN_Buffer_free(bufValue);
                  GWEN_Buffer_free(bufVarName);
                  GWEN_Buffer_free(bufTagName);
		  DBG_ERROR(GWEN_LOGDOMAIN, "Eror reading char");
                  return -1;
                }
		if (!isspace(chr) && !(chr<32 || chr==127))
                  break;
              } /* while */
            }
            if (chr==0) {
	      DBG_ERROR(GWEN_LOGDOMAIN, "unexpected EOF");
	      GWEN_XMLNode_free(newNode);
              GWEN_Buffer_free(bufValue);
              GWEN_Buffer_free(bufVarName);
              GWEN_Buffer_free(bufTagName);
	      return -1;
	    }

	    if (chr=='>' || chr=='/')
	      break;

	    /* read possibly following var */
	    chr=GWEN_XML__ReadWordBuf(bio,
				      flags &
				      ~GWEN_XML_FLAGS_NO_CONDENSE &
				      ~GWEN_XML_FLAGS_KEEP_CNTRL,
				      chr, " =/>", bufVarName);
            if (chr<0) {
	      GWEN_XMLNode_free(newNode);
              GWEN_Buffer_free(bufValue);
              GWEN_Buffer_free(bufVarName);
              GWEN_Buffer_free(bufTagName);
	      return -1;
	      DBG_ERROR(GWEN_LOGDOMAIN, "Eror reading word");
	    }
            GWEN_Buffer_AppendByte(bufVarName, 0);
            varname=GWEN_Buffer_GetStart(bufVarName);
            if (strcmp(varname, "?")==0) {
              if (*p=='?') {
                DBG_DEBUG(GWEN_LOGDOMAIN, "Found closing question mark");
                break;
              }
            }
            DBG_DEBUG(GWEN_LOGDOMAIN, "Found property \"%s\"", varname);

	    /* skip blanks */
	    if (isspace(chr)) {
	      while(1) {
		chr=GWEN_BufferedIO_ReadChar(bio);
		if (chr<0) {
                  if (chr==GWEN_BUFFEREDIO_CHAR_EOF)
                    break;
		  GWEN_XMLNode_free(newNode);
                  GWEN_Buffer_free(bufValue);
                  GWEN_Buffer_free(bufVarName);
                  GWEN_Buffer_free(bufTagName);
		  DBG_ERROR(GWEN_LOGDOMAIN, "Eror reading char");
		  return -1;
		}
		if (!isspace(chr) && !(chr<32 || chr==127))
		  break;
	      }
	    }

            value=0;
	    if (chr=='=') {
	      chr=0;
	      /* skip blanks */
	      while(1) {
		chr=GWEN_BufferedIO_ReadChar(bio);
		if (chr<0) {
                  if (chr==GWEN_BUFFEREDIO_CHAR_EOF)
                    break;
		  GWEN_XMLNode_free(newNode);
                  GWEN_Buffer_free(bufValue);
                  GWEN_Buffer_free(bufVarName);
                  GWEN_Buffer_free(bufTagName);
		  DBG_ERROR(GWEN_LOGDOMAIN, "Eror reading char");
		  return -1;
		}
		if (!isspace(chr))
		  break;
	      }
	      if (chr==0) {
                DBG_ERROR(GWEN_LOGDOMAIN,
                          "Line %d: Value expected for property \"%s\"",
                          GWEN_BufferedIO_GetLines(bio),
                          varname);
		GWEN_XMLNode_free(newNode);
                GWEN_Buffer_free(bufValue);
                GWEN_Buffer_free(bufVarName);
                GWEN_Buffer_free(bufTagName);
		return -1;
	      }

	      if (chr=='>' || chr=='/') {
                DBG_ERROR(GWEN_LOGDOMAIN,
                          "Line %d: Value expected for property \"%s\"",
                          GWEN_BufferedIO_GetLines(bio),
                          varname);
		GWEN_XMLNode_free(newNode);
                GWEN_Buffer_free(bufValue);
                GWEN_Buffer_free(bufVarName);
                GWEN_Buffer_free(bufTagName);
		return -1;
	      }

	      /* read value */
	      chr=GWEN_XML__ReadWordBuf(bio,
					flags &
					~GWEN_XML_FLAGS_NO_CONDENSE &
					~GWEN_XML_FLAGS_KEEP_CNTRL,
					chr, " />", bufValue);
	      if (chr<0) {
		GWEN_XMLNode_free(newNode);
                GWEN_Buffer_free(bufValue);
                GWEN_Buffer_free(bufVarName);
                GWEN_Buffer_free(bufTagName);
		DBG_ERROR(GWEN_LOGDOMAIN, "Eror reading word");
                return -1;
              }
              GWEN_Buffer_AppendByte(bufValue, 0);
              value=GWEN_Buffer_GetStart(bufValue);
              DBG_DEBUG(GWEN_LOGDOMAIN, "Found value \"%s\"", value);
            } /* if value follows */

            if (varname) {
	      /* add property */
              GWEN_XMLPROPERTY *newProp;
              GWEN_BUFFER *tbuf1;
              GWEN_BUFFER *tbuf2;

              tbuf1=GWEN_Buffer_new(0, 32, 0, 1);
              if (GWEN_Text_UnescapeXmlToBuffer(varname,
                                                tbuf1)) {
                GWEN_Buffer_free(tbuf1);
                DBG_INFO(GWEN_LOGDOMAIN, "here");
                return -1;
              }
              tbuf2=GWEN_Buffer_new(0, 32, 0, 1);
              if (value) {
                if (GWEN_Text_UnescapeXmlToBuffer(value,
                                                  tbuf2)) {
                  GWEN_Buffer_free(tbuf2);
                  GWEN_Buffer_free(tbuf1);
                  DBG_INFO(GWEN_LOGDOMAIN, "here");
                  return -1;
                }
              }

              newProp=GWEN_XMLProperty_new(GWEN_Buffer_GetStart(tbuf1),
                                           GWEN_Buffer_GetStart(tbuf2));
              GWEN_Buffer_free(tbuf2);
              GWEN_Buffer_free(tbuf1);
              GWEN_XMLProperty_add(newProp, &(newNode->properties));
            } /* if varname */
          } /* while vars follow */

          if (chr=='/') {
            isEndTag=1;

            chr=GWEN_BufferedIO_ReadChar(bio);
            if (chr<0) {
              if (chr==GWEN_BUFFEREDIO_CHAR_EOF) {
                DBG_ERROR(GWEN_LOGDOMAIN, "Line %d: \">\" expected",
                          GWEN_BufferedIO_GetLines(bio));
              }
              else {
                DBG_ERROR(GWEN_LOGDOMAIN, "Error on ReadChar");
              }
              GWEN_XMLNode_free(newNode);
              GWEN_Buffer_free(bufValue);
              GWEN_Buffer_free(bufVarName);
              GWEN_Buffer_free(bufTagName);
              return -1;
            }
          }

          if (chr!='>') {
	    GWEN_XMLNode_free(newNode);
            GWEN_Buffer_free(bufValue);
            GWEN_Buffer_free(bufVarName);
            GWEN_Buffer_free(bufTagName);
	    DBG_ERROR(GWEN_LOGDOMAIN, "\">\" expected");
	    return -1;
	  }

          /* ok, now the tag is complete, add it */
          /* check for headers */
          if ((flags & GWEN_XML_FLAGS_HANDLE_HEADERS) &&
              (newNode->data[0]=='?' || newNode->data[0]=='!')) {
            if (currDepth==0) {
              DBG_DEBUG(GWEN_LOGDOMAIN, "Adding header \"%s\"",
                        newNode->data);
              /* unlink header tag and add it as header */
              GWEN_XMLNode_AddHeader(n, newNode);
            }
          }
          else {
            /* ok, add the tag */

            if ((flags & GWEN_XML_FLAGS_SHARE_TOPLEVEL) &&
                currDepth==0) {
              GWEN_XMLNODE *oldNode;
  
              oldNode=GWEN_XMLNode_FindFirstTag(n, newNode->data, 0, 0);
              if (oldNode) {
                /* use old node, copy properties */
                DBG_INFO(GWEN_LOGDOMAIN, "Line %d: Using old toplevel node for \"%s\"",
                         GWEN_BufferedIO_GetLines(bio),
                         newNode->data);
                GWEN_XMLNode_CopyProperties(oldNode, newNode, 0);
                GWEN_XMLNode_free(newNode);
                newNode=oldNode;
              }
              else {
                /* otherwise add new tag */
                if (currDepth>=GWEN_XML_MAX_DEPTH) {
                  DBG_ERROR(GWEN_LOGDOMAIN, "Line %d: Maximum depth exceeded",
                            GWEN_BufferedIO_GetLines(bio));
                  GWEN_XMLNode_free(newNode);
                  GWEN_Buffer_free(bufValue);
                  GWEN_Buffer_free(bufVarName);
                  GWEN_Buffer_free(bufTagName);
                  return -1;
                }
                GWEN_XMLNode_add(newNode, &(n->child));
                newNode->parent=n;
                DBG_DEBUG(GWEN_LOGDOMAIN, "Added node \"%s\"", newNode->data);
              }
            }
            else {
              if (currDepth>=GWEN_XML_MAX_DEPTH) {
                DBG_ERROR(GWEN_LOGDOMAIN, "Line %d: Maximum depth exceeded",
                          GWEN_BufferedIO_GetLines(bio));
                GWEN_XMLNode_free(newNode);
                GWEN_Buffer_free(bufValue);
                GWEN_Buffer_free(bufVarName);
                GWEN_Buffer_free(bufTagName);
                return -1;
              }
              GWEN_XMLNode_add(newNode, &(n->child));
              newNode->parent=n;
              DBG_DEBUG(GWEN_LOGDOMAIN, "Added node \"%s\"", newNode->data);
            }
            if (!isEndTag && !simpleTag) {
              /* only dive if this tag is not immediately ended */
              path[currDepth++]=n;
              n=newNode;
            }
            else {
              /* immediate endTag, if depth is 0: done */
              if (currDepth==0) {
                DBG_DEBUG(GWEN_LOGDOMAIN, "One node done");
                GWEN_Buffer_free(bufValue);
                GWEN_Buffer_free(bufVarName);
                GWEN_Buffer_free(bufTagName);
                return 0;
              }
            }
          }
          GWEN_Buffer_free(bufValue);
          GWEN_Buffer_free(bufVarName);
        } /* if start tag */
      } /* if no remark */

      chr=0; /* make begin of loop read new char */
      GWEN_Buffer_free(bufTagName);
    } /* if "<" */
    else {
      /* add data to current tag */
      GWEN_XMLNODE *newNode;
      GWEN_BUFFER *bufData;
      GWEN_BUFFER *tbuf;

      bufData=GWEN_Buffer_new(0, 256, 0, 1);
      chr=GWEN_XML__ReadWordBuf(bio, flags | GWEN_XML_FLAGS__DATA,
				chr, "<", bufData);
      if (chr<0) {
	GWEN_Buffer_free(bufData);
	DBG_INFO(GWEN_LOGDOMAIN, "Eror reading word");
	return -1;
      }

      tbuf=GWEN_Buffer_new(0, GWEN_Buffer_GetUsedBytes(bufData), 0, 1);
      if (GWEN_Text_UnescapeXmlToBuffer(GWEN_Buffer_GetStart(bufData),
                                        tbuf)) {
        GWEN_Buffer_free(tbuf);
        DBG_INFO(GWEN_LOGDOMAIN, "here");
        return -1;
      }

      newNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeData,
                               GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_free(tbuf);
      DBG_DEBUG(GWEN_LOGDOMAIN, "Added data \"%s\"",
                GWEN_Buffer_GetStart(bufData));
      GWEN_XMLNode_add(newNode, &(n->child));
      newNode->parent=n;

      GWEN_Buffer_free(bufData);
    }
    /* do not set chr=0, because we may already have the next char ('<') */
  } /* while !eof */

  if (currDepth!=0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Line %d: %d tags are still open",
              GWEN_BufferedIO_GetLines(bio),
              currDepth);
    for (chr = currDepth; chr > 0; chr--) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "  still open: <%s>",
		n->data);
      n=path[chr-1];
    }
    return -1;
  }

  return 0;
}



int GWEN_XML_ReadFileInt(GWEN_XMLNODE *n,
                         const char *path,
                         const char *file,
                         GWEN_STRINGLIST *sl,
                         GWEN_TYPE_UINT32 flags){
  GWEN_BUFFEREDIO *dm;
  int fd;
  unsigned int i;
  char *p;
  char *fullname;

  assert(n);
  assert(file);

  fullname=0;

  if (*file!='/') {
    /* create full name */
    i=0;
    if (path)
      i+=strlen(path)+1;
    i+=strlen(file)+1;
    fullname=(char*)malloc(i);
    assert(fullname);
    fullname[0]=0;
    if (path) {
      if (*path) {
        strcat(fullname, path);
        if (path[strlen(path)-1]!='/')
          strcat(fullname, "/");
      }
    }
    strcat(fullname, file);

    /* open file */
    fd=open(fullname, O_RDONLY);
    if (fd==-1) {
      if (!sl) {
	DBG_INFO(GWEN_LOGDOMAIN, "open(%s): %s",
		 fullname,
		 strerror(errno));
	free(fullname);
        return -1;
      }
      else {
        GWEN_STRINGLISTENTRY *se;
        int gotcha;

        DBG_INFO(GWEN_LOGDOMAIN, "open(%s): %s",
                 fullname,
                 strerror(errno));
        free(fullname);
        fullname=0;
        gotcha=0;

        /* try the search path */
        se=GWEN_StringList_FirstEntry(sl);
        while(se) {
          const char *sp;

          sp=GWEN_StringListEntry_Data(se);
          if (!sp) {
            DBG_ERROR(GWEN_LOGDOMAIN, "No data in string list entry, internal error");
            return -1;
          }
          if (!*sp) {
            DBG_ERROR(GWEN_LOGDOMAIN,
                      "Empty string in string list entry, internal error");
            return -1;
          }

          i=0;
          i+=strlen(sp)+1;
          i+=strlen(file)+1;
          fullname=(char*)malloc(i);
          assert(fullname);
          fullname[0]=0;
          strcpy(fullname, sp);
          if (fullname[strlen(fullname)-1]!='/')
            strcat(fullname, "/");
          strcat(fullname, file);
          /* try to open this file */
          fd=open(fullname, O_RDONLY);
          if (fd!=-1) {
            close(fd);
            gotcha=1;
            break;
          }
          else {
            DBG_INFO(GWEN_LOGDOMAIN, "open(%s): %s",
                     fullname,
                     strerror(errno));
          }

          free(fullname);
          fullname=0;
          se=GWEN_StringListEntry_Next(se);
        } /* while */
        if (!gotcha) {
	  DBG_INFO(GWEN_LOGDOMAIN, "Could not open file \"%s\".", file);
	  return -1;
        }
        /* now fullname points to the full path */
      }
    } /* if file not found */
    else
      close(fd);
  }
  else {
    /* create full name from file name only, since it is absolute */
    i=0;
    i+=strlen(file)+1;
    fullname=(char*)malloc(i);
    assert(fullname);
    fullname[0]=0;
    strcat(fullname, file);
  }

  /* open file */
  fd=open(fullname, O_RDONLY);
  if (fd==-1) {
    DBG_INFO(GWEN_LOGDOMAIN, "open(%s): %s",
	     fullname,
	     strerror(errno));
    free(fullname);
    return -1;
  }

  /* divide file name into path and file name */
  path=fullname;
  p=strrchr(fullname, '/');
  if (p)
    *p=0;
  else
    fullname[0]=0;

  dm=GWEN_BufferedIO_File_new(fd);
  GWEN_BufferedIO_SetReadBuffer(dm,0,1024);

  while(!GWEN_BufferedIO_CheckEOF(dm)) {
    if (GWEN_XML_ReadBIO(n, dm, flags, path, sl, GWEN_XML_ReadFileInt)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error parsing");
      free(fullname);
      GWEN_BufferedIO_Close(dm);
      GWEN_BufferedIO_free(dm);
      return -1;
    }
  } /* while */

  free(fullname);
  GWEN_BufferedIO_Close(dm);
  GWEN_BufferedIO_free(dm);

  return 0;
}



GWEN_XMLNODE_TYPE GWEN_XMLNode_GetType(const GWEN_XMLNODE *n){
  assert(n);
  return n->type;
}


GWEN_XMLNODE *GWEN_XMLNode_Next(const GWEN_XMLNODE *n) {
  assert(n);
  return n->next;
}


void GWEN_XMLNode_Dump(const GWEN_XMLNODE *n, FILE *f, int ind) {
  GWEN_XMLPROPERTY *p;
  GWEN_XMLNODE *c;
  int i;
  int simpleTag;

  assert(n);

  for(i=0; i<ind; i++)
    fprintf(f, " ");

  simpleTag=0;
  if (n->type==GWEN_XMLNodeTypeTag) {
    if (n->data)
      fprintf(f, "<%s", n->data);
    else
      fprintf(f, "<UNKNOWN");
    p=n->properties;
    while (p) {
      if (p->value)
        fprintf(f, " %s=\"%s\"", p->name, p->value);
      else
        fprintf(f, " %s", p->name);
      p=p->next;
    }

    if (n->data) {
      if (n->data[0]=='?') {
        simpleTag=1;
        fprintf(f, "?");
      }
      else if (n->data[0]=='!') {
        simpleTag=1;
      }
    }

    fprintf(f, ">\n");
    if (!simpleTag) {
      c=n->child;
      while(c) {
        GWEN_XMLNode_Dump(c, f, ind+2);
        c=c->next;
      }
      for(i=0; i<ind; i++)
        fprintf(f, " ");
      if (n->data)
        fprintf(f, "</%s>\n", n->data);
      else
        fprintf(f, "</UNKNOWN>\n");
    }
  }
  else if (n->type==GWEN_XMLNodeTypeData) {
    if (n->data) {
      fprintf(f, "%s\n", n->data);
    }
  }
  else if (n->type==GWEN_XMLNodeTypeComment) {
    fprintf(f, "<!--");
    if (n->data) {
      fprintf(f, "%s", n->data);
    }
    fprintf(f, "-->\n");
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Unknown tag type (%d)", n->type);
  }
}


GWEN_XMLNODE *GWEN_XMLNode_FindNode(const GWEN_XMLNODE *node,
                                    GWEN_XMLNODE_TYPE t, const char *data) {
  GWEN_XMLNODE *n;

  assert(node);
  assert(data);

  n=node->child;
  while(n) {
    if (n->type==t)
      if (n->data)
	if (strcasecmp(n->data, data)==0)
	  break;
    n=n->next;
  } /* while */

  if (!n) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "Node %d:\"%s\" not found", t, data);
    return 0;
  }

  return n;
}



void GWEN_XMLNode_UnlinkChild(GWEN_XMLNODE *n, GWEN_XMLNODE *child){
  assert(n);
  assert(child);
  GWEN_XMLNode_del(child, &(n->child));
  child->next=0;
  child->parent=0;
}



void GWEN_XMLNode_RemoveChildren(GWEN_XMLNODE *n){
  GWEN_XMLNODE *cn;

  assert(n);
  cn=n->child;
  while(cn) {
    GWEN_XMLNODE *ncn;

    ncn=cn->next;
    GWEN_XMLNode_free(cn);
    cn=ncn;
  } /* while */
  n->child=0;
}



void GWEN_XMLNode_CopyProperties(GWEN_XMLNODE *tn,
                                 const GWEN_XMLNODE *sn,
                                 int overwrite){
  const GWEN_XMLPROPERTY *sp;
  GWEN_XMLPROPERTY *tp;

  assert(tn);
  assert(sn);

  sp=sn->properties;
  while(sp) {
    GWEN_XMLPROPERTY *np;

    assert(sp->name);
    tp=tn->properties;
    /* lookup property in target */
    while(tp) {

      assert(tp->name);
      if (strcasecmp(tp->name, sp->name)==0) {
	/* property already exists */
	if (overwrite) {
          /* overwrite old property */
	  free(tp->value);
	  tp->value=0;
	  if (sp->value)
	    tp->value=strdup(sp->value);
	}
	break;
      }
      tp=tp->next;
    } /* while */

    if (!tp) {
      /* property not found, simply copy and add it */
      np=GWEN_XMLProperty_dup(sp);
      GWEN_XMLProperty_add(np, &(tn->properties));
    }

    sp=sp->next;
  } /* while */
}



GWEN_XMLNODE *GWEN_XMLNode_GetFirstOfType(const GWEN_XMLNODE *n,
                                          GWEN_XMLNODE_TYPE t){
  GWEN_XMLNODE *nn;

  assert(n);
  nn=n->child;
  while(nn) {
    if (nn->type==t)
      return nn;
    nn=nn->next;
  } /* while */
  return 0;
}



GWEN_XMLNODE *GWEN_XMLNode_GetNextOfType(const GWEN_XMLNODE *n,
                                         GWEN_XMLNODE_TYPE t){
  assert(n);
  while(n) {
    if (n->type==t)
      return (GWEN_XMLNODE *)n;
    n=n->next;
  } /* while */
  return 0;
}



GWEN_XMLNODE *GWEN_XMLNode_GetFirstTag(const GWEN_XMLNODE *n){
  return GWEN_XMLNode_GetFirstOfType(n, GWEN_XMLNodeTypeTag);
}



GWEN_XMLNODE *GWEN_XMLNode_GetNextTag(const GWEN_XMLNODE *n){
  if (!n->next)
    return 0;
  return GWEN_XMLNode_GetNextOfType(n->next, GWEN_XMLNodeTypeTag);
}



GWEN_XMLNODE *GWEN_XMLNode_GetFirstData(const GWEN_XMLNODE *n){
  return GWEN_XMLNode_GetFirstOfType(n, GWEN_XMLNodeTypeData);
}



GWEN_XMLNODE *GWEN_XMLNode_GetNextData(const GWEN_XMLNODE *n){
  if (!n->next)
    return 0;
  return GWEN_XMLNode_GetNextOfType(n->next, GWEN_XMLNodeTypeData);
}



GWEN_XMLNODE *GWEN_XMLNode_FindTag(const GWEN_XMLNODE *n,
                                   const char *tname,
                                   const char *pname,
                                   const char *pvalue){
  while(n) {
    if (-1!=GWEN_Text_ComparePattern(n->data, tname, 0)) {
      if (pname) {
        const char *p;

        p=GWEN_XMLNode_GetProperty(n, pname, 0);
        if (p) {
          if (!pvalue)
            return (GWEN_XMLNODE*)n;
          if (-1!=GWEN_Text_ComparePattern(pvalue, p, 0))
            return (GWEN_XMLNODE*)n;
        }
        else {
          /* return this node if pvalue is 0 an the property does not exist */
          if (!pvalue)
            return (GWEN_XMLNODE*)n;
        }
      } /* if pname */
      else
        return (GWEN_XMLNODE*)n;
    }
    n=GWEN_XMLNode_GetNextTag(n);
  } /* while */
  return 0;
}



GWEN_XMLNODE *GWEN_XMLNode_FindFirstTag(const GWEN_XMLNODE *n,
                                        const char *tname,
                                        const char *pname,
                                        const char *pvalue){
  GWEN_XMLNODE *nn;

  nn=GWEN_XMLNode_GetFirstTag(n);
  if (!nn)
    return 0;
  return GWEN_XMLNode_FindTag(nn,
                              tname,
                              pname,
                              pvalue);
}



GWEN_XMLNODE *GWEN_XMLNode_FindNextTag(const GWEN_XMLNODE *n,
                                       const char *tname,
                                       const char *pname,
                                       const char *pvalue){
  GWEN_XMLNODE *nn;

  nn=GWEN_XMLNode_GetNextTag(n);
  if (!nn)
    return 0;
  return GWEN_XMLNode_FindTag(nn,
                              tname,
                              pname,
                              pvalue);
}

















GWEN_XMLNODE_PATH *GWEN_XMLNode_Path_new(){
  GWEN_XMLNODE_PATH *p;

  GWEN_NEW_OBJECT(GWEN_XMLNODE_PATH, p);
  return p;
}



GWEN_XMLNODE_PATH *GWEN_XMLNode_Path_dup(const GWEN_XMLNODE_PATH *np){
  GWEN_XMLNODE_PATH *p;
  unsigned int i;

  GWEN_NEW_OBJECT(GWEN_XMLNODE_PATH, p);
  p->pos=np->pos;
  for (i=0; i<np->pos; i++) {
    p->nodes[i]=np->nodes[i];
  }
  return p;
}



void GWEN_XMLNode_Path_free(GWEN_XMLNODE_PATH *np){
  free(np);
}



int GWEN_XMLNode_Path_Dive(GWEN_XMLNODE_PATH *np,
                           GWEN_XMLNODE *n){
  unsigned int i;

  if (np->pos>=GWEN_XML_MAX_DEPTH) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Path too deep");
    return 1;
  }

  /* check for double entries */
  for (i=0; i<np->pos; i++) {
    assert(np->nodes[i]!=n);
  }
  np->nodes[np->pos++]=n;
  DBG_DEBUG(GWEN_LOGDOMAIN, "Dived to %d", np->pos);
  return 0;
}



GWEN_XMLNODE *GWEN_XMLNode_Path_Surface(GWEN_XMLNODE_PATH *np){
  if (np->pos==0) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "Root reached");
    return 0;
  }
  DBG_DEBUG(GWEN_LOGDOMAIN, "Surfaced to %d", np->pos-1);
  return np->nodes[--np->pos];
}



void GWEN_XMLNode_Path_Dump(GWEN_XMLNODE_PATH *np){
  unsigned int i;

  if (np->pos==0) {
    DBG_NOTICE(GWEN_LOGDOMAIN, "Empty path");
  }
  for (i=0; i<np->pos; i++) {
    DBG_NOTICE(GWEN_LOGDOMAIN, "Path entry %d:", i);
    GWEN_XMLNode_Dump(np->nodes[i], stderr, 1);
  }
}




int GWEN_XMLNode__WriteToStream(const GWEN_XMLNODE *n,
                                GWEN_BUFFEREDIO *bio,
                                GWEN_TYPE_UINT32 flags,
                                unsigned int ind) {
  GWEN_XMLPROPERTY *p;
  GWEN_XMLNODE *c;
  int i;
  int simpleTag;
  GWEN_ERRORCODE err;

  assert(n);

  if ((flags & GWEN_XML_FLAGS_INDENT) &&
      GWEN_BufferedIO_GetLinePos(bio)==0){
    for(i=0; i<ind; i++) {
      err=GWEN_BufferedIO_WriteChar(bio, ' ');
      if (!GWEN_Error_IsOk(err)) {
        DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
        return -1;
      }
    }
  }

  simpleTag=0;
  if (n->type==GWEN_XMLNodeTypeTag) {
    err=GWEN_BufferedIO_WriteChar(bio, '<');
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
      return -1;
    }
    if (n->data) {
      GWEN_BUFFER *tbuf;

      tbuf=GWEN_Buffer_new(0, strlen(n->data), 0, 1);
      if (GWEN_Text_EscapeXmlToBuffer(n->data, tbuf)) {
        GWEN_Buffer_free(tbuf);
        DBG_INFO(GWEN_LOGDOMAIN, "here");
        return -1;
      }
      err=GWEN_BufferedIO_Write(bio, GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_free(tbuf);
    }
    else
      err=GWEN_BufferedIO_Write(bio, "UNKNOWN");
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
      return -1;
    }
    p=n->properties;
    if (p) {
      while (p) {
        GWEN_BUFFER *tbuf;

        if (flags & GWEN_XML_FLAGS_SIMPLE) {
          err=GWEN_BufferedIO_WriteChar(bio, ' ');
          if (!GWEN_Error_IsOk(err)) {
            DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
            return -1;
          }
        }
        else {
	  err=GWEN_BufferedIO_WriteLine(bio, "");
	  if (!GWEN_Error_IsOk(err)) {
	    DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
	    return -1;
	  }

	  if (flags & GWEN_XML_FLAGS_INDENT) {
	    if (GWEN_BufferedIO_GetLinePos(bio)==0){
	      for(i=0; i<ind+3; i++) {
		err=GWEN_BufferedIO_WriteChar(bio, ' ');
		if (!GWEN_Error_IsOk(err)) {
		  DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
		  return -1;
		}
	      }
	    }
	  }
	}

        tbuf=GWEN_Buffer_new(0, strlen(p->name), 0, 1);
        if (GWEN_Text_EscapeXmlToBuffer(p->name, tbuf)) {
          GWEN_Buffer_free(tbuf);
          DBG_INFO(GWEN_LOGDOMAIN, "here");
          return -1;
        }
        err=GWEN_BufferedIO_Write(bio, GWEN_Buffer_GetStart(tbuf));
        GWEN_Buffer_free(tbuf);
        if (!GWEN_Error_IsOk(err)) {
          DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
          return -1;
        }
  
        if (p->value) {
          err=GWEN_BufferedIO_Write(bio, "=\"");
          if (!GWEN_Error_IsOk(err)) {
            DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
            return -1;
          }
          tbuf=GWEN_Buffer_new(0, strlen(p->value), 0, 1);
          if (GWEN_Text_EscapeXmlToBuffer(p->value, tbuf)) {
            GWEN_Buffer_free(tbuf);
            DBG_INFO(GWEN_LOGDOMAIN, "here");
            return -1;
          }
          err=GWEN_BufferedIO_Write(bio, GWEN_Buffer_GetStart(tbuf));
          GWEN_Buffer_free(tbuf);
          if (!GWEN_Error_IsOk(err)) {
            DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
            return -1;
          }
          err=GWEN_BufferedIO_Write(bio, "\"");
          if (!GWEN_Error_IsOk(err)) {
            DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
            return -1;
          }
        }
        p=p->next;
      } /* while */

      if (!(flags & GWEN_XML_FLAGS_SIMPLE)) {
        err=GWEN_BufferedIO_WriteLine(bio, "");
        if (!GWEN_Error_IsOk(err)) {
          DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
          return -1;
        }
        if (GWEN_BufferedIO_GetLinePos(bio)==0){
          for(i=0; i<ind; i++) {
            err=GWEN_BufferedIO_WriteChar(bio, ' ');
            if (!GWEN_Error_IsOk(err)) {
              DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
              return -1;
            }
          }
        }
      }
    } /* if there are properties */

    if (n->data) {
      if (n->data[0]=='?') {
        simpleTag=1;
        err=GWEN_BufferedIO_WriteChar(bio, '?');
        if (!GWEN_Error_IsOk(err)) {
          DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
          return -1;
        }
      }
      else if (n->data[0]=='!') {
        simpleTag=1;
      }
    }

    if (n->child==0) {
      if (n->data[0]=='?')
	err=GWEN_BufferedIO_Write(bio, ">");
      else
	err=GWEN_BufferedIO_Write(bio, "/>");
      if (!GWEN_Error_IsOk(err)) {
        DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
        return -1;
      }
    }
    else {
      if ((flags & GWEN_XML_FLAGS_SIMPLE) &&
          !GWEN_XMLNode_GetFirstTag(n))
        err=GWEN_BufferedIO_Write(bio, ">");
      else
        err=GWEN_BufferedIO_WriteLine(bio, ">");
      if (!GWEN_Error_IsOk(err)) {
        DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
        return -1;
      }

      if (!simpleTag) {
        c=n->child;
        while(c) {
          if (GWEN_XMLNode__WriteToStream(c, bio, flags, ind+2)) {
            DBG_INFO(GWEN_LOGDOMAIN, "Error writing tag \"%s\"", n->data);
            return -1;
          }
          if ((c->type!=GWEN_XMLNodeTypeData) ||
              (c->type==GWEN_XMLNodeTypeData &&
               !(flags & GWEN_XML_FLAGS_SIMPLE))){
            err=GWEN_BufferedIO_WriteLine(bio, "");
            if (!GWEN_Error_IsOk(err)) {
              DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
              return -1;
            }
          }
          c=c->next;
        }
        if ((flags & GWEN_XML_FLAGS_INDENT) &&
            GWEN_BufferedIO_GetLinePos(bio)==0){
          for(i=0; i<ind; i++) {
            err=GWEN_BufferedIO_WriteChar(bio, ' ');
            if (!GWEN_Error_IsOk(err)) {
              DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
              return -1;
            }
          }
        }
        err=GWEN_BufferedIO_Write(bio, "</");
        if (!GWEN_Error_IsOk(err)) {
          DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
          return -1;
        }
        if (n->data) {
          GWEN_BUFFER *tbuf;

          tbuf=GWEN_Buffer_new(0, strlen(n->data), 0, 1);
          if (GWEN_Text_EscapeXmlToBuffer(n->data, tbuf)) {
            GWEN_Buffer_free(tbuf);
            DBG_INFO(GWEN_LOGDOMAIN, "here");
            return -1;
          }
          err=GWEN_BufferedIO_Write(bio, GWEN_Buffer_GetStart(tbuf));
          GWEN_Buffer_free(tbuf);
        }
        else
          err=GWEN_BufferedIO_Write(bio, "UNKNOWN");
        if (!GWEN_Error_IsOk(err)) {
          DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
          return -1;
        }
  
        err=GWEN_BufferedIO_Write(bio, ">");
        if (!GWEN_Error_IsOk(err)) {
          DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
          return -1;
        }
        if (!(flags & GWEN_XML_FLAGS_SIMPLE)) {
          err=GWEN_BufferedIO_WriteLine(bio, "");
          if (!GWEN_Error_IsOk(err)) {
            DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
            return -1;
          }
        }
      } /* if no simply tag */
    } /* if tag has children */
  } /* if tag */
  else if (n->type==GWEN_XMLNodeTypeData) {
    if (n->data) {
      GWEN_BUFFER *tbuf;

      tbuf=GWEN_Buffer_new(0, strlen(n->data), 0, 1);
      if (GWEN_Text_EscapeXmlToBuffer(n->data, tbuf)) {
        GWEN_Buffer_free(tbuf);
        DBG_INFO(GWEN_LOGDOMAIN, "here");
        return -1;
      }

      if (flags & GWEN_XML_FLAGS_SIMPLE)
        err=GWEN_BufferedIO_Write(bio, GWEN_Buffer_GetStart(tbuf));
      else
        err=GWEN_BufferedIO_WriteLine(bio, GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_free(tbuf);
      if (!GWEN_Error_IsOk(err)) {
        DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
        return -1;
      }
    }
  }
  else if (n->type==GWEN_XMLNodeTypeComment) {
    if (flags & GWEN_XML_FLAGS_WRITE_COMMENTS) {
      err=GWEN_BufferedIO_Write(bio, "<!--");
      if (!GWEN_Error_IsOk(err)) {
        DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
        return -1;
      }

      if (n->data) {
        GWEN_BUFFER *tbuf;
  
        tbuf=GWEN_Buffer_new(0, strlen(n->data), 0, 1);
        if (GWEN_Text_EscapeXmlToBuffer(n->data, tbuf)) {
          GWEN_Buffer_free(tbuf);
          DBG_INFO(GWEN_LOGDOMAIN, "here");
          return -1;
        }
        err=GWEN_BufferedIO_Write(bio, GWEN_Buffer_GetStart(tbuf));
        GWEN_Buffer_free(tbuf);
        if (!GWEN_Error_IsOk(err)) {
          DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
          return -1;
        }
      }
      err=GWEN_BufferedIO_Write(bio, "-->");
      if (!GWEN_Error_IsOk(err)) {
        DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
        return -1;
      }
    }
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Unknown tag type (%d)", n->type);
  }

  return 0;
}



int GWEN_XMLNode_WriteToStream(const GWEN_XMLNODE *n,
                               GWEN_BUFFEREDIO *bio,
                               GWEN_TYPE_UINT32 flags){
  const GWEN_XMLNODE *nn;

  if (n->header && (flags & GWEN_XML_FLAGS_HANDLE_HEADERS)) {
    GWEN_TYPE_UINT32 lflags;
    lflags=flags & ~GWEN_XML_FLAGS_HANDLE_HEADERS;
    nn=n->header;
    while(nn) {
      if (GWEN_XMLNode__WriteToStream(nn, bio, lflags, 0))
        return -1;
      if (nn->next) {
        GWEN_ERRORCODE err;

        err=GWEN_BufferedIO_WriteLine(bio, "");
        if (!GWEN_Error_IsOk(err)) {
          DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
          return -1;
        }
      }

      nn=nn->next;
    }
    if (n->child) {
      GWEN_ERRORCODE err;

      err=GWEN_BufferedIO_WriteLine(bio, "");
      if (!GWEN_Error_IsOk(err)) {
        DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
        return -1;
      }
    }
  }

  nn=n->child;
  while(nn) {
    if (GWEN_XMLNode__WriteToStream(nn, bio, flags, 0))
      return -1;
    if (nn->next) {
      GWEN_ERRORCODE err;

      err=GWEN_BufferedIO_WriteLine(bio, "");
      if (!GWEN_Error_IsOk(err)) {
        DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
        return -1;
      }
    }

    nn=nn->next;
  } /* while */

  return 0;
}



int GWEN_XMLNode_WriteFile(const GWEN_XMLNODE *n,
                           const char *fname,
                           GWEN_TYPE_UINT32 flags){
  GWEN_BUFFEREDIO *bio;
  GWEN_ERRORCODE err;
  int fd;
  int rv;

  fd=open(fname, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  if (fd==-1) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Error opening file \"%s\": %s",
              fname,
              strerror(errno));
    return -1;
  }

  bio=GWEN_BufferedIO_File_new(fd);
  GWEN_BufferedIO_SetWriteBuffer(bio, 0, 1024);
  rv=GWEN_XMLNode_WriteToStream(n, bio, flags);
  err=GWEN_BufferedIO_Close(bio);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO(GWEN_LOGDOMAIN, "called from here");
    GWEN_BufferedIO_free(bio);
    return -1;
  }
  GWEN_BufferedIO_free(bio);
  return rv;
}




const char *GWEN_XMLNode_GetCharValue(const GWEN_XMLNODE *n,
                                      const char *name,
                                      const char *defValue) {
  GWEN_XMLNODE *nn;

  nn=GWEN_XMLNode_FindFirstTag(n, name, 0, 0);
  while(nn) {
    GWEN_XMLNODE *dn;

    dn=GWEN_XMLNode_GetFirstData(nn);
    if (dn) {
      if (dn->data)
        return dn->data;
    }
    nn=GWEN_XMLNode_FindNextTag(nn, name, 0, 0);
  }

  return defValue;
}



const char *GWEN_XMLNode_GetLocalizedCharValue(const GWEN_XMLNODE *n,
                                               const char *name,
                                               const char *defValue) {
  GWEN_XMLNODE *nn=0;
  GWEN_STRINGLIST *langl;

  langl=GWEN_I18N_GetCurrentLocaleList();
  if (langl) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(langl);
    while(se) {
      const char *l;

      l=GWEN_StringListEntry_Data(se);
      DBG_NOTICE(GWEN_LOGDOMAIN, "Trying locale \"%s\"", l);
      assert(l);
      nn=GWEN_XMLNode_FindFirstTag(n, name, "lang", l);
      while(nn) {
	GWEN_XMLNODE *dn;

	dn=GWEN_XMLNode_GetFirstData(nn);
	if (dn) {
	  if (dn->data && *(dn->data))
	    return dn->data;
	}
	nn=GWEN_XMLNode_FindNextTag(nn, name, "lang", l);
      } /* while nn */
      se=GWEN_StringListEntry_Next(se);
    } /* while */
  } /* if language list available */

  /* otherwise try without locale */
  nn=GWEN_XMLNode_FindFirstTag(n, name, 0, 0);
  while(nn) {
    GWEN_XMLNODE *dn;

    dn=GWEN_XMLNode_GetFirstData(nn);
    if (dn) {
      if (dn->data)
        return dn->data;
    }
    nn=GWEN_XMLNode_FindNextTag(nn, name, 0, 0);
  }

  return defValue;
}



void GWEN_XMLNode_SetCharValue(GWEN_XMLNODE *n,
                               const char *name,
                               const char *value){
  GWEN_XMLNODE *nn;

  nn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, name);
  if (value) {
    GWEN_XMLNODE *nnn;

    nnn=GWEN_XMLNode_new(GWEN_XMLNodeTypeData, value);
    GWEN_XMLNode_AddChild(nn, nnn);
  }
  GWEN_XMLNode_AddChild(n, nn);
}



int GWEN_XMLNode_GetIntValue(const GWEN_XMLNODE *n,
                             const char *name,
                             int defValue) {
  const char *p;
  int res;

  p=GWEN_XMLNode_GetCharValue(n, name, 0);
  if (!p)
    return defValue;
  if (1!=sscanf(p, "%i", &res))
    return defValue;
  return res;
}



void GWEN_XMLNode_SetIntValue(GWEN_XMLNODE *n,
                              const char *name,
                              int value){
  char numbuf[32];

  snprintf(numbuf, sizeof(numbuf)-1, "%d", value);
  numbuf[sizeof(numbuf)-1]=0;
  GWEN_XMLNode_SetCharValue(n, name, numbuf);
}



GWEN_XMLPROPERTY *GWEN_XMLNode_GetFirstProperty(const GWEN_XMLNODE *n){
  assert(n);
  return n->properties;
}



GWEN_XMLPROPERTY *GWEN_XMLNode_GetNextProperty(const GWEN_XMLNODE *n,
                                               const GWEN_XMLPROPERTY *pr){
  assert(n);
  assert(pr);
  return pr->next;
}



const char *GWEN_XMLProperty_GetName(const GWEN_XMLPROPERTY *pr){
  assert(pr);
  return pr->name;
}



const char *GWEN_XMLProperty_GetValue(const GWEN_XMLPROPERTY *pr){
  assert(pr);
  return pr->value;
}



int GWEN_XMLNode_IsChildOf(const GWEN_XMLNODE *parent,
                           const GWEN_XMLNODE *child) {
  GWEN_XMLNODE *n;

  if (!child || !parent || child==parent)
    return 0;
  n=child->parent;
  while(n) {
    if (n==parent)
      return 1;
    n=n->parent;
  }
  return 0;
}



int GWEN_XMLNode_GetXPath(const GWEN_XMLNODE *n1,
                          const GWEN_XMLNODE *n2,
                          GWEN_BUFFER *nbuf) {
  GWEN_BUFFER *lbuf;
  const GWEN_XMLNODE *ln1;
  const GWEN_XMLNODE *ln2;

  if (!n1 && !n2) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Both nodes are NULL");
    return -1;
  }

  if (!n1) {
    n1=n2;
    while(n1->parent)
      n1=n1->parent;
  }

  if (!n2) {
    n2=n1;
    while(n2->parent)
      n2=n2->parent;
  }

  if (n2==n1) {
    GWEN_Buffer_AppendString(nbuf, "here()");
    return 0;
  }

  lbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_ReserveBytes(lbuf, 128);

  ln1=n1->parent;
  if (ln1) {
    GWEN_Buffer_AppendString(lbuf, "../");
    while(ln1) {
      if (ln1==n2) {
        /* found n2 */
        GWEN_Buffer_AppendBuffer(nbuf, lbuf);
        GWEN_Buffer_free(lbuf);
        return 0;
      }
      if (GWEN_XMLNode_IsChildOf(ln1, n2))
        break;
      ln1=ln1->parent;
      GWEN_Buffer_AppendString(lbuf, "../");
    }

    if (!ln1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Nodes do not share root node");
      GWEN_Buffer_free(lbuf);
      return -1;
    }

    /* append path to n1 */
    GWEN_Buffer_AppendBuffer(nbuf, lbuf);
  }
  DBG_ERROR(GWEN_LOGDOMAIN, "Path so far: %s", GWEN_Buffer_GetStart(lbuf));

  /* get path to n2 */
  GWEN_Buffer_Reset(lbuf);

  ln2=n2;
  while(ln2) {
    GWEN_XMLNODE *tn;
    int idx;
    char idxbuf[32];

    if (ln2->parent==ln1)
      break;

    /* count occurences of this tag in this level */
    idx=1;
    tn=ln2->parent;
    if (tn) {
      tn=GWEN_XMLNode_FindFirstTag(tn, ln2->data, 0, 0);

      while(tn) {
        if (tn==ln2)
          break;
        idx++;
        tn=GWEN_XMLNode_FindNextTag(tn, ln2->data, 0, 0);
      }
    }

    snprintf(idxbuf, sizeof(idxbuf), "[%d]", idx);
    idxbuf[sizeof(idxbuf)-1]=0;
    GWEN_Buffer_InsertString(lbuf, idxbuf);
    GWEN_Buffer_InsertString(lbuf, GWEN_XMLNode_GetData(ln2));
    GWEN_Buffer_InsertByte(lbuf, '/');
    ln2=ln2->parent;
  }
  DBG_ERROR(GWEN_LOGDOMAIN, "Path so far: %s", GWEN_Buffer_GetStart(lbuf));
  assert(ln2);

  /* append path to n2 */
  GWEN_Buffer_AppendBuffer(nbuf, lbuf);
  GWEN_Buffer_free(lbuf);
  return 0;
}



void* GWEN_XMLNode_HandlePath(const char *entry,
                              void *data,
                              GWEN_TYPE_UINT32 flags) {
  GWEN_XMLNODE *n;
  GWEN_XMLNODE *nn;
  int idx;
  char *tag;
  char *p;

  n=(GWEN_XMLNODE*)data;

  if (flags & GWEN_PATH_FLAGS_VARIABLE) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "GWEN_PATH_FLAGS_VARIABLE not allowed for XPATH");
    return 0;
  }

  if (flags & GWEN_PATH_FLAGS_ROOT) {
    while(n->parent)
      n=n->parent;
    if (*entry=='/')
      entry++;
  }

  if (strcasecmp(entry, "..")==0) {
    return n->parent;
  }
  else if (strcasecmp(entry, ".")==0 ||
           strcasecmp(entry, "here()")==0) {
    return n;
  }

  idx=1;
  tag=strdup(entry);
  assert(tag);
  p=strchr(tag, '[');
  if (p) {
    *p=0;
    p++;
    if (1!=sscanf(p, "%d]", &idx)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Bad path entry \"%s\" (bad index)",
                entry);
      free(tag);
      return 0;
    }
    if (idx<1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Bad index %d in path element \"%s\"",
                idx, entry);
      free(tag);
      return 0;
    }
  }

  /* check whether we are allowed to simply create the node */
  if (
      ((flags & GWEN_PATH_FLAGS_LAST) &&
       (((flags & GWEN_PATH_FLAGS_VARIABLE) &&
         (flags & GWEN_PATH_FLAGS_CREATE_VAR)) ||
        (!(flags & GWEN_PATH_FLAGS_VARIABLE) &&
         (flags & GWEN_PATH_FLAGS_CREATE_GROUP)))
      ) ||
      (
       !(flags & GWEN_PATH_FLAGS_LAST) &&
       (flags & GWEN_PATH_FLAGS_PATHCREATE))
     ) {
    /* simply create the new variable/group */
    if (flags & GWEN_PATH_FLAGS_VARIABLE) {
      /* not allowed for now */
      free(tag);
      return 0;
    }
    else {
      if (idx!=1) {
        DBG_ERROR(GWEN_LOGDOMAIN,
                  "Can not create tag with index!=1 (%s)", entry);
        free(tag);
        return 0;
      }
      DBG_VERBOUS(GWEN_LOGDOMAIN,
                  "Unconditionally creating tag \"%s\"", entry);
      nn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, tag);
      GWEN_XMLNode_AddChild(n, nn);
      free(tag);
      return nn;
    }
  }

  /* find the node */
  if (flags & GWEN_PATH_FLAGS_VARIABLE) {
    nn=0;
  }
  else {
    int i;

    i=idx;
    nn=GWEN_XMLNode_FindFirstTag(n, tag, 0, 0);
    while(--i) {
      nn=GWEN_XMLNode_FindNextTag(nn, tag, 0, 0);
    }
  }

  if (!nn) {
    /* node not found, check, if we are allowed to create it */
    if (
        (!(flags & GWEN_PATH_FLAGS_LAST) &&
         (flags & GWEN_PATH_FLAGS_PATHMUSTEXIST)) ||
        (flags & GWEN_PATH_FLAGS_NAMEMUSTEXIST)
       ) {
      if (flags & GWEN_PATH_FLAGS_VARIABLE) {
        DBG_VERBOUS(GWEN_LOGDOMAIN,
                    "Variable \"%s\" does not exist", entry);
      }
      else {
        DBG_VERBOUS(GWEN_LOGDOMAIN,
                    "Tag \"%s\" does not exist", entry);
      }
      free(tag);
      return 0;
    }
    /* create the new variable/group */
    if (flags & GWEN_PATH_FLAGS_VARIABLE) {
      /* variable not allowed */
      free(tag);
      return 0;
    }
    else {
      DBG_VERBOUS(GWEN_LOGDOMAIN,
                  "Tag \"%s\" not found, creating", entry);
      nn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, tag);
      GWEN_XMLNode_AddChild(n, nn);
    }
  } /* if node not found */
  else {
    /* node does exist, check whether this is ok */
    if (
        ((flags & GWEN_PATH_FLAGS_LAST) &
         (flags & GWEN_PATH_FLAGS_NAMEMUSTNOTEXIST)) ||
        (!(flags & GWEN_PATH_FLAGS_LAST) &
         (flags & GWEN_PATH_FLAGS_PATHMUSTNOTEXIST))
       ) {
      DBG_VERBOUS(GWEN_LOGDOMAIN, "Entry \"%s\" already exists", entry);
      free(tag);
      return 0;
    }
  }

  free(tag);
  return nn;
}



GWEN_XMLNODE *GWEN_XMLNode_GetNodeByXPath(GWEN_XMLNODE *n,
                                          const char *path,
                                          GWEN_TYPE_UINT32 flags){
  return (GWEN_XMLNODE*)GWEN_Path_Handle(path,
                                         (void*)n,
                                         flags,
                                         GWEN_XMLNode_HandlePath);
}



GWEN_XMLNODE *GWEN_XMLNode_GetHeader(const GWEN_XMLNODE *n){
  assert(n);
  return n->header;
}



void GWEN_XMLNode_AddHeader(GWEN_XMLNODE *n, GWEN_XMLNODE *nh){
  assert(n);
  assert(nh);
  GWEN_XMLNode_add(nh, &(n->header));
}



void GWEN_XMLNode_DelHeader(GWEN_XMLNODE *n, GWEN_XMLNODE *nh){
  assert(n);
  assert(nh);
  GWEN_XMLNode_del(nh, &(n->header));
}



void GWEN_XMLNode_ClearHeaders(GWEN_XMLNODE *n){
  assert(n);
  GWEN_XMLNode_freeAll(n->header);
  n->header=0;
}











int GWEN_XML_AddNameSpace(GWEN_STRINGLIST2 *sl,
                          const char *prefix,
                          const char *name){
  GWEN_BUFFER *nbuf;
  int rv;

  nbuf=GWEN_Buffer_new(0, 32, 0, 1);
  if (prefix)
    GWEN_Buffer_AppendString(nbuf, prefix);
  GWEN_Buffer_AppendByte(nbuf, ':');
  GWEN_Buffer_AppendString(nbuf, name);
  rv=GWEN_StringList2_AppendString(sl, GWEN_Buffer_GetStart(nbuf), 0,
                                   GWEN_StringList2_IntertMode_NoDouble);
  GWEN_Buffer_free(nbuf);
  return rv;
}



const char *GWEN_XML_FindNameSpaceByPrefix(GWEN_STRINGLIST2 *sl,
                                           const char *s){
  GWEN_STRINGLIST2_ITERATOR *it;

  it=GWEN_StringList2_First(sl);
  if (it) {
    const char *t;

    t=GWEN_StringList2Iterator_Data(it);
    assert(t);
    while(t) {
      const char *p;

      p=strchr(t, ':');
      assert(p);
      if ((s==0 && p==t) || (s && strncasecmp(t, s, p-t)==0))
        return t;
      t=GWEN_StringList2Iterator_Next(it);
    } /* while */
    GWEN_StringList2Iterator_free(it);
  }
  return 0;
}



const char *GWEN_XML_FindNameSpaceByName(GWEN_STRINGLIST2 *sl,
                                         const char *s){
  GWEN_STRINGLIST2_ITERATOR *it;

  it=GWEN_StringList2_First(sl);
  if (it) {
    const char *t;

    t=GWEN_StringList2Iterator_Data(it);
    assert(t);
    while(t) {
      const char *p;

      p=strchr(t, ':');
      assert(p);
      p++;
      if (strcasecmp(p, s)==0) {
	GWEN_StringList2Iterator_free(it);
	return t;
      }
      t=GWEN_StringList2Iterator_Next(it);
    } /* while */
    GWEN_StringList2Iterator_free(it);
  }
  return 0;
}



const char *GWEN_XML_FindNameSpace(GWEN_STRINGLIST2 *sl,
                                   const char *prefix,
                                   const char *name){
  GWEN_BUFFER *nbuf;
  GWEN_STRINGLIST2_ITERATOR *it;

  nbuf=GWEN_Buffer_new(0, 32, 0, 1);
  if (prefix)
    GWEN_Buffer_AppendString(nbuf, prefix);
  GWEN_Buffer_AppendByte(nbuf, ':');
  GWEN_Buffer_AppendString(nbuf, name);

  it=GWEN_StringList2_First(sl);
  if (it) {
    const char *t;

    t=GWEN_StringList2Iterator_Data(it);
    assert(t);
    while(t) {
      const char *p;

      p=strchr(t, ':');
      assert(p);
      p++;
      if (strcasecmp(p, GWEN_Buffer_GetStart(nbuf))==0) {
	GWEN_StringList2Iterator_free(it);
	GWEN_Buffer_free(nbuf);
	return t;
      }
      t=GWEN_StringList2Iterator_Next(it);
    } /* while */
    GWEN_StringList2Iterator_free(it);
  }

  GWEN_Buffer_free(nbuf);
  return 0;
}



int GWEN_XMLNode__CheckNameSpaceDecls1(GWEN_XMLNODE *n,
                                       GWEN_STRINGLIST2 *sl,
                                       const char *currentNameSpace) {
  GWEN_XMLPROPERTY *pr;
  GWEN_XMLNODE *nn;
  char *localNameSpace;

  localNameSpace=0;

  /* remove all unnecessary namespace declarations from this node */
  pr=n->properties;
  while(pr) {
    GWEN_XMLPROPERTY *prNext;

    prNext=pr->next;
    if (strcasecmp(pr->name, "xmlns")==0) {
      /* default namespace changed ? */
      if (localNameSpace) {
        if (strcasecmp(pr->value, localNameSpace)==0) {
          /* already mentioned name space, remove duplicate property */
          GWEN_XMLProperty_del(pr, &n->properties);
          GWEN_XMLProperty_free(pr);
        }
        else {
          /* current namespace changed */
          free(localNameSpace);
          localNameSpace=strdup(pr->value);
        }
      }
      else if (currentNameSpace) {
        if (strcasecmp(pr->value, currentNameSpace)==0) {
          /* already active name space, remove property */
          GWEN_XMLProperty_del(pr, &n->properties);
          GWEN_XMLProperty_free(pr);
        }
        else {
          /* current namespace changed */
          free(localNameSpace);
          localNameSpace=strdup(pr->value);
        }
      }
      else {
        /* set current namespace */
        free(localNameSpace);
        localNameSpace=strdup(pr->value);
      }
    }
    else if (strncasecmp(pr->name, "xmlns:", 6)==0) {
      const char *prefix;
      const char *x;

      prefix=strchr(pr->name, ':');
      prefix++;

      /* check for redefinition */
      x=GWEN_XML_FindNameSpaceByName(sl, prefix);
      if (x) {
        const char *p;

        /* prefix already in use, check whether it is the same namespace */
        p=strchr(x, ':');
        assert(p);
        p++;
        if (strcasecmp(p, pr->value)!=0) {
          GWEN_BUFFER *xpath;

          /* same prefix, different namespace */
          xpath=GWEN_Buffer_new(0, 256, 0, 1);
          GWEN_XMLNode_GetXPath(0, n, xpath);
          DBG_ERROR(GWEN_LOGDOMAIN,
                    "Redefinition of namespace prefix \"%s\" in \"%s\"",
                    prefix, GWEN_Buffer_GetStart(xpath));
          GWEN_Buffer_free(xpath);
          return -1;
        } /* if different namespace for same prefix */
        else {
          /* already in list, remove property here */
          GWEN_XMLProperty_del(pr, &n->properties);
          GWEN_XMLProperty_free(pr);
        }
      }
      else {
        GWEN_XML_AddNameSpace(sl, prefix, pr->value);
      }
    }
    pr=prNext;
  } /* while */

  /* do the same on all sub nodes */
  nn=GWEN_XMLNode_GetFirstTag(n);
  while(nn) {
    int rv;

    rv=GWEN_XMLNode__CheckNameSpaceDecls1(nn, sl,
                                          localNameSpace?localNameSpace:
                                          currentNameSpace);
    if (rv) {
      free(localNameSpace);
      return rv;
    }
    nn=GWEN_XMLNode_GetNextTag(nn);
  }

  free(localNameSpace);
  return 0;
}



int GWEN_XMLNode__CheckAndSetNameSpace(GWEN_XMLNODE *n,
                                       const char *prefix,
                                       const char *nspace) {
  GWEN_XMLPROPERTY *pr;
  const char *p;
  int inUse;

  inUse=0;
  /* check current tag for prefix */
  if (prefix) {
    p=strchr(n->data, ':');
    if (p) {
      if (strncasecmp(n->data, prefix, p-n->data)==0) {
        DBG_DEBUG(GWEN_LOGDOMAIN, "Prefix \"%s\" used in tag \"%s\"",
                   prefix, n->data);
        inUse=1;
      }
    }

    if (!inUse) {
      /* check all attributes for prefixes */
      pr=n->properties;
      while(pr) {
        p=strchr(pr->name, ':');
        if (p) {
          if (strncasecmp(pr->name, prefix, p-pr->name)==0) {
            DBG_DEBUG(GWEN_LOGDOMAIN,
                       "Prefix \"%s\" used in attribute \"%s\" of tag \"%s\"",
                       prefix, pr->name, n->data);
            inUse=1;
            break;
          }
          else {
            DBG_DEBUG(GWEN_LOGDOMAIN,
                       "Prefix \"%s\" not used in attribute \"%s\" of tag \"%s\"",
                       prefix, pr->name, n->data);
          }
        }
        pr=pr->next;
      } /* while */
    }
  } /* if prefix */
  else {
    /* no prefix, check whether the current element hasn't any */
    p=strchr(n->data, ':');
    if (!p) {
      /* current tag has no prefix, check whether we have a namespace
       * declaration here */
      if (GWEN_XMLNode_GetProperty(n, "xmlns", 0)==0) {
        /* no, so the current namespace from above is used */
        DBG_DEBUG(GWEN_LOGDOMAIN,
                   "No prefix, current namespace is used");
        inUse=1;
      }
    }
  } /* if no prefix */

  if (inUse) {
    GWEN_BUFFER *nbuf;

    nbuf=GWEN_Buffer_new(0, 32, 0, 1);
    GWEN_Buffer_AppendString(nbuf, "xmlns");
    if (prefix) {
      GWEN_Buffer_AppendByte(nbuf, ':');
      GWEN_Buffer_AppendString(nbuf, prefix);
    }
    GWEN_XMLNode__SetProperty(n, GWEN_Buffer_GetStart(nbuf), nspace, 1);
    GWEN_Buffer_free(nbuf);
    return 1;
  }

  return 0;
}



int GWEN_XMLNode__SetNameSpaces(GWEN_XMLNODE *n,
                                const char *prefix,
                                const char *nspace) {
  GWEN_XMLNODE *nn;
  int rv;

  rv=GWEN_XMLNode__CheckAndSetNameSpace(n, prefix, nspace);
  if (rv)
    return rv;

  nn=GWEN_XMLNode_GetFirstTag(n);
  while(nn) {
    int rv;

    rv=GWEN_XMLNode__CheckAndSetNameSpace(nn, prefix, nspace);
    if (rv==-1)
      return rv;
    else if (rv==0) {
      /* check children */
      rv=GWEN_XMLNode__SetNameSpaces(nn, prefix, nspace);
      if (rv)
        return rv;
    }

    nn=GWEN_XMLNode_GetNextTag(nn);
  }

  return 0;
}



int GWEN_XMLNode__CheckNameSpaceDecls3(GWEN_XMLNODE *n) {
  GWEN_XMLPROPERTY *pr;
  GWEN_XMLNODE *nn;
  int rv;

  /* move all namespace declarations from this node to the nodes
   * of first use */
  pr=n->properties;
  while(pr) {
    GWEN_XMLPROPERTY *prNext;

    prNext=pr->next;
    if (strcasecmp(pr->name, "xmlns")==0 ||
        strncasecmp(pr->name, "xmlns:", 6)==0) {
      const char *prefix;

      prefix=strchr(pr->name, ':');
      if (prefix)
        prefix++;

      GWEN_XMLProperty_del(pr, &n->properties);
      rv=GWEN_XMLNode__SetNameSpaces(n, prefix, pr->value);
      DBG_DEBUG(GWEN_LOGDOMAIN, "Removing property \"%s\"",
                 pr->name);
      GWEN_XMLProperty_free(pr);
      if (rv==-1)
        return rv;
    }
    pr=prNext;
  } /* while */

  /* do the same on all sub nodes */
  nn=GWEN_XMLNode_GetFirstTag(n);
  while(nn) {
    int rv;

    rv=GWEN_XMLNode__CheckNameSpaceDecls3(nn);
    if (rv) {
      return rv;
    }
    nn=GWEN_XMLNode_GetNextTag(nn);
  }

  return 0;
}




int GWEN_XMLNode_NormalizeNameSpaces(GWEN_XMLNODE *n) {
  const char *ns;
  int rv;
  GWEN_STRINGLIST2 *sl;

  ns=GWEN_XMLNode_GetProperty(n, "xmlns", 0);
  sl=GWEN_StringList2_new();
  rv=GWEN_XMLNode__CheckNameSpaceDecls1(n, sl, ns);
  GWEN_StringList2_free(sl);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    return rv;
  }

  //rv=GWEN_XMLNode__CheckNameSpaceDecls2(n, ns);
  rv=GWEN_XMLNode__CheckNameSpaceDecls3(n);
  if (rv==-1)
    return rv;
  return 0;
}









