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
#include "gwenhyfwar/debug.h"
#include "gwenhyfwar/misc.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>



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



GWEN_XMLPROPERTY *GWEN_XMLProperty_dup(GWEN_XMLPROPERTY *p){
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


GWEN_XMLNODE *GWEN_XMLNode_dup(GWEN_XMLNODE *n){
  GWEN_XMLNODE *nn, *cn, *ncn;
  GWEN_XMLPROPERTY *p;

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


const char *GWEN_XMLNode_GetProperty(GWEN_XMLNODE *n, const char *name,
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


const char *GWEN_XMLNode_GetData(GWEN_XMLNODE *n){
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


GWEN_XMLNODE *GWEN_XMLNode_GetChild(GWEN_XMLNODE *n){
  assert(n);
  return n->child;
}


GWEN_XMLNODE *GWEN_XMLNode_GetParent(GWEN_XMLNODE *n){
  assert(n);
  return n->parent;
}


void GWEN_XMLNode_AddChild(GWEN_XMLNODE *n, GWEN_XMLNODE *child){
  assert(n);
  GWEN_XMLNode_add(child, &(n->child));
  child->parent=n;
}


int GWEN_XML__ReadWord(GWEN_BUFFEREDIO *bio,
                       char chr,
                       const char *delims,
                       char *buffer,
                       unsigned int size) {
  int inQuote;

  assert(size);
  inQuote=0;
  size--;
  buffer[0]=0;

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
	if (size<1) {
	  DBG_ERROR(0, "Word too long");
	  return -1;
	}
	*buffer=chr;
	buffer++;
	size--;
      }
    }
    else {
      if (chr=='"') {
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
	if (size<1) {
	  DBG_ERROR(0, "Word too long");
	  return -1;
	}
	*buffer=chr;
        buffer++;
	size--;
      }
    }
    chr=0;
  } /* while */
  *buffer=0;

  return chr;
}



int GWEN_XML_Parse(GWEN_XMLNODE *n, GWEN_BUFFEREDIO *bio,
                   unsigned int flags) {
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
      char tagname[GWEN_XML_MAX_TAGNAMELEN];
      char *p;

      isEndTag=0;

      /* we have a tag */
      chr=GWEN_XML__ReadWord(bio, 0, " ><", tagname, sizeof(tagname));
      if (chr<0)
	return -1;

      p=tagname;
      if (*p=='/') {
	isEndTag=1;
	p++;
      }
      DBG_DEBUG(0, "Found tag \"%s\"", tagname);

      isComment=0;
      if (strlen(p)>=3)
	if (strncmp(p,"!--",3)==0)
	  isComment=1;
      if (isComment) {
	char comment[GWEN_XML_MAX_REMARKLEN];
	int comlen;
        GWEN_XMLNODE *newNode;

	comlen=0;
	comment[0]=0;

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
	      return -1;
	    }
          }

          if (comlen>=sizeof(comment)) {
            DBG_ERROR(0, "Comment too long !");
            return -1;
          }
	  comment[comlen++]=chr;

	  if (comlen>=3) {
	    if (strncmp(comment+comlen-3,"-->",3)==0) {
	      comlen-=3;
	      comment[comlen]=0;
              break;
            }
            else {
              if (!(flags & GWEN_XML_FLAGS_READ_COMMENTS)) {
                DBG_VERBOUS(0, "Clipping comment to 2 bytes");
                memmove(comment, comment+comlen-2, 2);
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
                                   comment);
          GWEN_XMLNode_add(newNode, &(n->child));
          newNode->parent=n;
          DBG_DEBUG(0, "Added comment: \"%s\"", comment);
        }
        else {
          DBG_DEBUG(0, "Skip comment");
        }
      } /* if remark */
      else {
	/* handle tagname */
	if (isEndTag) {
	  /* handle endtag */
	  if (currDepth<1) {
	    DBG_ERROR(0, "More endtags than start tags !");
	    return -1;
	  }
	  if (strcasecmp(n->data, p)!=0) {
	    DBG_ERROR(0, "endtag \"%s\" does not match last start tag (\"%s\")",
		      tagname, n->data);
	    return -1;
	  }
	  /* surface */
	  n=path[currDepth-1];
	  currDepth--;
	  if (currDepth==0) {
	    DBG_DEBUG(0, "One node done");
	    return 0;
	  }
	}
	else {
	  /* this is a start tag */
	  GWEN_XMLNODE *newNode;
	  char varname[GWEN_XML_MAX_VARNAMELEN];
	  char value[GWEN_XML_MAX_VALUELEN];

	  if (*p) {
	    int j;

	    j=strlen(p)-1;
	    if (p[j]=='/') {
	      if (chr!='>') {
		DBG_ERROR(0, "\"/\" only allowed just before \">\"");
		return -1;
	      }
	      p[j]=0;
	      isEndTag=1;
	    }
	  }

	  newNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, p);
	  while (chr!='>' && chr!='/') {
	    varname[0]=0;
	    value[0]=0;

	    /* skip blanks */
	    chr=0;
	    while(!GWEN_BufferedIO_CheckEOF(bio)) {
	      chr=GWEN_BufferedIO_ReadChar(bio);
	      if (chr<0) {
		GWEN_XMLNode_free(newNode);
		return -1;
	      }
	      if (!isspace(chr) && !iscntrl(chr))
		break;
	    }
	    if (chr==0) {
	      DBG_ERROR(0, "unexpected EOF");
	      GWEN_XMLNode_free(newNode);
	      return -1;
	    }

	    if (chr=='>' || chr=='/')
	      break;

	    /* read possibly following var */
	    chr=GWEN_XML__ReadWord(bio, chr, " =/>", varname, sizeof(varname));
	    if (chr<0) {
	      GWEN_XMLNode_free(newNode);
	      return -1;
	    }

	    DBG_DEBUG(0, "Found property \"%s\"", varname);

	    /* skip blanks */
	    if (isspace(chr)) {
	      while(!GWEN_BufferedIO_CheckEOF(bio)) {
		chr=GWEN_BufferedIO_ReadChar(bio);
		if (chr<0) {
		  GWEN_XMLNode_free(newNode);
		  return -1;
		}
		if (!isspace(chr) && !iscntrl(chr))
		  break;
	      }
	    }

	    if (chr=='=') {
	      chr=0;
	      /* skip blanks */
	      while(!GWEN_BufferedIO_CheckEOF(bio)) {
		chr=GWEN_BufferedIO_ReadChar(bio);
		if (chr<0) {
		  GWEN_XMLNode_free(newNode);
		  return -1;
		}
		if (!isspace(chr))
		  break;
	      }
	      if (chr==0) {
		DBG_ERROR(0, "Value expected for property \"%s\"",
			  varname);
		GWEN_XMLNode_free(newNode);
		return -1;
	      }

	      if (chr=='>' || chr=='/') {
		DBG_ERROR(0, "Value expected for property \"%s\"",
			  varname);
		GWEN_XMLNode_free(newNode);
		return -1;
	      }

	      /* read value */
	      chr=GWEN_XML__ReadWord(bio, chr, " />", value, sizeof(value));
	      if (chr<0) {
		GWEN_XMLNode_free(newNode);
		return -1;
	      }
	      DBG_DEBUG(0, "Found value \"%s\"", value);
	    } /* if value follows */

	    if (varname[0]) {
	      /* add property */
	      GWEN_XMLPROPERTY *newProp;

	      newProp=GWEN_XMLProperty_new(varname,
                                           value[0]?value:0);
	      GWEN_XMLProperty_add(newProp, &(newNode->properties));
	    } /* if varname */
	  } /* while vars follow */

	  if (chr=='/') {
	    isEndTag=1;

	    if (GWEN_BufferedIO_CheckEOF(bio)) {
	      DBG_ERROR(0, "\">\" expected");
	      GWEN_XMLNode_free(newNode);
	      return -1;
	    }

	    chr=GWEN_BufferedIO_ReadChar(bio);
	    if (chr<0) {
	      DBG_ERROR(0, "Error on ReadChar");
	      GWEN_XMLNode_free(newNode);
	      return -1;
	    }
	  }

	  if (chr!='>') {
	    DBG_ERROR(0, "\">\" expected");
	    GWEN_XMLNode_free(newNode);
	    return -1;
	  }

	  /* ok, now the tag is complete, add it */
	  if (currDepth>=GWEN_XML_MAX_DEPTH) {
	    DBG_ERROR(0, "Maximum depth exceeded");
	    GWEN_XMLNode_free(newNode);
	    return -1;
	  }
	  GWEN_XMLNode_add(newNode, &(n->child));
          newNode->parent=n;
	  DBG_DEBUG(0, "Added node \"%s\"", newNode->data);
	  if (!isEndTag) {
	    /* only dive if this tag is not immediately ended */
	    path[currDepth++]=n;
	    n=newNode;
	  }
	  else {
	    /* immediate endTag, if depth is 0: done */
	    if (currDepth==0) {
	      DBG_DEBUG(0, "One node done");
	      return 0;
	    }
	  }
	} /* if start tag */
      } /* if no remark */

      chr=0; /* make begin of loop read new char */
    } /* if "<" */
    else {
      /* add data to current tag */
      GWEN_XMLNODE *newNode;
      char data[GWEN_XML_MAX_DATALEN];

      chr=GWEN_XML__ReadWord(bio, chr, "<", data, sizeof(data));
      if (chr<0) {
	GWEN_XMLNode_free(newNode);
	return -1;
      }
      newNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeData, data);
      DBG_DEBUG(0, "Added data \"%s\"", data);
      GWEN_XMLNode_add(newNode, &(n->child));
      newNode->parent=n;
    }
    /* do not set chr=0, because we may already have the next char ('<') */
  } /* while !eof */

  if (currDepth!=0) {
    DBG_ERROR(0, "%d tags are still open", currDepth);
    return -1;
  }

  return 0;
}


