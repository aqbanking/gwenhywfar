/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sun Nov 23 2003
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


#include "gwentime_p.h"
#include <gwenhywfar/gwentime.h>
#include <gwenhywfar/debug.h>

#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>


GWEN_LIST_FUNCTIONS(GWEN_TIME_TMPLCHAR, GWEN_TimeTmplChar)



GWEN_TIME *GWEN_CurrentTime(void){
  GWEN_TIME *t;

  GWEN_NEW_OBJECT(GWEN_TIME, t);
  if (GWEN_Time__GetCurrentTime(t)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not get current time");
    GWEN_Time_free(t);
    return 0;
  }
  return t;
}



GWEN_TIME *GWEN_Time_fromSeconds(uint32_t secs) {
  GWEN_TIME *t;

  GWEN_NEW_OBJECT(GWEN_TIME, t);
  t->secs=secs;
  return t;
}



int GWEN_Time_AddSeconds(GWEN_TIME *ti,
			 uint32_t secs) {
  uint32_t i;

  assert(ti);
  i=ti->secs+secs;
  if (i<ti->secs) {
    DBG_INFO(GWEN_LOGDOMAIN,
	     "Overflow when adding %u seconds", secs);
    return GWEN_ERROR_INVALID;
  }
  ti->secs=i;
  return 0;
}



int GWEN_Time_SubSeconds(GWEN_TIME *ti,
			 uint32_t secs) {
  assert(ti);

  if (ti->secs<secs) {
    DBG_INFO(GWEN_LOGDOMAIN,
	     "Underflow when subtracting %u seconds",
	     secs);
    return GWEN_ERROR_INVALID;
  }
  ti->secs-=secs;
  return 0;
}


void GWEN_Time__SetSecsAndMSecs(GWEN_TIME *ti,
                                uint32_t secs,
                                uint32_t msecs){
  assert(ti);
  ti->secs=secs;
  ti->msecs=msecs;
}



int GWEN_Time_toDb(const GWEN_TIME *t, GWEN_DB_NODE *db) {
  GWEN_DB_NODE *dbT;
  int i1, i2, i3;

  assert(t);
  assert(db);
  dbT=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, "date");
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "inUtc", 1);

  assert(dbT);
  if (GWEN_Time_GetBrokenDownUtcDate(t, &i1, &i2, &i3)) {
    DBG_INFO(GWEN_LOGDOMAIN, "Could not break down date");
    return -1;
  }
  GWEN_DB_SetIntValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "day", i1);
  GWEN_DB_SetIntValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "month", i2+1);
  GWEN_DB_SetIntValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "year", i3);

  dbT=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, "time");
  assert(dbT);
  if (GWEN_Time_GetBrokenDownUtcTime(t, &i1, &i2, &i3)) {
    DBG_INFO(GWEN_LOGDOMAIN, "Could not break down time");
    return -1;
  }
  GWEN_DB_SetIntValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "hour", i1);
  GWEN_DB_SetIntValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "min", i2);
  GWEN_DB_SetIntValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "sec", i3);

  return 0;
}



GWEN_TIME *GWEN_Time_fromDb(GWEN_DB_NODE *db) {
  GWEN_TIME *t;
  GWEN_DB_NODE *dbT;
  int day, month, year;
  int hour, min, sec;
  int inUtc;

  day=month=year=0;
  hour=min=sec=0;

  inUtc=GWEN_DB_GetIntValue(db, "inUtc", 0, 0);
  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "date");
  if (dbT) {
    day=GWEN_DB_GetIntValue(dbT, "day", 0, 0);
    month=GWEN_DB_GetIntValue(dbT, "month", 0, 1)-1;
    year=GWEN_DB_GetIntValue(dbT, "year", 0, 0);
    if (!day || !year) {
      DBG_INFO(GWEN_LOGDOMAIN, "Bad date in DB");
      return 0;
    }
  }

  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "time");
  if (dbT) {
    hour=GWEN_DB_GetIntValue(dbT, "hour", 0, 0);
    min=GWEN_DB_GetIntValue(dbT, "min", 0, 0);
    sec=GWEN_DB_GetIntValue(dbT, "sec", 0, 0);
  }

  DBG_VERBOUS(GWEN_LOGDOMAIN,
              "Creating time from this: %04d/%02d/%02d - %02d:%02d:%02d (%d)",
              year, month, day, hour, min, sec, inUtc);
  t=GWEN_Time_new(year, month, day, hour, min, sec, inUtc);
  if (!t) {
    DBG_INFO(GWEN_LOGDOMAIN, "Bad date/time");
    return 0;
  }

  return t;
}



