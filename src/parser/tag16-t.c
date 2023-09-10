/***************************************************************************
    begin       : Sun Jun 13 2004
    copyright   : (C) 2023 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#include <gwenhywfar/testframework.h>
#include <gwenhywfar/text.h>

#include "tag16-t.h"


#ifdef GWENHYWFAR_ENABLE_TESTCODE


/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int GWENHYWFAR_CB test1(GWEN_TEST_MODULE *mod);
static int GWENHYWFAR_CB test2(GWEN_TEST_MODULE *mod);
static int GWENHYWFAR_CB test3(GWEN_TEST_MODULE *mod);
static int GWENHYWFAR_CB test4(GWEN_TEST_MODULE *mod);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */


int GWEN_Tag16_AddTests(GWEN_TEST_MODULE *mod)
{
  GWEN_TEST_MODULE *newMod;

  newMod=GWEN_Test_Module_AddModule(mod, "GWEN_Tag16", NULL);

  GWEN_Test_Module_AddTest(newMod, "add/read uint32_t tag", test1, NULL);
  GWEN_Test_Module_AddTest(newMod, "add/read uint64_t tag", test2, NULL);
  GWEN_Test_Module_AddTest(newMod, "add/read string tag",   test3, NULL);
  GWEN_Test_Module_AddTest(newMod, "write/read tag lists",   test4, NULL);

  return 0;
}



int test1(GWEN_UNUSED GWEN_TEST_MODULE *mod)
{
  GWEN_BUFFER *buf;
  GWEN_TAG16 *tag;
  uint32_t value1=0x01234567;
  uint32_t value2;

  buf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Tag16_WriteUint32TagToBuffer(0xaa, value1, buf);
  GWEN_Buffer_Rewind(buf);
  tag=GWEN_Tag16_fromBuffer(buf, 0);
  if (tag==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Error parsing tag from this:");
    GWEN_Text_LogString(GWEN_Buffer_GetStart(buf), GWEN_Buffer_GetUsedBytes(buf), GWEN_LOGDOMAIN, GWEN_LoggerLevel_Error);
    GWEN_Buffer_free(buf);
    return GWEN_ERROR_GENERIC;
  }
  value2=GWEN_Tag16_GetTagDataAsUint32(tag, 0);
  if (value2!=value1) {
    DBG_ERROR(GWEN_LOGDOMAIN, "value read (%08x) != value written (%08x)", value2, value1);
    GWEN_Buffer_free(buf);
    return GWEN_ERROR_GENERIC;
  }
  GWEN_Buffer_free(buf);

  return 0;
}



int test2(GWEN_UNUSED GWEN_TEST_MODULE *mod)
{
  GWEN_BUFFER *buf;
  GWEN_TAG16 *tag;
  uint64_t value1=0x0123456789abcdefLL;
  uint64_t value2;

  buf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Tag16_WriteUint64TagToBuffer(0xbb, value1, buf);
  GWEN_Buffer_Rewind(buf);
  tag=GWEN_Tag16_fromBuffer(buf, 0);
  if (tag==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Error parsing tag from this:");
    GWEN_Text_LogString(GWEN_Buffer_GetStart(buf), GWEN_Buffer_GetUsedBytes(buf), GWEN_LOGDOMAIN, GWEN_LoggerLevel_Error);
    GWEN_Buffer_free(buf);
    return GWEN_ERROR_GENERIC;
  }
  value2=GWEN_Tag16_GetTagDataAsUint64(tag, 0);
  if (value2!=value1) {
    DBG_ERROR(GWEN_LOGDOMAIN, "value read (%016lx) != value written (%016lx)",
	      (unsigned long int) value2, (unsigned long int) value1);
    GWEN_Buffer_free(buf);
    return GWEN_ERROR_GENERIC;
  }
  GWEN_Buffer_free(buf);

  return 0;
}



int test3(GWEN_UNUSED GWEN_TEST_MODULE *mod)
{
  GWEN_BUFFER *buf;
  GWEN_TAG16 *tag;
  const char *value1="This is a test";
  char *value2;

  buf=GWEN_Buffer_new(0, 256, 0, 1);

  GWEN_Tag16_WriteStringTagToBuffer(0xcc, value1, buf);
  GWEN_Buffer_Rewind(buf);
  tag=GWEN_Tag16_fromBuffer(buf, 0);
  if (tag==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Error parsing tag from this:");
    GWEN_Text_LogString(GWEN_Buffer_GetStart(buf), GWEN_Buffer_GetUsedBytes(buf), GWEN_LOGDOMAIN, GWEN_LoggerLevel_Error);
    GWEN_Buffer_free(buf);
    return GWEN_ERROR_GENERIC;
  }
  value2=GWEN_Tag16_GetTagDataAsNewString(tag, NULL);
  if (value2==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No value read back");
    GWEN_Buffer_free(buf);
    return GWEN_ERROR_GENERIC;
  }
  if (strcmp(value1, value2)!=0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "value read (\"%s\") != value written (\"%s\")", value2, value1);
    free(value2);
    GWEN_Buffer_free(buf);
    return GWEN_ERROR_GENERIC;
  }
  free(value2);
  GWEN_Buffer_free(buf);
  return 0;
}



