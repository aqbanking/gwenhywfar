

#ifndef GWEN_EXAMPLE_INHERIT_DERIVEDCLASS_H
#define GWEN_EXAMPLE_INHERIT_DERIVEDCLASS_H

#include "baseclass.h"

/* constructor of the derived class. Please note that a pointer to the
 * underlying base class is returned, not to the derived class!
 * Functions of the derived class can get their private data (in this case
 * DERIVEDCLASS*) via GWEN_INHERIT_GETDATA (see "derivedclass.c").
 * Please note also that there is no destructor: This constructor registers
 * a callback function which does a cleanup of the DERIVEDCLASS data when
 * the destructor of the base class (in our example "BaseClass_free()") is
 * called. */
BASECLASS *DerivedClass_new(const char *data);

/* This is a function of the derived class. It can only operate on BASECLASS
 * object which have been created by "DerivedClass_new()" because only then
 * a private pointer to DERIVEDCLASS is registered with the base object. */
const char *DerivedClass_GetTestData(const BASECLASS *baseClass);


#endif /* GWEN_EXAMPLE_INHERIT_DERIVEDCLASS_H */



