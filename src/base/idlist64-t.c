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


/* This file is included by "idlist64.c" */

#include "idlist64-t.h"


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
static int GWENHYWFAR_CB test6(GWEN_TEST_MODULE *mod);
static int GWENHYWFAR_CB test7(GWEN_TEST_MODULE *mod);
static int GWENHYWFAR_CB test8(GWEN_TEST_MODULE *mod);
static int GWENHYWFAR_CB test9(GWEN_TEST_MODULE *mod);
static int GWENHYWFAR_CB test10(GWEN_TEST_MODULE *mod);
static int GWENHYWFAR_CB test11(GWEN_TEST_MODULE *mod);

static int _compareList1AgainstList2(GWEN_IDLIST64 *idList1, GWEN_IDLIST64 *idList2);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



int GWEN_IdList64_AddTests(GWEN_TEST_MODULE *mod)
{
  GWEN_TEST_MODULE *newMod;

  newMod=GWEN_Test_Module_AddModule(mod, "GWEN_IdList64", NULL);

  GWEN_Test_Module_AddTest(newMod, "test1", test1, NULL);
  GWEN_Test_Module_AddTest(newMod, "test2", test2, NULL);
  GWEN_Test_Module_AddTest(newMod, "test3", test3, NULL);
  GWEN_Test_Module_AddTest(newMod, "test4", test4, NULL);
  GWEN_Test_Module_AddTest(newMod, "test5", test5, NULL);
  GWEN_Test_Module_AddTest(newMod, "test6", test6, NULL);
  GWEN_Test_Module_AddTest(newMod, "test7", test7, NULL);
  GWEN_Test_Module_AddTest(newMod, "test8", test8, NULL);
  GWEN_Test_Module_AddTest(newMod, "test9", test9, NULL);
  GWEN_Test_Module_AddTest(newMod, "test10", test10, NULL);
  GWEN_Test_Module_AddTest(newMod, "test11", test11, NULL);

  return 0;
}



int test1(GWEN_UNUSED GWEN_TEST_MODULE *mod)
{
  GWEN_IDLIST64 *idList1;
  GWEN_IDLIST64_ITERATOR *iterator1;
  int i;
  uint64_t id;

  idList1=GWEN_IdList64_newWithSteps(64);
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
  while (id) {
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



int test2(GWEN_UNUSED GWEN_TEST_MODULE *mod)
{
  GWEN_IDLIST64 *idList1;
  GWEN_IDLIST64_ITERATOR *iterator1;
  int i;
  uint64_t id;
  int rv;

  idList1=GWEN_IdList64_newWithSteps(64);
  for (i=0; i<100; i++) {
    int64_t rv64;

    rv64=GWEN_IdList64_AddId(idList1, i+1);
    if (rv64<0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "here (%d)", (int) rv64);
      GWEN_IdList64_free(idList1);
      return (int) rv64;
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
  while (id) {
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
  while (id) {
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



int test3(GWEN_UNUSED GWEN_TEST_MODULE *mod)
{
  GWEN_IDLIST64 *ptrList1;
  int i;

  ptrList1=GWEN_IdList64_newWithSteps(256);

  for (i=0; i<2048; i++) {
    int64_t rv;

    rv=GWEN_IdList64_AddId(ptrList1, (uint64_t) i+1);
    if (rv<0) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Error on GWEN_IdList64_AddEntry(%d): %s (%d)",
                i, GWEN_Error_SimpleToString((int)rv), (int)rv);
      return rv;
    }
    if (rv!=(int64_t)i) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected index returned for entry %d: is %ld, should be %d ", i, (unsigned long) rv, i);
      return GWEN_ERROR_INTERNAL;
    }
  }


  for (i=0; i<8; i++) {
    GWEN_IDTABLE64 *table;
    int refCounter;

    table=GWEN_IdList64_GetTableAt(ptrList1, i);
    refCounter=table->refCount;
    if (refCounter!=1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected reference counter in table (list1) %d: is %d, should be 1 ", i, refCounter);
      return GWEN_ERROR_INTERNAL;
    }
  }

  GWEN_IdList64_free(ptrList1);

  return 0;
}



int test4(GWEN_UNUSED GWEN_TEST_MODULE *mod)
{
  GWEN_IDLIST64 *ptrList1;
  GWEN_IDLIST64 *ptrList2;
  int i;

  ptrList1=GWEN_IdList64_newWithSteps(256);

  for (i=0; i<2048; i++) {
    int64_t rv;

    rv=GWEN_IdList64_AddId(ptrList1, (uint64_t) i+1);
    if (rv<0) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Error on GWEN_IdList64_AddId(%d): %s (%d)",
                i, GWEN_Error_SimpleToString((int)rv), (int)rv);
      return rv;
    }
    if (rv!=(int64_t)i) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected index returned for entry %d: is %ld, should be %d ", i, (unsigned long) rv, i);
      return GWEN_ERROR_INTERNAL;
    }
  }


  for (i=0; i<8; i++) {
    GWEN_IDTABLE64 *table;
    int refCounter;

    table=GWEN_IdList64_GetTableAt(ptrList1, i);
    refCounter=table->refCount;
    if (refCounter!=1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected reference counter in table (list1) %d: is %d, should be 1 ", i, refCounter);
      return GWEN_ERROR_INTERNAL;
    }
  }

  ptrList2=GWEN_IdList64_LazyCopy(ptrList1);

  GWEN_IdList64_free(ptrList1);
  GWEN_IdList64_free(ptrList2);

  return 0;
}



