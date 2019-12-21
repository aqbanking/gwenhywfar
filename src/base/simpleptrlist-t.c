/***************************************************************************
    begin       : Fri Dec 06 2019
    copyright   : (C) 2019 by Martin Preuss
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


#include <gwenhywfar/mdigest.h>
#include <string.h>



/* ------------------------------------------------------------------------------------------------
 * type definitions
 * ------------------------------------------------------------------------------------------------
 */

typedef struct TEST_TYPE TEST_TYPE;
GWEN_LIST_FUNCTION_DEFS(TEST_TYPE, TestType)
struct TEST_TYPE {
  GWEN_LIST_ELEMENT(TEST_TYPE)
  int _refCounter;
  int testPosition;
  char *testStringPtr;
  uint8_t testHash[20];
};



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */


TEST_TYPE *TestType_new(const char *s, int pos);
void TestType_Attach(TEST_TYPE *tt);
void TestType_free(TEST_TYPE *tt);

int TestType_CalcHash(TEST_TYPE *tt);
int TestType_TestHash(const TEST_TYPE *tt);

GWENHYWFAR_CB void _attachToTestType(GWEN_SIMPLEPTRLIST *pl, void *p);
GWENHYWFAR_CB void _detachFromTestType(GWEN_SIMPLEPTRLIST *pl, void *p);


int test1(void);
int test2(void);
int test3(void);
int test4(void);
int test5(void);
int test6(void);

TEST_TYPE *createTestType(int num);
void dumpTestTypeList(TEST_TYPE_LIST *ttList);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */


int GWEN_SimplePtrList_Test(void)
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

  if (numFailed)
    return GWEN_ERROR_GENERIC;
  return 0;
}



GWEN_LIST_FUNCTIONS(TEST_TYPE, TestType)




TEST_TYPE *TestType_new(const char *s, int pos)
{
  TEST_TYPE *tt;

  GWEN_NEW_OBJECT(TEST_TYPE, tt);
  tt->_refCounter=1;
  GWEN_LIST_INIT(TEST_TYPE, tt);
  if (s)
    tt->testStringPtr=strdup(s);
  tt->testPosition=pos;
  return tt;
}



void TestType_Attach(TEST_TYPE *tt)
{
  assert(tt);
  assert(tt->_refCounter);

  if (tt && tt->_refCounter) {
    tt->_refCounter++;
    /*DBG_ERROR(GWEN_LOGDOMAIN, "Attached (%d: refcount=%d)", tt->testPosition, tt->_refCounter);*/
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "NULL pointer or already freed");
  }
}



void TestType_free(TEST_TYPE *tt)
{
  if (tt && tt->_refCounter) {
    /*DBG_ERROR(GWEN_LOGDOMAIN, "Detaching (%d: refcount=%d)", tt->testPosition, tt->_refCounter);*/
    if (tt->_refCounter==1) {
      GWEN_LIST_FINI(TEST_TYPE, tt);
      if (tt->testStringPtr)
        free(tt->testStringPtr);
      tt->_refCounter=0;
      GWEN_FREE_OBJECT(tt);
    }
    else
      tt->_refCounter--;
  }
}



int TestType_CalcHash(TEST_TYPE *tt)
{
  if (tt->testStringPtr && *(tt->testStringPtr)) {
    GWEN_MDIGEST *md;
    int rv;

    md=GWEN_MDigest_Rmd160_new();
    rv=GWEN_MDigest_Digest(md,
			   (const uint8_t*)tt->testStringPtr, strlen(tt->testStringPtr),
			   tt->testHash, sizeof(tt->testHash));
    if (rv<0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error on GWEN_MDigest_Begin: %s (%d)", GWEN_Error_SimpleToString(rv), rv);
      GWEN_MDigest_free(md);
      return rv;
    }
    GWEN_MDigest_free(md);
  }
  return 0;
}