GWEN_TIME *GWEN_Time__fromString(const char *s, const char *tmpl, int inUtc){
  int year, month, day;
  int hour, min, sec;
  const char *p;
  const char *t;
  GWEN_TIME *gwt;

  assert(s);
  assert(tmpl);
  year=month=day=0;
  hour=min=sec=0;

  p=s;
  t=tmpl;
  while(*t && *p) {
    int i;

    if (*t=='*') {
      t++;
      if (!*t) {
	DBG_ERROR(GWEN_LOGDOMAIN, "Bad pattern: Must not end with \"*\"");
        return 0;
      }
      i=0;
      while(*p) {
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

    if (i==-1 && strchr("YMDhms", *t)!=NULL) {
      DBG_INFO(GWEN_LOGDOMAIN,
	       "No more digits at [%s], continueing", t);
      p--;
    }
    else {
      switch(*t) {
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
      case 'h':
	if (i==-1) {
          DBG_INFO(GWEN_LOGDOMAIN, "here");
	  return 0;
	}
	hour*=10;
	hour+=i;
	break;
      case 'm':
	if (i==-1) {
          DBG_INFO(GWEN_LOGDOMAIN, "here");
	  return 0;
	}
	min*=10;
	min+=i;
	break;
      case 's':
	if (i==-1) {
          DBG_INFO(GWEN_LOGDOMAIN, "here");
	  return 0;
	}
	sec*=10;
	sec+=i;
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
  if (day==0)
    day=1;

  DBG_DEBUG(GWEN_LOGDOMAIN,
	    "Got this date/time: %04d/%02d/%02d, %02d:%02d:%02d",
	    year, month-1, day, hour, min, sec);

  /* get time in local time */
  gwt=GWEN_Time_new(year, month-1, day, hour, min, sec, inUtc);
  if (!gwt) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    return 0;
  }
  return gwt;
}



GWEN_TIME *GWEN_Time_fromString(const char *s, const char *tmpl){
  return GWEN_Time__fromString(s, tmpl, 0);
}



GWEN_TIME *GWEN_Time_fromUtcString(const char *s, const char *tmpl){
  return GWEN_Time__fromString(s, tmpl, 1);
}



GWEN_TIME *GWEN_Time_new(int year,
                         int month,
                         int day,
                         int hour,
                         int min,
                         int sec,
                         int inUtc){
  uint32_t s;

  if (inUtc)
    s=GWEN_Time__mktimeUtc(year, month, day, hour, min, sec);
  else {
    struct tm ti;
    struct tm *tp;
    time_t tt;

    tt=time(0);
    tp=localtime(&tt);
    assert(tp);
    memmove(&ti, tp, sizeof(ti));
    ti.tm_sec=sec;
    ti.tm_min=min;
    ti.tm_hour=hour;
    if (year<100) {
      if (year<72)
        year+=2000;
      year+=1900;
    }
    ti.tm_year=year-1900;
    ti.tm_mon=month;
    ti.tm_mday=day;
    ti.tm_yday=0;
    ti.tm_wday=0;
    tt=mktime(&ti);
    assert(tt!=(time_t)-1);
    s=(uint32_t)tt;
  }
  return GWEN_Time_fromSeconds(s);
}



uint32_t GWEN_Time__mktimeUtc(int year,
                                      int month,
                                      int day,
                                      int hour,
                                      int min,
                                      int sec) {
  uint32_t result;
  int i;
  int isLeap;
  const uint32_t hoursecs=60*60;
  const uint32_t daysecs=24*hoursecs;
  const uint32_t yearsecs=365*daysecs;
  const uint32_t monthDays[12]=
    {
      31, 28, 31, 30,
      31, 30, 31, 31,
      30, 31, 30, 31
    };

  result=(year-1970)*yearsecs;

  for (i=1970; i<year; i++)
    if ((((i % 4)==0) &&
          ((i % 100)!=0)) ||
        ((i % 400)==0))
      result+=daysecs;

  isLeap=((((year % 4)==0) &&
           ((year % 100)!=0)) ||
          ((year % 400)==0));

  for (i=0; i<month; i++)
    if (isLeap && i==1)
      result+=29*daysecs;
    else
      result+=monthDays[i]*daysecs;

  result+=(day-1)*daysecs;
  result+=(hour*hoursecs);
  result+=min*60;
  result+=sec;

  return result;
}



GWEN_TIME *GWEN_Time_dup(const GWEN_TIME *t){
  GWEN_TIME *newT;

  assert(t);
  GWEN_NEW_OBJECT(GWEN_TIME, newT);
  newT->secs=t->secs;
  newT->msecs=t->msecs;
  return newT;
}



void GWEN_Time_free(GWEN_TIME *t){
  if (t) {
    GWEN_FREE_OBJECT(t);
  }
}



double GWEN_Time_Diff(const GWEN_TIME *t1, const GWEN_TIME *t0){
  double d;

  assert(t1);
  assert(t0);

  d=1000.0*((double)(t1->secs)-(double)(t0->secs));
  d+=(double)((double)(t1->msecs)-(double)(t0->msecs));

  return d;
}



double GWEN_Time_DiffSeconds(const GWEN_TIME *t1, const GWEN_TIME *t0){
  double d;

  assert(t1);
  assert(t0);

  d=(double)(t1->secs)-(double)(t0->secs);
  d+=((double)((double)(t1->msecs)-(double)(t0->msecs)))/1000.0;

  return d;
}



int GWEN_Time_Compare(const GWEN_TIME *t1, const GWEN_TIME *t0){
  if (t1 && t0) {
    if (t1->secs<t0->secs)
      return -1;
    else if (t1->secs>t0->secs)
      return 1;
    else {
      if (t1->msecs<t0->msecs)
	return -1;
      else if (t1->msecs>t0->msecs)
	return 1;
      else
	return 0;
    }
  }
  else if (t1)
    return 1;
  else if (t0)
    return -1;

  return 0;
}



double GWEN_Time_Milliseconds(const GWEN_TIME *t){
  assert(t);
  return (double)((t->secs*1000)+(t->msecs));
}



uint32_t GWEN_Time_Seconds(const GWEN_TIME *t){
  assert(t);
  return t->secs;
}



int GWEN_Time_GetBrokenDownTime(const GWEN_TIME *t,
                                int *hours,
                                int *mins,
                                int *secs){
  struct tm *tb;
  time_t tt;

  assert(t);
  tt=t->secs;
  tb=localtime(&tt);
  if (!tb) {
    DBG_ERROR(GWEN_LOGDOMAIN, "localtime(): %s", strerror(errno));
    return -1;
  }
  *hours=tb->tm_hour;
  *mins=tb->tm_min;
  *secs=tb->tm_sec;
  return 0;
}



int GWEN_Time_GetBrokenDownUtcTime(const GWEN_TIME *t,
                                   int *hours,
                                   int *mins,
                                   int *secs){
  struct tm *tb;
  time_t tt;

  assert(t);
  tt=t->secs;
  tb=gmtime(&tt);
  if (!tb) {
    DBG_ERROR(GWEN_LOGDOMAIN, "gmtime(): %s", strerror(errno));
    return -1;
  }
  *hours=tb->tm_hour;
  *mins=tb->tm_min;
  *secs=tb->tm_sec;
  return 0;
}



int GWEN_Time_GetBrokenDownDate(const GWEN_TIME *t,
                                int *days,
                                int *month,
                                int *year){
  struct tm *tb;
  time_t tt;

  assert(t);
  tt=t->secs;
  tb=localtime(&tt);
  if (!tb) {
    DBG_ERROR(GWEN_LOGDOMAIN, "localtime(): %s", strerror(errno));
    return -1;
  }
  *days=tb->tm_mday;
  *month=tb->tm_mon;
  *year=tb->tm_year+1900;
  return 0;
}



int GWEN_Time_GetBrokenDownUtcDate(const GWEN_TIME *t,
                                   int *days,
                                   int *month,
                                   int *year){
  struct tm *tb;
  time_t tt;

  assert(t);
  tt=t->secs;
  tb=gmtime(&tt);
  if (!tb) {
    DBG_ERROR(GWEN_LOGDOMAIN, "gmtime(): %s", strerror(errno));
    return -1;
  }
  *days=tb->tm_mday;
  *month=tb->tm_mon;
  *year=tb->tm_year+1900;
  return 0;
}



/* TODO: compiler says "function returns an aggregate" */
struct tm GWEN_Time_toTm(const GWEN_TIME *t) {
  struct tm *tb;
  time_t tt;

  assert(t);
  tt=t->secs;
  tb=localtime(&tt);
  return *tb;
}

time_t GWEN_Time_toTime_t(const GWEN_TIME *t) {
  assert(t);
  return t->secs;
}




GWEN_TIME_TMPLCHAR *GWEN_TimeTmplChar_new(char c) {
  GWEN_TIME_TMPLCHAR *e;

  GWEN_NEW_OBJECT(GWEN_TIME_TMPLCHAR, e);
  GWEN_LIST_INIT(GWEN_TIME_TMPLCHAR, e);
  e->character=c;
  return e;
}



void GWEN_TimeTmplChar_free(GWEN_TIME_TMPLCHAR *e) {
  if (e) {
    free(e->content);
    GWEN_LIST_FINI(GWEN_TIME_TMPLCHAR, e);
    GWEN_FREE_OBJECT(e);
  }
}


GWEN_TIME_TMPLCHAR *GWEN_Time__findTmplChar(GWEN_TIME_TMPLCHAR_LIST *ll,
                                            char c) {
  GWEN_TIME_TMPLCHAR *e;

  e=GWEN_TimeTmplChar_List_First(ll);
  while(e) {
    if (e->character==c)
      break;
    e=GWEN_TimeTmplChar_List_Next(e);
  }

  return e;
}




void GWEN_Time__sampleTmplChars(GWEN_UNUSED const GWEN_TIME *t, const char *tmpl,
                                GWEN_UNUSED GWEN_BUFFER *buf,
                                GWEN_TIME_TMPLCHAR_LIST *ll) {
  const char *s;

  s=tmpl;
  while(*s) {
    if (strchr("YMDhms", *s)) {
      GWEN_TIME_TMPLCHAR *e;

      e=GWEN_Time__findTmplChar(ll, *s);
      if (!e) {
        /* new entry, create it */
        e=GWEN_TimeTmplChar_new(*s);
        GWEN_TimeTmplChar_List_Add(e, ll);
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



void GWEN_Time__fillTmplChars(const GWEN_TIME *t,
                              GWEN_TIME_TMPLCHAR_LIST *ll,
                              int useUtc) {
  GWEN_TIME_TMPLCHAR *e;
  int year, month, day, hour, minute, second;

  if (useUtc) {
    GWEN_Time_GetBrokenDownUtcDate(t, &day, &month, &year);
    GWEN_Time_GetBrokenDownUtcTime(t, &hour, &minute, &second);
  }
  else {
    GWEN_Time_GetBrokenDownDate(t, &day, &month, &year);
    GWEN_Time_GetBrokenDownTime(t, &hour, &minute, &second);
  }

  e=GWEN_TimeTmplChar_List_First(ll);
  while(e) {
    int v;
    char buffer[32];

    switch(e->character) {
    case 'Y': v=year; break;
    case 'M': v=month+1; break;
    case 'D': v=day; break;
    case 'h': v=hour; break;
    case 'm': v=minute; break;
    case 's': v=second; break;
    default:  v=-1; break;
    }
    if (v==-1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unknown character, should not happen here");
      abort();
    }
    buffer[0]=0;
    snprintf(buffer, sizeof(buffer)-1, "%0*d", GWEN_TIME_TMPL_MAX_COUNT, v);
    buffer[sizeof(buffer)-1]=0;
    e->content=strdup(buffer);
    e->nextChar=strlen(e->content)-(e->count);
    e=GWEN_TimeTmplChar_List_Next(e);
  }
}




int GWEN_Time__toString(const GWEN_TIME *t, const char *tmpl,
                        GWEN_BUFFER *buf, int useUtc) {
  GWEN_TIME_TMPLCHAR_LIST *ll;
  const char *s;

  ll=GWEN_TimeTmplChar_List_new();
  GWEN_Time__sampleTmplChars(t, tmpl, buf, ll);
  GWEN_Time__fillTmplChars(t, ll, useUtc);

  s=tmpl;
  while(*s) {
    if (strchr("YMDhms", *s)) {
      GWEN_TIME_TMPLCHAR *e;
      char c;

      e=GWEN_Time__findTmplChar(ll, *s);
      assert(e);
      assert(e->content);
      if (s[1]=='*') {
        /* append full string */
        GWEN_Buffer_AppendString(buf, e->content);
        /* skip asterisk */
        s++;
      }
      else {
        c=e->content[e->nextChar++];
        assert(c);
        GWEN_Buffer_AppendByte(buf, c);
      }
    }
    else
      GWEN_Buffer_AppendByte(buf, *s);
    s++;
  }
  GWEN_TimeTmplChar_List_free(ll);
  return 0;
}



int GWEN_Time_toString(const GWEN_TIME *t, const char *tmpl,
                       GWEN_BUFFER *buf) {
  return GWEN_Time__toString(t, tmpl, buf, 0);
}



int GWEN_Time_toUtcString(const GWEN_TIME *t, const char *tmpl,
                          GWEN_BUFFER *buf) {
  return GWEN_Time__toString(t, tmpl, buf, 1);
}