int test5(GWEN_UNUSED GWEN_TEST_MODULE *mod)
{
  GWEN_IDLIST64 *ptrList1;
  GWEN_IDLIST64 *ptrList2;
  int i;

  ptrList1=GWEN_IdList64_newWithSteps(256);

  for (i=0; i<2048; i++) {
    int64_t rv;

    rv=GWEN_IdList64_AddId(ptrList1, (uint64_t) i+1);
    if (rv<0) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Error on GWEN_IdList64_AddId(%d): %s (%d)",
                i, GWEN_Error_SimpleToString((int)rv), (int)rv);
      return rv;
    }
    if (rv!=(int64_t)i) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected index returned for entry %d: is %ld, should be %d ", i, (unsigned long) rv, i);
      return GWEN_ERROR_INTERNAL;
    }
  }

  ptrList2=GWEN_IdList64_LazyCopy(ptrList1);

  for (i=0; i<2048; i++) {
    int64_t entry;

    entry=GWEN_IdList64_GetIdAt(ptrList1, i);
    if (entry<0) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Error on GWEN_IdList64_GetIdAt(%d, list1): %s (%d)",
                i, GWEN_Error_SimpleToString(entry), (int)entry);
      return entry;
    }
    if (entry != (int64_t)(i+1)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Non-matching entry at index %d: is %ld, should be %d ", i, (unsigned long) entry, i);
      return GWEN_ERROR_INTERNAL;
    }
  }

  for (i=0; i<2048; i++) {
    int64_t entry;

    entry=GWEN_IdList64_GetIdAt(ptrList2, i);
    if (entry<0) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Error on GWEN_IdList64_GetIdAt(%d, list2): %s (%d)",
                i, GWEN_Error_SimpleToString(entry), (int)entry);
      return entry;
    }
    if (entry != (int64_t)(i+1)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Non-matching entry at index %d: is %ld, should be %d ", i, (unsigned long) entry, i);
      return GWEN_ERROR_INTERNAL;
    }
  }


  for (i=0; i<8; i++) {
    GWEN_IDTABLE64 *table;
    int refCounter;

    table=GWEN_IdList64_GetTableAt(ptrList1, i);
    refCounter=table->refCount;
    if (refCounter!=1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected reference counter in table (list1) %d: is %d, should be 1 ", i, refCounter);
      return GWEN_ERROR_INTERNAL;
    }
  }

  for (i=0; i<8; i++) {
    GWEN_IDTABLE64 *table;
    int refCounter;

    table=GWEN_IdList64_GetTableAt(ptrList2, i);
    refCounter=table->refCount;
    if (refCounter!=1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected reference counter in table (list2) %d: is %d, should be 1 ", i, refCounter);
      return GWEN_ERROR_INTERNAL;
    }
  }

  GWEN_IdList64_free(ptrList2);
  GWEN_IdList64_free(ptrList1);

  return 0;
}



