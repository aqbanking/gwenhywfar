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


void GWEN_XMLNode_SetProperty(GWEN_XMLNODE *n,
                              const char *name, const char *value){
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
    GWEN_XMLProperty_add(p, &(n->properties));
  }
}



void GWEN_XMLNode_IncUsage(GWEN_XMLNODE *n){
  assert(n);
  n->usage++;
}



void GWEN_XMLNode_DecUsage(GWEN_XMLNODE *n){
  assert(n);
  if (n->usage==0) {
    DBG_WARN(0, "Node usage already is zero");
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
                          char chr,
                          const char *delims,
                          GWEN_BUFFER *buf) {
  int inQuote;
  int lastWasSpace;

  inQuote=0;
  lastWasSpace=0;

  while(1) {
    /* get character, if needed */
    if (chr==0) {
      if (GWEN_BufferedIO_CheckEOF(bio))
	break;
      chr=GWEN_BufferedIO_ReadChar(bio);
      if (chr<0) {
	DBG_ERROR(0, "Error on ReadChar");
	return -1;
      }
    }
    if (iscntrl(chr))
      chr=' ';

    if (inQuote) {
      if (chr=='"') {
	inQuote=0;
        break;
      }
      else {
        lastWasSpace=0;
        GWEN_Buffer_AppendByte(buf, chr);
      }
    }
    else {
      if (chr=='"') {
        lastWasSpace=0;
	inQuote=1;
      }
      else if (strchr(delims, chr)) {
	break;
      }
      else if (chr=='<') {
	DBG_ERROR(0, "No tags inside a tag definition allowed");
        return -1;
      }
      else {
        if (!inQuote) {
          if (isspace(chr)) {
            if (!lastWasSpace) {
              lastWasSpace=1;
              GWEN_Buffer_AppendByte(buf, chr);
            }
          }
          else {
            lastWasSpace=0;
            GWEN_Buffer_AppendByte(buf, chr);
          }
        }
        else {
          lastWasSpace=0;
          GWEN_Buffer_AppendByte(buf, chr);
        }
      }
    }
    chr=0;
  } /* while */

  if (chr=='"')
    return 0;
  if (lastWasSpace) {
    GWEN_TYPE_UINT32 s;

    s=GWEN_Buffer_GetUsedBytes(buf);
    GWEN_Buffer_Crop(buf, 0, s-1);
    GWEN_Buffer_SetPos(buf, s);
  }
  return chr;
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

  while (!GWEN_BufferedIO_CheckEOF(bio)) {
    /* read char (if none set) but skip blanks */
    if (chr==0) {
      chr=GWEN_BufferedIO_ReadChar(bio);
      if (chr<0) {
	DBG_ERROR(0, "Error on BufferedIO_ReadChar");
	return -1;
      }
    }
    eofMet=0;
    while(isspace(chr)) {
      if (GWEN_BufferedIO_CheckEOF(bio)) {
	eofMet=1;
        break;
      }
      chr=GWEN_BufferedIO_ReadChar(bio);
      if (chr<0) {
	DBG_ERROR(0, "Error on BufferedIO_ReadChar");
	return -1;
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
      chr=GWEN_XML__ReadWordBuf(bio, 0, " ><", bufTagName);
      if (chr<0) {
        GWEN_Buffer_free(bufTagName);
        return -1;
      }
      GWEN_Buffer_AppendByte(bufTagName, 0);
      p=GWEN_Buffer_GetStart(bufTagName);
      if (*p=='/') {
	isEndTag=1;
	p++;
      }
      DBG_DEBUG(0, "Found tag \"%s\"", GWEN_Buffer_GetStart(bufTagName));

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
        DBG_DEBUG(0, "Found comment");
	/* we have a remark tag, so read it over */
	while(1) {
	  if (chr==0) {
	    if (GWEN_BufferedIO_CheckEOF(bio)) {
	      DBG_ERROR(0, "Unexpected EOF within comment");
	      return -1;
	    }
	    chr=GWEN_BufferedIO_ReadChar(bio);
            if (chr<0) {
              GWEN_Buffer_free(bufComment);
              GWEN_Buffer_free(bufTagName);
	      return -1;
	    }
          }

          GWEN_Buffer_AppendByte(bufComment, chr);
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
                DBG_VERBOUS(0, "Clipping comment to 2 bytes");
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
        DBG_VERBOUS(0, "Comment finished");
        if (flags & GWEN_XML_FLAGS_READ_COMMENTS) {
          newNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeComment,
                                   GWEN_Buffer_GetStart(bufComment));
          GWEN_XMLNode_add(newNode, &(n->child));
          newNode->parent=n;
          DBG_DEBUG(0, "Added comment: \"%s\"",
                    GWEN_Buffer_GetStart(bufComment));
        }
        else {
          DBG_DEBUG(0, "Skip comment");
        }
        GWEN_Buffer_free(bufComment);
      } /* if remark */
      else {
	/* handle tagname */
	if (isEndTag) {
	  /* handle endtag */
	  if (currDepth<1) {
            DBG_ERROR(0, "Line %d: More endtags than start tags !",
                      GWEN_BufferedIO_GetLines(bio));
            GWEN_Buffer_free(bufTagName);
	    return -1;
	  }
	  if (strcasecmp(n->data, p)!=0) {
            DBG_ERROR(0,
                      "Line %d: endtag \"%s\" does not match "
                      "last start tag (\"%s\")",
                      GWEN_BufferedIO_GetLines(bio),
                      GWEN_Buffer_GetStart(bufTagName), n->data);
            GWEN_Buffer_free(bufTagName);
            return -1;
          }

          /* check whether the tag is "include" */
          if (!(flags & GWEN_XML_FLAGS_IGNORE_INCLUDE) &&
              strcasecmp(n->data, "include")==0 &&
              sl && fn) {
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
                DBG_WARN(0,
                         "Line %d: <INCLUDE> tag with unexpected children",
                         GWEN_BufferedIO_GetLines(bio));
              }
            }
            else {
              DBG_WARN(0, "Line %d: <INCLUDE> tag without children",
                       GWEN_BufferedIO_GetLines(bio));
            }

            if (iname) {
              int irv;
              GWEN_XMLNODE *newRoot;
              GWEN_XMLNODE *nparent;
              GWEN_XMLNODE *itag;

              newRoot=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "tmproot");
              DBG_INFO(0, "Line %d: Including file \"%s\" / \"%s\"",
                       GWEN_BufferedIO_GetLines(bio),
                       fpath, iname);
              irv=fn(newRoot, fpath, iname, sl, flags);
              if (irv) {
                DBG_INFO(0, "here");
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

                  DBG_INFO(0, "Line %d: Importing node to toplevel",
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
                      DBG_INFO(0,
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
            DBG_DEBUG(0, "One node done");
            GWEN_Buffer_free(bufTagName);
            return 0;
          }
        }
        else {
	  /* this is a start tag */
          GWEN_XMLNODE *newNode;
          GWEN_BUFFER *bufVarName;
          GWEN_BUFFER *bufValue;
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
            if (strcasecmp(p, "BR")==0 ||
                strcasecmp(p, "HR")==0)
              simpleTag=1;

	    j=strlen(p)-1;
	    if (p[j]=='/') {
	      if (chr!='>') {
                DBG_ERROR(0,
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
          newNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, p);
          while (chr!='>' && chr!='/') {
            GWEN_Buffer_Reset(bufVarName);
            GWEN_Buffer_Reset(bufValue);
            varname=0;
            value=0;

            /* skip blanks */
            /*chr=0; */
            DBG_DEBUG(0, "Current char is: \"%c\"", chr);
            if (chr==0 || isspace(chr)) {
              while(!GWEN_BufferedIO_CheckEOF(bio)) {
                chr=GWEN_BufferedIO_ReadChar(bio);
                if (chr<0) {
                  GWEN_XMLNode_free(newNode);
                  GWEN_Buffer_free(bufValue);
                  GWEN_Buffer_free(bufVarName);
                  GWEN_Buffer_free(bufTagName);
                  return -1;
                }
                if (!isspace(chr) && !iscntrl(chr))
                  break;
              } /* while */
            }
            if (chr==0) {
	      DBG_ERROR(0, "unexpected EOF");
	      GWEN_XMLNode_free(newNode);
              GWEN_Buffer_free(bufValue);
              GWEN_Buffer_free(bufVarName);
              GWEN_Buffer_free(bufTagName);
	      return -1;
	    }

	    if (chr=='>' || chr=='/')
	      break;

	    /* read possibly following var */
            chr=GWEN_XML__ReadWordBuf(bio, chr, " =/>", bufVarName);
            if (chr<0) {
	      GWEN_XMLNode_free(newNode);
              GWEN_Buffer_free(bufValue);
              GWEN_Buffer_free(bufVarName);
              GWEN_Buffer_free(bufTagName);
	      return -1;
	    }
            GWEN_Buffer_AppendByte(bufVarName, 0);
            varname=GWEN_Buffer_GetStart(bufVarName);
            if (strcmp(varname, "?")==0) {
              if (*p=='?') {
                DBG_DEBUG(0, "Found closing question mark");
                break;
              }
            }
            DBG_DEBUG(0, "Found property \"%s\"", varname);

	    /* skip blanks */
	    if (isspace(chr)) {
	      while(!GWEN_BufferedIO_CheckEOF(bio)) {
		chr=GWEN_BufferedIO_ReadChar(bio);
		if (chr<0) {
		  GWEN_XMLNode_free(newNode);
                  GWEN_Buffer_free(bufValue);
                  GWEN_Buffer_free(bufVarName);
                  GWEN_Buffer_free(bufTagName);
		  return -1;
		}
		if (!isspace(chr) && !iscntrl(chr))
		  break;
	      }
	    }

            value=0;
	    if (chr=='=') {
	      chr=0;
	      /* skip blanks */
	      while(!GWEN_BufferedIO_CheckEOF(bio)) {
		chr=GWEN_BufferedIO_ReadChar(bio);
		if (chr<0) {
		  GWEN_XMLNode_free(newNode);
                  GWEN_Buffer_free(bufValue);
                  GWEN_Buffer_free(bufVarName);
                  GWEN_Buffer_free(bufTagName);
		  return -1;
		}
		if (!isspace(chr))
		  break;
	      }
	      if (chr==0) {
                DBG_ERROR(0, "Line %d: Value expected for property \"%s\"",
                          GWEN_BufferedIO_GetLines(bio),
                          varname);
		GWEN_XMLNode_free(newNode);
                GWEN_Buffer_free(bufValue);
                GWEN_Buffer_free(bufVarName);
                GWEN_Buffer_free(bufTagName);
		return -1;
	      }

	      if (chr=='>' || chr=='/') {
                DBG_ERROR(0, "Line %d: Value expected for property \"%s\"",
                          GWEN_BufferedIO_GetLines(bio),
                          varname);
		GWEN_XMLNode_free(newNode);
                GWEN_Buffer_free(bufValue);
                GWEN_Buffer_free(bufVarName);
                GWEN_Buffer_free(bufTagName);
		return -1;
	      }

	      /* read value */
              chr=GWEN_XML__ReadWordBuf(bio, chr, " />", bufValue);
              if (chr<0) {
		GWEN_XMLNode_free(newNode);
                GWEN_Buffer_free(bufValue);
                GWEN_Buffer_free(bufVarName);
                GWEN_Buffer_free(bufTagName);
                return -1;
              }
              GWEN_Buffer_AppendByte(bufValue, 0);
              value=GWEN_Buffer_GetStart(bufValue);
              DBG_DEBUG(0, "Found value \"%s\"", value);
            } /* if value follows */

            if (varname) {
	      /* add property */
	      GWEN_XMLPROPERTY *newProp;

	      newProp=GWEN_XMLProperty_new(varname,
                                           value);
	      GWEN_XMLProperty_add(newProp, &(newNode->properties));
            } /* if varname */
          } /* while vars follow */

          if (chr=='/') {
            isEndTag=1;

            if (GWEN_BufferedIO_CheckEOF(bio)) {
              DBG_ERROR(0, "Line %d: \">\" expected",
                        GWEN_BufferedIO_GetLines(bio));
              GWEN_XMLNode_free(newNode);
              GWEN_Buffer_free(bufValue);
              GWEN_Buffer_free(bufVarName);
              GWEN_Buffer_free(bufTagName);
              return -1;
            }

            chr=GWEN_BufferedIO_ReadChar(bio);
            if (chr<0) {
              DBG_ERROR(0, "Error on ReadChar");
              GWEN_XMLNode_free(newNode);
              GWEN_Buffer_free(bufValue);
              GWEN_Buffer_free(bufVarName);
              GWEN_Buffer_free(bufTagName);
              return -1;
            }
          }

	  if (chr!='>') {
            DBG_ERROR(0, "Line %d: \">\" expected",
                      GWEN_BufferedIO_GetLines(bio));
	    GWEN_XMLNode_free(newNode);
            GWEN_Buffer_free(bufValue);
            GWEN_Buffer_free(bufVarName);
            GWEN_Buffer_free(bufTagName);
	    return -1;
	  }

          /* ok, now the tag is complete, add it */
          if ((flags & GWEN_XML_FLAGS_SHARE_TOPLEVEL) &&
              currDepth==0) {
            GWEN_XMLNODE *oldNode;

            oldNode=GWEN_XMLNode_FindFirstTag(n, newNode->data, 0, 0);
            if (oldNode) {
              /* use old node, copy properties */
              DBG_INFO(0, "Line %d: Using old toplevel node for \"%s\"",
                       GWEN_BufferedIO_GetLines(bio),
                       newNode->data);
              GWEN_XMLNode_CopyProperties(oldNode, newNode, 0);
              GWEN_XMLNode_free(newNode);
              newNode=oldNode;
            }
            else {
              /* otherwise add bew tag */
              if (currDepth>=GWEN_XML_MAX_DEPTH) {
                DBG_ERROR(0, "Line %d: Maximum depth exceeded",
                          GWEN_BufferedIO_GetLines(bio));
                GWEN_XMLNode_free(newNode);
                GWEN_Buffer_free(bufValue);
                GWEN_Buffer_free(bufVarName);
                GWEN_Buffer_free(bufTagName);
                return -1;
              }
              GWEN_XMLNode_add(newNode, &(n->child));
              newNode->parent=n;
              DBG_DEBUG(0, "Added node \"%s\"", newNode->data);
            }
          }
          else {
            if (currDepth>=GWEN_XML_MAX_DEPTH) {
              DBG_ERROR(0, "Line %d: Maximum depth exceeded",
                        GWEN_BufferedIO_GetLines(bio));
              GWEN_XMLNode_free(newNode);
              GWEN_Buffer_free(bufValue);
              GWEN_Buffer_free(bufVarName);
              GWEN_Buffer_free(bufTagName);
              return -1;
            }
            GWEN_XMLNode_add(newNode, &(n->child));
            newNode->parent=n;
            DBG_DEBUG(0, "Added node \"%s\"", newNode->data);
          }
          if (!isEndTag && !simpleTag) {
	    /* only dive if this tag is not immediately ended */
	    path[currDepth++]=n;
	    n=newNode;
          }
          else {
	    /* immediate endTag, if depth is 0: done */
	    if (currDepth==0) {
	      DBG_DEBUG(0, "One node done");
              GWEN_Buffer_free(bufValue);
              GWEN_Buffer_free(bufVarName);
              GWEN_Buffer_free(bufTagName);
	      return 0;
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

      bufData=GWEN_Buffer_new(0, 128, 0, 1);
      chr=GWEN_XML__ReadWordBuf(bio, chr, "<", bufData);
      if (chr<0) {
        GWEN_Buffer_free(bufData);
        return -1;
      }
      GWEN_Buffer_AppendByte(bufData, 0);

      newNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeData,
                               GWEN_Buffer_GetStart(bufData));
      DBG_DEBUG(0, "Added data \"%s\"",
                GWEN_Buffer_GetStart(bufData));
      GWEN_XMLNode_add(newNode, &(n->child));
      newNode->parent=n;

      GWEN_Buffer_free(bufData);
    }
    /* do not set chr=0, because we may already have the next char ('<') */
  } /* while !eof */

  if (currDepth!=0) {
    DBG_ERROR(0, "Line %d: %d tags are still open",
              GWEN_BufferedIO_GetLines(bio),
              currDepth);
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
	DBG_INFO(0, "open(%s): %s",
		 fullname,
		 strerror(errno));
	free(fullname);
        return -1;
      }
      else {
        GWEN_STRINGLISTENTRY *se;
        int gotcha;

        DBG_INFO(0, "open(%s): %s",
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
            DBG_ERROR(0, "No data in string list entry, internal error");
            return -1;
          }
          if (!*sp) {
            DBG_ERROR(0,
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
            DBG_INFO(0, "open(%s): %s",
                     fullname,
                     strerror(errno));
          }

          free(fullname);
          fullname=0;
          se=GWEN_StringListEntry_Next(se);
        } /* while */
        if (!gotcha) {
	  DBG_INFO(0, "Could not open file \"%s\".", file);
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
    DBG_INFO(0, "open(%s): %s",
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
      DBG_ERROR(0, "Error parsing");
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
    DBG_ERROR(0, "Unknown tag type (%d)", n->type);
  }
}


GWEN_XMLNODE *GWEN_XMLNode_FindNode(GWEN_XMLNODE *node,
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
    DBG_DEBUG(0, "Node %d:\"%s\" not found", t, data);
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



GWEN_XMLNODE *GWEN_XMLNode_GetFirstOfType(GWEN_XMLNODE *n,
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



GWEN_XMLNODE *GWEN_XMLNode_GetNextOfType(GWEN_XMLNODE *n,
                                         GWEN_XMLNODE_TYPE t){
  assert(n);
  while(n) {
    if (n->type==t)
      return n;
    n=n->next;
  } /* while */
  return 0;
}



GWEN_XMLNODE *GWEN_XMLNode_GetFirstTag(GWEN_XMLNODE *n){
  return GWEN_XMLNode_GetFirstOfType(n, GWEN_XMLNodeTypeTag);
}



GWEN_XMLNODE *GWEN_XMLNode_GetNextTag(GWEN_XMLNODE *n){
  if (!n->next)
    return 0;
  return GWEN_XMLNode_GetNextOfType(n->next, GWEN_XMLNodeTypeTag);
}



GWEN_XMLNODE *GWEN_XMLNode_GetFirstData(GWEN_XMLNODE *n){
  return GWEN_XMLNode_GetFirstOfType(n, GWEN_XMLNodeTypeData);
}



GWEN_XMLNODE *GWEN_XMLNode_GetNextData(GWEN_XMLNODE *n){
  if (!n->next)
    return 0;
  return GWEN_XMLNode_GetNextOfType(n->next, GWEN_XMLNodeTypeData);
}



GWEN_XMLNODE *GWEN_XMLNode_FindTag(GWEN_XMLNODE *n,
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
            return n;
          if (-1!=GWEN_Text_ComparePattern(pvalue, p, 0))
            return n;
        }
      } /* if pname */
      else
        return n;
    }
    n=GWEN_XMLNode_GetNextTag(n);
  } /* while */
  return 0;
}



GWEN_XMLNODE *GWEN_XMLNode_FindFirstTag(GWEN_XMLNODE *n,
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



GWEN_XMLNODE *GWEN_XMLNode_FindNextTag(GWEN_XMLNODE *n,
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
    DBG_ERROR(0, "Path too deep");
    return 1;
  }

  /* check for double entries */
  for (i=0; i<np->pos; i++) {
    assert(np->nodes[i]!=n);
  }
  np->nodes[np->pos++]=n;
  DBG_DEBUG(0, "Dived to %d", np->pos);
  return 0;
}



GWEN_XMLNODE *GWEN_XMLNode_Path_Surface(GWEN_XMLNODE_PATH *np){
  if (np->pos==0) {
    DBG_DEBUG(0, "Root reached");
    return 0;
  }
  DBG_DEBUG(0, "Surfaced to %d", np->pos-1);
  return np->nodes[--np->pos];
}



void GWEN_XMLNode_Path_Dump(GWEN_XMLNODE_PATH *np){
  unsigned int i;

  if (np->pos==0) {
    DBG_NOTICE(0, "Empty path");
  }
  for (i=0; i<np->pos; i++) {
    DBG_NOTICE(0, "Path entry %d:", i);
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

  if (flags & GWEN_XML_FLAGS_INDENT) {
    for(i=0; i<ind; i++) {
      err=GWEN_BufferedIO_WriteChar(bio, ' ');
      if (!GWEN_Error_IsOk(err)) {
        DBG_ERROR_ERR(0, err);
        return -1;
      }
    }
  }

  simpleTag=0;
  if (n->type==GWEN_XMLNodeTypeTag) {
    err=GWEN_BufferedIO_WriteChar(bio, '<');
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR_ERR(0, err);
      return -1;
    }
    if (n->data)
      err=GWEN_BufferedIO_Write(bio, n->data);
    else
      err=GWEN_BufferedIO_Write(bio, "UNKNOWN");
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR_ERR(0, err);
      return -1;
    }
    p=n->properties;
    if (p) {
      while (p) {
        if (!(flags & GWEN_XML_FLAGS_INDENT)) {
          err=GWEN_BufferedIO_WriteChar(bio, ' ');
          if (!GWEN_Error_IsOk(err)) {
            DBG_ERROR_ERR(0, err);
            return -1;
          }
        }
        else {
          err=GWEN_BufferedIO_WriteLine(bio, "");
          if (!GWEN_Error_IsOk(err)) {
            DBG_ERROR_ERR(0, err);
            return -1;
          }
          for(i=0; i<ind+3; i++) {
            err=GWEN_BufferedIO_WriteChar(bio, ' ');
            if (!GWEN_Error_IsOk(err)) {
              DBG_ERROR_ERR(0, err);
              return -1;
            }
          }
        }

        err=GWEN_BufferedIO_Write(bio, p->name);
        if (!GWEN_Error_IsOk(err)) {
          DBG_ERROR_ERR(0, err);
          return -1;
        }
  
        if (p->value) {
          err=GWEN_BufferedIO_Write(bio, "=\"");
          if (!GWEN_Error_IsOk(err)) {
            DBG_ERROR_ERR(0, err);
            return -1;
          }
          err=GWEN_BufferedIO_Write(bio, p->value);
          if (!GWEN_Error_IsOk(err)) {
            DBG_ERROR_ERR(0, err);
            return -1;
          }
          err=GWEN_BufferedIO_Write(bio, "\"");
          if (!GWEN_Error_IsOk(err)) {
            DBG_ERROR_ERR(0, err);
            return -1;
          }
        }
        p=p->next;
      } /* while */

      if (flags & GWEN_XML_FLAGS_INDENT) {
        err=GWEN_BufferedIO_WriteLine(bio, "");
        if (!GWEN_Error_IsOk(err)) {
          DBG_ERROR_ERR(0, err);
          return -1;
        }
        for(i=0; i<ind; i++) {
          err=GWEN_BufferedIO_WriteChar(bio, ' ');
          if (!GWEN_Error_IsOk(err)) {
            DBG_ERROR_ERR(0, err);
            return -1;
          }
        }
      }
    } /* if there are properties */

    if (n->data) {
      if (n->data[0]=='?') {
        simpleTag=1;
        err=GWEN_BufferedIO_WriteChar(bio, '?');
        if (!GWEN_Error_IsOk(err)) {
          DBG_ERROR_ERR(0, err);
          return -1;
        }
      }
      else if (n->data[0]=='!') {
        simpleTag=1;
      }
    }

    if (n->child==0) {
      err=GWEN_BufferedIO_WriteLine(bio, "/>");
      if (!GWEN_Error_IsOk(err)) {
        DBG_ERROR_ERR(0, err);
        return -1;
      }
    }
    else {
      err=GWEN_BufferedIO_WriteLine(bio, ">");
      if (!GWEN_Error_IsOk(err)) {
        DBG_ERROR_ERR(0, err);
        return -1;
      }
      if (!simpleTag) {
        c=n->child;
        while(c) {
          if (GWEN_XMLNode__WriteToStream(c, bio, flags, ind+2)) {
            DBG_INFO(0, "Error writing tag \"%s\"", n->data);
            return -1;
          }
          c=c->next;
        }
        if (flags & GWEN_XML_FLAGS_INDENT) {
          for(i=0; i<ind; i++) {
            err=GWEN_BufferedIO_WriteChar(bio, ' ');
            if (!GWEN_Error_IsOk(err)) {
              DBG_ERROR_ERR(0, err);
              return -1;
            }
          }
        }
        err=GWEN_BufferedIO_Write(bio, "</");
        if (!GWEN_Error_IsOk(err)) {
          DBG_ERROR_ERR(0, err);
          return -1;
        }
        if (n->data)
          err=GWEN_BufferedIO_Write(bio, n->data);
        else
          err=GWEN_BufferedIO_Write(bio, "UNKNOWN");
        if (!GWEN_Error_IsOk(err)) {
          DBG_ERROR_ERR(0, err);
          return -1;
        }
  
        err=GWEN_BufferedIO_WriteLine(bio, ">");
        if (!GWEN_Error_IsOk(err)) {
          DBG_ERROR_ERR(0, err);
          return -1;
        }
        if (flags & GWEN_XML_FLAGS_INDENT) {
          err=GWEN_BufferedIO_WriteLine(bio, "");
          if (!GWEN_Error_IsOk(err)) {
            DBG_ERROR_ERR(0, err);
            return -1;
          }
        }
      } /* if no simply tag */
    } /* if tag has children */
  } /* if tag */
  else if (n->type==GWEN_XMLNodeTypeData) {
    if (n->data) {
      err=GWEN_BufferedIO_WriteLine(bio, n->data);
      if (!GWEN_Error_IsOk(err)) {
        DBG_ERROR_ERR(0, err);
        return -1;
      }
    }
  }
  else if (n->type==GWEN_XMLNodeTypeComment) {
    err=GWEN_BufferedIO_Write(bio, "<!--");
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR_ERR(0, err);
      return -1;
    }

    if (n->data) {
      err=GWEN_BufferedIO_Write(bio, n->data);
      if (!GWEN_Error_IsOk(err)) {
        DBG_ERROR_ERR(0, err);
        return -1;
      }
    }
    err=GWEN_BufferedIO_WriteLine(bio, "-->");
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR_ERR(0, err);
      return -1;
    }
  }
  else {
    DBG_ERROR(0, "Unknown tag type (%d)", n->type);
  }

  return 0;
}



int GWEN_XMLNode_WriteToStream(const GWEN_XMLNODE *n,
                               GWEN_BUFFEREDIO *bio,
                               GWEN_TYPE_UINT32 flags){
  const GWEN_XMLNODE *nn;

  nn=n->child;
  while(nn) {
    if (GWEN_XMLNode__WriteToStream(nn, bio, flags, 0))
      return -1;

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
    DBG_ERROR(0, "Error opening file \"%s\": %s",
              fname,
              strerror(errno));
    return -1;
  }

  bio=GWEN_BufferedIO_File_new(fd);
  GWEN_BufferedIO_SetWriteBuffer(bio, 0, 1024);
  rv=GWEN_XMLNode_WriteToStream(n, bio, flags);
  err=GWEN_BufferedIO_Close(bio);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO(0, "called from here");
    GWEN_BufferedIO_free(bio);
    return -1;
  }
  GWEN_BufferedIO_free(bio);
  return rv;
}




