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
#ifdef HAVE_LOCALE_H
# include <locale.h>
#endif
#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/stringlist.h>



typedef struct {
  int character;
  const char *replace;
} GWEN_TEXT_ESCAPE_ENTRY;

static const GWEN_TEXT_ESCAPE_ENTRY gwen_text__xml_escape_chars[]= {
{'&', "&amp;"},
{'<', "&lt;"},
{'>', "&gt;"},
{'\'', "&apos;"},
{'\"', "&quot;"},
{0, 0}
};



char *GWEN_Text_GetWord(const char *src,
                        const char *delims,
                        char *buffer,
                        unsigned int maxsize,
                        GWEN_TYPE_UINT32 flags,
                        const char **next){
  unsigned int size;
  int lastWasBlank;
  int lastBlankPos;
  int insideQuotes;
  int lastWasEscape;

  assert(maxsize);

  /* skip leading blanks, if wanted */
  if (flags & GWEN_TEXT_FLAGS_DEL_LEADING_BLANKS) {
    while(*src && (unsigned char)(*src)<33)
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
            DBG_DEBUG(GWEN_LOGDOMAIN,
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
        if (isspace((int)((unsigned char)*src)) && !lastWasEscape) {
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
    DBG_DEBUG(GWEN_LOGDOMAIN, "Missing \" after word");
    return 0;
  }
  /* check whether the source string was correctly terminated */
  if (flags & GWEN_TEXT_FLAGS_NEED_DELIMITER) {
    if (*src) {
      if (strchr(delims, *src)==0) {
        DBG_ERROR(GWEN_LOGDOMAIN, "No delimiter found within specified length");
	return 0;
      }
    }
    else {
      if (!(flags & GWEN_TEXT_FLAGS_NULL_IS_DELIMITER)) {
	DBG_ERROR(GWEN_LOGDOMAIN, "String ends without delimiter");
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



int GWEN_Text_GetWordToBuffer(const char *src,
                              const char *delims,
                              GWEN_BUFFER *buf,
                              GWEN_TYPE_UINT32 flags,
                              const char **next){
  int lastWasBlank;
  int lastBlankPos;
  int insideQuotes;
  int lastWasEscape;

  /* skip leading blanks, if wanted */
  if (flags & GWEN_TEXT_FLAGS_DEL_LEADING_BLANKS) {
    while(*src && (unsigned char)(*src)<33)
      src++;
  }

  /* get word */
  lastWasBlank=0;
  lastBlankPos=-1;
  lastWasEscape=0;
  insideQuotes=0;

  if (*src=='"') {
    insideQuotes=1;
    if (flags & GWEN_TEXT_FLAGS_DEL_QUOTES)
      src++;
  }

  while(*src) {
    if (lastWasEscape) {
      GWEN_Buffer_AppendByte(buf, *src);
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
	    DBG_ERROR(GWEN_LOGDOMAIN,
		      "Found a closing \" without an opening one "
		      "(consider using a backslash to escape)");
	    return -1;
          }
        }


        if (insideQuotes ||
            !lastWasBlank ||
            (lastWasBlank &&
             !(flags & GWEN_TEXT_FLAGS_DEL_MULTIPLE_BLANKS))) {
          /* only copy if last char was NOT blank or
           * last was blank but the caller does not want to have multiple
           * blanks removed */
	  GWEN_Buffer_AppendByte(buf, *src);
        }
        /* remember next loop whether this char was a blank */
        if (isspace((int)*src) && !lastWasEscape) {
          lastWasBlank=1;
	  lastBlankPos=GWEN_Buffer_GetPos(buf);
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

  if (insideQuotes) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Missing \" after word");
    return -1;
  }
  /* check whether the source string was correctly terminated */
  if (flags & GWEN_TEXT_FLAGS_NEED_DELIMITER) {
    if (*src) {
      if (strchr(delims, *src)==0) {
        DBG_ERROR(GWEN_LOGDOMAIN, "No delimiter found within specified length");
	return -1;
      }
    }
    else {
      if (!(flags & GWEN_TEXT_FLAGS_NULL_IS_DELIMITER)) {
	DBG_ERROR(GWEN_LOGDOMAIN, "String ends without delimiter");
	return -1;
      }
    }
  }

  /* remove trailing blanks, if wanted */
  if (flags & GWEN_TEXT_FLAGS_DEL_TRAILING_BLANKS) {
    if (lastBlankPos!=-1)
      GWEN_Buffer_Crop(buf, 0, lastBlankPos);
  }

  *next=src;
  return 0;
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
	DBG_ERROR(GWEN_LOGDOMAIN, "Buffer too small");
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
	DBG_ERROR(GWEN_LOGDOMAIN, "Buffer too small");
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
	DBG_ERROR(GWEN_LOGDOMAIN, "Buffer too small");
	return 0;
      }
    }
    else {
      if (*src=='%') {
	unsigned char d1, d2;
	unsigned char c;

	/* skip '%' */
	src++;
	if (!(*src) || !isxdigit((int)*src)) {
	  DBG_ERROR(GWEN_LOGDOMAIN, "Incomplete escape sequence (no digits)");
	  return 0;
	}
	/* read first digit */
	d1=(unsigned char)(toupper(*src));

	/* get second digit */
	src++;
	if (!(*src) || !isxdigit((int)*src)) {
	  DBG_ERROR(GWEN_LOGDOMAIN, "Incomplete escape sequence (only 1 digit)");
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
	  DBG_ERROR(GWEN_LOGDOMAIN, "Buffer too small");
	  return 0;
	}
      }
      else {
	DBG_ERROR(GWEN_LOGDOMAIN, "Found non-alphanum "
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


char *GWEN_Text_ToHex(const char *src, unsigned l,
                      char *buffer, unsigned int maxsize) {
  unsigned int pos;
  unsigned int size;

  if ((l*2)+1 > maxsize) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Buffer too small");
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
                             unsigned l,
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
	DBG_ERROR(GWEN_LOGDOMAIN, "Buffer too small");
        return 0;
      }
      buffer[size++]=c;
      j++;
      if (j==groupsize) {
	if (size+1>=maxsize) {
	  DBG_ERROR(GWEN_LOGDOMAIN, "Buffer too small");
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
      DBG_ERROR(GWEN_LOGDOMAIN, "Buffer too small");
      return 0;
    }
    if (!skipThis) {
      buffer[size++]=c;
      j++;
      if (j==groupsize) {
	if (pos+1<l) {
	  if (size+1>=maxsize) {
	    DBG_ERROR(GWEN_LOGDOMAIN, "Buffer too small");
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



int GWEN_Text_ToHexBuffer(const char *src, unsigned l,
                          GWEN_BUFFER *buf,
                          unsigned int groupsize,
                          char delimiter,
                          int skipLeadingZeroes){
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
      if (GWEN_Buffer_AppendByte(buf, c)) {
        DBG_INFO(GWEN_LOGDOMAIN, "here");
        return -1;
      }
      j++;
      if (groupsize && j==groupsize) {
        if (GWEN_Buffer_AppendByte(buf, delimiter)) {
          DBG_INFO(GWEN_LOGDOMAIN, "here");
          return -1;
        }
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
    if (!skipThis) {
      if (GWEN_Buffer_AppendByte(buf, c)) {
        DBG_INFO(GWEN_LOGDOMAIN, "here");
        return -1;
      }
      j++;
      if (groupsize && j==groupsize) {
	if (pos+1<l) {
          if (GWEN_Buffer_AppendByte(buf, delimiter)) {
            DBG_INFO(GWEN_LOGDOMAIN, "here");
            return -1;
          }
        }
        j=0;
      }
    }
    pos++;
  }
  return 0;
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
    if (!isxdigit((int)*src)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Bad char in hex string");
      return -1;
    }
    d1=(unsigned char)(toupper(*src));

    /* get second digit */
    src++;
    if (!(*src) || !isxdigit((int)*src)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Incomplete hex byte (only 1 digit)");
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
        DBG_ERROR(GWEN_LOGDOMAIN, "Buffer too small (maxsize=%d)", maxsize);
      return -1;
    }
  } /* while */

  return size;
}



int GWEN_Text_FromHexBuffer(const char *src, GWEN_BUFFER *buf) {
  while(*src) {
    unsigned char d1, d2;
    unsigned char c;

    /* read first digit */
    if (isspace((int)*src)) {
      src++;
    }
    else {
      if (!isxdigit((int)*src)) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Bad char in hex string");
        return -1;
      }
      d1=(unsigned char)(toupper(*src));
  
      /* get second digit */
      src++;
      if (!(*src) || !isxdigit((int)*src)) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Incomplete hex byte (only 1 digit)");
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
      GWEN_Buffer_AppendByte(buf, (char)c);
    }
  } /* while */

  return 0;
}



int GWEN_Text_FromBcdBuffer(const char *src, GWEN_BUFFER *buf) {
  unsigned int l;
  int fakeByte;

  l=strlen(src);
  fakeByte=(l%2);
  while(*src) {
    unsigned char d1, d2;
    unsigned char c;

    if (fakeByte) {
      d1=0;
      fakeByte=0;
    }
    else {
      /* read first digit */
      if (!isdigit((int)*src)) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Bad char in bcd string");
        return -1;
      }
      d1=(unsigned char)(*src);
      src++;
    }
    /* get second digit */
    if (!(*src) || !isxdigit((int)*src)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Incomplete BCD byte (only 1 digit)");
      return -1;
    }
    d2=(unsigned char)(*src);
    src++;

    /* compute character */
    d1-='0';
    c=(d1<<4)&0xf0;
    d2-='0';
    c+=(d2&0xf);
    /* store character */
    GWEN_Buffer_AppendByte(buf, (char)c);
  } /* while */

  return 0;
}



int GWEN_Text_ToBcdBuffer(const char *src, unsigned l,
                          GWEN_BUFFER *buf,
                          unsigned int groupsize,
                          char delimiter,
                          int skipLeadingZeroes){
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
    c+='0';
    if (!skipThis) {
      if (GWEN_Buffer_AppendByte(buf, c)) {
        DBG_INFO(GWEN_LOGDOMAIN, "here");
        return -1;
      }
      j++;
      if (groupsize && j==groupsize) {
        if (GWEN_Buffer_AppendByte(buf, delimiter)) {
          DBG_INFO(GWEN_LOGDOMAIN, "here");
          return -1;
        }
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
    c+='0';
    if (!skipThis) {
      if (GWEN_Buffer_AppendByte(buf, c)) {
        DBG_INFO(GWEN_LOGDOMAIN, "here");
        return -1;
      }
      j++;
      if (groupsize && j==groupsize) {
	if (pos+1<l) {
          if (GWEN_Buffer_AppendByte(buf, delimiter)) {
            DBG_INFO(GWEN_LOGDOMAIN, "here");
            return -1;
          }
        }
        j=0;
      }
    }
    pos++;
  }
  return 0;
}



int GWEN_Text_Compare(const char *s1, const char *s2, int ign) {
  if (s1)
    if (*s1==0)
      s1=0;
  if (s2)
    if (*s2==0)
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

  unsigned int _wpos = *wpos, _ppos = *ppos, _matches = *matches;

  a=0;
  b=0;
  wlength=strlen(w);
  plength=strlen(p);

  while (_wpos<wlength && _ppos<plength) {
    a=w[_wpos];
    b=p[_ppos];
    if (b=='*') {
      *wpos = _wpos;
      *ppos = _ppos;
      *matches = _matches;
      return 1;
    }
    if (!sensecase) {
      a=toupper(a);
      b=toupper(b);
    }
    /* count matches */
    if (a==b)
      ++_matches;
    if (a!=b && b!='?') {
      *wpos = _wpos;
      *ppos = _ppos;
      *matches = _matches;
      return 0;
    }
    ++_wpos;
    ++_ppos;
  }
  /* both at end, would be ok */
  if (_wpos==wlength && _ppos==plength) {
    *wpos = _wpos;
    *ppos = _ppos;
    *matches = _matches;
    return 1;
  }
  /* word ends, pattern doesnt, would be ok if pattern is '*' here */
  if (_wpos>=wlength && _ppos<plength)
    if (p[_ppos]=='*') {
      *wpos = _wpos;
      *ppos = _ppos;
      *matches = _matches;
      return 1;
    }
  /* otherwise no match ;-/ */
  *wpos = _wpos;
  *ppos = _ppos;
  *matches = _matches;
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

  /* compare until first occurrence of '*' */
  if (!GWEN_Text__cmpSegment(w,&wpos,p,&ppos,sensecase,&matches)) {
    return -1;
  }

  while(1) {
    /* if pattern ends we have done it */
    if (ppos>=plength)
      return matches;
    /* skip '*' in pattern */
    ppos++;
    /* if pattern ends behind '*' the word matches */
    if (ppos>=plength)
      return matches;
    /* find next matching segment */
    if (!GWEN_Text__findSegment(w,&wpos,p,&ppos,sensecase,&matches)) {
      return -1;
    }
  } /* while */
  /* I know, we'll never get to here ;-) */
  return -1;
}



int GWEN_Text_NumToString(int num, char *buffer, unsigned int bufsize,
                          int fillchar){
  char lbuffer[128];
  unsigned int i;

  sprintf(lbuffer,"%d", num);
  i=strlen(lbuffer);
  if (i>=bufsize) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Buffer too small (%d>=%d)", i, bufsize);
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



void GWEN_Text_DumpString(const char *s, unsigned l, FILE *f, unsigned insert) {
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

    /* show hex dump */
    for (i=pos; i<j; i++) {
      fprintf(f,"%02x ",(unsigned char)s[i]);
    }
    if (j-pos<16)
      for (i=0; i<16-(j-pos); i++)
	fprintf(f,"   ");
    /* show text */
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



void GWEN_Text_DumpString2Buffer(const char *s, unsigned l,
                                 GWEN_BUFFER *mbuf,
                                 unsigned insert) {
  unsigned int i;
  unsigned int j;
  unsigned int pos;
  unsigned k;
  char numbuf[32];

  pos=0;
  for (k=0; k<insert; k++)
    GWEN_Buffer_AppendByte(mbuf, ' ');
  GWEN_Buffer_AppendString(mbuf,"String size is ");
  snprintf(numbuf, sizeof(numbuf), "%d", l);
  GWEN_Buffer_AppendString(mbuf, numbuf);
  GWEN_Buffer_AppendByte(mbuf, '\n');
  while(pos<l) {
    for (k=0; k<insert; k++)
      GWEN_Buffer_AppendByte(mbuf, ' ');
    snprintf(numbuf, sizeof(numbuf),"%04x: ",pos);
    GWEN_Buffer_AppendString(mbuf, numbuf);
    j=pos+16;
    if (j>=l)
      j=l;

    /* show hex dump */
    for (i=pos; i<j; i++) {
      snprintf(numbuf, sizeof(numbuf),"%02x ", (unsigned char)s[i]);
      GWEN_Buffer_AppendString(mbuf, numbuf);
    }
    if (j-pos<16)
      for (i=0; i<16-(j-pos); i++)
        GWEN_Buffer_AppendString(mbuf, "   ");
    /* show text */
    for (i=pos; i<j; i++) {
      if (s[i]<32)
        GWEN_Buffer_AppendByte(mbuf, '.');
      else
        GWEN_Buffer_AppendByte(mbuf, s[i]);
    }
    GWEN_Buffer_AppendByte(mbuf, '\n');
    pos+=16;
  }
}







int GWEN_Text_EscapeToBuffer(const char *src, GWEN_BUFFER *buf) {
  while(*src) {
    unsigned char x;

    x=(unsigned char)*src;
    if (!(
          (x>='A' && x<='Z') ||
          (x>='a' && x<='z') ||
          (x>='0' && x<='9'))) {
      unsigned char c;

      GWEN_Buffer_AppendByte(buf, '%');
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



int GWEN_Text_UnescapeToBuffer(const char *src, GWEN_BUFFER *buf) {
  while(*src) {
    unsigned char x;

    x=(unsigned char)*src;
    if (
	(x>='A' && x<='Z') ||
	(x>='a' && x<='z') ||
	(x>='0' && x<='9')) {
      GWEN_Buffer_AppendByte(buf, *src);
    }
    else {
      if (*src=='%') {
	unsigned char d1, d2;
	unsigned char c;

	/* skip '%' */
	src++;
	if (!(*src) || !isxdigit((int)*src)) {
	  DBG_ERROR(GWEN_LOGDOMAIN, "Incomplete escape sequence (no digits)");
	  return 0;
	}
	/* read first digit */
	d1=(unsigned char)(toupper(*src));

	/* get second digit */
	src++;
	if (!(*src) || !isxdigit((int)*src)) {
	  DBG_ERROR(GWEN_LOGDOMAIN, "Incomplete escape sequence (only 1 digit)");
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
        GWEN_Buffer_AppendByte(buf, (char)c);
      }
      else {
        DBG_ERROR(GWEN_LOGDOMAIN, "Found non-alphanum "
                  "characters in escaped string (\"%s\")",
                  src);
        return -1;
      }
    }
    src++;
  } /* while */

  return 0;
}



int GWEN_Text_EscapeToBufferTolerant(const char *src, GWEN_BUFFER *buf) {
  while(*src) {
    unsigned char x;

    x=(unsigned char)*src;
    if (!(
          (x>='A' && x<='Z') ||
          (x>='a' && x<='z') ||
          (x>='0' && x<='9') ||
          x==' ' ||
          x=='.' ||
          x==',' ||
          x=='.' ||
          x=='*' ||
          x=='?'
         )) {
      unsigned char c;

      GWEN_Buffer_AppendByte(buf, '%');
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



int GWEN_Text_UnescapeToBufferTolerant(const char *src, GWEN_BUFFER *buf) {
  while(*src) {
    const char *srcBak;
    int charHandled;

    srcBak=src;
    charHandled=0;
    if (*src=='%') {
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



int GWEN_Text_EscapeToBufferTolerant2(GWEN_BUFFER *src, GWEN_BUFFER *buf) {
  while(GWEN_Buffer_GetBytesLeft(src)) {
    int z;
    unsigned char x;

    z=GWEN_Buffer_ReadByte(src);
    if (z==-1) {
      DBG_INFO(GWEN_LOGDOMAIN, "here");
      return -1;
    }
    x=(unsigned char)z;
    if (!(
          (x>='A' && x<='Z') ||
          (x>='a' && x<='z') ||
          (x>='0' && x<='9') ||
          x==' ' ||
          x=='.' ||
          x==',' ||
          x=='.' ||
          x=='*' ||
          x=='?'
         )) {
      unsigned char c;

      GWEN_Buffer_AppendByte(buf, '%');
      c=(((unsigned char)x)>>4)&0xf;
      if (c>9)
	c+=7;
      c+='0';
      GWEN_Buffer_AppendByte(buf, c);
      c=((unsigned char)x)&0xf;
      if (c>9)
        c+=7;
      c+='0';
      GWEN_Buffer_AppendByte(buf, c);
    }
    else
      GWEN_Buffer_AppendByte(buf, x);
  } /* while */

  return 0;
}



void GWEN_Text_LogString(const char *s, unsigned l,
                         const char *logDomain,
                         GWEN_LOGGER_LEVEL lv){
  GWEN_BUFFER *mbuf;

  mbuf=GWEN_Buffer_new(0, ((l*16)<1024)?1024:l*16, 0, 1);
  GWEN_Text_DumpString2Buffer(s, l, mbuf, 0);
  GWEN_Logger_Log(logDomain, lv, GWEN_Buffer_GetStart(mbuf));
  GWEN_Buffer_free(mbuf);
}



void GWEN_Text_CondenseBuffer(GWEN_BUFFER *buf){
  const char *p;
  char *dst;
  unsigned int size;
  unsigned int i;
  int lastWasBlank;
  char *lastBlankPos;

  dst=GWEN_Buffer_GetStart(buf);
  p=dst;
  size=GWEN_Buffer_GetUsedBytes(buf);
  lastWasBlank=0;
  lastBlankPos=0;

  for (i=0; i<size; i++) {
    /* remember next loop whether this char was a blank */
    if (isspace((int)*p)) {
      if (!lastWasBlank) {
        /* store only one blank */
        lastWasBlank=1;
        lastBlankPos=dst;
        *(dst++)=*p;
      }
    }
    else {
      lastWasBlank=0;
      lastBlankPos=0;
      *(dst++)=*p;
    }
    p++;
  }

  /* remove trailing blanks */
  if (lastBlankPos!=0)
    dst=lastBlankPos;

  size=dst-GWEN_Buffer_GetStart(buf);
  GWEN_Buffer_Crop(buf, 0, size);
}



int GWEN_Text_DoubleToBuffer(double num, GWEN_BUFFER *buf){
  char numbuf[128];
  int rv;
#ifdef HAVE_SETLOCALE
  const char *orig_locale = setlocale(LC_NUMERIC, NULL);
  char *currentLocale = strdup(orig_locale ? orig_locale : "C");
  setlocale(LC_NUMERIC,"C");
#endif

  rv=snprintf(numbuf, sizeof(numbuf), "%lf", num);

#ifdef HAVE_SETLOCALE
  setlocale(LC_NUMERIC, currentLocale);
  free(currentLocale);
#endif

  if (rv<1 || rv>=sizeof(numbuf))
    return -1;
  GWEN_Buffer_AppendString(buf, numbuf);
  return 0;
}



int GWEN_Text_StringToDouble(const char *s, double *num){
  int rv;
#ifdef HAVE_SETLOCALE
  const char *orig_locale = setlocale(LC_NUMERIC, NULL);
  char *currentLocale = strdup(orig_locale ? orig_locale : "C");
  setlocale(LC_NUMERIC,"C");
#endif

  rv=sscanf(s, "%lf", num);

#ifdef HAVE_SETLOCALE
  setlocale(LC_NUMERIC, currentLocale);
  free(currentLocale);
#endif

  if (rv!=1)
    return -1;
  return 0;
}



double GWEN_Text__CheckSimilarity(const char *s1, const char *s2, int ign){
  int nboth;
  int nmatch;
  double pc;

  nboth=strlen(s1)+strlen(s2);
  nmatch=0;
  if (ign) {
    while(*s1 && *s2) {
      const char *t;
      int lmatch;

      /* find next equal in s2 */
      t=s2;
      lmatch=0;
      while(*t) {
        if (toupper(*s1)==toupper(*t)) {
          lmatch=2;
          break;
        }
        if (isalnum((int)*s1) && isalnum((int)*t)) {
          lmatch=1;
          break;
        }
      } /* while */

      if (lmatch) {
        nmatch+=lmatch;
        s2=t+1;
      }

      s1++;
    } /* while */
  }
  else {
    while(*s1 && *s2) {
      const char *t;
      int lmatch;

      /* find next equal in s2 */
      t=s2;
      lmatch=0;
      while(*t) {
        if (*s1==*t) {
          lmatch=2;
          break;
        }
        if (toupper(*s1)==toupper(*t)) {
          lmatch=1;
          break;
        }
        if (isalnum((int)*s1) && isalnum((int)*t)) {
          lmatch=1;
          break;
        }
      } /* while */

      if (lmatch) {
        nmatch+=lmatch;
        s2=t+1;
      }

      s1++;
    } /* while */
  }

  pc=(nmatch*100)/nboth;
  return pc;
}



double GWEN_Text_CheckSimilarity(const char *s1, const char *s2, int ign){
  double pc1, pc2;

  pc1=GWEN_Text__CheckSimilarity(s1, s2, ign);
  pc2=GWEN_Text__CheckSimilarity(s2, s1, ign);
  if (pc2>pc1)
    return pc2;
  return pc1;
}



int GWEN_Text_CountUtf8Chars(const char *s, int len) {
  int count;
  int handled;

  if (len==0)
    len=strlen(s);
  count=0;
  handled=0;
  while(handled<len) {
    unsigned char c;
    int i;

    c=(unsigned char)*s;
    if ((c & 0xfe)==0xfc)
      i=5;
    else if ((c & 0xfc)==0xf8)
      i=4;
    else if ((c & 0xf8)==0xf0)
      i=3;
    else if ((c & 0xf0)==0xe0)
      i=2;
    else if ((c & 0xe0)==0xc0)
      i=1;
    else if (c & 0x80) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Invalid UTF8 character at pos %d", handled);
      return -1;
    }
    else
      i=0;
    if (handled+i+1>len) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Incomplete UTF8 sequence at pos %d", handled);
      return -1;
    }
    s++;
    if (i) {
      int j;

      for (j=0; j<j; j++) {
        if ((((unsigned char)*s) & 0xc0)!=0xc0) {
          DBG_ERROR(GWEN_LOGDOMAIN,
                    "Invalid UTF8 sequence at pos %d (rel %d of %d)",
                    handled, j, i);
        }
        s++;
      }
    }
    handled+=i+1;
    count++;
  } /* while */

  return count;
}



int GWEN_Text_UnescapeXmlToBuffer(const char *src, GWEN_BUFFER *buf) {
  while(*src) {
    unsigned char x;
    int match;

    match=0;
    x=(unsigned char)*src;
    if (x=='&') {
      const GWEN_TEXT_ESCAPE_ENTRY *e;
      e=gwen_text__xml_escape_chars;
      while(e->replace) {
        int l;

        l=strlen(e->replace);
        if (strncasecmp(src, e->replace, l)==0) {
          GWEN_Buffer_AppendByte(buf, e->character);
          src+=l;
          match=1;
          break;
        }
        e++;
      } /* while */
    }
    if (!match) {
      GWEN_Buffer_AppendByte(buf, *src);
      src++;
    }
  } /* while */

  return 0;
}



int GWEN_Text_EscapeXmlToBuffer(const char *src, GWEN_BUFFER *buf) {
  while(*src) {
    unsigned char x;
    const GWEN_TEXT_ESCAPE_ENTRY *e;
    int match;

    match=0;
    x=(unsigned char)*src;
    e=gwen_text__xml_escape_chars;
    while(e->replace) {
      if (x==e->character) {
        GWEN_Buffer_AppendString(buf, e->replace);
        match=1;
        break;
      }
      e++;
    } /* while */

    if (!match) {
      GWEN_Buffer_AppendByte(buf, *src);
    }
    src++;
  } /* while */

  return 0;
}