int test6(GWEN_UNUSED GWEN_TEST_MODULE *mod)
{
  GWEN_IDLIST64 *ptrList1;
  GWEN_IDLIST64 *ptrList2;
  int i;

  ptrList1=GWEN_IdList64_newWithSteps(256);

  for (i=0; i<2048; i++) {
    int64_t rv;

    rv=GWEN_IdList64_AddId(ptrList1, (uint64_t) i+1);
    if (rv<0) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Error on GWEN_IdList64_AddId(%d): %s (%d)",
                i, GWEN_Error_SimpleToString((int)rv), (int)rv);
      return rv;
    }
    if (rv!=(int64_t)i) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected index returned for entry %d: is %ld, should be %d ", i, (unsigned long) rv, i);
      return GWEN_ERROR_INTERNAL;
    }
  }

  ptrList2=GWEN_IdList64_LazyCopy(ptrList1);

  for (i=0; i<8; i++) {
    GWEN_IDTABLE64 *table;
    int refCounter;

    table=GWEN_IdList64_GetTableAt(ptrList1, i);
    refCounter=table->refCount;
    if (refCounter!=1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected reference counter in table %d: is %d, should be 1", i, refCounter);
      return GWEN_ERROR_INTERNAL;
    }
  }

  GWEN_IdList64_free(ptrList1);

  for (i=0; i<8; i++) {
    GWEN_IDTABLE64 *table;
    int refCounter;

    table=GWEN_IdList64_GetTableAt(ptrList2, i);
    refCounter=table->refCount;
    if (refCounter!=1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected reference counter in table (list2) %d: is %d, should be 1 ", i, refCounter);
      return GWEN_ERROR_INTERNAL;
    }
  }

  GWEN_IdList64_free(ptrList2);

  return 0;
}



