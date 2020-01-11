
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "derivedclass_p.h"

/* this macro announces to the inheritance code that we are about to
 * extend BASECLASS with DERIVEDCLASS. Please consider this macro a black box,
 * don't rely on the implementation of this macro! */
GWEN_INHERIT(BASECLASS, DERIVEDCLASS)



BASECLASS *DerivedClass_new(const char *data)
{
  BASECLASS *baseClass;
  DERIVEDCLASS *derivedClass;

  /* first create the base class by calling its constructor. This also sets
   * up the inheritance data stored with the base object */
  baseClass=BaseClass_new();

  /* create our private data which is to be assigned to the base object
   * in the next step */
  GWEN_NEW_OBJECT(DERIVEDCLASS, derivedClass)
  derivedClass->testData=strdup(data);

  /* now link our private data to the base object. This makes also sure that
   * out provided cleanup function "DerivedClass_FreeData()" is called as
   * soon as the destructor of the base object is called. */
  GWEN_INHERIT_SETDATA(BASECLASS, DERIVEDCLASS, baseClass, derivedClass,
                       DerivedClass_FreeData);

  /* return the pointer to the created base class */
  return baseClass;
}



/* the first pointer is a pointer to the base class (in our case BASECLASS)
 * the second is a pointer to the private data (here DERIVEDCLASS).
 * When this function is called the base object still exists. */
void GWENHYWFAR_CB DerivedClass_FreeData(GWEN_UNUSED void *bp, void *p)
{
  DERIVEDCLASS *derivedClass;

  derivedClass=(DERIVEDCLASS *) p;
  free(derivedClass->testData);
  GWEN_FREE_OBJECT(derivedClass);
}



const char *DerivedClass_GetTestData(const BASECLASS *baseClass)
{
  DERIVEDCLASS *derivedClass;

  assert(baseClass);
  /* get the pointer to our private data which we assigned via
   * "GWEN_INHERIT_SETDATA" */
  derivedClass=GWEN_INHERIT_GETDATA(BASECLASS, DERIVEDCLASS, baseClass);
  assert(derivedClass);

  return derivedClass->testData;
}





