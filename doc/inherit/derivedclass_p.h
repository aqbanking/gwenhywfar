

#ifndef GWEN_EXAMPLE_INHERIT_DERIVEDCLASS_P_H
#define GWEN_EXAMPLE_INHERIT_DERIVEDCLASS_P_H

#include "derivedclass.h"


typedef struct DERIVEDCLASS DERIVEDCLASS;
struct DERIVEDCLASS {
  /* this struct only contains private data for the derived class.
   * The inheritance administration data is only needed in the base class. */
  char *testData;
};
void GWENHYWFAR_CB DerivedClass_FreeData(void *bp, void *p);


#endif /* GWEN_EXAMPLE_INHERIT_DERIVEDCLASS_P_H */