int test7(GWEN_UNUSED GWEN_TEST_MODULE *mod)
{
  GWEN_IDLIST64 *ptrList1;
  GWEN_IDLIST64 *ptrList2;
  int i;
  int64_t iEntry;

  ptrList1=GWEN_IdList64_newWithSteps(256);

  for (i=0; i<2048; i++) {
    int64_t rv;

    rv=GWEN_IdList64_AddId(ptrList1, (uint64_t) i+1);
    if (rv<0) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Error on GWEN_IdList64_AddId(%d): %s (%d)",
                i, GWEN_Error_SimpleToString((int)rv), (int)rv);
      return rv;
    }
    if (rv!=(int64_t)i) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected index returned for entry %d: is %ld, should be %d ", i, (unsigned long) rv, i);
      return GWEN_ERROR_INTERNAL;
    }
  }

  for (i=0; i<8; i++) {
    GWEN_IDTABLE64 *table;
    int refCounter;

    table=GWEN_IdList64_GetTableAt(ptrList1, i);
    refCounter=table->refCount;
    if (refCounter!=1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected reference counter in table %d: is %d, should be 1", i, refCounter);
      return GWEN_ERROR_INTERNAL;
    }
  }

  ptrList2=GWEN_IdList64_LazyCopy(ptrList1);

  for (i=0; i<8; i++) {
    GWEN_IDTABLE64 *table;
    int refCounter;

    table=GWEN_IdList64_GetTableAt(ptrList2, i);
    refCounter=table->refCount;
    if (refCounter!=1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected reference counter in table %d: is %d, should be 1 ", i, refCounter);
      return GWEN_ERROR_INTERNAL;
    }
  }

  for (i=0; i<8; i++) {
    GWEN_IDTABLE64 *table1;
    GWEN_IDTABLE64 *table2;

    table1=GWEN_IdList64_GetTableAt(ptrList1, i);
    table2=GWEN_IdList64_GetTableAt(ptrList2, i);
    if (table1!=table2) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Table at position %d should be equal but isn't", i);
      return GWEN_ERROR_INTERNAL;
    }
  }


  iEntry=GWEN_IdList64_AddId(ptrList2, (uint64_t) 2048);
  if (iEntry<0) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Error on GWEN_IdList64_AddId(%d): %s (%d)",
              i, GWEN_Error_SimpleToString((int)iEntry), (int)iEntry);
    return (int)iEntry;
  }
  if (iEntry!=(int64_t)2048) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected index returned for entry %d: is %ld, should be 2048", i, (unsigned long) iEntry);
    return GWEN_ERROR_INTERNAL;
  }


  if (GWEN_IdList64_GetIdAt(ptrList1, 2048)>0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Entry 2048 exists in list1 when it should not");
    return GWEN_ERROR_INTERNAL;
  }

  iEntry=GWEN_IdList64_GetIdAt(ptrList2, 2049);
  if (iEntry>0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Entry 2049 exists in list2 when it should not (%ld)", (long int) iEntry);
    return GWEN_ERROR_INTERNAL;
  }


  for (i=0; i<8; i++) {
    GWEN_IDTABLE64 *table;
    int refCounter;

    table=GWEN_IdList64_GetTableAt(ptrList1, i);
    refCounter=table->refCount;
    if (refCounter!=2) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected reference counter in table %d: is %d, should be 2 ", i, refCounter);
      return GWEN_ERROR_INTERNAL;
    }
  }

  for (i=0; i<8; i++) {
    GWEN_IDTABLE64 *table;
    int refCounter;

    table=GWEN_IdList64_GetTableAt(ptrList2, i);
    refCounter=table->refCount;
    if (refCounter!=2) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected reference counter in table %d: is %d, should be 2 ", i, refCounter);
      return GWEN_ERROR_INTERNAL;
    }
  }

  for (i=8; i<9; i++) {
    GWEN_IDTABLE64 *table;
    int refCounter;

    table=GWEN_IdList64_GetTableAt(ptrList2, i);
    refCounter=table->refCount;
    if (refCounter!=1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected reference counter in table %d: is %d, should be 1 ", i, refCounter);
      return GWEN_ERROR_INTERNAL;
    }
  }

  return 0;
}



