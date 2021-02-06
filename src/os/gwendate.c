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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "gwendate_p.h"
#include "i18n_l.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>


#include <time.h>
#include <ctype.h>



static const uint8_t daysInMonth[12]= {
  31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};


static void _writeAsString(GWEN_DATE *gd);
static GWEN_DATE *_createFromGregorianAndUseGivenString(int y, int m, int d, const char *s);




GWEN_DATE *GWEN_Date_fromGregorian(int y, int m, int d)
{
  return _createFromGregorianAndUseGivenString(y, m, d, NULL);
}



/* if s is given it must contain a string of minimum 8 bytes length from which the date was derived,
 * if not given the string representation will be generated. */
GWEN_DATE *_createFromGregorianAndUseGivenString(int y, int m, int d, const char *s)
{
  GWEN_DATE *gd;

  if (m<1 || m>12 || d<1 || d>31) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Bad date values (erroneous year=%d, month=%d, day=%d)", y, m, d);
    return NULL;
  }

  GWEN_NEW_OBJECT(GWEN_DATE, gd);
  gd->year=y;
  gd->month=m;
  gd->day=d;
  gd->julian=(1461*(y+4800+(m-14)/12))/4+
             (367*(m-2-12*((m-14)/12)))/12-
             (3*((y+4900+(m-14)/12)/100))/4+
             d-32075;

  if (s && *s) {
    memmove(gd->asString, s, 8);
    gd->asString[8]=0;
  }
  else
    _writeAsString(gd);

  return gd;
}



void GWEN_Date_setJulian(GWEN_DATE *gd, int julian)
{
  int l, n, i, j /*, len */;

  l=julian+68569;
  n=(4*l)/146097;
  l=l-(146097*n+3)/4;
  i=(4000*(l+1))/1461001;
  l=l-(1461*i)/4+31;
  j=(80*l)/2447;
  gd->day=l-(2447*j)/80;
  l=j/11;
  gd->month=j+2-(12*l);
  gd->year=100*(n-49)+i+l;
  gd->julian=julian;

#if 1
  _writeAsString(gd);
#else
  len=snprintf(gd->asString, sizeof(gd->asString)-1,
               "%04d%02d%02d",
               gd->year, gd->month, gd->day);
  gd->asString[sizeof(gd->asString)-1]=0;
  if ((int)(sizeof(gd->asString)-1) < len) {
    DBG_ERROR(GWEN_LOGDOMAIN, "truncated date string [%s]", gd->asString);
  }
#endif

}



void _writeAsString(GWEN_DATE *gd)
{
  char *ptr;
  int x;

  ptr=gd->asString+8;
  *(ptr--)=0;

  x=gd->day;
  *(ptr--)='0'+(x%10);
  x/=10;
  *(ptr--)='0'+(x%10);

  x=gd->month;
  *(ptr--)='0'+(x%10);
  x/=10;
  *(ptr--)='0'+(x%10);

  x=gd->year;
  *(ptr--)='0'+(x%10);
  x/=10;
  *(ptr--)='0'+(x%10);
  x/=10;
  *(ptr--)='0'+(x%10);
  x/=10;
  *ptr='0'+(x%10);
}


void GWEN_Date_AddDays(GWEN_DATE *gd, int days)
{
  GWEN_Date_setJulian(gd, gd->julian+days);
}



void GWEN_Date_SubDays(GWEN_DATE *gd, int days)
{
  GWEN_Date_setJulian(gd, gd->julian-days);
}



GWEN_DATE *GWEN_Date_fromJulian(int julian)
{
  GWEN_DATE *gd;

  GWEN_NEW_OBJECT(GWEN_DATE, gd);
  GWEN_Date_setJulian(gd, julian);
  return gd;
}



GWEN_DATE *GWEN_Date_fromLocalTime(time_t t)
{
  struct tm *ltm;

  ltm=localtime(&t);
  if (ltm) {
    GWEN_DATE *gd;

    gd=GWEN_Date_fromGregorian(ltm->tm_year+1900, ltm->tm_mon+1, ltm->tm_mday);
    return gd;
  }

  return NULL;
}



