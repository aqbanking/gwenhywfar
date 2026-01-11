/***************************************************************************
 begin       : Sat Jan 10 2026
 copyright   : (C) 2026 by Martin Preuss
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

/* This file is included by "quicksort.c" */

#include "quicksort-t.h"
#include <gwenhywfar/error.h>



#ifdef GWENHYWFAR_ENABLE_TESTCODE

/* ------------------------------------------------------------------------------------------------
 * definitions
 * ------------------------------------------------------------------------------------------------
 */

#define TEST1_ARRAYSIZE 16



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int GWENHYWFAR_CB test1(GWEN_TEST_MODULE *mod);
static int _compareInts_cb(const void *pA, const void *pB, void *pArg);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */

int GWEN_Quicksort_AddTests(GWEN_TEST_MODULE *mod)
{
  GWEN_TEST_MODULE *newMod;

  newMod=GWEN_Test_Module_AddModule(mod, "GWEN_Quicksort", NULL);

  GWEN_Test_Module_AddTest(newMod, "sort integer array", test1, NULL);

  return 0;
}




/* ................................................................................................
 * test 1: sort an array of numbers
 * ................................................................................................
 */

int test1(GWEN_UNUSED GWEN_TEST_MODULE *mod)
{
  int currentArrayOfInts[TEST1_ARRAYSIZE] ={64, 84,  3, 95, 45,  3, 65, 46, 45, 12,  7, 39, 23, 54, 73, 99};
  const int expectedArrayOfInts[TEST1_ARRAYSIZE]={ 3,  3,  7, 12, 23, 39, 45, 45, 46, 54, 64, 65, 73, 84, 95, 99};
  int n;
  int argUseCount=0;
  int i;
  int errors=0;

  n=TEST1_ARRAYSIZE;

  GWEN_QuickSort(currentArrayOfInts, n, sizeof(int), _compareInts_cb, (void*) &argUseCount);

  if (argUseCount==0) {
    fprintf(stderr, "Compare function never called.\n");
    errors++;
  }

#if 0
  /* print array */
  fprintf(stderr, "Array: ");
  for (i=0; i<n; i++)
    fprintf(stderr, "%s%d", i?", ":"", currentArrayOfInts[i]);
  fprintf(stderr, "\n");
#endif

  for (i=0; i<n; i++) {
    if (currentArrayOfInts[i]!=expectedArrayOfInts[i]) {
      fprintf(stderr, "Sorted array differes at pos %d: %d != %d\n", i, currentArrayOfInts[i], expectedArrayOfInts[i]);
      errors++;
    }
  }

  if (errors) {
    fprintf(stderr, "At least some parts of the test failed.\n");
    return GWEN_ERROR_GENERIC;
  }

  return 0;
}



int _compareInts_cb(const void *pA, const void *pB, void *pArg)
{
  const int *pIntA;
  const int *pIntB;
  int *pIntArg;

  pIntArg=(int*) pArg;
  (*pIntArg)++;
  pIntA=(const int*) pA;
  pIntB=(const int*) pB;

  if (*pIntA < *pIntB)
    return -1;
  else if (*pIntA > *pIntB)
    return 1;
  else
    return 0;
}



#else

#include "debug.h"

int GWEN_Quicksort_AddTests(GWEN_TEST_MODULE *mod)
{
  DBG_ERROR(GWEN_LOGDOMAIN, "Gwenhywfar was compiled without test code enabled.");
  return GWEN_ERROR_GENERIC;
}



#endif
