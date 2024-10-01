/***************************************************************************
    begin       : Wed Mar 22 2023
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

#include "./timestamp_p.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int _calcJulian(int y, int m, int d);
static void _writeAsString(GWEN_TIMESTAMP *tstamp);
static void _setDate(GWEN_TIMESTAMP *tstamp, int year, int month, int day);
static void _setTime(GWEN_TIMESTAMP *tstamp, int hour, int minute, int second);
static void _setFromInt64(GWEN_TIMESTAMP *tstamp, int64_t i);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */


GWEN_TIMESTAMP *GWEN_Timestamp_new(int year, int month, int day,
				   int hour, int minute, int second)
{
  GWEN_TIMESTAMP *tstamp=NULL;

  GWEN_NEW_OBJECT(GWEN_TIMESTAMP, tstamp);
  GWEN_Timestamp_SetDateAndTime(tstamp, year, month, day, hour, minute, second);
  return tstamp;
}



GWEN_TIMESTAMP *GWEN_Timestamp_dup(const GWEN_TIMESTAMP *tstampSrc)
{
  if (tstampSrc) {
    GWEN_TIMESTAMP *tstamp;

    GWEN_NEW_OBJECT(GWEN_TIMESTAMP, tstamp);
    memmove(tstamp, tstampSrc, sizeof(GWEN_TIMESTAMP));
    return tstamp;
  }
  return NULL;
}



void GWEN_Timestamp_free(GWEN_TIMESTAMP *tstamp)
{
  if (tstamp) {
    GWEN_FREE_OBJECT(tstamp);
  }
}



const char *GWEN_Timestamp_GetString(const GWEN_TIMESTAMP *tstamp)
{
  return tstamp->asString;
}



int64_t GWEN_Timestamp_toInt64(const GWEN_TIMESTAMP *tstamp)
{
  int64_t result;

  result=
    (tstamp->second)+
    ((int64_t)(tstamp->minute)*60)+
    ((int64_t)(tstamp->hour)*60*60)+
    ((int64_t)(tstamp->julian)*24*60*60);

  return result;
}



GWEN_TIMESTAMP *GWEN_Timestamp_fromInt64(int64_t i)
{
  GWEN_TIMESTAMP *tstamp=NULL;

  GWEN_NEW_OBJECT(GWEN_TIMESTAMP, tstamp);
  _setFromInt64(tstamp, i);
  return tstamp;
}



void _setFromInt64(GWEN_TIMESTAMP *tstamp, int64_t i)
{
  if (tstamp) {
    tstamp->second=i%60;
    i/=60;
    tstamp->minute=i%60;
    i/=60;
    tstamp->hour=i%24;
    i/=24;
    GWEN_Timestamp_SetJulianDate(tstamp, i);
  }
}