int TestType_TestHash(const TEST_TYPE *tt)
{
  if (tt->testStringPtr && *(tt->testStringPtr)) {
    GWEN_MDIGEST *md;
    uint8_t hash[20];
    int rv;

    md=GWEN_MDigest_Rmd160_new();
    rv=GWEN_MDigest_Digest(md,
			   (const uint8_t*)tt->testStringPtr, strlen(tt->testStringPtr),
			   hash, sizeof(hash));
    if (rv<0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error on GWEN_MDigest_Digest: %s (%d)", GWEN_Error_SimpleToString(rv), rv);
      GWEN_MDigest_free(md);
      return rv;
    }
    GWEN_MDigest_free(md);

    if (memcmp(hash, tt->testHash, sizeof(hash))!=0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Invalid hash, object was modified.");
      return GWEN_ERROR_GENERIC;
    }
  }
  return 0;
}



TEST_TYPE *createTestType(int num)
{
  TEST_TYPE *tt;
  char testString[256];
  int rv;

  snprintf(testString, sizeof(testString), "This is test string number %d", num);
  tt=TestType_new(testString, num);
  if (tt==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not create test object %d", num);
    return NULL;
  }
  rv=TestType_CalcHash(tt);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    TestType_free(tt);
    return NULL;
  }
  return tt;
}



void _attachToTestType(GWEN_UNUSED GWEN_SIMPLEPTRLIST *pl, void *p)
{
  TEST_TYPE *tt;

  tt=(TEST_TYPE*) p;
  TestType_Attach(tt);
}



void _detachFromTestType(GWEN_UNUSED GWEN_SIMPLEPTRLIST *pl, void *p)
{
  TEST_TYPE *tt;

  tt=(TEST_TYPE*) p;
  TestType_free(tt);
}



void dumpTestTypeList(TEST_TYPE_LIST *ttList)
{
  TEST_TYPE *tt;

  tt=TestType_List_First(ttList);
  while(tt) {
    fprintf(stderr, "%5d: %3d: %s\n", tt->testPosition, tt->_refCounter, tt->testStringPtr);
    tt=TestType_List_Next(tt);
  }
}



/* ------------------------------------------------------------------------------------------------
 * test 1: test struct members after construction
 * ------------------------------------------------------------------------------------------------
 */

int test1(void)
{
  GWEN_SIMPLEPTRLIST *pl;
  int i;

  pl=GWEN_SimplePtrList_new(128, 128);
  if (pl==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not create pointer list");
    return GWEN_ERROR_GENERIC;
  }

  if (pl->refCount!=1) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid object field: refCounter");
    GWEN_SimplePtrList_free(pl);
    return GWEN_ERROR_GENERIC;
  }

  if (pl->maxEntries!=128) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid object field: maxEntries");
    GWEN_SimplePtrList_free(pl);
    return GWEN_ERROR_GENERIC;
  }

  if (pl->usedEntries!=0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid object field: usedEntries");
    GWEN_SimplePtrList_free(pl);
    return GWEN_ERROR_GENERIC;
  }

  if (pl->steps!=128) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid object field: steps");
    GWEN_SimplePtrList_free(pl);
    return GWEN_ERROR_GENERIC;
  }


  if (pl->entryList->refCounter!=1) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid internal object field: refCounter");
    GWEN_SimplePtrList_free(pl);
    return GWEN_ERROR_GENERIC;
  }

  if (pl->entryList->storedEntries!=128) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid internal object field: storedEntries");
    GWEN_SimplePtrList_free(pl);
    return GWEN_ERROR_GENERIC;
  }


  for (i=0; i<128; i++) {
    if (pl->entryList->entries[i]!=NULL) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Invalid object entry: idx %d is not NULL", i);
      GWEN_SimplePtrList_free(pl);
      return GWEN_ERROR_GENERIC;
    }
  }


  GWEN_SimplePtrList_free(pl);
  return 0;
}



/* ------------------------------------------------------------------------------------------------
 * test 2: test adding pointers
 * ------------------------------------------------------------------------------------------------
 */

