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


#ifdef GWENHYWFAR_ENABLE_TESTCODE



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int GWENHYWFAR_CB test1(GWEN_TEST_MODULE *mod);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



int GWEN_Timestamp_AddTests(GWEN_TEST_MODULE *mod)
{
  GWEN_TEST_MODULE *newMod;

  newMod=GWEN_Test_Module_AddModule(mod, "GWEN_Timestamp", NULL);

  GWEN_Test_Module_AddTest(newMod, "test GWEN_Timestamp_toTimeT() and GWEN_Timestamp_fromLocalTime()", test1, NULL);

  return 0;
}



int test1(GWEN_UNUSED GWEN_TEST_MODULE *mod)
{
  GWEN_TIMESTAMP *ts1;
  time_t t1;
  const char *s1;
  GWEN_TIMESTAMP *ts2;
  time_t t2;
  const char *s2;

  ts1=GWEN_Timestamp_NowInLocalTime();
  t1=GWEN_Timestamp_toTimeT(ts1);
  ts2=GWEN_Timestamp_fromLocalTime(t1);
  t2=GWEN_Timestamp_toTimeT(ts2);

  s1=GWEN_Timestamp_GetString(ts1);
  s2=GWEN_Timestamp_GetString(ts2);

  if (strcasecmp(s1, s2)!=0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Times as string not equal (%s != %s)", s1, s2);
    return GWEN_ERROR_GENERIC;
  }

  if (t1!=t2) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Times in time_t not equal (%llu != %llu)",
              (unsigned long long int) t1,
              (unsigned long long int) t2);
    return GWEN_ERROR_GENERIC;
  }

  return 0;
}



#else

int GWEN_Timestamp_AddTests(GWEN_UNUSED GWEN_TEST_MODULE *mod)
{
  DBG_ERROR(GWEN_LOGDOMAIN, "Gwenhywfar was compiled without test code enabled.");
  return GWEN_ERROR_GENERIC;
}


#endif