GWEN_TIMESTAMP *GWEN_Timestamp_fromStructTm(const struct tm *ltm)
{
  if (ltm) {
    GWEN_TIMESTAMP *tstamp;

    tstamp=GWEN_Timestamp_new(ltm->tm_year+1900, ltm->tm_mon+1, ltm->tm_mday,
                              ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
    return tstamp;
  }

  return NULL;
}



GWEN_TIMESTAMP *GWEN_Timestamp_fromLocalTime(time_t ti)
{
  return GWEN_Timestamp_fromStructTm(localtime(&ti));
}



GWEN_TIMESTAMP *GWEN_Timestamp_fromGmTime(time_t ti)
{
  return GWEN_Timestamp_fromStructTm(gmtime(&ti));
}



time_t GWEN_Timestamp_toTimeT(const GWEN_TIMESTAMP *tstamp)
{
  struct tm ti;
  struct tm *tp;
  time_t tt;

  tt=time(NULL);
  tp=localtime(&tt);
  assert(tp);
  memmove(&ti, tp, sizeof(ti));

  ti.tm_sec=tstamp->second;
  ti.tm_min=tstamp->minute;
  ti.tm_hour=tstamp->hour;

  ti.tm_year=tstamp->year-1900;
  ti.tm_mon=tstamp->month-1;
  ti.tm_mday=tstamp->day;
  ti.tm_yday=0;
  ti.tm_wday=0;
  tt=mktime(&ti);
  assert(tt!=(time_t)-1);
  return tt;
}



GWEN_TIMESTAMP *GWEN_Timestamp_NowInLocalTime()
{
  time_t ti;

  ti=time(NULL);
  return GWEN_Timestamp_fromStructTm(localtime(&ti));
}



GWEN_TIMESTAMP *GWEN_Timestamp_NowInGmTime()
{
  time_t ti;

  ti=time(NULL);
  return GWEN_Timestamp_fromStructTm(gmtime(&ti));
}



void GWEN_Timestamp_SetDateAndTime(GWEN_TIMESTAMP *tstamp,
				   int year, int month, int day,
				   int hour, int minute, int second)
{
  _setDate(tstamp, year, month, day);
  _setTime(tstamp, hour, minute, second);
  _writeAsString(tstamp);
}



void GWEN_Timestamp_SetDate(GWEN_TIMESTAMP *tstamp, int year, int month, int day)
{
  _setDate(tstamp, year, month, day);
  _writeAsString(tstamp);
}



void GWEN_Timestamp_SetJulianDate(GWEN_TIMESTAMP *tstamp, int julian)
{
  int l, n, i, j;

  l=julian+68569;
  n=(4*l)/146097;
  l=l-(146097*n+3)/4;
  i=(4000*(l+1))/1461001;
  l=l-(1461*i)/4+31;
  j=(80*l)/2447;

  tstamp->day=l-(2447*j)/80;
  l=j/11;
  tstamp->month=j+2-(12*l);
  tstamp->year=100*(n-49)+i+l;
  tstamp->julian=julian;

  _writeAsString(tstamp);
}



void GWEN_Timestamp_SetTime(GWEN_TIMESTAMP *tstamp, int hour, int minute, int second)
{
  _setTime(tstamp, hour, minute, second);
  _writeAsString(tstamp);
}



int GWEN_Timestamp_GetYear(const GWEN_TIMESTAMP *tstamp)
{
  return tstamp->year;
}



int GWEN_Timestamp_GetMonth(const GWEN_TIMESTAMP *tstamp)
{
  return tstamp->month;
}



int GWEN_Timestamp_GetDay(const GWEN_TIMESTAMP *tstamp)
{
  return tstamp->day;
}



int GWEN_Timestamp_GetHour(const GWEN_TIMESTAMP *tstamp)
{
  return tstamp->hour;
}



int GWEN_Timestamp_GetMinute(const GWEN_TIMESTAMP *tstamp)
{
  return tstamp->minute;
}



int GWEN_Timestamp_GetSecond(const GWEN_TIMESTAMP *tstamp)
{
  return tstamp->second;
}



int GWEN_Timestamp_Compare(const GWEN_TIMESTAMP *tstamp1, const GWEN_TIMESTAMP *tstamp0)
{
  if (tstamp0 && tstamp1) {
    int64_t v1, v0;

    v1=GWEN_Timestamp_toInt64(tstamp1);
    v0=GWEN_Timestamp_toInt64(tstamp0);
    if (v1==v0)
      return 0;
    else if (v1>v0)
      return 1;
    else
      return -1;

  }
  else if (tstamp0)
    return 1;
  else if (tstamp1)
    return -1;
  else
    return 0;
}




int _calcJulian(int y, int m, int d)
{
  return (1461*(y+4800+(m-14)/12))/4+
    (367*(m-2-12*((m-14)/12)))/12-
    (3*((y+4900+(m-14)/12)/100))/4+
    d-32075;
}



void _setDate(GWEN_TIMESTAMP *tstamp, int year, int month, int day)
{
  tstamp->year=year;
  tstamp->month=month;
  tstamp->day=day;
  tstamp->julian=_calcJulian(year, month, day);
}



void _setTime(GWEN_TIMESTAMP *tstamp, int hour, int minute, int second)
{
  tstamp->hour=hour;
  tstamp->minute=minute;
  tstamp->second=second;
}



void _writeAsString(GWEN_TIMESTAMP *tstamp)
{
  char *ptr;
  int x;

  ptr=tstamp->asString+14;
  *(ptr--)=0;

  x=tstamp->second;
  *(ptr--)='0'+(x%10);
  x/=10;
  *(ptr--)='0'+(x%10);

  x=tstamp->minute;
  *(ptr--)='0'+(x%10);
  x/=10;
  *(ptr--)='0'+(x%10);

  x=tstamp->hour;
  *(ptr--)='0'+(x%10);
  x/=10;
  *(ptr--)='0'+(x%10);

  x=tstamp->day;
  *(ptr--)='0'+(x%10);
  x/=10;
  *(ptr--)='0'+(x%10);

  x=tstamp->month;
  *(ptr--)='0'+(x%10);
  x/=10;
  *(ptr--)='0'+(x%10);

  x=tstamp->year;
  *(ptr--)='0'+(x%10);
  x/=10;
  *(ptr--)='0'+(x%10);
  x/=10;
  *(ptr--)='0'+(x%10);
  x/=10;
  *ptr='0'+(x%10);
}



GWEN_TIMESTAMP *GWEN_Timestamp_fromString(const char *s)
{
  if (s && strlen(s)>=14) {
    int year, month, day, hour, min, sec;
    GWEN_TIMESTAMP *result;
    const char *originalPtr;

    originalPtr=s;
    year=*(s++)-'0';
    year*=10;
    year+=*(s++)-'0';
    year*=10;
    year+=*(s++)-'0';
    year*=10;
    year+=*(s++)-'0';

    month=*(s++)-'0';
    month*=10;
    month+=*(s++)-'0';

    day=*(s++)-'0';
    day*=10;
    day+=*(s++)-'0';

    hour=*(s++)-'0';
    hour*=10;
    hour+=*(s++)-'0';

    min=*(s++)-'0';
    min*=10;
    min+=*(s++)-'0';

    sec=*(s++)-'0';
    sec*=10;
    sec+=*(s++)-'0';

    result=GWEN_Timestamp_new(year, month, day, hour, min, sec);
    if (!result) {
      DBG_INFO(GWEN_LOGDOMAIN, "Bad timestamp string [%s]", originalPtr);
    }
    return result;
  }
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "Bad timestamp string [%s]", s?s:"<empty>");
    return NULL;
  }
}



