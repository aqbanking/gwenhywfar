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


#include "gwentime_p.h"
#include <gwenhywfar/gwentime.h>
#include <gwenhywfar/debug.h>

#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>



GWEN_TIME *GWEN_CurrentTime(){
  GWEN_TIME *t;

  GWEN_NEW_OBJECT(GWEN_TIME, t);
  if (GWEN_Time__GetCurrentTime(t)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not get current time");
    GWEN_Time_free(t);
    return 0;
  }
  return t;
}



GWEN_TIME *GWEN_Time_fromSeconds(GWEN_TYPE_UINT32 secs) {
  GWEN_TIME *t;

  GWEN_NEW_OBJECT(GWEN_TIME, t);
  t->secs=secs;
  return t;
}



void GWEN_Time__SetSecsAndMSecs(GWEN_TIME *ti,
                                GWEN_TYPE_UINT32 secs,
                                GWEN_TYPE_UINT32 msecs){
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

  DBG_NOTICE(GWEN_LOGDOMAIN,
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

    if (isdigit(*p))
      i=(*p)-'0';
    else
      i=-1;

    switch(*t) {
    case 'Y':
      if (i==-1)
	return 0;
      year*=10;
      year+=i;
      break;
    case 'M':
      if (i==-1)
	return 0;
      month*=10;
      month+=i;
      break;
    case 'D':
      if (i==-1)
	return 0;
      day*=10;
      day+=i;
      break;
    case 'h':
      if (i==-1)
	return 0;
      hour*=10;
      hour+=i;
      break;
    case 'm':
      if (i==-1)
	return 0;
      min*=10;
      min+=i;
      break;
    case 's':
      if (i==-1)
	return 0;
      sec*=10;
      sec+=i;
      break;
    default:
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Unknown character in template, will skip in both strings");
      break;
    }
    t++;
    p++;
  } /* while */

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
  GWEN_TYPE_UINT32 s;

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
    s=(GWEN_TYPE_UINT32)tt;
  }
  return GWEN_Time_fromSeconds(s);
}



GWEN_TYPE_UINT32 GWEN_Time__mktimeUtc(int year,
                                      int month,
                                      int day,
                                      int hour,
                                      int min,
                                      int sec) {
  GWEN_TYPE_UINT32 result;
  int i;
  int isLeap;
  const GWEN_TYPE_UINT32 hoursecs=60*60;
  const GWEN_TYPE_UINT32 daysecs=24*hoursecs;
  const GWEN_TYPE_UINT32 yearsecs=365*daysecs;
  const GWEN_TYPE_UINT32 monthDays[12]=
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

  d=((t1->secs*1000)+(t1->msecs))-
    ((t0->secs*1000)+(t0->msecs));

  return d;
}



double GWEN_Time_Milliseconds(const GWEN_TIME *t){
  assert(t);
  return (double)((t->secs*1000)+(t->msecs));
}



GWEN_TYPE_UINT32 GWEN_Time_Seconds(const GWEN_TIME *t){
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



struct tm GWEN_Time_To_tm(const GWEN_TIME *t) {
  struct tm *tb;
  time_t tt;

  assert(t);
  tt=t->secs;
  tb=localtime(&tt);
  return *tb;
}

time_t GWEN_Time_To_time_t(const GWEN_TIME *t) {
  assert(t);
  return t->secs;
}

