/***************************************************************************
    begin       : Mon Feb 10 2020
    copyright   : (C) 2020 by Martin Preuss
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


/* This file is included by "buffer.c" */


#include <gwenhywfar/testframework.h>
#include "buffer-t.h"


#ifdef GWENHYWFAR_ENABLE_TESTCODE



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int GWENHYWFAR_CB test1(GWEN_TEST_MODULE *mod);
static int GWENHYWFAR_CB test2(GWEN_TEST_MODULE *mod);
static int GWENHYWFAR_CB test3(GWEN_TEST_MODULE *mod);
static int GWENHYWFAR_CB test4(GWEN_TEST_MODULE *mod);
static int GWENHYWFAR_CB test5(GWEN_TEST_MODULE *mod);





/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */


int GWEN_Buffer_AddTests(GWEN_TEST_MODULE *mod)
{
  GWEN_TEST_MODULE *newMod;

  newMod=GWEN_Test_Module_AddModule(mod, "GWEN_Buffer", NULL);

  GWEN_Test_Module_AddTest(newMod, "append args (simple)", test1, NULL);
  GWEN_Test_Module_AddTest(newMod, "append args (long string)", test2, NULL);

  GWEN_Test_Module_AddTest(newMod, "cutout text between strings (onlyBetween=0)", test3, NULL);
  GWEN_Test_Module_AddTest(newMod, "cutout text between strings (onlyBetween=1)", test4, NULL);
  GWEN_Test_Module_AddTest(newMod, "cutout text between strings, add something behind", test5, NULL);

  return 0;
}



int test1(GWEN_UNUSED GWEN_TEST_MODULE *mod)
{
  GWEN_BUFFER *buf;
  int rv;

  buf=GWEN_Buffer_new(0, 16, 0, 1);
  rv=GWEN_Buffer_AppendArgs(buf, "%s-%d", "TESTSTRING", 1234);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not append args");
    GWEN_Buffer_free(buf);
    return GWEN_ERROR_GENERIC;
  }

  if (strcmp(GWEN_Buffer_GetStart(buf), "TESTSTRING-1234")!=0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected string in buffer (%s)", GWEN_Buffer_GetStart(buf));
    GWEN_Buffer_free(buf);
    return GWEN_ERROR_GENERIC;
  }
  GWEN_Buffer_free(buf);

  return 0;
}



int test2(GWEN_UNUSED GWEN_TEST_MODULE *mod)
{
  GWEN_BUFFER *buf;
  int rv;
  static const char *testString=
    "0123456789"
    "0123456789"
    "0123456789"
    "0123456789"
    "0123456789"
    "0123456789"
    "0123456789"
    "0123456789"
    "0123456789"
    "0123456789"
    "0123456789"
    "0123456789"
    "0123456789"
    "0123456789";

  buf=GWEN_Buffer_new(0, 16, 0, 1);
  rv=GWEN_Buffer_AppendArgs(buf, "%s", testString);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not append args");
    GWEN_Buffer_free(buf);
    return GWEN_ERROR_GENERIC;
  }

  if (strcmp(GWEN_Buffer_GetStart(buf), testString)!=0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected string in buffer (%s)", GWEN_Buffer_GetStart(buf));
    GWEN_Buffer_free(buf);
    return GWEN_ERROR_GENERIC;
  }
  GWEN_Buffer_free(buf);

  return 0;
}



int test3(GWEN_UNUSED GWEN_TEST_MODULE *mod)
{
  GWEN_BUFFER *buf;
  int rv;
  static const char *testString1=
    "0000000000"
    "<begin>"
    "1111111111"
    "<end>"
    "222222222";
  static const char *testString2=
    "<begin>"
    "1111111111"
    "<end>";

  buf=GWEN_Buffer_new(0, 16, 0, 1);
  rv=GWEN_Buffer_AppendString(buf, testString1);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not append string");
    GWEN_Buffer_free(buf);
    return GWEN_ERROR_GENERIC;
  }

  rv=GWEN_Buffer_KeepTextBetweenStrings(buf, "<begin>", "<end>", 0);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not cut data out (%d)", rv);
    GWEN_Buffer_free(buf);
    return GWEN_ERROR_GENERIC;
  }


  if (strcmp(GWEN_Buffer_GetStart(buf), testString2)!=0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected string in buffer (%s)", GWEN_Buffer_GetStart(buf));
    GWEN_Buffer_free(buf);
    return GWEN_ERROR_GENERIC;
  }
  GWEN_Buffer_free(buf);

  return 0;
}



