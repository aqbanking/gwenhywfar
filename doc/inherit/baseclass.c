

#include "baseclass_p.h"


/* This implements the inherit functions which are specific to this
 * module. */
GWEN_INHERIT_FUNCTIONS(BASECLASS)



BASECLASS *BaseClass_new() {
  BASECLASS *baseClass;

  /* this macro simply allocates memory for a BASECLASS object and
   * presets the memory area with zeroes */
  GWEN_NEW_OBJECT(BASECLASS, baseClass)
  /* this macro allows GWEN to initialize the inheritance data defined
   * in baseclass_p.h within the struct definition (GWEN_INHERIT_ELEMENT) */
  GWEN_INHERIT_INIT(BASECLASS, baseClass)

  /* the rest is specific to our example module */
  baseClass->someMember=12345;
  return baseClass;
}



void BaseClass_free(BASECLASS *baseClass) {
  if (baseClass) {
    /* this macro calls the cleanup functions of all registered private
     * data pointers (if any) */
    GWEN_INHERIT_FINI(BASECLASS, baseClass)
    /* this macro simply calls free() for now, maybe later we can do some
     * memory leak checking with it */
    GWEN_FREE_OBJECT(baseClass);
  }
}


