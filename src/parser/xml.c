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

#include "chameleon/xml_p.h"
#include "chameleon/chameleon.h"
#include "chameleon/debug.h"
#include "chameleon/misc.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>


XMLPROPERTY *XMLProperty_new(const char *name, const char *value){
  XMLPROPERTY *p;

  p=(XMLPROPERTY *)malloc(sizeof(XMLPROPERTY));
  assert(p);
  memset(p,0,sizeof(XMLPROPERTY));
  if (name)
    p->name=strdup(name);
  if (value)
    p->value=strdup(value);
  return p;
}


void XMLProperty_free(XMLPROPERTY *p){
  if (p) {
    free(p->name);
    free(p->value);
    free(p);
  }
}


XMLPROPERTY *XMLProperty_dup(XMLPROPERTY *p){
  return XMLProperty_new(p->name, p->value);
}


void XMLProperty_add(XMLPROPERTY *p, XMLPROPERTY **head){
  GWENHYFWAR_LIST_ADD(XMLPROPERTY, p, head);
}


void XMLProperty_del(XMLPROPERTY *p, XMLPROPERTY **head){
  GWENHYFWAR_LIST_DEL(XMLPROPERTY, p, head);
}


void XMLProperty_freeAll(XMLPROPERTY *p) {
  while(p) {
    XMLPROPERTY *next;

    next=p->next;
    XMLProperty_free(p);
    p=next;
  } /* while */
}




XMLNODE *XMLNode_new(XMLNODE_TYPE t, const char *data){
  XMLNODE *n;

  n=(XMLNODE *)malloc(sizeof(XMLNODE));
  assert(n);
  memset(n,0,sizeof(XMLNODE));
  n->type=t;
  if (data)
    n->data=strdup(data);
  return n;
}


void XMLNode_free(XMLNODE *n){
  if (n) {
    XMLProperty_freeAll(n->properties);
    free(n->data);
    XMLNode_freeAll(n->child);
    free(n);
  }
}


void XMLNode_freeAll(XMLNODE *n){
  while(n) {
    XMLNODE *next;

    next=n->next;
    XMLNode_free(n);
    n=next;
  } /* while */
}


XMLNODE *XMLNode_dup(XMLNODE *n){
  XMLNODE *nn, *cn, *ncn;
  XMLPROPERTY *p;

  /* duplicate node itself */
  nn=XMLNode_new(n->type, n->data);

  /* duplicate properties */
  p=n->properties;
  while(p) {
    XMLPROPERTY *np;

    np=XMLProperty_dup(p);
    XMLProperty_add(np, &(nn->properties));
    p=p->next;
  } /* while */

  /* duplicate children */
  cn=n->child;
  while(cn) {
    ncn=XMLNode_dup(cn);
    XMLNode_add(ncn, &(nn->child));
    ncn->parent=nn;
    cn=cn->next;
  } /* while */

  return nn;
}


void XMLNode_add(XMLNODE *n, XMLNODE **head){
  GWENHYFWAR_LIST_ADD(XMLNODE, n, head);
}


void XMLNode_del(XMLNODE *n, XMLNODE **head){
  GWENHYFWAR_LIST_DEL(XMLNODE, n, head);
  n->parent=0;
}


const char *XMLNode_GetProperty(XMLNODE *n, const char *name,
				const char *defaultValue){
  XMLPROPERTY *p;

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


void XMLNode_SetProperty(XMLNODE *n, const char *name, const char *value){
  XMLPROPERTY *p;

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
    p=XMLProperty_new(name, value);
    XMLProperty_add(p, &(n->properties));
  }
}


const char *XMLNode_GetData(XMLNODE *n){
  assert(n);
  return n->data;
}


void XMLNode_SetData(XMLNODE *n, const char *data){
  assert(n);
  free(n->data);
  if (data)
    n->data=strdup(data);
  else
    n->data=0;
}


XMLNODE *XMLNode_GetChild(XMLNODE *n){
  assert(n);
  return n->child;
}