int test2(void)
{

  GWEN_SIMPLEPTRLIST *pl;
  int i;

  pl=GWEN_SimplePtrList_new(128, 128);
  for (i=0; i<1024; i++) {
    TEST_TYPE *tt;
    int64_t idx;

    tt=createTestType(i);
    idx=GWEN_SimplePtrList_AddPtr(pl, tt);
    if (idx<0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error adding pointer %d to list: %s (%d)", i, GWEN_Error_SimpleToString((int)idx), (int)idx);
      GWEN_SimplePtrList_free(pl);
      return (int) idx;
    }
  } /* for */

  if (pl->maxEntries!=1024) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid object field: maxEntries");
    GWEN_SimplePtrList_free(pl);
    return GWEN_ERROR_GENERIC;
  }

  if (pl->usedEntries!=1024) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid object field: usedEntries");
    GWEN_SimplePtrList_free(pl);
    return GWEN_ERROR_GENERIC;
  }

  for (i=0; i<1024; i++) {
    TEST_TYPE *tt;
    int rv;

    tt=(TEST_TYPE*) GWEN_SimplePtrList_GetPtrAt(pl, i);
    if (tt==NULL) {
      DBG_ERROR(GWEN_LOGDOMAIN, "No object at position %d", i);
      GWEN_SimplePtrList_free(pl);
      return GWEN_ERROR_GENERIC;
    }

    rv=TestType_TestHash(tt);
    if (rv<0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error testing hash of object %d: %s (%d)", i, GWEN_Error_SimpleToString(rv), rv);
      GWEN_SimplePtrList_free(pl);
      return rv;
    }
  } /* for */

  /* free all objects */
  for (i=0; i<1024; i++) {
    TEST_TYPE *tt;

    tt=(TEST_TYPE*) GWEN_SimplePtrList_GetPtrAt(pl, i);
    if (tt==NULL) {
      DBG_ERROR(GWEN_LOGDOMAIN, "No object at position %d", i);
      GWEN_SimplePtrList_free(pl);
      return GWEN_ERROR_GENERIC;
    }
    GWEN_SimplePtrList_SetPtrAt(pl, i, NULL);
    TestType_free(tt);
  } /* for */

  GWEN_SimplePtrList_free(pl);
  return 0;
}



/* ------------------------------------------------------------------------------------------------
 * test 3: test lazy copying
 * ------------------------------------------------------------------------------------------------
 */