time_t GWEN_Date_toLocalTime(const GWEN_DATE *gd)
{
  struct tm ti;
  struct tm *tp;
  time_t tt;

  tt=time(0);
  tp=localtime(&tt);
  assert(tp);
  memmove(&ti, tp, sizeof(ti));
  ti.tm_sec=0;
  ti.tm_min=0;
  ti.tm_hour=0;
  ti.tm_year=gd->year-1900;
  ti.tm_mon=gd->month-1;
  ti.tm_mday=gd->day;
  ti.tm_yday=0;
  ti.tm_wday=0;
  tt=mktime(&ti);
  assert(tt!=(time_t)-1);
  return tt;
}



GWEN_DATE *GWEN_Date_fromGmTime(time_t t)
{
  struct tm *ltm;

  ltm=gmtime(&t);
  if (ltm) {
    GWEN_DATE *gd;

    gd=GWEN_Date_fromGregorian(ltm->tm_year+1900, ltm->tm_mon+1, ltm->tm_mday);
    return gd;
  }

  return NULL;
}




GWEN_DATE *GWEN_Date_CurrentDate(void)
{
  time_t l;

  time(&l);
  return GWEN_Date_fromLocalTime(l);
}



GWEN_DATE *GWEN_Date_dup(const GWEN_DATE *ogd)
{
  GWEN_DATE *gd;

  assert(ogd);

  GWEN_NEW_OBJECT(GWEN_DATE, gd);
#if 0
  gd->year=ogd->year;
  gd->month=ogd->month;
  gd->day=ogd->day;
  gd->julian=ogd->julian;
  memmove(gd->asString, ogd->asString, sizeof(gd->asString));
#else
  memmove(gd, ogd, sizeof(GWEN_DATE));
#endif
  return gd;
}



GWEN_DATE *GWEN_Date_fromString(const char *s)
{

#if 1
  if (s && strlen(s)>7) {
    int y, m, d;
    GWEN_DATE *result;
    const char *originalPtr;

    originalPtr=s;
    y=*(s++)-'0';
    y*=10;
    y+=*(s++)-'0';
    y*=10;
    y+=*(s++)-'0';
    y*=10;
    y+=*(s++)-'0';

    m=*(s++)-'0';
    m*=10;
    m+=*(s++)-'0';

    d=*(s++)-'0';
    d*=10;
    d+=*(s++)-'0';

    result=_createFromGregorianAndUseGivenString(y, m, d, originalPtr);
    if (!result) {
      DBG_INFO(GWEN_LOGDOMAIN, "Bad date string [%s]", originalPtr);
    }
    return result;
  }
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "Bad date string [%s]", s?s:"<empty>");
    return NULL;
  }
#else
  int y, m, d;

  if (3==sscanf(s, "%04d%02d%02d", &y, &m, &d)) {
    GWEN_DATE *result = GWEN_Date_fromGregorian(y, m, d);
    if (!result)
      DBG_INFO(GWEN_LOGDOMAIN, "Bad date string [%s]", s);
    return result;
  }
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "Bad date string [%s]", s);
    return NULL;
  }
#endif
}



void GWEN_Date_free(GWEN_DATE *gd)
{
  if (gd) {
    GWEN_FREE_OBJECT(gd);
  }
}



int GWEN_Date_IsLeapYear(int y)
{
  return ((y%4==0) && (y%100!=0)) || (y%400==0);
}




int GWEN_Date_DaysInMonth(const GWEN_DATE *gd)
{
  assert(gd);
  if (gd->month==2 &&
      ((((gd->year%4)==0) && ((gd->year)%100!=0)) || ((gd->year)%400==0)))
    /* February in a leap year */
    return 29;
  else
    return daysInMonth[gd->month-1];
}



int GWEN_Date_DaysInYear(const GWEN_DATE *gd)
{
  GWEN_DATE *gd11;
  int result;

  assert(gd);

  gd11=GWEN_Date_fromGregorian(gd->year, 1, 1);
  result=(gd->julian)-(gd11->julian);
  GWEN_Date_free(gd11);

  return result;
}



int GWEN_Date_GetYear(const GWEN_DATE *gd)
{
  assert(gd);
  return gd->year;
}



int GWEN_Date_GetMonth(const GWEN_DATE *gd)
{
  assert(gd);
  return gd->month;
}



int GWEN_Date_GetDay(const GWEN_DATE *gd)
{
  assert(gd);
  return gd->day;
}



int GWEN_Date_GetJulian(const GWEN_DATE *gd)
{
  assert(gd);
  return gd->julian;
}



