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
struct TEST_TYPE {
  int testPosition;
  char *testStringPtr;
  uint8_t testHash[20];
};


/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */


TEST_TYPE *TestType_new(const char *s, int pos);
void TestType_free(TEST_TYPE *tt);

int TestType_CalcHash(TEST_TYPE *tt);
int TestType_TestHash(const TEST_TYPE *tt);


int test1(void);
int test2(void);
int test3(void);
TEST_TYPE *createTestType(int num);



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

  if (numFailed)
    return GWEN_ERROR_GENERIC;
  return 0;
}





TEST_TYPE *TestType_new(const char *s, int pos)
{
  TEST_TYPE *tt;

  GWEN_NEW_OBJECT(TEST_TYPE, tt);
  if (s)
    tt->testStringPtr=strdup(s);
  tt->testPosition=pos;
  return tt;
}



void TestType_free(TEST_TYPE *tt)
{
  if (tt) {
    if (tt->testStringPtr)
      free(tt->testStringPtr);
    GWEN_FREE_OBJECT(tt);
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







#else

int GWEN_SimplePtrList_Test(void)
{
  DBG_ERROR(GWEN_LOGDOMAIN, "Gwenhywfar was compiled without test code enabled.");
  return GWEN_ERROR_GENERIC;
}


#endif