int test3(void)
{

  GWEN_SIMPLEPTRLIST *pl;
  GWEN_SIMPLEPTRLIST *plCopy;
  int i;

  pl=GWEN_SimplePtrList_new(128, 128);
  for (i=0; i<1000; i++) {
    TEST_TYPE *tt;
    int64_t idx;

    tt=createTestType(i);
    idx=GWEN_SimplePtrList_AddPtr(pl, tt);
    if (idx<0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error adding pointer %d to list: %s (%d)", i, GWEN_Error_SimpleToString((int)idx), (int)idx);
      GWEN_SimplePtrList_free(pl);
      return (int) idx;
    }
  } /* for */

  plCopy=GWEN_SimplePtrList_LazyCopy(pl);
  if (plCopy==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not copy pointer list");
    GWEN_SimplePtrList_free(plCopy);
    GWEN_SimplePtrList_free(pl);
    return GWEN_ERROR_INVALID;
  }

  if (!(plCopy->flags & GWEN_SIMPLEPTRLIST_FLAGS_COPYONWRITE)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid object field: flags");
    GWEN_SimplePtrList_free(plCopy);
    GWEN_SimplePtrList_free(pl);
    return GWEN_ERROR_GENERIC;
  }

  if (!(pl->flags & GWEN_SIMPLEPTRLIST_FLAGS_COPYONWRITE)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid object field in old list: flags");
    GWEN_SimplePtrList_free(plCopy);
    GWEN_SimplePtrList_free(pl);
    return GWEN_ERROR_GENERIC;
  }

  if (plCopy->entryList != pl->entryList) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid object field: entryList (should be the same after lazy copy)");
    GWEN_SimplePtrList_free(plCopy);
    GWEN_SimplePtrList_free(pl);
    return GWEN_ERROR_GENERIC;
  }

  if (pl->entryList->refCounter!=2) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid internal object field: refCounter (should be 2)");
    GWEN_SimplePtrList_free(pl);
    return GWEN_ERROR_GENERIC;
  }

  for (i=1000; i<1024; i++) {
    TEST_TYPE *tt;
    int64_t idx;

    tt=createTestType(i);
    idx=GWEN_SimplePtrList_AddPtr(plCopy, tt);
    if (idx<0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error adding pointer %d to list: %s (%d)", i, GWEN_Error_SimpleToString((int)idx), (int)idx);
      GWEN_SimplePtrList_free(plCopy);
      GWEN_SimplePtrList_free(pl);
      return (int) idx;
    }
  } /* for */

  if (plCopy->flags & GWEN_SIMPLEPTRLIST_FLAGS_COPYONWRITE) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid object field: flags (should have cleared GWEN_SIMPLEPTRLIST_FLAGS_COPYONWRITE)");
    GWEN_SimplePtrList_free(plCopy);
    GWEN_SimplePtrList_free(pl);
    return GWEN_ERROR_GENERIC;
  }

  if (!(pl->flags & GWEN_SIMPLEPTRLIST_FLAGS_COPYONWRITE)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid object field in old list: flags (should not have cleared GWEN_SIMPLEPTRLIST_FLAGS_COPYONWRITE)");
    GWEN_SimplePtrList_free(plCopy);
    GWEN_SimplePtrList_free(pl);
    return GWEN_ERROR_GENERIC;
  }

  if (plCopy->entryList == pl->entryList) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid object field: entryList (should have changed)");
    GWEN_SimplePtrList_free(plCopy);
    GWEN_SimplePtrList_free(pl);
    return GWEN_ERROR_GENERIC;
  }

  if (pl->entryList->refCounter!=1) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid internal object field: refCounter (should be back to 1)");
    GWEN_SimplePtrList_free(pl);
    return GWEN_ERROR_GENERIC;
  }

  if (plCopy->entryList->refCounter!=1) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid internal object field: refCounter (should be 1)");
    GWEN_SimplePtrList_free(pl);
    return GWEN_ERROR_GENERIC;
  }

  if (plCopy->maxEntries!=1024+128) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid object field: maxEntries (should be 1152)");
    GWEN_SimplePtrList_free(pl);
    return GWEN_ERROR_GENERIC;
  }

  if (plCopy->usedEntries!=1024) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid object field: usedEntries");
    GWEN_SimplePtrList_free(pl);
    return GWEN_ERROR_GENERIC;
  }

  if (pl->maxEntries!=1024) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid object field: maxEntries (should still be 1024)");
    GWEN_SimplePtrList_free(pl);
    return GWEN_ERROR_GENERIC;
  }

  if (pl->usedEntries!=1000) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid object field: usedEntries (should still be 1000)");
    GWEN_SimplePtrList_free(pl);
    return GWEN_ERROR_GENERIC;
  }


  for (i=0; i<1024; i++) {
    TEST_TYPE *tt;
    int rv;

    tt=(TEST_TYPE*) GWEN_SimplePtrList_GetPtrAt(plCopy, i);
    if (tt==NULL) {
      DBG_ERROR(GWEN_LOGDOMAIN, "No object at position %d", i);
      GWEN_SimplePtrList_free(plCopy);
      GWEN_SimplePtrList_free(pl);
      return GWEN_ERROR_GENERIC;
    }

    rv=TestType_TestHash(tt);
    if (rv<0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error testing hash of object %d: %s (%d)", i, GWEN_Error_SimpleToString(rv), rv);
      GWEN_SimplePtrList_free(plCopy);
      GWEN_SimplePtrList_free(pl);
      return rv;
    }
  } /* for */

  /* free all objects */
  for (i=0; i<1024; i++) {
    TEST_TYPE *tt;

    tt=(TEST_TYPE*) GWEN_SimplePtrList_GetPtrAt(plCopy, i);
    if (tt==NULL) {
      DBG_ERROR(GWEN_LOGDOMAIN, "No object at position %d", i);
      GWEN_SimplePtrList_free(plCopy);
      GWEN_SimplePtrList_free(pl);
      return GWEN_ERROR_GENERIC;
    }
    GWEN_SimplePtrList_SetPtrAt(plCopy, i, NULL);
    TestType_free(tt);
  } /* for */


  GWEN_SimplePtrList_free(plCopy);
  GWEN_SimplePtrList_free(pl);
  return 0;
}