int test4(GWEN_UNUSED GWEN_TEST_MODULE *mod)
{
  GWEN_BUFFER *buf;
  GWEN_TAG16_LIST *tagList;
  const GWEN_TAG16 *tag;
  const char *value1="This is a test";
  uint32_t value2=0x12345678;
  uint64_t value3=0x0123456789abcdefLL;

  buf=GWEN_Buffer_new(0, 256, 0, 1);

  GWEN_Tag16_WriteStringTagToBuffer(0xcc, value1, buf);
  GWEN_Tag16_WriteUint32TagToBuffer(0xaa, value2, buf);
  GWEN_Tag16_WriteUint64TagToBuffer(0xbb, value3, buf);
  GWEN_Buffer_Rewind(buf);

  tagList=GWEN_Tag16_List_fromBuffer((const uint8_t*) GWEN_Buffer_GetStart(buf), GWEN_Buffer_GetUsedBytes(buf), 0);
  if (tagList==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Error parsing tag list from this:");
    GWEN_Text_LogString(GWEN_Buffer_GetStart(buf), GWEN_Buffer_GetUsedBytes(buf), GWEN_LOGDOMAIN, GWEN_LoggerLevel_Error);
    GWEN_Buffer_free(buf);
    return GWEN_ERROR_GENERIC;
  }

  /* test string_t */
  tag=GWEN_Tag16_List_FindFirstByTagType(tagList, 0xcc);
  if (tag==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Tag for string test not found");
    GWEN_Tag16_List_free(tagList);
    GWEN_Buffer_free(buf);
    return GWEN_ERROR_GENERIC;
  }
  else {
    char *value;

    value=GWEN_Tag16_GetTagDataAsNewString(tag, NULL);
    if (value==NULL) {
      DBG_ERROR(GWEN_LOGDOMAIN, "No string value read back");
      GWEN_Tag16_List_free(tagList);
      GWEN_Buffer_free(buf);
      return GWEN_ERROR_GENERIC;
    }
    if (strcmp(value1, value)!=0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "value read (\"%s\") != value written (\"%s\")", value, value1);
      free(value);
      GWEN_Tag16_List_free(tagList);
      GWEN_Buffer_free(buf);
      return GWEN_ERROR_GENERIC;
    }
    free(value);
  }

  /* test uint32_t */
  tag=GWEN_Tag16_List_FindFirstByTagType(tagList, 0xaa);
  if (tag==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Tag for uint32_t test not found");
    GWEN_Tag16_List_free(tagList);
    GWEN_Buffer_free(buf);
    return GWEN_ERROR_GENERIC;
  }
  else {
    uint32_t value;

    value=GWEN_Tag16_GetTagDataAsUint32(tag, 0);
    if (value!=value2) {
      DBG_ERROR(GWEN_LOGDOMAIN, "value read (%016lx) != value written (%016lx)",
		(unsigned long int) value, (unsigned long int) value2);
      GWEN_Tag16_List_free(tagList);
      GWEN_Buffer_free(buf);
      return GWEN_ERROR_GENERIC;
    }
  }

  /* test uint64_t */
  tag=GWEN_Tag16_List_FindFirstByTagType(tagList, 0xbb);
  if (tag==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Tag for uint64_t test not found");
    GWEN_Tag16_List_free(tagList);
    GWEN_Buffer_free(buf);
    return GWEN_ERROR_GENERIC;
  }
  else {
    uint64_t value;

    value=GWEN_Tag16_GetTagDataAsUint64(tag, 0);
    if (value!=value3) {
      DBG_ERROR(GWEN_LOGDOMAIN, "value read (%016lx) != value written (%016lx)",
		(unsigned long int) value, (unsigned long int) value3);
      GWEN_Tag16_List_free(tagList);
      GWEN_Buffer_free(buf);
      return GWEN_ERROR_GENERIC;
    }
  }

  GWEN_Tag16_List_free(tagList);
  GWEN_Buffer_free(buf);

  return 0;
}








#else


int GWEN_Tag16_AddTests(GWEN_TEST_MODULE *mod)
{
  DBG_ERROR(GWEN_LOGDOMAIN, "Gwenhywfar was compiled without test code enabled.");
  return GWEN_ERROR_GENERIC;
}


#endif