int GWEN_XML_ReadFile(GWEN_XMLNODE *n, const char *filepath,
                      unsigned int flags){
  GWEN_BUFFEREDIO *dm;
  int fd;

  fd=open(filepath, O_RDONLY);
  if (fd==-1) {
    DBG_ERROR(0, "open(%s): %s",
	      filepath,
	      strerror(errno));
    return -1;
  }

  dm=GWEN_BufferedIO_File_new(fd);
  GWEN_BufferedIO_SetReadBuffer(dm,0,1024);

  while(!GWEN_BufferedIO_CheckEOF(dm)) {
    if (GWEN_XML_Parse(n, dm, flags)) {
      DBG_ERROR(0, "Error parsing");
      GWEN_BufferedIO_Close(dm);
      GWEN_BufferedIO_free(dm);
      return -1;
    }
  } /* while */

  GWEN_BufferedIO_Close(dm);
  GWEN_BufferedIO_free(dm);

  return 0;
}


GWEN_XMLNODE_TYPE GWEN_XMLNode_GetType(GWEN_XMLNODE *n){
  assert(n);
  return n->type;
}


GWEN_XMLNODE *GWEN_XMLNode_Next(GWEN_XMLNODE *n) {
  assert(n);
  return n->next;
}


void GWEN_XMLNode_Dump(GWEN_XMLNODE *n, FILE *f, int ind) {
  GWEN_XMLPROPERTY *p;
  GWEN_XMLNODE *c;
  int i;

  assert(n);

  for(i=0; i<ind; i++)
    fprintf(f, " ");

  if (n->type==GWEN_XMLNodeTypeTag) {
    if (n->data)
      fprintf(f, "<%s", n->data);
    else
      fprintf(f, "<UNKNOWN");
    p=n->properties;
    while (p) {
      fprintf(f, " %s=\"%s\"", p->name, p->value);
      p=p->next;
    }
    fprintf(f, ">\n");
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
                                 GWEN_XMLNODE *sn,
                                 int overwrite){
  GWEN_XMLPROPERTY *sp;
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



GWEN_XMLNODE_PATH *GWEN_XMLNode_Path_new(){
  GWEN_XMLNODE_PATH *p;

  GWEN_NEW_OBJECT(GWEN_XMLNODE_PATH, p);
  return p;
}



GWEN_XMLNODE_PATH *GWEN_XMLNode_Path_dup(GWEN_XMLNODE_PATH *np){
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

  DBG_DEBUG(0, "Diving to %08x", (unsigned int)n);
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