int GWEN_Date_WeekDay(const GWEN_DATE *gd)
{
  assert(gd);
  return (gd->julian+1)%7; /* 0=Sunday */
}



const char *GWEN_Date_GetString(const GWEN_DATE *gd)
{
  assert(gd);
  return gd->asString;
}



int GWEN_Date_Compare(const GWEN_DATE *gd1, const GWEN_DATE *gd0)
{
  if (gd0 && gd1) {
    if (gd1->julian==gd0->julian)
      return 0;
    else if (gd1->julian>gd0->julian)
      return 1;
    else
      return -1;
  }
  else if (gd0)
    return 1;
  else if (gd1)
    return -1;
  else
    return 0;
}



int GWEN_Date_Diff(const GWEN_DATE *gd1, const GWEN_DATE *gd0)
{
  assert(gd1);
  assert(gd0);

  return gd1->julian-gd0->julian;
}



GWEN_DATE *GWEN_Date_fromTime(const GWEN_TIME *ti)
{
  GWEN_BUFFER *tbuf;
  GWEN_DATE *gd;

  tbuf=GWEN_Buffer_new(0, 32, 0, 1);
  GWEN_Time_toString(ti, "YYYYMMDD", tbuf);
  gd=GWEN_Date_fromString(GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return gd;
}




GWEN_DATE *GWEN_Date_fromStringWithTemplate(const char *s, const char *tmpl)
{
  int year, month, day;
  const char *p;
  const char *t;
  GWEN_DATE *gwt;

  assert(s);
  assert(tmpl);
  year=month=day=0;

  p=s;
  t=tmpl;
  while (*t && *p) {
    int i;

    if (*t=='*') {
      t++;
      if (!*t) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Bad pattern: Must not end with \"*\"");
        return 0;
      }
      i=0;
      while (*p) {
        if (!isdigit((int)*p))
          break;
        if (*p==*t)
          break;
        i*=10;
        i+=(*p)-'0';
        p++;
      } /* while */
    }
    else {
      if (isdigit((int)*p))
        i=(*p)-'0';
      else
        i=-1;
      p++;
    }

    if (i==-1 && strchr("YMD", *t)!=NULL) {
      DBG_INFO(GWEN_LOGDOMAIN,
               "No more digits at [%s], continuing", t);
      p--;
    }
    else {
      switch (*t) {
      case 'Y':
        if (i==-1) {
          DBG_INFO(GWEN_LOGDOMAIN, "here");
          return 0;
        }
        year*=10;
        year+=i;
        break;
      case 'M':
        if (i==-1) {
          DBG_INFO(GWEN_LOGDOMAIN, "here");
          return 0;
        }
        month*=10;
        month+=i;
        break;
      case 'D':
        if (i==-1) {
          DBG_INFO(GWEN_LOGDOMAIN, "here");
          return 0;
        }
        day*=10;
        day+=i;
        break;
      default:
        DBG_VERBOUS(GWEN_LOGDOMAIN,
                    "Unknown character in template, will skip in both strings");
        break;
      }
    }
    t++;
  } /* while */

  if (year<100)
    year+=2000;

  DBG_DEBUG(GWEN_LOGDOMAIN,
            "Got this date/time: %04d/%02d/%02d",
            year, month, day);

  /* get time in local time */
  gwt=GWEN_Date_fromGregorian(year, month, day);
  if (!gwt) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Bad date string [%s]", s);
    return 0;
  }
  return gwt;
}





GWEN_LIST_FUNCTIONS(GWEN_DATE_TMPLCHAR, GWEN_DateTmplChar)


GWEN_DATE_TMPLCHAR *GWEN_DateTmplChar_new(char c)
{
  GWEN_DATE_TMPLCHAR *e;

  GWEN_NEW_OBJECT(GWEN_DATE_TMPLCHAR, e);
  GWEN_LIST_INIT(GWEN_DATE_TMPLCHAR, e);
  e->character=c;
  switch (c) {
  case 'Y':
    e->maxCount=4;
    break;
  case 'M':
    e->maxCount=2;
    break;
  case 'D':
    e->maxCount=2;
    break;
  case 'W':
    e->maxCount=1;
    break;
  case 'w':
  default:
    e->maxCount=GWEN_DATE_TMPL_MAX_COUNT;
    break;
  }

  return e;
}



