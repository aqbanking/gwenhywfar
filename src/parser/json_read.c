/***************************************************************************
 copyright   : (C) 2023 by Martin Preuss
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


#include "./json_p.h"
#include "./json_read.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>

#include <ctype.h>



static GWEN_JSON_ELEM *_readObject(const char *s, const char **next);
static GWEN_JSON_ELEM *_readArray(const char *s, const char **next);
static int _readKey(const char *s, GWEN_BUFFER *buf, const char **next);
static GWEN_JSON_ELEM *_readValue(const char *s, const char **next);
static int _readSimpleValueAndReturnType(const char *s, GWEN_BUFFER *buf, const char **next);
static int _readTokenUntilChar(const char *s, GWEN_BUFFER *buf, int c, const char **next);
static int _readQuotedToken(const char *s, GWEN_BUFFER *buf, const char **next);




GWEN_JSON_ELEM *GWEN_JsonElement_fromString(const char *s)
{
  while(*s && isspace(*s))
    s++;
  if (*s=='{') {
    GWEN_JSON_ELEM *jeObject;

    s++;
    jeObject=_readObject(s, &s);
    if (jeObject==NULL) {
      DBG_INFO(GWEN_LOGDOMAIN, "here");
      return NULL;
    }

    return jeObject;
  }

  return NULL;
}




GWEN_JSON_ELEM *_readObject(const char *s, const char **next)
{
  GWEN_BUFFER *buf;
  GWEN_JSON_ELEM *jeObject;

  jeObject=GWEN_JsonElement_new(GWEN_JSON_ELEMTYPE_OBJECT, NULL);
  buf=GWEN_Buffer_new(0, 64, 0, 1);
  while(*s) {
    int rv;
    GWEN_JSON_ELEM *jeKey;
    GWEN_JSON_ELEM *jeValue;

    while(*s && isspace(*s))
      s++;
    if (!*s || *s=='}')
      break;

    GWEN_Buffer_Reset(buf);
    rv=_readKey(s, buf, &s);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(buf);
      GWEN_JsonElement_free(jeObject);
      return NULL;
    }
    jeKey=GWEN_JsonElement_new(GWEN_JSON_ELEMTYPE_KEY, GWEN_Buffer_GetStart(buf));
    GWEN_JsonElement_Tree2_AddChild(jeObject, jeKey);

    while(*s && isspace(*s))
      s++;

    jeValue=_readValue(s, &s);
    if (jeValue==NULL) {
      DBG_INFO(GWEN_LOGDOMAIN, "here");
      GWEN_Buffer_free(buf);
      GWEN_JsonElement_free(jeObject);
      return NULL;
    }
    GWEN_JsonElement_Tree2_AddChild(jeKey, jeValue);

    while(*s && isspace(*s))
      s++;

    if (*s!=',')
      break;
    s++;
  } /* while */
  GWEN_Buffer_free(buf);

  if (*s!='}') {
    DBG_INFO(GWEN_LOGDOMAIN, "missing closing curly bracket");
    GWEN_JsonElement_free(jeObject);
    return NULL;
  }
  s++;
  if (next)
    *next=s;
  return jeObject;
}



GWEN_JSON_ELEM *_readArray(const char *s, const char **next)
{
  GWEN_JSON_ELEM *jeArray;

  jeArray=GWEN_JsonElement_new(GWEN_JSON_ELEMTYPE_ARRAY, NULL);
  while(*s) {
    GWEN_JSON_ELEM *jeValue;

    while(*s && isspace(*s))
      s++;
    if (!*s || *s==']')
      break;

    jeValue=_readValue(s, &s);
    if (jeValue==NULL) {
      DBG_INFO(GWEN_LOGDOMAIN, "here");
      GWEN_JsonElement_free(jeArray);
      return NULL;
    }
    GWEN_JsonElement_Tree2_AddChild(jeArray, jeValue);

    while(*s && isspace(*s))
      s++;

    if (*s!=',')
      break;
    s++;
  } /* while */

  if (*s!=']') {
    DBG_INFO(GWEN_LOGDOMAIN, "missing closing square bracket");
    GWEN_JsonElement_free(jeArray);
    return NULL;
  }
  s++;
  if (next)
    *next=s;
  return jeArray;
}



