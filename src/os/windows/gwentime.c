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



/* The idea of this function is taken from a posting by Anders Carlsson on the mailing list
 * bug-gnu-chess (http://mail.gnu.org/archive/html/bug-gnu-chess/2004-01/msg00020.html)
*/
GWEN_TIME *GWEN_CurrentTime(){
  GWEN_TIME *t;
  union {
       GWEN_TYPE_UINT64 ns100; /*time since 1 Jan 1601 in 100ns units */
       FILETIME ft;
     } current_date;

  GetSystemTimeAsFileTime( &(current_date.ft));

  GWEN_NEW_OBJECT(GWEN_TIME, t);
  t->usec=(long)((current_date.ns100 / 10LL) % 1000000LL );
  t->sec=(long)((current_date.ns100-(116444736000000000LL))/10000000LL);
  return t;
}



GWEN_TIME *GWEN_Time_dup(const GWEN_TIME *t){
  GWEN_TIME *newT;

  assert(t);
  GWEN_NEW_OBJECT(GWEN_TIME, newT);
  newT->sec=t->sec;
  newT->usec=t->usec;
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

   d=((t1->sec*1000)+(t1->usec/1000))-
    ((t0->sec*1000)+(t0->usec/1000));

  return d;
}



double GWEN_Time_Milliseconds(const GWEN_TIME *t){
  assert(t);
  return (double)((t->sec*1000)+(t->usec/1000));
}



GWEN_TYPE_UINT32 GWEN_Time_Seconds(const GWEN_TIME *t){
  assert(t);
  return t->sec;
}



int GWEN_Time_GetBrokenDownTime(const GWEN_TIME *t,
                                int *hours,
                                int *mins,
                                int *secs){
  struct tm *tb;

  assert(t);
  tb=localtime(&(t->sec));
  if (!tb) {
    DBG_ERROR(0, "localtime(): %s", strerror(errno));
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

  assert(t);
  tb=gmtime(&(t->sec));
  if (!tb) {
    DBG_ERROR(0, "gmtime(): %s", strerror(errno));
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
  tb=localtime(&(t->sec));
  if (!tb) {
    DBG_ERROR(0, "localtime(): %s", strerror(errno));
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

  assert(t);
  tb=gmtime(&(t->sec));
  if (!tb) {
    DBG_ERROR(0, "gmtime(): %s", strerror(errno));
    return -1;
  }
  *days=tb->tm_mday;
  *month=tb->tm_mon;
  *year=tb->tm_year+1900;
  return 0;
}



GWENHYWFAR_API GWEN_TIME *GWEN_Time_fromSeconds(GWEN_TYPE_UINT32 s){
  GWEN_TIME *t;

  GWEN_NEW_OBJECT(GWEN_TIME, t);
  t->sec=s;
  t->usec=0;
  return t;
}



GWEN_TIME *GWEN_Time_new(int year,
                         int month,
                         int day,
                         int hour,
                         int min,
                         int sec,
                         int inUtc){
  int year, month, day;
  int hour, min, sec;
  GWEN_TIME *gwt;
  time_t tt;
  struct tm ti;
  struct tm *tp;

  tt=time(0);
  if (inUtc)
    tp=gmtime(&tt);
  else
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
  gwt->sec=tt;
  gwt->usec=0;
  return gwt;
}