void GWEN_DateTmplChar_free(GWEN_DATE_TMPLCHAR *e)
{
  if (e) {
    free(e->content);
    GWEN_LIST_FINI(GWEN_DATE_TMPLCHAR, e);
    GWEN_FREE_OBJECT(e);
  }
}



GWEN_DATE_TMPLCHAR *GWEN_Date__findTmplChar(GWEN_DATE_TMPLCHAR_LIST *ll, char c)
{
  GWEN_DATE_TMPLCHAR *e;

  e=GWEN_DateTmplChar_List_First(ll);
  while (e) {
    if (e->character==c)
      break;
    e=GWEN_DateTmplChar_List_Next(e);
  }

  return e;
}




void GWEN_Date__sampleTmplChars(GWEN_UNUSED const GWEN_DATE *t, const char *tmpl,
                                GWEN_UNUSED GWEN_BUFFER *buf,
                                GWEN_DATE_TMPLCHAR_LIST *ll)
{
  const char *s;

  s=tmpl;
  while (*s) {
    if (strchr("YMDWw", *s)) {
      GWEN_DATE_TMPLCHAR *e;

      e=GWEN_Date__findTmplChar(ll, *s);
      if (!e) {
        /* new entry, create it */
        e=GWEN_DateTmplChar_new(*s);
        GWEN_DateTmplChar_List_Add(e, ll);
      }
      assert(e);
      e->count++;
    }
    else {
      DBG_DEBUG(GWEN_LOGDOMAIN, "Unknown character in template (%02x)",
                *s);
    }
    s++;
  }
}



void GWEN_Date__fillTmplChars(const GWEN_DATE *t, GWEN_DATE_TMPLCHAR_LIST *ll)
{
  GWEN_DATE_TMPLCHAR *e;


  e=GWEN_DateTmplChar_List_First(ll);
  while (e) {
    int v;

    if (e->character=='w') {
      const char *s=NULL;

      switch (GWEN_Date_WeekDay(t)) {
      case 0:
        s=I18N("Sunday");
        break;
      case 1:
        s=I18N("Monday");
        break;
      case 2:
        s=I18N("Tuesday");
        break;
      case 3:
        s=I18N("Wednesday");
        break;
      case 4:
        s=I18N("Thursday");
        break;
      case 5:
        s=I18N("Friday");
        break;
      case 6:
        s=I18N("Saturday");
        break;
      default:
        DBG_DEBUG(GWEN_LOGDOMAIN, "Invalid week day (%2d)", GWEN_Date_WeekDay(t));
        s=NULL;
        break;
      }
      assert(s);
      e->content=strdup(s);
      e->nextChar=0;
    }
    else {
      char buffer[32];
      int clen;

      switch (e->character) {
      case 'Y':
        v=t->year;
        break;
      case 'M':
        v=t->month;
        break;
      case 'D':
        v=t->day;
        break;
      case 'W':
        v=GWEN_Date_WeekDay(t);
        break;
      default:
        v=-1;
        break;
      }
      if (v==-1) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Unknown character, should not happen here");
        abort();
      }
      buffer[0]=0;
      snprintf(buffer, sizeof(buffer)-1, "%0*d", e->maxCount, v);
      buffer[sizeof(buffer)-1]=0;
      e->content=strdup(buffer);
      /* adjust counter if there are more than maxCount template chars */
      clen=strlen(e->content);
      if (e->count>clen)
        e->count=clen;
      e->nextChar=clen-(e->count);
    }

    e=GWEN_DateTmplChar_List_Next(e);
  }
}




int GWEN_Date_toStringWithTemplate(const GWEN_DATE *t, const char *tmpl, GWEN_BUFFER *buf)
{
  GWEN_DATE_TMPLCHAR_LIST *ll;
  const char *s;

  ll=GWEN_DateTmplChar_List_new();
  GWEN_Date__sampleTmplChars(t, tmpl, buf, ll);
  GWEN_Date__fillTmplChars(t, ll);

  s=tmpl;
  while (*s) {
    if (strchr("YMDWw", *s)) {
      GWEN_DATE_TMPLCHAR *e;
      char c;

      e=GWEN_Date__findTmplChar(ll, *s);
      assert(e);
      assert(e->content);
      if (s[1]=='*') {
        /* append full string */
        GWEN_Buffer_AppendString(buf, e->content);
        /* skip asterisk */
        s++;
      }
      else {
        c=e->content[e->nextChar];
        if (c!=0) {
          GWEN_Buffer_AppendByte(buf, c);
          e->nextChar++;
        }
      }
    }
    else
      GWEN_Buffer_AppendByte(buf, *s);
    s++;
  }
  GWEN_DateTmplChar_List_free(ll);
  return 0;
}