int test8(GWEN_UNUSED GWEN_TEST_MODULE *mod)
{
  GWEN_IDLIST64 *ptrList1;
  GWEN_IDLIST64 *ptrList2;
  int i;

  ptrList1=GWEN_IdList64_newWithSteps(256);

  for (i=0; i<2048; i++) {
    int64_t rv;

    rv=GWEN_IdList64_AddId(ptrList1, (uint64_t) i+1);
    if (rv<0) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Error on GWEN_IdList64_AddId(%d): %s (%d)",
                i, GWEN_Error_SimpleToString((int)rv), (int)rv);
      return rv;
    }
    if (rv!=(int64_t)i) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected index returned for entry %d: is %ld, should be %d ", i, (unsigned long) rv, i);
      return GWEN_ERROR_INTERNAL;
    }
  }

  ptrList2=GWEN_IdList64_LazyCopy(ptrList1);

  for (i=2048; i<4096; i++) {
    int64_t rv;

    rv=GWEN_IdList64_AddId(ptrList2, (uint64_t) i+1);
    if (rv<0) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Error on GWEN_IdList64_AddId(%d): %s (%d)",
                i, GWEN_Error_SimpleToString((int)rv), (int)rv);
      return rv;
    }
    if (rv!=(int64_t)i) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected index returned for entry %d: is %ld, should be %d ", i, (unsigned long) rv, i);
      return GWEN_ERROR_INTERNAL;
    }
  }


  for (i=0; i<2048; i++) {
    int64_t entry;

    entry=GWEN_IdList64_GetIdAt(ptrList1, i);
    if (entry<0) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Error on GWEN_IdList64_GetIdAt(%d, list1): %s (%d)",
                i, GWEN_Error_SimpleToString(entry), (int)entry);
      return entry;
    }
    if (entry != (int64_t)(i+1)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Non-matching entry at index %d: is %ld, should be %d ", i, (unsigned long) entry, i);
      return GWEN_ERROR_INTERNAL;
    }
  }

  if (GWEN_IdList64_GetIdAt(ptrList1, 2048)>0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Entry 2048 exists in list1 when it should not");
    return GWEN_ERROR_INTERNAL;
  }

  for (i=0; i<4096; i++) {
    int64_t entry;

    entry=GWEN_IdList64_GetIdAt(ptrList2, i);
    if (entry<0) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Error on GWEN_IdList64_GetIdAt(%d, list2): %s (%d)",
                i, GWEN_Error_SimpleToString(entry), (int)entry);
      return entry;
    }
    if (entry != (int64_t)(i+1)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Non-matching entry at index %d: is %ld, should be %d ", i, (unsigned long) entry, i);
      return GWEN_ERROR_INTERNAL;
    }
  }

  if (GWEN_IdList64_GetIdAt(ptrList1, 4096)>=0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Entry 4096 exists in list2 when it should not");
    return GWEN_ERROR_INTERNAL;
  }


  for (i=0; i<8; i++) {
    GWEN_IDTABLE64 *table;
    int refCounter;

    table=GWEN_IdList64_GetTableAt(ptrList1, i);
    refCounter=table->refCount;
    if (refCounter!=2) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected reference counter in table %d: is %d, should be 2 ", i, refCounter);
      return GWEN_ERROR_INTERNAL;
    }
  }

  for (i=0; i<8; i++) {
    GWEN_IDTABLE64 *table;
    int refCounter;

    table=GWEN_IdList64_GetTableAt(ptrList2, i);
    refCounter=table->refCount;
    if (refCounter!=2) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected reference counter in table %d: is %d, should be 2 ", i, refCounter);
      return GWEN_ERROR_INTERNAL;
    }
  }

  for (i=8; i<16; i++) {
    GWEN_IDTABLE64 *table;
    int refCounter;

    table=GWEN_IdList64_GetTableAt(ptrList2, i);
    refCounter=table->refCount;
    if (refCounter!=1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected reference counter in table %d: is %d, should be 1 ", i, refCounter);
      return GWEN_ERROR_INTERNAL;
    }
  }


  GWEN_IdList64_free(ptrList1);

  for (i=0; i<(2048/256); i++) {
    GWEN_IDTABLE64 *table;
    int refCounter;

    table=GWEN_IdList64_GetTableAt(ptrList2, i);
    refCounter=table->refCount;
    if (refCounter!=1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected reference counter in table (list2) %d: is %d, should be 1 ", i, refCounter);
      return GWEN_ERROR_INTERNAL;
    }
  }

  return 0;
}



