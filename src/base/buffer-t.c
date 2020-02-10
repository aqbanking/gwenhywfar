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


#if GWENHYWFAR_ENABLE_TESTCODE



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int GWENHYWFAR_CB test1(GWEN_TEST_MODULE *mod);
static int GWENHYWFAR_CB test2(GWEN_TEST_MODULE *mod);





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







#else

int GWEN_Buffer_AddTests(GWEN_TEST_MODULE *mod)
{
  DBG_ERROR(GWEN_LOGDOMAIN, "Gwenhywfar was compiled without test code enabled.");
  return GWEN_ERROR_GENERIC;
}


#endif