int GWEN_Date_toDb(const GWEN_DATE *dt, GWEN_DB_NODE *db)
{
  const char *s;

  assert(dt);
  s=GWEN_Date_GetString(dt);
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "dateString", s);
  return 0;
}



GWEN_DATE *GWEN_Date_fromDb(GWEN_DB_NODE *db)
{
  const char *s;

  s=GWEN_DB_GetCharValue(db, "dateString", 0, NULL);
  if (s && *s) {
    GWEN_DATE *dt;

    dt=GWEN_Date_fromString(s);
    if (dt==NULL) {
      DBG_INFO(GWEN_LOGDOMAIN, "Invalid date [%s]", s);
      return NULL;
    }

    return dt;
  }
  else {
    DBG_VERBOUS(GWEN_LOGDOMAIN, "no or empty date");
    return NULL;
  }
}



GWEN_DATE *GWEN_Date_GetThisMonthStart(const GWEN_DATE *dt)
{
  return GWEN_Date_fromGregorian(GWEN_Date_GetYear(dt), GWEN_Date_GetMonth(dt), 1);
}



GWEN_DATE *GWEN_Date_GetThisMonthEnd(const GWEN_DATE *dt)
{
  int day;

  switch (GWEN_Date_GetMonth(dt)) {
  case 1:
  case 3:
  case 5:
  case 7:
  case 8:
  case 10:
  case 12:
    day=31;
    break;
  case 2:
    if (GWEN_Date_IsLeapYear(GWEN_Date_GetYear(dt)))
      day=29;
    else
      day=28;
    break;

  case 4:
  case 6:
  case 9:
  case 11:
    day=30;
    break;

  default:
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid month (%d)", GWEN_Date_GetMonth(dt));
    abort();
    break;
  }
  return GWEN_Date_fromGregorian(GWEN_Date_GetYear(dt), GWEN_Date_GetMonth(dt), day);
}



GWEN_DATE *GWEN_Date_GetThisQuarterYearStart(const GWEN_DATE *dt)
{
  int m;

  m=GWEN_Date_GetMonth(dt)>>2;
  switch (m) {
  case 0:
    return GWEN_Date_fromGregorian(GWEN_Date_GetYear(dt), 1, 1);
  case 1:
    return GWEN_Date_fromGregorian(GWEN_Date_GetYear(dt), 4, 1);
  case 2:
    return GWEN_Date_fromGregorian(GWEN_Date_GetYear(dt), 7, 1);
  case 3:
    return GWEN_Date_fromGregorian(GWEN_Date_GetYear(dt), 10, 1);
  default:
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid quarter (%d)", m);
    break;
  }

  return NULL;
}



GWEN_DATE *GWEN_Date_GetThisQuarterYearEnd(const GWEN_DATE *dt)
{
  int m;

  m=GWEN_Date_GetMonth(dt)>>2;
  switch (m) {
  case 0:
    return GWEN_Date_fromGregorian(GWEN_Date_GetYear(dt), 3, 31);
  case 1:
    return GWEN_Date_fromGregorian(GWEN_Date_GetYear(dt), 6, 30);
  case 2:
    return GWEN_Date_fromGregorian(GWEN_Date_GetYear(dt), 9, 30);
  case 3:
    return GWEN_Date_fromGregorian(GWEN_Date_GetYear(dt), 12, 31);
  default:
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid quarter (%d)", m);
    break;
  }

  return NULL;
}



GWEN_DATE *GWEN_Date_GetThisHalfYearStart(const GWEN_DATE *dt)
{
  if (GWEN_Date_GetMonth(dt)<7)
    return GWEN_Date_fromGregorian(GWEN_Date_GetYear(dt), 1, 1);
  else
    return GWEN_Date_fromGregorian(GWEN_Date_GetYear(dt), 7, 1);
}



