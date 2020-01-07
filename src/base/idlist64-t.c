/***************************************************************************
    begin       : Mon Mar 01 2004
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


/* This file is included by "simpleptrlist.c" */


#if GWENHYWFAR_ENABLE_TESTCODE



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */


static int test1(void);
static int test2(void);
#if 0
static int test3(void);
static int test4(void);
static int test5(void);
static int test6(void);
#endif



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



int GWEN_IdList64_Test(void)
{
  int rv;
  int numFailed=0;

  rv=test1();
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Test 1: failed (%d)", rv);
    numFailed++;
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Test 1: passed");
  }

  rv=test2();
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Test 2: failed (%d)", rv);
    numFailed++;
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Test 2: passed");
  }

#if 0
  rv=test3();
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Test 3: failed (%d)", rv);
    numFailed++;
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Test 3: passed");
  }

  rv=test4();
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Test 4: failed (%d)", rv);
    numFailed++;
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Test 4: passed");
  }

  rv=test5();
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Test 5: failed (%d)", rv);
    numFailed++;
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Test 5: passed");
  }

  rv=test6();
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Test 6: failed (%d)", rv);
    numFailed++;
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Test 6: passed");
  }
#endif

  if (numFailed)
    return GWEN_ERROR_GENERIC;
  return 0;
}



int test1(void)
{
  GWEN_IDLIST64 *idList1;
  GWEN_IDLIST64_ITERATOR *iterator1;
  int i;
  uint64_t id;

  idList1=GWEN_IdList64_new(64);
  for (i=0; i<100; i++) {
    int64_t rv;

    rv=GWEN_IdList64_AddId(idList1, i+1);
    if (rv<0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "here (%d)", (int) rv);
      GWEN_IdList64_free(idList1);
      return (int) rv;
    }
    if (rv!=(int64_t) i) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected position for id at index %u (is %lu, expected %u)",
		i, (unsigned long) rv, i);
      GWEN_IdList64_free(idList1);
      return (int) rv;
    }
  }

  i=0;
  iterator1=GWEN_IdList64_Iterator_new(idList1);
  id=GWEN_IdList64_Iterator_GetFirstId(iterator1);
  while(id) {
    if (id!=i+1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected id at index %lu (is %lu, expected %lu)",
                (unsigned long) i, (unsigned long) id, (unsigned long) i+1);
      GWEN_IdList64_Iterator_free(iterator1);
      GWEN_IdList64_free(idList1);
      return GWEN_ERROR_GENERIC;
    }
    i++;
    id=GWEN_IdList64_Iterator_GetNextId(iterator1);
  }
  GWEN_IdList64_Iterator_free(iterator1);
  GWEN_IdList64_free(idList1);
  return 0;
}



int test2(void)
{
  GWEN_IDLIST64 *idList1;
  GWEN_IDLIST64_ITERATOR *iterator1;
  int i;
  uint64_t id;
  int rv;

  idList1=GWEN_IdList64_new(64);
  for (i=0; i<100; i++) {
    int64_t rv;

    rv=GWEN_IdList64_AddId(idList1, i+1);
    if (rv<0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "here (%d)", (int) rv);
      GWEN_IdList64_free(idList1);
      return (int) rv;
    }
  }

  rv=GWEN_IdList64_ReverseSort(idList1);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "here (%d)", (int) rv);
    GWEN_IdList64_free(idList1);
    return (int) rv;
  }


  i=0;
  iterator1=GWEN_IdList64_Iterator_new(idList1);
  id=GWEN_IdList64_Iterator_GetFirstId(iterator1);
  while(id) {
    if (id!=100-i) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected id at index %lu (is %lu, expected %lu)",
                (unsigned long) i, (unsigned long) id, (unsigned long) i+1);
      GWEN_IdList64_Iterator_free(iterator1);
      GWEN_IdList64_free(idList1);
      return GWEN_ERROR_GENERIC;
    }
    i++;
    id=GWEN_IdList64_Iterator_GetNextId(iterator1);
  }
  if (i!=100) {
    DBG_ERROR(GWEN_LOGDOMAIN, "More entries than expected (is %d, expected %d)", i, 100);
    GWEN_IdList64_Iterator_free(iterator1);
    GWEN_IdList64_free(idList1);
    return GWEN_ERROR_GENERIC;
  }
  GWEN_IdList64_Iterator_free(iterator1);


  rv=GWEN_IdList64_Sort(idList1);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "here (%d)", (int) rv);
    GWEN_IdList64_free(idList1);
    return (int) rv;
  }

  i=0;
  iterator1=GWEN_IdList64_Iterator_new(idList1);
  id=GWEN_IdList64_Iterator_GetFirstId(iterator1);
  while(id) {
    if (id!=i+1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected id at index %lu (is %lu, expected %lu)",
                (unsigned long) i, (unsigned long) id, (unsigned long) i+1);
      GWEN_IdList64_Iterator_free(iterator1);
      GWEN_IdList64_free(idList1);
      return GWEN_ERROR_GENERIC;
    }
    i++;
    id=GWEN_IdList64_Iterator_GetNextId(iterator1);
  }
  if (i!=100) {
    DBG_ERROR(GWEN_LOGDOMAIN, "More entries than expected (is %d, expected %d)", i, 100);
    GWEN_IdList64_Iterator_free(iterator1);
    GWEN_IdList64_free(idList1);
    return GWEN_ERROR_GENERIC;
  }
  GWEN_IdList64_Iterator_free(iterator1);


  GWEN_IdList64_free(idList1);
  return 0;
}





#else

int GWEN_IdList64_Test(void)
{
  DBG_ERROR(GWEN_LOGDOMAIN, "Gwenhywfar was compiled without test code enabled.");
  return GWEN_ERROR_GENERIC;
}

#endif