int GWEN_Timestamp_toDb(const GWEN_TIMESTAMP *tstamp, GWEN_DB_NODE *db)
{
  const char *s;

  s=GWEN_Timestamp_GetString(tstamp);
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "timestamp", s);
  return 0;
}



GWEN_TIMESTAMP *GWEN_Timestamp_fromDb(GWEN_DB_NODE *db)
{
  const char *s;

  s=GWEN_DB_GetCharValue(db, "timestamp", 0, NULL);
  if (s && *s) {
    GWEN_TIMESTAMP *tstamp;

    tstamp=GWEN_Timestamp_fromString(s);
    if (tstamp==NULL) {
      DBG_INFO(GWEN_LOGDOMAIN, "Invalid timestamp [%s]", s);
      return NULL;
    }
    return tstamp;
  }
  else {
    DBG_VERBOUS(GWEN_LOGDOMAIN, "no or empty timestamp");
    return NULL;
  }

}


void GWEN_Timestamp_AddSeconds(GWEN_TIMESTAMP *tstamp, int seconds)
{
  if (tstamp && seconds!=0) {
    int64_t ti;

    ti=GWEN_Timestamp_toInt64(tstamp)+seconds;
    _setFromInt64(tstamp, ti);
  }
}





/* include test code */
#include "timestamp-t.c"




