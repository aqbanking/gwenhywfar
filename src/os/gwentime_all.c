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


#include <gwenhywfar/gwentime.h>
#include <gwenhywfar/debug.h>

#include <time.h>
#include <ctype.h>



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
    DBG_INFO(0, "Could not break down date");
    return -1;
  }
  GWEN_DB_SetIntValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "day", i1);
  GWEN_DB_SetIntValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "month", i2);
  GWEN_DB_SetIntValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "year", i3);

  dbT=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, "time");
  assert(dbT);
  if (GWEN_Time_GetBrokenDownUtcTime(t, &i1, &i2, &i3)) {
    DBG_INFO(0, "Could not break down time");
    return -1;
  }
  GWEN_DB_SetIntValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "hours", i1);
  GWEN_DB_SetIntValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "minutes", i2);
  GWEN_DB_SetIntValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "seconds", i3);

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

  inUtc=GWEN_DB_GetIntValue(db, "inUtc", 0, 1);
  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "date");
  if (dbT) {
    day=GWEN_DB_GetIntValue(dbT, "day", 0, 0);
    month=GWEN_DB_GetIntValue(dbT, "month", 0, 0);
    year=GWEN_DB_GetIntValue(dbT, "year", 0, 0);
    if (!day || !year) {
      DBG_INFO(0, "Bad date in DB");
      return 0;
    }
  }

  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "time");
  if (dbT) {
    hour=GWEN_DB_GetIntValue(dbT, "hour", 0, 0);
    min=GWEN_DB_GetIntValue(dbT, "min", 0, 0);
    sec=GWEN_DB_GetIntValue(dbT, "sec", 0, 0);
  }

  t=GWEN_Time_new(year, month, day, hour, min, sec, inUtc);
  if (!t) {
    DBG_INFO(0, "Bad date/time");
    return 0;
  }

  return t;
}



GWEN_TIME *GWEN_Time_fromString(const char *s, const char *tmpl){
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
      DBG_DEBUG(0,
		"Unknown character in template, will skip in both strings");
      break;
    }
    t++;
    p++;
  } /* while */

  /* get time in local time */
  gwt=GWEN_Time_new(year, month, day, hour, min, sec, 0);
  if (!gwt) {
    DBG_INFO(0, "here");
    return 0;
  }
  return gwt;
}

