int test4(GWEN_UNUSED GWEN_TEST_MODULE *mod)
{
  GWEN_BUFFER *buf;
  int rv;
  static const char *testString1=
    "0000000000"
    "<begin>"
    "1111111111"
    "<end>"
    "222222222";
  static const char *testString2=
    "1111111111";

  buf=GWEN_Buffer_new(0, 16, 0, 1);
  rv=GWEN_Buffer_AppendString(buf, testString1);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not append string");
    GWEN_Buffer_free(buf);
    return GWEN_ERROR_GENERIC;
  }

  rv=GWEN_Buffer_KeepTextBetweenStrings(buf, "<begin>", "<end>", 1);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not cut data out (%d)", rv);
    GWEN_Buffer_free(buf);
    return GWEN_ERROR_GENERIC;
  }

  if (strcmp(GWEN_Buffer_GetStart(buf), testString2)!=0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected string in buffer (%s)", GWEN_Buffer_GetStart(buf));
    GWEN_Buffer_free(buf);
    return GWEN_ERROR_GENERIC;
  }

  if (GWEN_Buffer_GetUsedBytes(buf)!=strlen(testString2)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid buffer size (%d)", GWEN_Buffer_GetUsedBytes(buf));
  }

  GWEN_Buffer_free(buf);

  return 0;
}



int test5(GWEN_UNUSED GWEN_TEST_MODULE *mod)
{
  GWEN_BUFFER *buf;
  int rv;
  static const char *testString1=
    "0000000000"
    "<begin>"
    "1111111111"
    "<end>"
    "222222222";
  static const char *testString2=
    "1111111111";
  static const char *testString3=
    "3333333333";
  static const char *testString4=
    "1111111111"
    "3333333333";

  buf=GWEN_Buffer_new(0, 16, 0, 1);
  rv=GWEN_Buffer_AppendString(buf, testString1);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not append string");
    GWEN_Buffer_free(buf);
    return GWEN_ERROR_GENERIC;
  }

  rv=GWEN_Buffer_KeepTextBetweenStrings(buf, "<begin>", "<end>", 1);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not cut data out (%d)", rv);
    GWEN_Buffer_free(buf);
    return GWEN_ERROR_GENERIC;
  }

  if (strcmp(GWEN_Buffer_GetStart(buf), testString2)!=0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected string in buffer (%s)", GWEN_Buffer_GetStart(buf));
    GWEN_Buffer_free(buf);
    return GWEN_ERROR_GENERIC;
  }

  if (GWEN_Buffer_GetUsedBytes(buf)!=strlen(testString2)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid buffer size (%d)", GWEN_Buffer_GetUsedBytes(buf));
  }

  rv=GWEN_Buffer_AppendString(buf, testString3);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not append 2nd string");
    GWEN_Buffer_free(buf);
    return GWEN_ERROR_GENERIC;
  }

  if (strcmp(GWEN_Buffer_GetStart(buf), testString4)!=0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected string in buffer (%s)", GWEN_Buffer_GetStart(buf));
    GWEN_Buffer_free(buf);
    return GWEN_ERROR_GENERIC;
  }

  GWEN_Buffer_free(buf);

  return 0;
}







#else

int GWEN_Buffer_AddTests(GWEN_UNUSED GWEN_TEST_MODULE *mod)
{
  DBG_ERROR(GWEN_LOGDOMAIN, "Gwenhywfar was compiled without test code enabled.");
  return GWEN_ERROR_GENERIC;
}


#endif

