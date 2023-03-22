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



GWEN_TIMESTAMP *GWEN_Timestamp_dup(const GWEN_TIMESTAMP *tstamp)
{
  if (tstamp)
    return GWEN_Timestamp_new(tstamp->year, tstamp->month, tstamp->day,
                              tstamp->hour, tstamp->minute, tstamp->second);
  return NULL;
}



void GWEN_Timestamp_free(GWEN_TIMESTAMP *tstamp)
{
  if (tstamp) {
    GWEN_FREE_OBJECT(tstamp);
  }
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
  tstamp->second=i%60;
  i/=60;
  tstamp->minute=i%60;
  i/=60;
  tstamp->hour=i%24;
  i/=24;

  GWEN_Timestamp_SetJulianDate(tstamp, i);

  return tstamp;
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
  GWEN_Timestamp_SetDate(tstamp, year, month, day);
  GWEN_Timestamp_SetTime(tstamp, hour, minute, second);
}



void GWEN_Timestamp_SetDate(GWEN_TIMESTAMP *tstamp, int year, int month, int day)
{
  tstamp->year=year;
  tstamp->month=month;
  tstamp->day=day;
  tstamp->julian=_calcJulian(year, month, day);
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
}



void GWEN_Timestamp_SetTime(GWEN_TIMESTAMP *tstamp, int hour, int minute, int second)
{
  tstamp->hour=hour;
  tstamp->minute=minute;
  tstamp->second=second;
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





int _calcJulian(int y, int m, int d)
{
  return (1461*(y+4800+(m-14)/12))/4+
    (367*(m-2-12*((m-14)/12)))/12-
    (3*((y+4900+(m-14)/12)/100))/4+
    d-32075;
}



/* include test code */
#include "timestamp-t.c"