int _readKey(const char *s, GWEN_BUFFER *buf, const char **next)
{
  int rv;

  /* skip blanks */
  while(*s && isspace(*s))
    s++;
  if (!(*s))
    return GWEN_ERROR_NO_DATA;

  if (*s!='"') {
    DBG_INFO(GWEN_LOGDOMAIN, "missing opening quote");
    return GWEN_ERROR_BAD_DATA;
  }

  /* read everything until closing quote */
  s++;
  rv=_readQuotedToken(s, buf, &s);
  if (rv<1) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return GWEN_ERROR_BAD_DATA;
  }
  if (*s!='"') {
    DBG_INFO(GWEN_LOGDOMAIN, "missing closing quote");
    return GWEN_ERROR_BAD_DATA;
  }
  s++;
  while(*s && isspace(*s))
    s++;
  if (*s!=':') {
    DBG_INFO(GWEN_LOGDOMAIN, "missing colon char");
    return GWEN_ERROR_BAD_DATA;
  }
  s++;

  if (next)
    *next=s;
  return 0;
}



GWEN_JSON_ELEM *_readValue(const char *s, const char **next)
{
  GWEN_JSON_ELEM *jeValue=NULL;

  /* skip blanks */
  while(*s && isspace(*s))
    s++;
  if (*s=='{') {
    s++;
    jeValue=_readObject(s, &s);
    if (jeValue==NULL) {
      DBG_INFO(GWEN_LOGDOMAIN, "error reading object");
      return NULL;
    }
  }
  else if (*s=='[') {
    s++;
    jeValue=_readArray(s, &s);
    if (jeValue==NULL) {
      DBG_INFO(GWEN_LOGDOMAIN, "error reading array");
      return NULL;
    }
  }
  else {
    GWEN_BUFFER *buf;
    int rv;

    buf=GWEN_Buffer_new(0, 64, 0, 1);
    rv=_readSimpleValueAndReturnType(s, buf, &s);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(buf);
      return NULL;
    }
    jeValue=GWEN_JsonElement_new(rv, GWEN_Buffer_GetStart(buf));
    GWEN_Buffer_free(buf);
  }

  if (next)
    *next=s;

  return jeValue;
}




int _readSimpleValueAndReturnType(const char *s, GWEN_BUFFER *buf, const char **next)
{
  int rv;
  int elementType=GWEN_JSON_ELEMTYPE_UNKNOWN;

  /* skip blanks */
  while(*s && isspace(*s))
    s++;
  if (!(*s))
    return GWEN_ERROR_NO_DATA;

  if (*s=='"') {
    /* string element, read everything until closing quote */
    s++;
    rv=_readQuotedToken(s, buf, &s);
    if (rv<1) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return GWEN_ERROR_BAD_DATA;
    }
    if (*s!='"') {
      DBG_INFO(GWEN_LOGDOMAIN, "missing closing quote");
      return GWEN_ERROR_BAD_DATA;
    }
    s++;
    elementType=GWEN_JSON_ELEMTYPE_STRING;
  }
  else {
    uint32_t pos;
    const char *p;

    pos=GWEN_Buffer_GetPos(buf);
    rv=_readTokenUntilChar(s, buf, 0, &s);
    if (rv<1) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return GWEN_ERROR_BAD_DATA;
    }
    /* determine type */
    p=GWEN_Buffer_GetStart(buf)+pos;
    if (*p=='-' || *p=='.' || isdigit(*p)) {
      elementType=GWEN_JSON_ELEMTYPE_NUM;
    }
    else if (strcasecmp(p, "TRUE")==0 || strcasecmp(p, "FALSE")==0) {
      elementType=GWEN_JSON_ELEMTYPE_BOOL;
    }
    else if (strcasecmp(p, "NULL")==0) {
      elementType=GWEN_JSON_ELEMTYPE_NULL;
    }
  }
  if (next)
    *next=s;
  return elementType;
}



int _readTokenUntilChar(const char *s, GWEN_BUFFER *buf, int c, const char **next)
{
  int lastWasEscape=0;
  int bytesAdded=0;

  while(*s) {
    if (lastWasEscape) {
      GWEN_Buffer_AppendByte(buf, *s);
      lastWasEscape=0;
      bytesAdded++;
    }
    else {
      if (*s=='\\')
	lastWasEscape=1;
      else {
	if (*s==c || strchr(",:{}[]", *s)!=NULL)
          break;
        if (!iscntrl(*s)) {
          GWEN_Buffer_AppendByte(buf, *s);
          bytesAdded++;
        }
      }
    }
    s++;
  }
  if (next)
    *next=s;

  return bytesAdded;
}



int _readQuotedToken(const char *s, GWEN_BUFFER *buf, const char **next)
{
  int lastWasEscape=0;
  int bytesAdded=0;

  while(*s) {
    if (lastWasEscape) {
      GWEN_Buffer_AppendByte(buf, *s);
      lastWasEscape=0;
      bytesAdded++;
    }
    else {
      if (*s=='\\')
	lastWasEscape=1;
      else {
	if (*s=='"')
	  break;
	GWEN_Buffer_AppendByte(buf, *s);
	bytesAdded++;
      }
    }
    s++;
  }
  if (next)
    *next=s;

  return bytesAdded;
}





