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


#include "text.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <gwenhyfwar/gwenhyfwarapi.h>
#include <gwenhyfwar/debug.h>

char *GWEN_Text_GetWord(const char *src,
                        const char *delims,
                        char *buffer,
                        unsigned int maxsize,
                        unsigned int flags,
                        const char **next){
  unsigned int size;
  int lastWasBlank;
  int lastBlankPos;
  int insideQuotes;
  int lastWasEscape;

  assert(maxsize);

  /* skip leading blanks, if wanted */
  if (flags & GWEN_TEXT_FLAGS_DEL_LEADING_BLANKS) {
    while(*src && *src<33)
      src++;
  }

  /* get word */
  size=0;
  lastWasBlank=0;
  lastBlankPos=-1;
  lastWasEscape=0;
  insideQuotes=0;

  if (*src=='"') {
    insideQuotes=1;
    if (flags & GWEN_TEXT_FLAGS_DEL_QUOTES)
      src++;
  }

  while(*src && size<(maxsize-1)) {
    if (lastWasEscape) {
      buffer[size]=*src;
      size++;
      lastWasEscape=0;
      lastWasBlank=0;
      lastBlankPos=-1;
    }
    else {
      if (*src=='\\' && (flags & GWEN_TEXT_FLAGS_CHECK_BACKSLASH)) {
        lastWasEscape=1;
        lastWasBlank=0;
        lastBlankPos=-1;
      }
      else {
        if (!insideQuotes && strchr(delims, *src)!=0)
          break;
        if (*src=='"') {
          if (insideQuotes) {
            insideQuotes=0;
            src++;
            break;
          }
          else {
            DBG_DEBUG(0,
                      "Found a closing \" without an opening one "
                      "(consider using a backslash to escape)");
            return 0;
          }
        }


        if (insideQuotes ||
            !lastWasBlank ||
            (lastWasBlank &&
             !(flags & GWEN_TEXT_FLAGS_DEL_MULTIPLE_BLANKS))) {
          /* only copy if last char was NOT blank or
           * last was blank but the caller does not want to have multiple
           * blanks removed */
          buffer[size]=*src;
          size++;
        }
        /* remember next loop whether this char was a blank */
        if (isspace(*src) && !lastWasEscape) {
          lastWasBlank=1;
          lastBlankPos=size;
        }
        else {
          lastWasBlank=0;
          lastBlankPos=-1;
        }
      } /* if this is not a backslash */
    } /* !lastWasEscape */
      /* advance source pointer */
      src++;
    } /* while */

  /* add trailing null to correctly terminate the buffer */
  buffer[size]=0;

  if (insideQuotes) {
    DBG_DEBUG(0, "Missing \" after word");
    return 0;
  }
  /* check whether the source string was correctly terminated */
  if (flags & GWEN_TEXT_FLAGS_NEED_DELIMITER) {
    if (*src) {
      if (strchr(delims, *src)==0) {
        DBG_ERROR(0, "No delimiter found within specified length");
	return 0;
      }
    }
    else {
      if (!(flags & GWEN_TEXT_FLAGS_NULL_IS_DELIMITER)) {
	DBG_ERROR(0, "String ends without delimiter");
	return 0;
      }
    }
  }

  /* remove trailing blanks, if wanted */
  if (flags & GWEN_TEXT_FLAGS_DEL_TRAILING_BLANKS) {
    if (lastBlankPos!=-1)
      buffer[lastBlankPos]=0;
  }

  *next=src;
  return buffer;
}



char *GWEN_Text_Escape(const char *src,
                       char *buffer,
                       unsigned int maxsize) {
  unsigned int size;

  size=0;
  while(*src) {
    unsigned char x;

    x=(unsigned char)*src;
    if (!(
	  (x>='A' && x<='Z') ||
	  (x>='a' && x<='z') ||
	  (x>='0' && x<='9'))) {
      unsigned char c;

      if ((maxsize-1)<size+3) {
	DBG_ERROR(0, "Buffer too small");
	return 0;
      }
      buffer[size++]='%';
      c=(((unsigned char)(*src))>>4)&0xf;
      if (c>9)
	c+=7;
      c+='0';
      buffer[size++]=c;
      c=((unsigned char)(*src))&0xf;
      if (c>9)
	c+=7;
      c+='0';
      buffer[size++]=c;
    }
    else {
      if (size<(maxsize-1))
	buffer[size++]=*src;
      else {
	DBG_ERROR(0, "Buffer too small");
	return 0;
      }
    }

    src++;
  } /* while */

  buffer[size]=0;
  return buffer;
}


