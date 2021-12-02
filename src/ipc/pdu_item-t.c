/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2021 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#if GWEN_ENABLE_TESTCODE


/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int GWENHYWFAR_CB test1(GWEN_TEST_MODULE *mod);




static const uint8_t _testData[]={
  0x01, 0x00, 0x00, 0x00, 0x00, 0xCD, 0x00, 0x01,
  0x00, 0x00, 0x41, 0x4E, 0x59, 0x2D, 0x53, 0x43,
  0x50, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x45, 0x43, 0x48, 0x4F, 0x53, 0x43,
  0x55, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x10, 0x00, 0x00, 0x15, 0x31, 0x2E,
  0x32, 0x2E, 0x38, 0x34, 0x30, 0x2E, 0x31, 0x30,
  0x30, 0x30, 0x38, 0x2E, 0x33, 0x2E, 0x31, 0x2E,
  0x31, 0x2E, 0x31, 0x20, 0x00, 0x00, 0x2E, 0x01,
  0x00, 0xFF, 0x00, 0x30, 0x00, 0x00, 0x11, 0x31,
  0x2E, 0x32, 0x2E, 0x38, 0x34, 0x30, 0x2E, 0x31,
  0x30, 0x30, 0x30, 0x38, 0x2E, 0x31, 0x2E, 0x31,
  0x40, 0x00, 0x00, 0x11, 0x31, 0x2E, 0x32, 0x2E,
  0x38, 0x34, 0x30, 0x2E, 0x31, 0x30, 0x30, 0x30,
  0x38, 0x2E, 0x31, 0x2E, 0x32, 0x50, 0x00, 0x00,
  0x3A, 0x51, 0x00, 0x00, 0x04, 0x00, 0x00, 0x40,
  0x00, 0x52, 0x00, 0x00, 0x1B, 0x31, 0x2E, 0x32,
  0x2E, 0x32, 0x37, 0x36, 0x2E, 0x30, 0x2E, 0x37,
  0x32, 0x33, 0x30, 0x30, 0x31, 0x30, 0x2E, 0x33,
  0x2E, 0x30, 0x2E, 0x33, 0x2E, 0x36, 0x2E, 0x33,
  0x55, 0x00, 0x00, 0x0F, 0x4F, 0x46, 0x46, 0x49,
  0x53, 0x5F, 0x44, 0x43, 0x4D, 0x54, 0x4B, 0x5F,
  0x33, 0x36, 0x33 };




/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



int GWEN_PduItem_AddTests(GWEN_TEST_MODULE *mod)
{
  GWEN_TEST_MODULE *newMod;

  newMod=GWEN_Test_Module_AddModule(mod, "GWEN_PdduItem", NULL);

  GWEN_Test_Module_AddTest(newMod, "test1", test1, NULL);

  return 0;
}



int test1(GWEN_UNUSED GWEN_TEST_MODULE *mod)
{
  uint32_t size;
  int rv;
  GWEN_PDU_ITEM *pdu;

  size=sizeof(_testData);

  pdu=GWEN_PduItem_new();
  rv=GWEN_PduItem_ReadBuffer32(pdu, _testData, size);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Error in GWEN_PduItem_ReadBuffer32(): %d", rv);
    return rv;
  }

  return 0;
}









#else

int GWEN_PduItem_AddTests(GWEN_UNUSED GWEN_TEST_MODULE *mod)
{
  DBG_ERROR(GWEN_LOGDOMAIN, "Gwenhywfar was compiled without test code enabled.");
  return GWEN_ERROR_GENERIC;
}


#endif




