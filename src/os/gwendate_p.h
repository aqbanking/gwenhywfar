/***************************************************************************
    begin       : Tue Jul 07 2009
    copyright   : (C) 2019 by Martin Preuss
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


#ifndef GWEN_DATE_P_H
#define GWEN_DATE_P_H

#include "gwendate.h"


#define GWEN_DATE_TMPL_MAX_COUNT 32


struct GWEN_DATE {
  int year;
  int month;
  int day;
  int julian;
  char asString[10];
};



typedef struct GWEN_DATE_TMPLCHAR GWEN_DATE_TMPLCHAR;
GWEN_LIST_FUNCTION_DEFS(GWEN_DATE_TMPLCHAR, GWEN_DateTmplChar)
struct GWEN_DATE_TMPLCHAR {
  GWEN_LIST_ELEMENT(GWEN_DATE_TMPLCHAR)
  char character;
  int count;
  int maxCount;
  int nextChar;
  char *content;
};

static GWEN_DATE_TMPLCHAR *GWEN_DateTmplChar_new(char c);
static void GWEN_DateTmplChar_free(GWEN_DATE_TMPLCHAR *e);
static GWEN_DATE_TMPLCHAR *GWEN_Date__findTmplChar(GWEN_DATE_TMPLCHAR_LIST *ll, char c);
static void GWEN_Date__sampleTmplChars(GWEN_UNUSED const GWEN_DATE *t, const char *tmpl,
                                       GWEN_UNUSED GWEN_BUFFER *buf,
                                       GWEN_DATE_TMPLCHAR_LIST *ll);
static void GWEN_Date__fillTmplChars(const GWEN_DATE *t,
                                     GWEN_DATE_TMPLCHAR_LIST *ll);

static void GWEN_Date_setJulian(GWEN_DATE *gd, int julian);







#endif