char *GWEN_Text_Unescape(const char *src,
                         char *buffer,
                         unsigned int maxsize){
  unsigned int size;

  size=0;

  while(*src) {
    unsigned char x;

    x=(unsigned char)*src;
    if (
	(x>='A' && x<='Z') ||
	(x>='a' && x<='z') ||
	(x>='0' && x<='9')) {
      if (size<(maxsize-1))
	buffer[size++]=*src;
      else {
	DBG_ERROR(0, "Buffer too small");
	return 0;
      }
    }
    else {
      if (*src=='%') {
	unsigned char d1, d2;
	unsigned char c;

	/* skip '%' */
	src++;
	if (!(*src) || !isxdigit(*src)) {
	  DBG_ERROR(0, "Incomplete escape sequence (no digits)");
	  return 0;
	}
	/* read first digit */
	d1=(unsigned char)(toupper(*src));

	/* get second digit */
	src++;
	if (!(*src) || !isxdigit(*src)) {
	  DBG_ERROR(0, "Incomplete escape sequence (only 1 digit)");
	  return 0;
	}
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
	if (size<(maxsize-1))
	  buffer[size++]=(char)c;
	else {
	  DBG_ERROR(0, "Buffer too small");
	  return 0;
	}
      }
      else {
	DBG_ERROR(0, "Found non-alphanum "
		  "characters in escaped string (\"%s\")",
		  src);
        return 0;
      }
    }
    src++;
  } /* while */

  buffer[size]=0;
  return buffer;
}


char *GWEN_Text_ToHex(const char *src, int l,
                      char *buffer, unsigned maxsize) {
  unsigned int pos;
  unsigned int size;

  if ((l*2)+1 > maxsize) {
    DBG_ERROR(0, "Buffer too small");
    return 0;
  }

  pos=0;
  size=0;
  while(pos<l) {
    unsigned char c;

    c=(((unsigned char)(src[pos]))>>4)&0xf;
    if (c>9)
      c+=7;
    c+='0';
    buffer[size++]=c;
    c=((unsigned char)(src[pos]))&0xf;
    if (c>9)
      c+=7;
    c+='0';
    buffer[size++]=c;
    pos++;
  }
  buffer[size]=0;
  return buffer;
}



char *GWEN_Text_ToHexGrouped(const char *src,
                             int l,
                             char *buffer,
                             unsigned maxsize,
                             unsigned int groupsize,
                             char delimiter,
                             int skipLeadingZeroes) {
  unsigned int pos;
  unsigned int size;
  unsigned int j;

  j=0;

  pos=0;
  size=0;
  j=0;
  while(pos<l) {
    unsigned char c;
    int skipThis;

    skipThis=0;
    c=(((unsigned char)(src[pos]))>>4)&0xf;
    if (skipLeadingZeroes) {
      if (c==0)
	skipThis=1;
      else
	skipLeadingZeroes=0;
    }
    if (c>9)
      c+=7;
    c+='0';
    if (!skipThis) {
      if (size+1>=maxsize) {
	DBG_ERROR(0, "Buffer too small");
        return 0;
      }
      buffer[size++]=c;
      j++;
      if (j==groupsize) {
	if (size+1>=maxsize) {
	  DBG_ERROR(0, "Buffer too small");
	  return 0;
	}
	buffer[size++]=delimiter;
	j=0;
      }
    }

    skipThis=0;
    c=((unsigned char)(src[pos]))&0xf;
    if (skipLeadingZeroes) {
      if (c==0 && pos+1<l)
	skipThis=1;
      else
	skipLeadingZeroes=0;
    }
    if (c>9)
      c+=7;
    c+='0';
    if (size+1>=maxsize) {
      DBG_ERROR(0, "Buffer too small");
      return 0;
    }
    if (!skipThis) {
      buffer[size++]=c;
      j++;
      if (j==groupsize) {
	if (pos+1<l) {
	  if (size+1>=maxsize) {
	    DBG_ERROR(0, "Buffer too small");
	    return 0;
	  }
	  buffer[size++]=delimiter;
	}
	j=0;
      }
    }
    pos++;
  }
  buffer[size]=0;
  return buffer;
}



int GWEN_Text_FromHex(const char *src, char *buffer, unsigned maxsize){
  unsigned int pos;
  unsigned int size;

  pos=0;
  size=0;
  while(*src) {
    unsigned char d1, d2;
    unsigned char c;

    /* read first digit */
    if (!isxdigit(*src)) {
      DBG_ERROR(0, "Bad char in hex string");
      return -1;
    }
    d1=(unsigned char)(toupper(*src));

    /* get second digit */
    src++;
    if (!(*src) || !isxdigit(*src)) {
      DBG_ERROR(0, "Incomplete hex byte (only 1 digit)");
      return -1;
    }
    d2=(unsigned char)(toupper(*src));
    src++;

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
    if (size<(maxsize))
      buffer[size++]=(char)c;
    else {
      DBG_ERROR(0, "Buffer too small");
      return -1;
    }
  } /* while */

  return size;
}



