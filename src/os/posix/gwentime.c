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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "gwentime_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>

#include <time.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>


GWEN_TIME *GWEN_CurrentTime(){
  GWEN_TIME *t;
  struct timezone tz;

  GWEN_NEW_OBJECT(GWEN_TIME, t);
  if (gettimeofday(&(t->tv), &tz)) {
    DBG_ERROR(0, "Could not get current time");
    GWEN_FREE_OBJECT(t);
    return 0;
  }

  return t;
}



GWEN_TIME *GWEN_Time_dup(const GWEN_TIME *t){
  GWEN_TIME *newT;

  assert(t);
  GWEN_NEW_OBJECT(GWEN_TIME, newT);
  memmove(&newT->tv, &t->tv, sizeof(newT->tv));
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

  d=((t1->tv.tv_sec*1000)+(t1->tv.tv_usec/1000))-
    ((t0->tv.tv_sec*1000)+(t0->tv.tv_usec/1000));

  return d;
}



double GWEN_Time_Milliseconds(const GWEN_TIME *t){
  assert(t);
  return (double)((t->tv.tv_sec*1000)+(t->tv.tv_usec/1000));
}



GWEN_TYPE_UINT32 GWEN_Time_Seconds(const GWEN_TIME *t){
  assert(t);
  return t->tv.tv_sec;
}



int GWEN_Time_GetBrokenDownTime(const GWEN_TIME *t,
                                int *hours,
                                int *mins,
                                int *secs){
  struct tm *tb;

  assert(t);
  tb=localtime(&(t->tv.tv_sec));
  if (!tb) {
    DBG_ERROR(0, "localtime(): %s", strerror(errno));
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

  assert(t);
  tb=localtime(&(t->tv.tv_sec));
  if (!tb) {
    DBG_ERROR(0, "localtime(): %s", strerror(errno));
    return -1;
  }
  *days=tb->tm_mday;
  *month=tb->tm_mon;
  *year=tb->tm_year+1900;
  return 0;
}



GWEN_TIME *GWEN_Time_fromString(const char *s, const char *tmpl){
  int year, month, day;
  int hour, min, sec;
  const char *p;
  const char *t;
  GWEN_TIME *gwt;
  time_t tt;
  struct tm ti;
  struct tm *tp;

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
      DBG_DEBUG(0,
		"Unknown character in template, will skip in both strings");
      break;
    }
    t++;
    p++;
  } /* while */

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
  ti.tm_mon=month-1;
  ti.tm_mday=day;
  ti.tm_yday=0;
  ti.tm_wday=0;
  tt=mktime(&ti);
  assert(tt!=-1);

  GWEN_NEW_OBJECT(GWEN_TIME, gwt);
  gwt->tv.tv_sec=tt;
  gwt->tv.tv_usec=0;
  return gwt;
}



GWENHYWFAR_API GWEN_TIME *GWEN_Time_fromSeconds(GWEN_TYPE_UINT32 s){
  GWEN_TIME *t;

  GWEN_NEW_OBJECT(GWEN_TIME, t);
  t->tv.tv_sec=s;
  t->tv.tv_usec=0;
  return t;
}









