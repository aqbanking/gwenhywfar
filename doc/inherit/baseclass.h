

#ifndef GWEN_EXAMPLE_INHERIT_BASECLASS_H
#define GWEN_EXAMPLE_INHERIT_BASECLASS_H

#include <gwenhywfar/inherit.h>


typedef struct BASECLASS BASECLASS;

/* this macro defines some internal inheritance functions which are needed
 * by GWEN's inheritance code. Please consider this macro a black box, don't
 * rely on its members! */
GWEN_INHERIT_FUNCTION_DEFS(BASECLASS)

BASECLASS *BaseClass_new();
void BaseClass_free(BASECLASS *baseClass);


#endif /* GWEN_EXAMPLE_INHERIT_BASECLASS_H */

