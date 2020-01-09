/***************************************************************************
 begin       : Thu Jan 09 2020
 copyright   : (C) 2020 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_TEST_FRAMEWORK_P_H
#define GWEN_TEST_FRAMEWORK_P_H

#include "testframework.h"



struct GWEN_TEST_FRAMEWORK {
  GWEN_TEST_MODULE *modulesRoot;
  GWEN_DB_NODE *paramsDb;
};




#endif