int test9(GWEN_UNUSED GWEN_TEST_MODULE *mod)
{
  GWEN_IDLIST64 *ptrList1;
  GWEN_IDLIST64 *ptrList2;
  int i;
  int rv;

  ptrList1=GWEN_IdList64_newWithSteps(256);

  for (i=0; i<2047; i++) {
    int64_t rv64;

    rv64=GWEN_IdList64_AddId(ptrList1, (uint64_t) i+1);
    if (rv64<0) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Error on GWEN_IdList64_AddId(%d): %s (%d)",
                i, GWEN_Error_SimpleToString((int)rv64), (int)rv64);
      return rv;
    }
    if (rv64!=(int64_t)i) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected index returned for entry %d: is %ld, should be %d ", i, (unsigned long) rv64, i);
      return GWEN_ERROR_INTERNAL;
    }
  }

  for (i=0; i<8; i++) {
    GWEN_IDTABLE64 *table;
    int refCounter;

    table=GWEN_IdList64_GetTableAt(ptrList1, i);
    refCounter=table->refCount;
    if (refCounter!=1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected reference counter in table %d: is %d, should be 1", i, refCounter);
      return GWEN_ERROR_INTERNAL;
    }
  }

  ptrList2=GWEN_IdList64_LazyCopy(ptrList1);

  rv=GWEN_IdList64_AddId(ptrList2, (uint64_t) 2047);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Error on GWEN_IdList64_AddId(%d): %s (%d)",
              i, GWEN_Error_SimpleToString((int)rv), (int)rv);
    return rv;
  }
  if (rv!=(int64_t)2047) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected index returned for entry %d: is %ld, should be 2047", i, (unsigned long) rv);
    return GWEN_ERROR_INTERNAL;
  }

  if (GWEN_IdList64_GetTableAt(ptrList2, 8)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Table 8 exists when it should not");
    return GWEN_ERROR_INTERNAL;
  }


  if (GWEN_IdList64_GetIdAt(ptrList2, 2049)>0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Entry 2049 exists in list2 when it should not");
    return GWEN_ERROR_INTERNAL;
  }


  for (i=0; i<8; i++) {
    GWEN_IDTABLE64 *table;
    int refCounter;

    table=GWEN_IdList64_GetTableAt(ptrList1, i);
    refCounter=table->refCount;
    if (refCounter!=2) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected reference counter in table %d: is %d, should be 2 ", i, refCounter);
      return GWEN_ERROR_INTERNAL;
    }
  }

  for (i=0; i<8; i++) {
    GWEN_IDTABLE64 *table;
    int refCounter;

    table=GWEN_IdList64_GetTableAt(ptrList2, i);
    refCounter=table->refCount;
    if (refCounter!=2) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected reference counter in table %d: is %d, should be 2 ", i, refCounter);
      return GWEN_ERROR_INTERNAL;
    }
  }

  for (i=8; i<8; i++) {
    GWEN_IDTABLE64 *table;
    int refCounter;

    table=GWEN_IdList64_GetTableAt(ptrList2, i);
    refCounter=table->refCount;
    if (refCounter!=1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected reference counter in table %d: is %d, should be 1 ", i, refCounter);
      return GWEN_ERROR_INTERNAL;
    }
  }

  return 0;
}



int test10(GWEN_UNUSED GWEN_TEST_MODULE *mod)
{
  GWEN_IDLIST64 *ptrList1;
  GWEN_IDLIST64 *ptrList2;
  int i;

  ptrList1=GWEN_IdList64_newWithSteps(256);

  for (i=0; i<2048; i++) {
    int64_t rv;

    rv=GWEN_IdList64_AddId(ptrList1, (uint64_t) i+1);
    if (rv<0) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Error on GWEN_IdList64_AddId(%d): %s (%d)",
                i, GWEN_Error_SimpleToString((int)rv), (int)rv);
      return rv;
    }
    if (rv!=(int64_t)i) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected index returned for entry %d: is %ld, should be %d ", i, (unsigned long) rv, i);
      return GWEN_ERROR_INTERNAL;
    }
  }

  ptrList2=GWEN_IdList64_dup(ptrList1);

  for (i=0; i<2048; i++) {
    int64_t entry;

    entry=GWEN_IdList64_GetIdAt(ptrList1, i);
    if (entry<0) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Error on GWEN_IdList64_GetIdAt(%d, list1): %s (%d)",
                i, GWEN_Error_SimpleToString(entry), (int)entry);
      return entry;
    }
    if (entry != (int64_t)(i+1)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Non-matching entry at index %d: is %ld, should be %d ", i, (unsigned long) entry, i);
      return GWEN_ERROR_INTERNAL;
    }
  }

  for (i=0; i<2048; i++) {
    int64_t entry;

    entry=GWEN_IdList64_GetIdAt(ptrList2, i);
    if (entry<0) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Error on GWEN_IdList64_GetIdAt(%d, list2): %s (%d)",
                i, GWEN_Error_SimpleToString(entry), (int)entry);
      return entry;
    }
    if (entry != (int64_t)(i+1)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Non-matching entry at index %d: is %ld, should be %d ", i, (unsigned long) entry, i);
      return GWEN_ERROR_INTERNAL;
    }
  }


  for (i=0; i<8; i++) {
    GWEN_IDTABLE64 *table;
    int refCounter;

    table=GWEN_IdList64_GetTableAt(ptrList1, i);
    refCounter=table->refCount;
    if (refCounter!=1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected reference counter in table (list1) %d: is %d, should be 1 ", i, refCounter);
      return GWEN_ERROR_INTERNAL;
    }
  }

  for (i=0; i<8; i++) {
    GWEN_IDTABLE64 *table;
    int refCounter;

    table=GWEN_IdList64_GetTableAt(ptrList2, i);
    refCounter=table->refCount;
    if (refCounter!=1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected reference counter in table (list2) %d: is %d, should be 1 ", i, refCounter);
      return GWEN_ERROR_INTERNAL;
    }
  }

  GWEN_IdList64_free(ptrList2);
  GWEN_IdList64_free(ptrList1);

  return 0;
}



