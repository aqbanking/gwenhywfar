/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Wed Mar 24 2004
    copyright   : (C) 2004 by Martin Preuss
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


#ifndef GWEN_TIME_P_H
#define GWEN_TIME_P_H

#include "gwentime_l.h"


#define GWEN_TIME_TMPL_MAX_COUNT 8

struct GWEN_TIME {
  uint32_t secs;
  uint32_t msecs;
};



uint32_t GWEN_Time__mktimeUtc(int year,
                              int month,
                              int day,
                              int hour,
                              int min,
                              int sec);



typedef struct GWEN_TIME_TMPLCHAR GWEN_TIME_TMPLCHAR;
GWEN_LIST_FUNCTION_DEFS(GWEN_TIME_TMPLCHAR, GWEN_TimeTmplChar)
struct GWEN_TIME_TMPLCHAR {
  GWEN_LIST_ELEMENT(GWEN_TIME_TMPLCHAR)
  char character;
  int count;
  int nextChar;
  char *content;
};
static GWEN_TIME_TMPLCHAR *GWEN_TimeTmplChar_new(char c);
static void GWEN_TimeTmplChar_free(GWEN_TIME_TMPLCHAR *e);

static GWEN_TIME_TMPLCHAR *GWEN_Time__findTmplChar(GWEN_TIME_TMPLCHAR_LIST *ll,
                                                   char c);
static void GWEN_Time__sampleTmplChars(const GWEN_TIME *t, const char *tmpl,
                                       GWEN_BUFFER *buf,
                                       GWEN_TIME_TMPLCHAR_LIST *ll);
static void GWEN_Time__fillTmplChars(const GWEN_TIME *t,
                                     GWEN_TIME_TMPLCHAR_LIST *ll,
                                     int useUtc);
static int GWEN_Time__toString(const GWEN_TIME *t, const char *tmpl,
                               GWEN_BUFFER *buf, int useUtc);


#endif