/* ------------------------------------------------------------------------------------------------
 * test 4: test adding pointers with attach/detach
 * ------------------------------------------------------------------------------------------------
 */

int test4(void)
{

  GWEN_SIMPLEPTRLIST *pl;
  TEST_TYPE_LIST *ttList;
  int i;

  pl=GWEN_SimplePtrList_new(128, 128);
  GWEN_SimplePtrList_SetAttachObjectFn(pl, _attachToTestType);
  GWEN_SimplePtrList_SetFreeObjectFn(pl, _detachFromTestType);
  GWEN_SimplePtrList_AddFlags(pl, GWEN_SIMPLEPTRLIST_FLAGS_ATTACHTOOBJECTS);
  GWEN_SimplePtrList_AddFlags(pl, GWEN_SIMPLEPTRLIST_FLAGS_DETACHFROMOBJECTS);

  ttList=TestType_List_new();

  for (i=0; i<1024; i++) {
    TEST_TYPE *tt;
    int64_t idx;

    tt=createTestType(i);
    TestType_List_Add(tt, ttList);

    idx=GWEN_SimplePtrList_AddPtr(pl, tt);
    if (idx<0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error adding pointer %d to list: %s (%d)", i, GWEN_Error_SimpleToString((int)idx), (int)idx);
      GWEN_SimplePtrList_free(pl);
      return (int) idx;
    }
  } /* for */

  if (1) {
    TEST_TYPE *tt;
    int cnt=0;

    tt=TestType_List_First(ttList);
    while(tt) {
      if (tt->_refCounter!=2) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Refcounter of object is not 2");
        TestType_List_free(ttList);
        GWEN_SimplePtrList_free(pl);
        return GWEN_ERROR_GENERIC;
      }
      cnt++;
      tt=TestType_List_Next(tt);
    }
    if (cnt!=1024) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Too few objects in list (%d instead of %d)", cnt, 1024);
    }
  }

  for (i=0; i<1024; i++) {
    TEST_TYPE *tt;
    int rv;

    tt=(TEST_TYPE*) GWEN_SimplePtrList_GetPtrAt(pl, i);
    if (tt==NULL) {
      DBG_ERROR(GWEN_LOGDOMAIN, "No object at position %d", i);
      TestType_List_free(ttList);
      GWEN_SimplePtrList_free(pl);
      return GWEN_ERROR_GENERIC;
    }

    rv=TestType_TestHash(tt);
    if (rv<0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error testing hash of object %d: %s (%d)", i, GWEN_Error_SimpleToString(rv), rv);
      TestType_List_free(ttList);
      GWEN_SimplePtrList_free(pl);
      return rv;
    }
  } /* for */

  GWEN_SimplePtrList_free(pl);

  if (1) {
    TEST_TYPE *tt;
    int cnt=0;

    tt=TestType_List_First(ttList);
    while(tt) {
      if (tt->_refCounter!=1) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Refcounter of object is not 1");
        TestType_List_free(ttList);
        return GWEN_ERROR_GENERIC;
      }
      cnt++;
      tt=TestType_List_Next(tt);
    }
    if (cnt!=1024) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Too few objects in list (%d instead of %d)", cnt, 1024);
    }
  }

  TestType_List_free(ttList);

  return 0;
}



/* ------------------------------------------------------------------------------------------------
 * test 5: test lazy copying with attach/detach
 * ------------------------------------------------------------------------------------------------
 */