int test11(GWEN_UNUSED GWEN_TEST_MODULE *mod)
{
  GWEN_IDLIST64 *ptrList1;
  GWEN_IDLIST64 *ptrList2;
  int i;
  int rv;

  ptrList1=GWEN_IdList64_newWithSteps(256);

  for (i=0; i<2048; i++) {
    int64_t rv64;

    rv64=GWEN_IdList64_AddId(ptrList1, (uint64_t) i+1);
    if (rv64<0) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Error on GWEN_IdList64_AddId(%d): %s (%d)",
                i, GWEN_Error_SimpleToString((int)rv64), (int)rv64);
      return rv;
    }
    if (rv64!=(int64_t)i) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected index returned for entry %d: is %ld, should be %d ", i, (unsigned long) rv64, i);
      return GWEN_ERROR_INTERNAL;
    }
  }

  ptrList2=GWEN_IdList64_dup(ptrList1);


  rv=_compareList1AgainstList2(ptrList1, ptrList2);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "here (%d)", (int) rv);
    GWEN_IdList64_free(ptrList2);
    GWEN_IdList64_free(ptrList1);
    return (int) rv;
  }

  GWEN_IdList64_free(ptrList2);
  GWEN_IdList64_free(ptrList1);

  return 0;
}





int _compareList1AgainstList2(GWEN_IDLIST64 *idList1, GWEN_IDLIST64 *idList2)
{
  GWEN_IDLIST64_ITERATOR *iter1;
  GWEN_IDLIST64_ITERATOR *iter2;
  uint64_t id1;
  uint64_t id2;
  uint64_t index=0;

  iter1=GWEN_IdList64_Iterator_new(idList1);
  iter2=GWEN_IdList64_Iterator_new(idList2);
  id1=GWEN_IdList64_Iterator_GetFirstId(iter1);
  id2=GWEN_IdList64_Iterator_GetFirstId(iter2);
  while (id1>0 && id2>0) {
    if (id1 != id2) {
      fprintf(stderr, "Tables differ at position %lu (%lu != %lu)\n",
              (unsigned long) index,
              (unsigned long) id1,
              (unsigned long) id2);
      GWEN_IdList64_Iterator_free(iter2);
      GWEN_IdList64_Iterator_free(iter1);
      return GWEN_ERROR_GENERIC;
    }
    index++;
    id1=GWEN_IdList64_Iterator_GetNextId(iter1);
    id2=GWEN_IdList64_Iterator_GetNextId(iter2);
  }
  if (GWEN_IdList64_Iterator_GetNextId(iter1)>0 || GWEN_IdList64_Iterator_GetNextId(iter2)>0) {
    fprintf(stderr, "NUmber of elements in lists differs\n");
    GWEN_IdList64_Iterator_free(iter2);
    GWEN_IdList64_Iterator_free(iter1);
    return GWEN_ERROR_GENERIC;
  }
  GWEN_IdList64_Iterator_free(iter2);
  GWEN_IdList64_Iterator_free(iter1);

  return 0;
}



#else

int GWEN_IdList64_AddTests(GWEN_TEST_MODULE *mod)
{
  DBG_ERROR(GWEN_LOGDOMAIN, "Gwenhywfar was compiled without test code enabled.");
  return GWEN_ERROR_GENERIC;
}

#endif