int GWEN_Text_Compare(const char *s1, const char *s2, int ign) {
  if (s1)
    if (strlen(s1)==0)
      s1=0;
  if (s2)
    if (strlen(s2)==0)
      s2=0;
  if (!s1 && !s2)
    return 0;
  if (!s1 && s2)
    return 1;
  if (s1 && !s2)
    return -1;
  if (ign)
    return strcasecmp(s1, s2);
  else
    return strcmp(s1, s2);
}




int GWEN_Text__cmpSegment(const char *w, unsigned int *wpos,
                          const char *p, unsigned int *ppos,
                          int sensecase,
                          unsigned int *matches) {
  char a;
  char b;
  unsigned wlength;
  unsigned plength;

  a=0;
  b=0;
  wlength=strlen(w);
  plength=strlen(p);


  while (*wpos<wlength && *ppos<plength) {
    a=w[*wpos];
    b=p[*ppos];
    if (b=='*')
      return 1;
    if (!sensecase) {
      a=toupper(a);
      b=toupper(b);
    }
    // count matches
    if (a==b)
      (*matches)++;
    if (a!=b && b!='?')
      return 0;
    (*wpos)++;
    (*ppos)++;
  }
  // both at end, would be ok
  if (*wpos==wlength && *ppos==plength)
    return 1;
  // word ends, pattern doesnt, would be ok if pattern is '*' here
  if (*wpos>=wlength && *ppos<plength)
    if (p[*ppos]=='*')
      return 1;
  // otherwise no match ;-/
  return 0;
}



int GWEN_Text__findSegment(const char *w, unsigned int *wpos,
                           const char *p, unsigned int *ppos,
                           int sensecase,
                           unsigned int *matches) {
  unsigned int lwpos, lppos, lmatches;
  unsigned wlength;

  wlength=strlen(w);
  lwpos=*wpos;
  lppos=*ppos;
  lmatches=*matches;
  while(lwpos<wlength) {
    *ppos=lppos;
    *wpos=lwpos;
    *matches=lmatches;
    if (GWEN_Text__cmpSegment(w,wpos,p,ppos,sensecase,matches))
      return 1;
    lwpos++;
  }
  return 0;
}


int GWEN_Text_ComparePattern(const char *w, const char *p, int sensecase) {
  unsigned int ppos;
  unsigned int wpos;
  unsigned int matches;
  unsigned int plength;

  ppos=wpos=matches=0;
  plength=strlen(p);

  // compare until first occurrence of '*'
  if (!GWEN_Text__cmpSegment(w,&wpos,p,&ppos,sensecase,&matches))
    return -1;

  while(1) {
    // if pattern ends we have done it
    if (ppos>=plength)
      return matches;
    // skip '*' in pattern
    ppos++;
    // if pattern ends behind '*' the word matches
    if (ppos>=plength)
      return matches;
    // find next matching segment
    if (!GWEN_Text__findSegment(w,&wpos,p,&ppos,sensecase,&matches))
      return -1;
  } // while
  // I know, we'll never get to here ;-)
  return -1;
}



int GWEN_Text_NumToString(int num, char *buffer, unsigned int bufsize,
                          int fillchar){
  char lbuffer[128];
  int i;

  sprintf(lbuffer,"%d", num);
  i=strlen(lbuffer);
  if (i>=bufsize) {
    DBG_ERROR(0, "Buffer too small (%d>=%d)", i, bufsize);
    return -1;
  }
  if (fillchar>0) {
    /* fill right, but first copy chars */
    strcpy(buffer, lbuffer);
    while(i<bufsize-1)
      buffer[i++]=fillchar;
    buffer[i]=0;
    return bufsize;
  }
  else if (fillchar<0) {
    int j, k;

    fillchar=-fillchar;
    j=bufsize-1-i;
    for (k=0; k<j; k++)
      buffer[k]=fillchar;
    buffer[k]=0;
    strcat(buffer, lbuffer);
    return bufsize;
  }
  else {
    /* dont fill, just copy */
    strcpy(buffer, lbuffer);
    return i;
  }
}



void GWEN_Text_DumpString(const char *s, int l, FILE *f, int insert) {
  unsigned int i;
  unsigned int j;
  unsigned int pos;
  unsigned k;

  pos=0;
  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f,"String size is %d:\n",l);
  while(pos<l) {
    for (k=0; k<insert; k++)
      fprintf(f, " ");
    fprintf(f,"%04x: ",pos);
    j=pos+16;
    if (j>=l)
      j=l;

    // show hex dump
    for (i=pos; i<j; i++) {
      fprintf(f,"%02x ",(unsigned char)s[i]);
    }
    if (j-pos<16)
      for (i=0; i<16-(j-pos); i++)
	fprintf(f,"   ");
    // show text
    for (i=pos; i<j; i++) {
      if (s[i]<32)
	fprintf(f,".");
      else
	fprintf(f,"%c",s[i]);
    }
    fprintf(f,"\n");
    pos+=16;
  }
}