GWEN_DATE *GWEN_Date_GetThisHalfYearEnd(const GWEN_DATE *dt)
{
  if (GWEN_Date_GetMonth(dt)<7)
    return GWEN_Date_fromGregorian(GWEN_Date_GetYear(dt), 6, 30);
  else
    return GWEN_Date_fromGregorian(GWEN_Date_GetYear(dt), 12, 31);
}



GWEN_DATE *GWEN_Date_GetThisYearStart(const GWEN_DATE *dt)
{
  return GWEN_Date_fromGregorian(GWEN_Date_GetYear(dt), 1, 1);
}



GWEN_DATE *GWEN_Date_GetThisYearEnd(const GWEN_DATE *dt)
{
  return GWEN_Date_fromGregorian(GWEN_Date_GetYear(dt), 12, 31);
}



GWEN_DATE *GWEN_Date_GetLastMonthStart(const GWEN_DATE *dt)
{
  GWEN_DATE *tmpDate;
  GWEN_DATE *result;
  int j;

  tmpDate=GWEN_Date_fromGregorian(GWEN_Date_GetYear(dt), GWEN_Date_GetMonth(dt), 1);
  j=GWEN_Date_GetJulian(tmpDate)-1;
  GWEN_Date_free(tmpDate);
  tmpDate=GWEN_Date_fromJulian(j);
  result=GWEN_Date_fromGregorian(GWEN_Date_GetYear(tmpDate), GWEN_Date_GetMonth(tmpDate), 1);
  GWEN_Date_free(tmpDate);
  return result;
}



GWEN_DATE *GWEN_Date_GetLastMonthEnd(const GWEN_DATE *dt)
{
  GWEN_DATE *tmpDate;
  int j;

  tmpDate=GWEN_Date_fromGregorian(GWEN_Date_GetYear(dt), GWEN_Date_GetMonth(dt), 1);
  j=GWEN_Date_GetJulian(tmpDate)-1;
  GWEN_Date_free(tmpDate);
  return GWEN_Date_fromJulian(j);
}



GWEN_DATE *GWEN_Date_GetLastQuarterYearStart(const GWEN_DATE *dt)
{
  GWEN_DATE *tmpDate;
  GWEN_DATE *result;

  tmpDate=GWEN_Date_GetLastQuarterYearEnd(dt);
  result=GWEN_Date_GetThisQuarterYearStart(tmpDate);
  GWEN_Date_free(tmpDate);
  return result;
}



GWEN_DATE *GWEN_Date_GetLastQuarterYearEnd(const GWEN_DATE *dt)
{
  GWEN_DATE *tmpDate;
  int j;

  tmpDate=GWEN_Date_GetThisQuarterYearStart(dt);
  j=GWEN_Date_GetJulian(tmpDate)-1;
  GWEN_Date_free(tmpDate);
  return GWEN_Date_fromJulian(j);
}



GWEN_DATE *GWEN_Date_GetLastHalfYearStart(const GWEN_DATE *dt)
{
  GWEN_DATE *tmpDate;
  GWEN_DATE *result;

  tmpDate=GWEN_Date_GetLastHalfYearEnd(dt);
  result=GWEN_Date_GetThisHalfYearStart(tmpDate);
  GWEN_Date_free(tmpDate);
  return result;
}



GWEN_DATE *GWEN_Date_GetLastHalfYearEnd(const GWEN_DATE *dt)
{
  GWEN_DATE *tmpDate;
  int j;

  tmpDate=GWEN_Date_GetThisHalfYearStart(dt);
  j=GWEN_Date_GetJulian(tmpDate)-1;
  GWEN_Date_free(tmpDate);
  return GWEN_Date_fromJulian(j);
}



GWEN_DATE *GWEN_Date_GetLastYearStart(const GWEN_DATE *dt)
{
  GWEN_DATE *tmpDate;
  GWEN_DATE *result;

  tmpDate=GWEN_Date_GetLastYearEnd(dt);
  result=GWEN_Date_GetThisYearStart(tmpDate);
  GWEN_Date_free(tmpDate);
  return result;
}



GWEN_DATE *GWEN_Date_GetLastYearEnd(const GWEN_DATE *dt)
{
  GWEN_DATE *tmpDate;
  int j;

  tmpDate=GWEN_Date_GetThisYearStart(dt);
  j=GWEN_Date_GetJulian(tmpDate)-1;
  GWEN_Date_free(tmpDate);
  return GWEN_Date_fromJulian(j);
}








