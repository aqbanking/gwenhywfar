/***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#include <gwenhywfar/testframework.h>
#include <gwenhywfar/text.h>

#include "text-t.h"


#ifdef GWENHYWFAR_ENABLE_TESTCODE


/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int GWENHYWFAR_CB test1(GWEN_TEST_MODULE *mod);

static int GWENHYWFAR_CB _storeIndexCb(void *cbPtr, const char *name, int index, int maxLen, GWEN_BUFFER *dstBuf);
static int _checkIndex(const char *s, int expectedIndex);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */


int GWEN_Text_AddTests(GWEN_TEST_MODULE *mod)
{
  GWEN_TEST_MODULE *newMod;

  newMod=GWEN_Test_Module_AddModule(mod, "GWEN_Text", NULL);

  GWEN_Test_Module_AddTest(newMod, "ReplaceVars: variable index", test1, NULL);

  return 0;
}



int test1(GWEN_UNUSED GWEN_TEST_MODULE *mod)
{
  /* GWEN_Text_ReplaceVars() passes -1 for "[]", which means all values */
  if (_checkIndex("$(VAR)", 0) ||
      _checkIndex("$(VAR[3])", 3) ||
      _checkIndex("$(VAR[200])", 200) ||
      _checkIndex("$(VAR[])", -1))
    return GWEN_ERROR_GENERIC;
  return 0;
}



int _checkIndex(const char *s, int expectedIndex)
{
  GWEN_BUFFER *buf;
  int index=-2;
  int rv;

  buf=GWEN_Buffer_new(0, 64, 0, 1);
  rv=GWEN_Text_ReplaceVars(s, buf, _storeIndexCb, &index);
  GWEN_Buffer_free(buf);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Error replacing vars in \"%s\" (%d)", s, rv);
    return GWEN_ERROR_GENERIC;
  }
  if (index!=expectedIndex) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Index for \"%s\" is %d, expected %d", s, index, expectedIndex);
    return GWEN_ERROR_GENERIC;
  }
  return 0;
}



int GWENHYWFAR_CB _storeIndexCb(void *cbPtr,
                                GWEN_UNUSED const char *name,
                                int index,
                                GWEN_UNUSED int maxLen,
                                GWEN_UNUSED GWEN_BUFFER *dstBuf)
{
  *((int *) cbPtr)=index;
  return 0;
}



#else


int GWEN_Text_AddTests(GWEN_UNUSED GWEN_TEST_MODULE *mod)
{
  DBG_ERROR(GWEN_LOGDOMAIN, "Gwenhywfar was compiled without test code enabled.");
  return GWEN_ERROR_GENERIC;
}


#endif