int test5(void)
{

  GWEN_SIMPLEPTRLIST *pl;
  GWEN_SIMPLEPTRLIST *plCopy;
  TEST_TYPE_LIST *ttList;
  int i;

  pl=GWEN_SimplePtrList_new(128, 128);
  GWEN_SimplePtrList_SetAttachObjectFn(pl, _attachToTestType);
  GWEN_SimplePtrList_SetFreeObjectFn(pl, _detachFromTestType);
  GWEN_SimplePtrList_AddFlags(pl, GWEN_SIMPLEPTRLIST_FLAGS_ATTACHTOOBJECTS);
  GWEN_SimplePtrList_AddFlags(pl, GWEN_SIMPLEPTRLIST_FLAGS_DETACHFROMOBJECTS);

  ttList=TestType_List_new();
  for (i=0; i<1000; i++) {
    TEST_TYPE *tt;
    int64_t idx;

    tt=createTestType(i);
    TestType_List_Add(tt, ttList);
    idx=GWEN_SimplePtrList_AddPtr(pl, tt);
    if (idx<0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error adding pointer %d to list: %s (%d)", i, GWEN_Error_SimpleToString((int)idx), (int)idx);
      GWEN_SimplePtrList_free(pl);
      return (int) idx;
    }
  } /* for */

  plCopy=GWEN_SimplePtrList_LazyCopy(pl);
  if (plCopy==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not copy pointer list");
    GWEN_SimplePtrList_free(plCopy);
    GWEN_SimplePtrList_free(pl);
    return GWEN_ERROR_INVALID;
  }

  if (!(plCopy->flags & GWEN_SIMPLEPTRLIST_FLAGS_ATTACHTOOBJECTS)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid object field in list: flags (should have set GWEN_SIMPLEPTRLIST_FLAGS_ATTACHTOOBJECTS)");
    GWEN_SimplePtrList_free(plCopy);
    GWEN_SimplePtrList_free(pl);
    return GWEN_ERROR_GENERIC;
  }

  if (!(plCopy->flags & GWEN_SIMPLEPTRLIST_FLAGS_DETACHFROMOBJECTS)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid object field in list: flags (should have set GWEN_SIMPLEPTRLIST_FLAGS_DETACHFROMOBJECTS)");
    GWEN_SimplePtrList_free(plCopy);
    GWEN_SimplePtrList_free(pl);
    return GWEN_ERROR_GENERIC;
  }


  for (i=1000; i<1024; i++) {
    TEST_TYPE *tt;
    int64_t idx;

    tt=createTestType(i);
    TestType_List_Add(tt, ttList);
    idx=GWEN_SimplePtrList_AddPtr(plCopy, tt);
    if (idx<0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error adding pointer %d to list: %s (%d)", i, GWEN_Error_SimpleToString((int)idx), (int)idx);
      GWEN_SimplePtrList_free(plCopy);
      GWEN_SimplePtrList_free(pl);
      return (int) idx;
    }
  } /* for */


  for (i=0; i<1024; i++) {
    TEST_TYPE *tt;
    int rv;

    tt=(TEST_TYPE*) GWEN_SimplePtrList_GetPtrAt(plCopy, i);
    if (tt==NULL) {
      DBG_ERROR(GWEN_LOGDOMAIN, "No object at position %d", i);
      TestType_List_free(ttList);
      GWEN_SimplePtrList_free(plCopy);
      GWEN_SimplePtrList_free(pl);
      return GWEN_ERROR_GENERIC;
    }

    rv=TestType_TestHash(tt);
    if (rv<0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error testing hash of object %d: %s (%d)", i, GWEN_Error_SimpleToString(rv), rv);
      TestType_List_free(ttList);
      GWEN_SimplePtrList_free(plCopy);
      GWEN_SimplePtrList_free(pl);
      return rv;
    }
  } /* for */


  if (1) {
    TEST_TYPE *tt;
    int cnt=0;

    tt=TestType_List_First(ttList);
    while(tt) {
      if (cnt<1000) {
        if (tt->_refCounter!=3) {
          DBG_ERROR(GWEN_LOGDOMAIN, "Refcounter of object %d is not 3 (%d)", cnt, tt->_refCounter);
          dumpTestTypeList(ttList);
          TestType_List_free(ttList);
          GWEN_SimplePtrList_free(pl);
          return GWEN_ERROR_GENERIC;
        }
      }
      else {
        if (tt->_refCounter!=2) {
          DBG_ERROR(GWEN_LOGDOMAIN, "Refcounter of object is not 2");
          TestType_List_free(ttList);
          GWEN_SimplePtrList_free(pl);
          return GWEN_ERROR_GENERIC;
        }
      }
      cnt++;
      tt=TestType_List_Next(tt);
    }
    if (cnt!=1024) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Too few objects in list (%d instead of %d)", cnt, 1024);
    }
  }


  GWEN_SimplePtrList_free(plCopy);

  if (1) {
    TEST_TYPE *tt;
    int cnt=0;

    tt=TestType_List_First(ttList);
    while(tt) {
      if (cnt<1000) {
        if (tt->_refCounter!=2) {
          DBG_ERROR(GWEN_LOGDOMAIN, "Refcounter of object is not 2");
          TestType_List_free(ttList);
          GWEN_SimplePtrList_free(pl);
          return GWEN_ERROR_GENERIC;
        }
      }
      else {
        if (tt->_refCounter!=1) {
          DBG_ERROR(GWEN_LOGDOMAIN, "Refcounter of object is not 1");
          TestType_List_free(ttList);
          GWEN_SimplePtrList_free(pl);
          return GWEN_ERROR_GENERIC;
        }
      }
      cnt++;
      tt=TestType_List_Next(tt);
    }
    if (cnt!=1024) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Too few objects in list (%d instead of %d)", cnt, 1024);
    }
  }


  GWEN_SimplePtrList_free(pl);

  if (1) {
    TEST_TYPE *tt;
    int cnt=0;

    tt=TestType_List_First(ttList);
    while(tt) {
      if (tt->_refCounter!=1) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Refcounter of object is not 1");
        TestType_List_free(ttList);
        GWEN_SimplePtrList_free(pl);
        return GWEN_ERROR_GENERIC;
      }
      cnt++;
      tt=TestType_List_Next(tt);
    }
    if (cnt!=1024) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Too few objects in list (%d instead of %d)", cnt, 1024);
    }
  }

  TestType_List_free(ttList);

  return 0;
}