XMLNODE *XMLNode_GetParent(XMLNODE *n){
  assert(n);
  return n->parent;
}


void XMLNode_AddChild(XMLNODE *n, XMLNODE *child){
  assert(n);
  XMLNode_add(child, &(n->child));
  child->parent=n;
}


int XML__ReadWord(BUFFEREDIOTABLE *bio,
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
      if (BufferedIO_CheckEOF(bio))
	break;
      chr=BufferedIO_ReadChar(bio);
      if (chr<0) {
	DBG_ERROR("Error on ReadChar");
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
	  DBG_ERROR("Word too long");
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
	DBG_ERROR("No tags inside a tag definition allowed");
        return -1;
      }
      else {
	if (size<1) {
	  DBG_ERROR("Word too long");
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



int XML_Parse(XMLNODE *n, BUFFEREDIOTABLE *bio) {
  XMLNODE *path[XML_MAX_DEPTH];
  int currDepth;
  int chr;
  int isEndTag;
  int eofMet;
  int isComment;

  currDepth=0;
  chr=0;

  while (!BufferedIO_CheckEOF(bio)) {
    /* read char (if none set) but skip blanks */
    if (chr==0) {
      chr=BufferedIO_ReadChar(bio);
      if (chr<0) {
	DBG_ERROR("Error on BufferedIO_ReadChar");
	return -1;
      }
    }
    eofMet=0;
    while(isspace(chr)) {
      if (BufferedIO_CheckEOF(bio)) {
	eofMet=1;
        break;
      }
      chr=BufferedIO_ReadChar(bio);
      if (chr<0) {
	DBG_ERROR("Error on BufferedIO_ReadChar");
	return -1;
      }
    }
    if (eofMet)
      break;

    if (chr=='<') {
      char tagname[XML_MAX_TAGNAMELEN];
      char *p;

      isEndTag=0;

      /* we have a tag */
      chr=XML__ReadWord(bio, 0, " ><", tagname, sizeof(tagname));
      if (chr<0)
	return -1;

      p=tagname;
      if (*p=='/') {
	isEndTag=1;
	p++;
      }
      DBG_DEBUG("Found tag \"%s\"", tagname);

      isComment=0;
      if (strlen(p)>=3)
	if (strncmp(p,"!--",3)==0)
	  isComment=1;
      if (isComment) {
	char comment[XML_MAX_REMARKLEN];
	int comlen;
        XMLNODE *newNode;

	comlen=0;
	comment[0]=0;

        DBG_DEBUG("Found comment");
	/* we have a remark tag, so read it over */
	while(1) {
	  if (chr==0) {
	    if (BufferedIO_CheckEOF(bio)) {
	      DBG_ERROR("Unexpected EOF within comment");
	      return -1;
	    }
	    chr=BufferedIO_ReadChar(bio);
	    if (chr<0) {
	      return -1;
	    }
	  }
	  comment[comlen++]=chr;

	  if (comlen>=3) {
	    if (strncmp(comment+comlen-3,"-->",3)==0) {
	      comlen-=3;
	      comment[comlen]=0;
	      break;
	    }
	  }
	  chr=0;
	} /* while */

	/* create new node */
	newNode=XMLNode_new(XMLNodeTypeComment,
			    comment);
	XMLNode_add(newNode, &(n->child));
	newNode->parent=n;
	DBG_DEBUG("Added comment: \"%s\"", comment);
      } /* if remark */
      else {
	/* handle tagname */
	if (isEndTag) {
	  /* handle endtag */
	  if (currDepth<1) {
	    DBG_ERROR("More endtags than start tags !");
	    return -1;
	  }
	  if (strcasecmp(n->data, p)!=0) {
	    DBG_ERROR("endtag \"%s\" does not match last start tag (\"%s\")",
		      tagname, n->data);
	    return -1;
	  }
	  /* surface */
	  n=path[currDepth-1];
	  currDepth--;
	  if (currDepth==0) {
	    DBG_DEBUG("One node done");
	    return 0;
	  }
	}
	else {
	  /* this is a start tag */
	  XMLNODE *newNode;
	  char varname[XML_MAX_VARNAMELEN];
	  char value[XML_MAX_VALUELEN];

	  if (*p) {
	    int j;

	    j=strlen(p)-1;
	    if (p[j]=='/') {
	      if (chr!='>') {
		DBG_ERROR("\"/\" only allowed just before \">\"");
		return -1;
	      }
	      p[j]=0;
	      isEndTag=1;
	    }
	  }

	  newNode=XMLNode_new(XMLNodeTypeTag, p);
	  while (chr!='>' && chr!='/') {
	    varname[0]=0;
	    value[0]=0;

	    /* skip blanks */
	    chr=0;
	    while(!BufferedIO_CheckEOF(bio)) {
	      chr=BufferedIO_ReadChar(bio);
	      if (chr<0) {
		XMLNode_free(newNode);
		return -1;
	      }
	      if (!isspace(chr) && !iscntrl(chr))
		break;
	    }
	    if (chr==0) {
	      DBG_ERROR("unexpected EOF");
	      XMLNode_free(newNode);
	      return -1;
	    }

	    if (chr=='>' || chr=='/')
	      break;

	    /* read possibly following var */
	    chr=XML__ReadWord(bio, chr, " =/>", varname, sizeof(varname));
	    if (chr<0) {
	      XMLNode_free(newNode);
	      return -1;
	    }

	    DBG_DEBUG("Found property \"%s\"", varname);

	    /* skip blanks */
	    if (isspace(chr)) {
	      while(!BufferedIO_CheckEOF(bio)) {
		chr=BufferedIO_ReadChar(bio);
		if (chr<0) {
		  XMLNode_free(newNode);
		  return -1;
		}
		if (!isspace(chr) && !iscntrl(chr))
		  break;
	      }
	    }

	    if (chr=='=') {
	      chr=0;
	      /* skip blanks */
	      while(!BufferedIO_CheckEOF(bio)) {
		chr=BufferedIO_ReadChar(bio);
		if (chr<0) {
		  XMLNode_free(newNode);
		  return -1;
		}
		if (!isspace(chr))
		  break;
	      }
	      if (chr==0) {
		DBG_ERROR("Value expected for property \"%s\"",
			  varname);
		XMLNode_free(newNode);
		return -1;
	      }

	      if (chr=='>' || chr=='/') {
		DBG_ERROR("Value expected for property \"%s\"",
			  varname);
		XMLNode_free(newNode);
		return -1;
	      }

	      /* read value */
	      chr=XML__ReadWord(bio, chr, " />", value, sizeof(value));
	      if (chr<0) {
		XMLNode_free(newNode);
		return -1;
	      }
	      DBG_DEBUG("Found value \"%s\"", value);
	    } /* if value follows */

	    if (varname[0]) {
	      /* add property */
	      XMLPROPERTY *newProp;

	      newProp=XMLProperty_new(varname,
				      value[0]?value:0);
	      XMLProperty_add(newProp, &(newNode->properties));
	    } /* if varname */
	  } /* while vars follow */

	  if (chr=='/') {
	    isEndTag=1;

	    if (BufferedIO_CheckEOF(bio)) {
	      DBG_ERROR("\">\" expected");
	      XMLNode_free(newNode);
	      return -1;
	    }

	    chr=BufferedIO_ReadChar(bio);
	    if (chr<0) {
	      DBG_ERROR("Error on ReadChar");
	      XMLNode_free(newNode);
	      return -1;
	    }
	  }

	  if (chr!='>') {
	    DBG_ERROR("\">\" expected");
	    XMLNode_free(newNode);
	    return -1;
	  }

	  /* ok, now the tag is complete, add it */
	  if (currDepth>=XML_MAX_DEPTH) {
	    DBG_ERROR("Maximum depth exceeded");
	    XMLNode_free(newNode);
	    return -1;
	  }
	  XMLNode_add(newNode, &(n->child));
          newNode->parent=n;
	  DBG_DEBUG("Added node \"%s\"", newNode->data);
	  if (!isEndTag) {
	    /* only dive if this tag is not immediately ended */
	    path[currDepth++]=n;
	    n=newNode;
	  }
	  else {
	    /* immediate endTag, if depth is 0: done */
	    if (currDepth==0) {
	      DBG_DEBUG("One node done");
	      return 0;
	    }
	  }
	} /* if start tag */
      } /* if no remark */

      chr=0; /* make begin of loop read new char */
    } /* if "<" */
    else {
      /* add data to current tag */
      XMLNODE *newNode;
      char data[XML_MAX_DATALEN];

      chr=XML__ReadWord(bio, chr, "<", data, sizeof(data));
      if (chr<0) {
	XMLNode_free(newNode);
	return -1;
      }
      newNode=XMLNode_new(XMLNodeTypeData, data);
      DBG_DEBUG("Added data \"%s\"", data);
      XMLNode_add(newNode, &(n->child));
      newNode->parent=n;
    }
    /* do not set chr=0, because we may already have the next char ('<') */
  } /* while !eof */

  if (currDepth!=0) {
    DBG_ERROR("%d tags are still open", currDepth);
    return -1;
  }

  return 0;
}


int XML_ReadFile(XMLNODE *n, const char *filepath){
  BUFFEREDIOTABLE *dm;
  int fd;

  fd=open(filepath, O_RDONLY);
  if (fd==-1) {
    DBG_ERROR("open(%s): %s",
	      filepath,
	      strerror(errno));
    return -1;
  }

  dm=BufferedIO_File_new(fd);
  BufferedIO_SetReadBuffer(dm,0,1024);

  while(!BufferedIO_CheckEOF(dm)) {
    if (XML_Parse(n, dm)) {
      DBG_ERROR("Error parsing");
      BufferedIO_Close(dm);
      BufferedIO_free(dm);
      return -1;
    }
  } /* while */

  BufferedIO_Close(dm);
  BufferedIO_free(dm);

  return 0;
}


XMLNODE_TYPE XMLNode_GetType(XMLNODE *n){
  assert(n);
  return n->type;
}


XMLNODE *XMLNode_Next(XMLNODE *n) {
  assert(n);
  return n->next;
}


void XMLNode_Dump(XMLNODE *n, FILE *f, int ind) {
  XMLPROPERTY *p;
  XMLNODE *c;
  int i;

  assert(n);

  for(i=0; i<ind; i++)
    fprintf(f, " ");

  if (n->type==XMLNodeTypeTag) {
    if (n->data)
      fprintf(f, "<%s", n->data);
    else
      fprintf(f, "<UNKNOWN");
    p=n->properties;
    while (p) {
      fprintf(f, " %s=%s", p->name, p->value);
      p=p->next;
    }
    fprintf(f, ">\n");
    c=n->child;
    while(c) {
      XMLNode_Dump(c, f, ind+2);
      c=c->next;
    }
    for(i=0; i<ind; i++)
      fprintf(f, " ");
    if (n->data)
      fprintf(f, "</%s>\n", n->data);
    else
      fprintf(f, "</UNKNOWN>\n");
  }
  else if (n->type==XMLNodeTypeData) {
    if (n->data) {
      fprintf(f, "%s\n", n->data);
    }
  }
  else if (n->type==XMLNodeTypeComment) {
    fprintf(f, "<!--");
    if (n->data) {
      fprintf(f, "%s", n->data);
    }
    fprintf(f, "-->\n");
  }
  else {
    DBG_ERROR("Unknown tag type (%d)", n->type);
  }
}


XMLNODE *XMLNode_FindNode(XMLNODE *node,
			  XMLNODE_TYPE t, const char *data) {
  XMLNODE *n;

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
    DBG_DEBUG("Node %d:\"%s\" not found", t, data);
    return 0;
  }

  return n;
}


void XMLNode_UnlinkChild(XMLNODE *n, XMLNODE *child){
  assert(n);
  assert(child);
  XMLNode_del(child, &(n->child));
  child->next=0;
  child->parent=0;
}




