/***************************************************************************
 begin       : Thu Jan 09 2020
 copyright   : (C) 2020 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_TEST_FRAMEWORK_H
#define GWEN_TEST_FRAMEWORK_H

#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/testmodule.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef struct GWEN_TEST_FRAMEWORK GWEN_TEST_FRAMEWORK;



GWENHYWFAR_API GWEN_TEST_FRAMEWORK *TestFramework_new(void);

GWENHYWFAR_API void TestFramework_AddModule(GWEN_TEST_FRAMEWORK *tf, GWEN_TEST_MODULE *mod);

GWENHYWFAR_API int TestFramework_Run(GWEN_TEST_FRAMEWORK *tf, int argc, char **argv);

GWENHYWFAR_API GWEN_TEST_MODULE *TestFramework_GetModulesRoot(const GWEN_TEST_FRAMEWORK *tf);

GWENHYWFAR_API void TestFramework_free(GWEN_TEST_FRAMEWORK *tf);


#ifdef __cplusplus
}
#endif



#endif