/* ------------------------------------------------------------------------------------------------
 * test 6: test lazy copying with attach/detach and setPtr
 * ------------------------------------------------------------------------------------------------
 */

int test6(void)
{

  GWEN_SIMPLEPTRLIST *pl;
  GWEN_SIMPLEPTRLIST *plCopy;
  TEST_TYPE_LIST *ttList;
  int i;

  pl=GWEN_SimplePtrList_new(128, 128);
  GWEN_SimplePtrList_SetAttachObjectFn(pl, _attachToTestType);
  GWEN_SimplePtrList_SetFreeObjectFn(pl, _detachFromTestType);
  GWEN_SimplePtrList_AddFlags(pl, GWEN_SIMPLEPTRLIST_FLAGS_ATTACHTOOBJECTS);
  GWEN_SimplePtrList_AddFlags(pl, GWEN_SIMPLEPTRLIST_FLAGS_DETACHFROMOBJECTS);

  ttList=TestType_List_new();
  for (i=0; i<1024; i++) {
    TEST_TYPE *tt;
    int64_t idx;

    tt=createTestType(i);
    TestType_List_Add(tt, ttList);
    idx=GWEN_SimplePtrList_AddPtr(pl, tt);
    if (idx<0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error adding pointer %d to list: %s (%d)", i, GWEN_Error_SimpleToString((int)idx), (int)idx);
      GWEN_SimplePtrList_free(pl);
      return (int) idx;
    }
  } /* for */

  plCopy=GWEN_SimplePtrList_LazyCopy(pl);
  if (plCopy==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not copy pointer list");
    GWEN_SimplePtrList_free(plCopy);
    GWEN_SimplePtrList_free(pl);
    return GWEN_ERROR_INVALID;
  }

  if (!(plCopy->flags & GWEN_SIMPLEPTRLIST_FLAGS_ATTACHTOOBJECTS)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid object field in list: flags (should have set GWEN_SIMPLEPTRLIST_FLAGS_ATTACHTOOBJECTS)");
    GWEN_SimplePtrList_free(plCopy);
    GWEN_SimplePtrList_free(pl);
    return GWEN_ERROR_GENERIC;
  }

  if (!(plCopy->flags & GWEN_SIMPLEPTRLIST_FLAGS_DETACHFROMOBJECTS)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Invalid object field in list: flags (should have set GWEN_SIMPLEPTRLIST_FLAGS_DETACHFROMOBJECTS)");
    GWEN_SimplePtrList_free(plCopy);
    GWEN_SimplePtrList_free(pl);
    return GWEN_ERROR_GENERIC;
  }


  if (1) {
    TEST_TYPE *tt;
    int rv;

    tt=createTestType(1024);
    TestType_List_Add(tt, ttList);
    rv=GWEN_SimplePtrList_SetPtrAt(plCopy, 100, tt);
    if (rv<0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error setting pointer %d in list: %s (%d)", 100, GWEN_Error_SimpleToString(rv), rv);
      GWEN_SimplePtrList_free(plCopy);
      GWEN_SimplePtrList_free(pl);
      return rv;
    }
  }


  for (i=0; i<1024; i++) {
    TEST_TYPE *tt;
    int rv;

    tt=(TEST_TYPE*) GWEN_SimplePtrList_GetPtrAt(plCopy, i);
    if (tt==NULL) {
      DBG_ERROR(GWEN_LOGDOMAIN, "No object at position %d", i);
      TestType_List_free(ttList);
      GWEN_SimplePtrList_free(plCopy);
      GWEN_SimplePtrList_free(pl);
      return GWEN_ERROR_GENERIC;
    }

    rv=TestType_TestHash(tt);
    if (rv<0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error testing hash of object %d: %s (%d)", i, GWEN_Error_SimpleToString(rv), rv);
      TestType_List_free(ttList);
      GWEN_SimplePtrList_free(plCopy);
      GWEN_SimplePtrList_free(pl);
      return rv;
    }
  } /* for */


  if (1) {
    TEST_TYPE *tt;
    int cnt=0;

    tt=TestType_List_First(ttList);
    while(tt) {
      if (cnt==1024 || cnt==100) {
        if (tt->_refCounter!=2) {
          DBG_ERROR(GWEN_LOGDOMAIN, "Refcounter of object %d is not 2 (%d)", cnt, tt->_refCounter);
          TestType_List_free(ttList);
          GWEN_SimplePtrList_free(plCopy);
          GWEN_SimplePtrList_free(pl);
          return GWEN_ERROR_GENERIC;
        }
      }
      else {
        if (tt->_refCounter!=3) {
          DBG_ERROR(GWEN_LOGDOMAIN, "Refcounter of object %d is not 3 (%d)", cnt, tt->_refCounter);
          TestType_List_free(ttList);
          GWEN_SimplePtrList_free(plCopy);
          GWEN_SimplePtrList_free(pl);
          return GWEN_ERROR_GENERIC;
        }
      }
      cnt++;
      tt=TestType_List_Next(tt);
    }
    if (cnt!=1025) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Too few objects in list (%d instead of %d)", cnt, 1024);
    }
  }

  if (1) {
    TEST_TYPE *tt1;
    TEST_TYPE *tt2;

    tt1=(TEST_TYPE*) GWEN_SimplePtrList_GetPtrAt(plCopy, 100);
    tt2=(TEST_TYPE*) GWEN_SimplePtrList_GetPtrAt(pl, 100);
    if (!(tt1 && tt2 && tt1!=tt2)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Pointers 100 are unexpectedly equal in both lists");
      TestType_List_free(ttList);
      GWEN_SimplePtrList_free(plCopy);
      GWEN_SimplePtrList_free(pl);
      return GWEN_ERROR_GENERIC;
    }

    if (tt1->_refCounter!=2) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Refcounter in copy list object 100 is not 2 (%d)", tt1->_refCounter);
      TestType_List_free(ttList);
      GWEN_SimplePtrList_free(plCopy);
      GWEN_SimplePtrList_free(pl);
      return GWEN_ERROR_GENERIC;
    }
  }

  GWEN_SimplePtrList_free(plCopy);
  GWEN_SimplePtrList_free(pl);
  TestType_List_free(ttList);

  return 0;
}







#else

int GWEN_SimplePtrList_Test(void)
{
  DBG_ERROR(GWEN_LOGDOMAIN, "Gwenhywfar was compiled without test code enabled.");
  return GWEN_ERROR_GENERIC;
}


#endif


